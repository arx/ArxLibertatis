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

#include "io/log/LogBackend.h"

#include "platform/Platform.h"

void logger::Backend::format(std::ostream & os, const logger::Source & file, int line, Logger::LogLevel level, const std::string & str) {
	
	switch(level) {
		case Logger::Debug:   os << "[D]"; break;
		case Logger::Info:    os << "[I]"; break;
		case Logger::Warning: os << "[W]"; break;
		case Logger::Error:   os << "[E]"; break;
		case Logger::None: ARX_DEAD_CODE();
	}
	
	os << ' ' << file.name << ':' << line << "  " << str << std::endl;
}
