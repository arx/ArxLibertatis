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

#ifndef ARX_TOOLS_CRASHREPORTER_ERRORREPORT_H
#define ARX_TOOLS_CRASHREPORTER_ERRORREPORT_H

#include <vector>

#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <boost/scope_exit.hpp>

#include <QString>
#include <QList>
#include <QDateTime>
#include <QStringList>

#include "platform/crashhandler/CrashInfo.h"

class ErrorReport {
	
public:
	
	class IProgressNotifier {
		
	public:
		
		virtual void taskStarted(const QString & taskDescription, int numSteps) = 0;
		virtual void taskStepStarted(const QString & taskStepDescription) = 0;
		virtual void taskStepEnded() = 0;
		virtual void setError(const QString & strError) = 0;
		virtual void setDetailedError(const QString & strDetailedError) = 0;
		
	};
	
	struct File {
		
		QString path;
		bool attachToReport;
		
		File(const QString & _path, bool _attach) : path(_path), attachToReport(_attach) { }
		
	};
	
	typedef std::vector<File> FileList;
	
public:
	
	explicit ErrorReport(const QString & sharedMemoryName);
	
	bool GenerateReport(IProgressNotifier * progressNotifier);
	bool SendReport(IProgressNotifier * progressNotifier);
	
	FileList & GetAttachedFiles();
	
	const QString & GetErrorDescription() const;
	const QString & GetIssueLink() const;
	
	const QList<QString> & getFailedFiles() const { return m_failedFiles; }
	
	void SetLoginInfo(const QString & username, const QString & password);
	void SetReproSteps(const QString & reproSteps);
	
private:
	
	bool Initialize();
	
	void getCrashInfo();
	
	void AddFile(const QString & fileName);
	
	void ReleaseApplicationLock();
	
private:
	
	FileList m_AttachedFiles;
	
	int m_ProcessArchitecture;

	QString m_SharedMemoryName;
	boost::interprocess::shared_memory_object m_SharedMemory;
	
	boost::interprocess::mapped_region m_MemoryMappedRegion;
	CrashInfo * m_pCrashInfo;
	
	QString m_ReportUniqueID;
	QString m_ReportTitle;
	QString m_ReportDescription;
	QString m_ReproSteps;
	QString m_IssueLink;

	QString m_Username;
	QString m_Password;

	QString m_DetailedError;
	
	QList<QString> m_failedFiles;
	
};

#endif // ARX_TOOLS_CRASHREPORTER_ERRORREPORT_H
