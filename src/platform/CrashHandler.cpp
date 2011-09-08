
#include "platform/CrashHandler.h"

#include <cstring>
#include <cstdio>
#include <cstdlib>

#include "io/Logger.h"

#include "Configure.h"

#if defined(HAVE_UNISTD_H_) && defined(HAVE_SIGNAL_H)

#include <unistd.h>
#include <signal.h>

static void crashHandler(int signal) {
	
	int pid = getpid();
	
	LogError << "Bloody Gobblers! " << strsignal(signal) << ", pid=" << getpid();
	
	if(!fork()) {
		
		// Redirect stdout to stderr.
		dup2(2, 1);
		
		char name_buf[512];
		name_buf[readlink("/proc/self/exe", name_buf, 511)] = 0;
		
		
		char pid_buf[30];
		memset(&pid_buf, 0, sizeof(pid_buf));
		sprintf(pid_buf, "%d", pid);
		
		// Try to execute gdb to get a very detailed stack trace.
		execlp("gdb", "gdb", "--batch", "-n", "-ex", "thread", "-ex", "set confirm off", "-ex", "set print frame-arguments all", "-ex", "set print static-members off", "-ex", "bt full", "-ex", "kill", name_buf, pid_buf, NULL);
		
		// GDB failed to start.
		LogWarning << "Install GDB for to get an automatic backtrace.";
		
		kill(pid, SIGKILL);
		abort();
	} else {
		while(true);
	}
}

void initCrashHandler() {
	
	signal(SIGSEGV, crashHandler);
	signal(SIGILL, crashHandler);
	signal(SIGFPE, crashHandler);
	//signal(SIGABRT, crashHandler);
	
}

#else

void initCrashHandler() {
	
	// TODO implement for windows
	
}

#endif
