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

#ifndef ARX_GRAPHICS_OPENGL_OPENGLUTIL_H
#define ARX_GRAPHICS_OPENGL_OPENGLUTIL_H

#include "platform/Platform.h"
#include "Configure.h"

#if ARX_HAVE_EPOXY

#include <epoxy/gl.h>

#define ARX_HAVE_GL_VER(x, y) (epoxy_gl_version() >= x##y)
#define ARX_HAVE_GL_EXT(name) epoxy_has_gl_extension("GL_" #name)

#elif ARX_HAVE_GLEW

#include <GL/glew.h>

#define ARX_HAVE_GL_VER(x, y) GLEW_VERSION_##x##_##y
#define ARX_HAVE_GL_EXT(name) GLEW_##name

#else
#error "OpenGL renderer not supported: need ARX_HAVE_EPOXY or ARX_HAVE_GLEW"
#endif

#endif // ARX_GRAPHICS_OPENGL_OPENGLUTIL_H
