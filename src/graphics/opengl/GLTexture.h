/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GRAPHICS_OPENGL_GLTEXTURE_H
#define ARX_GRAPHICS_OPENGL_GLTEXTURE_H

#include <boost/intrusive/list_hook.hpp>

#include "graphics/opengl/OpenGLUtil.h"
#include "graphics/texture/Texture.h"
#include "graphics/texture/TextureStage.h"
#include "platform/Platform.h"

class OpenGLRenderer;
class GLTextureStage;

typedef boost::intrusive::list_base_hook<boost::intrusive::link_mode<boost::intrusive::auto_unlink> > GLTextureListHook;

class GLTexture arx_final : public Texture, public GLTextureListHook {
	
public:
	
	explicit GLTexture(OpenGLRenderer * renderer);
	~GLTexture();
	
	bool create();
	void upload();
	void destroy();
	
	void apply(GLTextureStage * stage);
	
	void updateMaxAnisotropy();
	
private:
	
	OpenGLRenderer * renderer;
	
	GLuint tex;
	
	TextureStage::WrapMode wrapMode;
	TextureStage::FilterMode minFilter;
	TextureStage::FilterMode magFilter;
	
	bool isNPOT;
	
	friend class GLTextureStage;
};

#endif // ARX_GRAPHICS_OPENGL_GLTEXTURE_H
