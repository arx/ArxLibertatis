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
	
	explicit Server(const QString & adress);

	bool login(const QString& username, const QString& password);
	bool createCrashReport(const QString& title, const QString& description, int& issue_id);
	bool attachFile(int issue_id, const QString& filePath, const QString& fileDescription, const QString& comment);
	bool findIssue(const QString& text, int& issue_id);

	QUrl getUrl() const;

private:
	bool waitForReply();
	bool getIssueIdFromUrl(const QUrl& url, int& issue_id);

private:
	QString					m_ServerAddress;
	QNetworkAccessManager	m_NetAccessManager;
	QNetworkReply*			m_CurrentReply;
	QUrl					m_CurrentUrl;
};

}

#endif // ARX_TOOLS_CRASHREPORTER_TBG_TBG_H
