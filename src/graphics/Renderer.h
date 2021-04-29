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

#include <stddef.h>
#include <vector>

#include "graphics/Color.h"
#include "math/Types.h"
#include "platform/Platform.h"
#include "util/Flags.h"

struct TexturedVertex;
struct SMY_VERTEX;
struct SMY_VERTEX3;
class TextureContainer;
class TextureStage;
class Image;
class Texture;
class Texture2D;
template <class Vertex> class VertexBuffer;

enum BlendingFactor {
	BlendZero,              //!< Zero
	BlendOne,               //!< One
	BlendSrcColor,          //!< Source color
	BlendSrcAlpha,          //!< Source alpha
	BlendInvSrcColor,       //!< One minus source color
	BlendInvSrcAlpha,       //!< One minus source alpha
	BlendSrcAlphaSaturate,  //!< Source alpha saturate
	BlendDstColor,          //!< Destination color
	BlendDstAlpha,          //!< Destination alpha
	BlendInvDstColor,       //!< One minus destination color
	BlendInvDstAlpha        //!< One minus destination alpha
};

enum CullingMode {
	CullNone,
	CullCW,
	CullCCW
};

class RenderState {
	
	enum Sizes {
		CullSize = 2,
		DepthOffsetSize = 5,
		BlendSize = 4,
	};
	
	enum Offsets {
		Cull,
		Fog = Cull + CullSize,
		ColorKey,
		DepthTest,
		DepthWrite,
		DepthOffset,
		BlendSrc = DepthOffset + DepthOffsetSize,
		BlendDst = BlendSrc + BlendSize,
		End = BlendDst + BlendSize
	};
	
	// We coul use bitfields here instead but they are missing (an efficient) operator==.
	u32 m_state;
	
	template <size_t Offset, size_t Size>
	u32 get() const {
		return (m_state >> Offset) & ((u32(1) << Size) - 1);
	}
	
	template <size_t Offset, size_t Size>
	void set(u32 value) {
		m_state = (m_state & ~(((u32(1) << Size) - 1) << Offset)) | (value << Offset);
	}
	
public:
	
	RenderState()
		: m_state(0)
	{
		ARX_STATIC_ASSERT(sizeof(m_state) * 8 >= End, "fields do not fit into m_state");
		disableBlend();
	}
	
	bool operator==(const RenderState & o) { return m_state == o.m_state; }
	bool operator!=(const RenderState & o) { return m_state != o.m_state; }
	
	void setCull(CullingMode mode) {
		arx_assert(mode >= 0 && unsigned(mode) < (1u << CullSize));
		set<Cull, CullSize>(mode);
	}
	
	RenderState cull(CullingMode mode = CullCCW) const {
		RenderState copy = *this;
		copy.setCull(mode);
		return copy;
	}
	
	CullingMode getCull() const {
		return CullingMode(get<Cull, CullSize>());
	}
	
	void setFog(bool enable) {
		set<Fog, 1>(enable);
	}
	
	RenderState fog(bool enable = true) const {
		RenderState copy = *this;
		copy.setFog(enable);
		return copy;
	}
	
	bool getFog() const {
		return get<Fog, 1>() != 0;
	}
	
	void setColorKey(bool enable) {
		set<ColorKey, 1>(enable);
	}
	
	RenderState colorKey(bool enable = true) const {
		RenderState copy = *this;
		copy.setColorKey(enable);
		return copy;
	}
	
	bool getColorKey() const {
		return get<ColorKey, 1>() != 0;
	}
	
	void setDepthTest(bool enable) {
		set<DepthTest, 1>(enable);
	}
	
	RenderState depthTest(bool enable = true) const {
		RenderState copy = *this;
		copy.setDepthTest(enable);
		return copy;
	}
	
	bool getDepthTest() const {
		return get<DepthTest, 1>() != 0;
	}
	
	void setDepthWrite(bool enable) {
		set<DepthWrite, 1>(enable);
	}
	
	RenderState depthWrite(bool enable = true) const {
		RenderState copy = *this;
		copy.setDepthWrite(enable);
		return copy;
	}
	
	bool getDepthWrite() const {
		return get<DepthWrite, 1>() != 0;
	}
	
	void setDepthOffset(unsigned offset) {
		arx_assert(offset < (1u << DepthOffsetSize));
		set<DepthOffset, DepthOffsetSize>(offset);
	}
	
	void disableDepthOffset() {
		setDepthOffset(0);
	}
	
	RenderState depthOffset(unsigned offset) const {
		RenderState copy = *this;
		copy.setDepthOffset(offset);
		return copy;
	}
	
	RenderState noDepthOffset() const {
		RenderState copy = *this;
		copy.disableDepthOffset();
		return copy;
	}
	
	unsigned getDepthOffset() const {
		return get<DepthOffset, DepthOffsetSize>();
	}
	
	void setBlend(BlendingFactor src, BlendingFactor dst) {
		arx_assert(src >= 0 && unsigned(src) < (1u << BlendSize));
		arx_assert(dst >= 0 && unsigned(dst) < (1u << BlendSize));
		set<BlendSrc, BlendSize>(src);
		set<BlendDst, BlendSize>(dst);
	}
	
	void disableBlend() {
		setBlend(BlendOne, BlendZero);
	}
	
	RenderState blend(BlendingFactor src = BlendSrcAlpha,
	                  BlendingFactor dst = BlendInvSrcAlpha) const {
		RenderState copy = *this;
		copy.setBlend(src, dst);
		return copy;
	}
	
	RenderState noBlend() const {
		RenderState copy = *this;
		copy.disableBlend();
		return copy;
	}
	
	BlendingFactor getBlendSrc() const {
		return BlendingFactor(get<BlendSrc, BlendSize>());
	}
	
	BlendingFactor getBlendDst() const {
		return BlendingFactor(get<BlendDst, BlendSize>());
	}
	
	bool isBlendEnabled() {
		return getBlendSrc() != BlendOne || getBlendDst() != BlendZero;
	}
	
};

class Renderer {
	
public:
	
	class Listener {
		
	public:
		
		virtual ~Listener() { }
		
		virtual void onRendererInit(Renderer &) { }
		virtual void onRendererShutdown(Renderer &) { }
		
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
	
	enum FillMode {
		FillWireframe,
		FillSolid
	};
	
	//! Target surface
	enum BufferType {
		ColorBuffer   = (1<<0),
		DepthBuffer   = (1<<1)
	};
	DECLARE_FLAGS(BufferType, BufferFlags)
	
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
	 * Renderer will not be fully initialized until calling \ref afterResize().
	 * Does *not* notify any listeners.
	 */
	virtual void initialize() = 0;
	
	//! * \return true if the renderer has been fully initialized and is ready for use.
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
	
	// Matrices
	virtual void SetViewMatrix(const glm::mat4x4 & matView) = 0;
	virtual void GetViewMatrix(glm::mat4x4 & matView) const = 0;
	virtual void SetProjectionMatrix(const glm::mat4x4 & matProj) = 0;
	virtual void GetProjectionMatrix(glm::mat4x4 & matProj) const = 0;
	
	// Texture management
	virtual void ReleaseAllTextures() {}
	virtual void RestoreAllTextures() {}

	// Factory
	virtual Texture2D * CreateTexture2D() = 0;
	
	virtual void SetAlphaFunc(PixelCompareFunc func, float fef) = 0; // Ref = [0.0f, 1.0f]
	
	// Viewport
	virtual void SetViewport(const Rect & viewport) = 0;
	virtual Rect GetViewport() = 0;
	
	// Scissor
	virtual void SetScissor(const Rect & rect) = 0;
	
	// Render Target
	virtual void Clear(BufferFlags bufferFlags, Color clearColor = Color::none, float clearDepth = 1.f, size_t nrects = 0, Rect * rect = 0) = 0;
	
	// Fog
	virtual void SetFogColor(Color color) = 0;
	virtual void SetFogParams(float fogStart, float fogEnd) = 0;
	virtual bool isFogInEyeCoordinates() = 0;
	
	// Rasterizer
	virtual void SetAntialiasing(bool enable) = 0;
	virtual void SetFillMode(FillMode mode) = 0;
	
	// Texturing
	unsigned int GetTextureStageCount() const { return m_TextureStages.size(); }
	TextureStage * GetTextureStage(unsigned int textureStage);
	const TextureStage * GetTextureStage(unsigned int textureStage) const;
	void ResetTexture(unsigned int textureStage);
	Texture * GetTexture(unsigned int textureStage) const;
	void SetTexture(unsigned int textureStage, Texture * pTexture);
	void SetTexture(unsigned int textureStage, TextureContainer * pTextureContainer);
	
	virtual float getMaxSupportedAnisotropy() const = 0;
	virtual void setMaxAnisotropy(float value) = 0;
	
	virtual VertexBuffer<TexturedVertex> * createVertexBufferTL(size_t capacity, BufferUsage usage) = 0;
	virtual VertexBuffer<SMY_VERTEX> * createVertexBuffer(size_t capacity, BufferUsage usage) = 0;
	virtual VertexBuffer<SMY_VERTEX3> * createVertexBuffer3(size_t capacity, BufferUsage usage) = 0;
	
	virtual void drawIndexed(Primitive primitive, const TexturedVertex * vertices, size_t nvertices, unsigned short * indices, size_t nindices) = 0;
	
	virtual bool getSnapshot(Image & image) = 0;
	virtual bool getSnapshot(Image & image, size_t width, size_t height) = 0;
	
	void setRenderState(RenderState state) { m_state = state; }
	RenderState getRenderState() const { return m_state; }
	
	// TODO remove these when all uses are changed to RenderState
	enum RenderStateFlag {
		AlphaBlending,
		ColorKey,
		DepthTest,
		DepthWrite,
		Fog,
	};
	void SetRenderState(RenderStateFlag renderState, bool enable);
	void SetCulling(CullingMode mode) { m_state.setCull(mode); }
	void SetDepthBias(int depthBias) { m_state.setDepthOffset(depthBias); }
	void SetBlendFunc(BlendingFactor srcFactor, BlendingFactor dstFactor) {
		m_srcBlend = srcFactor, m_dstBlend = dstFactor;
		if(m_hasBlend) {
			m_state.setBlend(srcFactor, dstFactor);
		}
	}
	
protected:
	
	std::vector<TextureStage *> m_TextureStages;
	bool m_initialized;
	RenderState m_state;
	
	// TODO remove these when all uses are changed to RenderState
	bool m_hasBlend;
	BlendingFactor m_srcBlend;
	BlendingFactor m_dstBlend;
	
	void onRendererInit();
	void onRendererShutdown();
	
private:
	
	typedef std::vector<Listener *> Listeners;
	
	Listeners m_listeners; //! Listeners for renderer events
	
};

DECLARE_FLAGS_OPERATORS(Renderer::BufferFlags)

extern Renderer * GRenderer;

/*!
 * RAII helper class to set a render state for the current scope
 *
 * Sets the requested render state on construction and restores the old render state
 * on destruction.
 *
 * Example usage:
 * \code
 * {
 *   UseRenderState state(RenderState().blend(BlendOne, BlendOne);
 *   // render with additive blending
 * }
 * \endcode
 */
class UseRenderState {
	
	RenderState  m_old;
	
public:
	
	explicit UseRenderState(RenderState state)
		: m_old(GRenderer->getRenderState())
	{
		GRenderer->setRenderState(state);
	}
	
	~UseRenderState() {
		GRenderer->setRenderState(m_old);
	}
	
};

//! Default render state for 2D compositing
inline RenderState render2D() {
	return RenderState().blend();
}

//! Default render state for 3D rendering
inline RenderState render3D() {
	return RenderState().depthTest().depthWrite().cull().colorKey();
}

#endif // ARX_GRAPHICS_RENDERER_H
