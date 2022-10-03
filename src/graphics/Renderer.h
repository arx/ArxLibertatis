/*
 * Copyright 2011-2022 Arx Libertatis Team (see the AUTHORS file)
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
#include <memory>
#include <vector>

#include "graphics/Color.h"
#include "graphics/texture/TextureStage.h"
#include "math/Types.h"
#include "platform/Platform.h"
#include "util/Flags.h"

struct TexturedVertex;
struct SMY_VERTEX;
struct SMY_VERTEX3;
class TextureContainer;
class Image;
class Texture;
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

class RenderState {
	
	static inline constexpr size_t DepthOffsetSize = 5;
	static inline constexpr size_t BlendSize = 4;
	
	enum Offsets {
		Cull,
		Fog,
		AlphaCutout,
		DepthTest,
		DepthWrite,
		DepthOffset,
		BlendSrc = DepthOffset + DepthOffsetSize,
		BlendDst = BlendSrc + BlendSize,
		End = BlendDst + BlendSize
	};
	
	// We could use bitfields here instead but they are missing (an efficient) operator==.
	u32 m_state = 0;
	
	template <size_t Offset, size_t Size>
	[[nodiscard]] constexpr u32 get() const noexcept {
		return (m_state >> Offset) & ((u32(1) << Size) - 1);
	}
	
	template <size_t Offset, size_t Size>
	constexpr void set(u32 value) noexcept {
		m_state = (m_state & ~(((u32(1) << Size) - 1) << Offset)) | (value << Offset);
	}
	
public:
	
	constexpr RenderState() noexcept {
		static_assert(sizeof(m_state) * 8 >= End, "fields do not fit into m_state");
		disableBlend();
	}
	
	[[nodiscard]] constexpr bool operator==(const RenderState & o) const noexcept {
		return m_state == o.m_state;
	}
	[[nodiscard]] constexpr bool operator!=(const RenderState & o) const noexcept {
		return m_state != o.m_state;
	}
	
	constexpr void setCull(bool cullBackfaces) noexcept {
		set<Cull, 1>(cullBackfaces);
	}
	
	[[nodiscard]] constexpr RenderState cull(bool cullBackfaces = true) const noexcept {
		RenderState copy = *this;
		copy.setCull(cullBackfaces);
		return copy;
	}
	
	[[nodiscard]] constexpr bool getCull() const noexcept {
		return get<Cull, 1>() != 0;
	}
	
	constexpr void setFog(bool enable) noexcept {
		set<Fog, 1>(enable);
	}
	
	[[nodiscard]] constexpr RenderState fog(bool enable = true) const noexcept {
		RenderState copy = *this;
		copy.setFog(enable);
		return copy;
	}
	
	[[nodiscard]] constexpr bool getFog() const noexcept {
		return get<Fog, 1>() != 0;
	}
	
	constexpr void setAlphaCutout(bool enable) noexcept {
		set<AlphaCutout, 1>(enable);
	}
	
	[[nodiscard]] constexpr RenderState alphaCutout(bool enable = true) const noexcept {
		RenderState copy = *this;
		copy.setAlphaCutout(enable);
		return copy;
	}
	
	[[nodiscard]] constexpr bool getAlphaCutout() const noexcept {
		return get<AlphaCutout, 1>() != 0;
	}
	
	constexpr void setDepthTest(bool enable) noexcept {
		set<DepthTest, 1>(enable);
	}
	
	[[nodiscard]] constexpr RenderState depthTest(bool enable = true) const noexcept {
		RenderState copy = *this;
		copy.setDepthTest(enable);
		return copy;
	}
	
	[[nodiscard]] constexpr bool getDepthTest() const noexcept {
		return get<DepthTest, 1>() != 0;
	}
	
	constexpr void setDepthWrite(bool enable) noexcept {
		set<DepthWrite, 1>(enable);
	}
	
	[[nodiscard]] constexpr RenderState depthWrite(bool enable = true) const noexcept {
		RenderState copy = *this;
		copy.setDepthWrite(enable);
		return copy;
	}
	
	[[nodiscard]] constexpr bool getDepthWrite() const noexcept {
		return get<DepthWrite, 1>() != 0;
	}
	
	constexpr void setDepthOffset(unsigned offset) noexcept {
		arx_assert(offset < (1u << DepthOffsetSize));
		set<DepthOffset, DepthOffsetSize>(offset);
	}
	
	constexpr void disableDepthOffset() noexcept {
		setDepthOffset(0);
	}
	
	[[nodiscard]] constexpr RenderState depthOffset(unsigned offset) const noexcept {
		RenderState copy = *this;
		copy.setDepthOffset(offset);
		return copy;
	}
	
	[[nodiscard]] constexpr RenderState noDepthOffset() const noexcept {
		RenderState copy = *this;
		copy.disableDepthOffset();
		return copy;
	}
	
	[[nodiscard]] constexpr unsigned getDepthOffset() const noexcept {
		return get<DepthOffset, DepthOffsetSize>();
	}
	
	constexpr void setBlend(BlendingFactor src, BlendingFactor dst) noexcept {
		arx_assert(src >= 0 && unsigned(src) < (1u << BlendSize));
		arx_assert(dst >= 0 && unsigned(dst) < (1u << BlendSize));
		set<BlendSrc, BlendSize>(src);
		set<BlendDst, BlendSize>(dst);
	}
	
	constexpr void setBlendAdditive() noexcept {
		setBlend(BlendSrcAlpha, BlendOne);
	}
	
	constexpr void disableBlend() noexcept {
		setBlend(BlendOne, BlendZero);
	}
	
	[[nodiscard]] constexpr RenderState blend(BlendingFactor src = BlendSrcAlpha,
	                                          BlendingFactor dst = BlendInvSrcAlpha) const noexcept {
		RenderState copy = *this;
		copy.setBlend(src, dst);
		return copy;
	}
	
	[[nodiscard]] constexpr RenderState blendAdditive() const noexcept {
		RenderState copy = *this;
		copy.setBlendAdditive();
		return copy;
	}
	
	[[nodiscard]] constexpr RenderState noBlend() const noexcept {
		RenderState copy = *this;
		copy.disableBlend();
		return copy;
	}
	
	[[nodiscard]] constexpr BlendingFactor getBlendSrc() const noexcept {
		return BlendingFactor(get<BlendSrc, BlendSize>());
	}
	
	[[nodiscard]] constexpr BlendingFactor getBlendDst() const noexcept {
		return BlendingFactor(get<BlendDst, BlendSize>());
	}
	
	[[nodiscard]] constexpr bool isBlendEnabled() const noexcept {
		return getBlendSrc() != BlendOne || getBlendDst() != BlendZero;
	}
	
};

class Renderer {
	
public:
	
	class Listener {
		
	public:
		
		virtual ~Listener() = default;
		
		virtual void onRendererInit(Renderer & renderer) { ARX_UNUSED(renderer); }
		virtual void onRendererShutdown(Renderer & renderer) { ARX_UNUSED(renderer); }
		
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
		ColorBuffer = 1 << 0,
		DepthBuffer = 1 << 1
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
	
	enum AlphaCutoutAntialising {
		NoAlphaCutoutAA = 0,
		FuzzyAlphaCutoutAA = 1,
		CrispAlphaCutoutAA = 2
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
	bool isInitialized() const { return m_initialized; }
	
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
	virtual void SetProjectionMatrix(const glm::mat4x4 & matProj) = 0;
	
	// Texture management
	virtual void ReleaseAllTextures() = 0;
	virtual void RestoreAllTextures() = 0;
	virtual void reloadColorKeyTextures() = 0;

	// Factory
	[[nodiscard]] virtual Texture * createTexture() = 0;
	
	// Viewport
	virtual void SetViewport(const Rect & viewport) = 0;
	
	// Scissor
	virtual void SetScissor(const Rect & rect) = 0;
	
	// Render Target
	virtual void Clear(BufferFlags bufferFlags, Color clearColor = Color(), float clearDepth = 1.f,
	                   size_t nrects = 0, Rect * rect = 0) = 0;
	
	// Fog
	virtual void SetFogColor(Color color) = 0;
	virtual void SetFogParams(float fogStart, float fogEnd) = 0;
	
	// Rasterizer
	virtual void SetAntialiasing(bool enable) = 0;
	virtual void SetFillMode(FillMode mode) = 0;
	
	// Texturing
	[[nodiscard]] size_t getTextureStageCount() const { return m_TextureStages.size(); }
	[[nodiscard]] TextureStage * GetTextureStage(size_t textureStage);
	[[nodiscard]] const TextureStage * GetTextureStage(size_t textureStage) const;
	void ResetTexture(unsigned int textureStage);
	[[nodiscard]] Texture * GetTexture(unsigned int textureStage) const;
	void SetTexture(unsigned int textureStage, Texture * pTexture);
	void SetTexture(unsigned int textureStage, TextureContainer * pTextureContainer);
	
	[[nodiscard]] virtual float getMaxSupportedAnisotropy() const = 0;
	virtual void setMaxAnisotropy(float value) = 0;
	
	[[nodiscard]] virtual AlphaCutoutAntialising getMaxSupportedAlphaCutoutAntialiasing() const = 0;
	
	[[nodiscard]] virtual std::unique_ptr<VertexBuffer<TexturedVertex>> createVertexBufferTL(size_t capacity, BufferUsage usage) = 0;
	[[nodiscard]] virtual std::unique_ptr<VertexBuffer<SMY_VERTEX>> createVertexBuffer(size_t capacity, BufferUsage usage) = 0;
	[[nodiscard]] virtual std::unique_ptr<VertexBuffer<SMY_VERTEX3>> createVertexBuffer3(size_t capacity, BufferUsage usage) = 0;
	
	virtual void drawIndexed(Primitive primitive, const TexturedVertex * vertices, size_t nvertices, unsigned short * indices, size_t nindices) = 0;
	
	virtual bool getSnapshot(Image & image) = 0;
	virtual bool getSnapshot(Image & image, size_t width, size_t height) = 0;
	
	void setRenderState(RenderState state) noexcept { m_state = state; }
	[[nodiscard]] RenderState getRenderState() const noexcept { return m_state; }
	
protected:
	
	std::vector<std::unique_ptr<TextureStage>> m_TextureStages;
	bool m_initialized;
	RenderState m_state;
	
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
 *   UseRenderState state(RenderState().blendAdditive();
 *   // render with additive blending
 * }
 * \endcode
 */
class [[nodiscard]] UseRenderState {
	
	RenderState m_old;
	
public:
	
	explicit UseRenderState(RenderState state) noexcept
		: m_old(GRenderer->getRenderState())
	{
		GRenderer->setRenderState(state);
	}
	
	UseRenderState(const UseRenderState &) = delete;
	UseRenderState & operator=(const UseRenderState &) = delete;
	
	~UseRenderState() {
		GRenderer->setRenderState(m_old);
	}
	
};

/*!
 * RAII helper class to set a texture state for the current scope
 *
 * Sets the requested texture state on construction and restores the old texture state
 * on destruction.
 */
class [[nodiscard]] UseTextureState {
	
	TextureStage::WrapMode m_oldWrapMode;
	TextureStage::FilterMode m_oldMinFilter;
	TextureStage::FilterMode m_oldMagFilter;
	
public:
	
	UseTextureState(TextureStage::FilterMode minFilter, TextureStage::FilterMode magFilter,
	                TextureStage::WrapMode wrapMode)
		: m_oldWrapMode(GRenderer->GetTextureStage(0)->getWrapMode())
		, m_oldMinFilter(GRenderer->GetTextureStage(0)->getMinFilter())
		, m_oldMagFilter(GRenderer->GetTextureStage(0)->getMagFilter())
	{
		GRenderer->GetTextureStage(0)->setWrapMode(wrapMode);
		GRenderer->GetTextureStage(0)->setMinFilter(minFilter);
		GRenderer->GetTextureStage(0)->setMagFilter(magFilter);
	}
	
	UseTextureState(TextureStage::FilterMode filter, TextureStage::WrapMode wrapMode)
		: m_oldWrapMode(GRenderer->GetTextureStage(0)->getWrapMode())
		, m_oldMinFilter(GRenderer->GetTextureStage(0)->getMinFilter())
		, m_oldMagFilter(GRenderer->GetTextureStage(0)->getMagFilter())
	{
		GRenderer->GetTextureStage(0)->setWrapMode(wrapMode);
		GRenderer->GetTextureStage(0)->setMinFilter(filter);
		GRenderer->GetTextureStage(0)->setMagFilter(filter);
	}
	
	UseTextureState(const UseTextureState &) = delete;
	UseTextureState & operator=(const UseTextureState &) = delete;
	
	~UseTextureState() {
		GRenderer->GetTextureStage(0)->setWrapMode(m_oldWrapMode);
		GRenderer->GetTextureStage(0)->setMinFilter(m_oldMinFilter);
		GRenderer->GetTextureStage(0)->setMagFilter(m_oldMagFilter);
	}
	
};

//! Default render state for 2D compositing
[[nodiscard]] inline constexpr RenderState render2D() noexcept {
	return RenderState().blend();
}

//! Default render state for 3D rendering
[[nodiscard]] inline constexpr RenderState render3D() noexcept {
	return RenderState().depthTest().depthWrite().fog();
}

#endif // ARX_GRAPHICS_RENDERER_H
