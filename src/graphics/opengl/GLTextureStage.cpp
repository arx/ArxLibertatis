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

#include "graphics/opengl/GLTextureStage.h"

#include "graphics/opengl/GLTexture.h"
#include "graphics/opengl/OpenGLRenderer.h"
#include "io/log/Logger.h"

GLTextureStage::GLTextureStage(OpenGLRenderer * _renderer, unsigned stage) : TextureStage(stage), renderer(_renderer), tex(NULL), current(NULL) {
	
	// Set default state
	
	if(mStage == 0) {
		ops[ColorOp] = OpModulate;
		ops[AlphaOp] = OpSelectArg1;
		glActiveTexture(GL_TEXTURE0);
		setTexEnv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		setTexEnv(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE); // TODO change the AL default to match OpenGL
		glEnable(GL_TEXTURE_2D);
	} else {
		ops[ColorOp] = OpDisable;
		ops[AlphaOp] = OpDisable;
	}
	
}

GLTextureStage::~GLTextureStage() {
	resetTexture();
}

Texture * GLTextureStage::getTexture() const {
	return tex;
}

void GLTextureStage::setTexture(Texture * texture) {
	
	arx_assert(texture != NULL);
	
	tex = reinterpret_cast<GLTexture *>(texture);
}

void GLTextureStage::resetTexture() {
	tex = NULL;
}

struct GLTexEnvParam {
	
	GLenum combine;
	
	GLenum source;
	
	GLenum scale;
	
};

static const GLTexEnvParam glTexEnv[] = {
	{
		GL_COMBINE_RGB,
		GL_SOURCE0_RGB,
		GL_RGB_SCALE
	}, {
		GL_COMBINE_ALPHA,
		GL_SOURCE0_ALPHA,
		GL_ALPHA_SCALE
	}
};

void GLTextureStage::setArg(OpType alpha, GLint arg) {
	
	setTexEnv(GL_TEXTURE_ENV, glTexEnv[alpha].source, arg);
}

void GLTextureStage::setOp(OpType alpha, GLint op, GLint scale) {

	setTexEnv(GL_TEXTURE_ENV, glTexEnv[alpha].combine, op);
	setTexEnv(GL_TEXTURE_ENV, glTexEnv[alpha].scale, scale);
}

void GLTextureStage::setOp(OpType alpha, TextureOp op) {
	
	if(mStage != 0) {
		glActiveTexture(GL_TEXTURE0 + mStage);
	}
		
	bool wasEnabled = isEnabled();
	
	ops[alpha] = op;
	
	bool enabled = isEnabled();
	if(wasEnabled != enabled) {
		if(enabled) {
			glEnable(GL_TEXTURE_2D);
			setTexEnv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
			renderer->maxTextureStage = std::max<size_t>(mStage, renderer->maxTextureStage);
		} else {
			glDisable(GL_TEXTURE_2D);
			if(renderer->maxTextureStage == mStage) {
				renderer->maxTextureStage = 0;
				for(int stage = mStage - 1; stage >= 0; stage--) {
					if(renderer->GetTextureStage(stage)->isEnabled()) {
						renderer->maxTextureStage = stage;
						break;
					}
				}
			}
		}
	}
	
	switch(op) {
		
		case OpDisable: {
			setOp(alpha, GL_REPLACE, 1);
			setArg(alpha, GL_PREVIOUS);
			break;
		}
		
		case OpSelectArg1: {
			setOp(alpha, GL_REPLACE, 1);
			setArg(alpha, GL_TEXTURE);
			break;
		}
		
		case OpModulate: {
			setOp(alpha, GL_MODULATE, 1);
			setArg(alpha, GL_TEXTURE);
			break;
		}
		
		case OpModulate2X: {
			setOp(alpha, GL_MODULATE, 2);
			setArg(alpha, GL_TEXTURE);
			break;
		}
		
		case OpModulate4X: {
			setOp(alpha, GL_MODULATE, 4);
			setArg(alpha, GL_TEXTURE);
			break;
		}
		
	}

	if(mStage != 0) {
		glActiveTexture(GL_TEXTURE0);
	}
}

void GLTextureStage::setTexEnv(GLenum target, GLenum pname, GLint param) {

	IntegerStateCache::iterator it = m_stateCacheIntegers.find(pname);
	if(it == m_stateCacheIntegers.end() || it->second != param) {
		glTexEnvi(target, pname, param);
		m_stateCacheIntegers[pname] = param;
	}
}

void GLTextureStage::setColorOp(TextureOp op) {
	setOp(ColorOp, op);
}

void GLTextureStage::setAlphaOp(TextureOp op) {
	setOp(AlphaOp, op);
}

void GLTextureStage::setMipMapLODBias(float bias) {
	
	if(mStage != 0) {
		glActiveTexture(GL_TEXTURE0 + mStage);
	}
	
	glTexEnvf(GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, bias);
	
	if(mStage != 0) {
		glActiveTexture(GL_TEXTURE0);
	}
}

void GLTextureStage::apply() {
	
	if(!tex && !current) {
		return;
	}
	
	if(mStage != 0) {
		glActiveTexture(GL_TEXTURE0 + mStage);
	}
	
	if(tex != current) {
		glBindTexture(GL_TEXTURE_2D, tex ? tex->tex : GL_NONE), current = tex;
	}
	
	if(tex) {
		
		bool apply = true;
		for(size_t i = 0; i < mStage; i++) {
			GLTextureStage * stage = renderer->GetTextureStage(i);
			if(stage->tex == tex && stage->isEnabled()) {
				apply = false;
				#ifdef ARX_DEBUG
				if(stage->getWrapMode() != getWrapMode()
				   || stage->getMinFilter() != getMinFilter() || stage->getMagFilter() != getMagFilter()) {
					static bool warned = false;
					if(!warned) {
						LogWarning << "Same texture used in multiple stages with different attributes.";
						warned = true;
					}
				}
				#else
				break;
				#endif
			}
		}
		
		if(apply) {
			tex->apply(this);
		}
		
	}

	if(mStage != 0) {
		glActiveTexture(GL_TEXTURE0);
	}
}
