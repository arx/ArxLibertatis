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

#include "platform/CrashHandler.h"



#include "io/fs/Filesystem.h"
#include "io/resource/ResourcePath.h"
#include "io/log/Logger.h"
#include "platform/Platform.h"
#include "Configure.h"

#if defined(HAVE_CRASHHANDLER_POSIX)
#include "platform/crashhandler/CrashHandlerPOSIX.h"
#elif defined(HAVE_CRASHHANDLER_WINDOWS)
#include "platform/crashhandler/CrashHandlerWindows.h"
#else
#include "platform/crashhandler/CrashHandlerImpl.h"
#endif

static CrashHandlerImpl * gCrashHandlerImpl = 0;
static int gInitCount = 0;

bool CrashHandler::initialize() {
	
	if(!gCrashHandlerImpl) {
		
#if defined(HAVE_CRASHHANDLER_POSIX)
		
		gCrashHandlerImpl = new CrashHandlerPOSIX();
		
#elif defined(HAVE_CRASHHANDLER_WINDOWS)

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
	
	gInitCount++;
	return true;
}

void CrashHandler::shutdown() {
	gInitCount--;
	if(gInitCount == 0) {
		gCrashHandlerImpl->shutdown();
		delete gCrashHandlerImpl;
		gCrashHandlerImpl = 0;
	}
}

bool CrashHandler::isInitialized() {
	return gCrashHandlerImpl != 0;
}

bool CrashHandler::addAttachedFile(const fs::path& file) {
	if(!isInitialized())
		return false;

	return gCrashHandlerImpl->addAttachedFile(file);
}

bool CrashHandler::setNamedVariable(const std::string& name, const std::string& value) {
	if(!isInitialized())
		return false;

	return gCrashHandlerImpl->setNamedVariable(name, value);
}

bool CrashHandler::setReportLocation(const fs::path& location) {
	if(!isInitialized())
		return false;

	return gCrashHandlerImpl->setReportLocation(location);
}

bool CrashHandler::deleteOldReports(size_t nbReportsToKeep) {
	if(!isInitialized())
		return false;

	return gCrashHandlerImpl->deleteOldReports(nbReportsToKeep);
}

bool CrashHandler::registerThreadCrashHandlers() {
	if(!isInitialized())
		return false;

	return gCrashHandlerImpl->registerThreadCrashHandlers();
}

void CrashHandler::unregisterThreadCrashHandlers() {
	if(!isInitialized())
		return;

	gCrashHandlerImpl->unregisterThreadCrashHandlers();
}

void CrashHandler::registerCrashCallback(CrashCallback crashCallback) {
	if(!isInitialized())
		return;

	gCrashHandlerImpl->registerCrashCallback(crashCallback);
}

void CrashHandler::unregisterCrashCallback(CrashCallback crashCallback) {
	if(!isInitialized())
		return;

	gCrashHandlerImpl->unregisterCrashCallback(crashCallback);
}
