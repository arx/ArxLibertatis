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

#include "CrashHandlerPOSIX.h"

#include <float.h>
#include <signal.h>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>

typedef void (*signal_handler)(int);

struct PlatformCrashHandlers {
	signal_handler m_SIGSEGVHandler;    // Illegal storage access handler.
	signal_handler m_SIGILLHandler;     // SIGINT handler.
	signal_handler m_SIGFPEHandler;     // FPE handler.
	signal_handler m_SIGABRTHandler;    // SIGABRT handler.
};

void SignalHandler(int signalCode);

CrashHandlerPOSIX* CrashHandlerPOSIX::m_sInstance = 0;

CrashHandlerPOSIX::CrashHandlerPOSIX() {
	m_sInstance = this;
}

CrashHandlerPOSIX::~CrashHandlerPOSIX() {
	m_sInstance = 0;
}

CrashHandlerPOSIX& CrashHandlerPOSIX::getInstance() {
	arx_assert(m_sInstance != 0);
	return *m_sInstance;
}

bool CrashHandlerPOSIX::registerCrashHandlers() {
	
	arx_assert(m_pPreviousCrashHandlers == 0);
	m_pPreviousCrashHandlers = new PlatformCrashHandlers;
	
	// Catch 'bad' signals so we can print some debug output.
	
#ifdef SIGSEGV
	m_pPreviousCrashHandlers->m_SIGSEGVHandler = signal(SIGSEGV, SignalHandler);
#endif
	
#ifdef SIGILL
	m_pPreviousCrashHandlers->m_SIGILLHandler = signal(SIGILL, SignalHandler);
#endif
	
#ifdef SIGFPE
	m_pPreviousCrashHandlers->m_SIGFPEHandler = signal(SIGFPE, SIGFPEHandler);
#endif
	
#ifdef SIGABRT
	m_pPreviousCrashHandlers->m_SIGABRTHandler = signal(SIGABRT, SignalHandler);
#endif
	
	// We must also register the main thread crash handlers.
	return registerThreadCrashHandlers();
}

void CrashHandlerPOSIX::unregisterCrashHandlers() {
	
	unregisterThreadCrashHandlers();
	
#ifdef SIGSEGV
	signal(SIGSEGV, m_pPreviousCrashHandlers->m_SIGSEGVHandler);
#endif
	
#ifdef SIGILL
	signal(SIGILL, m_pPreviousCrashHandlers->m_SIGILLHandler);
#endif
	
#ifdef SIGFPE
	signal(SIGFPE, m_pPreviousCrashHandlers->m_SIGFPEHandler);
#endif
	
#ifdef SIGABRT
	signal(SIGABRT, m_pPreviousCrashHandlers->m_SIGABRTHandler);
#endif
	
	delete m_pPreviousCrashHandlers;
	m_pPreviousCrashHandlers = 0;
}


bool CrashHandlerPOSIX::registerThreadCrashHandlers() {
	// All POSIX signals are process wide, so no thread specific actions are needed
	return true;
}

void CrashHandlerPOSIX::unregisterThreadCrashHandlers() {
	// All POSIX signals are process wide, so no thread specific actions are needed
}

enum CrashType {
	SIGNAL_SIGABRT,
	SIGNAL_SIGFPE,
	SIGNAL_SIGILL,
	SIGNAL_SIGSEGV,
	SIGNAL_UNKNOWN
};

void CrashHandlerPOSIX::handleCrash(int crashType, int FPECode) {
	
	Autolock autoLock(&m_Lock);
	
	// Run the callbacks
	for(std::vector<CrashHandler::CrashCallback>::iterator it = m_crashCallbacks.begin(); it != m_crashCallbacks.end(); ++it) {
		(*it)();
	}
	
	const char* crashSummary;
	
	switch(crashType) {
		case SIGNAL_SIGABRT:     crashSummary = "Abnormal termination"; break;
		case SIGNAL_SIGFPE:      crashSummary = "Floating-point error"; break;
		case SIGNAL_SIGILL:      crashSummary = "Illegal instruction"; break;
		case SIGNAL_SIGSEGV:     crashSummary = "Illegal storage access"; break;
		case SIGNAL_UNKNOWN:     crashSummary = "Unknown signal"; break;
		default:                 crashSummary = "Unknown error"; break;
	}
	
	strcpy(m_pCrashInfo->detailedCrashInfo, crashSummary);
	if(crashType == SIGNAL_SIGFPE) {
		// Append detailed information in case of a FPE exception
		const char* FPEDetailed;
		switch(FPECode) {
			case FPE_INVALID:         FPEDetailed = ": Invalid result"; break;
			case FPE_DENORMAL:        FPEDetailed = ": Denormal operand"; break;
			case FPE_ZERODIVIDE:      FPEDetailed = ": Divide by zero"; break;
			case FPE_OVERFLOW:        FPEDetailed = ": Overflow"; break;
			case FPE_UNDERFLOW:       FPEDetailed = ": Underflow"; break;
			case FPE_INEXACT:         FPEDetailed = ": Inexact precision"; break;
			case FPE_UNEMULATED:      FPEDetailed = ": Unemulated"; break;
			case FPE_SQRTNEG:         FPEDetailed = ": Negative square root"; break;
			case FPE_STACKOVERFLOW:   FPEDetailed = ": Stack Overflow"; break;
			case FPE_STACKUNDERFLOW:  FPEDetailed = ": Stack Underflow"; break;
			case FPE_EXPLICITGEN:     FPEDetailed = ": raise( SIGFPE ) was called"; break;
			case FPE_MULTIPLE_TRAPS:  FPEDetailed = ": Multiple traps"; break;
			case FPE_MULTIPLE_FAULTS: FPEDetailed = ": Multiple faults"; break;
			default:                  FPEDetailed = "";
		}
	}
	strcat(m_pCrashInfo->detailedCrashInfo, "\n\n");
	
	// Get current thread id
	m_pCrashInfo->threadId = boost::interprocess::detail::get_current_thread_id();
	
	strcpy(m_pCrashInfo->crashReportFolder, "Crashes");
	
	char arguments[256];
	strcpy(arguments, "-crashinfo=");
	strcat(arguments, m_SharedMemoryName.c_str());
	bool bCreateProcess = start_the_process_here("CrashReporter/arxcrashreporterforlinux.exe", arguments);
	
	// If CrashReporter was started, wait for its signal before exiting.
	if(bCreateProcess) {
		m_pCrashInfo->exitLock.wait();
	}
	
	exit(1);
}


void SignalHandler(int signalCode) {
	int crashType;
	switch(signalCode) {
		case SIGABRT: crashType = SIGNAL_SIGABRT; break;
		case SIGILL:  crashType = SIGNAL_SIGILL; break;
		case SIGSEGV: crashType = SIGNAL_SIGSEGV; break;
		case SIGFPE:  crashType = SIGNAL_SIGFPE; break;
		default:      crashType = SIGNAL_UNKNOWN; break;
	}
	CrashHandlerPOSIX::getInstance().handleCrash(crashType);
}

void SIGFPEHandler(int code, int FPECode) {
	CrashHandlerPOSIX::getInstance().handleCrash(SIGNAL_SIGFPE, FPECode);
}
