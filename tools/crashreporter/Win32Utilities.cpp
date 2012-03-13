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

#include "crashreporter/Win32Utilities.h"

#ifdef HAVE_WINAPI

// Windows
#include <windows.h>
#include <string>
#include <sstream>
#include <psapi.h>
#include <dbghelp.h>

// Boost
#include <boost/crc.hpp>

#include "io/fs/FilePath.h"

typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
typedef BOOL (WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD);
#define PRODUCT_PROFESSIONAL	0x00000030
#define VER_SUITE_WH_SERVER	0x00008000

bool GetWindowsVersionName(char* str, int bufferSize)
{
	OSVERSIONINFOEX osvi;
	SYSTEM_INFO si;
	BOOL bOsVersionInfoEx;

	DWORD dwType; 
	
	ZeroMemory(&si, sizeof(SYSTEM_INFO));
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX)); 
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO*) &osvi); 
	
	if(bOsVersionInfoEx == 0)
		return false; // Call GetNativeSystemInfo if supported or GetSystemInfo otherwise.
	
	PGNSI pGNSI = (PGNSI) GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetNativeSystemInfo");
	if(NULL != pGNSI)
		pGNSI(&si);
	else 
		GetSystemInfo(&si); // Check for unsupported OS
	
	if (VER_PLATFORM_WIN32_NT != osvi.dwPlatformId || osvi.dwMajorVersion <= 4 )
		return false;
	
	std::stringstream os;
	os << "Microsoft "; // Test for the specific product. 
	
	if ( osvi.dwMajorVersion == 6 )
	{
		if( osvi.dwMinorVersion == 0 )
		{
			if( osvi.wProductType == VER_NT_WORKSTATION )
				os << "Windows Vista ";
			else
				os << "Windows Server 2008 ";
		}  
		if ( osvi.dwMinorVersion == 1 )
		{
			if( osvi.wProductType == VER_NT_WORKSTATION )
				os << "Windows 7 ";
			else
				os << "Windows Server 2008 R2 ";
		}  
		
		PGPI pGPI = (PGPI) GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetProductInfo");
		pGPI( osvi.dwMajorVersion, osvi.dwMinorVersion, 0, 0, &dwType); 
		
		switch( dwType )
		{
		case PRODUCT_ULTIMATE:
			os << "Ultimate Edition";
			break;
		case PRODUCT_PROFESSIONAL:
			os << "Professional";
			break;
		case PRODUCT_HOME_PREMIUM:
			os << "Home Premium Edition";
			break;
		case PRODUCT_HOME_BASIC:
			os << "Home Basic Edition";
			break;
		case PRODUCT_ENTERPRISE:
			os << "Enterprise Edition";
			break;
		case PRODUCT_BUSINESS:
			os << "Business Edition";
			break;
		case PRODUCT_STARTER:
			os << "Starter Edition";
			break;
		case PRODUCT_CLUSTER_SERVER:
			os << "Cluster Server Edition";
			break;
		case PRODUCT_DATACENTER_SERVER:
			os << "Datacenter Edition";
			break;
		case PRODUCT_DATACENTER_SERVER_CORE:
			os << "Datacenter Edition (core installation)";
			break;
		case PRODUCT_ENTERPRISE_SERVER:
			os << "Enterprise Edition";
			break;
		case PRODUCT_ENTERPRISE_SERVER_CORE:
			os << "Enterprise Edition (core installation)";
			break;
		case PRODUCT_ENTERPRISE_SERVER_IA64:
			os << "Enterprise Edition for Itanium-based Systems";
			break;
		case PRODUCT_SMALLBUSINESS_SERVER:
			os << "Small Business Server";
			break;
		case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
			os << "Small Business Server Premium Edition";
			break;
		case PRODUCT_STANDARD_SERVER:
			os << "Standard Edition";
			break;
		case PRODUCT_STANDARD_SERVER_CORE:
			os << "Standard Edition (core installation)";
			break;
		case PRODUCT_WEB_SERVER:
			os << "Web Server Edition";
			break;
		}
	} 
	
	if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 )
	{
		if( GetSystemMetrics(SM_SERVERR2) )
			os <<  "Windows Server 2003 R2, ";
		else if ( osvi.wSuiteMask & VER_SUITE_STORAGE_SERVER )
			os <<  "Windows Storage Server 2003";
		else if ( osvi.wSuiteMask & VER_SUITE_WH_SERVER )
			os <<  "Windows Home Server";
		else if( osvi.wProductType == VER_NT_WORKSTATION && si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64)
			os <<  "Windows XP Professional x64 Edition";
		else 
			os << "Windows Server 2003, ";  // Test for the server type.

		if ( osvi.wProductType != VER_NT_WORKSTATION )
		{
			if ( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_IA64 )
			{
				if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
					os <<  "Datacenter Edition for Itanium-based Systems";
				else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
					os <<  "Enterprise Edition for Itanium-based Systems";
			}   
			else if ( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64 )
			{
				if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
					os <<  "Datacenter x64 Edition";
				else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
					os <<  "Enterprise x64 Edition";
				else 
					os <<  "Standard x64 Edition";
			}  
			else
			{
				if ( osvi.wSuiteMask & VER_SUITE_COMPUTE_SERVER )
					os <<  "Compute Cluster Edition";
				else if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
					os <<  "Datacenter Edition";
				else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
					os <<  "Enterprise Edition";
				else if ( osvi.wSuiteMask & VER_SUITE_BLADE )
					os <<  "Web Edition";
				else 
					os <<  "Standard Edition";
			}
		}
	}
	
	if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 )
	{
		os << "Windows XP ";
		if( osvi.wSuiteMask & VER_SUITE_PERSONAL )
			os <<  "Home Edition";
		else 
			os <<  "Professional";
	} 
	
	if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
	{
		os << "Windows 2000 ";  
		
		if ( osvi.wProductType == VER_NT_WORKSTATION )
		{
			os <<  "Professional";
		}
		else 
		{
			if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
				os <<  "Datacenter Server";
			else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
				os <<  "Advanced Server";
			else 
				os <<  "Server";
		}
	} 
	
	// Include service pack (if any) and build number.
	if(strlen(osvi.szCSDVersion) > 0) 
		os << " " << osvi.szCSDVersion;
	
	os << " (build " << osvi.dwBuildNumber << ")";
	if ( osvi.dwMajorVersion >= 6 )
	{
		if ( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64 )
			os <<  ", 64-bit";
		else if (si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_INTEL )
			os << ", 32-bit";
	} 
	
	strcpy_s(str, bufferSize, os.str().c_str());
	return true; 
}

bool Is64BitWindows()
{
#if defined(_WIN64)
	return true;  // 64-bit programs run only on Win64
#elif defined(_WIN32)
	// 32-bit programs run on both 32-bit and 64-bit Windows
	BOOL f64 = FALSE;
	return IsWow64Process(GetCurrentProcess(), &f64) && f64;
#else
	return false;
#endif
}

ULONG64 ConvertSystemTimeToULONG64( const SYSTEMTIME& st )
{
	FILETIME ft ;
	SystemTimeToFileTime( &st, &ft ) ;
	ULARGE_INTEGER integer ;
	integer.LowPart = ft.dwLowDateTime ;
	integer.HighPart = ft.dwHighDateTime ;
	return integer.QuadPart ;
}

static BOOL CALLBACK LoadModuleCB(PCSTR ModuleName, DWORD64 ModuleBase, ULONG ModuleSize, PVOID UserContext)
{
    SymLoadModule64((HANDLE)UserContext, 0, ModuleName, ModuleName, ModuleBase, ModuleSize);
    return TRUE;
}

bool GetCallStackInfo(HANDLE hProcess, HANDLE hThread, PCONTEXT pContext, std::string& callstack, std::string& callstackTop, u32& callstackCrc)
{
	DWORD options = SymGetOptions();
	options |= SYMOPT_LOAD_LINES;
    options &= ~SYMOPT_DEFERRED_LOADS;
    options &= ~SYMOPT_UNDNAME;
	SymSetOptions (options);

	SymInitialize(hProcess, NULL, FALSE);

	STACKFRAME64 stackFrame;
	memset(&stackFrame, 0, sizeof(stackFrame));

	DWORD imageType;

#ifdef _M_IX86
	imageType = IMAGE_FILE_MACHINE_I386;
	stackFrame.AddrPC.Offset = pContext->Eip;
	stackFrame.AddrPC.Mode = AddrModeFlat;
	stackFrame.AddrFrame.Offset = pContext->Ebp;
	stackFrame.AddrFrame.Mode = AddrModeFlat;
	stackFrame.AddrStack.Offset = pContext->Esp;
	stackFrame.AddrStack.Mode = AddrModeFlat;
#elif _M_X64
	imageType = IMAGE_FILE_MACHINE_AMD64;
	stackFrame.AddrPC.Offset = pContext->Rip;
	stackFrame.AddrPC.Mode = AddrModeFlat;
	stackFrame.AddrFrame.Offset = pContext->Rsp;
	stackFrame.AddrFrame.Mode = AddrModeFlat;
	stackFrame.AddrStack.Offset = pContext->Rsp;
	stackFrame.AddrStack.Mode = AddrModeFlat;
#else
	#error "Unsupported"
#endif

	const size_t STACKWALK_MAX_NAMELEN = 1024;
	IMAGEHLP_SYMBOL64* pSymbol = (IMAGEHLP_SYMBOL64*)malloc(sizeof(IMAGEHLP_SYMBOL64) + STACKWALK_MAX_NAMELEN);
	memset(pSymbol, 0, sizeof(IMAGEHLP_SYMBOL64) + STACKWALK_MAX_NAMELEN);
	pSymbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
	pSymbol->MaxNameLength = STACKWALK_MAX_NAMELEN;
	
	CHAR undFullName[STACKWALK_MAX_NAMELEN];

	IMAGEHLP_LINE64 Line;
    memset( &Line, 0, sizeof(Line) );
	Line.SizeOfStruct = sizeof(Line);

	IMAGEHLP_MODULE64 Module;
	memset(&Module, 0, sizeof(Module));
    Module.SizeOfStruct = sizeof(Module);

	EnumerateLoadedModules64(hProcess, LoadModuleCB, hProcess);

	boost::crc_32_type callstackCRC;
	std::stringstream callstackStr;

	bool bDone = false;
	const int MAX_DEPTH = 256;
	for(int iEntry = 0; (iEntry < MAX_DEPTH) && !bDone; ++iEntry)
	{
		bool bRet;
		
		bRet = StackWalk64(imageType, hProcess, hThread, &stackFrame, pContext, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL) == TRUE;
		if(!bRet || stackFrame.AddrPC.Offset == 0)
		{
			bDone = true;
			continue;
		}

		callstackCRC.process_bytes(&stackFrame.AddrPC.Offset, sizeof(stackFrame.AddrPC.Offset));

		DWORD64 dwDisplacementSymbol = 0;
		DWORD dwDisplacementLine = 0;

		char* pSymbolName = 0;

		bool bHasSymInfo = SymGetSymFromAddr64(hProcess, stackFrame.AddrPC.Offset, &dwDisplacementSymbol, pSymbol) == TRUE;
		if(bHasSymInfo)
		{
			pSymbolName = pSymbol->Name;
			DWORD dwRet = UnDecorateSymbolName(pSymbol->Name, undFullName, STACKWALK_MAX_NAMELEN, UNDNAME_COMPLETE);
			if(dwRet != 0)
				pSymbolName = undFullName;
		}
			
		bool bHasLineInfo = SymGetLineFromAddr64(hProcess, stackFrame.AddrPC.Offset, &dwDisplacementLine, &Line) == TRUE;
		bool bHasModuleInfo = SymGetModuleInfo64(hProcess, stackFrame.AddrPC.Offset, &Module) == TRUE;

		callstackStr << "  ";

		if(bHasModuleInfo)
			callstackStr << fs::path(Module.ImageName).filename();
		else
			callstackStr << "??";

		callstackStr << "!";
		
		if(bHasSymInfo)
		{
			callstackStr << pSymbolName;
			callstackStr << "() ";
		}
		else
		{
			callstackStr << "0x" << std::hex << stackFrame.AddrPC.Offset << std::dec << " ";
		}
			
		if(bHasLineInfo)
		{
			callstackStr << " ";
			callstackStr << fs::path(Line.FileName).filename();
			callstackStr << "(";
			callstackStr << Line.LineNumber;
			callstackStr << ") ";
		}
			
		if(iEntry == 0)
			callstackTop = callstackStr.str();

		callstackStr << "\n";
	}

	callstack = callstackStr.str();
	callstackCrc = callstackCRC.checksum();

	return !callstack.empty();
}

std::string GetRegisters(PCONTEXT pCtx)
{
	std::string registersStr;
	char buf[256];
		
#if defined(_WIN64)
	sprintf(buf, "  RAX:%016X  RBX:%016X  RCX:%016X  RDX:%016X  RSI:%016X  RDI:%016X\n", pCtx->Rax, pCtx->Rbx, pCtx->Rcx, pCtx->Rdx, pCtx->Rsi, pCtx->Rdi);
	registersStr += buf;

	sprintf(buf, "  CS:RIP:%04X:%08X\n", pCtx->SegCs, pCtx->Rip);
	registersStr += buf;

	sprintf(buf, "  SS:RSP:%04X:%016X  RBP:%016X\n", pCtx->SegSs, pCtx->Rsp, pCtx->Rbp);
	registersStr += buf;
#else
    sprintf(buf, "  EAX:%08X  EBX:%08X  ECX:%08X  EDX:%08X  ESI:%08X  EDI:%08X\n", pCtx->Eax, pCtx->Ebx, pCtx->Ecx, pCtx->Edx, pCtx->Esi, pCtx->Edi);
	registersStr += buf;

	sprintf(buf, "  CS:EIP:%04X:%08X\n", pCtx->SegCs, pCtx->Eip);
	registersStr += buf;

	sprintf(buf, "  SS:ESP:%04X:%08X  EBP:%08X\n", pCtx->SegSs, pCtx->Esp, pCtx->Ebp);
	registersStr += buf;
#endif

	sprintf(buf, "  DS:%04X  ES:%04X  FS:%04X  GS:%04X\n", pCtx->SegDs, pCtx->SegEs, pCtx->SegFs, pCtx->SegGs);
	registersStr += buf;

	sprintf(buf, "  Flags:%08X\n", pCtx->EFlags);
	registersStr += buf;

	return registersStr;
}

// Given an exception code, returns a pointer to a static string with a description of the exception                                         
std::string GetExceptionString( DWORD dwCode )
{
    #define EXCEPTION( x ) case EXCEPTION_##x: return (#x);

    switch ( dwCode )
    {
        EXCEPTION( ACCESS_VIOLATION )
        EXCEPTION( DATATYPE_MISALIGNMENT )
        EXCEPTION( BREAKPOINT )
        EXCEPTION( SINGLE_STEP )
        EXCEPTION( ARRAY_BOUNDS_EXCEEDED )
        EXCEPTION( FLT_DENORMAL_OPERAND )
        EXCEPTION( FLT_DIVIDE_BY_ZERO )
        EXCEPTION( FLT_INEXACT_RESULT )
        EXCEPTION( FLT_INVALID_OPERATION )
        EXCEPTION( FLT_OVERFLOW )
        EXCEPTION( FLT_STACK_CHECK )
        EXCEPTION( FLT_UNDERFLOW )
        EXCEPTION( INT_DIVIDE_BY_ZERO )
        EXCEPTION( INT_OVERFLOW )
        EXCEPTION( PRIV_INSTRUCTION )
        EXCEPTION( IN_PAGE_ERROR )
        EXCEPTION( ILLEGAL_INSTRUCTION )
        EXCEPTION( NONCONTINUABLE_EXCEPTION )
        EXCEPTION( STACK_OVERFLOW )
        EXCEPTION( INVALID_DISPOSITION )
        EXCEPTION( GUARD_PAGE )
        EXCEPTION( INVALID_HANDLE )
    }

    // If not one of the "known" exceptions, try to get the string
    // from NTDLL.DLL's message table.

    static char szBuffer[512] = { 0 };

    FormatMessageA( FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE,
                    GetModuleHandleA( "NTDLL.DLL" ),
                    dwCode, 0, szBuffer, sizeof( szBuffer ), 0 );

    return szBuffer;
}

#endif // HAVE_WINAPI
