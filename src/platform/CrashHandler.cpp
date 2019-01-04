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

#include "platform/CrashHandler.h"

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <sstream>

#include <boost/algorithm/string/predicate.hpp>

#include "io/fs/Filesystem.h"
#include "io/resource/ResourcePath.h"
#include "io/log/Logger.h"
#include "platform/Platform.h"
#include "Configure.h"

#if ARX_HAVE_CRASHHANDLER_POSIX
#include "platform/crashhandler/CrashHandlerPOSIX.h"
#define ARX_HAVE_CRASHHANDLER 1
#elif ARX_HAVE_CRASHHANDLER_WINDOWS
#include "platform/crashhandler/CrashHandlerWindows.h"
#define ARX_HAVE_CRASHHANDLER 1
#else
#define ARX_HAVE_CRASHHANDLER 0
#endif

#if ARX_HAVE_CRASHHANDLER

static CrashHandlerImpl * gCrashHandlerImpl = 0;
static int gCrashHandlerInitCount = 0;

#ifdef ARX_DEBUG

typedef void(*AssertHandler)(const char * expr, const char * file, unsigned int line,
                             const char * msg);
extern AssertHandler g_assertHandler;

static void crashAssertHandler(const char * expr, const char * file, unsigned int line,
                               const char * msg) {
	
	const char * filename = file + std::strlen(file);
	while(filename != file && filename[-1] != '/' && filename[-1] != '\\') {
		filename--;
	}
	
	std::ostringstream oss;
	oss << "Assertion Failed at " << filename << ":" << line << ": " << expr << "\n";
	if(msg) {
		oss << "Message: " << msg << "\n";
	}
	oss << "\n";
	
	gCrashHandlerImpl->addText(oss.str().c_str());
	
}

#endif // ARX_DEBUG

#endif // ARX_HAVE_CRASHHANDLER

bool CrashHandler::initialize(int argc, char ** argv) {
	
	#if ARX_HAVE_CRASHHANDLER
	
	if(!gCrashHandlerImpl) {
		
		#if ARX_HAVE_CRASHHANDLER_POSIX
		
		gCrashHandlerImpl = new CrashHandlerPOSIX();
		
		#elif ARX_HAVE_CRASHHANDLER_WINDOWS
		
		if(IsDebuggerPresent()) {
			LogInfo << "Debugger attached, disabling crash handler.";
			return false;
		}
		
		gCrashHandlerImpl = new CrashHandlerWindows();
		
		#endif
		
		const char * arg = "--crashinfo=";
		if(argc >= 2 && boost::starts_with(argv[1], arg)) {
			if(gCrashHandlerImpl) {
				gCrashHandlerImpl->processCrash(argv[1] + std::strlen(arg));
			}
			std::exit(0);
		}
		
		if(gCrashHandlerImpl) {
			bool initialized = gCrashHandlerImpl->initialize();
			if(!initialized) {
				delete gCrashHandlerImpl;
				gCrashHandlerImpl = 0;
				return false;
			}
		} else {
			return false;
		}
		
		#ifdef ARX_DEBUG
		g_assertHandler = crashAssertHandler;
		#endif
	}
	
	gCrashHandlerInitCount++;
	return true;
	
	#else
	ARX_UNUSED(argc), ARX_UNUSED(argv);
	return false;
	#endif
}

void CrashHandler::shutdown() {
#if ARX_HAVE_CRASHHANDLER
	gCrashHandlerInitCount--;
	if(gCrashHandlerInitCount == 0) {
		#ifdef ARX_DEBUG
		g_assertHandler = NULL;
		#endif
		gCrashHandlerImpl->shutdown();
		delete gCrashHandlerImpl;
		gCrashHandlerImpl = 0;
	}
#endif
}

bool CrashHandler::isInitialized() {
#if ARX_HAVE_CRASHHANDLER
	return gCrashHandlerImpl != 0;
#else
	return false;
#endif
}

bool CrashHandler::addAttachedFile(const fs::path & file) {
#if ARX_HAVE_CRASHHANDLER
	if(!isInitialized()) {
		return false;
	}
	return gCrashHandlerImpl->addAttachedFile(file);
#else
	ARX_UNUSED(file);
	return false;
#endif
}

bool CrashHandler::setVariable(const std::string & name, const std::string & value) {
#if ARX_HAVE_CRASHHANDLER
	if(!isInitialized()) {
		return false;
	}
	return gCrashHandlerImpl->setVariable(name, value);
#else
	ARX_UNUSED(name), ARX_UNUSED(value);
	return false;
#endif
}

bool CrashHandler::setWindow(u64 window) {
#if ARX_HAVE_CRASHHANDLER
	if(!isInitialized()) {
		return false;
	}
	gCrashHandlerImpl->setWindow(window);
	return true;
#else
	ARX_UNUSED(window);
	return false;
#endif
}

bool CrashHandler::setReportLocation(const fs::path & location) {
#if ARX_HAVE_CRASHHANDLER
	if(!isInitialized()) {
		return false;
	}
	return gCrashHandlerImpl->setReportLocation(location);
#else
	ARX_UNUSED(location);
	return false;
#endif
}

bool CrashHandler::deleteOldReports(size_t nbReportsToKeep) {
#if ARX_HAVE_CRASHHANDLER
	if(!isInitialized()) {
		return false;
	}
	return gCrashHandlerImpl->deleteOldReports(nbReportsToKeep);
#else
	ARX_UNUSED(nbReportsToKeep);
	return false;
#endif
}

bool CrashHandler::registerThreadCrashHandlers() {
#if ARX_HAVE_CRASHHANDLER
	if(!isInitialized()) {
		return false;
	}
	return gCrashHandlerImpl->registerThreadCrashHandlers();
#else
	return false;
#endif
}

void CrashHandler::unregisterThreadCrashHandlers() {
#if ARX_HAVE_CRASHHANDLER
	if(!isInitialized()) {
		return;
	}
	gCrashHandlerImpl->unregisterThreadCrashHandlers();
#endif
}

void CrashHandler::registerCrashCallback(CrashCallback crashCallback) {
#if ARX_HAVE_CRASHHANDLER
	if(!isInitialized()) {
		return;
	}
	gCrashHandlerImpl->registerCrashCallback(crashCallback);
#else
	ARX_UNUSED(crashCallback);
#endif
}

void CrashHandler::unregisterCrashCallback(CrashCallback crashCallback) {
#if ARX_HAVE_CRASHHANDLER
	if(!isInitialized()) {
		return;
	}
	gCrashHandlerImpl->unregisterCrashCallback(crashCallback);
#else
	ARX_UNUSED(crashCallback);
#endif
}
