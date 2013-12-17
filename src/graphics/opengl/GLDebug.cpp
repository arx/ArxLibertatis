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

#include <GL/glew.h>

#include "io/log/Logger.h"

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

// GLEW undefines GLAPIENTRY after using it
#if ARX_PLATFORM == ARX_PLATFORM_WIN32
	#define ARX_GLAPIENTRY __stdcall
#else
	#define ARX_GLAPIENTRY
#endif

void ARX_GLAPIENTRY callback(GLenum source,
                             GLenum type,
                             GLuint id,
                             GLenum severity,
                             GLsizei length,
                             const GLchar *message,
                             void *userParam
) {
	ARX_UNUSED(length);
	ARX_UNUSED(userParam);
	
	std::ostringstream buffer;
	
	buffer << sourceToString(source) << " " << typeToString(type) << " #"<< id << ": " << message;
	
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
	
	if(!GLEW_ARB_debug_output) {
		LogWarning << "OpenGL debug output disabled";
		return;
	}
	
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
	glDebugMessageCallbackARB(gldebug::callback, NULL);
	
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

}

