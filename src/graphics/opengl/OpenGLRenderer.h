/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GRAPHICS_OPENGL_OPENGLRENDERER_H
#define ARX_GRAPHICS_OPENGL_OPENGLRENDERER_H

#include <boost/intrusive/list.hpp>

#include "graphics/Renderer.h"
#include "graphics/opengl/GLTexture.h"
#include "graphics/opengl/OpenGLUtil.h"
#include "math/Rectangle.h"
#include "platform/Platform.h"

class GLTextureStage;

class OpenGLRenderer arx_final : public Renderer {
	
public:
	
	OpenGLRenderer();
	~OpenGLRenderer();
	
	void initialize();
	
	void beforeResize(bool wasOrIsFullscreen);
	void afterResize();
	
	// Matrices
	void SetViewMatrix(const glm::mat4x4 & matView);
	void SetProjectionMatrix(const glm::mat4x4 & matProj);
	
	// Texture management
	void ReleaseAllTextures();
	void RestoreAllTextures();
	void reloadColorKeyTextures();
	
	// Factory
	Texture * createTexture();
	
	// Viewport
	void SetViewport(const Rect & viewport);
	
	void SetScissor(const Rect & rect);
	
	// Render Target
	void Clear(BufferFlags bufferFlags, Color clearColor = Color::none, float clearDepth = 1.f, size_t nrects = 0, Rect * rect = 0);
	
	// Fog
	void SetFogColor(Color color);
	void SetFogParams(float fogStart, float fogEnd);
	
	// Rasterizer
	void SetAntialiasing(bool enable);
	void SetFillMode(FillMode mode);
	
	float getMaxAnisotropy() const { return m_maximumAnisotropy; }
	float getMaxSupportedAnisotropy() const { return m_maximumSupportedAnisotropy; }
	void setMaxAnisotropy(float value);
	
	AlphaCutoutAntialising getMaxSupportedAlphaCutoutAntialiasing() const;
	
	VertexBuffer<TexturedVertex> * createVertexBufferTL(size_t capacity, BufferUsage usage);
	VertexBuffer<SMY_VERTEX> * createVertexBuffer(size_t capacity, BufferUsage usage);
	VertexBuffer<SMY_VERTEX3> * createVertexBuffer3(size_t capacity, BufferUsage usage);
	
	void drawIndexed(Primitive primitive, const TexturedVertex * vertices, size_t nvertices, unsigned short * indices, size_t nindices);
	
	bool getSnapshot(Image & image);
	bool getSnapshot(Image & image, size_t width, size_t height);
	
	GLTextureStage * GetTextureStage(size_t textureStage) {
		return reinterpret_cast<GLTextureStage *>(Renderer::GetTextureStage(textureStage));
	}
	
	template <class Vertex>
	void beforeDraw() { flushState(); selectTrasform<Vertex>(); }
	
	bool hasTextureNPOT() { return m_hasTextureNPOT; }
	bool hasSizedTextureFormats() const { return m_hasSizedTextureFormats; }
	bool hasIntensityTextures() const { return m_hasIntensityTextures; }
	bool hasBGRTextureTransfer() const { return m_hasBGRTextureTransfer; }
	
	bool hasMapBuffer() const { return m_hasMapBuffer; }
	bool hasMapBufferRange() const { return m_hasMapBufferRange; }
	bool hasBufferStorage() const { return m_hasBufferStorage; }
	bool hasBufferUsageStream() const { return m_hasBufferUsageStream; }
	bool hasDrawRangeElements() const { return m_hasDrawRangeElements; }
	bool hasDrawElementsBaseVertex() const { return m_hasDrawElementsBaseVertex; }
	bool hasClearDepthf() const { return m_hasClearDepthf; }
	bool hasVertexFogCoordinate() const { return m_hasVertexFogCoordinate; }
	bool hasSampleShading() const { return m_hasSampleShading; }
	
private:
	
	void shutdown();
	void reinit();
	
	Rect viewport;
	
	void flushState();
	
	template <class Vertex>
	void selectTrasform();
	
	void enableTransform();
	void disableTransform();
	
	friend class GLTextureStage;
	
	size_t maxTextureStage; // the highest active texture stage
	
	float m_maximumAnisotropy;
	float m_maximumSupportedAnisotropy;
	
	typedef boost::intrusive::list<GLTexture, boost::intrusive::constant_time_size<false> > TextureList;
	TextureList textures;
	
	RenderState m_glstate;
	GLenum m_glcull;
	
	Rect m_scissor;
	
	int m_MSAALevel;
	bool m_hasMSAA;
	
	bool m_hasTextureNPOT;
	bool m_hasSizedTextureFormats;
	bool m_hasIntensityTextures;
	bool m_hasBGRTextureTransfer;
	
	bool m_hasMapBuffer;
	bool m_hasMapBufferRange;
	bool m_hasBufferStorage;
	bool m_hasBufferUsageStream;
	bool m_hasDrawRangeElements;
	bool m_hasDrawElementsBaseVertex;
	bool m_hasClearDepthf;
	bool m_hasVertexFogCoordinate;
	bool m_hasSampleShading;
	
};

template <class Vertex>
void OpenGLRenderer::selectTrasform() { enableTransform(); }

template <>
inline void OpenGLRenderer::selectTrasform<TexturedVertex>() { disableTransform(); }

#endif // ARX_GRAPHICS_OPENGL_OPENGLRENDERER_H
