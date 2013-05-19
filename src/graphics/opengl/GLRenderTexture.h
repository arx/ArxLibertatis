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

#ifndef ARX_GRAPHICS_OPENGL_GLRENDERTEXTURE_H
#define ARX_GRAPHICS_OPENGL_GLRENDERTEXTURE_H

#include <boost/noncopyable.hpp>

#include "graphics/opengl/OpenGLUtil.h"
#include "math/Types.h"

class GLRenderTexture : public boost::noncopyable {
	
public:
	GLRenderTexture(Vec2s size);
	~GLRenderTexture();
	
	Vec2s size();
	void setSize(Vec2s size);
	bool isValid();
	
	void attach();
	void bind();
	
private:
	Vec2s m_size;
	bool m_valid;
	
	GLuint m_framebuffer;
	GLuint m_texture;
	GLuint m_depth;
	
	const char * framebufferStatusString(GLenum status);
};

#endif // ARX_GRAPHICS_OPENGL_GLRENDERTEXTURE_H
