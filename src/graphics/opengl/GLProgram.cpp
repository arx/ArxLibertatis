/*
 * Copyright 2014 Arx Libertatis Team (see the AUTHORS file)
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

#include "graphics/opengl/GLProgram.h"

#include <boost/algorithm/string.hpp>

#include "io/log/Logger.h"
#include "io/resource/PakReader.h"

static void loadShader(GLuint program, const res::path & shaderPath) {
	
	PakFile *file = g_resources->getFile(shaderPath);
	
	if(!file) {
		LogError << "Shader file does not exist " << shaderPath;
		return;
	}
	
	GLenum shaderType;
	
	if(boost::algorithm::ends_with(shaderPath.basename(), "vert")) {
		shaderType = GL_VERTEX_SHADER;
	} else if(boost::algorithm::ends_with(shaderPath.basename(), "frag")) {
		shaderType = GL_FRAGMENT_SHADER;
	} else {
		LogWarning << "Unknown shader type";
		return;
	}
	
	GLuint shader = glCreateShader(shaderType);
	if(!shader) {
		LogWarning << "Failed to create shader";
		return;
	}
	
	size_t fileSize = file->size();
	char * source = new char[fileSize + 1];
	source[fileSize] = '\0';
	
	file->read(source);
			
	const char * src = (const char *)source;
	glShaderSource(shader, 1, &src, NULL);
	glCompileShader(shader);
	
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if(!status) {
		char msg[1024];
		glGetShaderInfoLog(shader, ARRAY_SIZE(msg), NULL, msg);
		LogWarning << "Failed to compile shader: " << msg << " " << shaderPath;
		LogWarning << source;
		
		glDeleteShader(shader);
		delete[] source;
		return;
	}
	
	glAttachShader(program, shader);
	
	//delete[] source;
}

GLProgram::GLProgram() {
	
	GLuint program = glCreateProgram();
	if(!program) {
		LogWarning << "Failed to create program object";
		return;
	}
	
	loadShader(program, "shaders/nop.postprocess.vert.glsl");
	loadShader(program, "shaders/nop.postprocess.frag.glsl");
	
	glLinkProgram(program);
	
	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if(!status) {
		char msg[1024];
		glGetProgramInfoLog(program, ARRAY_SIZE(msg), NULL, msg);
		LogWarning << "Failed to link shader: " << msg;
		return;
	}
	
	m_program = program;
	m_isValid = true;
}

GLProgram::~GLProgram() {
	glDeleteProgram(m_program);
}

GLint GLProgram::getAttribute(const char * name) {
	return glGetAttribLocation(m_program, name);
}

GLint GLProgram::getUniform(const char * name) {
	return glGetUniformLocation(m_program, name);
}

void GLProgram::use() {
	glUseProgram(m_program);
}
