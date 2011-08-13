
#include "graphics/opengl/GLTextureStage.h"

GLTextureStage::GLTextureStage(unsigned stage) : TextureStage(stage) { }

void GLTextureStage::SetTexture(Texture * pTexture) {
	ARX_UNUSED(pTexture); // TODO implement
}

void GLTextureStage::ResetTexture() {
	// TODO implement
}

void GLTextureStage::SetColorOp(TextureOp textureOp, TextureArg texArg1, TextureArg texArg2) {
	ARX_UNUSED(textureOp), ARX_UNUSED(texArg1), ARX_UNUSED(texArg2); // TODO implement
}

void GLTextureStage::SetColorOp(TextureOp textureOp) {
	ARX_UNUSED(textureOp); // TODO implement
}

void GLTextureStage::SetAlphaOp(TextureOp textureOp, TextureArg texArg1, TextureArg texArg2) {
	ARX_UNUSED(textureOp), ARX_UNUSED(texArg1), ARX_UNUSED(texArg2); // TODO implement
}

void GLTextureStage::SetAlphaOp(TextureOp textureOp) {
	ARX_UNUSED(textureOp); // TODO implement
}

void GLTextureStage::SetWrapMode(WrapMode wrapMode) {
	ARX_UNUSED(wrapMode); // TODO implement
}

void GLTextureStage::SetMinFilter(FilterMode filterMode) {
	ARX_UNUSED(filterMode); // TODO implement
}

void GLTextureStage::SetMagFilter(FilterMode filterMode) {
	ARX_UNUSED(filterMode); // TODO implement
}

void GLTextureStage::SetMipFilter(FilterMode filterMode) {
	ARX_UNUSED(filterMode); // TODO implement
}

void GLTextureStage::SetMipMapLODBias(float bias) {
	ARX_UNUSED(bias); // TODO implement
}

void GLTextureStage::SetTextureCoordIndex(int texCoordIdx) {
	ARX_UNUSED(texCoordIdx); // TODO implement
}
