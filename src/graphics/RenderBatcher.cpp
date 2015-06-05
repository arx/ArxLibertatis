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

#include "platform/profiler/Profiler.h"

RenderBatcher::RenderBatcher() : m_VertexBuffer(NULL) {
}

RenderBatcher::~RenderBatcher() {
	reset();
}

void RenderBatcher::add(const RenderMaterial& mat, const TexturedVertex (&tri)[3]) {
	
	VertexBatch & batch = m_BatchedSprites[mat];
	
	batch.push_back(tri[0]);
	batch.push_back(tri[1]);
	batch.push_back(tri[2]);
}

void RenderBatcher::add(const RenderMaterial& mat, const TexturedQuad& sprite) {
	
	VertexBatch & batch = m_BatchedSprites[mat];
	
	batch.push_back(sprite.v[0]);
	batch.push_back(sprite.v[1]);
	batch.push_back(sprite.v[2]);
	
	batch.push_back(sprite.v[0]);
	batch.push_back(sprite.v[2]);
	batch.push_back(sprite.v[3]);
}

void RenderBatcher::render() {
	
	ARX_PROFILE_FUNC();
	
	for(Batches::const_iterator it = m_BatchedSprites.begin(); it != m_BatchedSprites.end(); ++it) {
		if(!it->second.empty()) {
			it->first.apply();
			m_VertexBuffer->draw(Renderer::TriangleList, &it->second.front(), it->second.size());
			GRenderer->GetTextureStage(0)->setAlphaOp(TextureStage::OpSelectArg1);
		}
	}

	GRenderer->ResetTexture(0);
	GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapRepeat);
	GRenderer->SetDepthBias(0);
	GRenderer->SetRenderState(Renderer::DepthTest, true);
	GRenderer->SetCulling(Renderer::CullCCW);
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
}

void RenderBatcher::clear() {
	for(Batches::iterator itBatch = m_BatchedSprites.begin(); itBatch != m_BatchedSprites.end(); ++itBatch)
		itBatch->second.clear();
}

void RenderBatcher::reset() {
	
	ARX_PROFILE_FUNC();
	
	clear();
	m_BatchedSprites.clear();
}

u32 RenderBatcher::getMemoryUsed() const {
	u32 memoryUsed = 0;

	for(Batches::const_iterator it = m_BatchedSprites.begin(); it != m_BatchedSprites.end(); ++it) {
		memoryUsed += it->second.capacity() * sizeof(TexturedVertex);
	}

	return memoryUsed;
}

void RenderBatcher::initialize() {
	arx_assert(m_VertexBuffer == NULL);
	m_VertexBuffer = new CircularVertexBuffer<TexturedVertex>(GRenderer->createVertexBufferTL(32 * 1024, Renderer::Stream));
}

void RenderBatcher::shutdown() {
	arx_assert(m_VertexBuffer != NULL);
	delete m_VertexBuffer;
	m_VertexBuffer = NULL;
}

RenderBatcher& RenderBatcher::getInstance() {
	static RenderBatcher renderBatcher;
	return renderBatcher;
}

RenderMaterial::RenderMaterial() 
	: m_texture(0)
	, m_depthTest(false)
	, m_blendType(Opaque)
	, m_layer(Effect)
	, m_wrapMode(TextureStage::WrapRepeat)
	, m_depthBias(0)
	, m_cullingMode(Renderer::CullNone) {
}

bool RenderMaterial::operator<(const RenderMaterial & other) const {
	// First, sort by layer
	if(m_layer != other.m_layer) {
		return m_layer < other.m_layer;
	}

	// Then by blend type
	if(m_blendType != other.m_blendType) {
		return m_blendType < other.m_blendType;
	}

	// Then depth test state
	if(m_depthTest != other.m_depthTest) {
		return m_depthTest;
	}

	// Then depth bias
	if(m_depthBias != other.m_depthBias) {
		return m_depthBias < other.m_depthBias;
	}

	// Then texture
	if(m_texture != other.m_texture) {
		return m_texture < other.m_texture;
	}
	
	// Then cull mode
	if(m_cullingMode != other.m_cullingMode) {
		return m_cullingMode < other.m_cullingMode;
	}

	// Then wrap mode
	if(m_wrapMode != other.m_wrapMode) {
		return m_wrapMode < other.m_wrapMode;
	}	

	// Materials are equals...
	return false;
}

void RenderMaterial::apply() const {
		
	if(m_texture) {
		GRenderer->SetTexture(0, m_texture);
	} else {
		GRenderer->ResetTexture(0);
	}

	GRenderer->GetTextureStage(0)->setWrapMode(m_wrapMode);
	GRenderer->SetDepthBias(m_depthBias);

	GRenderer->SetRenderState(Renderer::DepthTest, m_depthTest);

	GRenderer->SetCulling(m_cullingMode);

	if(m_blendType == Opaque) {
		GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	} else {
		GRenderer->SetRenderState(Renderer::AlphaBlending, true);
		
		switch(m_blendType) {
		
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
		
		case Subtractive2:
			GRenderer->GetTextureStage(0)->setAlphaOp(TextureStage::OpModulate);
			GRenderer->SetBlendFunc(Renderer::BlendInvSrcAlpha, Renderer::BlendInvSrcAlpha);
			break;
		
		default:
			ARX_DEAD_CODE();
		}
	}
}

RenderMaterial RenderMaterial::getCurrent() {
	
	RenderMaterial mat;
	
	mat.setTexture(GRenderer->GetTexture(0));
	if(GRenderer->GetRenderState(Renderer::AlphaBlending)) {
		Renderer::PixelBlendingFactor srcFactor, dstFactor;
		GRenderer->GetBlendFunc(srcFactor, dstFactor);
		
		if(srcFactor == Renderer::BlendOne && dstFactor == Renderer::BlendOne)
			mat.setBlendType(RenderMaterial::Additive);
		else if(srcFactor == Renderer::BlendSrcAlpha && dstFactor == Renderer::BlendOne)
			mat.setBlendType(RenderMaterial::AlphaAdditive);
		else if((srcFactor == Renderer::BlendOne && dstFactor == Renderer::BlendInvSrcColor) ||
				(srcFactor == Renderer::BlendInvDstColor && dstFactor == Renderer::BlendOne))
			mat.setBlendType(RenderMaterial::Screen);
		else if(srcFactor == Renderer::BlendZero && dstFactor == Renderer::BlendInvSrcColor)
			mat.setBlendType(RenderMaterial::Subtractive);
		else
			ARX_DEAD_CODE();
	} else {
		mat.setBlendType(RenderMaterial::Opaque);
	}
	
	mat.setDepthBias(GRenderer->GetDepthBias());
	mat.setWrapMode(GRenderer->GetTextureStage(0)->getWrapMode());
	mat.setDepthTest(GRenderer->GetRenderState(Renderer::DepthTest));
	mat.setCulling(GRenderer->GetCulling());
	
	return mat;
}
