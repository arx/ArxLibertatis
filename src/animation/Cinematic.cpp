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

#include "animation/Cinematic.h"

#include <cmath>
#include <algorithm>

#include "animation/CinematicKeyframer.h"

#include "core/Application.h"

#include "graphics/Color.h"
#include "graphics/Math.h"
#include "graphics/Renderer.h"
#include "graphics/Vertex.h"
#include "graphics/data/Mesh.h"
#include "graphics/data/CinematicTexture.h"
#include "graphics/effects/CinematicEffects.h"
#include "graphics/texture/TextureStage.h"
#include "graphics/texture/Texture.h"

#include "math/Angle.h"

#include "scene/CinematicSound.h"

static const int WIDTHS = 512;
static const int HEIGHTS = 384;

static int LargeurRender, HauteurRender;

TexturedVertex AllTLVertex[40000];

extern float DreamTable[];

C_KEY KeyTemp;

static bool FlashBlancEnCours;
static float OldSpeedFlashBlanc;
static int OldColorFlashBlanc;

extern float	FlashAlpha;
extern float SpecialFadeDx;

extern Rect g_size;

float ADJUSTX(float a) {
	return (((((a)-(WIDTHS>>1))*((float)LargeurRender/(float)WIDTHS))+(WIDTHS>>1)))*(640.f/(float)LargeurRender); //*((float)LARGEURS/(float)LargeurRender)
}

float ADJUSTY(float a) {
	return (((((a)-(HEIGHTS>>1))*((float)HauteurRender/(float)HEIGHTS))+(HEIGHTS>>1)))*(480.f/(float)HauteurRender);  //*((float)HAUTEURS/(float)HauteurRender)
}

Cinematic::Cinematic(int _w, int _h) {
	
	LargeurRender = _w;
	HauteurRender = _h;
	
	numbitmap = -1;
	numbitmapsuiv = -1;
	fx = -1;
	changekey = true;
	idsound = -1;
	key = NULL;
	projectload = false; 
	ti = tichoose = INTERP_BEZIER;
	speedchoose = 1.f;
	
	m_flIntensityRND = 0.f;
}

Cinematic::~Cinematic() {
	DeleteAllBitmap();
}

/*-------------------------------------------------------------------*/
void FillKeyTemp(Vec3f * pos, float az, int frame, int numbitmap, int numfx, short ti, int color, int colord, int colorf, float speed, int idsound, short force, CinematicLight * light, Vec3f * posgrille, float azgrille, float speedtrack)
{
	KeyTemp.frame = frame;
	KeyTemp.numbitmap = numbitmap;
	KeyTemp.fx = numfx;
	KeyTemp.typeinterp = ti;
	KeyTemp.force = force;
	KeyTemp.pos = *pos;
	KeyTemp.angz = az;
	KeyTemp.color = color;
	KeyTemp.colord = colord;
	KeyTemp.colorf = colorf;
	KeyTemp.idsound = idsound;
	KeyTemp.speed = speed;
	KeyTemp.posgrille = *posgrille;
	KeyTemp.angzgrille = azgrille;
	KeyTemp.speedtrack = speedtrack;

	if (light)
	{
		KeyTemp.light = *light;
	}
	else
	{
		KeyTemp.light.intensity = -2.f;
	}
}

/* Reinit */
void Cinematic::OneTimeSceneReInit() {
	
	m_camera.size = Anglef(160.f, 60.f, 60.f);
	m_camera.orgTrans.pos = Vec3f(900.f, -160.f, 4340.f);
	m_camera.angle = Anglef(3.f, 268.f, 0.f);
	m_camera.clip = Rect(LargeurRender, HauteurRender);
	m_camera.center = m_camera.clip.center();
	m_camera.focal = 350.f;
	m_camera.bkgcolor = Color::none;

	SetCameraDepth(m_camera, 2500.f);
	
	numbitmap = -1;
	numbitmapsuiv = -1;
	fx = -1;
	changekey = true;
	idsound = -1;
	key = NULL;
	
	projectload = false;
	
	DeleteAllBitmap();
	DeleteAllSound();
	
	DeleteTrack();
	
	FlashBlancEnCours = false;
	
	m_flIntensityRND = 0.f;
	
}

void Cinematic::New() {
	
	projectload = false;
	
	numbitmap = -1;
	numbitmapsuiv = -1;
	fx = -1;
	key = NULL;
	
	DeleteTrack();
	DeleteAllBitmap();
	DeleteAllSound();
	
	AllocTrack(0, 100, 30.f);
	FillKeyTemp(&pos, angz, 0, -1, -1, INTERP_BEZIER, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 1.f, -1, 1, NULL, &posgrille, angzgrille, 1.f);
	AddKey(&KeyTemp, true, true, true);
	FillKeyTemp(&pos, angz, 100, -1, -1, INTERP_BEZIER, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 1.f, -1, 1, NULL, &posgrille, angzgrille, 1.f);
	AddKey(&KeyTemp, true, true, true);
	this->lightd = this->lightchoose = this->light;
	
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
	GRenderer->SetCulling(Renderer::CullNone);
	GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapClamp);
	
	GRenderer->GetTextureStage(0)->setMipMapLODBias(0);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetRenderState(Renderer::Fog, false);
	
}

void Cinematic::DeleteDeviceObjects() {
	
	GRenderer->SetRenderState(Renderer::DepthTest, true);
	GRenderer->SetRenderState(Renderer::DepthWrite, true);
	GRenderer->SetCulling(Renderer::CullCCW);
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

int CalculLight(CinematicLight * light, float x, float y, int col)
{
	float	ra = (float)sqrt((light->pos.x - x) * (light->pos.x - x) + (light->pos.y - y) * (light->pos.y - y));

	if(ra > light->fallout) {
		return (Color::fromBGRA(col) * LightRND).toBGRA();
	}
	else
	{
		Color3f color;

		if(ra < light->fallin) {
			color = light->color * LightRND;
		} else {
			ra = (light->fallout - ra) / (light->fallout - light->fallin);
			color = light->color * (LightRND * ra);
		}
		
		Color in = Color::fromBGRA(col);
		in.r = min(in.r + (int)color.r, 255);
		in.g = min(in.g + (int)color.g, 255);
		in.b = min(in.b + (int)color.b, 255);
		return in.toBGRA();
	}
}

static Vec3f LocalPos;
static float LocalSin, LocalCos;

void TransformLocalVertex(Vec3f * vbase, TexturedVertex * d3dv) {
	d3dv->p.x = vbase->x * LocalCos + vbase->y * LocalSin + LocalPos.x;
	d3dv->p.y = vbase->x * -LocalSin + vbase->y * LocalCos + LocalPos.y;
	d3dv->p.z = vbase->z + LocalPos.z;
}

void DrawGrille(CinematicGrid * grille, int col, int fx, CinematicLight * light, Vec3f * posgrille, float angzgrille)
{
	int nb = grille->nbvertexs;
	Vec3f * v = grille->vertexs;
	TexturedVertex * d3dv = AllTLVertex;

	LocalPos = *posgrille;
	LocalSin = (float)sin(radians(angzgrille));
	LocalCos = (float)cos(radians(angzgrille));

	if((fx & 0x0000FF00) == FX_DREAM) {
		float * dream = DreamTable;

		while(nb--) {
			TexturedVertex vtemp;
			Vec3f t;
			t.x = v->x + *dream++;
			t.y = v->y + *dream++;
			t.z = v->z;
			TransformLocalVertex(&t, &vtemp);
			EE_RTP(&vtemp, d3dv);
			if(light) {
				d3dv->color = CalculLight(light, d3dv->p.x, d3dv->p.y, col);
				d3dv->p.x = ADJUSTX(d3dv->p.x);
				d3dv->p.y = ADJUSTY(d3dv->p.y);
			} else {
				d3dv->p.x = ADJUSTX(d3dv->p.x);
				d3dv->p.y = ADJUSTY(d3dv->p.y);
				d3dv->color = col;
			}
			v++;
			d3dv++;
		}
	} else {
		if(light) {
			while(nb--) {
				TexturedVertex vtemp;
				TransformLocalVertex(v, &vtemp);
				EE_RTP(&vtemp, d3dv);
				d3dv->color = CalculLight(light, d3dv->p.x, d3dv->p.y, col);
				d3dv->p.x = ADJUSTX(d3dv->p.x);
				d3dv->p.y = ADJUSTY(d3dv->p.y);
				v++;
				d3dv++;
			}
		} else {
			while(nb--) {
				TexturedVertex vtemp;
				TransformLocalVertex(v, &vtemp);
				EE_RTP(&vtemp, d3dv);
				d3dv->p.x = ADJUSTX(d3dv->p.x);
				d3dv->p.y = ADJUSTY(d3dv->p.y);
				d3dv->color = col;
				v++;
				d3dv++;
			}
		}
	}

	C_UV* uvs = grille->uvs;

	for(std::vector<C_INDEXED>::iterator it = grille->mats.begin(); it != grille->mats.end(); ++it)
	{
		C_INDEXED* mat = &(*it);

		if (mat->tex)
			GRenderer->SetTexture(0, mat->tex);
		else
			GRenderer->ResetTexture(0);
		
		int	nb2 = mat->nbvertexs;
		while(nb2--) {
			AllTLVertex[uvs->indvertex].uv = uvs->uv;
			uvs++;
		}
		
		GRenderer->drawIndexed(Renderer::TriangleList, AllTLVertex, grille->nbvertexs,
		                       &grille->inds->i1 + mat->startind, mat->nbind);
	}
}
/*---------------------------------------------------------------*/
void Cinematic::Render(float FDIFF) {
	
	CinematicBitmap * tb;

	LargeurRender = g_size.width();
	HauteurRender = g_size.height();

	if(projectload) {
		GRenderer->Clear(Renderer::ColorBuffer);
		GRenderer->BeginScene();

		GereTrack(this, FDIFF);

		//sound
		if(changekey && idsound >= 0)
			PlaySoundKeyFramer(idsound);

		//draw
		GRenderer->SetBlendFunc(Renderer::BlendSrcAlpha, Renderer::BlendInvSrcAlpha);

		GRenderer->GetTextureStage(0)->setColorOp(TextureStage::OpModulate, TextureStage::ArgTexture, TextureStage::ArgDiffuse);
		GRenderer->GetTextureStage(0)->setAlphaOp(TextureStage::OpModulate, TextureStage::ArgTexture, TextureStage::ArgDiffuse);

		GRenderer->GetTextureStage(1)->disableAlpha();
		
		//image key
		tb = m_bitmaps[numbitmap];

		//fx
		int col = 0x00FFFFFF;

		switch(fx & 0x000000FF) {
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
		switch(fx & 0x0000ff00) {
			case FX_DREAM:

				if ((this->fxsuiv & 0x0000ff00) == FX_DREAM)
					FX_DreamPrecalc(tb, 15.f, (FPS > 1.f) ? GetTrackFPS() / FPS : 0.f);
				else
					FX_DreamPrecalc(tb, 15.f * a, (FPS > 1.f) ? GetTrackFPS() / FPS : 0.f);

				break;
			default:
				break;
		}

		m_camera.orgTrans.pos = pos;
		m_camera.setTargetCamera(m_camera.orgTrans.pos.x, m_camera.orgTrans.pos.y, 0.f);
		m_camera.angle.setPitch(0);
		m_camera.angle.setRoll(angz);
		m_camera.clip = Rect(LargeurRender, HauteurRender);
		m_camera.center = m_camera.clip.center();
		PrepareCamera(&m_camera);
		SetActiveCamera(&m_camera);

		int alpha = ((int)(a * 255.f)) << 24;

		if(force ^ 1)
			alpha = 0xFF000000;

		col |= alpha;

		CinematicLight lightt, *l = NULL;

		if(this->light.intensity >= 0.f && this->lightd.intensity >= 0.f) {
			lightt = this->light;
			lightt.pos.x += (float)(LargeurRender >> 1);
			lightt.pos.y += (float)(HauteurRender >> 1);

			static const float SPEEDINTENSITYRND = 10.f;
			float flIntensityRNDToReach = lightt.intensiternd * rnd();
			m_flIntensityRND += (flIntensityRNDToReach - m_flIntensityRND) * FDIFF * SPEEDINTENSITYRND;
			m_flIntensityRND = m_flIntensityRND < 0.f ? 0.f : m_flIntensityRND > 1.f ? 1.f : m_flIntensityRND;

			LightRND = lightt.intensity + (lightt.intensiternd * rnd());

			if(LightRND > 1.f)
				LightRND = 1.f;

			l = &lightt;
		}

		if(tb->grid.nbvertexs)
			DrawGrille(&tb->grid, col, fx, l, &posgrille, angzgrille);

		//PASS #2
		if(force & 1) {
			switch(ti) {
				case INTERP_NO:
					m_camera.orgTrans.pos = possuiv;
					m_camera.setTargetCamera(m_camera.orgTrans.pos.x, m_camera.orgTrans.pos.y, 0.f);
					m_camera.angle.setPitch(0);
					m_camera.angle.setRoll(angzsuiv);
					PrepareCamera(&m_camera);
					break;
				case INTERP_LINEAR:
					break;
				case INTERP_BEZIER:
					break;
			}

			tb = m_bitmaps[numbitmapsuiv];

			alpha = 0xFF000000 - alpha;
			col &= 0x00FFFFFF;
			col |= alpha;

			l = NULL;

			if(this->light.intensity >= 0.f && this->lightd.intensity >= 0.f) {
				lightt = this->lightd;
				lightt.pos.x += (float)(LargeurRender >> 1);
				lightt.pos.y += (float)(HauteurRender >> 1);
				LightRND = lightt.intensity + (lightt.intensiternd * rnd());

				if(LightRND > 1.f)
					LightRND = 1.f;

				l = &lightt;
			}

			if(tb->grid.nbvertexs)
				DrawGrille(&tb->grid, col, fx, l, &posgrillesuiv, angzgrillesuiv);
		}

		//effets qui continuent avec le temps
		if(FlashBlancEnCours && (fx & 0x00ff0000) != FX_FLASH) {
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
			SpecialFadeDx = 0.f;
		}
		
		if(changekey) {
			changekey = false;
		}
		
		//post fx
		switch(fx & 0x00ff0000) {
			case FX_FLASH:
				FlashBlancEnCours = FX_FlashBlanc((float)LargeurRender, (float)HauteurRender, speed, colorflash, GetTrackFPS(), FPS);
				break;
			case FX_APPEAR:

				break;
			case FX_APPEAR2:

				break;
			default:
				break;
		}
		
		GRenderer->EndScene();

		CalcFPS();
	}	
}
