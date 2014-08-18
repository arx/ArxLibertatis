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

#ifndef ARX_GRAPHICS_OPENGL_GLPOSTPROCESS_H
#define ARX_GRAPHICS_OPENGL_GLPOSTPROCESS_H

#include <boost/noncopyable.hpp>

#include "graphics/opengl/GLProgram.h"
#include "graphics/opengl/GLRenderTexture.h"

class GLPostProcess : public boost::noncopyable {
public:
	GLPostProcess(const Vec2s & size);
	~GLPostProcess();
	
	void resize(const Vec2s & size);
	Vec2s size();
	
	void use();
	void render(Rect screenSize);
	
private:
	GLRenderTexture * m_texture;
	GLProgram * m_program;
	
	GLint attribute_v_coord_postproc;
	GLint uniform_fbo_texture;
	
	GLint m_uniform_gamma;
	
	GLuint m_vertices;
};

#endif // ARX_GRAPHICS_OPENGL_GLPOSTPROCESS_H
