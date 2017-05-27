/*
 * Copyright 2017 Arx Libertatis Team (see the AUTHORS file)
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

#include "window/SDL2X11Util.h"

// These functions are compiled separately to avoid namespace pollution by X11 headers

#include <SDL_syswm.h>

uint64_t SDL2X11_getNativeWindowHandle(SDL_Window * window) {
	
	SDL_SysWMinfo info;
	
	SDL_VERSION(&info.version);
	if(!SDL_GetWindowWMInfo(window, &info) || info.subsystem != SDL_SYSWM_X11) {
		return 0;
	}
	
	return info.info.x11.window;
}
