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

#include "graphics/opengl/GLRenderTexture.h"

#include "io/log/Logger.h"

GLRenderTexture::GLRenderTexture(Vec2s size)
	: m_size(size)
	, m_valid(false)
{
	// Texture
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &m_texture);
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_size.x, m_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	// Depth buffer
	glGenRenderbuffers(1, &m_depth);
	glBindRenderbuffer(GL_RENDERBUFFER, m_depth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, m_size.x, m_size.y);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	
	// Framebuffer
	glGenFramebuffers(1, &m_framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depth);
	
	GLenum status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
	if(status == GL_FRAMEBUFFER_COMPLETE) {
		m_valid = true;
	} else {
		// We should only request pixel types required by the spec
		arx_assert(status != GL_FRAMEBUFFER_UNSUPPORTED);
		
		LogError << "Framebuffer Error: " << framebufferStatusString(status) << "\n";
	}
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLRenderTexture::~GLRenderTexture() {
	
	glDeleteTextures(1, &m_texture);
	glDeleteRenderbuffers(1, &m_depth);
	glDeleteFramebuffers(1, &m_framebuffer);
}

Vec2s GLRenderTexture::size() {
	
	return m_size;
}

void GLRenderTexture::setSize(Vec2s size) {
	
	m_size = size;
	
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_size.x, m_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	glBindRenderbuffer(GL_RENDERBUFFER, m_depth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, m_size.x, m_size.y);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

bool GLRenderTexture::isValid() {
	return m_valid;
}

void GLRenderTexture::attach() {
	
	glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GLRenderTexture::bind() {
	glBindTexture(GL_TEXTURE_2D, m_texture);
}

const char *GLRenderTexture::framebufferStatusString(GLenum status) {
	
	switch(status) {
		case GL_FRAMEBUFFER_COMPLETE:                      return "COMPLETE";
		case GL_FRAMEBUFFER_UNDEFINED:                     return "UNDEFINED";
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:         return "INCOMPLETE_ATTACHMENT";
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: return "INCOMPLETE_MISSING_ATTACHMENT";
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:        return "INCOMPLETE_DRAW_BUFFER";
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:        return "INCOMPLETE_READ_BUFFER";
		case GL_FRAMEBUFFER_UNSUPPORTED:                   return "UNSUPPORTED";
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:        return "INCOMPLETE_MULTISAMPLE";
		case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:      return "INCOMPLETE_LAYER_TARGETS";
		default:                                           return "(unknown)";
	}
}
