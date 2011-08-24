
#ifndef ARX_GRAPHICS_OPENGL_OPENGLRENDERER_H
#define ARX_GRAPHICS_OPENGL_OPENGLRENDERER_H

#include "graphics/Renderer.h"
#include "math/Rectangle.h"

class GLTextureStage;
class OpenGLRenderer : public Renderer {
	
public:
	
	OpenGLRenderer();
	~OpenGLRenderer();
	
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
	
	inline GLTextureStage * GetTextureStage(unsigned int textureStage) {
		return reinterpret_cast<GLTextureStage *>(Renderer::GetTextureStage(textureStage));
	}
	
private:
	
	bool useVertexArrays;
	bool useVBOs;
	
	Rect viewport;
	
	void applyTextureStages();
	
	template <class Vertex>
	void selectTrasform();
	
	void enableTransform();
	void disableTransform();
	
	template <class Vertex>
	inline void beforeDraw() { applyTextureStages(); selectTrasform<Vertex>(); }
	
	template <class Vertex>
	friend class GLNoVertexBuffer;
	template <class Vertex>
	friend class GLVertexBuffer;
	
	friend class GLTextureStage;
	
	size_t maxTextureStage; // the highest active texture stage
	
};

template <class Vertex>
inline void OpenGLRenderer::selectTrasform() { enableTransform(); }

template <>
inline void OpenGLRenderer::selectTrasform<TexturedVertex>() { disableTransform(); }

#endif // ARX_GRAPHICS_OPENGL_OPENGLRENDERER_H
