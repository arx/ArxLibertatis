/*
 * Copyright 2014-2017 Arx Libertatis Team (see the AUTHORS file)
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

#include <atomic>
#include <cstring>
#include <map>
#include <iomanip>
#include <string.h>
#include <vector>

#include <boost/array.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/foreach.hpp>

#include "io/fs/FilePath.h"
#include "io/fs/FileStream.h"
#include "io/log/Logger.h"

#include "platform/Thread.h"
#include "platform/Time.h"
#include "platform/profiler/ProfilerDataFormat.h"

#include "util/String.h"

class Profiler {
	
public:
	
	Profiler();
	
	void flush();
	void reset();
	
	void registerThread(const std::string & threadName);
	void unregisterThread();
	
	void addProfilePoint(const char * tag, thread_id_type threadId,
	                     PlatformInstant startTime, PlatformInstant endTime);
	
private:
	
	static const u32 NB_SAMPLES = 100 * 1000;
	
	struct ProfilerSample {
		const char * tag;
		thread_id_type threadId;
		PlatformInstant startTime;
		PlatformInstant endTime;
	};

	struct ProfilerThread {
		std::string    threadName;
		thread_id_type threadId;
		PlatformInstant startTime;
		PlatformInstant endTime;
	};

	typedef std::map<thread_id_type, ProfilerThread> ThreadInfos;
	
	ThreadInfos        m_threads;
	
	boost::array<ProfilerSample, NB_SAMPLES> m_samples;
	std::atomic<int> m_writeIndex;
	volatile bool    m_canWrite;
	
	void writeProfileLog();
	
};


Profiler::Profiler() {
	reset();
}

void Profiler::reset() {
	m_writeIndex = 0;
	m_samples.fill(ProfilerSample());
	m_canWrite = true;
}

void Profiler::registerThread(const std::string & threadName) {
	thread_id_type threadId = Thread::getCurrentThreadId();
	ProfilerThread & thread = m_threads[threadId];
	thread.threadName = threadName;
	thread.threadId = threadId;
	thread.startTime = platform::getTime();
	thread.endTime = thread.startTime;
}
	
void Profiler::unregisterThread() {
	thread_id_type threadId = Thread::getCurrentThreadId();
	ProfilerThread & thread = m_threads[threadId];
	thread.endTime = platform::getTime();
}

void Profiler::addProfilePoint(const char * tag, thread_id_type threadId,
                               PlatformInstant startTime, PlatformInstant endTime) {
	
	while(!m_canWrite);
	
	u32 pos = m_writeIndex.fetch_add(1);
	
	ProfilerSample & sample = m_samples[pos % NB_SAMPLES];
	sample.tag = tag;
	sample.threadId = threadId;
	sample.startTime = startTime;
	sample.endTime = endTime;
}

void Profiler::flush() {
	
	m_canWrite = false;
	writeProfileLog();
	reset();
}


template <typename T>
void writeStruct(std::ofstream & out, T & data, size_t & pos) {
	out.write(reinterpret_cast<const char *>(&data), sizeof(T));
	pos += sizeof(T);
}

static void writeChunk(std::ofstream & out, ArxProfilerChunkType type, size_t dataSize, size_t & pos) {
	SavedProfilerChunkHeader chunk;
	chunk.type = type;
	chunk.size = dataSize;
	std::memset(chunk.padding, 0, sizeof(chunk.padding));
	writeStruct(out, chunk, pos);
	LogDebug("Writing chunk at offset " << pos << " type " << chunk.type << " size " << chunk.size);
}

class ProfilerStringTable : public boost::noncopyable {
public:
	u32 add(std::string value) {
		
		boost::container::flat_map<std::string, u32>::iterator si = m_map.find(value);
		u32 stringIndex;
		
		if(si == m_map.end()) {
			m_list.push_back(value);
			stringIndex = m_list.size() - 1;
			m_map[value] = stringIndex;
		} else {
			stringIndex = si->second;
		}
		
		return stringIndex;
	}
	
	int entries() {
		return m_list.size();
	}
	
	std::string data() {
		std::string result = boost::algorithm::join(m_list, std::string("\0", 1));
		return result;
	}
	
private:
	boost::container::flat_map<std::string, u32> m_map;
	std::vector<std::string> m_list;
};

void Profiler::writeProfileLog() {
	
	std::string filename = util::getDateTimeString() + ".arxprof";
	LogInfo << "Writing profiler log to: " << filename;
	
	fs::ofstream out(fs::path(filename), std::ios::binary | std::ios::out);
	size_t pos = 0;
	
	int fileVersion = 1;
	
	LogDebug("Writing Header");
	
	SavedProfilerHeader header;
	std::strncpy(header.magic, profilerMagic, 8);
	header.version = fileVersion;
	std::memset(header.padding, 0, sizeof(header.padding));
	writeStruct(out, header, pos);
	
	LogDebug("Building string table");
	ProfilerStringTable stringTable;
	std::vector<SavedProfilerThread> threadsData;
	std::vector<SavedProfilerSample> samplesData;
	
	for(ThreadInfos::const_iterator it = m_threads.begin(); it != m_threads.end(); ++it) {
		const ProfilerThread & thread = it->second;
		
		u32 stringIndex = stringTable.add(thread.threadName);
		
		SavedProfilerThread saved;
		saved.stringIndex = stringIndex;
		saved.threadId = thread.threadId;
		saved.startTime = toUs(thread.startTime);
		saved.endTime = toUs(thread.endTime);
		threadsData.push_back(saved);
	}
	
	// Profile points
	u32 index = 0;
	u32 numItems = m_writeIndex;
	
	if(numItems >= NB_SAMPLES) {
		index = numItems;
		numItems = NB_SAMPLES;
	}
	
	for(u32 i = 0; i < numItems; ++i, ++index) {
		ProfilerSample & sample = m_samples[index % NB_SAMPLES];
		
		u32 stringIndex = stringTable.add(sample.tag);
		
		SavedProfilerSample saved;
		saved.stringIndex = stringIndex;
		saved.threadId = sample.threadId;
		saved.startTime = toUs(sample.startTime);
		saved.endTime = toUs(sample.endTime);
		samplesData.push_back(saved);
	}
	
	LogInfo << "Writing data: "
	" Strings " << stringTable.entries() <<
	", Threads " << threadsData.size() <<
	", Points "  << samplesData.size();
	
	{
		std::string stringsData = stringTable.data();
		size_t dataSize = stringsData.size() + 1; // termination
		writeChunk(out, ArxProfilerChunkType_Strings, dataSize, pos);
		out.write(stringsData.c_str(), dataSize);
		pos += dataSize;
	}
	
	{
		size_t dataSize = threadsData.size() * sizeof(SavedProfilerThread);
		writeChunk(out, ArxProfilerChunkType_Threads, dataSize, pos);
		out.write((const char *)threadsData.data(), dataSize);
		pos += dataSize;
	}
	
	{
		size_t dataSize = samplesData.size() * sizeof(SavedProfilerSample);
		writeChunk(out, ArxProfilerChunkType_Samples, dataSize, pos);
		out.write((const char *)samplesData.data(), dataSize);
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


profiler::Scope::Scope(const char * tag)
	: m_tag(tag)
	, m_startTime(platform::getTime())
{
	arx_assert(tag != 0 && tag[0] != '\0');
}

profiler::Scope::~Scope() {
	g_profiler.addProfilePoint(m_tag, Thread::getCurrentThreadId(), m_startTime, platform::getTime());
}

#else

void profiler::initialize() {}
void profiler::flush() {}

void profiler::registerThread(const std::string & threadName) {
	ARX_UNUSED(threadName);
}

void profiler::unregisterThread() {}

#endif // BUILD_PROFILER_INSTRUMENT
