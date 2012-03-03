/*
 * Copyright 2011 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "crashreporter/tbg/tbg.h"

#include <QFile>
#include <QFileInfo>
#include <QEventLoop>

namespace TBG
{

Server::Server(const QString& serverAddress)
	: m_ServerAddress(serverAddress)
{
}

bool Server::login(const QString& username, const QString& password)
{
	QUrl params;

	params.addQueryItem("tbg3_password", password);
	params.addQueryItem("tbg3_referer", m_ServerAddress);
	params.addQueryItem("tbg3_username", username);
			
	QUrl loginUrl = m_ServerAddress + "/do/login";
	QNetworkRequest request(loginUrl);
	request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");

	QByteArray data = params.encodedQuery();
	m_CurrentReply = m_NetAccessManager.post(request, data);

	bool bSucceeded = waitForReply();
	m_CurrentReply->deleteLater();
	
	return bSucceeded;
}

bool Server::createCrashReport(const QString& title, const QString& description, int& issue_id)
{
	QUrl params;

	params.addQueryItem("project_id", "2");
	params.addQueryItem("issuetype_id", "7");
	params.addQueryItem("title", title);
	params.addQueryItem("description", description);
	params.addQueryItem("reproduction_steps", "");
	params.addQueryItem("build_id", "1");
	params.addQueryItem("component_id", "");
	params.addQueryItem("category_id", "0");
	params.addQueryItem("reproducability_id", "0");
	params.addQueryItem("estimated_time", "");
	params.addQueryItem("priority_id", "0");
	params.addQueryItem("resolution_id", "0");
	params.addQueryItem("severity_id", "0");
			
	QUrl newIssueUrl = m_ServerAddress + "/arx/issues/new";
	QNetworkRequest request(newIssueUrl);
	request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");

	QByteArray data = params.encodedQuery();

	m_CurrentReply = m_NetAccessManager.post(request, data);
	bool bSucceeded = waitForReply();
	if(bSucceeded)
	{
		QUrl issueJSON = m_CurrentReply->url().toString() + "?format=json";
		QNetworkRequest request(issueJSON);

		m_CurrentReply->deleteLater();
		m_CurrentReply = m_NetAccessManager.get(request);
		bSucceeded = waitForReply();
		if(bSucceeded)
		{
			QByteArray data = m_CurrentReply->readAll();

			issue_id = -1;

			QString dataStr(data);
			const QString idToken = "{\"id\":";
			if(dataStr.startsWith(idToken))
			{
				int posEnd = dataStr.indexOf(",");
				if(posEnd != -1)
					issue_id = dataStr.mid(idToken.length(), posEnd - idToken.length()).toInt();
			}

			if(issue_id == -1)
				bSucceeded = false;
		}
	}

	m_CurrentReply->deleteLater();
	return bSucceeded;
}


bool Server::attachFile(int issue_id, const QString& filePath, const QString& fileDescription, const QString& comment)
{
	QFile file(filePath, this);
	if(!file.open(QIODevice::ReadOnly))
		return false;
	
	QFileInfo fileInfo(file);	

	QByteArray boundaryRegular(QString("--"+QString::number(qrand(), 10)).toAscii());
	QByteArray boundary("\r\n--"+boundaryRegular+"\r\n");
	QByteArray boundaryLast("\r\n--"+boundaryRegular+"--\r\n");

	QByteArray dataToSend;
	dataToSend.append("--"+boundaryRegular+"\r\n");
	dataToSend.append("Content-Disposition: form-data; name=\"uploader_file\"; filename=\""+fileInfo.fileName().toUtf8()+"\"\r\n");
	dataToSend.append("Content-Type: application/octet-stream\r\n\r\n");
	dataToSend.append(file.readAll());
	dataToSend.append(boundary);
	dataToSend.append("Content-Disposition: form-data; name=\"uploader_file_description\"\r\n\r\n");
	dataToSend.append(fileDescription.toUtf8());
	dataToSend.append(boundary);
	dataToSend.append("Content-Disposition: form-data; name=\"comment\"\r\n\r\n");
	dataToSend.append(comment.toUtf8());
	dataToSend.append(boundaryLast);

	QString urlString = QString("https://bugs.arx-libertatis.org/upload/to/issue/") + QString::number(issue_id);
	QUrl url(urlString);
	QNetworkRequest request(url);
	request.setRawHeader("Content-Type","multipart/form-data; boundary=\""+boundaryRegular+"\"");
	request.setHeader(QNetworkRequest::ContentLengthHeader,dataToSend.size());

	m_CurrentReply = m_NetAccessManager.post(request,dataToSend);
	bool bSucceeded = waitForReply();
	m_CurrentReply->deleteLater();
	return bSucceeded;
}

bool Server::waitForReply()
{
	QUrl lastRedirectUrl;
	do
	{
		QEventLoop loop;
		loop.connect(m_CurrentReply, SIGNAL(finished()), SLOT(quit()));
		loop.exec();

		// Handle redirections
		QUrl redirectUrl = m_CurrentReply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
		if(!redirectUrl.isEmpty() && redirectUrl != lastRedirectUrl)
		{
			lastRedirectUrl = redirectUrl;

			// Attribute "RedirectionTargetAttribute" might return a relative URL, so we must resolve it
			QUrl baseUrl = m_ServerAddress;
			redirectUrl = baseUrl.resolved(redirectUrl);

			m_CurrentReply->deleteLater();
			m_CurrentReply = m_NetAccessManager.get(QNetworkRequest(redirectUrl));
		}
		else
		{
			lastRedirectUrl.clear();
		}
	} while(!lastRedirectUrl.isEmpty());

	m_CurrentUrl = m_CurrentReply->url();

	return m_CurrentReply->error() == QNetworkReply::NoError;
}

QUrl Server::getUrl() const
{
	QUrl httpUrl = m_CurrentUrl;
	httpUrl.setScheme("http");
	return httpUrl;
}

} // namespace TBG
