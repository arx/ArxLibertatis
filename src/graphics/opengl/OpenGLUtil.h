/*
 * Copyright 2015-2019 Arx Libertatis Team (see the AUTHORS file)
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

#include <algorithm>
#include <limits>
#include <string>
#include <vector>

#include "platform/Platform.h"
#include "Configure.h"

#if ARX_PLATFORM == ARX_PLATFORM_WIN32
// Make sure we get the APIENTRY define from windows.h first to avoid a re-definition warning
#include <windows.h>
#endif

#if ARX_HAVE_EPOXY
#include <epoxy/gl.h>
#elif ARX_HAVE_GLEW
#include <GL/glew.h>
#else
#error "OpenGL renderer not supported: need ARX_HAVE_EPOXY or ARX_HAVE_GLEW"
#endif

class OpenGLInfo {
	
	const char * m_versionString;
	const char * m_vendor;
	const char * m_renderer;
	bool m_isES;
	u32 m_version;
	u32 m_versionOverride;
	std::vector<std::string> m_extensionOverrides;
	
	bool has(const char * extension, u32 version) const;
	
	/*!
	 * \brief Override OpenGL version and extension support
	 *
	 * \param string List of override tokens separated by whitespace, commas, semicolons or colons
	 *
	 * Supported string tokens are
	 *  "-GL_ext"        Disable OpenGL extension GL_ext
	 *  "+GL_ext"        Re-enable OpenGL extension GL_ext if it was disabled by a previous token
	 *  "GLx.y" or "x.y" Re-enable extensions part of OpenGL version x.y and disabling all others
	 *  "GLxy" or "xy"   Re-enable extensions part of OpenGL version x.y and disabling all others
	 *  "GLx" or "x"     Re-enable extensions part of OpenGL version x.0 and disabling all others
	 *  "+*" or "+"      Re-enable all non-core OpenGL extensions
	 *  "-*" or "-"      Disable all non-core OpenGL extensions
	 *
	 * Later tokens override previous ones.
	 * This means that extension tokens before version tokens are ignored.
	 *
	 * Can be called multiple times to merge overrides from multiple sources.
	 */
	void parseOverrideConfig(const std::string & string);
	
public:
	
	OpenGLInfo();
	
	//! \return the OpenGL version string with any "OpenGL ES*" prefix removed"
	const char * versionString() { return m_versionString; }
	
	//! \return the OpenGL vendor string
	const char * vendor() { return m_vendor; }
	
	//! \return the OpenGL renderer string
	const char * renderer() { return m_renderer; }
	
	//! \return true if the corrent context type is OpenGL ES
	bool isES() const { return m_isES; }
	
	//! \return ture if the context version is at least major.minor
	bool is(u32 major, u32 minor) const {
		arx_assert(minor < 10);
		return std::min(m_version, m_versionOverride) >= (major * 10 + minor);
	}
	
	/*!
	 * \brief Check support for a non-core OpenGL extension
	 *
	 * \param extension Name of the extension to check support for
	 *
	 * \return true if the extension can be used.
	 */
	bool has(const char * extension) const { return has(extension, std::numeric_limits<s32>::max()); }
	
	/*!
	 * \brief Check support for a core OpenGL extension
	 *
	 * \param extension Name of the extension to check support for
	 * \param major     Minimum major OpenGL version that implies support for this extension
	 * \param minor     Minimum minor OpenGL version that implies support for this extension
	 *
	 * \return true if the extension can be used.
	 */
	bool has(const char * extension, u32 major, u32 minor) const {
		arx_assert(minor < 10);
		return has(extension, major * 10 + minor);
	}
	
};

#endif // ARX_GRAPHICS_OPENGL_OPENGLUTIL_H
