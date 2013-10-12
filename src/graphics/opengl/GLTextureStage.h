/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
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

#include "graphics/opengl/OpenGLUtil.h"
#include "graphics/texture/TextureStage.h"

#include <map>

class OpenGLRenderer;
class GLTexture2D;

class GLTextureStage : public TextureStage {
	
public:
	
	GLTextureStage(OpenGLRenderer * renderer, unsigned textureStage);
	~GLTextureStage();
	
	void setTexture(Texture * pTexture);
	void resetTexture();
	
	void setColorOp(TextureOp textureOp, TextureArg arg0, TextureArg arg1);
	void setColorOp(TextureOp textureOp);
	void setAlphaOp(TextureOp textureOp, TextureArg arg0, TextureArg arg1);
	void setAlphaOp(TextureOp textureOp);
	
	void setWrapMode(WrapMode wrapMode);
	
	void setMinFilter(FilterMode filterMode);
	void setMagFilter(FilterMode filterMode);
	void setMipFilter(FilterMode filterMode);
	
	void setMipMapLODBias(float bias);
	
	void apply();
	
private:
	
	inline bool isEnabled() { return ((ops[Color] != OpDisable) || (ops[Alpha] != OpDisable)); }
	
	OpenGLRenderer * renderer;
	
	enum OpType {
		Color,
		Alpha
	};
	
	enum Arg {
		Arg0,
		Arg1
	};
	
	TextureOp ops[2];
	TextureArg args[2][2];
	
	void setArg(OpType alpha, Arg idx, TextureArg arg);
	
	void setOp(OpType alpha, GLint op, GLfloat scale);
	void setOp(OpType alpha, TextureOp op);
	void setOp(OpType alpha, TextureOp op, TextureArg arg0, TextureArg arg1);

	void setTexEnv(GLenum target, GLenum pname, GLint param);
	void setTexEnv(GLenum target, GLenum pname, GLfloat param);

	GLTexture2D * tex;
	GLTexture2D * current;
	
	WrapMode wrapMode;
	FilterMode minFilter;
	FilterMode magFilter;
	FilterMode mipFilter;

	typedef std::map<GLenum, GLint> IntegerStateCache;
	IntegerStateCache m_stateCacheIntegers;

	typedef std::map<GLenum, GLfloat> FloatStateCache;
	FloatStateCache m_stateCacheFloats;
	
	friend class GLTexture2D;
};

#endif // ARX_GRAPHICS_OPENGL_GLTEXTURESTAGE_H
