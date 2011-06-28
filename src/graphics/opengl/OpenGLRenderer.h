
#ifndef ARX_GRAPHICS_OPENGL_OPENGLRENDERER_H
#define ARX_GRAPHICS_OPENGL_OPENGLRENDERER_H

#include "graphics/Renderer.h"
#include "Configure.h"

#ifdef HAVE_OPENGL

class OpenGLRenderer : Renderer {
	
	OpenGLRenderer() { };
	~OpenGLRenderer() { };
	
	// TODO implement
	
	virtual void Initialize() = 0;
	
	// Scene begin/end...
	virtual bool BeginScene() = 0;
	virtual bool EndScene() = 0;
	
	// Matrices
	virtual void SetViewMatrix(const EERIEMATRIX & matView) = 0;
	virtual void SetViewMatrix(const Vec3f & vPosition, const Vec3f & vDir, const Vec3f & vUp) = 0;
	virtual void GetViewMatrix(EERIEMATRIX & matView) const = 0;
	virtual void SetProjectionMatrix(const EERIEMATRIX & matProj) = 0;
	virtual void GetProjectionMatrix(EERIEMATRIX & matProj) const = 0;
	
	// Texture management
	virtual void ReleaseAllTextures() = 0;
	virtual void RestoreAllTextures() = 0;
	
	// Factory
	virtual Texture2D * CreateTexture2D() = 0;
	
	// Render states
	void SetRenderState(RenderState renderState, bool enable);
	
	// Alphablending & Transparency
	virtual void SetAlphaFunc(PixelCompareFunc func, float fef) = 0; // Ref = [0.0f, 1.0f]
	virtual void SetBlendFunc(PixelBlendingFactor srcFactor, PixelBlendingFactor dstFactor) = 0;
	
	// Viewport
	virtual void SetViewport(const Rect & viewport) = 0;
	virtual Rect GetViewport() = 0;
	
	// Projection
	virtual void Begin2DProjection(float left, float right, float bottom, float top, float zNear, float zFar) = 0;
	virtual void End2DProjection() = 0;
	
	// Render Target
	virtual void Clear(BufferFlags bufferFlags, Color clearColor = Color::none, float clearDepth = 1.f, size_t nrects = 0, Rect * rect = 0) = 0;
	
	// Fog
	virtual void SetFogColor(Color color) = 0;
	virtual void SetFogParams(FogMode fogMode, float fogStart, float fogEnd, float fogDensity = 1.0f) = 0;
	
	// Rasterizer
	virtual void SetAntialiasing(bool enable) = 0;
	virtual void SetCulling(CullingMode mode) = 0;
	virtual void SetDepthBias(int depthBias) = 0;
	virtual void SetFillMode(FillMode mode) = 0;
	
	// Texturing
	virtual unsigned int GetTextureStageCount() const = 0;
	virtual TextureStage* GetTextureStage(unsigned int textureStage) = 0;
	virtual void ResetTexture(unsigned int textureStage) = 0;
	virtual void SetTexture(unsigned int textureStage, Texture * pTexture) = 0;
	virtual void SetTexture(unsigned int textureStage, TextureContainer* pTextureContainer) = 0;
	
	virtual float GetMaxAnisotropy() const = 0;
	
	// Utilities...
	virtual void DrawTexturedRect(float x, float y, float w, float h, float uStart, float vStart, float uEnd, float vEnd, Color color) = 0;
	
	virtual VertexBuffer<TexturedVertex> * createVertexBufferTL(size_t capacity, BufferUsage usage) = 0;
	virtual VertexBuffer<SMY_D3DVERTEX> * createVertexBuffer(size_t capacity, BufferUsage usage) = 0;
	virtual VertexBuffer<SMY_D3DVERTEX3> * createVertexBuffer3(size_t capacity, BufferUsage usage) = 0;
	
	virtual void drawIndexed(Primitive primitive, const TexturedVertex * vertices, size_t nvertices, unsigned short * indices, size_t nindices) = 0;
	
	virtual bool getSnapshot(Image & image) = 0;
	virtual bool getSnapshot(Image & image, size_t width, size_t height) = 0;
	
};

#endif // HAVE_OPENGL

#endif // ARX_GRAPHICS_OPENGL_OPENGLRENDERER_H
