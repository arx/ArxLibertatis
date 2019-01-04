/*
 * Copyright 2011-2019 Arx Libertatis Team (see the AUTHORS file)
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

#include "cinematic/Cinematic.h"

#include <cmath>
#include <algorithm>

#include "cinematic/CinematicKeyframer.h"

#include "core/Application.h"

#include "cinematic/CinematicFormat.h"
#include "cinematic/CinematicTexture.h"
#include "cinematic/CinematicEffects.h"
#include "core/ArxGame.h"
#include "core/Config.h"
#include "core/Core.h"
#include "graphics/Color.h"
#include "graphics/Math.h"
#include "graphics/Renderer.h"
#include "graphics/Vertex.h"
#include "graphics/data/Mesh.h"
#include "graphics/texture/TextureStage.h"
#include "graphics/texture/Texture.h"
#include "graphics/DrawLine.h"

#include "math/Angle.h"

#include "cinematic/CinematicSound.h"

TexturedVertex AllTLVertex[40000];

extern float DreamTable[];

static bool FlashBlancEnCours;
static float OldSpeedFlashBlanc;
static Color OldColorFlashBlanc;

extern float FlashAlpha;

Cinematic::Cinematic(Vec2i size)
	: m_pos(0.f)
	, angz(0.f)
	, m_nextPos(0.f)
	, m_nextAngz(0.f)
	, numbitmap(-1)
	, m_nextNumbitmap(-1)
	, a(0.f)
	, fx(-1)
	, m_nextFx(0)
	, changekey(true)
	, m_key(NULL)
	, projectload(false)
	, ti(INTERP_BEZIER)
	, force(0)
	, speed(0.f)
	, idsound(-1)
	, posgrille(0.f)
	, angzgrille(0.f)
	, m_nextPosgrille(0.f)
	, m_nextAngzgrille(0.f)
	, speedtrack(0.f)
	, flTime(0)
	, cinRenderSize(size)
{ }

Cinematic::~Cinematic() {
	DeleteAllBitmap();
}

/* Reinit */
void Cinematic::OneTimeSceneReInit() {
	
	m_camera.m_pos = Vec3f(900.f, -160.f, 4340.f);
	m_camera.angle = Anglef(3.f, 268.f, 0.f);
	m_camera.focal = 350.f;
	m_camera.cdepth = 2500.f;
	
	numbitmap = -1;
	m_nextNumbitmap = -1;
	fx = -1;
	changekey = true;
	idsound = -1;
	m_key = NULL;
	
	projectload = false;
	
	DeleteAllBitmap();
	DeleteAllSound();
	
	DeleteTrack();
	
	FlashBlancEnCours = false;
	
	flicker.reset();
	flickerd.reset();
	
}

void Cinematic::DeleteAllBitmap() {
	
	for(std::vector<CinematicBitmap *>::iterator it = m_bitmaps.begin(); it != m_bitmaps.end(); ++it) {
		delete *it;
	}
	
	m_bitmaps.clear();
}

static float LightRND;

static Color CalculLight(CinematicLight * light, Vec2f pos, Color col) {
	
	float distance = glm::distance(Vec2f(light->pos), pos);
	
	if(distance > light->fallout) {
		return col * LightRND;
	}
	
	float attenuation = LightRND / 255.f;
	if(distance >= light->fallin) {
		attenuation *= (light->fallout - distance) / (light->fallout - light->fallin);
	}
	
	return Color(Color3f(col) + light->color * attenuation, col.a);
}

static Vec3f TransformLocalVertex(const Vec2f & vbase, const Vec3f & LocalPos, float LocalSin, float LocalCos) {
	Vec3f p;
	p.x = vbase.x * LocalCos + vbase.y * LocalSin + LocalPos.x;
	p.y = vbase.x * -LocalSin + vbase.y * LocalCos + LocalPos.y;
	p.z = LocalPos.z;
	return p;
}

void DrawGrille(CinematicBitmap * bitmap, Color col, int fx, CinematicLight * light,
                const Vec3f & pos, float angle, const CinematicFadeOut & fade) {
	
	CinematicGrid * grille = &bitmap->grid;
	size_t nb = grille->m_nbvertexs;
	Vec2f * v = grille->m_vertexs.data();
	TexturedVertex * d3dv = AllTLVertex;

	float LocalSin = glm::sin(glm::radians(angle));
	float LocalCos = glm::cos(glm::radians(angle));

	if((fx & CinematicFxPreMask) == FX_DREAM) {
		float * dream = DreamTable;

		while(nb--) {
			Vec2f t;
			t.x = v->x + *dream++;
			t.y = v->y + *dream++;
			Vec3f vtemp = TransformLocalVertex(t, pos, LocalSin, LocalCos);
			worldToClipSpace(vtemp, *d3dv);
			if(light) {
				d3dv->color = CalculLight(light, Vec2f(d3dv->p.x, d3dv->p.y) / d3dv->w, col).toRGBA();
			} else {
				d3dv->color = col.toRGBA();
			}
			v++;
			d3dv++;
		}
	} else {
		while(nb--) {
			Vec3f vtemp = TransformLocalVertex(*v, pos, LocalSin, LocalCos);
			worldToClipSpace(vtemp, *d3dv);
			if(light) {
				d3dv->color = CalculLight(light, Vec2f(d3dv->p.x, d3dv->p.y) / d3dv->w, col).toRGBA();
			} else {
				d3dv->color = col.toRGBA();
			}
			v++;
			d3dv++;
		}
	}
	
	CinematicFadeOut fo;
	bool fadeEdges = (config.interface.cinematicWidescreenMode == CinematicFadeEdges)
		&& (fade.left != 0.f || fade.right != 0.f || fade.top != 0.f || fade.bottom != 0.f);
	if(fadeEdges) {
		fo = fade;
		float fade_witdh = 128.f;
		fo.left   *= fade_witdh;
		fo.right  *= fade_witdh;
		fo.top    *= fade_witdh;
		fo.bottom *= fade_witdh;
	}
	
	C_UV * uvs = grille->m_uvs.data();
	for(std::vector<C_INDEXED>::iterator it = grille->m_mats.begin(); it != grille->m_mats.end(); ++it)
	{
		C_INDEXED * mat = &*it;

		arx_assert(mat->tex);
		GRenderer->SetTexture(0, mat->tex);
		
		int nb2 = mat->nbvertexs;
		while(nb2--) {
			AllTLVertex[uvs->indvertex].uv = uvs->uv;
			
			if(fadeEdges) {
				
				// Reconstruct position in the bitmap
				Vec2f p = Vec2f(mat->bitmapdep) + uvs->uv * Vec2f(mat->tex->getStoredSize());
				
				// Roughen up the lines
				Vec2f f = 0.75f + glm::sin(p) * 0.25f;
				
				float interp = 1.f;
				if(p.x < fo.left * f.y) {
					interp *= glm::clamp(p.x / (fo.left * f.y), 0.f, 1.f);
				}
				if(float(bitmap->m_size.x) - p.x < fo.right * f.y) {
					interp *= glm::clamp((float(bitmap->m_size.x) - p.x) / (fo.right * f.y), 0.f, 1.f);
				}
				if(p.y < fo.top * f.x) {
					interp *= glm::clamp(p.y / (fo.top * f.x), 0.f, 1.f);
				}
				if(float(bitmap->m_size.y) - p.y < fo.bottom * f.x) {
					interp *= glm::clamp((float(bitmap->m_size.y) - p.y) / (fo.bottom * f.x), 0.f, 1.f);
				}
				
				if(interp != 1.f) {
					Color color = Color::fromRGBA(AllTLVertex[uvs->indvertex].color);
					color.a = std::min(color.a, Color::Traits::convert(interp));
					AllTLVertex[uvs->indvertex].color = color.toRGBA();
				}
				
			}
			
			uvs++;
		}
		
		GRenderer->drawIndexed(Renderer::TriangleList, AllTLVertex, grille->m_nbvertexs,
		                       &grille->m_inds.data()->i1 + mat->startind, mat->nbind);
	}
	
}
/*---------------------------------------------------------------*/
void Cinematic::Render(PlatformDuration frameDuration) {
	
	bool resized = (cinRenderSize != g_size.size());
	cinRenderSize = g_size.size();
	
	if(!projectload) {
		return;
	}
	
	
	GRenderer->Clear(Renderer::ColorBuffer);
	
	GereTrack(this, frameDuration, resized, true);
	
	if(changekey && idsound >= 0) {
		PlaySoundKeyFramer(size_t(idsound));
	}
	
	if(config.interface.cinematicWidescreenMode == CinematicLetterbox) {
		s32 w = s32(640 * g_sizeRatio.y);
		GRenderer->SetScissor(Rect(Vec2i((g_size.width() - w) / 2, 0), w, g_size.height()));
	}
	
	UseRenderState state(render2D());
	UseTextureState textureState(TextureStage::FilterLinear, TextureStage::WrapClamp);
	GRenderer->GetTextureStage(0)->setAlphaOp(TextureStage::OpModulate);
	
	CinematicBitmap * tb = m_bitmaps[numbitmap];
	
	// Effect color
	Color col = Color::white;
	
	switch(fx & CinematicFxMask) {
		case FX_FADEIN:
			col = Color(Color4f(color) * a + Color4f(colord) * (1.f - a));
			break;
		case FX_FADEOUT:
			col = Color(Color4f(color) * (1.f - a) + Color4f(colord) * a);
			break;
		case FX_BLUR:
			FX_Blur(this, tb, m_camera);
			break;
		default:
			break;
	}
	
	// Effect precalculation
	switch(fx & CinematicFxPreMask) {
		case FX_DREAM:
			
			if ((m_nextFx & CinematicFxPreMask) == FX_DREAM)
				FX_DreamPrecalc(tb, 15.f, (FPS > 1.f) ? GetTrackFPS() / FPS : 0.f);
			else
				FX_DreamPrecalc(tb, 15.f * a, (FPS > 1.f) ? GetTrackFPS() / FPS : 0.f);
			
			break;
		default:
			break;
	}
	
	arx_assert(isallfinite(m_pos));
	m_camera.m_pos = m_pos;
	m_camera.angle.setPitch(0);
	m_camera.angle.setYaw(0);
	m_camera.angle.setRoll(angz);
	PrepareCamera(&m_camera, g_size);
	
	int alpha = int(a * 255.f);
	
	if(force ^ 1)
		alpha = 255;
	
	col.a = alpha;
	
	
	
	static const float SPEEDINTENSITYRND = 60.f / 1000.f;
	float FDIFF = toMs(frameDuration);
	
	{
		
		CinematicLight * l = NULL;
		CinematicLight lightt;
		if(m_light.intensity >= 0.f && m_lightd.intensity >= 0.f) {
			lightt = m_light;
			
			lightt.pos = lightt.pos * g_sizeRatio.y + Vec3f(g_size.center(), 0.f);
			lightt.fallin *= g_sizeRatio.y;
			lightt.fallout *= g_sizeRatio.y;
			
			flicker.update(FDIFF * SPEEDINTENSITYRND);
			LightRND =  std::min(lightt.intensity + lightt.intensiternd * flicker.get(), 1.f);
			
			l = &lightt;
		}
		
		if(tb->grid.m_nbvertexs) {
			DrawGrille(tb, col, fx, l, posgrille, angzgrille, fadegrille);
		}
		
	}
	
	// Second pass
	if(force & 1) {
		switch(ti) {
			case INTERP_NO:
				arx_assert(isallfinite(m_nextPos));
				
				m_camera.m_pos = m_nextPos;
				m_camera.angle.setPitch(0);
				m_camera.angle.setYaw(0);
				m_camera.angle.setRoll(m_nextAngz);
				PrepareCamera(&m_camera, g_size);
				break;
			case INTERP_LINEAR:
				break;
			case INTERP_BEZIER:
				break;
		}
		
		tb = m_bitmaps[m_nextNumbitmap];
		
		alpha = 255 - alpha;
		col.a = alpha;
		
		CinematicLight * l = NULL;
		CinematicLight lightt;
		
		if(m_light.intensity >= 0.f && m_lightd.intensity >= 0.f) {
			lightt = m_lightd;
			
			lightt.pos = lightt.pos * g_sizeRatio.y + Vec3f(g_size.center(), 0.f);
			lightt.fallin *= g_sizeRatio.y;
			lightt.fallout *= g_sizeRatio.y;
			
			flickerd.update(FDIFF * SPEEDINTENSITYRND);
			LightRND =  std::min(lightt.intensity + lightt.intensiternd * flickerd.get(), 1.f);
			
			l = &lightt;
		}
		
		if(tb->grid.m_nbvertexs)
			DrawGrille(tb, col, fx, l, m_nextPosgrille, m_nextAngzgrille, m_nextFadegrille);
		
	}
	
	GRenderer->GetTextureStage(0)->setAlphaOp(TextureStage::OpSelectArg1);
	
	// Effects that continue over time
	if(FlashBlancEnCours && (fx & CinematicFxPostMask) != FX_FLASH) {
		speed = OldSpeedFlashBlanc;
		colorflash = OldColorFlashBlanc;
		if(fx < 0) {
			fx = FX_FLASH;
		} else {
			fx |= FX_FLASH;
		}
	} else {
		if(changekey) {
			FlashAlpha = 0.f;
		}
		OldSpeedFlashBlanc = speed;
		OldColorFlashBlanc = colorflash;
	}
	
	if(changekey) {
		changekey = false;
	}
	
	// Post-effect
	switch(fx & CinematicFxPostMask) {
		case FX_FLASH:
			FlashBlancEnCours = FX_FlashBlanc(Vec2f(cinRenderSize), speed, colorflash, GetTrackFPS(), FPS);
			break;
		case FX_APPEAR:
			break;
		case FX_APPEAR2:
			break;
		default:
			break;
	}
	
	CalcFPS();
	
	if(config.interface.cinematicWidescreenMode == CinematicLetterbox) {
		GRenderer->SetScissor(Rect::ZERO);
	}
	
	if(g_debugInfo == InfoPanelGuiDebug) {
		GRenderer->SetFillMode(Renderer::FillWireframe);
		float x = 640.f / 2 * g_sizeRatio.y;
		float c = float(g_size.center().x);
		drawLine(Vec2f(c - x, 0.f), Vec2f(c - x, g_size.height()), 1.f, Color::red);
		drawLine(Vec2f(c + x, 0.f), Vec2f(c + x, g_size.height()), 1.f, Color::red);
		GRenderer->SetFillMode(Renderer::FillSolid);
	}
	
}
