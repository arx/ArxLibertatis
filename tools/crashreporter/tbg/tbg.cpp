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
#include <QXmlStreamReader>

#include <boost/range/size.hpp>

namespace TBG
{

Server::Server(const QString& serverAddress)
	: m_ServerAddress(serverAddress)
	, m_ServerPrefix(serverAddress + "/arxcrashreporter/v1")
{
}

bool Server::login(const QString& username, const QString& password)
{
	QUrl params;
	
	params.addQueryItem("tbg3_password", password);
	params.addQueryItem("tbg3_referer", m_ServerAddress);
	params.addQueryItem("tbg3_username", username);
	
	QUrl loginUrl = m_ServerPrefix + "/do/login";
	QNetworkRequest request(loginUrl);
	request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");
	
	QByteArray data = params.encodedQuery();
	m_CurrentReply = m_NetAccessManager.post(request, data);
	
	bool bSucceeded = waitForReply();
	m_CurrentReply->deleteLater();
	
	return bSucceeded;
}

bool Server::createCrashReport(const QString& title, const QString& description, const QString& reproSteps, int version_id, int& issue_id)
{
	QUrl params;
	
	params.addQueryItem("project_id", "2");
	params.addQueryItem("issuetype_id", "7");
	params.addQueryItem("title", title);
	params.addQueryItem("description", description);
	params.addQueryItem("reproduction_steps", reproSteps);
	params.addQueryItem("build_id", QString::number(version_id));
	params.addQueryItem("component_id", "");
	params.addQueryItem("category_id", "0");
	params.addQueryItem("reproducability_id", "0");
	params.addQueryItem("estimated_time", "");
	params.addQueryItem("priority_id", "0");
	params.addQueryItem("resolution_id", "0");
	params.addQueryItem("severity_id", "0");
	
	QUrl newIssueUrl = m_ServerPrefix + "/arx/issues/new";
	QNetworkRequest request(newIssueUrl);
	request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");
	
	QByteArray data = params.encodedQuery();
	
	m_CurrentReply = m_NetAccessManager.post(request, data);
	bool bSucceeded = waitForReply();
	if(bSucceeded)
	{
		QUrl currentUrl = m_CurrentReply->url();
		m_CurrentReply->deleteLater();
		bSucceeded = getIssueIdFromUrl(currentUrl, issue_id);
	}
	
	return bSucceeded;
}

bool Server::addComment(int issue_id, const QString& comment)
{
	QUrl params;

	params.addQueryItem("comment_visibility", "1");
	params.addQueryItem("comment_body", comment);
	params.addQueryItem("comment_save_changes", "1");
	
	QString strUrl = QString("/comment/add/for/module/core/item/type/1/id/%1").arg(QString::number(issue_id));

	QUrl newIssueUrl = m_ServerPrefix + strUrl;
	QNetworkRequest request(newIssueUrl);
	request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");

	QByteArray data = params.encodedQuery();

	m_CurrentReply = m_NetAccessManager.post(request, data);
	bool bSucceeded = waitForReply();
	m_CurrentReply->deleteLater();
	
	return bSucceeded;
}

bool Server::setOperatingSystem(int issue_id, int os_id)
{
	return setFieldValue("operatingsystem", issue_id, os_id);
}

bool Server::setArchitecture(int issue_id, int arch_id)
{
	return setFieldValue("architecture", issue_id, arch_id);
}

bool Server::setFieldValue(const QString& fieldName, int issue_id, int value_id)
{
	QString strUrl = QString("/arx/issues/%1/set/%2/%2_value/%3").arg(QString::number(issue_id),
																	  fieldName,
																	  QString::number(value_id));

	QUrl newIssueUrl = m_ServerPrefix + strUrl;
	QNetworkRequest request(newIssueUrl);
	m_CurrentReply = m_NetAccessManager.get(request);
	bool bSucceeded = waitForReply();
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

	QString urlString = m_ServerPrefix + "/upload/to/issue/" + QString::number(issue_id);
	QUrl url(urlString);
	QNetworkRequest request(url);
	request.setRawHeader("Content-Type","multipart/form-data; boundary=\""+boundaryRegular+"\"");
	request.setHeader(QNetworkRequest::ContentLengthHeader,dataToSend.size());

	m_CurrentReply = m_NetAccessManager.post(request,dataToSend);
	bool bSucceeded = waitForReply();
	m_CurrentReply->deleteLater();
	return bSucceeded;
}

bool Server::findIssue(const QString& text, int& issue_id)
{
	issue_id = -1; // Not found

	QString addrSearchRSS = m_ServerPrefix +  "/arx/issues/find/format/rss?issues/project_key/arx&filters[text][operator]==&filters[text][value]=";
	addrSearchRSS += text;
	QUrl urlSearchRSS(addrSearchRSS);

	QNetworkRequest request(urlSearchRSS);
	m_CurrentReply = m_NetAccessManager.get(request);
	bool bSucceeded = waitForReply();
	if(bSucceeded)
	{
		QXmlStreamReader xml ;
		xml.setDevice(m_CurrentReply);

		// Using XPath would have probably been simpler, but it would
		// add a dependency to QtXmlPatterns...
		// Look for the first "/rss/channel/item/link" entry...
		const QString XML_PATH[] = { "rss", "channel", "item", "link" };
		int currentItem = 0;
		QString issueLink;

		while(!xml.atEnd() && !xml.hasError())
		{
			// Read next element
			QXmlStreamReader::TokenType token = xml.readNext();

			if(currentItem == boost::size(XML_PATH))
			{
				issueLink = xml.text().toString();
				break;
			}

			// If token is StartElement, we'll see if we can read it.
			if(token == QXmlStreamReader::StartElement) 
			{
				if(xml.name() == XML_PATH[currentItem])
					currentItem++;
			}
		}

		if(!issueLink.isEmpty())
		{
			bSucceeded = getIssueIdFromUrl(QUrl(issueLink), issue_id);
		}
		else
		{
			bSucceeded = true; // Issue not found, but search was successful nonetheless, issue_id will be -1
		}
	}

	return bSucceeded;
}

bool Server::getIssueIdFromUrl(const QUrl& url, int& issue_id)
{
	QUrl issueJSON = url.toString() + "?format=json";
	QNetworkRequest request(issueJSON);

	m_CurrentReply = m_NetAccessManager.get(request);
	bool bSucceeded = waitForReply();
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
