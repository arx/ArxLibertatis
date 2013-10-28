/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
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
#endif

bool CrashHandler::initialize() {
	
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
	}
	
	gCrashHandlerInitCount++;
	return true;
	
#else
	return false;
#endif
}

void CrashHandler::shutdown() {
#if ARX_HAVE_CRASHHANDLER
	gCrashHandlerInitCount--;
	if(gCrashHandlerInitCount == 0) {
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

bool CrashHandler::setNamedVariable(const std::string & name, const std::string & value) {
#if ARX_HAVE_CRASHHANDLER
	if(!isInitialized()) {
		return false;
	}
	return gCrashHandlerImpl->setNamedVariable(name, value);
#else
	ARX_UNUSED(name), ARX_UNUSED(value);
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
