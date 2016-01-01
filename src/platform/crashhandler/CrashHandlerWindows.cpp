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
#include <intrin.h>  // _ReturnAddress()
#include <csignal>

#include "io/log/Logger.h"

typedef void (*signal_handler)(int signal);

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
}

CrashHandlerWindows::~CrashHandlerWindows() {
	m_sInstance = 0;
}

bool CrashHandlerWindows::initialize() {
	
	if(!CrashHandlerImpl::initialize()) {
		return false;
	}
	
	// Cache WideString exe & arg so we don't need to do the conversion in the crash handler
	m_exe = m_executable.string();
	m_args = m_executable.filename() + " --crashinfo=" + m_SharedMemoryName;
	
	return true;
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
	m_pPreviousCrashHandlers->m_invalidParameterHandler
		= _set_invalid_parameter_handler(InvalidParameterHandler);
	
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
	
	ThreadExceptionHandlers& threadHandlers
		= m_pPreviousCrashHandlers->m_threadExceptionHandlers[dwThreadId];
	
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
	
	std::map<DWORD, ThreadExceptionHandlers>::iterator it
		= m_pPreviousCrashHandlers->m_threadExceptionHandlers.find(dwThreadId);
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

void CrashHandlerWindows::handleCrash(int crashType, void * crashExtraInfo, int fpeCode) {
	
	Autolock autoLock(&m_Lock);
	
	EXCEPTION_POINTERS * pExceptionPointers = (EXCEPTION_POINTERS*)crashExtraInfo;
	
	// Run the callbacks
	for(std::vector<CrashHandler::CrashCallback>::iterator it = m_crashCallbacks.begin();
	    it != m_crashCallbacks.end(); ++it) {
		(*it)();
	}
	
	m_pCrashInfo->signal = crashType;
	m_pCrashInfo->code = fpeCode;
	
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
	m_pCrashInfo->threadHandle = u64(GetCurrentThread());
	
	// Try to spawn a sub-process to process the crash info
	STARTUPINFO si;
	memset(&si, 0, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	PROCESS_INFORMATION pi;
	memset(&pi, 0, sizeof(PROCESS_INFORMATION));
	BOOL created = CreateProcessW(m_exe, m_args.data(), NULL, NULL, FALSE,
	                              0, NULL, NULL, &si, &pi);
	if(created) {
		while(true) {
			if(m_pCrashInfo->exitLock.try_wait()) {
				break;
			}
			if(WaitForSingleObject(pi.hProcess, 100) != WAIT_TIMEOUT) {
				break;
			}
		}
		TerminateProcess(GetCurrentProcess(), 1);
		unregisterCrashHandlers();
		std::abort();
	}
	
	// Fallback: process the crash info in-process
	unregisterCrashHandlers();
	processCrash();
	
	std::abort();
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

void InvalidParameterHandler(const wchar_t * /*expression*/, const wchar_t * /*function*/,
                             const wchar_t * /*file*/, unsigned int /*line*/,
                             uintptr_t /*pReserved*/) {
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
