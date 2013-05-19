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

#include "graphics/opengl/GLPostProcess.h"

#include "graphics/opengl/OpenGLUtil.h"

#include "graphics/opengl/GLProgram.h"
#include "graphics/opengl/GLRenderTexture.h"
#include "graphics/opengl/GLVertexBuffer.h"

GLPostProcess::GLPostProcess(const Vec2s & size)
{
	m_texture = new GLRenderTexture(size);
	
	m_program = new GLProgram();
	attribute_v_coord_postproc = m_program->getAttribute("v_coord");
	uniform_fbo_texture = m_program->getUniform("fbo_texture");
	
	if(attribute_v_coord_postproc == -1) {
		LogWarning << "Could not bind attribute: v_coord";
	}
	
	if(uniform_fbo_texture == -1) {
		LogWarning << "Could not bind uniform: fbo_texture";
	}
	
	
	GLfloat fbo_vertices[] = {
		-1, -1,
		1, -1,
		-1,  1,
		1,  1,
	};
	
	glGenBuffers(1, &m_vertices);
	
	bindBuffer(m_vertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(fbo_vertices), fbo_vertices, GL_STATIC_DRAW);
	unbindBuffer(m_vertices);
}

GLPostProcess::~GLPostProcess() {
	glDeleteBuffers(1, &m_vertices);
}

void GLPostProcess::resize(const Vec2s & size) {
	m_texture->setSize(size);
}

Vec2s GLPostProcess::size()
{
	return m_texture->size();
}

void GLPostProcess::use() {
	if(!m_program->isValid())
		return;
	
	m_texture->attach();
}

void GLPostProcess::render(Rect viewport) {
	
	if(!m_program->isValid())
		return;
	
	GLuint oldProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*) &oldProgram);
	
	glDisable(GL_CULL_FACE);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, viewport.width(), viewport.height());
	glClearColor(0.0, 0.0, 0.0, 1.0);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_program->use();
	m_texture->bind();
	
	glUniform1i(uniform_fbo_texture, /*GL_TEXTURE*/0);
	
	glEnableVertexAttribArray(attribute_v_coord_postproc);
	bindBuffer(m_vertices);
	glVertexAttribPointer(
				attribute_v_coord_postproc,  // attribute
				2,                  // number of elements per vertex, here (x,y)
				GL_FLOAT,           // the type of each element
				GL_FALSE,           // take our values as-is
				0,                  // no extra data between each position
				0                   // offset of first element
				);
	
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	
	unbindBuffer(m_vertices);
	glDisableVertexAttribArray(attribute_v_coord_postproc);
	
	glUseProgram(oldProgram);
}
