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

/*
#if defined(HAVE_FORK) && defined(HAVE_READLINK) && defined(HAVE_KILL) \
	&& defined(HAVE_SIGNAL) && defined(SIGKILL) \
	&& (defined(HAVE_EXECLP) || (defined(HAVE_BACKTRACE) && defined(HAVE_BACKTRACE_SYMBOLS_FD))) \
	&& (defined(SIGSEGV) || defined(SIGILL) || defined(SIGFPE))



#if defined(HAVE_BACKTRACE) && defined(HAVE_BACKTRACE_SYMBOLS_FD)
#include <execinfo.h>
#endif

#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

static void crashHandler(int signal_) {
	
	int pid = getpid(); // Get the PID if the original process to debug.
	
	// TODO avoid using functions that allocate memory in the crash handler
	
#ifdef HAVE_STRSIGNAL
	LogError << strsignal(signal_) << ", pid=" << getpid();
#else
	LogError << "Caught signal " << signal_ << ", pid=" << getpid();
#endif
	
	fflush(stdout), fflush(stderr);
	
	// Fork to keep the backtrace of the original process clean.
	if(!fork()) {
		
		// Fork again so we retain control after launching GDB.
		int child = fork();
		if(!child) {
			
#ifdef HAVE_EXECLP
			
#ifdef HAVE_DUP2
			// Redirect stdout to stderr.
			dup2(2, 1);
#endif
			
			// Prepare executable and pid arguments for GDB.
			char name_buf[512];
			name_buf[readlink("/proc/self/exe", name_buf, 511)] = 0;
			char pid_buf[30];
			memset(&pid_buf, 0, sizeof(pid_buf));
			sprintf(pid_buf, "%d", pid);
			
			// Try to execute gdb to get a very detailed stack trace.
			execlp("gdb", "gdb", "--batch", "-n", "-ex", "thread", "-ex", "set confirm off", "-ex", "set print frame-arguments all", "-ex", "set print static-members off", "-ex", "thread apply all bt full", name_buf, pid_buf, NULL);
			
			// GDB failed to start.
			LogWarning << "Install GDB to get better backtraces.";
			
#endif // HAVE_EXECLP
			
#if defined(HAVE_BACKTRACE) && defined(HAVE_BACKTRACE_SYMBOLS_FD)
			{
				
				void * buffer[100];
				
				// Fallback to generate a basic stack trace.
				size_t size = backtrace(buffer, ARRAY_SIZE(buffer));
				
				// Print the stacktrace, skipping the innermost stack frame.
				if(size > 1) {
					backtrace_symbols_fd(buffer + 1, size - 1, 2);
				}
				
			}
#endif
			
			fflush(stdout), fflush(stderr);
			
			exit(1);
			
		} else {
			
			// Wait for GDB to exit.
			waitpid(child, NULL, 0);
			
			LogError << "Bloody Gobblers! Please report this, including the complete log output above.";
			
			fflush(stdout), fflush(stderr);
			
			// Kill the original, busy-waiting process.
			kill(pid, SIGKILL);
			
			exit(1);
		}
		
	} else {
		// Busy wait so we don't enter any additional stack frames and keep the backtrace clean.
		while(true);
	}
	
}

// don't have enough POSIX functionality for backtraces
#elif 0 // TODO ARX_PLATFORM == ARX_PLATFORM_WIN32
*/

#if defined(HAVE_CRASHHANDLER_POSIX)
#include "platform/crashhandler/CrashHandlerPOSIX.h"
#elif defined(HAVE_CRASHHANDLER_WINDOWS)
#include "platform/crashhandler/CrashHandlerWindows.h"
#endif

static CrashHandlerImpl * gCrashHandlerImpl = 0;
static int gInitCount = 0;

bool CrashHandler::initialize() {
	
	if(!gCrashHandlerImpl){
		
#if defined(HAVE_CRASHHANDLER_POSIX)
		
		gCrashHandlerImpl = new CrashHandlerPOSIX();
		
#elif defined(HAVE_CRASHHANDLER_WINDOWS)
		
		if(IsDebuggerPresent()) {
			LogInfo << "Debugger attached, disabling crash handler.";
			return false;
		}
		
		gCrashHandlerImpl = new CrashHandlerWindows();
		
#endif
		
		bool initialized = gCrashHandlerImpl->initialize();
		if(!initialized) {
			delete gCrashHandlerImpl;
			gCrashHandlerImpl = 0;
			return false;
		}
	}
	
	gInitCount++;
	return true;
}

void CrashHandler::shutdown() {
	gInitCount--;
	if(gInitCount == 0) {
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
