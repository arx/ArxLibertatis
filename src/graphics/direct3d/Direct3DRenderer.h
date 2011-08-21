
#ifndef ARX_GRAPHICS_DIRECT3D_DIRECT3DRENDERER_H
#define ARX_GRAPHICS_DIRECT3D_DIRECT3DRENDERER_H

#include <d3d.h>

#include "graphics/Renderer.h"
#include "window/D3D7Window.h"

class Direct3DRenderer : public Renderer {
	
public:
	
	Direct3DRenderer(D3D7Window * window);
	~Direct3DRenderer();
	
	void Initialize();
	
	// Scene begin/end...
	bool BeginScene();
	bool EndScene();
	
	// Matrices
	void SetViewMatrix(const EERIEMATRIX & matView);
	void GetViewMatrix(EERIEMATRIX & matView) const;
	void SetProjectionMatrix(const EERIEMATRIX & matProj);
	void GetProjectionMatrix(EERIEMATRIX & matProj) const;
	
	// Texture management
	void ReleaseAllTextures();
	void RestoreAllTextures();
	
	// Factory
	Texture2D * CreateTexture2D();
	
	// Render states
	void SetRenderState(RenderState renderState, bool enable);
	
	// Alphablending & Transparency
	void SetAlphaFunc(PixelCompareFunc func, float fef); // Ref = [0.0f, 1.0f]
	void SetBlendFunc(PixelBlendingFactor srcFactor, PixelBlendingFactor dstFactor);
	
	// Viewport
	void SetViewport(const Rect & viewport);
	Rect GetViewport();
	
	// Projection
	void Begin2DProjection(float left, float right, float bottom, float top, float zNear, float zFar);
	void End2DProjection();
	
	// Render Target
	void Clear(BufferFlags bufferFlags, Color clearColor = Color::none, float clearDepth = 1.f, size_t nrects = 0, Rect * rect = 0);
	
	// Fog
	void SetFogColor(Color color);
	void SetFogParams(FogMode fogMode, float fogStart, float fogEnd, float fogDensity = 1.0f);
	
	// Rasterizer
	void SetAntialiasing(bool enable);
	void SetCulling(CullingMode mode);
	void SetDepthBias(int depthBias);
	void SetFillMode(FillMode mode);
	
	float GetMaxAnisotropy() const;
	
	// Utilities...
	void DrawTexturedRect(float x, float y, float w, float h, float uStart, float vStart, float uEnd, float vEnd, Color color);
	
	VertexBuffer<TexturedVertex> * createVertexBufferTL(size_t capacity, BufferUsage usage);
	VertexBuffer<SMY_VERTEX> * createVertexBuffer(size_t capacity, BufferUsage usage);
	VertexBuffer<SMY_VERTEX3> * createVertexBuffer3(size_t capacity, BufferUsage usage);
	
	void drawIndexed(Primitive primitive, const TexturedVertex * vertices, size_t nvertices, unsigned short * indices, size_t nindices);
	
	bool getSnapshot(Image & image);
	bool getSnapshot(Image & image, size_t width, size_t height);
	
	bool isFogInEyeCoordinates();
	
private:
	
	LPDIRECT3DDEVICE7 device;
	D3D7Window * window;
	
};

#endif // ARX_GRAPHICS_DIRECT3D_DIRECT3DRENDERER_H
