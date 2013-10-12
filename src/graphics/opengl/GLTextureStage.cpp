/*
 * Copyright 2011-2013 Arx Libertatis Team (see the AUTHORS file)
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

#include "graphics/opengl/GLTexture2D.h"
#include "graphics/opengl/OpenGLRenderer.h"

GLTextureStage::GLTextureStage(OpenGLRenderer * _renderer, unsigned stage) : TextureStage(stage), renderer(_renderer), tex(NULL), current(NULL) {
	
	// Set default state
	wrapMode = WrapRepeat;
	minFilter = FilterLinear;
	magFilter = FilterLinear;
	mipFilter = FilterLinear;
	
	args[Color][Arg0] = ArgTexture;
	args[Color][Arg1] = ArgCurrent;
	args[Alpha][Arg0] = ArgTexture;
	args[Alpha][Arg1] = ArgCurrent;
	if(mStage == 0) {
		ops[Color] = OpModulate;
		ops[Alpha] = OpSelectArg1;
		glActiveTexture(GL_TEXTURE0);
		setTexEnv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glEnable(GL_TEXTURE_2D);
	} else {
		ops[Color] = OpDisable;
		ops[Alpha] = OpDisable;
	}
}

GLTextureStage::~GLTextureStage() {
	resetTexture();
}

void GLTextureStage::setTexture(Texture * texture) {
	
	arx_assert(texture != NULL);
	
	tex = reinterpret_cast<GLTexture2D *>(texture);
}

void GLTextureStage::resetTexture() {
	tex = NULL;
}

static const GLint glTexSource[] = {
	GL_PRIMARY_COLOR, // ArgDiffuse,
	GL_PREVIOUS, // ArgCurrent,
	GL_TEXTURE // ArgTexture
};

struct GLTexEnvParam {
	
	GLenum combine;
	
	GLenum sources[2];
	GLenum operands[2];
	
	GLint normal;
	GLint complement;
	
	GLenum scale;
	
};

static const GLTexEnvParam glTexEnv[] = {
	{
		GL_COMBINE_RGB,
		{ GL_SOURCE0_RGB, GL_SOURCE1_RGB },
		{ GL_OPERAND0_RGB, GL_OPERAND1_RGB },
		GL_SRC_COLOR,
		GL_ONE_MINUS_SRC_COLOR,
		GL_RGB_SCALE
	}, {
		GL_COMBINE_ALPHA,
		{ GL_SOURCE0_ALPHA, GL_SOURCE1_ALPHA },
		{ GL_OPERAND0_ALPHA, GL_OPERAND1_ALPHA },
		GL_SRC_ALPHA,
		GL_ONE_MINUS_SRC_ALPHA,
		GL_ALPHA_SCALE
	}
};

void GLTextureStage::setArg(OpType alpha, Arg idx, TextureArg arg) {
	
	setTexEnv(GL_TEXTURE_ENV, glTexEnv[alpha].sources[idx], glTexSource[arg & ArgMask]);
	GLint op = (arg & ArgComplement) ? glTexEnv[alpha].complement : glTexEnv[alpha].normal;
	setTexEnv(GL_TEXTURE_ENV, glTexEnv[alpha].operands[idx], op);
}

void GLTextureStage::setOp(OpType alpha, GLint op, GLfloat scale) {

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
	
	CHECK_GL;
	
	switch(op) {
		
		case OpDisable: {
			setOp(alpha, GL_REPLACE, 1);
			setArg(alpha, Arg0, ArgCurrent);
			CHECK_GL;
			break;
		}
		
		case OpSelectArg1: {
			setOp(alpha, GL_REPLACE, 1);
			setArg(alpha, Arg0, args[alpha][Arg0]);
			CHECK_GL;
			break;
		}
		
		case OpSelectArg2: {
			setOp(alpha, GL_REPLACE, 1);
			setArg(alpha, Arg0, args[alpha][Arg1]);
			CHECK_GL;
			break;
		}
		
		case OpModulate: {
			setOp(alpha, GL_MODULATE, 1);
			setArg(alpha, Arg0, args[alpha][Arg0]);
			setArg(alpha, Arg1, args[alpha][Arg1]);
			CHECK_GL;
			break;
		}
		
		case OpModulate2X: {
			setOp(alpha, GL_MODULATE, 2);
			setArg(alpha, Arg0, args[alpha][Arg0]);
			setArg(alpha, Arg1, args[alpha][Arg1]);
			CHECK_GL;
			break;
		}
		
		case OpModulate4X: {
			setOp(alpha, GL_MODULATE, 4);
			setArg(alpha, Arg0, args[alpha][Arg0]);
			setArg(alpha, Arg1, args[alpha][Arg1]);
			CHECK_GL;
			break;
		}
		
		case OpAddSigned: {
			setOp(alpha, GL_ADD_SIGNED, 1);
			setArg(alpha, Arg0, args[alpha][Arg0]);
			setArg(alpha, Arg1, args[alpha][Arg1]);
			CHECK_GL;
			break;
		}
		
	}

	if(mStage != 0) {
		glActiveTexture(GL_TEXTURE0);
	}
}

void GLTextureStage::setOp(OpType alpha, TextureOp op, TextureArg arg0, TextureArg arg1) {
	
	if(op != OpDisable) {
		if(op != OpSelectArg2) {
			args[alpha][0] = arg0;
		}
		if(op != OpSelectArg1) {
			args[alpha][1] = arg1;
		}
	}
	
	setOp(alpha, op);
}

void GLTextureStage::setTexEnv(GLenum target, GLenum pname, GLint param) {

	IntegerStateCache::iterator it = m_stateCacheIntegers.find(pname);
	if(it == m_stateCacheIntegers.end() || it->second != param) {
		glTexEnvi(target, pname, param);
		m_stateCacheIntegers[pname] = param;
	}
}

void GLTextureStage::setTexEnv(GLenum target, GLenum pname, GLfloat param) {

	FloatStateCache::iterator it = m_stateCacheFloats.find(pname);
	if(it == m_stateCacheFloats.end() || it->second != param) {
		glTexEnvf(target, pname, param);
		m_stateCacheFloats[pname] = param;
	}
}

void GLTextureStage::setColorOp(TextureOp op, TextureArg arg0, TextureArg arg1) {
	setOp(Color, op, arg0, arg1);
}

void GLTextureStage::setColorOp(TextureOp op) {
	setOp(Color, op);
}

void GLTextureStage::setAlphaOp(TextureOp op, TextureArg arg0, TextureArg arg1) {
	setOp(Alpha, op, arg0, arg1);
}

void GLTextureStage::setAlphaOp(TextureOp op) {
	setOp(Alpha, op);
}

void GLTextureStage::setWrapMode(WrapMode _wrapMode) {
	wrapMode = _wrapMode;
}

void GLTextureStage::setMinFilter(FilterMode filterMode) {
	minFilter = filterMode;
}

void GLTextureStage::setMagFilter(FilterMode filterMode) {
	magFilter = filterMode;
}

void GLTextureStage::setMipFilter(FilterMode filterMode) {
	mipFilter = filterMode;
}

void GLTextureStage::setMipMapLODBias(float bias) {
	
	if(mStage != 0) {
		glActiveTexture(GL_TEXTURE0 + mStage);
	}
	
	setTexEnv(GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, bias);
	
	if(mStage != 0) {
		glActiveTexture(GL_TEXTURE0);
	}

	CHECK_GL;
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
				if(stage->wrapMode != wrapMode || stage->minFilter != minFilter || stage->magFilter != magFilter || stage->mipFilter != mipFilter) {
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

	CHECK_GL;
}
