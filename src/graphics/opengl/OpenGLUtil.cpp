/*
 * Copyright 2021 Arx Libertatis Team (see the AUTHORS file)
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

#include "graphics/opengl/OpenGLUtil.h"

#include <cstring>

#include <boost/algorithm/string/predicate.hpp>


OpenGLInfo::OpenGLInfo()
	: m_versionString("")
	, m_vendor("")
	, m_renderer("")
	, m_isES(false)
	, m_version(0)
{
	#if ARX_HAVE_EPOXY
	m_isES = !epoxy_is_desktop_gl();
	m_version = epoxy_gl_version();
	#elif ARX_HAVE_GLEW
	if(glewIsSupported("GL_VERSION_4_4")) {
		m_version = 44;
	} else if(glewIsSupported("GL_VERSION_4_3")) {
		m_version = 43;
	} else if(glewIsSupported("GL_VERSION_4_2")) {
		m_version = 42;
	} else if(glewIsSupported("GL_VERSION_4_1")) {
		m_version = 41;
	} else if(glewIsSupported("GL_VERSION_4_0")) {
		m_version = 40;
	} else if(glewIsSupported("GL_VERSION_3_2")) {
		m_version = 32;
	} else if(glewIsSupported("GL_VERSION_3_1")) {
		m_version = 31;
	} else if(glewIsSupported("GL_VERSION_3_0")) {
		m_version = 30;
	} else if(glewIsSupported("GL_VERSION_2_1")) {
		m_version = 21;
	} else if(glewIsSupported("GL_VERSION_2_0")) {
		m_version = 20;
	} else if(glewIsSupported("GL_VERSION_1_5")) {
		m_version = 15;
	} else if(glewIsSupported("GL_VERSION_1_4")) {
		m_version = 14;
	}
	#endif
	
	m_versionString = reinterpret_cast<const char *>(glGetString(GL_VERSION));
	const char * prefix = "OpenGL ";
	if(boost::starts_with(m_versionString, prefix)) {
		m_versionString += std::strlen(prefix);
	}
	m_vendor = reinterpret_cast<const char *>(glGetString(GL_VENDOR));
	m_renderer = reinterpret_cast<const char *>(glGetString(GL_RENDERER));
	
}

bool OpenGLInfo::has(const char * extension, u32 version) const {
	
	if(m_version < version) {
		#if ARX_HAVE_EPOXY
		bool supported = epoxy_has_gl_extension(extension);
		#elif ARX_HAVE_GLEW
		bool supported = glewIsSupported(extension);
		#endif
		if(!supported) {
			return false;
		}
	}
	
	return true;
}
