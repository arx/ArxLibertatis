/*
 * Copyright 2014 Arx Libertatis Team (see the AUTHORS file)
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

#include "platform/profiler/Profiler.h"

#if BUILD_PROFILER_INSTRUMENT

#include <map>
#include <iomanip>
#include <string.h>
#include <vector>

#include <boost/atomic.hpp>

#include "io/fs/FilePath.h"
#include "io/fs/FileStream.h"
#include "io/log/Logger.h"
#include "util/String.h"

#include "platform/profiler/ProfilerDataFormat.h"

class Profiler {
	
public:
	Profiler();
	
	void flush();
	void reset();
	
	void registerThread(const std::string& threadName);
	void unregisterThread();
	
	void addProfilePoint(const char* tag, thread_id_type threadId, u64 startTime, u64 endTime);
	
private:
	static const u32 NB_POINTS = 100 * 1000;
	
	struct ProfilePoint {
		const char*    tag;
		thread_id_type threadId;
		u64            startTime;
		u64            endTime;
	};

	struct ThreadInfo {
		std::string    threadName;
		thread_id_type threadId;
		u64            startTime;
		u64            endTime;
	};

	typedef std::map<thread_id_type, ThreadInfo> ThreadInfos;
	
	ThreadInfos        m_threads;
	ProfilePoint       m_points[NB_POINTS];
	boost::atomic<int> m_writeIndex;
	bool               m_canWrite;
	
	void writeProfileLog();
};


Profiler::Profiler() {
	m_writeIndex = 0;
	m_canWrite = true;
	memset(m_points, 0, sizeof(m_points));
}

void Profiler::reset() {
	m_writeIndex = 0;
	memset(m_points, 0, sizeof(m_points));
}

void Profiler::registerThread(const std::string& threadName) {
	thread_id_type threadId = Thread::getCurrentThreadId();
	ThreadInfo& threadInfo = m_threads[threadId];
	threadInfo.threadName = threadName;
	threadInfo.threadId = threadId;
	threadInfo.startTime = platform::getTimeUs();
	threadInfo.endTime = threadInfo.startTime;
}
	
void Profiler::unregisterThread() {
	thread_id_type threadId = Thread::getCurrentThreadId();
	ThreadInfo& threadInfo = m_threads[threadId];
	threadInfo.endTime = platform::getTimeUs();
}

void Profiler::addProfilePoint(const char* tag, thread_id_type threadId, u64 startTime, u64 endTime) {
	
	while(!m_canWrite);
	
	u32 pos = m_writeIndex.fetch_add(1);
	
	ProfilePoint& point = m_points[pos % NB_POINTS];
	point.tag = tag;
	point.threadId = threadId;
	point.startTime = startTime;
	point.endTime = endTime;
}

void Profiler::flush() {
	
	m_canWrite = false;
	writeProfileLog();
	reset();
	m_canWrite = true;
}

void Profiler::writeProfileLog() {
	
	std::string filename = util::getDateTimeString() + ".perf";
	fs::ofstream out(fs::path(filename), std::ios::binary | std::ios::out);
	
	// Threads info
	SavedThreadInfoHeader threadsHeader;
	threadsHeader.size = m_threads.size();
	out.write((const char*)&threadsHeader, sizeof(SavedThreadInfoHeader));
	
	for(ThreadInfos::const_iterator it = m_threads.begin(); it != m_threads.end(); ++it) {
		const ThreadInfo& threadInfo = it->second;
		
		SavedThreadInfo saved;
		util::storeString(saved.threadName, threadInfo.threadName);
		saved.threadId = threadInfo.threadId;
		saved.startTime = threadInfo.startTime;
		saved.endTime = threadInfo.endTime;
		out.write((const char*)&saved, sizeof(SavedThreadInfo));
	}
	
	// Profile points
	u32 index = 0;
	u32 numItems = m_writeIndex;
	
	if(numItems >= NB_POINTS) {
		index = numItems;
		numItems = NB_POINTS;
	}
	
	// Threads info
	SavedProfilePointHeader pointsHeader;
	pointsHeader.size = numItems;
	out.write((const char*)&pointsHeader, sizeof(SavedProfilePointHeader));
	
	for(u32 i = 0; i < numItems; ++i, ++index) {
		ProfilePoint& point = m_points[index % NB_POINTS];
		
		SavedProfilePoint saved;
		util::storeString(saved.tag, point.tag);
		saved.threadId = point.threadId;
		saved.startTime = point.startTime;
		saved.endTime = point.endTime;
		
		out.write((const char*)&saved, sizeof(SavedProfilePoint));
	}
	
	out.close();
}



static Profiler g_profiler;

void profiler::initialize() {
	LogInfo << "Profiler enabled";
	g_profiler.registerThread("main");
}

void profiler::flush() {
	g_profiler.flush();
}

void profiler::registerThread(const std::string & threadName) {
	g_profiler.registerThread(threadName);
}

void profiler::unregisterThread() {
	g_profiler.unregisterThread();
}

void profiler::addProfilePoint(const char * tag, thread_id_type threadId, u64 startTime, u64 endTime) {
	g_profiler.addProfilePoint(tag, threadId, startTime, endTime);
}

#else

void profiler::initialize() {
}

void profiler::flush() {
}

void profiler::registerThread(const std::string & threadName) {
	ARX_UNUSED(threadName);
}

void profiler::unregisterThread() {
}

void profiler::addProfilePoint(const char * tag, thread_id_type threadId, u64 startTime, u64 endTime) {
	ARX_UNUSED(tag);
	ARX_UNUSED(threadId);
	ARX_UNUSED(startTime);
	ARX_UNUSED(endTime);
}

#endif // BUILD_PROFILER_INSTRUMENT
