/*
 * Copyright 2011-2013 Arx Libertatis Team (see the AUTHORS file)
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

#if ARX_PLATFORM == ARX_PLATFORM_WIN32

// Windows
#include <windows.h>
#include <string>
#include <sstream>
#include <psapi.h>
#include <dbghelp.h>

// Boost
#include <boost/crc.hpp>

#include "io/fs/FilePath.h"

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

#endif // ARX_PLATFORM == ARX_PLATFORM_WIN32
