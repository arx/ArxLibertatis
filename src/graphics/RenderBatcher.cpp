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

#include "graphics/RenderBatcher.h"

#include <list>
#include <map>
#include <vector>

SpriteBatcher::~SpriteBatcher() {
	reset();
}

void SpriteBatcher::add(const SpriteMaterial& mat, const TexturedVertex (&vertices)[3]) {
	SpriteVertices*& pVerts = m_BatchedSprites[mat];
	if(!pVerts)
		pVerts = requestBuffer();

	pVerts->reserve(pVerts->size() + 3);
		
	pVerts->push_back(vertices[0]);
	pVerts->push_back(vertices[1]);
	pVerts->push_back(vertices[2]);
}

void SpriteBatcher::add(const SpriteMaterial& mat, const TexturedQuad& sprite) {
	SpriteVertices*& pVerts = m_BatchedSprites[mat];
	if(!pVerts)
		pVerts = requestBuffer();

	pVerts->reserve(pVerts->size() + 6);
		
	pVerts->push_back(sprite.v[0]);
	pVerts->push_back(sprite.v[1]);
	pVerts->push_back(sprite.v[2]);

	pVerts->push_back(sprite.v[0]);
	pVerts->push_back(sprite.v[2]);
	pVerts->push_back(sprite.v[3]);
}

void SpriteBatcher::render() const {
	for(Batches::const_iterator it = m_BatchedSprites.begin(); it != m_BatchedSprites.end(); ++it) {
		if(!it->second->empty()) {
			it->first.apply();
			m_VertexBuffer->draw(Renderer::TriangleList, &it->second->front(), it->second->size());
		}
	}
}

void SpriteBatcher::clear() {
	for(Batches::iterator itBatch = m_BatchedSprites.begin(); itBatch != m_BatchedSprites.end(); ++itBatch)
		releaseBuffer(itBatch->second);
	m_BatchedSprites.clear();
}

void SpriteBatcher::reset() {
	clear();
		
	for(BufferPool::iterator it = m_BufferPool.begin(); it != m_BufferPool.end(); ++it) {
		delete *it;
	}

	m_BufferPool.clear();
}
	
SpriteBatcher::SpriteVertices* SpriteBatcher::requestBuffer() {
	static const u32 DEFAULT_NB_SPRITES_PER_BUFFER = 512;
	static const u32 NB_VERTICES_PER_SPRITE = 6;

	SpriteVertices* pVertices = NULL;
	if(!m_BufferPool.empty()) {
		pVertices = m_BufferPool.back();
		m_BufferPool.pop_back();
	} else {
		pVertices = new SpriteVertices();
		pVertices->reserve(DEFAULT_NB_SPRITES_PER_BUFFER * NB_VERTICES_PER_SPRITE);
	}

	return pVertices;
}

void SpriteBatcher::releaseBuffer(SpriteVertices* pVertices) {
	pVertices->clear();
	m_BufferPool.push_back(pVertices);
}

SpriteMaterial::SpriteMaterial() 
	: texture(0)
	, depthTest(false)
	, blendType(Opaque)
	, wrapMode(TextureStage::WrapRepeat)
	, depthBias(0) {
}

bool SpriteMaterial::operator<(const SpriteMaterial & other) const {
	// First sort by blend type
	if(blendType < other.blendType) {
		return true;
	}

	// Then texture
	if(texture < other.texture) {
		return true;
	}

	// Then depth test state
	if(depthTest != other.depthTest) {
		return depthTest;
	}

	// Then wrap mode
	if(wrapMode < other.wrapMode) {
		return true;
	}

	// Then depth bias
	if(depthBias > other.depthBias) {
		return true;
	}

	return false;
}

void SpriteMaterial::apply() const {
		
	if(texture) {
		GRenderer->SetTexture(0, texture);
	} else {
		GRenderer->ResetTexture(0);
	}

	GRenderer->GetTextureStage(0)->setWrapMode(wrapMode);
	GRenderer->SetDepthBias(depthBias);

	GRenderer->SetRenderState(Renderer::DepthTest, depthTest);

	if(blendType == Opaque) {
		GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	} else {
		GRenderer->SetRenderState(Renderer::AlphaBlending, true);

		switch(blendType) {
		case Additive:
			GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
			break;

		case AlphaAdditive:
			GRenderer->SetBlendFunc(Renderer::BlendSrcAlpha, Renderer::BlendOne);
			break;

		case Screen:
			GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendInvSrcColor);
			break;

		case Subtractive:
			GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);
			break;

		default:
			arx_error_msg("Invalid blend type.");
		}
	}
}
