/*
 * Copyright 2013 Arx Libertatis Team (see the AUTHORS file)
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

#include "platform/OS.h"

#include "Configure.h"

#include <sstream>
#include <vector>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/predicate.hpp>

#ifdef ARX_HAVE_WINAPI
#include <windows.h>
#include <cstring>
#endif

#ifdef ARX_HAVE_UNAME
#include <sys/utsname.h>
#endif

// yes, we need stdio.h, POSIX doesn't know about cstdio
#ifdef ARX_HAVE_POPEN
#include <stdio.h>
#endif

#include "io/fs/FilePath.h"
#include "io/fs/Filesystem.h"
#include "io/fs/FileStream.h"
#include "platform/Architecture.h"
#include "platform/Platform.h"
#include "util/String.h"

// windows specific functions
//#ifdef ARX_HAVE_WINAPI

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

typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
typedef BOOL (WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD);

static bool GetWindowsVersionName(char* str, int bufferSize)
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
	
	strncpy(str, os.str().c_str(), bufferSize);
	return true; 
}

static bool Is64BitWindows()
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

//#endif // ARX_HAVE_WINAPI

namespace platform {

	
std::string getOSName() {
	
	#ifdef ARX_HAVE_WINAPI
	// Get operating system friendly name from registry.
	char buffer[256];
	if(GetWindowsVersionName(buffer, ARRAY_SIZE(buffer))) {
		return buffer;
	}
	#endif
	
	#ifdef ARX_HAVE_UNAME
	struct utsname uname_buf;
	if(uname(&uname_buf) == 0) {
		return std::string(uname_buf.sysname) + ' ' + uname_buf.release;
	}
	#endif
	
	#if ARX_PLATFORM == ARX_PLATFORM_LINUX
	return "Linux";
	#elif ARX_PLATFORM == ARX_PLATFORM_WIN32
	return "Windows";
	#elif ARX_PLATFORM == ARX_PLATFORM_MACOSX
	return "Darwin"
	#elif ARX_PLATFORM == ARX_PLATFORM_BSD
	return "BSD";
	#elif ARX_PLATFORM == ARX_PLATFORM_UNIX
	return "UNIX";
	#endif
	
	return std::string();
}


std::string getOSArchitecture() {
	
	#ifdef ARX_HAVE_WINAPI
	// Determine if Windows is 64-bit.
	return Is64BitWindows() ? ARX_ARCH_NAME_X86_64 : ARX_ARCH_NAME_X86;
	#endif
	
	#ifdef ARX_HAVE_UNAME
	struct utsname uname_buf;
	if(uname(&uname_buf) == 0) {
		return uname_buf.machine;
	}
	#endif
	
	return std::string();
}


#if ARX_PLATFORM == ARX_PLATFORM_LINUX

#if defined(ARX_HAVE_POPEN) && defined(ARX_HAVE_PCLOSE)

static std::string getOutputOf(const char * command) {
	FILE * pipe = popen(command, "r");
	if(!pipe) {
		return std::string();
	}
	char buffer[1024];
	std::string result;
	while(!feof(pipe)) {
		if(size_t count = fread(buffer, 1, ARRAY_SIZE(buffer), pipe)) {
			result.append(buffer, count);
		}
	}
	pclose(pipe);
	return result;
}

#endif


/*!
 * Parse key-value pairs from /etc/os-release or `lsb_release -a` to form a
 * pretty distribution name.
 *
 * @param is        Input stream for the text to parse.
 * @param separator Character used to separate keys and values.
 * @param keys      Keys that should be used in the final name.
 *                  Values are added to the name in order of the keys listed here
 *                  unless the name so far already contains the value.
 *                  Prepend a a '(' character to surround in parentheses before adding it
 *                  to the name (unless the name so far is empty).
 * @param keyCount  Number of entries in the @c keys array.
 */
static std::string parseDistributionName(std::istream & is, const char separator,
                                         const char * keys[], size_t keyCount) {
	
	std::vector<std::string> values;
	values.resize(keyCount);
	
	std::string line;
	while(std::getline(is, line).good()) {
		
		// Ignore comments and empty lines
		if(line.empty() || line[0] == '#') {
			continue;
		}
		
		// Split key and value
		size_t pos = line.find(separator);
		if(pos == std::string::npos) {
			// Ignore bad syntax
			continue;
		}
		std::string key = line.substr(0, pos);
		boost::trim(key);
		std::string value = util::unescapeString(boost::trim_copy(line.substr(pos + 1)));
		
		// Ignore empty values
		if(key.empty() || value.empty() || value == "n/a") {
			continue;
		}
		
		for(size_t i = 0; i < keyCount; i++) {
			
			// Only use the first value for each key type
			if(!values[i].empty()) {
				continue;
			}
			
			const char * kkey = keys[i];
			if(*kkey == '(') {
				kkey++;
			}
			if(key == kkey) {
				values[i] = value;
				break;
			}
		}
		
	}
	
	std::string name;
	for(size_t i = 0; i < keyCount; i++) {
		
		// Skip missing keys
		if(values[i].empty()) {
			continue;
		}
		
		// Skip values that are already part of the name
		if(boost::icontains(name, values[i])) {
			continue;
		}
		
		// Add the new value to the name
		if(name.empty()) {
			name = values[i];
		} else if(*keys[i] == '(') {
			name.push_back(' ');
			name.push_back('(');
			name += values[i];
			name.push_back(')');
		} else {
			name.push_back(' ');
			name += values[i];
		}
		
	}
	return name;
}

#endif // ARX_PLATFORM == ARX_PLATFORM_LINUX


std::string getOSDistribution() {
	
	#if ARX_PLATFORM == ARX_PLATFORM_LINUX
	
	// Get distribution information from SystemD's /etc/os-release
	// Spec: http://www.freedesktop.org/software/systemd/man/os-release.html
	{
		fs::ifstream ifs("/etc/os-release");
		if(ifs.is_open()) {
			const char * keys[] = { "PRETTY_NAME", "NAME", "VERSION", "VERSION_ID" };
			std::string distro = parseDistributionName(ifs, '=', keys, ARRAY_SIZE(keys));
			if(!distro.empty()) {
				return distro;
			}
		}
	}
	
	// Get distribution information from `lsb_release -a` output.
	// Don't parse /etc/lsb-release ourselves unless there is no other way
	// because lsb_release may have distro-specific patches
	#if defined(ARX_HAVE_POPEN) && defined(ARX_HAVE_PCLOSE)
	{
		std::istringstream iss(getOutputOf("lsb_release -a"));
		const char * keys[] = { "Description", "Distributor ID", "Release", "(Codename" };
		std::string distro = parseDistributionName(iss, ':', keys, ARRAY_SIZE(keys));
		if(!distro.empty()) {
			return distro;
		}
	}
	#endif
	
	// Fallback for older / non-LSB-compliant distros.
	// Release file list taken from http://linuxmafia.com/faq/Admin/release-files.html
	
	const char * release_files[] = {
		"/etc/annvix-release",
		"/etc/arch-release",
		"/etc/arklinux-release",
		"/etc/aurox-release",
		"/etc/blackcat-release",
		"/etc/cobalt-release",
		"/etc/conectiva-release",
		"/etc/fedora-release",
		"/etc/gentoo-release",
		"/etc/immunix-release",
		"/etc/lfs-release",
		"/etc/linuxppc-release",
		"/etc/mandriva-release",
		"/etc/mandrake-release",
		"/etc/mandakelinux-release",
		"/etc/mklinux-release",
		"/etc/nld-release",
		"/etc/pld-release",
		"/etc/slackware-release",
		"/etc/e-smith-release",
		"/etc/release",
		"/etc/sun-release",
		"/etc/SuSE-release",
		"/etc/novell-release",
		"/etc/sles-release",
		"/etc/tinysofa-release",
		"/etc/turbolinux-release",
		"/etc/ultrapenguin-release",
		"/etc/UnitedLinux-release",
		"/etc/va-release",
		"/etc/yellowdog-release",
		"/etc/debian_release",
		"/etc/redhat-release",
		"/etc/frugalware-release",
		"/etc/altlinux-release",
		"/etc/meego-release",
		"/etc/mageia-release",
		"/etc/system-release",
	};
	for(size_t i = 0; i < ARRAY_SIZE(release_files); i++) {
		std::string distro = fs::read(release_files[i]);
		boost::trim(distro);
		if(!distro.empty()) {
			return distro;
		}
	}
	
	const char * version_files[][2] = {
		{ "/etc/debian_version", "Debian " },
		{ "/etc/knoppix_version", "Knoppix " },
		{ "/etc/redhat_version", "RedHat " },
		{ "/etc/slackware-version", "Slackware " },
		{ "/etc/angstrom-version", "Ångström " },
	};
	for(size_t i = 0; i < ARRAY_SIZE(version_files); i++) {
		if(fs::exists(version_files[i][0])) {
			std::string distro = version_files[i][1] + fs::read(release_files[i]);
			boost::trim(distro);
			return distro;
		}
	}
	
	// Fallback: parse /etc/lsb-release ourselves
	{
		fs::ifstream ifs("/etc/lsb-release");
		if(ifs.is_open()) {
			const char * keys[] = {
				"DISTRIB_DESCRIPTION", "DISTRIB_ID", "DISTRIB_RELEASE", "(DISTRIB_CODENAME"
			};
			std::string distro = parseDistributionName(ifs, '=', keys, ARRAY_SIZE(keys));
			if(!distro.empty()) {
				return distro;
			}
		}
	}
	
	#endif // ARX_PLATFORM == ARX_PLATFORM_LINUX
	
	return std::string();
}


} // namespace platform
