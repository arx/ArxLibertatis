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

#define ARX_STATIC_ASSERT(Cond, Msg) typedef char static_assertion_ ## Msg[(Cond) ? 1 : -1]

uint64_t SDL2X11_getNativeWindowHandle(SDL_Window * window) {
	
	/*
	 * The size of the SDL_SysWMinfo structure depends on the build-time configuration of SDL.
	 * If Arx is built with a SDL install that was configured without Wayland, but is run on
	 * Wayland (with a Wayland-enabled SDL), the SDL_GetWindowWMInfo call will write past the
	 * end of the SDL_SysWMinfo struct.
	 * https://bugzilla.libsdl.org/show_bug.cgi?id=3428 - fixed in SDL 2.0.6
	 */
	struct SysWMinfo {
		SDL_SysWMinfo data;
		char padding[1024];
	} info;
	
	/* Need to increase padding if this fails (also in ARX_SDL_SysWMinfo) */
	ARX_STATIC_ASSERT(sizeof(info.data) <= sizeof(info.padding), padding_size);
	
	SDL_VERSION(&info.data.version);
	struct SysWMinfo * infoptr = &info;
	if(!SDL_GetWindowWMInfo(window, (SDL_SysWMinfo *)infoptr) || info.data.subsystem != SDL_SYSWM_X11) {
		return 0;
	}
	
	return info.data.info.x11.window;
}
