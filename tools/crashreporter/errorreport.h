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

#ifndef ARX_TOOLS_CRASHREPORTER_ERRORREPORT_H
#define ARX_TOOLS_CRASHREPORTER_ERRORREPORT_H

#include <vector>

#include "Configure.h"

#ifdef HAVE_WINAPI
// Win32
#include <winsock2.h>
#endif // HAVE_WINAPI

// BOOST
#define BOOST_DATE_TIME_NO_LIB
#include <boost/version.hpp>
#if BOOST_VERSION < 104500
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-fpermissive"
#endif
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#if BOOST_VERSION < 104500
#pragma GCC diagnostic pop
#endif
#include <boost/scope_exit.hpp>

// Qt
#include <QString>
#include <QList>
#include <QDateTime>
#include <QStringList>

#include "platform/crashhandler/CrashInfo.h"
#include "io/fs/FilePath.h"

class ErrorReport
{
public:
	class IProgressNotifier
	{
	public:
		virtual void taskStarted(const std::string& taskDescription, int numSteps) = 0;
		virtual void taskStepStarted(const std::string& taskStepDescription) = 0;
		virtual void taskStepEnded() = 0;
		virtual void setError(const std::string& strError) = 0;
	};

	struct File
	{
		fs::path	path;
		bool		attachToReport;

		File(const fs::path _path, bool _attach)
		{
			path = _path;
			attachToReport = _attach;
		}
	};

	typedef std::vector<File> FileList;

public:
	
	explicit ErrorReport(const std::string & sharedMemoryName);
	
	bool GenerateReport(IProgressNotifier* progressNotifier);
	bool SendReport(IProgressNotifier* progressNotifier);

	FileList& GetAttachedFiles();

	const std::string& GetErrorDescription() const;

private:
	bool Initialize();

	bool WriteReport(const fs::path& fileName);
	bool GenerateArchive();

	bool GetScreenshot(const fs::path& fileName, int quality = -1, bool bGrayscale = false);
	bool GetCrashDump(const fs::path& fileName);
	bool GetMachineInfo(const fs::path& fileName);
	bool GetMiscCrashInfo();

	void AddFile(const fs::path& fileName);
	
	void ReleaseApplicationLock();

private:
	fs::path m_ReportFolder;

	FileList m_AttachedFiles;

	QDateTime m_CrashDateTime;
	double m_RunningTimeSec;
	
	QString m_OSName;
	bool m_OSIs64Bit;

	QString m_ProcessName;
	fs::path m_ProcessPath;
	quint64 m_ProcessMemoryUsage;
	bool m_ProcessIs64Bit;

	std::string m_SharedMemoryName;
	boost::interprocess::shared_memory_object m_SharedMemory;
	
	boost::interprocess::mapped_region m_MemoryMappedRegion;
	CrashInfo* m_pCrashInfo;

	std::string m_ReportDetailedDescription;
};

#endif // ARX_TOOLS_CRASHREPORTER_ERRORREPORT_H
