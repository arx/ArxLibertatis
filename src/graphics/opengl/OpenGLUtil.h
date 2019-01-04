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

#ifndef ARX_GRAPHICS_OPENGL_OPENGLUTIL_H
#define ARX_GRAPHICS_OPENGL_OPENGLUTIL_H

#include "platform/Platform.h"
#include "Configure.h"

#if ARX_PLATFORM == ARX_PLATFORM_WIN32
// Make sure we get the APIENTRY define from windows.h first to avoid a re-definition warning
#include <windows.h>
#endif

#if ARX_HAVE_EPOXY

#include <epoxy/gl.h>

#define ARX_HAVE_GL_VER(x, y) (epoxy_gl_version() >= x##y)
#define ARX_HAVE_GL_EXT(name) epoxy_has_gl_extension("GL_" #name)
#define ARX_HAVE_GLES_VER(x, y) ARX_HAVE_GL_VER(x, y)
#define ARX_HAVE_GLES_EXT(name) ARX_HAVE_GL_EXT(name)

#elif ARX_HAVE_GLEW

#include <GL/glew.h>

#define ARX_HAVE_GL_VER(x, y) (glewIsSupported("GL_VERSION_" #x "_" #y) != 0)
#define ARX_HAVE_GL_EXT(name) (glewIsSupported("GL_" #name) != 0)
#define ARX_HAVE_GLES_VER(x, y) (false)
#define ARX_HAVE_GLES_EXT(name) (false)

#else
#error "OpenGL renderer not supported: need ARX_HAVE_EPOXY or ARX_HAVE_GLEW"
#endif

#endif // ARX_GRAPHICS_OPENGL_OPENGLUTIL_H
