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

#include "utilities_win32.h"

#ifdef HAVE_WINAPI

#include <windows.h>
#include <string>
#include <sstream>
#include <Psapi.h>

typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
typedef BOOL (WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD);
#define PRODUCT_PROFESSIONAL	0x00000030
#define VER_SUITE_WH_SERVER	0x00008000


struct FindWindowData
{
	DWORD   dwProcessId;
	HWND	hWndMain;
};

BOOL CALLBACK EnumWndProc(HWND hWnd, LPARAM lParam)
{
	FindWindowData* pFWD = (FindWindowData*)lParam;

	// Get process ID
	if(IsWindowVisible(hWnd)) // Get only wisible windows
	{
		// Determine the process ID of the current window
		DWORD dwProcessId = 0;
		GetWindowThreadProcessId(hWnd, &dwProcessId);

		// Compare window process ID to our process ID
		if(dwProcessId == pFWD->dwProcessId)
		{	 
			DWORD dwStyle = GetWindowLong(hWnd, GWL_STYLE);
			if((dwStyle&WS_CHILD)==0) // Get only non-child windows
			{
				DWORD dwExStyle = GetWindowLong(hWnd, GWL_EXSTYLE);

				if((dwExStyle & WS_EX_APPWINDOW) &&
				   ((dwStyle & (WS_CAPTION|WS_SYSMENU)) ||		// Windowed
					(dwStyle & (WS_POPUP|WS_VISIBLE))			// Fullscreen
				   )
				  )		
				{
					pFWD->hWndMain = hWnd;
					return FALSE;
				}
			}
		}
	}

	return TRUE;
}

HWND GetMainWindow(unsigned int processId)
{
	FindWindowData fwd;
	fwd.dwProcessId = processId;
	fwd.hWndMain = 0;
	EnumWindows(EnumWndProc, (LPARAM)&fwd);

	return fwd.hWndMain;
}

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

#endif // HAVE_WINAPI
