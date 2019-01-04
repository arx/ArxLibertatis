/*
 * Copyright 2014-2018 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_PLATFORM_PROFILER_PROFILER_H
#define ARX_PLATFORM_PROFILER_PROFILER_H

#include <string>

#include "core/TimeTypes.h"
#include "platform/Platform.h"

namespace profiler {

//! Initialize the Profiler
void initialize();

//! Write the collected profile data to disk
void flush();

void registerThread(const std::string & threadName);
void unregisterThread();

#if BUILD_PROFILER_INSTRUMENT

class Scope {
	
	const char * m_tag;
	PlatformInstant m_startTime;
	
public:
	
	explicit Scope(const char * tag);
	~Scope();
	
};

#endif // BUILD_PROFILER_INSTRUMENT

} // namespace profiler

#if BUILD_PROFILER_INSTRUMENT
	#define ARX_PROFILE(tag)           profiler::Scope profileScopeTag##__LINE__(#tag)
	#define ARX_PROFILE_FUNC()         profiler::Scope profileScopeFunc##__LINE__(__FUNCTION__)
#else
	#define ARX_PROFILE(tag)           ARX_DISCARD(tag)
	#define ARX_PROFILE_FUNC()         ARX_DISCARD()
#endif // BUILD_PROFILER_INSTRUMENT

#endif // ARX_PLATFORM_PROFILER_PROFILER_H
