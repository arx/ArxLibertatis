/*
 * Copyright 2011-2016 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GRAPHICS_OPENGL_GLTEXTURESTAGE_H
#define ARX_GRAPHICS_OPENGL_GLTEXTURESTAGE_H

#include <map>

#include "graphics/opengl/OpenGLUtil.h"
#include "graphics/texture/TextureStage.h"

class OpenGLRenderer;
class GLTexture;

class GLTextureStage : public TextureStage {
	
public:
	
	GLTextureStage(OpenGLRenderer * renderer, unsigned textureStage);
	~GLTextureStage();
	
	Texture * getTexture() const;
	void setTexture(Texture * pTexture);
	void resetTexture();
	
	void setColorOp(TextureOp textureOp);
	void setAlphaOp(TextureOp textureOp);
	
	WrapMode getWrapMode() const;
	void setWrapMode(WrapMode wrapMode);
	
	void setMinFilter(FilterMode filterMode);
	void setMagFilter(FilterMode filterMode);
	
	void setMipMapLODBias(float bias);
	
	void apply();
	
private:
	
	bool isEnabled() { return ((ops[Color] != OpDisable) || (ops[Alpha] != OpDisable)); }
	
	OpenGLRenderer * renderer;
	
	enum OpType {
		Color,
		Alpha
	};
	
	TextureOp ops[2];
	
	void setArg(OpType alpha, GLint arg);
	
	void setOp(OpType alpha, GLint op, GLint scale);
	void setOp(OpType alpha, TextureOp op);

	void setTexEnv(GLenum target, GLenum pname, GLint param);

	GLTexture * tex;
	GLTexture * current;
	
	WrapMode wrapMode;
	FilterMode minFilter;
	FilterMode magFilter;

	typedef std::map<GLenum, GLint> IntegerStateCache;
	IntegerStateCache m_stateCacheIntegers;
	
	friend class GLTexture;
};

#endif // ARX_GRAPHICS_OPENGL_GLTEXTURESTAGE_H
