/*
 * Copyright 2013-2017 Arx Libertatis Team (see the AUTHORS file)
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

#include "graphics/opengl/GLDebug.h"

#include <cstring>

#include "core/Benchmark.h"
#include "graphics/opengl/OpenGLUtil.h"
#include "io/log/Logger.h"
#include "platform/ProgramOptions.h"
#include "util/cmdline/Optional.h"

#include "Configure.h"

namespace gldebug {

#ifdef GL_KHR_debug

static const char * sourceToString(GLenum source) {
	
	switch(source) {
		case GL_DEBUG_SOURCE_API:             return "API";
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   return "Window";
		case GL_DEBUG_SOURCE_SHADER_COMPILER: return "Shader";
		case GL_DEBUG_SOURCE_THIRD_PARTY:     return "Third-party";
		case GL_DEBUG_SOURCE_APPLICATION:     return "Application";
		case GL_DEBUG_SOURCE_OTHER:           return "Other";
		default:                                  return "(unknown)";
	}
}

static const char * typeToString(GLenum type) {
	
	switch(type) {
		case GL_DEBUG_TYPE_ERROR:               return "error";
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "deprecated";
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  return "undefined";
		case GL_DEBUG_TYPE_PORTABILITY:         return "portability";
		case GL_DEBUG_TYPE_PERFORMANCE:         return "performance";
		case GL_DEBUG_TYPE_OTHER:               return "info";
		default:                                return "(unknown)";
	}
}

// GLEW versions before 1.11.0 undefine GLAPIENTRY after using it - fix that
#if !ARX_HAVE_GLEW || defined(GLEW_VERSION_4_5)
	#define ARX_GLAPIENTRY GLAPIENTRY // GLEW 1.11.0 or newer
#elif ARX_PLATFORM == ARX_PLATFORM_WIN32
	#define ARX_GLAPIENTRY __stdcall
#else
	#define ARX_GLAPIENTRY
#endif

static void ARX_GLAPIENTRY callback(GLenum source, GLenum type, GLuint id,
                                    GLenum severity, GLsizei length,
                                    const GLchar * message, const void * userParam) {
	
	ARX_UNUSED(length);
	ARX_UNUSED(userParam);
	
	std::ostringstream buffer;
	
	const GLchar * end = message + std::strlen(message);
	while(end != message && (*(end - 1) == '\r' || *(end - 1) == '\n')) {
		end--;
	}
	
	buffer << sourceToString(source) << " " << typeToString(type) << " #"<< id << ": ";
	buffer.write(message, end - message);
	
	switch(severity) {
	case GL_DEBUG_SEVERITY_HIGH:
		LogError << buffer.str();
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		LogWarning << buffer.str();
		break;
	case GL_DEBUG_SEVERITY_LOW:
		LogInfo << buffer.str();
		break;
	default:
		break;
	}
}

void initialize() {
	
	if(!isEnabled()) {
		return;
	}
	
	bool have_debug = ARX_HAVE_GL_VER(4, 3) || ARX_HAVE_GL_EXT(KHR_debug);
	#if ARX_HAVE_EPOXY
	have_debug = have_debug || (epoxy_is_desktop_gl() && ARX_HAVE_GL_EXT(ARB_debug_output));
	#endif
	if(!have_debug) {
		LogWarning << "OpenGL debug output not available";
		return;
	}
	
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	
	// GLEW versions before 1.11.0 define GLDEBUGPROC with a non-const user pointer
	#if !ARX_HAVE_GLEW || defined(GLEW_VERSION_4_5)
	glDebugMessageCallback(gldebug::callback, NULL);
	#else
	glDebugMessageCallback((GLDEBUGPROC)gldebug::callback, NULL);
	#endif
	
	// Forward messages with high severity level
	glDebugMessageControl(GL_DONT_CARE,
	                      GL_DONT_CARE,
	                      GL_DEBUG_SEVERITY_HIGH,
	                      0,
	                      NULL,
	                      GL_TRUE);
	
	// Forward messages with medium severity level
	glDebugMessageControl(GL_DONT_CARE,
	                      GL_DONT_CARE,
	                      GL_DEBUG_SEVERITY_MEDIUM,
	                      0,
	                      NULL,
	                      GL_TRUE);
	
	// Forward messages from the application
	glDebugMessageControl(GL_DEBUG_SOURCE_APPLICATION,
	                      GL_DONT_CARE,
	                      GL_DONT_CARE,
	                      0,
	                      NULL,
	                      GL_TRUE);
	
	
	std::string strInitialized("OpenGL debug output enabled");
	glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION,
	                     GL_DEBUG_TYPE_OTHER,
	                     1,
	                     GL_DEBUG_SEVERITY_LOW,
	                     GLsizei(strInitialized.size()), strInitialized.c_str());
}

#else // GL_KHR_debug

void initialize() {
	LogWarning << "OpenGL debug output not supported in this build";
}

#endif

#if ARX_DEBUG_GL
static bool g_enable = true;
#else
static bool g_enable = false;
#endif

bool isEnabled() {
	return g_enable && !benchmark::isEnabled();
}

static void enable() {
	g_enable = true;
}

ARX_PROGRAM_OPTION("debug-gl", NULL, "Enable OpenGL debug output", &enable)

static const char * getGLErrorString(GLenum error) {
	
	switch(error) {
		case GL_NO_ERROR:          return "GL_NO_ERROR";
		case GL_INVALID_ENUM:      return "GL_INVALID_ENUM";
		case GL_INVALID_VALUE:     return "GL_INVALID_VALUE";
		case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
		case GL_STACK_OVERFLOW:    return "GL_STACK_OVERFLOW";
		case GL_STACK_UNDERFLOW:   return "GL_STACK_UNDERFLOW";
		case GL_OUT_OF_MEMORY:     return "GL_OUT_OF_MEMORY";
		default:                   return "(unknown error)";
	}
}

void endFrame() {
	
	if(!isEnabled())
		return;
	
	if(GLenum error = glGetError()) {
		LogError << "GL error: " << error << " = " << getGLErrorString(error);
	}
}

} // namespace gldebug

