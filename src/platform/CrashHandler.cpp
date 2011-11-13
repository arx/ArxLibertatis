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

#include <cstdio>
#include <cstdlib>
#include <csignal>

#include "io/Filesystem.h"
#include "io/FilePath.h"
#include "io/log/Logger.h"
#include "platform/Platform.h"
#include "Configure.h"

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

void initCrashHandler() {
	
	// Catch 'bad' signals so we can print some debug output.
	
#ifdef SIGSEGV
	signal(SIGSEGV, crashHandler);
#endif
	
#ifdef SIGILL
	signal(SIGILL, crashHandler);
#endif
	
#ifdef SIGFPE
	signal(SIGFPE, crashHandler);
#endif
	
#ifdef SIGABRT
	signal(SIGABRT, crashHandler);
#endif
	
}

// don't have enough POSIX functionality for backtraces
#elif 0 // TODO ARX_PLATFORM == ARX_PLATFORM_WIN32

#include <new>
#include <cfloat>

#include <boost/interprocess/detail/os_thread_functions.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

CrashHandler* CrashHandler::m_sInstance = 0;

CrashHandler::CrashHandler()
	: m_pCrashInfo(0)
	, m_pPreviousCrashHandlers(0) {

	arx_assert(m_sInstance == 0);
	m_sInstance = this;
}

CrashHandler::~CrashHandler() {
	m_sInstance = 0;
}

CrashHandler& CrashHandler::getInstance() {
	arx_assert(m_sInstance != 0);
	return *m_sInstance;
}

bool CrashHandler::init() {
	Autolock autoLock(&m_Lock);

	if(m_pCrashInfo) {
		LogError << "Crash handler is already initialized.";
		return false;
	}

	if(IsDebuggerPresent()) {
		LogInfo << "Debugger attached, disabling crash handler.";
	}

	bool crashReporterFound = fs::exists("CrashReporter/CrashReporter.exe");
	if(crashReporterFound) {
		LogInfo << "CrashReporter found, initializing crash handler.";
	} else {
		LogError << "CrashReporter not found, disabling crash handler.";
		return false;
	}

	if(!initSharedMemory()) {
		LogError << "Failed to initialize shared memory.";
		return false;
	}

	fillBasicCrashInfo();

	return registerCrashHandlers();
}

void CrashHandler::shutdown() {
	Autolock autoLock(&m_Lock);
	
	// todo-clean up this mess! :)
}

bool CrashHandler::initSharedMemory() {
	// Generate a random name for our shared memory object
	boost::uuids::uuid uid = boost::uuids::random_generator()();
	m_SharedMemoryName = boost::lexical_cast<std::string>(uid);

	// Create a shared memory object.
	m_SharedMemory = boost::interprocess::shared_memory_object(boost::interprocess::create_only, m_SharedMemoryName.c_str(), boost::interprocess::read_write);

	// This is the argument that will be sent to the CrashReporter in case a crash occurs.
	//m_SharedMemoryName = std::string("\"{") + m_SharedMemoryName + std::string("}\"");

	// Resize to fit the CrashInfo structure
	m_SharedMemory.truncate(sizeof(CrashInfo));

	// Map the whole shared memory in this process
	m_MemoryMappedRegion = boost::interprocess::mapped_region(m_SharedMemory, boost::interprocess::read_write);

	// Our CrashInfo will be stored in this shared memory.
	m_pCrashInfo = new (m_MemoryMappedRegion.get_address()) CrashInfo;

	return true;
}

void CrashHandler::fillBasicCrashInfo() {
	m_pCrashInfo->processId = boost::interprocess::detail::get_current_process_id();
}

bool CrashHandler::addAttachedFile(const std::string& filename) {
	Autolock autoLock(&m_Lock);

	if(!m_pCrashInfo) {
		LogError << "Crash handler is not initialized.";
		return false;
	}

	if(m_pCrashInfo->nbFilesAttached == CrashInfo::MaxNbFiles) {
		LogError << "Too much files already attached to the crash report (" << m_pCrashInfo->nbFilesAttached << ").";
		return false;
	}

	if(filename.size() >= CrashInfo::MaxFilenameLen) {
		LogError << "File name is too long.";
		return false;
	}

	for(int i = 0; i < m_pCrashInfo->nbFilesAttached; i++) {
		if(strcmp(m_pCrashInfo->attachedFiles[i], filename.c_str()) == 0) {
			LogWarning << "File \"" << filename << "\" is already attached.";
			return false;
		}
	}

	strcpy(m_pCrashInfo->attachedFiles[m_pCrashInfo->nbFilesAttached], filename.c_str());
	m_pCrashInfo->nbFilesAttached++;

	return true;
}

bool CrashHandler::addNamedVariable(const std::string& name, const std::string& value) {
	Autolock autoLock(&m_Lock);

	if(!m_pCrashInfo) {
		LogError << "Crash handler is not initialized.";
		return false;
	}

	if(name.size() >= CrashInfo::MaxVariableNameLen) {
		LogError << "Variable name is too long.";
		return false;
	}

	if(value.size() >= CrashInfo::MaxVariableValueLen) {
		LogError << "Variable description is too long.";
		return false;
	}

	// Check if our array already contains this variable.
	for(int i = 0; i < m_pCrashInfo->nbVariables; i++) {
		if(strcmp(m_pCrashInfo->variables[i].name, name.c_str()) == 0) {
			strcpy(m_pCrashInfo->variables[i].value, value.c_str());
			return true;
		}
	}

	// Not found, must add a new one.
	if(m_pCrashInfo->nbVariables == CrashInfo::MaxNbVariables) {
		LogError << "Too much variables already added to the crash report (" << m_pCrashInfo->nbVariables << ").";
		return false;
	}

	strcpy(m_pCrashInfo->variables[m_pCrashInfo->nbVariables].name, name.c_str());
	strcpy(m_pCrashInfo->variables[m_pCrashInfo->nbVariables].value, value.c_str());
	m_pCrashInfo->nbVariables++;

	return true;
}

struct ThreadExceptionHandlers {
	terminate_handler m_terminateHandler;   // Terminate handler
	unexpected_handler m_unexpectedHandler; // Unexpected handler
	void (*m_SIGFPEHandler)(int);           // FPE handler
	void (*m_SIGILLHandler)(int);           // SIGILL handler
	void (*m_SIGSEGVHandler)(int);          // Illegal storage access handler
};

struct PlatformCrashHandlers {
	LPTOP_LEVEL_EXCEPTION_FILTER  m_SEHHandler;           // SEH exception filter.
	_purecall_handler m_pureCallHandler;                  // Pure virtual call exception filter.
	_PNH m_newHandler;                                    // New operator exception filter.
	_invalid_parameter_handler m_invalidParameterHandler; // Invalid parameter exception filter.
	void (*m_SIGABRTHandler)(int);                        // SIGABRT handler.
	void (*m_SIGINTHandler)(int);                         // SIGINT handler.
	void (*m_SIGTERMHandler)(int);                        // SIGTERM handler.

	// List of exception handlers installed for worker threads of current process.
	std::map<DWORD, ThreadExceptionHandlers> m_threadExceptionHandlers;
};

LONG WINAPI SEHHandler(PEXCEPTION_POINTERS pExceptionPtrs);
void PureCallHandler();
int NewHandler(size_t);
void InvalidParameterHandler(const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t pReserved);
void SignalHandler(int);

bool CrashHandler::registerCrashHandlers() {
	arx_assert(m_pPreviousCrashHandlers == 0);
	m_pPreviousCrashHandlers = new PlatformCrashHandlers;

	// Unhandled exception handler.
	m_pPreviousCrashHandlers->m_SEHHandler = SetUnhandledExceptionFilter(SEHHandler);

	// Prevent dialog box.
	_set_error_mode(_OUT_TO_STDERR);

	// Catch pure virtual function calls.
	// Because there is one _purecall_handler for the whole process,
	// calling this function immediately impacts all threads. The last
	// caller on any thread sets the handler.
	// http://msdn.microsoft.com/en-us/library/t296ys27.aspx
	m_pPreviousCrashHandlers->m_pureCallHandler = _set_purecall_handler(PureCallHandler);

	// Catch new operator memory allocation exceptions.
	_set_new_mode(1); // Force malloc() to call new handler too
	m_pPreviousCrashHandlers->m_newHandler = _set_new_handler(NewHandler);

	// Catch invalid parameter exceptions.
	m_pPreviousCrashHandlers->m_invalidParameterHandler = _set_invalid_parameter_handler(InvalidParameterHandler);

	// Catch an abnormal program termination.
	_set_abort_behavior(_CALL_REPORTFAULT, _CALL_REPORTFAULT);
	m_pPreviousCrashHandlers->m_SIGABRTHandler = signal(SIGABRT, SignalHandler);

	// Catch illegal instruction handler.
	m_pPreviousCrashHandlers->m_SIGINTHandler = signal(SIGINT, SignalHandler);

	// Catch a termination request.
	m_pPreviousCrashHandlers->m_SIGTERMHandler = signal(SIGTERM, SignalHandler);

	// We must also register the main thread crash handlers.
	return registerThreadCrashHandlers();
}

void CrashHandler::unregisterCrashHandlers() {
	unregisterThreadCrashHandlers();

	SetUnhandledExceptionFilter(m_pPreviousCrashHandlers->m_SEHHandler);
	_set_purecall_handler(m_pPreviousCrashHandlers->m_pureCallHandler);
	_set_new_handler(m_pPreviousCrashHandlers->m_newHandler);
	_set_invalid_parameter_handler(m_pPreviousCrashHandlers->m_invalidParameterHandler);
	signal(SIGABRT, m_pPreviousCrashHandlers->m_SIGABRTHandler);
	signal(SIGINT, m_pPreviousCrashHandlers->m_SIGINTHandler);
	signal(SIGTERM, m_pPreviousCrashHandlers->m_SIGTERMHandler);

	if(!m_pPreviousCrashHandlers->m_threadExceptionHandlers.empty())
		LogWarning << "Some threads crash handlers are still registered.";

	delete m_pPreviousCrashHandlers;
	m_pPreviousCrashHandlers = 0;
}

void TerminateHandler();
void UnexpectedHandler();
void SIGFPEHandler(int code, int subcode);
void SIGILLHandler(int);
void SIGSEGVHandler(int);

bool CrashHandler::registerThreadCrashHandlers() {
	Autolock autoLock(&m_Lock);

	DWORD dwThreadId = GetCurrentThreadId();

	std::map<DWORD, ThreadExceptionHandlers>::iterator it = m_pPreviousCrashHandlers->m_threadExceptionHandlers.find(dwThreadId);
	if(it != m_pPreviousCrashHandlers->m_threadExceptionHandlers.end()) {
		LogWarning << "Crash handlers are already registered for this thread.";
		return false;
	}

	ThreadExceptionHandlers& threadHandlers = m_pPreviousCrashHandlers->m_threadExceptionHandlers[dwThreadId];

	// Catch terminate() calls.
	// In a multithreaded environment, terminate functions are maintained
	// separately for each thread. Each new thread needs to install its own
	// terminate function. Thus, each thread is in charge of its own termination handling.
	// http://msdn.microsoft.com/en-us/library/t6fk7h29.aspx
	threadHandlers.m_terminateHandler = set_terminate(TerminateHandler);

	// Catch unexpected() calls.
	// In a multithreaded environment, unexpected functions are maintained
	// separately for each thread. Each new thread needs to install its own
	// unexpected function. Thus, each thread is in charge of its own unexpected handling.
	// http://msdn.microsoft.com/en-us/library/h46t5b69.aspx
	threadHandlers.m_unexpectedHandler = set_unexpected(UnexpectedHandler);

	// Catch a floating point error
	typedef void (*sigh)(int);
	threadHandlers.m_SIGFPEHandler = signal(SIGFPE, (sigh)SIGFPEHandler);

	// Catch an illegal instruction
	threadHandlers.m_SIGILLHandler = signal(SIGILL, SignalHandler);

	// Catch illegal storage access errors
	threadHandlers.m_SIGSEGVHandler = signal(SIGSEGV, SignalHandler);

	return true;
}

void CrashHandler::unregisterThreadCrashHandlers() {
	Autolock autoLock(&m_Lock);

	DWORD dwThreadId = GetCurrentThreadId();

	std::map<DWORD, ThreadExceptionHandlers>::iterator it = m_pPreviousCrashHandlers->m_threadExceptionHandlers.find(dwThreadId);
	if(it == m_pPreviousCrashHandlers->m_threadExceptionHandlers.end()) {
		LogWarning << "Crash handlers were not registered for this thread.";
		return;
	}

	ThreadExceptionHandlers& threadHandlers = it->second;

	set_terminate(threadHandlers.m_terminateHandler);
	set_unexpected(threadHandlers.m_unexpectedHandler);

	signal(SIGFPE, threadHandlers.m_SIGFPEHandler);
	signal(SIGILL, threadHandlers.m_SIGILLHandler);
	signal(SIGSEGV, threadHandlers.m_SIGSEGVHandler);

	m_pPreviousCrashHandlers->m_threadExceptionHandlers.erase(it);
}

enum CrashType {
	SEH_EXCEPTION,
	TERMINATE_CALL,
	UNEXPECTED_CALL,
	PURE_CALL,
	NEW_OPERATOR_ERROR,
	INVALID_PARAMETER,
	SIGNAL_SIGABRT,
	SIGNAL_SIGFPE,
	SIGNAL_SIGILL,
	SIGNAL_SIGINT,
	SIGNAL_SIGSEGV,
	SIGNAL_SIGTERM,
	SIGNAL_UNKNOWN
};

void appendExceptionContext(char* crashDetails, EXCEPTION_POINTERS* pExceptionInfo) {

	char buf[256];

	PCONTEXT pCtx = pExceptionInfo->ContextRecord;
	strcat(crashDetails, "Registers:\n");
		
	// TODO - Fix registers for win64...

	sprintf(buf, "  EAX:%08X  EBX:%08X  ECX:%08X  EDX:%08X  ESI:%08X  EDI:%08X\n", pCtx->Eax, pCtx->Ebx, pCtx->Ecx, pCtx->Edx, pCtx->Esi, pCtx->Edi);
	strcat(crashDetails, buf);

	sprintf(buf, "  CS:EIP:%04X:%08X\n", pCtx->SegCs, pCtx->Eip);
	strcat(crashDetails, buf);

	sprintf(buf, "  SS:ESP:%04X:%08X  EBP:%08X\n", pCtx->SegSs, pCtx->Esp, pCtx->Ebp);
	strcat(crashDetails, buf);

	sprintf(buf, "  DS:%04X  ES:%04X  FS:%04X  GS:%04X\n", pCtx->SegDs, pCtx->SegEs, pCtx->SegFs, pCtx->SegGs);
	strcat(crashDetails, buf);

	sprintf(buf, "  Flags:%08X\n\n", pCtx->EFlags);
	strcat(crashDetails, buf);
}

void CrashHandler::handleCrash(int crashType, void* crashExtraInfo, int FPECode) {
	Autolock autoLock(&m_Lock);

	const char* crashSummary;

	switch(crashType) {
		case SEH_EXCEPTION:      crashSummary = "Unhandled exception"; break;
		case TERMINATE_CALL:     crashSummary = "terminate() was called"; break;
		case UNEXPECTED_CALL:    crashSummary = "unexpected() was called"; break;
		case PURE_CALL:          crashSummary = "Pure virtual function called"; break;
		case NEW_OPERATOR_ERROR: crashSummary = "new operator failed"; break;
		case INVALID_PARAMETER:  crashSummary = "Invalid parameter detected"; break;
		case SIGNAL_SIGABRT:     crashSummary = "Abnormal termination"; break;
		case SIGNAL_SIGFPE:      crashSummary = "Floating-point error"; break;
		case SIGNAL_SIGILL:      crashSummary = "Illegal instruction"; break;
		case SIGNAL_SIGINT:      crashSummary = "CTRL+C signal"; break;
		case SIGNAL_SIGSEGV:     crashSummary = "Illegal storage access"; break;
		case SIGNAL_SIGTERM:     crashSummary = "Termination request"; break;
		case SIGNAL_UNKNOWN:     crashSummary = "Unknown signal"; break;
		default:                 crashSummary = "Unknown error"; break;
	}
	
	strcpy(m_pCrashInfo->detailedCrashInfo, crashSummary);
	if(crashType == SIGNAL_SIGFPE) {
		// Append detailed information in case of a FPE exception
		const char* FPEDetailed;
		switch(FPECode) {
			case _FPE_INVALID:         FPEDetailed = ": Invalid result"; break;
			case _FPE_DENORMAL:        FPEDetailed = ": Denormal operand"; break;
			case _FPE_ZERODIVIDE:      FPEDetailed = ": Divide by zero"; break;
			case _FPE_OVERFLOW:        FPEDetailed = ": Overflow"; break;
			case _FPE_UNDERFLOW:       FPEDetailed = ": Underflow"; break;
			case _FPE_INEXACT:         FPEDetailed = ": Inexact precision"; break;
			case _FPE_UNEMULATED:      FPEDetailed = ": Unemulated"; break;
			case _FPE_SQRTNEG:         FPEDetailed = ": Negative square root"; break;
			case _FPE_STACKOVERFLOW:   FPEDetailed = ": Stack Overflow"; break;
			case _FPE_STACKUNDERFLOW:  FPEDetailed = ": Stack Underflow"; break;
			case _FPE_EXPLICITGEN:     FPEDetailed = ": raise( SIGFPE ) was called"; break;
			case _FPE_MULTIPLE_TRAPS:  FPEDetailed = ": Multiple traps"; break;
			case _FPE_MULTIPLE_FAULTS: FPEDetailed = ": Multiple faults"; break;
			default:                   FPEDetailed = "";
		}
	}
	strcat(m_pCrashInfo->detailedCrashInfo, "\n\n");

	EXCEPTION_POINTERS ExceptionPointers;
	memset(&ExceptionPointers, 0, sizeof(ExceptionPointers));

	if(crashExtraInfo != 0) {
		EXCEPTION_POINTERS* pExceptionPointers = (EXCEPTION_POINTERS*)crashExtraInfo;
		appendExceptionContext(m_pCrashInfo->detailedCrashInfo, pExceptionPointers);
		m_pCrashInfo->pExceptionPointers = pExceptionPointers;
	} else {
		RtlCaptureContext(ExceptionPointers.ContextRecord);
		ExceptionPointers.ExceptionRecord->ExceptionCode = 0;
		ExceptionPointers.ExceptionRecord->ExceptionAddress = _ReturnAddress();
		m_pCrashInfo->pExceptionPointers = &ExceptionPointers;
	}

	// Get current thread id
	m_pCrashInfo->threadId = boost::interprocess::detail::get_current_thread_id();

	strcpy(m_pCrashInfo->crashReportFolder, "Crashes");
	m_pCrashInfo->miniDumpType = MiniDumpNormal;

	STARTUPINFO si;
	memset(&si, 0, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);

	PROCESS_INFORMATION pi;
	memset(&pi, 0, sizeof(PROCESS_INFORMATION));
	
	char arguments[256];
	strcpy(arguments, "-crashinfo=");
	strcat(arguments, m_SharedMemoryName.c_str());

	BOOL bCreateProcess = CreateProcess("CrashReporter/CrashReporter.exe", arguments, 0, 0, 0, 0, 0, 0, &si, &pi);

	// If CrashReporter was started, wait for its signal before exiting.
	if(bCreateProcess) {
		m_pCrashInfo->exitLock.wait();
	}
	
	TerminateProcess(GetCurrentProcess(), 1);
}

LONG WINAPI SEHHandler(PEXCEPTION_POINTERS pExceptionPtrs) {
	CrashHandler::getInstance().handleCrash(SEH_EXCEPTION, pExceptionPtrs);
	return EXCEPTION_EXECUTE_HANDLER;
}

void TerminateHandler() {
	CrashHandler::getInstance().handleCrash(TERMINATE_CALL);
}

void UnexpectedHandler() {
	CrashHandler::getInstance().handleCrash(UNEXPECTED_CALL);
}

void PureCallHandler() {
	CrashHandler::getInstance().handleCrash(PURE_CALL);
}

void InvalidParameterHandler(const wchar_t* /*expression*/, const wchar_t* /*function*/, const wchar_t* /*file*/, unsigned int /*line*/, uintptr_t /*pReserved*/) {
	CrashHandler::getInstance().handleCrash(INVALID_PARAMETER);
}

int NewHandler(size_t) {
	CrashHandler::getInstance().handleCrash(NEW_OPERATOR_ERROR);
	return 0;
}

void SignalHandler(int signalCode) {
	int crashType;
	switch(signalCode) {
		case SIGABRT: crashType = SIGNAL_SIGABRT; break;
		case SIGILL:  crashType = SIGNAL_SIGILL; break;
		case SIGINT:  crashType = SIGNAL_SIGINT; break;
		case SIGSEGV: crashType = SIGNAL_SIGSEGV; break;
		case SIGTERM: crashType = SIGNAL_SIGTERM; break;
		case SIGFPE:  crashType = SIGNAL_SIGFPE; break;
		default:      crashType = SIGNAL_UNKNOWN; break;
	}

	CrashHandler::getInstance().handleCrash(crashType, _pxcptinfoptrs);
}

void SIGFPEHandler(int code, int FPECode) {
	CrashHandler::getInstance().handleCrash(SIGNAL_SIGFPE, _pxcptinfoptrs, FPECode);
}


#else

void initCrashHandler() {
	
	// TODO implement for this platform
	
}

#endif
