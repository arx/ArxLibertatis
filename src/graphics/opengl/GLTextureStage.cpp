
#include "graphics/opengl/GLTextureStage.h"

#include "graphics/opengl/GLTexture2D.h"
#include "graphics/opengl/OpenGLRenderer.h"

GLTextureStage::GLTextureStage(OpenGLRenderer * _renderer, unsigned stage) : TextureStage(stage), renderer(_renderer), tex(NULL), current(NULL) {
	
	// Set default state
	wrapMode = TextureStage::WrapRepeat;
	minFilter = TextureStage::FilterLinear;
	magFilter = TextureStage::FilterLinear;
	mipFilter = TextureStage::FilterLinear;

	args[Color][Arg0] = TextureStage::ArgTexture;
	args[Color][Arg1] = TextureStage::ArgCurrent;
	args[Alpha][Arg0] = TextureStage::ArgTexture;
	args[Alpha][Arg1] = TextureStage::ArgCurrent;	
}

GLTextureStage::~GLTextureStage() {
	ResetTexture();
}

void GLTextureStage::SetTexture(Texture * texture) {
	
	arx_assert(texture != NULL);
	
	if(texture == tex) {
		return;
	}
	
	tex = reinterpret_cast<GLTexture2D *>(texture);
}

void GLTextureStage::ResetTexture() {
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
	
	glTexEnvi(GL_TEXTURE_ENV, glTexEnv[alpha].sources[idx], glTexSource[arg & ArgMask]);
	GLint op = (arg & ArgComplement) ? glTexEnv[alpha].normal : glTexEnv[alpha].complement;
	glTexEnvi(GL_TEXTURE_ENV, glTexEnv[alpha].operands[idx], op);
}

void GLTextureStage::setOp(OpType alpha, GLenum op, GLint scale) {
	
	glTexEnvi(GL_TEXTURE_ENV, glTexEnv[alpha].combine, op);
	glTexEnvi(GL_TEXTURE_ENV, glTexEnv[alpha].scale, scale);
}

void GLTextureStage::setOp(OpType alpha, TextureOp op) {
	
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

void GLTextureStage::SetColorOp(TextureOp op, TextureArg arg0, TextureArg arg1) {
	setOp(Color, op, arg0, arg1);
}

void GLTextureStage::SetColorOp(TextureOp op) {
	setOp(Color, op);
}

void GLTextureStage::SetAlphaOp(TextureOp op, TextureArg arg0, TextureArg arg1) {
	setOp(Alpha, op, arg0, arg1);
}

void GLTextureStage::SetAlphaOp(TextureOp op) {
	setOp(Alpha, op);
}

void GLTextureStage::SetWrapMode(WrapMode _wrapMode) {
	wrapMode = _wrapMode, current = NULL;
}

void GLTextureStage::SetMinFilter(FilterMode filterMode) {
	minFilter = filterMode, current = NULL;
}

void GLTextureStage::SetMagFilter(FilterMode filterMode) {
	magFilter = filterMode, current = NULL;
}

void GLTextureStage::SetMipFilter(FilterMode filterMode) {
	mipFilter = filterMode, current = NULL;
}

void GLTextureStage::SetMipMapLODBias(float bias) {
	glActiveTexture(GL_TEXTURE0 + mStage);
	glTexEnvf(GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, bias);
	CHECK_GL;
}

void GLTextureStage::SetTextureCoordIndex(int texCoordIdx) {
	ARX_UNUSED(texCoordIdx); // TODO does OpenGL support this?
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
		for(size_t i = 0; i != mStage; i++) {
			if(renderer->GetTextureStage(0)->tex == tex) {
				apply = false;
#ifdef _DEBUG
				GLTextureStage * stage0 = renderer->GetTextureStage(0);
				if(stage0->wrapMode != wrapMode || stage0->minFilter != minFilter || stage0->magFilter != magFilter || stage0->mipFilter != mipFilter) {
					static bool warned = false;
					if(!warned) {
						LogWarning << "Same texture used in multiple stages with different attributes.";
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
