/*
 * Copyright 2011-2017 Arx Libertatis Team (see the AUTHORS file)
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

#include "graphics/effects/Halo.h"

#include "graphics/Draw.h"
#include "graphics/Vertex.h"

#include "animation/Animation.h"

const size_t HALOMAX = 2000;

static size_t HALOCUR[2] = {};
static TexturedVertex LATERDRAWHALO[2][HALOMAX * 6];

void Halo_AddVertices(TexturedVertex (&inVerts)[4]) {
	int blendType = inVerts[2].color == ColorRGBA_ZERO ? 0 : 1;

	TexturedVertex * vert = &LATERDRAWHALO[blendType][(HALOCUR[blendType] * 6)];
	if(HALOCUR[blendType] < HALOMAX - 1) {
		HALOCUR[blendType]++;
	}

	vert[0] = inVerts[0];
	vert[1] = inVerts[1];
	vert[2] = inVerts[2];
	
	vert[3] = inVerts[0];
	vert[4] = inVerts[2];
	vert[5] = inVerts[3];
}

void Halo_Render() {
	
	if(HALOCUR[0] == 0 && HALOCUR[1] == 0)
		return;
	
	GRenderer->ResetTexture(0);
	
	RenderState baseState = render3D().depthWrite(false);
	
	if(HALOCUR[0] > 0) {
		UseRenderState state(baseState.blend(BlendZero, BlendInvSrcColor));
		EERIEDRAWPRIM(Renderer::TriangleList, LATERDRAWHALO[0], HALOCUR[0] * 6);
		HALOCUR[0] = 0;
	}
	
	if(HALOCUR[1] > 0) {
		UseRenderState state(baseState.blend(BlendSrcColor, BlendOne));
		EERIEDRAWPRIM(Renderer::TriangleList, LATERDRAWHALO[1], HALOCUR[1] * 6);
		HALOCUR[1] = 0;
	}
	
}
