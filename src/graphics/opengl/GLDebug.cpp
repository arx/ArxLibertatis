/*
 * Copyright 2013 Arx Libertatis Team (see the AUTHORS file)
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

#include <GL/glew.h>

#include "io/log/Logger.h"

#include "platform/ProgramOptions.h"

#include "util/cmdline/Optional.h"

#include "Configure.h"

namespace gldebug {

#if defined(GL_ARB_debug_output)

static const char * sourceToString(GLenum source) {
	
	switch(source) {
		case GL_DEBUG_SOURCE_API_ARB:             return "API";
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:   return "Window";
		case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB: return "Shader";
		case GL_DEBUG_SOURCE_THIRD_PARTY_ARB:     return "Third-party";
		case GL_DEBUG_SOURCE_APPLICATION_ARB:     return "Application";
		case GL_DEBUG_SOURCE_OTHER_ARB:           return "Other";
		default:                                  return "(unknown)";
	}
}

static const char * typeToString(GLenum type) {
	
	switch(type) {
		case GL_DEBUG_TYPE_ERROR_ARB:               return "error";
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB: return "deprecated";
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:  return "undefined";
		case GL_DEBUG_TYPE_PORTABILITY_ARB:         return "portability";
		case GL_DEBUG_TYPE_PERFORMANCE_ARB:         return "performance";
		case GL_DEBUG_TYPE_OTHER_ARB:               return "info";
		default:                                    return "(unknown)";
	}
}

// GLEW versions before 1.11.0 undefine GLAPIENTRY after using it - fix that
#if defined(GLEW_VERSION_4_5)
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
	case GL_DEBUG_SEVERITY_HIGH_ARB:
		LogError << buffer.str();
		break;
	case GL_DEBUG_SEVERITY_MEDIUM_ARB:
		LogWarning << buffer.str();
		break;
	case GL_DEBUG_SEVERITY_LOW_ARB:
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
	
	if(!GLEW_ARB_debug_output) {
		LogWarning << "OpenGL debug output disabled";
		return;
	}
	
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
	
	// GLEW versions before 1.11.0 define GLDEBUGPROCARB with a non-const user pointer
	#if defined(GLEW_VERSION_4_5)
	glDebugMessageCallbackARB(gldebug::callback, NULL);
	#else
	glDebugMessageCallbackARB((GLDEBUGPROCARB)gldebug::callback, NULL);
	#endif
	
	// Forward messages with high severity level
	glDebugMessageControlARB(GL_DONT_CARE,
	                         GL_DONT_CARE,
	                         GL_DEBUG_SEVERITY_HIGH_ARB,
	                         0,
	                         NULL,
	                         GL_TRUE);
	
	// Forward messages with medium severity level
	glDebugMessageControlARB(GL_DONT_CARE,
	                         GL_DONT_CARE,
	                         GL_DEBUG_SEVERITY_MEDIUM_ARB,
	                         0,
	                         NULL,
	                         GL_TRUE);
	
	// Forward messages from the application
	glDebugMessageControlARB(GL_DEBUG_SOURCE_APPLICATION_ARB,
	                         GL_DONT_CARE,
	                         GL_DONT_CARE,
	                         0,
	                         NULL,
	                         GL_TRUE);
	
	
	std::string strInitialized("OpenGL debug output enabled");
	glDebugMessageInsertARB(GL_DEBUG_SOURCE_APPLICATION_ARB,
	                        GL_DEBUG_TYPE_OTHER_ARB,
	                        1,
	                        GL_DEBUG_SEVERITY_LOW_ARB,
	                        GLsizei(strInitialized.size()), strInitialized.c_str());
}

#else // !defined(GL_ARB_debug_output)

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
	return g_enable;
}

static void enable() {
	g_enable = true;
}

ARX_PROGRAM_OPTION("debug-gl", NULL, "Enable OpenGL debug output", &enable);

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

