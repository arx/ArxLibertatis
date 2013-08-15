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

#include "graphics/effects/Halo.h"

#include "graphics/Draw.h"
#include "graphics/Vertex.h"

#include "animation/Animation.h"

const size_t HALOMAX = 2000;

long HALOCUR = 0;
TexturedVertex LATERDRAWHALO[HALOMAX * 4];

TexturedVertex * Halo_AddVertex() {
	TexturedVertex *vert = &LATERDRAWHALO[(HALOCUR << 2)];

	if(HALOCUR < ((long)HALOMAX) - 1) {
		HALOCUR++;
	}
	return vert;
}

void Halo_Render() {
	if(HALOCUR > 0) {
		GRenderer->ResetTexture(0);
		GRenderer->SetBlendFunc(Renderer::BlendSrcColor, Renderer::BlendOne);
		GRenderer->SetRenderState(Renderer::AlphaBlending, true);
		GRenderer->SetCulling(Renderer::CullNone);
		GRenderer->SetRenderState(Renderer::DepthWrite, false);

		for(int i=0; i < HALOCUR; i++) {
			TexturedVertex *vert = &LATERDRAWHALO[(i<<2)];

			if(vert[2].color == 0) {
				GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);
				vert[2].color =0xFF000000;
				EERIEDRAWPRIM(Renderer::TriangleFan, vert, 4);
				GRenderer->SetBlendFunc(Renderer::BlendSrcColor, Renderer::BlendOne);
			} else {
				EERIEDRAWPRIM(Renderer::TriangleFan, vert, 4);
			}
		}

		HALOCUR = 0;
		GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	}
}
