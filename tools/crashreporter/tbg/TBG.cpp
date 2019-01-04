/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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

#include "crashreporter/tbg/TBG.h"

#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QFutureWatcher>
#include <QTimer>
#include <QUrl>
#include <QUuid>
#include <QXmlStreamReader>
#include <QtConcurrentRun>

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QUrlQuery>
#else
typedef QUrl QUrlQuery;
#endif

#include <boost/scoped_ptr.hpp>
#include <boost/range/size.hpp>

#include "crashreporter/tbg/HTTPClient.h"

namespace TBG {

static std::string toUTF8(const QString & string) {
	QByteArray data = string.toUtf8();
	return std::string(data, size_t(data.length()));
}

static std::string toStdString(const QByteArray & string) {
	return std::string(string.data(), string.length());
}

static QString toQString(const std::string & string) {
	return QString::fromUtf8(string.data(), string.length());
}

static QByteArray toQByteArray(const std::string & string) {
	return QByteArray(string.data(), string.length());
}

Server::Server(const QString & serverAddress, const std::string & userAgent)
	: m_serverAddress(serverAddress)
	, m_serverPrefix(serverAddress + "/arxcrashreporter/v1")
	, m_session(http::createSession(userAgent))
{ }

Server::~Server() {
	delete m_session;
}

static std::string qUrlQueryToPostData(const QUrlQuery & query) {
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
	return toStdString(query.encodedQuery());
#else
	return toUTF8(query.query(QUrl::FullyEncoded));
#endif
}

http::Response * Server::wait(const QFuture<http::Response *> & future) {
	
	QEventLoop loop;
	
	// Interrrupt the event loop when the result is available
	QFutureWatcher<http::Response *> watcher;
	watcher.setFuture(future);
	loop.connect(&watcher, SIGNAL(finished()), SLOT(quit()));
	
	// Prevent infinite loop if the future completes before we start the loop
	QTimer timer;
	loop.connect(&timer, SIGNAL(timeout()), SLOT(quit()));
	timer.setSingleShot(false);
	timer.start(1000);
	
	// Process events while waiting so that the UI stays responsive
	while(!future.isFinished()) {
		loop.exec();
	}
	
	http::Response * response = future.result();
	
	if(response->ok()) {
		m_lastErrorString.clear();
	} else if(!response->error().empty()) {
		m_lastErrorString = toQString(response->error());
	} else if(!response->data().empty()) {
		m_lastErrorString = toQString(response->data());
	} else {
		m_lastErrorString = "HTTP Error " + QString::number(response->status());
	}
	
	return response;
}

http::Response * Server::get(const http::Request & request) {
	return wait(QtConcurrent::run(m_session, &http::Session::get, request));
}

http::Response * Server::post(const http::POSTRequest & request) {
	return wait(QtConcurrent::run(m_session, &http::Session::post, request));
}

bool Server::login(const QString & username, const QString & password) {
	
	QUrlQuery params;
	params.addQueryItem("tbg3_password", password);
	params.addQueryItem("tbg3_referer", m_serverAddress);
	params.addQueryItem("tbg3_username", username);
	
	http::POSTRequest request(toUTF8(m_serverPrefix + "/do/login"));
	request.setData(qUrlQueryToPostData(params));
	request.setFollowRedirects(false);
	
	boost::scoped_ptr<http::Response> response(post(request));
	
	return response->ok();
}

QString Server::createCrashReport(const QString & title, const QString & description,
                                  const QString & reproSteps, int version_id,
                                  int & issue_id) {
	
	QUrlQuery params;
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
	
	http::POSTRequest request(toUTF8(m_serverPrefix + "/arx/issues/new"));
	request.setData(qUrlQueryToPostData(params));
	request.setFollowRedirects(false);
	
	boost::scoped_ptr<http::Response> response(post(request));
	
	if(response->ok() && getIssueIdFromUrl(response->url(), issue_id)) {
		return toQString(response->url());
	}
	
	return QString();
}

bool Server::addComment(int issue_id, const QString & comment) {
	
	QUrlQuery params;
	params.addQueryItem("comment_visibility", "1");
	params.addQueryItem("comment_body", comment);
	params.addQueryItem("comment_save_changes", "1");
	
	QString format = "/comment/add/for/module/core/item/type/1/id/%1";
	QString url = format.arg(QString::number(issue_id));

	http::POSTRequest request(toUTF8(m_serverPrefix + url));
	request.setData(qUrlQueryToPostData(params));
	request.setFollowRedirects(false);
	
	boost::scoped_ptr<http::Response> response(post(request));
	
	return response->ok();
}

bool Server::setOperatingSystem(int issue_id, int os_id) {
	return setFieldValue("operatingsystem", issue_id, os_id);
}

bool Server::setArchitecture(int issue_id, int arch_id) {
	return setFieldValue("architecture", issue_id, arch_id);
}

bool Server::setFieldValue(const QString & fieldName, int issue_id, int value_id) {
	
	QString format = "/arx/issues/%1/set/%2/%2_value/%3";
	QString url = format.arg(QString::number(issue_id), fieldName,
	                         QString::number(value_id));

	http::POSTRequest request(toUTF8(m_serverPrefix + url));
	request.setFollowRedirects(false);
	
	boost::scoped_ptr<http::Response> response(post(request));
	
	return response->ok();
}

bool Server::attachFile(int issue_id, const QString & filePath,
                        const QString & fileDescription, const QString & comment) {
	
	QFile file(filePath, this);
	if(!file.open(QIODevice::ReadOnly)) {
		return false;
	}
	
	if(file.size() > 20 * 1024 * 1024) {
		return false;
	}
	
	QString uuid = QUuid::createUuid().toString();
	QByteArray boundaryRegular = ("--" + uuid.mid(1, uuid.length() - 2)).toUtf8();
	QByteArray boundary = "\r\n--" + boundaryRegular + "\r\n";
	QByteArray boundaryLast = "\r\n--" + boundaryRegular + "--\r\n";

	QByteArray data;
	data.append("--" + boundaryRegular + "\r\n");
	data.append("Content-Disposition: form-data; name=\"uploader_file\"; filename=\""
	            + QFileInfo(file).fileName().toUtf8() + "\"\r\n");
	data.append("Content-Type: application/octet-stream\r\n\r\n");
	data.append(file.readAll());
	data.append(boundary);
	data.append("Content-Disposition: form-data; name=\"uploader_file_description\"\r\n\r\n");
	data.append(fileDescription.toUtf8());
	data.append(boundary);
	data.append("Content-Disposition: form-data; name=\"comment\"\r\n\r\n");
	data.append(comment.toUtf8());
	data.append(boundaryLast);
	
	QByteArray contentType = "multipart/form-data; boundary=\"" + boundaryRegular + "\"";

	QString url = "/upload/to/issue/" + QString::number(issue_id);
	
	http::POSTRequest request(toUTF8(m_serverPrefix + url));
	request.setData(toStdString(data));
	request.setContentType(toStdString(contentType));
	request.setFollowRedirects(false);
	
	boost::scoped_ptr<http::Response> response(post(request));
	
	return response->ok();
}

QString Server::findIssue(const QString & text, int & issue_id) {
	
	issue_id = -1; // Not found
	
	QUrlQuery params;
	params.addQueryItem("filters[text][operator]", "=");
	params.addQueryItem("filters[text][value]", text);
	
	QString url = "/arx/issues/find/format/rss?issues/project_key/arx&"
	              + toQString(qUrlQueryToPostData(params));
	
	http::Request request(toUTF8(m_serverPrefix + url));
	
	boost::scoped_ptr<http::Response> response(get(request));
	
	if(!response->ok()) {
		return QString();
	}
	
	QXmlStreamReader xml(toQByteArray(response->data()));
	
	// Using XPath would have probably been simpler, but it would
	// add a dependency to QtXmlPatterns...
	// Look for the first "/rss/channel/item/link" entry...
	const QString XML_PATH[] = { "rss", "channel", "item", "link" };
	
	size_t currentItem = 0;
	QString issueLink;
	while(!xml.atEnd() && !xml.hasError()) {
		
		// Read next element
		QXmlStreamReader::TokenType token = xml.readNext();
		
		if(currentItem == size_t(boost::size(XML_PATH))) {
			QString issue = xml.text().toString();
			if(getIssueIdFromUrl(toUTF8(issue), issue_id)) {
				return issue;
			}
			break;
		}
		
		// If token is StartElement, we'll see if we can read it.
		if(token == QXmlStreamReader::StartElement && xml.name() == XML_PATH[currentItem]) {
			currentItem++;
		}
		
	}
	
	return QString();
}

bool Server::getIssueIdFromUrl(const std::string & url, int & issue_id) {
	
	issue_id = -1; // Not found
	
	if(url.empty()) {
		m_lastErrorString = "Could not get issue URL";
		return false;
	}
	
	http::Request request(url + "?format=json");
	
	boost::scoped_ptr<http::Response> response(get(request));
	
	if(!response->ok()) {
		return false;
	}
	
	QString data = toQString(response->data());
	
	const QString idToken = "{\"id\":";
	if(data.startsWith(idToken)) {
		int posEnd = data.indexOf(",");
		if(posEnd != -1) {
			issue_id = data.mid(idToken.length(), posEnd - idToken.length()).toInt();
			if(issue_id >= 0) {
				return true;
			}
		}
	}
	
	m_lastErrorString = "Could not get issue ID";
	return false;
}

} // namespace TBG
