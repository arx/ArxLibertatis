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

#ifndef ARX_PLATFORM_CRASHHANDLER_CRASHHANDLERPOSIX_H
#define ARX_PLATFORM_CRASHHANDLER_CRASHHANDLERPOSIX_H

#include <string>

#include "platform/crashhandler/CrashHandlerImpl.h"

class CrashHandlerPOSIX : public CrashHandlerImpl {
	
public:
	
	CrashHandlerPOSIX();
	virtual ~CrashHandlerPOSIX();
	
	bool initialize();
	
	bool registerThreadCrashHandlers();
	void unregisterThreadCrashHandlers();
	
	void registerCrashCallback(CrashHandler::CrashCallback crashCallback);
	void unregisterCrashCallback(CrashHandler::CrashCallback crashCallback);
	
	void handleCrash(int signal, void * info, void * context);
	
	static CrashHandlerPOSIX & getInstance();
	
private:
	
	std::string m_arg;
	
	bool registerCrashHandlers();
	void unregisterCrashHandlers();
	
	void processCrashInfo();
	void processCrashSignal();
	void processCrashTrace();
	void processCrashDump();
	
	// Crash handlers to restore
	struct PlatformCrashHandlers * m_pPreviousCrashHandlers;
	static CrashHandlerPOSIX * m_sInstance;
	
};

#endif // ARX_PLATFORM_CRASHHANDLER_CRASHHANDLERPOSIX_H
