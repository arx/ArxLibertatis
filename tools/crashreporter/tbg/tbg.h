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

#ifndef ARX_TOOLS_CRASHREPORTER_TBG_TBG_H
#define ARX_TOOLS_CRASHREPORTER_TBG_TBG_H

#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

namespace TBG
{

class Server : public QObject
{
public:	
	enum OperatingSystem
	{
		OS_Linux = 100,			// Linux
		OS_MacOSX = 200,		// Mac
		OS_FreeBSD = 300,		// FreeBSD
		OS_Windows = 400,		// Windows
		OS_Other = 0xFFFFFFFF
	};

	enum Architecture
	{
		Arch_Amd64 = 1,
		Arch_x86 = 2,
		Arch_Other = 0xFFFFFFFF
	};

	explicit Server(const QString & adress);

	bool login(const QString& username, const QString& password);
	bool createCrashReport(const QString& title, const QString& description, int& issue_id);
	bool setOperatingSystem(int issue_id, int os_id);
	bool setArchitecture(int issue_id, int arch_id);
	bool attachFile(int issue_id, const QString& filePath, const QString& fileDescription, const QString& comment);
	bool findIssue(const QString& text, int& issue_id);

	QUrl getUrl() const;

private:
	bool waitForReply();
	bool setFieldValue(const QString& fieldName, int issue_id, int value_id);
	bool getIssueIdFromUrl(const QUrl& url, int& issue_id);

private:
	QString               m_ServerAddress;
	QString               m_ServerPrefix;
	QNetworkAccessManager m_NetAccessManager;
	QNetworkReply *       m_CurrentReply;
	QUrl                  m_CurrentUrl;
};

}

#endif // ARX_TOOLS_CRASHREPORTER_TBG_TBG_H
