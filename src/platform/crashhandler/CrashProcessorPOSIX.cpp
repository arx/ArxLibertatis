/*
 * Copyright 2015 Arx Libertatis Team (see the AUTHORS file)
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

#include "platform/crashhandler/CrashHandlerPOSIX.h"

#include <sstream>

#include <signal.h>


void CrashHandlerPOSIX::processCrashSignal() {
	
	std::ostringstream description;
	
	switch(m_pCrashInfo->signal) {
		
		#ifdef SIGILL
		case SIGILL: {
			description << "Illegal instruction";
			switch(m_pCrashInfo->code) {
				#ifdef ILL_ILLOPC
				case ILL_ILLOPC: description << ": illegal opcode"; break;
				#endif
				#ifdef ILL_ILLOPN
				case ILL_ILLOPN: description << ": illegal operand"; break;
				#endif
				#ifdef ILL_ADR
				case ILL_ADR: description << ": illegal addressing mode"); break;
				#endif
				#ifdef ILL_ILLTRP
				case ILL_ILLTRP: description << ": illegal trap"; break;
				#endif
				#ifdef ILL_PRVOPC
				case ILL_PRVOPC: description << ": privileged opcode"; break;
				#endif
				#ifdef ILL_PRVREG
				case ILL_PRVREG: description << ": privileged register"; break;
				#endif
				#ifdef ILL_COPROC
				case ILL_COPROC: description << ": coprocessor error"; break;
				#endif
				#ifdef ILL_BADSTK
				case ILL_BADSTK: description << ": internal stack error"; break;
				#endif
				default: break;
			}
			break;
		}
		#endif
		
		#ifdef SIGABRT
		case SIGABRT: {
			description << "Abnormal termination";
			break;
		}
		#endif
		
		#ifdef SIGBUS
		case SIGBUS: {
			description << "Bus error";
			switch(m_pCrashInfo->code) {
				#ifdef BUS_ADRALN
				case BUS_ADRALN: description << ": invalid address alignment"; break;
				#endif
				#ifdef BUS_ADRERR
				case BUS_ADRERR: description << ": non-existent physical address"; break;
				#endif
				#ifdef BUS_OBJERR
				case BUS_OBJERR: description << ": object specific hardware error"; break;
				#endif
				default: break;
			}
			break;
		}
		#endif
		
		#ifdef SIGFPE
		case SIGFPE: {
			description << "Floating-point error";
			switch(m_pCrashInfo->code) {
				#ifdef FPE_INTDIV
				case FPE_INTDIV: description << ": integer division by zero"; break;
				#endif
				#ifdef FPE_INTOVF
				case FPE_INTOVF: description << ": integer overflow"; break;
				#endif
				#ifdef FPE_FLTDIV
				case FPE_FLTDIV: description << ": floating point division by zero"; break;
				#endif
				#ifdef FPE_FLTOVF
				case FPE_FLTOVF: description << ": floating point overflow"; break;
				#endif
				#ifdef FPE_FLTUND
				case FPE_FLTUND: description << ": floating point underflow"; break;
				#endif
				#ifdef FPE_FLTRES
				case FPE_FLTRES: description << ": floating point inexact result"; break;
				#endif
				#ifdef FPE_FLTINV
				case FPE_FLTINV: description << ": invalid floating point operation"; break;
				#endif
				#ifdef FPE_FLTSUB
				case FPE_FLTSUB: description << ": subscript out of range"; break;
				#endif
				default: break;
			}
			break;
		}
		#endif
		
		#ifdef SIGSEGV
		case SIGSEGV: {
			description << "Illegal storage access";
			switch(m_pCrashInfo->code) {
				#ifdef SEGV_MAPERR
				case SEGV_MAPERR: {
					description << ": address not mapped to object";
					break;
				}
				#endif
				#ifdef SEGV_ACCERR
				case SEGV_ACCERR: {
					description << ": invalid permissions for mapped object";
					break;
				}
				#endif
				default: break;
			}
			break;
		}
		#endif
		
		default: {
			description << "Received signal " << m_pCrashInfo->signal;
			break;
		}
		
	}
	
	description << "\n\n";
	
	addText(description.str().c_str());
}
