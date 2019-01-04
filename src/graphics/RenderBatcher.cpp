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

#include "graphics/RenderBatcher.h"

#include <list>
#include <map>
#include <vector>

#include "graphics/Draw.h"
#include "graphics/texture/Texture.h"

#include "platform/profiler/Profiler.h"


RenderBatcher g_renderBatcher;

RenderBatcher::~RenderBatcher() {
	reset();
}

void RenderBatcher::add(const RenderMaterial & mat, const TexturedVertex (&vertices)[3]) {
	
	VertexBatch & batch = m_BatchedSprites[mat];
	
	batch.push_back(vertices[0]);
	batch.push_back(vertices[1]);
	batch.push_back(vertices[2]);
}

void RenderBatcher::add(const RenderMaterial & mat, const TexturedQuad & sprite) {
	
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
			UseRenderState state(it->first.apply());
			UseTextureState textureState(TextureStage::FilterLinear, it->first.getWrapMode());
			EERIEDRAWPRIM(Renderer::TriangleList, &it->second.front(), it->second.size(), true);
			GRenderer->GetTextureStage(0)->setAlphaOp(TextureStage::OpSelectArg1);
		}
	}
	
	GRenderer->ResetTexture(0);
	
}

void RenderBatcher::clear() {
	
	ARX_PROFILE_FUNC();
	
	for(Batches::iterator itBatch = m_BatchedSprites.begin(); itBatch != m_BatchedSprites.end(); ++itBatch)
		itBatch->second.clear();
}

void RenderBatcher::reset() {
	
	ARX_PROFILE_FUNC();
	
	clear();
	m_BatchedSprites.clear();
}

RenderMaterial::RenderMaterial()
	: m_texture(0)
	, m_depthTest(false)
	, m_blendType(Opaque)
	, m_layer(Effect)
	, m_wrapMode(TextureStage::WrapRepeat)
	, m_depthBias(0)
	, m_cullingMode(CullNone) {
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

RenderState RenderMaterial::apply() const {
	
	if(m_texture) {
		GRenderer->SetTexture(0, m_texture);
	} else {
		GRenderer->ResetTexture(0);
	}
	
	RenderState state = render3D();
	
	state.setAlphaCutout(m_texture && m_texture->hasAlpha());
	
	state.setDepthOffset(m_depthBias);

	state.setDepthTest(m_depthTest);
	
	state.setDepthWrite(false);

	state.setCull(m_cullingMode);
	
	switch(m_blendType) {
		case Opaque: {
			state.disableBlend();
			break;
		}
		case Additive: {
			state.setBlend(BlendOne, BlendOne);
			break;
		}
		case AlphaAdditive: {
			state.setBlend(BlendSrcAlpha, BlendOne);
			break;
		}
		case Screen: {
			state.setBlend(BlendOne, BlendInvSrcColor);
			break;
		}
		case Subtractive: {
			state.setBlend(BlendZero, BlendInvSrcColor);
			break;
		}
		case Subtractive2: {
			GRenderer->GetTextureStage(0)->setAlphaOp(TextureStage::OpModulate);
			state.setBlend(BlendInvSrcAlpha, BlendInvSrcAlpha);
			break;
		}
	}
	
	return state;
}
