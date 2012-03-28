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


#ifdef HAVE_SIGACTION

static void signalHandler(int signal, siginfo_t * info, void * context) {
	ARX_UNUSED(context);
	CrashHandlerPOSIX::getInstance().handleCrash(signal, info->si_code);
}

typedef struct sigaction signal_handler;

static void registerSignalHandler(int s, signal_handler & old_handler) {
	struct sigaction handler;
	handler.sa_flags = SA_RESETHAND | SA_SIGINFO;
	handler.sa_sigaction = signalHandler;
	sigemptyset(&handler.sa_mask);
	sigaction(s, &handler, &old_handler);
}

static void unregisterSignalHandler(int s, signal_handler & old_handler) {
	sigaction(s, &old_handler, NULL);
}

#else

static void signalHandler(int signal) {
	CrashHandlerPOSIX::getInstance().handleCrash(signal, -1);
}

typedef void (*signal_handler)(int signal);

static void registerSignalHandler(int s, signal_handler & old_handler) {
	old_handler = signal(s, signalHandler);
}

static void unregisterSignalHandler(int s, signal_handler & old_handler) {
	signal(s, old_handler);
}

#endif

struct PlatformCrashHandlers {
	
	signal_handler illHandler;
	signal_handler abrtHandler;
	signal_handler busHandler;
	signal_handler fpeHandler;
	signal_handler segvHandler;
	
};

CrashHandlerPOSIX* CrashHandlerPOSIX::m_sInstance = 0;

CrashHandlerPOSIX::CrashHandlerPOSIX() {
	m_sInstance = this;
	m_CrashHandlerApp = "arxcrashreporter";
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
	
#ifdef SIGILL
	registerSignalHandler(SIGILL, m_pPreviousCrashHandlers->illHandler);
#endif
	
#ifdef SIGABRT
	registerSignalHandler(SIGABRT, m_pPreviousCrashHandlers->abrtHandler);
#endif
	
#ifdef SIGBUS
	registerSignalHandler(SIGBUS, m_pPreviousCrashHandlers->busHandler);
#endif
	
#ifdef SIGFPE
	registerSignalHandler(SIGFPE, m_pPreviousCrashHandlers->fpeHandler);
#endif
	
#ifdef SIGSEGV
	registerSignalHandler(SIGSEGV, m_pPreviousCrashHandlers->segvHandler);
#endif
	
	// We must also register the main thread crash handlers.
	return registerThreadCrashHandlers();
}

static void removeCrashHandlers(PlatformCrashHandlers * previous) {
	
#ifdef SIGILL
	unregisterSignalHandler(SIGILL, previous->illHandler);
#endif
	
#ifdef SIGABRT
	unregisterSignalHandler(SIGABRT, previous->abrtHandler);
#endif
	
#ifdef SIGBUS
	unregisterSignalHandler(SIGBUS, previous->busHandler);
#endif
	
#ifdef SIGFPE
	unregisterSignalHandler(SIGFPE, previous->fpeHandler);
#endif
	
#ifdef SIGSEGV
	unregisterSignalHandler(SIGSEGV, previous->segvHandler);
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
	if(!m_CrashHandlerPath.empty()) {
		execl(m_CrashHandlerPath.string().c_str(), m_CrashHandlerPath.string().c_str(),
		      arguments, NULL);
	}
#endif
	
	// Try a crash reporter in the system path.
#ifdef HAVE_EXECLP
	execlp(m_CrashHandlerApp.c_str(), m_CrashHandlerApp.c_str(), arguments, NULL);
#endif
	
	// Something went wrong - the crash reporter failed to start!
	
	// TODO(crash-handler) start fallback in-process crash handler and dump everything to file
	
	// Kill the original, busy-waiting process.
	kill(m_pCrashInfo->processId, SIGKILL);
	
	exit(0);
}

void CrashHandlerPOSIX::handleCrash(int signal, int code) {
	
	// Remove crash handlers so we don't end in an infinite crash loop
	removeCrashHandlers(m_pPreviousCrashHandlers);
	
	// Run the callbacks
	for(std::vector<CrashHandler::CrashCallback>::iterator it = m_crashCallbacks.begin();
	    it != m_crashCallbacks.end(); ++it) {
		(*it)();
	}
	
	m_pCrashInfo->signal = signal;
	m_pCrashInfo->code = code;
	
	// Store the backtrace in the shared crash info
	#ifdef HAVE_BACKTRACE
		backtrace(m_pCrashInfo->backtrace, ARRAY_SIZE(m_pCrashInfo->backtrace));
	#endif
	
	m_pCrashInfo->crashBrokerLock.post();
	
	while(true) {
		// Busy wait so we don't enter any additional stack frames and keep the backtrace clean.
	}
}
