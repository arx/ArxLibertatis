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

#ifndef ARX_GRAPHICS_OPENGL_OPENGLRENDERER_H
#define ARX_GRAPHICS_OPENGL_OPENGLRENDERER_H

#include <map>
#include <boost/intrusive/list.hpp>

#include "graphics/Renderer.h"
#include "graphics/opengl/GLTexture2D.h"
#include "math/Rectangle.h"

class GLTextureStage;

class OpenGLRenderer : public Renderer {
	
public:
	
	OpenGLRenderer();
	~OpenGLRenderer();
	
	void initialize();
	
	void beforeResize(bool wasOrIsFullscreen);
	void afterResize();
	
	// Matrices
	void SetViewMatrix(const glm::mat4x4 & matView);
	void GetViewMatrix(glm::mat4x4 & matView) const;
	void SetProjectionMatrix(const glm::mat4x4 & matProj);
	void GetProjectionMatrix(glm::mat4x4 & matProj) const;
	
	// Texture management
	void ReleaseAllTextures();
	void RestoreAllTextures();

	// Factory
	Texture2D * CreateTexture2D();
	
	// Render states
	bool GetRenderState(RenderState renderState) const;
	void SetRenderState(RenderState renderState, bool enable);
	
	// Alphablending & Transparency
	void SetAlphaFunc(PixelCompareFunc func, float fef); // Ref = [0.0f, 1.0f]
	void GetBlendFunc(PixelBlendingFactor& srcFactor, PixelBlendingFactor& dstFactor) const;
	void SetBlendFunc(PixelBlendingFactor srcFactor, PixelBlendingFactor dstFactor);
	
	// Viewport
	void SetViewport(const Rect & viewport);
	Rect GetViewport();
	
	void SetScissor(const Rect & rect);
	
	// Render Target
	void Clear(BufferFlags bufferFlags, Color clearColor = Color::none, float clearDepth = 1.f, size_t nrects = 0, Rect * rect = 0);
	
	// Fog
	void SetFogColor(Color color);
	void SetFogParams(FogMode fogMode, float fogStart, float fogEnd, float fogDensity = 1.0f);
	
	// Rasterizer
	void SetAntialiasing(bool enable);
	CullingMode GetCulling() const;
	void SetCulling(CullingMode mode);
	int GetDepthBias() const;
	void SetDepthBias(int depthBias);
	void SetFillMode(FillMode mode);
	
	inline float getMaxAnisotropy() const { return m_maximumAnisotropy; }
	
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
	
	bool hasTextureNPOT() { return m_hasTextureNPOT; }
	
private:
	
	void shutdown();
	void reinit();
	
	bool useVertexArrays;
	bool useVBOs;
	
	Rect viewport;
	
	void applyTextureStages();
	
	template <class Vertex>
	void selectTrasform();
	
	void enableTransform();
	void disableTransform();

	bool getGLState(GLenum state) const;
	void setGLState(GLenum state, bool enable);
	
	template <class Vertex>
	inline void beforeDraw() { applyTextureStages(); selectTrasform<Vertex>(); }
	
	template <class Vertex>
	friend class GLNoVertexBuffer;
	template <class Vertex>
	friend class GLVertexBuffer;
	
	friend class GLTextureStage;
	
	size_t maxTextureStage; // the highest active texture stage
	
	GLuint shader;
	
	float m_maximumAnisotropy;
	
	typedef boost::intrusive::list<GLTexture2D, boost::intrusive::constant_time_size<false> > TextureList;
	TextureList textures;

	// State cache...
	void resetStateCache();
	typedef std::map<GLenum, bool> BoolStateCache;
	mutable BoolStateCache m_cachedStates;
	PixelBlendingFactor	m_cachedSrcBlend;
	PixelBlendingFactor	m_cachedDstBlend;
	CullingMode m_cachedCullMode;
	int m_cachedDepthBias;
	
	bool m_hasMSAA;
	bool m_hasColorKey;
	bool m_hasBlend;
	bool m_hasTextureNPOT;
	
};

template <class Vertex>
inline void OpenGLRenderer::selectTrasform() { enableTransform(); }

template <>
inline void OpenGLRenderer::selectTrasform<TexturedVertex>() { disableTransform(); }

#endif // ARX_GRAPHICS_OPENGL_OPENGLRENDERER_H
