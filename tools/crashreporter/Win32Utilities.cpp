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

#include "crashreporter/Win32Utilities.h"

#ifdef ARX_HAVE_WINAPI

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

// stolen from WinNT.h (last updated from 8.0 SDK)
// Product types
// This list grows with each OS release.
//
// There is no ordering of values to ensure callers
// do an equality test i.e. greater-than and less-than
// comparisons are not useful.
//
// NOTE: Values in this list should never be deleted.
//       When a product-type 'X' gets dropped from a
//       OS release onwards, the value of 'X' continues
//       to be used in the mapping table of GetProductInfo.
// MSDN: If the product has not been activated and is no longer in
//       the grace period, this parameter is set to
//       PRODUCT_UNLICENSED (0xABCDABCD).

#define PRODUCT_UNDEFINED                           0x00000000

#define PRODUCT_ULTIMATE                            0x00000001
#define PRODUCT_HOME_BASIC                          0x00000002
#define PRODUCT_HOME_PREMIUM                        0x00000003
#define PRODUCT_ENTERPRISE                          0x00000004
#define PRODUCT_HOME_BASIC_N                        0x00000005
#define PRODUCT_BUSINESS                            0x00000006
#define PRODUCT_STANDARD_SERVER                     0x00000007
#define PRODUCT_DATACENTER_SERVER                   0x00000008
#define PRODUCT_SMALLBUSINESS_SERVER                0x00000009
#define PRODUCT_ENTERPRISE_SERVER                   0x0000000A
#define PRODUCT_STARTER                             0x0000000B
#define PRODUCT_DATACENTER_SERVER_CORE              0x0000000C
#define PRODUCT_STANDARD_SERVER_CORE                0x0000000D
#define PRODUCT_ENTERPRISE_SERVER_CORE              0x0000000E
#define PRODUCT_ENTERPRISE_SERVER_IA64              0x0000000F
#define PRODUCT_BUSINESS_N                          0x00000010
#define PRODUCT_WEB_SERVER                          0x00000011
#define PRODUCT_CLUSTER_SERVER                      0x00000012
#define PRODUCT_HOME_SERVER                         0x00000013
#define PRODUCT_STORAGE_EXPRESS_SERVER              0x00000014
#define PRODUCT_STORAGE_STANDARD_SERVER             0x00000015
#define PRODUCT_STORAGE_WORKGROUP_SERVER            0x00000016
#define PRODUCT_STORAGE_ENTERPRISE_SERVER           0x00000017
#define PRODUCT_SERVER_FOR_SMALLBUSINESS            0x00000018
#define PRODUCT_SMALLBUSINESS_SERVER_PREMIUM        0x00000019
#define PRODUCT_HOME_PREMIUM_N                      0x0000001A
#define PRODUCT_ENTERPRISE_N                        0x0000001B
#define PRODUCT_ULTIMATE_N                          0x0000001C
#define PRODUCT_WEB_SERVER_CORE                     0x0000001D
#define PRODUCT_MEDIUMBUSINESS_SERVER_MANAGEMENT    0x0000001E
#define PRODUCT_MEDIUMBUSINESS_SERVER_SECURITY      0x0000001F
#define PRODUCT_MEDIUMBUSINESS_SERVER_MESSAGING     0x00000020
#define PRODUCT_SERVER_FOUNDATION                   0x00000021
#define PRODUCT_HOME_PREMIUM_SERVER                 0x00000022
#define PRODUCT_SERVER_FOR_SMALLBUSINESS_V          0x00000023
#define PRODUCT_STANDARD_SERVER_V                   0x00000024
#define PRODUCT_DATACENTER_SERVER_V                 0x00000025
#define PRODUCT_ENTERPRISE_SERVER_V                 0x00000026
#define PRODUCT_DATACENTER_SERVER_CORE_V            0x00000027
#define PRODUCT_STANDARD_SERVER_CORE_V              0x00000028
#define PRODUCT_ENTERPRISE_SERVER_CORE_V            0x00000029
#define PRODUCT_HYPERV                              0x0000002A
#define PRODUCT_STORAGE_EXPRESS_SERVER_CORE         0x0000002B
#define PRODUCT_STORAGE_STANDARD_SERVER_CORE        0x0000002C
#define PRODUCT_STORAGE_WORKGROUP_SERVER_CORE       0x0000002D
#define PRODUCT_STORAGE_ENTERPRISE_SERVER_CORE      0x0000002E
#define PRODUCT_STARTER_N                           0x0000002F
#define PRODUCT_PROFESSIONAL                        0x00000030
#define PRODUCT_PROFESSIONAL_N                      0x00000031
#define PRODUCT_SB_SOLUTION_SERVER                  0x00000032
#define PRODUCT_SERVER_FOR_SB_SOLUTIONS             0x00000033
#define PRODUCT_STANDARD_SERVER_SOLUTIONS           0x00000034
#define PRODUCT_STANDARD_SERVER_SOLUTIONS_CORE      0x00000035
#define PRODUCT_SB_SOLUTION_SERVER_EM               0x00000036
#define PRODUCT_SERVER_FOR_SB_SOLUTIONS_EM          0x00000037
#define PRODUCT_SOLUTION_EMBEDDEDSERVER             0x00000038
#define PRODUCT_SOLUTION_EMBEDDEDSERVER_CORE        0x00000039
#define PRODUCT_ESSENTIALBUSINESS_SERVER_MGMT       0x0000003B
#define PRODUCT_ESSENTIALBUSINESS_SERVER_ADDL       0x0000003C
#define PRODUCT_ESSENTIALBUSINESS_SERVER_MGMTSVC    0x0000003D
#define PRODUCT_ESSENTIALBUSINESS_SERVER_ADDLSVC    0x0000003E
#define PRODUCT_SMALLBUSINESS_SERVER_PREMIUM_CORE   0x0000003F
#define PRODUCT_CLUSTER_SERVER_V                    0x00000040
#define PRODUCT_EMBEDDED                            0x00000041
#define PRODUCT_STARTER_E                           0x00000042
#define PRODUCT_HOME_BASIC_E                        0x00000043
#define PRODUCT_HOME_PREMIUM_E                      0x00000044
#define PRODUCT_PROFESSIONAL_E                      0x00000045
#define PRODUCT_ENTERPRISE_E                        0x00000046
#define PRODUCT_ULTIMATE_E                          0x00000047
#define PRODUCT_ENTERPRISE_EVALUATION               0x00000048
#define PRODUCT_MULTIPOINT_STANDARD_SERVER          0x0000004C
#define PRODUCT_MULTIPOINT_PREMIUM_SERVER           0x0000004D
#define PRODUCT_STANDARD_EVALUATION_SERVER          0x0000004F
#define PRODUCT_DATACENTER_EVALUATION_SERVER        0x00000050
#define PRODUCT_ENTERPRISE_N_EVALUATION             0x00000054
#define PRODUCT_EMBEDDED_AUTOMOTIVE                 0x00000055
#define PRODUCT_EMBEDDED_INDUSTRY_A                 0x00000056
#define PRODUCT_THINPC                              0x00000057
#define PRODUCT_EMBEDDED_A                          0x00000058
#define PRODUCT_EMBEDDED_INDUSTRY                   0x00000059
#define PRODUCT_EMBEDDED_E                          0x0000005A
#define PRODUCT_EMBEDDED_INDUSTRY_E                 0x0000005B
#define PRODUCT_EMBEDDED_INDUSTRY_A_E               0x0000005C
#define PRODUCT_STORAGE_WORKGROUP_EVALUATION_SERVER 0x0000005F
#define PRODUCT_STORAGE_STANDARD_EVALUATION_SERVER  0x00000060
#define PRODUCT_CORE_ARM                            0x00000061
#define PRODUCT_CORE_N                              0x00000062
#define PRODUCT_CORE_COUNTRYSPECIFIC                0x00000063
#define PRODUCT_CORE_SINGLELANGUAGE                 0x00000064
#define PRODUCT_CORE                                0x00000065
#define PRODUCT_PROFESSIONAL_WMC                    0x00000067

#define PRODUCT_UNLICENSED                          0xABCDABCD

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
	
	if ( osvi.dwMajorVersion == 6 ) {

		if( osvi.dwMinorVersion == 0 ) {
			if( osvi.wProductType == VER_NT_WORKSTATION )
				os << "Windows Vista ";
			else
				os << "Windows Server 2008 ";
		} else if ( osvi.dwMinorVersion == 1 ) {
			if( osvi.wProductType == VER_NT_WORKSTATION )
				os << "Windows 7 ";
			else
				os << "Windows Server 2008 R2 ";
		} else if ( osvi.dwMinorVersion == 2 ) {
			if( osvi.wProductType == VER_NT_WORKSTATION )
				os << "Windows 8 ";
			else
				os << "Windows Server 2012 ";
		} else {
			os << " Windows Version " << osvi.dwMajorVersion << "." << osvi.dwMinorVersion << " ";
		}

		
		PGPI pGPI = (PGPI) GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetProductInfo");
		pGPI( osvi.dwMajorVersion, osvi.dwMinorVersion, 0, 0, &dwType); 
		
		switch( dwType )
		{
		case PRODUCT_BUSINESS:
			os << "Business";
			break;
		case PRODUCT_BUSINESS_N:
			os << "Business N";
			break;
		case PRODUCT_CLUSTER_SERVER:
			os << "HPC Edition";
			break;
		case PRODUCT_CLUSTER_SERVER_V:
			os << "Server Hyper Core V";
			break;
		case PRODUCT_CORE:
			//os << "Windows 8";
			break;
		case PRODUCT_CORE_N:
			os << "N";
			break;
		case PRODUCT_CORE_COUNTRYSPECIFIC:
			os << "China";
			break;
		case PRODUCT_CORE_SINGLELANGUAGE:
			os << "Single Language";
			break;
		case PRODUCT_DATACENTER_EVALUATION_SERVER:
			os << "Server Datacenter (evaluation installation)";
			break;
		case PRODUCT_DATACENTER_SERVER:
			os << "Server Datacenter (full installation)";
			break;
		case PRODUCT_DATACENTER_SERVER_CORE:
			os << "Server Datacenter (core installation)";
			break;
		case PRODUCT_DATACENTER_SERVER_CORE_V:
			os << "Server Datacenter without Hyper-V (core installation)";
			break;
		case PRODUCT_DATACENTER_SERVER_V:
			os << "Server Datacenter without Hyper-V (full installation)";
			break;
		case PRODUCT_ENTERPRISE:
			os << "Enterprise";
			break;
		case PRODUCT_ENTERPRISE_E:
			//os << "Not supported";
			break;
		case PRODUCT_ENTERPRISE_N_EVALUATION:
			os << "Enterprise N (evaluation installation)";
			break;
		case PRODUCT_ENTERPRISE_N:
			os << "Enterprise N";
			break;
		case PRODUCT_ENTERPRISE_EVALUATION:
			os << "Server Enterprise (evaluation installation)";
			break;
		case PRODUCT_ENTERPRISE_SERVER:
			os << "Server Enterprise (full installation)";
			break;
		case PRODUCT_ENTERPRISE_SERVER_CORE:
			os << "Server Enterprise (core installation)";
			break;
		case PRODUCT_ENTERPRISE_SERVER_CORE_V:
			os << "Server Enterprise without Hyper-V (core installation)";
			break;
		case PRODUCT_ENTERPRISE_SERVER_IA64:
			os << "Server Enterprise for Itanium-based Systems";
			break;
		case PRODUCT_ENTERPRISE_SERVER_V:
			os << "Server Enterprise without Hyper-V (full installation)";
			break;
		case PRODUCT_ESSENTIALBUSINESS_SERVER_MGMT:
			os << "Essential Server Solution Management";
			break;
		case PRODUCT_ESSENTIALBUSINESS_SERVER_ADDL:
			os << "Essential Server Solution Additional";
			break;
		case PRODUCT_ESSENTIALBUSINESS_SERVER_MGMTSVC:
			os << "Essential Server Solution Management SVC";
			break;
		case PRODUCT_ESSENTIALBUSINESS_SERVER_ADDLSVC:
			os << "Essential Server Solution Additional SVC";
			break;
		case PRODUCT_HOME_BASIC:
			os << "Home Basic";
			break;
		case PRODUCT_HOME_BASIC_E:
			//os << "Not supported";
			break;
		case PRODUCT_HOME_BASIC_N:
			os << "Home Basic N";
			break;
		case PRODUCT_HOME_PREMIUM:
			os << "Home Premium";
			break;
		case PRODUCT_HOME_PREMIUM_E:
			//os << "Not supported";
			break;
		case PRODUCT_HOME_PREMIUM_N:
			os << "Home Premium N";
			break;
		case PRODUCT_HOME_PREMIUM_SERVER:
			os << "Home Server 2011";
			break;
		case PRODUCT_HOME_SERVER:
			os << "Storage Server 2008 R2 Essentials";
			break;
		case PRODUCT_HYPERV:
			os << "Hyper-V Server";
			break;
		case PRODUCT_MEDIUMBUSINESS_SERVER_MANAGEMENT:
			os << "Essential Business Server Management Server";
			break;
		case PRODUCT_MEDIUMBUSINESS_SERVER_MESSAGING:
			os << "Essential Business Server Messaging Server";
			break;
		case PRODUCT_MEDIUMBUSINESS_SERVER_SECURITY:
			os << "Essential Business Server Security Server";
			break;
		case PRODUCT_MULTIPOINT_STANDARD_SERVER:
			os << "MultiPoint Server Standard (full installation)";
			break;
		case PRODUCT_MULTIPOINT_PREMIUM_SERVER:
			os << "MultiPoint Server Premium (full installation)";
			break;
		case PRODUCT_PROFESSIONAL:
			os << "Professional";
			break;
		case PRODUCT_PROFESSIONAL_E:
			//os << "Not supported";
			break;
		case PRODUCT_PROFESSIONAL_N:
			os << "Professional N";
			break;
		case PRODUCT_PROFESSIONAL_WMC:
			os << "Professional with Media Center";
			break;
		case PRODUCT_SB_SOLUTION_SERVER_EM:
			os << "Server For SB Solutions EM";
			break;
		case PRODUCT_SERVER_FOR_SB_SOLUTIONS:
			os << "Server For SB Solutions";
			break;
		case PRODUCT_SERVER_FOR_SB_SOLUTIONS_EM:
			os << "Server For SB Solutions EM";
			break;
		case PRODUCT_SERVER_FOR_SMALLBUSINESS:
			os << "Server 2008 for Windows Essential Server Solutions";
			break;
		case PRODUCT_SERVER_FOR_SMALLBUSINESS_V:
			os << "Server 2008 without Hyper-V for Windows Essential Server Solutions";
			break;
		case PRODUCT_SERVER_FOUNDATION:
			os << "Server Foundation";
			break;
		case PRODUCT_SB_SOLUTION_SERVER:
			os << "Small Business Server 2011 Essentials";
			break;
		case PRODUCT_SMALLBUSINESS_SERVER:
			os << "Small Business Server";
			break;
		case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
			os << "Small Business Server Premium";
			break;
		case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM_CORE:
			os << "Small Business Server Premium (core installation)";
			break;
		case PRODUCT_SOLUTION_EMBEDDEDSERVER:
			os << "MultiPoint Server";
			break;
		case PRODUCT_STANDARD_EVALUATION_SERVER:
			os << "Server Standard (evaluation installation)";
			break;
		case PRODUCT_STANDARD_SERVER:
			os << "Server Standard";
			break;
		case PRODUCT_STANDARD_SERVER_CORE:
			os << "Server Standard (core installation)";
			break;
		case PRODUCT_STANDARD_SERVER_V:
			os << "Server Standard without Hyper-V";
			break;
		case PRODUCT_STANDARD_SERVER_CORE_V:
			os << "Server Standard without Hyper-V (core installation)";
			break;
		case PRODUCT_STANDARD_SERVER_SOLUTIONS:
			os << "Server Solutions Premium";
			break;
		case PRODUCT_STANDARD_SERVER_SOLUTIONS_CORE:
			os << "Server Solutions Premium (core installation)";
			break;
		case PRODUCT_STARTER:
			os << "Starter";
			break;
		case PRODUCT_STARTER_E:
			//os << "Not supported";
			break;
		case PRODUCT_STARTER_N:
			os << "Starter N";
			break;
		case PRODUCT_STORAGE_ENTERPRISE_SERVER:
			os << "Storage Server Enterprise";
			break;
		case PRODUCT_STORAGE_ENTERPRISE_SERVER_CORE:
			os << "Storage Server Enterprise (core installation)";
			break;
		case PRODUCT_STORAGE_EXPRESS_SERVER:
			os << "Storage Server Express";
			break;
		case PRODUCT_STORAGE_EXPRESS_SERVER_CORE:
			os << "Storage Server Express (core installation)";
			break;
		case PRODUCT_STORAGE_STANDARD_EVALUATION_SERVER:
			os << "Storage Server Standard (evaluation installation)";
			break;
		case PRODUCT_STORAGE_STANDARD_SERVER:
			os << "Storage Server Standard";
			break;
		case PRODUCT_STORAGE_STANDARD_SERVER_CORE:
			os << "Storage Server Standard (core installation)";
			break;
		case PRODUCT_STORAGE_WORKGROUP_EVALUATION_SERVER:
			os << "Storage Server Workgroup (evaluation installation)";
			break;
		case PRODUCT_STORAGE_WORKGROUP_SERVER:
			os << "Storage Server Workgroup";
			break;
		case PRODUCT_STORAGE_WORKGROUP_SERVER_CORE:
			os << "Storage Server Workgroup (core installation)";
			break;
		case PRODUCT_UNDEFINED:
			os << "An unknown product";
			break;
		// just use unknown here since we do not care.
		case PRODUCT_UNLICENSED:
			os << "An unknown product";
			break;
		case PRODUCT_ULTIMATE:
			os << "Ultimate";
			break;
		case PRODUCT_ULTIMATE_E:
			//os << "Not supported";
			break;
		case PRODUCT_ULTIMATE_N:
			os << "Ultimate N";
			break;
		case PRODUCT_WEB_SERVER:
			os << "Web Server (full installation)";
			break;
		case PRODUCT_WEB_SERVER_CORE:
			os << "Web Server (core installation)";
			break;
		}

	} else if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 ) {

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
	} else if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 ) {

		os << "Windows XP ";
		if( osvi.wSuiteMask & VER_SUITE_PERSONAL )
			os <<  "Home Edition";
		else 
			os <<  "Professional";

	} else if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 ) {

		os << "Windows 2000 ";  
		
		if ( osvi.wProductType == VER_NT_WORKSTATION ) {
			os <<  "Professional";
		} else {
			if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
				os <<  "Datacenter Server";
			else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
				os <<  "Advanced Server";
			else 
				os <<  "Server";
		}
	} else {
		os << " Windows Version " << osvi.dwMajorVersion << "." << osvi.dwMinorVersion;
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

#endif // ARX_HAVE_WINAPI
