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

#ifndef ARX_GRAPHICS_RENDERER_H
#define ARX_GRAPHICS_RENDERER_H

#include <vector>

#include "platform/Flags.h"
#include "math/Types.h"
#include "graphics/Color.h"

struct EERIEMATRIX;
struct TexturedVertex;
struct SMY_VERTEX;
struct SMY_VERTEX3;
class TextureContainer;
class TextureStage;
class Image;
class Texture;
class Texture2D;
template <class Vertex> class VertexBuffer;

class Renderer {
	
public:
	
	class Listener {
		
	public:
		
		virtual ~Listener() { }
		
		virtual void onRendererInit(Renderer &) { }
		virtual void onRendererShutdown(Renderer &) { }
		
	};
	
	//! Render states
	enum RenderState {
		AlphaBlending,
		AlphaTest,
		ColorKey,
		DepthTest,
		DepthWrite,
		Fog,
		Lighting,
		ZBias
	};
	
	//! Pixel comparison functions
	enum PixelCompareFunc {
		CmpNever,               //!< Never
		CmpLess,                //!< Less
		CmpEqual,               //!< Equal
		CmpLessEqual,           //!< Less Equal
		CmpGreater,             //!< Greater
		CmpNotEqual,            //!< Not Equal
		CmpGreaterEqual,        //!< Greater Equal
		CmpAlways               //!< Always
	};
	
	//! Pixel blending factor
	enum PixelBlendingFactor {
		BlendZero,              //!< Zero
		BlendOne,               //!< One
		BlendSrcColor,          //!< Source color
		BlendSrcAlpha,          //!< Source alpha
		BlendInvSrcColor,       //!< Inverse source color
		BlendInvSrcAlpha,       //!< Inverse source alpha
		BlendSrcAlphaSaturate,  //!< Source alpha saturate
		BlendDstColor,          //!< Destination color
		BlendDstAlpha,          //!< Destination alpha
		BlendInvDstColor,       //!< Inverse destination color
		BlendInvDstAlpha        //!< Inverse destination alpha
	};
	
	//! Culling 
	enum CullingMode {
		CullNone,
		CullCW,
		CullCCW
	};
	
	enum FillMode {
		FillPoint,
		FillWireframe,
		FillSolid
	};
	
	//! Fog
	enum FogMode {
		FogNone,
		FogExp,
		FogExp2,
		FogLinear
	};
	
	//! Target surface
	enum BufferType {
		ColorBuffer   = (1<<0),
		DepthBuffer   = (1<<1),
		StencilBuffer = (1<<2)
	};
	DECLARE_FLAGS(BufferType, BufferFlags);
	
	enum Primitive {
		TriangleList,
		TriangleStrip,
		TriangleFan,
		LineList,
		LineStrip
	};
	
	enum BufferUsage {
		Static,
		Dynamic,
		Stream
	};
	
	Renderer();
	virtual ~Renderer();
	
	/*!
	 * Basic renderer initialization.
	 * Renderer will not be fully initialized until calling @ref afterResize().
	 * Does *not* notify any listeners.
	 */
	virtual void initialize() = 0;
	
	//! * @return true if the renderer has been fully initialized and is ready for use.
	bool isInitialized() { return m_initialized; }
	
	/*!
	 * Indicate that the renderer's window will be resized and the renderer may need
	 * to temporarily shutdown.
	 * Will notify listeners if the renderer has been shut down.
	 */
	virtual void beforeResize(bool wasOrIsFullscreen) = 0;
	
	/*!
	 * Indicate the renderer's window has been resized and the renderer may need to be
	 * (re-)initialized.
	 * Will notify listeners if the renderer wasn't already initialized.
	 */
	virtual void afterResize() = 0;
	
	void addListener(Listener * listener);
	void removeListener(Listener * listener);
	
	// Scene begin/end...
	virtual void BeginScene() = 0;
	virtual void EndScene() = 0;
	
	// Matrices
	virtual void SetViewMatrix(const EERIEMATRIX & matView) = 0;
	virtual void GetViewMatrix(EERIEMATRIX & matView) const = 0;
	virtual void SetProjectionMatrix(const EERIEMATRIX & matProj) = 0;
	virtual void GetProjectionMatrix(EERIEMATRIX & matProj) const = 0;
	
	// Texture management
	virtual void ReleaseAllTextures() {}
	virtual void RestoreAllTextures() {}

	// Factory
	virtual Texture2D * CreateTexture2D() = 0;
	
	// Render states
	virtual void SetRenderState(RenderState renderState, bool enable) = 0;
	
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
	virtual bool isFogInEyeCoordinates() = 0;
	
	// Rasterizer
	virtual void SetAntialiasing(bool enable) = 0;
	virtual void SetCulling(CullingMode mode) = 0;
	virtual void SetDepthBias(int depthBias) = 0;
	virtual void SetFillMode(FillMode mode) = 0;
	
	// Texturing
	inline unsigned int GetTextureStageCount() const { return m_TextureStages.size(); }
	TextureStage * GetTextureStage(unsigned int textureStage);
	void ResetTexture(unsigned int textureStage);
	void SetTexture(unsigned int textureStage, Texture * pTexture);
	void SetTexture(unsigned int textureStage, TextureContainer * pTextureContainer);
	
	virtual float GetMaxAnisotropy() const = 0;
	
	virtual VertexBuffer<TexturedVertex> * createVertexBufferTL(size_t capacity, BufferUsage usage) = 0;
	virtual VertexBuffer<SMY_VERTEX> * createVertexBuffer(size_t capacity, BufferUsage usage) = 0;
	virtual VertexBuffer<SMY_VERTEX3> * createVertexBuffer3(size_t capacity, BufferUsage usage) = 0;
	
	virtual void drawIndexed(Primitive primitive, const TexturedVertex * vertices, size_t nvertices, unsigned short * indices, size_t nindices) = 0;
	
	virtual bool getSnapshot(Image & image) = 0;
	virtual bool getSnapshot(Image & image, size_t width, size_t height) = 0;
	
protected:
	
	std::vector<TextureStage *> m_TextureStages;
	bool m_initialized;
	
	void onRendererInit();
	void onRendererShutdown();
	
private:
	
	typedef std::vector<Listener *> Listeners;
	
	Listeners m_listeners; //! Listeners for renderer events
	
};

DECLARE_FLAGS_OPERATORS(Renderer::BufferFlags)

extern Renderer * GRenderer;

#endif // ARX_GRAPHICS_RENDERER_H
