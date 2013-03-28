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

#include "crashreporter/Win32Utilities.h"

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
		fs::ifstream iss("/etc/lsb-release");
		const char * keys[] = {
			"DISTRIB_DESCRIPTION", "DISTRIB_ID", "DISTRIB_RELEASE", "(DISTRIB_CODENAME"
		};
		std::string distro = parseDistributionName(iss, '=', keys, ARRAY_SIZE(keys));
		if(!distro.empty()) {
			return distro;
		}
	}
	
	#endif // ARX_PLATFORM == ARX_PLATFORM_LINUX
	
	return std::string();
}


} // namespace platform
