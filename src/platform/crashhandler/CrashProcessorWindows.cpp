/*
 * Copyright 2015-2018 Arx Libertatis Team (see the AUTHORS file)
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

#include <cfloat>
#include <iomanip>
#include <sstream>

#include <psapi.h>
#include <dbghelp.h>

#include <boost/crc.hpp>
#include <boost/range/size.hpp>

#include "io/fs/FilePath.h"
#include "io/fs/Filesystem.h"

#include "platform/Architecture.h"
#include "platform/WindowsUtils.h"

#include "util/String.h"


static u64 convertFileTimeToInteger(const FILETIME & ft) {
	ULARGE_INTEGER integer;
	integer.LowPart = ft.dwLowDateTime;
	integer.HighPart = ft.dwHighDateTime;
	return integer.QuadPart;
}

void CrashHandlerWindows::processCrashInfo() {
	
	// Required for XP - for Vista+, PROCESS_QUERY_LIMITED_INFORMATION would be enough
	DWORD access = PROCESS_QUERY_INFORMATION | PROCESS_VM_READ;
	HANDLE process = OpenProcess(access, FALSE, m_pCrashInfo->processId);
	if(!process) {
		return;
	}
	
	// Get memory usage info
	PROCESS_MEMORY_COUNTERS meminfo;
	if(GetProcessMemoryInfo(process, &meminfo, sizeof(meminfo))) {
		m_pCrashInfo->memoryUsage = meminfo.WorkingSetSize;
	}
	
	// Determine how long thre process was running
	FILETIME creation, exit, kernel, user, now;
	if(GetProcessTimes(process, &creation, &exit, &kernel, &user)) {
		SYSTEMTIME time;
		GetSystemTime(&time);
		SystemTimeToFileTime(&time, &now);
		u64 delta = convertFileTimeToInteger(now) - convertFileTimeToInteger(creation);
		m_pCrashInfo->runningTime = double(delta) / 10000000;
	}
	
	CloseHandle(process);
	
}

static std::string getExceptionString(u32 code) {
	
	#define ARX_EXCEPTION(x) case EXCEPTION_##x: return ARX_STR(x);
	switch(code) {
		ARX_EXCEPTION(ACCESS_VIOLATION)
		ARX_EXCEPTION(DATATYPE_MISALIGNMENT)
		ARX_EXCEPTION(BREAKPOINT)
		ARX_EXCEPTION(SINGLE_STEP)
		ARX_EXCEPTION(ARRAY_BOUNDS_EXCEEDED)
		ARX_EXCEPTION(FLT_DENORMAL_OPERAND)
		ARX_EXCEPTION(FLT_DIVIDE_BY_ZERO)
		ARX_EXCEPTION(FLT_INEXACT_RESULT)
		ARX_EXCEPTION(FLT_INVALID_OPERATION)
		ARX_EXCEPTION(FLT_OVERFLOW)
		ARX_EXCEPTION(FLT_STACK_CHECK)
		ARX_EXCEPTION(FLT_UNDERFLOW)
		ARX_EXCEPTION(INT_DIVIDE_BY_ZERO)
		ARX_EXCEPTION(INT_OVERFLOW)
		ARX_EXCEPTION(PRIV_INSTRUCTION)
		ARX_EXCEPTION(IN_PAGE_ERROR)
		ARX_EXCEPTION(ILLEGAL_INSTRUCTION)
		ARX_EXCEPTION(NONCONTINUABLE)
		ARX_EXCEPTION(STACK_OVERFLOW)
		ARX_EXCEPTION(INVALID_DISPOSITION)
		ARX_EXCEPTION(GUARD_PAGE)
		ARX_EXCEPTION(INVALID_HANDLE)
		default: break;
	}
	#undef ARX_EXCEPTION
	
	// If not one of the "known" exceptions, try to get the string
	// from NTDLL.DLL's message table.
	return platform::getErrorString(code, GetModuleHandleW(L"ntdll.dll"));
}

void CrashHandlerWindows::processCrashSignal() {
	
	std::ostringstream description;
	
	switch(m_pCrashInfo->signal) {
		case USER_CRASH:         description << "Aborted"; break;
		case SEH_EXCEPTION:      description << "Unhandled exception"; break;
		case TERMINATE_CALL:     description << "terminate() was called"; break;
		case UNEXPECTED_CALL:    description << "unexpected() was called"; break;
		case PURE_CALL:          description << "Pure virtual function called"; break;
		case NEW_OPERATOR_ERROR: description << "new operator failed"; break;
		case INVALID_PARAMETER:  description << "Invalid parameter detected"; break;
		case SIGNAL_SIGABRT:     description << "Abnormal termination"; break;
		case SIGNAL_SIGFPE:      description << "Floating-point error"; break;
		case SIGNAL_SIGILL:      description << "Illegal instruction"; break;
		case SIGNAL_SIGINT:      description << "CTRL+C signal"; break;
		case SIGNAL_SIGSEGV:     description << "Illegal storage access"; break;
		case SIGNAL_SIGTERM:     description << "Termination request"; break;
		case SIGNAL_UNKNOWN:     description << "Unknown signal"; break;
		default:                 description << "Unknown error"; break;
	}
	
	// Append detailed information in case of a floating point exception
	if(m_pCrashInfo->signal == SIGNAL_SIGFPE) {
		switch(m_pCrashInfo->code) {
			case _FPE_INVALID:         description << ": Invalid result"; break;
			case _FPE_DENORMAL:        description << ": Denormal operand"; break;
			case _FPE_ZERODIVIDE:      description << ": Divide by zero"; break;
			case _FPE_OVERFLOW:        description << ": Overflow"; break;
			case _FPE_UNDERFLOW:       description << ": Underflow"; break;
			case _FPE_INEXACT:         description << ": Inexact precision"; break;
			case _FPE_UNEMULATED:      description << ": Unemulated"; break;
			case _FPE_SQRTNEG:         description << ": Negative square root"; break;
			case _FPE_STACKOVERFLOW:   description << ": Stack Overflow"; break;
			case _FPE_STACKUNDERFLOW:  description << ": Stack Underflow"; break;
			case _FPE_EXPLICITGEN:     description << ": raise( SIGFPE ) was called"; break;
			#ifdef _FPE_MULTIPLE_TRAPS // Not available on all VC++ versions
			case _FPE_MULTIPLE_TRAPS:  description << ": Multiple traps"; break;
			#endif
			#ifdef _FPE_MULTIPLE_FAULTS // Not available on all VC++ versions
			case _FPE_MULTIPLE_FAULTS: description << ": Multiple faults"; break;
			#endif
			default:                   break;
		}
	}
	
	description << "\n\n";
	
	if(m_pCrashInfo->exceptionCode) {
		std::string exception = getExceptionString(m_pCrashInfo->exceptionCode);
		if(!exception.empty()) {
			description << "Exception code: " << exception << "\n\n";
		}
	}
	
	addText(description.str().c_str());
}


static BOOL CALLBACK loadModuleCB(PCSTR name, DWORD64 base, ULONG size, PVOID context) {
	SymLoadModule64((HANDLE)context, 0, name, name, base, size);
	return TRUE;
}

void CrashHandlerWindows::processCrashTrace() {
	
	std::ostringstream description;
	
	// Open parent process handle
	HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_pCrashInfo->processId);
	if(!process) {
		description << "\nCould not open process: " << platform::getErrorString() << '\n';
		return;
	}
	
	HANDLE thread = OpenThread(THREAD_ALL_ACCESS, FALSE, m_pCrashInfo->threadId);
	if(!thread) {
		description << "\nCould not open thread: " << platform::getErrorString() << '\n';
	}
	
	DWORD options = SymGetOptions();
	options |= SYMOPT_LOAD_LINES;
	options &= ~SYMOPT_DEFERRED_LOADS;
	options &= ~SYMOPT_UNDNAME;
	SymSetOptions(options);
	if(SymInitialize(process, NULL, FALSE) != TRUE) {
		description << "\nCould not load symbols: " << platform::getErrorString() << '\n';
	}
	
	PCONTEXT context = reinterpret_cast<PCONTEXT>(m_pCrashInfo->contextRecord);
	ARX_STATIC_ASSERT(sizeof(m_pCrashInfo->contextRecord) >= sizeof(*context),
	                  "buffer too small");
	STACKFRAME64 stackFrame;
	memset(&stackFrame, 0, sizeof(stackFrame));
	DWORD imageType;
	#if ARX_ARCH == ARX_ARCH_X86
	imageType = IMAGE_FILE_MACHINE_I386;
	stackFrame.AddrPC.Offset = context->Eip;
	stackFrame.AddrPC.Mode = AddrModeFlat;
	stackFrame.AddrFrame.Offset = context->Ebp;
	stackFrame.AddrFrame.Mode = AddrModeFlat;
	stackFrame.AddrStack.Offset = context->Esp;
	stackFrame.AddrStack.Mode = AddrModeFlat;
	#elif ARX_ARCH == ARX_ARCH_X86_64
	imageType = IMAGE_FILE_MACHINE_AMD64;
	stackFrame.AddrPC.Offset = context->Rip;
	stackFrame.AddrPC.Mode = AddrModeFlat;
	stackFrame.AddrFrame.Offset = context->Rsp;
	stackFrame.AddrFrame.Mode = AddrModeFlat;
	stackFrame.AddrStack.Offset = context->Rsp;
	stackFrame.AddrStack.Mode = AddrModeFlat;
	#else
	#error "Unsupported architecture"
	#endif
	
	const size_t MaxSymbolLength = 1024;
	char symbolBuffer[sizeof(IMAGEHLP_SYMBOL64) + MaxSymbolLength];
	memset(symbolBuffer, 0, sizeof(symbolBuffer));
	IMAGEHLP_SYMBOL64 * symbol = reinterpret_cast<IMAGEHLP_SYMBOL64 *>(symbolBuffer);
	symbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
	symbol->MaxNameLength = MaxSymbolLength;
	char undecoratedName[MaxSymbolLength];
	
	IMAGEHLP_LINE64 line;
	memset(&line, 0, sizeof(line));
	line.SizeOfStruct = sizeof(line);
	
	IMAGEHLP_MODULE64 module;
	memset(&module, 0, sizeof(module));
	module.SizeOfStruct = sizeof(module);
	
	EnumerateLoadedModules64(process, loadModuleCB, process);
	
	description << "\nCallstack:\n";
	
	if(platform::isWoW64Process(process)) {
		description << " Warning: WoW64 process detected, stack may be corrupted!\n";
	}
	
	boost::crc_32_type checksum;

	for(int i = 0; i < CrashInfo::MaxCallstackDepth; ++i) {
		
		BOOL ret = StackWalk64(imageType, process, thread, &stackFrame, context, NULL,
		                       SymFunctionTableAccess64, SymGetModuleBase64, NULL);
		if(ret != TRUE || stackFrame.AddrPC.Offset == 0) {
			break;
		}
		DWORD64 address = stackFrame.AddrPC.Offset;
		
		char * function = 0;
		DWORD64 dSymbol = 0;
		BOOL hasSymbol = SymGetSymFromAddr64(process, address, &dSymbol, symbol);
		if(hasSymbol == TRUE) {
			DWORD undecorated = UnDecorateSymbolName(symbol->Name, undecoratedName,
			                                         MaxSymbolLength, UNDNAME_COMPLETE);
			function = (undecorated != 0) ? undecoratedName : symbol->Name;
		}
		
		DWORD dLine = 0;
		BOOL hasLine = SymGetLineFromAddr64(process, address, &dLine, &line);
		BOOL hasModule = SymGetModuleInfo64(process, address, &module);
		
		std::ostringstream frame;
		if(hasModule == TRUE) {
			std::string image = fs::path(module.ImageName).filename();
			frame << image;
			checksum.process_bytes(image.data(), image.length());
			address -= module.BaseOfImage;
		} else {
			frame << "??";
		}
		
		frame << '!';
		checksum.process_bytes(&address, sizeof(address));
		if(hasSymbol == TRUE) {
			frame << function << "()";
		} else {
			frame << "0x" << std::hex << stackFrame.AddrPC.Offset << std::dec;
		}
		if(hasLine == TRUE) {
			frame << "  " << fs::path(line.FileName).filename() << ':' << line.LineNumber;
		}
			
		if(i == 0) {
			util::storeStringTerminated(m_pCrashInfo->title, frame.str());
		}
		
		description << ' ' << frame.str() << '\n';
	}
	
	m_pCrashInfo->crashId = checksum.checksum();
	
	CloseHandle(thread);
	CloseHandle(process);
	
	addText(description.str().c_str());
}

void CrashHandlerWindows::processCrashDump() {
	
	if(m_pCrashInfo->miniDumpTmpFile[0] == L'\0') {
		return;
	}
	
	m_pCrashInfo->miniDumpTmpFile[boost::size(m_pCrashInfo->miniDumpTmpFile) - 1] = L'\0';
	fs::path tempfile = platform::WideString::toUTF8(m_pCrashInfo->miniDumpTmpFile);
	if(!fs::is_regular_file(tempfile)) {
		return;
	}
	
	fs::path minidump = m_crashReportDir / "crash.dmp";
	fs::rename(tempfile, minidump);
	
	addAttachedFile(minidump);
}
