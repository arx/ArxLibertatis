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

#ifndef ARX_CRASHREPORTER_ERRORREPORT_H
#define ARX_CRASHREPORTER_ERRORREPORT_H

// Win32
#include <winsock2.h>

// BOOST
#define BOOST_DATE_TIME_NO_LIB
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>

// Qt
#include <QString>
#include <QList>
#include <QDateTime>
#include <QStringList>

#include "platform/CrashInfo.h"

class ErrorReport
{
public:
	ErrorReport(const std::string& crashesFolder, const std::string& sharedMemoryName);

	bool Initialize();

	bool WriteReport(const std::string& fileName);
	bool GenerateArchive();
	bool Send();

	bool GetScreenshot(const std::string& fileName, int quality = -1, bool bGrayscale = false);
	bool GetCrashDump(const std::string& fileName);
	bool GetMachineInfo(const std::string& fileName);
	bool GetMiscCrashInfo();
	
	void ReleaseApplicationLock();

	const QStringList& GetAttachedFiles() const;

private:
	QString	GetFilePath(const std::string& fileName) const;

private:
	QString m_CrashesFolder;
	QString m_CurrentReportFolder;

	QStringList m_AttachedFiles;

	QDateTime m_CrashDateTime;
	double m_RunningTimeSec;
	
	QString m_OSName;
	bool m_OSIs64Bit;

	QString m_ProcessName;
	QString m_ProcessPath;
	quint64 m_ProcessMemoryUsage;
	bool m_ProcessIs64Bit;

	std::string m_SharedMemoryName;
	boost::interprocess::shared_memory_object m_SharedMemory;
	
	boost::interprocess::mapped_region m_MemoryMappedRegion;
	CrashInfo* m_pCrashInfo;
};

#endif // ARX_CRASHREPORTER_ERRORREPORT_H
