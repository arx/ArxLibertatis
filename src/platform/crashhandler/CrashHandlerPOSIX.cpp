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

#include "platform/crashhandler/CrashHandlerPOSIX.h"

#include "Configure.h"

#if defined(HAVE_BACKTRACE)
#include <execinfo.h>
#endif

#if defined(HAVE_PRCTL)
#include <sys/prctl.h>
#ifndef PR_SET_PTRACER
#define PR_SET_PTRACER 0x59616d61
#endif
#endif

#include <signal.h>
#include <string.h>
#include <unistd.h>

#include "platform/Environment.h"

typedef void (*signal_handler)(int signal);

static void SignalHandler(int signalCode) {
	CrashHandlerPOSIX::getInstance().handleCrash(signalCode);
}

static void SIGFPEHandler(int signalCode, int FPECode) {
	CrashHandlerPOSIX::getInstance().handleCrash(signalCode, FPECode);
}

struct PlatformCrashHandlers {
	signal_handler m_SIGSEGVHandler;    // Illegal storage access handler.
	signal_handler m_SIGILLHandler;     // SIGINT handler.
	signal_handler m_SIGFPEHandler;     // FPE handler.
	signal_handler m_SIGABRTHandler;    // SIGABRT handler.
};

CrashHandlerPOSIX* CrashHandlerPOSIX::m_sInstance = 0;

CrashHandlerPOSIX::CrashHandlerPOSIX() {
	m_sInstance = this;
	m_CrashHandlerApp = "./arxcrashreporter";
}

bool CrashHandlerPOSIX::initialize() {
	
	if(!CrashHandlerImpl::initialize()) {
		return false;
	}
	
	m_pCrashInfo->signal = 0;
	
#if defined(HAVE_PRCTL)
	// Allow all processes in the same pid namespace to PTRACE this process
	prctl(PR_SET_PTRACER, getpid());
#endif
	
	// pre-fork the crash handler
	if(!fork()) {
		
#if defined(HAVE_PRCTL) && defined(PR_SET_NAME)
		prctl(PR_SET_NAME, reinterpret_cast<unsigned long>("arxcrashhandler"));
#endif
		
		crashBroker();
	}
	
	return true;
}

void CrashHandlerPOSIX::shutdown() {
	if(m_pCrashInfo) {
		m_pCrashInfo->crashBrokerLock.post();
	}
}

CrashHandlerPOSIX::~CrashHandlerPOSIX() {
	m_sInstance = 0;
}

CrashHandlerPOSIX& CrashHandlerPOSIX::getInstance() {
	arx_assert(m_sInstance != 0);
	return *m_sInstance;
}

void CrashHandlerPOSIX::fillBasicCrashInfo() {
	CrashHandlerImpl::fillBasicCrashInfo();
	std::string exe = getExecutablePath();
	if(exe.length() < ARRAY_SIZE(m_pCrashInfo->execFullName)) {
		strcpy(m_pCrashInfo->execFullName, exe.c_str());
	} else {
		m_pCrashInfo->execFullName[0] = '\0';
	}
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
	
	// TODO(crash-handler) is the cast OK?
#ifdef SIGFPE
	m_pPreviousCrashHandlers->m_SIGFPEHandler = signal(SIGFPE, signal_handler(SIGFPEHandler));
#endif
	
#ifdef SIGABRT
	m_pPreviousCrashHandlers->m_SIGABRTHandler = signal(SIGABRT, SignalHandler);
#endif
	
	// We must also register the main thread crash handlers.
	return registerThreadCrashHandlers();
}

static void removeCrashHandlers(PlatformCrashHandlers * previous) {
	
#ifdef SIGSEGV
	signal(SIGSEGV, previous->m_SIGSEGVHandler);
#endif
	
#ifdef SIGILL
	signal(SIGILL, previous->m_SIGILLHandler);
#endif
	
#ifdef SIGFPE
	signal(SIGFPE, previous->m_SIGFPEHandler);
#endif
	
#ifdef SIGABRT
	signal(SIGABRT, previous->m_SIGABRTHandler);
#endif
	
}

void CrashHandlerPOSIX::unregisterCrashHandlers() {
	
	unregisterThreadCrashHandlers();
	
	removeCrashHandlers(m_pPreviousCrashHandlers);
	
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

PlatformCrashHandlers nullHandlers = { 0, 0, 0, 0 };

void CrashHandlerPOSIX::crashBroker() {
	
#if defined(HAVE_PRCTL) && defined(PR_SET_PDEATHSIG) && defined(SIGTERM)
	prctl(PR_SET_PDEATHSIG, SIGTERM);
#endif
	
	m_pCrashInfo->crashBrokerLock.wait();
	
	if(m_pCrashInfo->signal == 0) {
		exit(0);
	}
	
#if defined(HAVE_PRCTL) && defined(PR_SET_PDEATHSIG) && defined(SIGTERM)
		prctl(PR_SET_PDEATHSIG, 0);
#endif
	
	char arguments[256];
	strcpy(arguments, "-crashinfo=");
	strcat(arguments, m_SharedMemoryName.c_str());
	
	// Try a the crash reporter in the same directory as arx or in the current directory.
#ifdef HAVE_EXECL
	execl(m_CrashHandlerApp.c_str(), "arxcrashreporter", arguments, NULL);
#endif
	
	// Try a crash reporter in the system path.
#ifdef HAVE_EXECLP
	execlp("arxcrashreporter", "arxcrashreporter", arguments, NULL);
#endif
	
	// Something went wrong - the crash reporter failed to start!
	
	// TODO(crash-handler) start fallback in-process crash handler and dump everything to file
	
	// Kill the original, busy-waiting process.
	kill(m_pCrashInfo->processId, m_pCrashInfo->signal);
}

void CrashHandlerPOSIX::handleCrash(int crashType, int fpeCode) {
	
	// Remove crash handlers so we don't end in an infinite crash loop
	removeCrashHandlers(&nullHandlers);
	
	// Run the callbacks
	for(std::vector<CrashHandler::CrashCallback>::iterator it = m_crashCallbacks.begin();
	    it != m_crashCallbacks.end(); ++it) {
		(*it)();
	}
	
	m_pCrashInfo->signal = crashType;
	m_pCrashInfo->fpeCode = fpeCode;
	
	// Store the backtrace in the shared crash info
	#ifdef HAVE_BACKTRACE
		backtrace(m_pCrashInfo->backtrace, ARRAY_SIZE(m_pCrashInfo->backtrace));
	#endif
	
	m_pCrashInfo->crashBrokerLock.post();
	
	while(true) {
		// Busy wait so we don't enter any additional stack frames and keep the backtrace clean.
	}
}
