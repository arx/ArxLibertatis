/*
 * Copyright 2011-2013 Arx Libertatis Team (see the AUTHORS file)
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

#if ARX_HAVE_BACKTRACE
#include <execinfo.h>
#endif

#if ARX_HAVE_PRCTL
#include <sys/prctl.h>
#ifndef PR_SET_PTRACER
#define PR_SET_PTRACER 0x59616d61
#endif
#endif

#include <signal.h>
#include <string.h>
#include <unistd.h>

#include <iostream>


#if ARX_HAVE_SIGACTION

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

CrashHandlerPOSIX::CrashHandlerPOSIX() : m_pPreviousCrashHandlers(NULL) {
	m_sInstance = this;
	m_CrashHandlerApp = "arxcrashreporter";
}

bool CrashHandlerPOSIX::initialize() {
	
	if(!CrashHandlerImpl::initialize()) {
		return false;
	}
	
	m_pCrashInfo->signal = 0;
	
#if ARX_HAVE_PRCTL
	// Allow all processes in the same pid namespace to PTRACE this process
	prctl(PR_SET_PTRACER, getpid());
#endif
	
	return true;
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
	#if ARX_HAVE_BACKTRACE
		backtrace(m_pCrashInfo->backtrace, ARRAY_SIZE(m_pCrashInfo->backtrace));
	#endif
	
	// Using fork() in a signal handler is bad, but we are already crashing anyway
	// Maybe we should use the fork() syscall directly, like google breakpad does.
	// TODO Or better yet, switch to using google breakpad!
	if(fork() > 0) {
		while(true) {
			// Busy wait so we don't enter any additional stack frames
			// and keep the backtrace clean.
		}
	}
	
	// Try to execute the crash reporter
#ifdef ARX_HAVE_EXECVP
	char argument[256];
	strcpy(argument, "-crashinfo=");
	strcat(argument, m_SharedMemoryName.c_str());
	const char * args[] = { m_CrashHandlerPath.string().c_str(), argument, NULL };
	execvp(m_CrashHandlerPath.string().c_str(), const_cast<char **>(args));
#endif
	
	// Something went wrong - the crash reporter failed to start!
	
	std::cerr << "Arx Libertatis crashed with signal " << m_pCrashInfo->signal
	          << ", but " << m_CrashHandlerApp << " could not be found.\n";
	std::cerr << "arx.log might contain more information.\n";
	std::cerr << "Please install " << m_CrashHandlerApp
	          << " to generate a detailed bug report!" << std::endl;
	
	// Kill the original, busy-waiting process.
	kill(m_pCrashInfo->processId, SIGKILL);
	
	exit(0);
	
}
