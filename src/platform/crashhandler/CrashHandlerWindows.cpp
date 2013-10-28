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

#include "platform/crashhandler/CrashHandlerWindows.h"

#include <new.h>     // <new> won't do it... we need some MS specific functions...
#include <cfloat>    // For _FPE_XXX constants
#include <intrin.h>  // _ReturnAddress()
#include <csignal>

typedef void (*signal_handler)(int);

struct ThreadExceptionHandlers {
	terminate_handler m_terminateHandler;     // Terminate handler
	unexpected_handler m_unexpectedHandler;   // Unexpected handler
	signal_handler m_SIGFPEHandler;           // FPE handler
	signal_handler m_SIGILLHandler;           // SIGILL handler
	signal_handler m_SIGSEGVHandler;          // Illegal storage access handler
};

struct PlatformCrashHandlers {
	LPTOP_LEVEL_EXCEPTION_FILTER  m_SEHHandler;           // SEH exception filter.
	_purecall_handler m_pureCallHandler;                  // Pure virtual call exception filter.
	_PNH m_newHandler;                                    // New operator exception filter.
	_invalid_parameter_handler m_invalidParameterHandler; // Invalid parameter exception filter.
	signal_handler m_SIGABRTHandler;                      // SIGABRT handler.
	signal_handler m_SIGINTHandler;                       // SIGINT handler.
	signal_handler m_SIGTERMHandler;                      // SIGTERM handler.

	// List of exception handlers installed for worker threads of current process.
	std::map<DWORD, ThreadExceptionHandlers> m_threadExceptionHandlers;
};

LONG WINAPI SEHHandler(PEXCEPTION_POINTERS pExceptionPtrs);
void PureCallHandler();
int NewHandler(size_t);
void InvalidParameterHandler(const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t pReserved);
void SignalHandler(int signalCode);

CrashHandlerWindows* CrashHandlerWindows::m_sInstance = 0;

CrashHandlerWindows::CrashHandlerWindows() {
	m_sInstance = this;
	m_CrashHandlerApp = "arxcrashreporter.exe";
}

CrashHandlerWindows::~CrashHandlerWindows() {
	m_sInstance = 0;
}

CrashHandlerWindows& CrashHandlerWindows::getInstance() {
	arx_assert(m_sInstance != 0);
	return *m_sInstance;
}

bool CrashHandlerWindows::registerCrashHandlers() {
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

void CrashHandlerWindows::unregisterCrashHandlers() {
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
void SIGILLHandler(int signalCode);
void SIGSEGVHandler(int signalCode);

bool CrashHandlerWindows::registerThreadCrashHandlers() {
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
	threadHandlers.m_SIGFPEHandler = signal(SIGFPE, (signal_handler)SIGFPEHandler);

	// Catch an illegal instruction
	threadHandlers.m_SIGILLHandler = signal(SIGILL, SignalHandler);

	// Catch illegal storage access errors
	threadHandlers.m_SIGSEGVHandler = signal(SIGSEGV, SignalHandler);

	return true;
}

void CrashHandlerWindows::unregisterThreadCrashHandlers() {
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

// This callbask function is called by MiniDumpWriteDump
BOOL CALLBACK miniDumpCallback(PVOID, PMINIDUMP_CALLBACK_INPUT pInput, PMINIDUMP_CALLBACK_OUTPUT pOutput) {
	BOOL bRet = FALSE; 
	
	// Check parameters 
	if( pInput == 0 || pOutput == 0 ) 
		return FALSE; 

	// Process the callbacks 
	switch( pInput->CallbackType ) 
	{		
	case IncludeModuleCallback: // Include module infos into the dump 
	case IncludeThreadCallback: // Include thread infos into the dump 
	case ThreadCallback:        // Include all thread information into the minidump
	case ThreadExCallback:		// Include this information 
		bRet = TRUE; 
		break; 

	case ModuleCallback: 
	{
		// Does the module have ModuleReferencedByMemory flag set ? 
		if( !(pOutput->ModuleWriteFlags & ModuleReferencedByMemory) ) 
		{
			// No, it does not - exclude it
			pOutput->ModuleWriteFlags &= (~ModuleWriteModule); 
		}

		bRet = TRUE; 
	}
	break; 
				
	case MemoryCallback: 
	case CancelCallback: 
		// We do not include any information here -> return FALSE 
		bRet = FALSE;
		break; 
	}

	return bRet;
}

void CrashHandlerWindows::writeCrashDump(PEXCEPTION_POINTERS pExceptionPointers) {

	// Build the temporary path to store the minidump
    CHAR* tempPathBuffer = m_pCrashInfo->miniDumpTmpFile;
    GetTempPath(MAX_PATH, m_pCrashInfo->miniDumpTmpFile);
    DWORD tick = GetTickCount();
    char tickChar[24];
    _ultoa(tick, tickChar, 10);
    strcat(&tempPathBuffer[0], tickChar);
    strcat(&tempPathBuffer[0], ".dmp");

	// Create the minidump file
	HANDLE hFile = CreateFile(tempPathBuffer, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
		return;

	MINIDUMP_TYPE miniDumpType = (MINIDUMP_TYPE)(MiniDumpWithIndirectlyReferencedMemory | MiniDumpScanMemory);

	MINIDUMP_EXCEPTION_INFORMATION exceptionInfo;
	exceptionInfo.ThreadId = GetCurrentThreadId();
	exceptionInfo.ExceptionPointers = pExceptionPointers;
	exceptionInfo.ClientPointers = TRUE;
  
	MINIDUMP_CALLBACK_INFORMATION callbackInfo;
	callbackInfo.CallbackRoutine = miniDumpCallback;
	callbackInfo.CallbackParam = 0;

	// Write the minidump
	MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, miniDumpType, &exceptionInfo, NULL, NULL);
	CloseHandle(hFile);
}

void CrashHandlerWindows::handleCrash(int crashType, void* crashExtraInfo, int FPECode) {

	Autolock autoLock(&m_Lock);

	EXCEPTION_POINTERS* pExceptionPointers = (EXCEPTION_POINTERS*)crashExtraInfo;

	// Run the callbacks
	for(std::vector<CrashHandler::CrashCallback>::iterator it = m_crashCallbacks.begin(); it != m_crashCallbacks.end(); ++it)
		(*it)();
	
	// Get summary 
	getCrashSummary(crashType, FPECode);

	// Write crash dump
	writeCrashDump(pExceptionPointers);

	// Copy CONTEXT to crash info structure
	memset(&m_pCrashInfo->contextRecord, 0, sizeof(m_pCrashInfo->contextRecord));
	if(pExceptionPointers != 0) {
		m_pCrashInfo->exceptionCode = pExceptionPointers->ExceptionRecord->ExceptionCode;
		memcpy(&m_pCrashInfo->contextRecord, pExceptionPointers->ContextRecord, sizeof(m_pCrashInfo->contextRecord));
	} else {
		RtlCaptureContext(&m_pCrashInfo->contextRecord);
	}
	
	// Get current thread id
	m_pCrashInfo->threadHandle = GetCurrentThread();

	waitForReporter();
	
	TerminateProcess(GetCurrentProcess(), 1);
}

void CrashHandlerWindows::getCrashSummary(int crashType, int FPECode) {
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
#ifdef _FPE_MULTIPLE_TRAPS // Not available on all VC++ versions
			case _FPE_MULTIPLE_TRAPS:  FPEDetailed = ": Multiple traps"; break;
#endif
#ifdef _FPE_MULTIPLE_FAULTS // Not available on all VC++ versions
			case _FPE_MULTIPLE_FAULTS: FPEDetailed = ": Multiple faults"; break;
#endif
			default:                   FPEDetailed = "";
		}
		
		strcat(m_pCrashInfo->detailedCrashInfo, FPEDetailed);
	}
	strcat(m_pCrashInfo->detailedCrashInfo, "\n");
}

void CrashHandlerWindows::waitForReporter() {
	STARTUPINFO si;
	memset(&si, 0, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);

	PROCESS_INFORMATION pi;
	memset(&pi, 0, sizeof(PROCESS_INFORMATION));
	
	char arguments[256];
	strcpy(arguments, "-crashinfo=");
	strcat(arguments, m_SharedMemoryName.c_str());

	BOOL bCreateProcess;
	
	bCreateProcess = CreateProcess(m_CrashHandlerPath.string().c_str(), arguments, 0, 0, 0, 0, 0, 0, &si, &pi);
	
	// If CrashReporter was started, wait for its signal before exiting.
	// Also test if the crash reporter has exited so that we don't wait forever in exceptionnal situations.
	if(bCreateProcess) {
		HANDLE crashHandlerProcess = OpenProcess(SYNCHRONIZE, FALSE, pi.dwProcessId);
		
		bool exit;
		do {
			exit = m_pCrashInfo->exitLock.try_wait();
			if(!exit) {
				exit = WaitForSingleObject(crashHandlerProcess, 100) != WAIT_TIMEOUT;
			}
		} while(!exit);

		CloseHandle(crashHandlerProcess);
	} else {
		// TODO(crash-handler) start fallback in-process crash handler and dump everything to file
	}
}

LONG WINAPI SEHHandler(PEXCEPTION_POINTERS pExceptionPtrs) {
	CrashHandlerWindows::getInstance().handleCrash(SEH_EXCEPTION, pExceptionPtrs);
	return EXCEPTION_EXECUTE_HANDLER;
}

void TerminateHandler() {
	CrashHandlerWindows::getInstance().handleCrash(TERMINATE_CALL);
}

void UnexpectedHandler() {
	CrashHandlerWindows::getInstance().handleCrash(UNEXPECTED_CALL);
}

void PureCallHandler() {
	CrashHandlerWindows::getInstance().handleCrash(PURE_CALL);
}

void InvalidParameterHandler(const wchar_t* /*expression*/, const wchar_t* /*function*/, const wchar_t* /*file*/, unsigned int /*line*/, uintptr_t /*pReserved*/) {
	CrashHandlerWindows::getInstance().handleCrash(INVALID_PARAMETER);
}

int NewHandler(size_t) {
	CrashHandlerWindows::getInstance().handleCrash(NEW_OPERATOR_ERROR);
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

	CrashHandlerWindows::getInstance().handleCrash(crashType, _pxcptinfoptrs);
}

void SIGFPEHandler(int code, int FPECode) {
	ARX_UNUSED(code);
	CrashHandlerWindows::getInstance().handleCrash(SIGNAL_SIGFPE, _pxcptinfoptrs, FPECode);
}