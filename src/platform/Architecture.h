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

#ifndef ARX_PLATFORM_ARCHITECTURE_H
#define ARX_PLATFORM_ARCHITECTURE_H

namespace platform {

#define ARX_ARCH_UNKNOWN          0
#define ARX_ARCH_X86_64           1
#define ARX_ARCH_IA64             2
#define ARX_ARCH_X86              3
#define ARX_ARCH_ARM              4
#define ARX_ARCH_ALPHA            5
#define ARX_ARCH_M68K             6
#define ARX_ARCH_MIPS             7
#define ARX_ARCH_POWERPC          8
#define ARX_ARCH_SPARC            9

#define ARX_ARCH_NAME_UNKNOWN     ""
#define ARX_ARCH_NAME_X86_64      "x86_64"
#define ARX_ARCH_NAME_IA64        "ia64"
#define ARX_ARCH_NAME_X86         "x86"
#define ARX_ARCH_NAME_ARM         "arm"
#define ARX_ARCH_NAME_ALPHA       "alpha"
#define ARX_ARCH_NAME_M68K        "m68k"
#define ARX_ARCH_NAME_MIPS        "mips"
#define ARX_ARCH_NAME_POWERPC     "powerpc"
#define ARX_ARCH_NAME_SPARC       "sparc"

// Checks are shamelessly copied from
// https://sourceforge.net/apps/mediawiki/predef/index.php?title=Architectures

#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) \
      || defined(__x86_64) || defined(_M_X64)
#define ARX_ARCH ARX_ARCH_X86_64
#define ARX_ARCH_NAME ARX_ARCH_NAME_X86_64

#elif defined(__ia64__) || defined(_IA64) || defined(__IA64__) || defined(__ia64) \
      || defined(_M_IA64) || defined(__itanium__)
#define ARX_ARCH ARX_ARCH_IA64
#define ARX_ARCH_NAME ARX_ARCH_NAME_IA64

#elif defined(i386) || defined(__i386__) || defined(__i386) || defined(__IA32__) \
      || defined(_M_IX86) || defined(__X86__) || defined(_X86_) \
      || defined(__THW_INTEL__) || defined(__I86__) || defined(__INTEL__)
#define ARX_ARCH ARX_ARCH_X86
#define ARX_ARCH_NAME ARX_ARCH_NAME_X86

#elif defined(__arm__) || defined(__thumb__) || defined(__TARGET_ARCH_ARM) \
      || defined(__TARGET_ARCH_THUMB) || defined(_ARM)
#define ARX_ARCH ARX_ARCH_ARM
#define ARX_ARCH_NAME ARX_ARCH_NAME_ARM

#elif defined(__alpha__) || defined(__alpha) || defined(_M_ALPHA)
#define ARX_ARCH ARX_ARCH_ALPHA
#define ARX_ARCH_NAME ARX_ARCH_NAME_ALPHA

#elif defined(__m68k__) || defined(M68000) || defined(__MC68K__)
#define ARX_ARCH ARX_ARCH_M68K
#define ARX_ARCH_NAME ARX_ARCH_NAME_M68K

#elif defined(__mips__) || defined(mips) || defined(__mips) || defined(__MIPS__)
#define ARX_ARCH ARX_ARCH_MIPS
#define ARX_ARCH_NAME ARX_ARCH_NAME_MIPS

#elif defined(__powerpc) || defined(__powerpc__) || defined(__POWERPC__) \
      ||defined(__ppc__) || defined(_M_PPC) || defined(_ARCH_PPC)
#define ARX_ARCH ARX_ARCH_POWERPC
#define ARX_ARCH_NAME ARX_ARCH_NAME_POWERPC

#elif defined(__sparc__) || defined(__sparc)
#define ARX_ARCH ARX_ARCH_SPARC
#define ARX_ARCH_NAME ARX_ARCH_NAME_SPARC

#else
#define ARX_ARCH ARX_ARCH_UNKNOWN
#define ARX_ARCH_NAME ARX_ARCH_NAME_UNKNOWN

#endif

inline const char * getArchitectureName(unsigned arch) {
	switch(arch) {
		case ARX_ARCH_X86_64:  return ARX_ARCH_NAME_X86_64;
		case ARX_ARCH_IA64:    return ARX_ARCH_NAME_IA64;
		case ARX_ARCH_X86:     return ARX_ARCH_NAME_X86;
		case ARX_ARCH_ARM:     return ARX_ARCH_NAME_ARM;
		case ARX_ARCH_ALPHA:   return ARX_ARCH_NAME_ALPHA;
		case ARX_ARCH_M68K:    return ARX_ARCH_NAME_M68K;
		case ARX_ARCH_MIPS:    return ARX_ARCH_NAME_MIPS;
		case ARX_ARCH_POWERPC: return ARX_ARCH_NAME_POWERPC;
		case ARX_ARCH_SPARC:   return ARX_ARCH_NAME_SPARC;
		default: return ARX_ARCH_NAME_UNKNOWN;
	}
}

} // namespace platform

#endif // ARX_PLATFORM_ARCHITECTURE_H
