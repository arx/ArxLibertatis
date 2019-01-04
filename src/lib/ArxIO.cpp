/*
 * Copyright 2013-2018 Arx Libertatis Team (see the AUTHORS file)
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
#include <cstdlib>

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
			case Logger::Console:  levelName = "Console"; break;
			case Logger::Warning:  levelName = "Warning"; break;
			case Logger::Error:    levelName = "Error"; break;
			case Logger::Critical: levelName = "Critical"; break;
			case Logger::None: arx_unreachable();
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

void ArxIO_unpack_alloc(const char * in, const size_t inSize, char ** out, size_t * outSize) {
	std::string buffer = blast(in, inSize);
	*outSize = buffer.size();
	*out = new char[buffer.size()];
	std::memcpy(*out, buffer.data(), buffer.size());
}

void ArxIO_unpack_free(char * buffer) { // NOLINT const would not be appropriate here
	delete[] buffer;
}
