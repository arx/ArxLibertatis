
#include "graphics/opengl/OpenGLRenderer.h"

#include <GL/gl.h>

#include "graphics/Math.h"
#include "graphics/GraphicsUtility.h"
#include "graphics/opengl/GLTexture2D.h"
#include "graphics/opengl/GLTextureStage.h"
#include "graphics/opengl/GLVertexBuffer.h"
#include "io/Logger.h"

OpenGLRenderer::OpenGLRenderer() { };

OpenGLRenderer::~OpenGLRenderer() { };

void OpenGLRenderer::Initialize() {
	
	// TODO get the supported texture stage count
	m_TextureStages.resize(3);
	
	for(size_t i = 0; i < m_TextureStages.size(); ++i) {
		m_TextureStages[i] = new GLTextureStage(i);
		// Set default state
		m_TextureStages[i]->SetWrapMode(TextureStage::WrapRepeat);
		m_TextureStages[i]->SetMinFilter(TextureStage::FilterLinear);
		m_TextureStages[i]->SetMagFilter(TextureStage::FilterLinear);
		m_TextureStages[i]->SetMipFilter(TextureStage::FilterLinear);
	}
	
	SetRenderState(ColorKey, true);
	
	// Clear screen
	Clear(ColorBuffer | DepthBuffer);
}

bool OpenGLRenderer::BeginScene() {
	return true; // TODO implement
}

bool OpenGLRenderer::EndScene() {
	return true; // TODO implement
}

void OpenGLRenderer::SetViewMatrix(const EERIEMATRIX & matView) {
	ARX_UNUSED(matView); // TODO implement
}

void OpenGLRenderer::SetViewMatrix(const Vec3f & position, const Vec3f & dir, const Vec3f & up) {
	ARX_UNUSED(position), ARX_UNUSED(dir), ARX_UNUSED(up); // TODO implement
}

void OpenGLRenderer::GetViewMatrix(EERIEMATRIX & matView) const {
	Util_SetIdentityMatrix(matView); // TODO implement
}

void OpenGLRenderer::SetProjectionMatrix(const EERIEMATRIX & matProj) {
	ARX_UNUSED(matProj); // TODO implement
}

void OpenGLRenderer::GetProjectionMatrix(EERIEMATRIX & matProj) const {
	Util_SetIdentityMatrix(matProj); // TODO implement
}

void OpenGLRenderer::ReleaseAllTextures() {
	// TODO implement
}

void OpenGLRenderer::RestoreAllTextures() {
	// TODO implement
}

Texture2D * OpenGLRenderer::CreateTexture2D() {
	return new GLTexture2D; // TODO implement
}

static inline void setGLState(GLenum state, bool enable) {
	if(enable) {
		glEnable(state);
	} else {
		glDisable(state);
	}
}

void OpenGLRenderer::SetRenderState(RenderState renderState, bool enable) {
	
	switch(renderState) {
		
		case AlphaBlending: {
			setGLState(GL_ALPHA_TEST, enable);
			break;
		}
		
		case DepthTest: {
			setGLState(GL_DEPTH_TEST, enable);
			break;
		}
		
		case DepthWrite: {
			glDepthMask(enable ? GL_TRUE : GL_FALSE);
			break;
		}
		
		case Fog: {
			setGLState(GL_FOG, enable);
			break;
		}
		
		case Lighting: {
			setGLState(GL_LIGHTING, enable);
			break;
		}
		
		case ZBias: {
			setGLState(GL_POLYGON_OFFSET_FILL, enable);
			break;
		}
		
		default: // TODO implement
			LogWarning << "unsupported render state: " << renderState;
	}
	
}

void OpenGLRenderer::SetAlphaFunc(PixelCompareFunc func, float fef) {
	ARX_UNUSED(func), ARX_UNUSED(fef); // TODO implement
}

void OpenGLRenderer::SetBlendFunc(PixelBlendingFactor srcFactor, PixelBlendingFactor dstFactor) {
	ARX_UNUSED(srcFactor), ARX_UNUSED(dstFactor); // TODO implement
}

void OpenGLRenderer::SetViewport(const Rect & viewport) {
	ARX_UNUSED(viewport); // TODO implement
}

Rect OpenGLRenderer::GetViewport() {
	return Rect(1024, 768); // TODO implement
}

void OpenGLRenderer::Begin2DProjection(float left, float right, float bottom, float top, float zNear, float zFar) {
	ARX_UNUSED(left), ARX_UNUSED(right), ARX_UNUSED(bottom), ARX_UNUSED(top);
	ARX_UNUSED(zNear), ARX_UNUSED(zFar); // TODO implement
}

void OpenGLRenderer::End2DProjection() {
	// TODO implement
}

void OpenGLRenderer::Clear(BufferFlags bufferFlags, Color clearColor, float clearDepth, size_t nrects, Rect * rect) {
	ARX_UNUSED(bufferFlags), ARX_UNUSED(clearColor), ARX_UNUSED(clearDepth);
	ARX_UNUSED(nrects), ARX_UNUSED(rect); // TODO implement
}

void OpenGLRenderer::SetFogColor(Color color) {
	ARX_UNUSED(color); // TODO implement
}

void OpenGLRenderer::SetFogParams(FogMode fogMode, float fogStart, float fogEnd, float fogDensity) {
	ARX_UNUSED(fogMode), ARX_UNUSED(fogStart), ARX_UNUSED(fogEnd), ARX_UNUSED(fogDensity);
	// TODO implement
}

void OpenGLRenderer::SetAntialiasing(bool enable) {
	ARX_UNUSED(enable); // TODO implement
}

void OpenGLRenderer::SetCulling(CullingMode mode) {
	ARX_UNUSED(mode); // TODO implement
}

void OpenGLRenderer::SetDepthBias(int depthBias) {
	ARX_UNUSED(depthBias); // TODO implement
}

void OpenGLRenderer::SetFillMode(FillMode mode) {
	ARX_UNUSED(mode); // TODO implement
}

float OpenGLRenderer::GetMaxAnisotropy() const {
	return std::numeric_limits<float>::max();
}

void OpenGLRenderer::DrawTexturedRect(float x, float y, float w, float h, float uStart, float vStart, float uEnd, float vEnd, Color color) {
	ARX_UNUSED(x), ARX_UNUSED(y), ARX_UNUSED(w), ARX_UNUSED(h);
	ARX_UNUSED(uStart), ARX_UNUSED(vStart), ARX_UNUSED(uEnd), ARX_UNUSED(vEnd), ARX_UNUSED(color);
	// TODO implement
}

VertexBuffer<TexturedVertex> * OpenGLRenderer::createVertexBufferTL(size_t capacity, BufferUsage usage) {
	return new GLVertexBuffer<TexturedVertex>(capacity, usage); 
}

VertexBuffer<SMY_VERTEX> * OpenGLRenderer::createVertexBuffer(size_t capacity, BufferUsage usage) {
	return new GLVertexBuffer<SMY_VERTEX>(capacity, usage); 
}

VertexBuffer<SMY_VERTEX3> * OpenGLRenderer::createVertexBuffer3(size_t capacity, BufferUsage usage) {
	return new GLVertexBuffer<SMY_VERTEX3>(capacity, usage); 
}

void OpenGLRenderer::drawIndexed(Primitive primitive, const TexturedVertex * vertices, size_t nvertices, unsigned short * indices, size_t nindices) {
	ARX_UNUSED(primitive), ARX_UNUSED(vertices), ARX_UNUSED(nvertices);
	ARX_UNUSED(indices), ARX_UNUSED(nindices); // TODO implement
}

bool OpenGLRenderer::getSnapshot(Image & image) {
	ARX_UNUSED(image);
	return false; // TODO implement
}

bool OpenGLRenderer::getSnapshot(Image & image, size_t width, size_t height) {
	ARX_UNUSED(image), ARX_UNUSED(width), ARX_UNUSED(height);
	return false; // TODO implement
}

void OpenGLRenderer::setGamma(float brightness, float contrast, float gamma) {
	ARX_UNUSED(brightness), ARX_UNUSED(contrast), ARX_UNUSED(gamma); // TODO implement
}
