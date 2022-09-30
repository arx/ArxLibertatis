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
/* Based on:
===========================================================================
ARX FATALIS GPL Source Code
Copyright (C) 1999-2010 Arkane Studios SA, a ZeniMax Media company.

This file is part of the Arx Fatalis GPL Source Code ('Arx Fatalis Source Code'). 

Arx Fatalis Source Code is free software: you can redistribute it and/or modify it under the terms of the GNU General Public 
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Arx Fatalis Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Arx Fatalis Source Code.  If not, see 
<http://www.gnu.org/licenses/>.

In addition, the Arx Fatalis Source Code is also subject to certain additional terms. You should have received a copy of these 
additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Arx 
Fatalis Source Code. If not, please request a copy in writing from Arkane Studios at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing Arkane Studios, c/o 
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/
// Initial Code: Cyril Meynier
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved

#include "graphics/effects/PolyBoom.h"

#include <array>
#include <type_traits>

#include "animation/AnimationRender.h"

#include "core/Application.h"
#include "core/Config.h"
#include "core/Core.h"
#include "core/GameTime.h"
#include "core/TimeTypes.h"

#include "game/Entity.h"
#include "game/Player.h"
#include "game/Spells.h"

#include "graphics/Draw.h"
#include "graphics/GlobalFog.h"
#include "graphics/Math.h"
#include "graphics/data/Mesh.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/particle/ParticleTextures.h"
#include "graphics/texture/TextureStage.h"
#include "platform/profiler/Profiler.h"

#include "scene/Light.h"
#include "scene/Interactive.h"
#include "scene/Tiles.h"

#include "util/Range.h"


enum DecalType {
	ScorchMarkDecal,
	BloodDecal,
	WaterDecal
};

struct Decal {
	
	EERIEPOLY * polygon;
	float u[4];
	float v[4];
	Color3f rgb;
	DecalType type;
	bool fastdecay;
	TextureContainer * tc;
	ShortGameDuration elapsed;
	ShortGameDuration duration;
	
};

static_assert(std::is_trivially_copyable_v<Decal>);

static const size_t MAX_POLYBOOM = 4000;
static std::vector<Decal> g_decals;

static const float BOOM_RADIUS = 420.f;

size_t PolyBoomCount() {
	return g_decals.size();
}

void PolyBoomClear() {
	g_decals.clear();
}

void PolyBoomAddScorch(const Vec3f & poss) {
	
	for(auto tile : g_tiles->tilesAround(g_tiles->getTile(poss), 3))  {
		for(EERIEPOLY & polygon : tile.polygons()) {
			
			if((polygon.type & POLY_TRANS) && !(polygon.type & POLY_WATER)) {
				continue;
			}
			
			size_t nbvert = (polygon.type & POLY_QUAD) ? 4 : 3;
			
			float temp_uv1[4];
			
			bool dod = true;
			for(size_t k = 0; k < nbvert; k++) {
				float ddd = fdist(polygon.v[k].p, poss);
				if(ddd > BOOM_RADIUS) {
					dod = false;
					break;
				} else {
					temp_uv1[k] = 0.5f - ddd * (0.5f / BOOM_RADIUS);
				}
			}
			if(!dod) {
				continue;
			}
			
			if(g_decals.size() >= MAX_POLYBOOM) {
				continue;
			}
			
			Decal & decal = g_decals.emplace_back();
			decal.type = ScorchMarkDecal;
			decal.fastdecay = false;
			decal.polygon = &polygon;
			decal.tc = g_particleTextures.boom;
			decal.duration = 10s;
			decal.rgb = Color3f::black;
			for(size_t k = 0; k < nbvert; k++) {
				decal.v[k] = decal.u[k] = temp_uv1[k];
			}
			
		}
	}
	
}

void PolyBoomAddSplat(const Sphere & sp, const Color3f & col, long flags) {
	
	if(g_decals.size() > (MAX_POLYBOOM >> 2) - 30 || (g_decals.size() > 250 && sp.radius < 10)) {
		return;
	}
	
	float splatsize = 90;
	float size = std::min(sp.radius, 40.f) * 0.75f;
	switch(config.video.levelOfDetail) {
		case 2: {
			if(g_decals.size() > 160) {
				return;
			}
			splatsize = 90;
			size *= 1.f;
			break;
		}
		case 1: {
			if(g_decals.size() > 60) {
				return;
			}
			splatsize = 60;
			size *= 0.5f;
			break;
		}
		default: {
			if(g_decals.size() > 10) {
				return;
			}
			splatsize = 30;
			size *= 0.25f;
		}
	}
	
	Vec3f poss = sp.origin;
	float py;
	if(!CheckInPoly(poss + Vec3f(0.f, -40, 0.f), &py)) {
		return;
	}
	
	if(flags & 1) {
		py = poss.y;
	}
	
	EERIEPOLY TheoricalSplat;
	TheoricalSplat.v[0].p.x = -splatsize;
	TheoricalSplat.v[0].p.y = py;
	TheoricalSplat.v[0].p.z = -splatsize;
	TheoricalSplat.v[1].p.x = -splatsize;
	TheoricalSplat.v[1].p.y = py;
	TheoricalSplat.v[1].p.z = splatsize;
	TheoricalSplat.v[2].p.x = splatsize;
	TheoricalSplat.v[2].p.y = py;
	TheoricalSplat.v[2].p.z = splatsize;
	TheoricalSplat.v[3].p.x = splatsize;
	TheoricalSplat.v[3].p.y = py;
	TheoricalSplat.v[3].p.z = -splatsize;
	TheoricalSplat.type = POLY_QUAD;
	
	Vec3f RealSplatStart(-size, py, -size);
	
	TheoricalSplat.v[0].p.x += poss.x;
	TheoricalSplat.v[0].p.z += poss.z;
	
	TheoricalSplat.v[1].p.x += poss.x;
	TheoricalSplat.v[1].p.z += poss.z;
	
	TheoricalSplat.v[2].p.x += poss.x;
	TheoricalSplat.v[2].p.z += poss.z;
	
	TheoricalSplat.v[3].p.x += poss.x;
	TheoricalSplat.v[3].p.z += poss.z;
	
	RealSplatStart.x += poss.x;
	RealSplatStart.z += poss.z;
	
	float div = 1.f / (size * 2);
	
	for(Decal & decal : g_decals) {
		decal.fastdecay = true;
	}
	
	for(auto tile : g_tiles->tilesAround(g_tiles->getTile(poss), 3)) {
		for(EERIEPOLY & polygon : tile.intersectingPolygons()) {
			
			if((flags & 2) && !(polygon.type & POLY_WATER)) {
				continue;
			}
			
			if((polygon.type & POLY_TRANS) && !(polygon.type & POLY_WATER)) {
				continue;
			}
			
			size_t nbvert = (polygon.type & POLY_QUAD) ? 4 : 3;
			
			bool oki = false;
			
			for(size_t k = 0; k < nbvert; k++) {
				
				if(PointIn2DPolyXZ(&TheoricalSplat, polygon.v[k].p.x, polygon.v[k].p.z)
				   && glm::abs(polygon.v[k].p.y - py) < 100.f) {
					oki = true;
					break;
				}
				
				if(PointIn2DPolyXZ(&TheoricalSplat, (polygon.v[k].p.x + polygon.center.x) * 0.5f,
				                                    (polygon.v[k].p.z + polygon.center.z) * 0.5f)
				   && glm::abs(polygon.v[k].p.y - py) < 100.f) {
					oki = true;
					break;
				}
				
			}
			
			if(!oki && PointIn2DPolyXZ(&TheoricalSplat, polygon.center.x, polygon.center.z)
			   && glm::abs(polygon.center.y - py) < 100.f) {
				oki = true;
			}
			
			if(oki && g_decals.size() < MAX_POLYBOOM) {
				
				Decal & decal = g_decals.emplace_back();
				
				if(flags & 2) {
					decal.type = WaterDecal;
					long num = Random::get(0, 2);
					decal.tc = g_particleTextures.water_splat[num];
					decal.duration = 1500ms;
				} else {
					decal.type = BloodDecal;
					long num = Random::get(0, 5);
					decal.tc = g_particleTextures.bloodsplat[num];
					decal.duration = 400ms * size;
				}
				
				decal.fastdecay = false;
				decal.polygon = &polygon;
				decal.rgb = col;
				
				for(size_t k = 0; k < nbvert; k++) {
					
					float vdiff = glm::abs(polygon.v[k].p.y - RealSplatStart.y);
					
					decal.u[k] = (polygon.v[k].p.x - RealSplatStart.x) * div;
					if(decal.u[k] < 0.5f) {
						decal.u[k] -= vdiff * div;
					} else {
						decal.u[k] += vdiff * div;
					}
					
					decal.v[k] = (polygon.v[k].p.z - RealSplatStart.z) * div;
					if(decal.v[k] < 0.5f) {
						decal.v[k] -= vdiff * div;
					} else {
						decal.v[k] += vdiff * div;
					}
					
				}
				
			}
			
		}
	}
	
}


void PolyBoomDraw() {
	
	ARX_PROFILE_FUNC();
	
	ShortGameDuration delta(g_gameTime.lastFrameDuration());
	arx_assume(delta <= ShortGameDuration::max() / 6);
	
	for(Decal & decal : g_decals) {
		arx_assume(decal.elapsed <= ShortGameDuration::max() / 2);
		decal.elapsed += delta * (decal.fastdecay ? 3 : 1);
	}
	
	util::unordered_remove_if(g_decals, [](const Decal & decal) {
		return decal.elapsed >= decal.duration;
	});
	
	GRenderer->SetFogColor(Color::none); // TODO: not handled by RenderMaterial
	
	for(const Decal & decal : g_decals) {
		
		arx_assume(decal.duration > 0 && decal.duration <= ShortGameDuration::max() / 2);
		arx_assume(decal.elapsed >= 0 && decal.elapsed < decal.duration);
		
		float t = 1.f - decal.elapsed / decal.duration;
		
		size_t nbvert = (decal.polygon->type & POLY_QUAD) ? 4 : 3;
		
		std::array<TexturedVertexUntransformed, 4> vertices;
		
		RenderMaterial mat;
		mat.setDepthTest(true);
		mat.setDepthBias(8);
		mat.setLayer(RenderMaterial::Decal);
		mat.setWrapMode(TextureStage::WrapClamp);
		
		switch(decal.type) {
			
			case ScorchMarkDecal: {
				
				ColorRGBA color = (player.m_improve ? Color3f::red * (t * 0.4f) : Color3f::gray(t * 0.8f)).toRGB();
				for(size_t i = 0; i < nbvert; i++) {
					vertices[i].p = decal.polygon->v[i].p;
					vertices[i].uv.x = decal.u[i];
					vertices[i].uv.y = decal.v[i];
					vertices[i].color = color;
				}
				
				mat.setBlendType(player.m_improve ? RenderMaterial::Additive : RenderMaterial::Subtractive);
				
				break;
			}
			
			case BloodDecal: {
				
				float tr = std::max(1.f, t * 2.f - 0.5f);
				ColorRGBA col = Color4f(decal.rgb * t, glm::clamp(t * 1.5f, 0.f, 1.f)).toRGBA();
				
				for(size_t i = 0; i < nbvert; i++) {
					vertices[i].p = decal.polygon->v[i].p;
					vertices[i].uv.x = (decal.u[i] - 0.5f) * tr + 0.5f;
					vertices[i].uv.y = (decal.v[i] - 0.5f) * tr + 0.5f;
					vertices[i].color = col;
				}
				
				mat.setBlendType(RenderMaterial::Subtractive2);
				
				break;
			}
			
			case WaterDecal: {
				
				float tr = std::max(1.f, t * 2.f - 0.5f);
				ColorRGBA col = (decal.rgb * (t * 0.5f)).toRGB();
				
				bool cullXlow = true, cullXhigh = true, cullYlow = true, cullYhigh = true;
				for(size_t i = 0; i < nbvert; i++) {
					vertices[i].p = decal.polygon->v[i].p;
					vertices[i].uv.x = (decal.u[i] - 0.5f) * tr + 0.5f;
					vertices[i].uv.y = (decal.v[i] - 0.5f) * tr + 0.5f;
					vertices[i].color = col;
					cullXlow = cullXlow && vertices[i].uv.x < 0.f;
					cullXhigh = cullXhigh && vertices[i].uv.x > 1.f;
					cullYlow = cullYlow && vertices[i].uv.y < 0.f;
					cullYhigh = cullYhigh && vertices[i].uv.y > 1.f;
				}
				
				if(cullXlow || cullXhigh || cullYlow || cullYhigh) {
					continue;
				}
				
				mat.setBlendType(RenderMaterial::Screen);
				
				break;
			}
			
			default: arx_unreachable();
			
		}
		
		mat.setTexture(decal.tc);
		
		drawTriangle(mat, vertices.data());
		if(nbvert == 4) {
			drawTriangle(mat, vertices.data() + 1);
		}
		
	}
	
	GRenderer->SetFogColor(g_fogColor);
	
}
