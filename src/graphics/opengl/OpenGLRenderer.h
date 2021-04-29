/*
 * Copyright 2011-2016 Arx Libertatis Team (see the AUTHORS file)
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
	
	// Alphablending & Transparency
	void SetAlphaFunc(PixelCompareFunc func, float fef); // Ref = [0.0f, 1.0f]
	
	// Viewport
	void SetViewport(const Rect & viewport);
	Rect GetViewport();
	
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
	
	VertexBuffer<TexturedVertex> * createVertexBufferTL(size_t capacity, BufferUsage usage);
	VertexBuffer<SMY_VERTEX> * createVertexBuffer(size_t capacity, BufferUsage usage);
	VertexBuffer<SMY_VERTEX3> * createVertexBuffer3(size_t capacity, BufferUsage usage);
	
	void drawIndexed(Primitive primitive, const TexturedVertex * vertices, size_t nvertices, unsigned short * indices, size_t nindices);
	
	bool getSnapshot(Image & image);
	bool getSnapshot(Image & image, size_t width, size_t height);
	
	GLTextureStage * GetTextureStage(unsigned int textureStage) {
		return reinterpret_cast<GLTextureStage *>(Renderer::GetTextureStage(textureStage));
	}
	
	bool hasTextureNPOT() { return m_hasTextureNPOT; }

	template <class Vertex>
	void beforeDraw() { flushState(); selectTrasform<Vertex>(); }
	
private:
	
	void shutdown();
	void reinit();
	
	bool useVertexArrays;
	bool useVBOs;
	
	Rect viewport;
	
	void flushState();
	
	template <class Vertex>
	void selectTrasform();
	
	void enableTransform();
	void disableTransform();
	
	friend class GLTextureStage;
	
	size_t maxTextureStage; // the highest active texture stage
	
	GLuint shader;
	
	float m_maximumAnisotropy;
	float m_maximumSupportedAnisotropy;
	
	typedef boost::intrusive::list<GLTexture2D, boost::intrusive::constant_time_size<false> > TextureList;
	TextureList textures;
	
	RenderState m_glstate;
	GLenum m_glcull;
	
	int m_MSAALevel;
	bool m_hasMSAA;
	bool m_hasTextureNPOT;
	
};

template <class Vertex>
void OpenGLRenderer::selectTrasform() { enableTransform(); }

template <>
inline void OpenGLRenderer::selectTrasform<TexturedVertex>() { disableTransform(); }

#endif // ARX_GRAPHICS_OPENGL_OPENGLRENDERER_H
