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

#ifndef ARX_TOOLS_CRASHREPORTER_TBG_TBG_H
#define ARX_TOOLS_CRASHREPORTER_TBG_TBG_H

#include <string>

#include <QFuture>
#include <QString>

namespace http {
	class Session;
	class Request;
	class POSTRequest;
	class Response;
} // namespace http

namespace TBG {

class Server : public QObject {
	
public:
	
	enum OperatingSystem {
		OS_Linux = 100,   // Linux
		OS_macOS = 200,   // macOS
		OS_FreeBSD = 300, // FreeBSD
		OS_BSD = 350,     // *BSD
		OS_Windows = 400, // Windows
		OS_Other = -1
	};
	
	enum Architecture {
		Arch_Amd64 = 3,
		Arch_x86 = 2,
		Arch_Other = -1
	};
	
	explicit Server(const QString & serverAddress, const std::string & userAgent);
	~Server();
	
	bool login(const QString & username, const QString & password);
	QString createCrashReport(const QString & title, const QString & description,
	                          const QString & reproSteps, int version_id, int & issue_id);
	bool addComment(int issue_id, const QString & comment);
	bool setOperatingSystem(int issue_id, int os_id);
	bool setArchitecture(int issue_id, int arch_id);
	bool attachFile(int issue_id, const QString & filePath, const QString & fileDescription,
	                const QString & comment);
	QString findIssue(const QString & text, int & issue_id);
	
	const QString & getErrorString() const { return m_lastErrorString; }
	
private:
	
	http::Response * wait(const QFuture<http::Response *> & future);
	http::Response * get(const http::Request & request);
	http::Response * post(const http::POSTRequest & request);
	
	bool setFieldValue(const QString & fieldName, int issue_id, int value_id);
	bool getIssueIdFromUrl(const std::string & url, int & issue_id);
	
	QString m_serverAddress;
	QString m_serverPrefix;
	http::Session * m_session;
	QString m_lastErrorString;
	
};

} // namespace TBG

#endif // ARX_TOOLS_CRASHREPORTER_TBG_TBG_H
