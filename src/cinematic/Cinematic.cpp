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

extern float	FlashAlpha;

Cinematic::Cinematic(Vec2i size)
	: pos()
	, angz()
	, m_nextPos()
	, m_nextAngz()
	, numbitmap(-1)
	, m_nextNumbitmap(-1)
	, a()
	, fx(-1)
	, m_nextFx()
	, changekey(true)
	, key(NULL)
	, projectload(false)
	, ti(INTERP_BEZIER)
	, force()
	, color()
	, colord()
	, colorflash()
	, speed()
	, idsound(-1)
	, m_light()
	, m_lightd()
	, posgrille()
	, angzgrille()
	, m_nextPosgrille()
	, m_nextAngzgrille()
	, speedtrack()
	, flTime()
	, cinRenderSize(size)
{ }

Cinematic::~Cinematic() {
	DeleteAllBitmap();
}

/* Reinit */
void Cinematic::OneTimeSceneReInit() {
	
	m_camera.orgTrans.pos = Vec3f(900.f, -160.f, 4340.f);
	m_camera.angle = Anglef(3.f, 268.f, 0.f);
	m_camera.clip = Rect(cinRenderSize.x, cinRenderSize.y);
	m_camera.center = m_camera.clip.center();
	m_camera.focal = 350.f;
	m_camera.bkgcolor = Color::none;
	m_camera.cdepth = 2500.f;
	
	numbitmap = -1;
	m_nextNumbitmap = -1;
	fx = -1;
	changekey = true;
	idsound = -1;
	key = NULL;
	
	projectload = false;
	
	DeleteAllBitmap();
	DeleteAllSound();
	
	DeleteTrack();
	
	FlashBlancEnCours = false;
	
	flicker.reset();
	flickerd.reset();
	
}

void Cinematic::New() {
	
	projectload = false;
	
	numbitmap = -1;
	m_nextNumbitmap = -1;
	fx = -1;
	key = NULL;
	
	DeleteTrack();
	DeleteAllBitmap();
	DeleteAllSound();
	
	AllocTrack(0, 100, 30.f);
	
	{
	CinematicKeyframe key;
	key.frame = 0;
	key.numbitmap = -1;
	key.fx = -1;
	key.typeinterp = INTERP_BEZIER;
	key.force = 1;
	key.pos = pos;
	key.angz = angz;
	key.color = Color(255, 255, 255, 0);
	key.colord = Color(255, 255, 255, 0);
	key.colorf = Color(255, 255, 255, 0);
	key.idsound = -1;
	key.speed = 1.f;
	key.posgrille = posgrille;
	key.angzgrille = angzgrille;
	key.speedtrack = 1.f;
	
	AddKey(key);
	}
	
	{
	CinematicKeyframe key;
	key.frame = 100;
	key.numbitmap = -1;
	key.fx = -1;
	key.typeinterp = INTERP_BEZIER;
	key.force = 1;
	key.pos = pos;
	key.angz = angz;
	key.color = Color(255, 255, 255, 0);
	key.colord = Color(255, 255, 255, 0);
	key.colorf = Color(255, 255, 255, 0);
	key.idsound = -1;
	key.speed = 1.f;
	key.posgrille = posgrille;
	key.angzgrille = angzgrille;
	key.speedtrack = 1.f;
	key.light.intensity = -2.f;
	
	AddKey(key);
	}
	m_lightd = m_light;
	
	SetCurrFrame(GetStartFrame());
	
	projectload = true;
	
	FlashBlancEnCours = false;
	
}

void Cinematic::DeleteAllBitmap()
{
	for(std::vector<CinematicBitmap*>::iterator it = m_bitmaps.begin(); it != m_bitmaps.end(); ++it)
	{
		delete *it;
	}

	m_bitmaps.clear();
}

// Sets RenderStates
void Cinematic::InitDeviceObjects() {
	
	GRenderer->SetRenderState(Renderer::DepthTest, false);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetCulling(CullNone);
	GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapClamp);
	
	GRenderer->GetTextureStage(0)->setMipMapLODBias(0);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetRenderState(Renderer::Fog, false);
	
}

void Cinematic::DeleteDeviceObjects() {
	
	GRenderer->SetRenderState(Renderer::DepthTest, true);
	GRenderer->SetRenderState(Renderer::DepthWrite, true);
	GRenderer->SetCulling(CullCCW);
	GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapRepeat);
	
	GRenderer->GetTextureStage(0)->setMipMapLODBias(0);
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	GRenderer->SetRenderState(Renderer::Fog, true);
	
	GRenderer->GetTextureStage(0)->setColorOp(TextureStage::OpModulate,
	                                          TextureStage::ArgTexture,
	                                          TextureStage::ArgDiffuse);
	GRenderer->GetTextureStage(0)->setAlphaOp(TextureStage::OpModulate,
	                                          TextureStage::ArgTexture,
	                                          TextureStage::ArgDiffuse);
	
}

static float LightRND;

static Color CalculLight(CinematicLight * light, Vec2f pos, Color col) {
	
	float	ra = std::sqrt((light->pos.x - pos.x) * (light->pos.x - pos.x) + (light->pos.y - pos.y) * (light->pos.y - pos.y));

	if(ra > light->fallout) {
		return (col * LightRND);
	} else {
		Color3f color;

		if(ra < light->fallin) {
			color = light->color * LightRND;
		} else {
			ra = (light->fallout - ra) / (light->fallout - light->fallin);
			color = light->color * (LightRND * ra);
		}
		
		Color in = col;
		in.r = std::min(in.r + (int)color.r, 255);
		in.g = std::min(in.g + (int)color.g, 255);
		in.b = std::min(in.b + (int)color.b, 255);
		return in;
	}
}

static Vec3f TransformLocalVertex(const Vec3f & vbase, const Vec3f & LocalPos, float LocalSin, float LocalCos) {
	Vec3f p;
	p.x = vbase.x * LocalCos + vbase.y * LocalSin + LocalPos.x;
	p.y = vbase.x * -LocalSin + vbase.y * LocalCos + LocalPos.y;
	p.z = vbase.z + LocalPos.z;
	return p;
}

void DrawGrille(CinematicBitmap * bitmap, Color col, int fx, CinematicLight * light,
                const Vec3f & pos, float angle, const CinematicFadeOut & fade) {
	
	CinematicGrid * grille = &bitmap->grid;
	int nb = grille->m_nbvertexs;
	Vec3f * v = grille->m_vertexs.data();
	TexturedVertex * d3dv = AllTLVertex;

	Vec3f LocalPos = pos;
	float LocalSin = glm::sin(glm::radians(angle));
	float LocalCos = glm::cos(glm::radians(angle));

	if((fx & CinematicFxPreMask) == FX_DREAM) {
		float * dream = DreamTable;

		while(nb--) {
			Vec3f t;
			t.x = v->x + *dream++;
			t.y = v->y + *dream++;
			t.z = v->z;
			Vec3f vtemp = TransformLocalVertex(t, LocalPos, LocalSin, LocalCos);
			EE_RTP(vtemp, *d3dv);
			if(light) {
				d3dv->color = CalculLight(light, Vec2f(d3dv->p.x, d3dv->p.y), col).toRGBA();
			} else {
				d3dv->color = col.toRGBA();
			}
			v++;
			d3dv++;
		}
	} else {
		while(nb--) {
			Vec3f vtemp = TransformLocalVertex(*v, LocalPos, LocalSin, LocalCos);
			EE_RTP(vtemp, *d3dv);
			if(light) {
				d3dv->color = CalculLight(light, Vec2f(d3dv->p.x, d3dv->p.y), col).toRGBA();
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
		C_INDEXED* mat = &(*it);

		arx_assert(mat->tex);
		GRenderer->SetTexture(0, mat->tex);
		
		int	nb2 = mat->nbvertexs;
		while(nb2--) {
			AllTLVertex[uvs->indvertex].uv = uvs->uv;
			
			if(fadeEdges) {
				
				// Reconstruct position in the bitmap
				Vec2f pos = Vec2f(mat->bitmapdep) + uvs->uv * Vec2f(mat->tex->getStoredSize());
				
				// Roughen up the lines
				Vec2f f = 0.75f + glm::sin(pos) * 0.25f;
				
				float interp = 1.f;
				if(pos.x < fo.left * f.y) {
					interp *= glm::clamp(pos.x / (fo.left * f.y), 0.f, 1.f);
				}
				if(float(bitmap->m_size.x) - pos.x < fo.right * f.y) {
					interp *= glm::clamp((float(bitmap->m_size.x) - pos.x) / (fo.right * f.y), 0.f, 1.f);
				}
				if(pos.y < fo.top * f.x) {
					interp *= glm::clamp(pos.y / (fo.top * f.x), 0.f, 1.f);
				}
				if(float(bitmap->m_size.y) - pos.y < fo.bottom * f.x) {
					interp *= glm::clamp((float(bitmap->m_size.y) - pos.y) / (fo.bottom * f.x), 0.f, 1.f);
				}
				
				if(interp != 1.f) {
					u8 iinterp = interp * (Color::Limits::max() / ColorLimits<float>::max());
					Color color = Color::fromRGBA(AllTLVertex[uvs->indvertex].color);
					color.a = std::min(color.a, iinterp);
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
void Cinematic::Render(float FDIFF) {
	
	bool resized = (cinRenderSize != g_size.size());
	cinRenderSize = g_size.size();
	
	if(!projectload) {
		return;
	}
	
	GRenderer->Clear(Renderer::ColorBuffer);
	
	GereTrack(this, FDIFF, resized, true);
	
	//sound
	if(changekey && idsound >= 0)
		PlaySoundKeyFramer(idsound);
	
	//draw
	GRenderer->SetBlendFunc(BlendSrcAlpha, BlendInvSrcAlpha);
	
	GRenderer->GetTextureStage(0)->setColorOp(TextureStage::OpModulate, TextureStage::ArgTexture, TextureStage::ArgDiffuse);
	GRenderer->GetTextureStage(0)->setAlphaOp(TextureStage::OpModulate, TextureStage::ArgTexture, TextureStage::ArgDiffuse);
	
	GRenderer->GetTextureStage(1)->disableAlpha();
	
	if(config.interface.cinematicWidescreenMode == CinematicLetterbox) {
		float w = 640 * g_sizeRatio.y;
		GRenderer->SetScissor(Rect(Vec2i((g_size.width() - w) / 2, 0), w, g_size.height()));
	}
	
	//image key
	CinematicBitmap * tb = m_bitmaps[numbitmap];
	
	//fx
	Color col = Color(255, 255, 255, 0);
	
	switch(fx & CinematicFxMask) {
		case FX_FADEIN:
			col = FX_FadeIN(a, color, colord);
			break;
		case FX_FADEOUT:
			col = FX_FadeOUT(a, color, colord);
			break;
		case FX_BLUR:
			FX_Blur(this, tb, m_camera);
			break;
		default:
			break;
	}
	
	//fx precalculation
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
	
	m_camera.orgTrans.pos = pos;
	m_camera.angle.setYaw(0);
	m_camera.angle.setPitch(0);
	m_camera.angle.setRoll(angz);
	m_camera.clip = g_size;
	m_camera.center = g_size.center();
	PrepareCamera(&m_camera, g_size);
	SetActiveCamera(&m_camera);
	
	int alpha = (int)(a * 255.f);
	
	if(force ^ 1)
		alpha = 255;
	
	col.a = alpha;
	
	CinematicLight lightt, *l = NULL;
	
	static const float SPEEDINTENSITYRND = 60.f / 1000.f;
	
	if(m_light.intensity >= 0.f && m_lightd.intensity >= 0.f) {
		lightt = m_light;
		
		lightt.pos = lightt.pos * g_sizeRatio.y + Vec3f(g_size.center(), 0.f);
		lightt.fallin *= g_sizeRatio.y;
		lightt.fallout *= g_sizeRatio.y;
		
		flicker.update(FDIFF * SPEEDINTENSITYRND);
		LightRND =  std::min(lightt.intensity + lightt.intensiternd * flicker.get(), 1.f);
		
		l = &lightt;
	}
	
	if(tb->grid.m_nbvertexs)
		DrawGrille(tb, col, fx, l, posgrille, angzgrille, fadegrille);
	
	//PASS #2
	if(force & 1) {
		switch(ti) {
			case INTERP_NO:
				m_camera.orgTrans.pos = m_nextPos;
				m_camera.angle.setYaw(0);
				m_camera.angle.setPitch(0);
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
		
		l = NULL;
		
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
	
	//effets qui continuent avec le temps
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
	
	//post fx
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
		float c = g_size.center().x;
		drawLine(Vec2f(c - x, 0.f), Vec2f(c - x, g_size.height()), 1.f, Color::red);
		drawLine(Vec2f(c + x, 0.f), Vec2f(c + x, g_size.height()), 1.f, Color::red);
		GRenderer->SetFillMode(Renderer::FillSolid);
	}
}
