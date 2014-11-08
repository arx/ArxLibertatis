/*
 * Copyright 2013 Arx Libertatis Team (see the AUTHORS file)
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

#include "lib/ArxIO.h"

#include <deque>
#include <string>
#include <iostream>
#include <fstream>
#include <cstring>

#include "io/Blast.h"
#include "io/log/Logger.h"
#include "io/log/ConsoleLogger.h"

namespace {

class MemoryLogger : public logger::Backend {

public:
	void log(const logger::Source & file, int line, Logger::LogLevel level, const std::string & str) {

		ARX_UNUSED(file);
		ARX_UNUSED(line);

		if(level == Logger::Error) {
			m_lastError = str;
		}

		std::string levelName;
		switch(level) {
		case Logger::Debug:    levelName = "Debug"; break;
		case Logger::Info:     levelName = "Info"; break;
		case Logger::Warning:  levelName = "Warning"; break;
		case Logger::Error:    levelName = "Error"; break;
		case Logger::Critical: levelName = "Critical"; break;
		case Logger::None: ARX_DEAD_CODE(); return;
		}

		m_Lines.push_back(levelName + ": " + str);
	}

	void flush() {}

	std::string m_lastError;
	std::deque<std::string> m_Lines;
};

} // anonymous namespace

static MemoryLogger memLogger;

void ArxIO_init() {
	Logger::add(&memLogger);
	Logger::set("*", Logger::Debug);

	LogInfo << "Arx Io library initialized";
}

void ArxIO_getError(char * outMessage, int size) {
	if(!memLogger.m_lastError.empty()) {
		memLogger.m_lastError.copy(outMessage, size);
		memLogger.m_lastError.clear();
	}
}

int ArxIO_getLogLine(char * outMessage, int size) {
	if(memLogger.m_Lines.empty()) {
		return 0;
	}

	memLogger.m_Lines.front().copy(outMessage, size);
	memLogger.m_Lines.pop_front();

	return memLogger.m_Lines.size();
}

static size_t loadedFtlBufferSize = 0;
static char * loadedFtlBuffer = NULL;

int ArxIO_ftlLoad(char * filePath) {

	ArxIO_ftlRelease();

	std::ifstream is(filePath, std::ifstream::binary);

	if(!is) {
		LogError << "Opening file failed: " << filePath;
		return -1;
	}

	// get length of file:
	is.seekg(0, is.end);
	size_t compressedSize = is.tellg();
	is.seekg(0, is.beg);

	char * compressedData = new char[compressedSize];

	LogInfo << "Reading " << compressedSize << " uncompressed bytes";
	is.read(compressedData, compressedSize);

	if(is) {
		LogInfo << "File read successfully";
	} else {
		LogError << "Full read failed " << (compressedSize - is.gcount()) << " bytes left";
		delete[] compressedData;
		return -1;
	}

	is.close();

	size_t allocsize;
	char * dat = blastMemAlloc(compressedData, compressedSize, allocsize);
	if(!dat) {
		LogError << "Decompressing failed " << filePath;
		return -1;
	}

	LogInfo << "Uncompressed size: " << allocsize;

	loadedFtlBufferSize = allocsize;
	loadedFtlBuffer = dat;

	return 0;
}

int ArxIO_ftlRelease() {
	if(!loadedFtlBuffer) {
		return -1;
	}

	LogInfo << "Releasing loaded ftl data";

	loadedFtlBufferSize = 0;
	free(loadedFtlBuffer);
	loadedFtlBuffer = NULL;

	return 0;
}

int ArxIO_ftlGetRawDataSize() {
	return loadedFtlBufferSize;
}

int ArxIO_ftlGetRawData(char * outData, int size) {
	if(!loadedFtlBuffer || loadedFtlBufferSize == 0) {
		LogError << "No buffer loaded";
		return -1;
	}

	if(size < int(loadedFtlBufferSize)) {
		LogError << "Supplied buffer too small for data";
		return -2;
	}

	memcpy(outData, loadedFtlBuffer, size);

	return 0;
}


void ArxIO_unpack_alloc(const char * in, const size_t inSize, char ** out, size_t * outSize) {
	*out = blastMemAlloc(in, inSize, *outSize);
}

void ArxIO_unpack_free(char * buffer) {
	free(buffer);
}
