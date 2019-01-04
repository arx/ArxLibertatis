/*
 * Copyright 2015-2018 Arx Libertatis Team (see the AUTHORS file)
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

#include "core/Benchmark.h"

#include <algorithm>
#include <limits>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>

#include "core/Application.h"
#include "core/TimeTypes.h"

#include "io/log/Logger.h"

#include "platform/Platform.h"
#include "platform/ProgramOptions.h"
#include "platform/Time.h"

#include "util/cmdline/Optional.h"

namespace benchmark {

static const char * const g_names[] = {
	NULL,
	"startup",
	"splash",
	"menu",
	"loading",
	"game",
	"cutscene",
	"cinematic",
	"shutdown"
};

static bool g_enabled = false;
static PlatformDuration g_timeLimit = 0;
static Status g_currentStatus = None;
static PlatformInstant g_startTime = 0;
static int g_startCount = 0;
static const PlatformDuration PlatformDuration_MAX = PlatformDurationUs(std::numeric_limits<s64>::max());

struct Result {
	
	PlatformDuration m_totalTime;
	u32 m_frameCount;
	PlatformDuration m_minTime;
	PlatformDuration m_maxTime;
	
	Result()
		: m_totalTime(0)
		, m_frameCount(0)
		, m_minTime(PlatformDuration_MAX)
		, m_maxTime(0)
	{ }
	
	explicit Result(PlatformDuration time)
		: m_totalTime(time)
		, m_frameCount(1)
		, m_minTime(time)
		, m_maxTime(time)
	{ }
	
	void operator+=(const Result & other) {
		m_totalTime += other.m_totalTime;
		m_frameCount += other.m_frameCount;
		m_minTime = std::min(m_minTime, other.m_minTime);
		m_maxTime = std::max(m_maxTime, other.m_maxTime);
	}
	
	bool empty() const {
		return m_totalTime == 0;
	}
	
};

static Result g_current;

static Result g_results[ARRAY_SIZE(g_names)];

static void display(Status type, const Result & result, bool summary = false, bool last = false) {
	
	if(result.empty()) {
		return;
	}
	
	const char * prefix = "";
	const char * pprefix = "";
	if(last) {
		prefix = " └─ ";
		pprefix = "    ";
	} else if(summary) {
		prefix = " ├─ ";
		pprefix = " │  ";
	}
	
	float time = toMs(result.m_totalTime);
	float tmin = toMs(result.m_minTime);
	float tmax = toMs(result.m_maxTime);
	float average = time / float(result.m_frameCount);
	float framerate = 1000.f / average;
	float minrate = 1000.f / tmin;
	float maxrate = 1000.f / tmax;
	
	switch(type) {
		case Startup:
		case Shutdown:
		case Splash:
			LogInfo << prefix << "Completed " << g_names[type] << " in "
			        << std::fixed << std::setprecision(2) << time << " ms";
			break;
		case LoadLevel:
			if(!summary) {
				LogInfo << prefix << "Loaded level in "
				        << std::fixed << std::setprecision(2) << time << " ms";
			} else {
				LogInfo << prefix << "Loaded " << result.m_frameCount << " level"
				        << (result.m_frameCount == 1 ? "" : "s") << " in "
				        << std::fixed << std::setprecision(2) << time << " ms - average: "
				        << std::fixed << std::setprecision(3) << average << " ms";
				LogInfo << pprefix << "min: "
				        << std::fixed << std::setprecision(2) << tmin << " ms, max: "
				        << std::fixed << std::setprecision(2) << tmax << " ms";
			}
			break;
		default:
			LogInfo << prefix << "Rendered " << result.m_frameCount
			        << (g_names[type] ? " " : "") << (g_names[type] ? g_names[type] : "")
			        << " frames in " << std::fixed << std::setprecision(2) << time
			        << " ms - average: " << std::fixed << std::setprecision(3) << average
			        << " ms (" << std::fixed << std::setprecision(2) << framerate << " FPS)";
			LogInfo << pprefix << "min: "
			        << std::fixed << std::setprecision(3) << tmin << " ms ("
			        << std::fixed << std::setprecision(2) << minrate << " FPS), max: "
			        << std::fixed << std::setprecision(3) << tmax << " ms ("
			        << std::fixed << std::setprecision(2) << maxrate << " FPS)";
			break;
	}
	
}

static bool isNormalFrame(Status type) {
	switch(type) {
		case Scene:
		case Cutscene:
		case Cinematic:
			return true;
		default:
			return false;
	}
}

static bool isFrame(Status type) {
	switch(type) {
		case Scene:
		case Cutscene:
		case Cinematic:
		case Menu:
			return true;
		default:
			return false;
	}
}

static void endFrame() {
	
	PlatformInstant now = platform::getTime();
	g_current += Result(now - g_startTime);
	g_startTime = now;
	
}

static const int SkipFrames = 3;

//! End the current benchmark
static void end() {
	
	if(!g_enabled || g_currentStatus == None) {
		return;
	}
	
	if(g_currentStatus != None && g_startCount > SkipFrames) {
		
		if(isFrame(g_currentStatus)) {
			// Skip the last frame - it may not be reperasentative
		} else {
			endFrame();
		}
		
		display(g_currentStatus, g_current);
		
		arx_assert(size_t(g_currentStatus) < ARRAY_SIZE(g_results));
		g_results[g_currentStatus] += g_current;
		if(isNormalFrame(g_currentStatus)) {
			g_results[0] += g_current;
		}
		
	}
	
	g_currentStatus = None;
	g_startCount = 0;
	g_current = Result();
	
}

void begin(Status status) {
	
	if(!g_enabled || status == None) {
		return;
	}
	
	if(status != g_currentStatus) {
		end();
		g_currentStatus = status;
	}
	
	if(g_startCount < SkipFrames && isFrame(status)) {
		// Skip the first frames - they may not be reperasentative
		g_startCount++;
	} else if(g_startCount <= SkipFrames) {
		g_startCount = SkipFrames + 1;
		g_startTime = platform::getTime();
	} else {
		endFrame();
	}
	
	if(g_timeLimit != PlatformDuration_MAX && isNormalFrame(g_currentStatus)
	   && g_results[0].m_totalTime + g_current.m_totalTime >= g_timeLimit) {
		mainApp->quit();
	}
	
}

void shutdown() {
	
	if(!g_enabled) {
		return;
	}
	
	end();
	
	arx_assert(size_t(None) == 0);
	
	size_t last = 0;
	if(g_results[0].empty()) {
		for(size_t i = ARRAY_SIZE(g_results) - 1; i > 0; i--) {
			if(!g_results[i].empty()) {
				last = i;
				break;
			}
		}
		if(last == 0) {
			return;
		}
	}
	
	LogInfo << "Benchmark summary:";
	
	for(size_t i = 1; i < ARRAY_SIZE(g_results); i++) {
		display(Status(i), g_results[i], true, (i == last));
	}
	
	display(None, g_results[0], true, true);
	
}

bool isEnabled() {
	return g_enabled;
}

static void enable(util::cmdline::optional<std::string> limit) {
	if(limit && !limit->empty()) {
		boost::trim(*limit);
		size_t pos = limit->find_first_not_of("0123456789.");
		float multiplier = 1.f;
		if(pos != std::string::npos) {
			std::string unit = boost::trim_copy(limit->substr(pos));
			limit->resize(pos);
			boost::trim_right(*limit);
			if(unit == "ms") {
				multiplier = 1.f;
			} else if(unit == "s" || unit == "sec" || unit == "seconds" || unit == "second") {
				multiplier = 1000.f;
			} else if(unit == "m" || unit == "min" || unit == "minutes" || unit == "minute") {
				multiplier = 60.f * 1000.f;
			} else if(unit == "h" || unit == "hours" || unit == "hour") {
				multiplier = 60.f * 60.f * 1000.f;
			} else {
				throw util::cmdline::error(util::cmdline::error::invalid_cmd_syntax,
				                           "unknown unit \"" + unit + "\"");
			}
		}
		try {
			g_timeLimit = PlatformDurationMsf(multiplier * boost::lexical_cast<float>(*limit));
		} catch(...) {
			throw util::cmdline::error(util::cmdline::error::invalid_cmd_syntax,
			                           "inavlid number \"" + *limit + "\"");
		}
	} else {
		g_timeLimit = PlatformDuration_MAX;
	}
	g_enabled = true;
}

ARX_PROGRAM_OPTION_ARG("benchmark", "", "Log loading times and framerates", &enable, "TIMELIMIT")

} // namespace benchmark
