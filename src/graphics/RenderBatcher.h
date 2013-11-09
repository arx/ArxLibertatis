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

#ifndef ARX_GRAPHICS_RENDERBATCHER_H
#define ARX_GRAPHICS_RENDERBATCHER_H

#include "graphics/Renderer.h"
#include "graphics/VertexBuffer.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/texture/TextureStage.h"

#include <list>
#include <map>
#include <vector>

struct TexturedQuad {
	TexturedVertex v[4];
};

struct RenderMaterial {

	enum BlendType {
		Opaque,
		Additive,
		AlphaAdditive,
		Screen,
		Subtractive
	};

	RenderMaterial();

	bool operator<(const RenderMaterial & other) const;
	void apply() const;

	Texture * getTexture() const { return texture; }
	void setTexture(Texture * tex) { texture = tex; }
	void setTexture(TextureContainer* texContainer) { texture = texContainer ? (Texture *)texContainer->m_pTexture : NULL; }

	bool getDepthTest() const { return depthTest; }
	void setDepthTest(bool bEnable) { depthTest = bEnable; }

	BlendType getBlendType() const { return blendType; }
	void setBlendType(BlendType type) { blendType = type; }

	TextureStage::WrapMode getWrapMode() const { return wrapMode; }
	void setWrapMode(TextureStage::WrapMode mode) { wrapMode = mode; }

	int getDepthBias() const { return depthBias; }
	void setDepthBias(int bias) { depthBias = bias; }

	Renderer::CullingMode getCulling() const { return cullingMode; }
	void setCulling(Renderer::CullingMode cullMode) { cullingMode = cullMode; }

private:
	Texture * texture;
	bool depthTest;
	BlendType blendType;
	TextureStage::WrapMode wrapMode;
	int depthBias;
	Renderer::CullingMode cullingMode;
};

class RenderBatcher {
public:
	RenderBatcher();
	~RenderBatcher();

	void add(const RenderMaterial& mat, const TexturedVertex(&vertices)[3]);
	void add(const RenderMaterial& mat, const TexturedQuad& sprite);
	void render();
	void clear();
	void reset();

	static RenderBatcher& getInstance();
	
private:
	typedef std::vector<TexturedVertex> VertexBatch;
	typedef std::map<RenderMaterial, VertexBatch*> Batches;
	typedef std::list<VertexBatch*> BufferPool; // Avoid heavy reallocations on each frame

	VertexBatch* requestBuffer();
	void releaseBuffer(VertexBatch* pVertices);
	
private:
	BufferPool m_BufferPool;
	Batches m_BatchedSprites;
	CircularVertexBuffer<TexturedVertex> m_VertexBuffer;
};

#endif // ARX_GRAPHICS_RENDERBATCHER_H
