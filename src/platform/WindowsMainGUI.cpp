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

#include "platform/WindowsMain.h"

#include <windows.h>

#include "platform/WindowsMainCommon.h"


#if ARX_COMPILER_MSVC
#define ARX_EXPORT_SYMBOL __declspec(dllexport)
#else
#define ARX_EXPORT_SYMBOL __attribute__((dllexport))
#endif

// Request to use the more powerful GPU in multi-GPU setups
extern "C" {
	ARX_EXPORT_SYMBOL DWORD NvOptimusEnablement = 1;
	ARX_EXPORT_SYMBOL int AmdPowerXpressRequestHighPerformance = 1;
}

INT WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PSTR lpCmdLine,
                   _In_ INT nCmdShow) {
	ARX_UNUSED(hInstance);
	ARX_UNUSED(hPrevInstance);
	ARX_UNUSED(lpCmdLine);
	ARX_UNUSED(nCmdShow);
	
	WindowsMain init;
	
	return utf8_main(init.argc, init.argv);
}
