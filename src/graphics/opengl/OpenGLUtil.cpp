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
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>

#include "core/Config.h"
#include "io/log/Logger.h"
#include "platform/ProgramOptions.h"


static std::string g_glExtensionOverride;

static void setGlOverride(const std::string & string) {
	g_glExtensionOverride = string;
}

ARX_PROGRAM_OPTION_ARG("override-gl", NULL, "Override OpenGL version and extensions", &setGlOverride, "OVERRIDES")

OpenGLInfo::OpenGLInfo()
	: m_versionString("")
	, m_vendor("")
	, m_renderer("")
	, m_isES(false)
	, m_version(0)
	, m_versionOverride(std::numeric_limits<s32>::max())
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
	
	// Some older OpenGL implementations incorrectly claim support for GL_ARB_texture_non_power_of_two
	if(!isES() && !is(3, 0)) {
		GLint max = 0;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
		if(max < 8192) {
			m_extensionOverrides.push_back("-GL_ARB_texture_non_power_of_two");
		}
	}
	
	// Some versions of Intel's ig7icd32.dll/ig7icd64.dll Windows drivers crash when using per-sample shading
	// See bug https://arx.vg/1152 and duplicates (2018-2021)
	// Confirmed by users in http://arx.vg/1250 and https://arx.vg/1532 to be triggered by Crisp Alpha Cutout AA
	// with device "Intel(R) HD Graphics 4000" and versions "4.0.0 - Build 10.18.10.4276" and …".4252".
	// Other build numbers in similar-looking but unconfirmed crashes are 4358, 4653, 4885, 5059, 5069, 5129
	// and 5146 and for some of them the renderer does not have the " 4000" suffix.
	//
	// Note that there are also (other) crashes (most on shutdown) with matching driver versions seen in
	// http://arx.vg/645 and duplicates, before Crisp Alpha Cutout AA was added so other functionality may
	// also be buggy with this driver.
	#if ARX_PLATFORM == ARX_PLATFORM_WIN32
	if(!isES() && boost::equals(vendor(), "Intel")
	   && (boost::equals(renderer(), "Intel(R) HD Graphics")
	       || boost::equals(renderer(), "Intel(R) HD Graphics 4000"))
	   && boost::contains(versionString(), "Build 10.18.10.")) {
		m_extensionOverrides.push_back("-GL_ARB_sample_shading");
	}
	#endif
	
	parseOverrideConfig(config.video.extensionOverride);
	
	parseOverrideConfig(g_glExtensionOverride);
	
}

void OpenGLInfo::parseOverrideConfig(const std::string & string) {
	boost::char_separator<char> separator(" \t\r\n,;:");
	boost::tokenizer< boost::char_separator<char> > tokens(string, separator);
	bool first = true;
	BOOST_FOREACH(const std::string & token, tokens) {
		if(boost::starts_with(token, "+GL_") || boost::starts_with(token, "-GL_")) {
			m_extensionOverrides.push_back(token);
			first = false;
		} else {
			try {
				size_t offset = boost::starts_with(token, "GL") ? 2 : 0;
				size_t dot = token.find('.', offset);
				if(token == "+*" || token == "+") {
					m_versionOverride = std::numeric_limits<s32>::max();
				} else if(token == "-*" || token == "-") {
					m_versionOverride = m_version;
				} else if(dot != std::string::npos) {
					u32 major = boost::lexical_cast<u32>(token.substr(offset, dot - offset));
					u32 minor = boost::lexical_cast<u32>(token.substr(dot + 1));
					if(minor > 10) {
						throw 42;
					}
					m_versionOverride = major * 10 + minor;
				} else if(token.length() - offset > 1) {
					m_versionOverride = boost::lexical_cast<u32>(token.substr(offset));
				} else {
					m_versionOverride = boost::lexical_cast<u32>(token.substr(offset)) * 10;
				}
				if(!first) {
					LogWarning << "Ignoring OpenGL feature overrides before '" << token << "'";
				}
				m_extensionOverrides.clear();
				first = false;
			} catch(...) {
				LogWarning << "Invalid OpenGL feature override '" << token << "'";
			}
		}
	}
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
	
	BOOST_REVERSE_FOREACH(const std::string & override, m_extensionOverrides) {
		arx_assert(!override.empty());
		if(override.compare(1, override.size() - 1, extension) == 0) {
			if(override[0] == '+') {
				return true;
			} else {
				LogInfo << "Ignoring OpenGL extension " << extension;
				return false;
			}
		}
	}
	
	if(m_versionOverride >= version) {
		return true;
	} else {
		LogInfo << "Ignoring OpenGL extension " << extension;
		return false;
	}
	
}
