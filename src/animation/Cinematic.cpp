/*
 * Copyright 2011 Arx Libertatis Team (see the AUTHORS file)
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

#define WIDTHS 512
#define HEIGHTS 384

#define ADJUSTX(a) (((((a)-(WIDTHS>>1))*((float)LargeurRender/(float)WIDTHS))+(WIDTHS>>1)))*(640.f/(float)LargeurRender) //*((float)LARGEURS/(float)LargeurRender)
#define ADJUSTY(a) (((((a)-(HEIGHTS>>1))*((float)HauteurRender/(float)HEIGHTS))+(HEIGHTS>>1)))*(480.f/(float)HauteurRender)  //*((float)HAUTEURS/(float)HauteurRender)

/*---------------------------------------------------------------------------------*/

EERIE_CAMERA	Camera;
bool			LeftButton, RightButton;
int				 InsertKey;
C_KEY		*	KeyCopy;
int				LargeurRender, HauteurRender;
bool			InRender;
bool			ProjectModif;

//vertex
TexturedVertex		AllTLVertex[40000];

extern float DreamTable[];

C_KEY			KeyTemp;
bool			EditLight;
bool			ShiftKey;
bool			AltKey;

bool			FlashBlancEnCours;
bool			SpecialFadeEnCours;
float			OldSpeedFlashBlanc;
float			OldSpeedSpecialFade;
int				OldColorFlashBlanc;
int				OldFxSpecialFade;
int				LSoundChoose;

/*---------------------------------------------------------------------------------*/
 
/*---------------------------------------------------------------------------------*/
extern float	FlashAlpha;
extern char FileNameDirLoad[];
extern char FileNameDirSave[];
extern int UndoPile;
extern float SpecialFadeDx;
extern long DANAESIZX;
extern long DANAESIZY;

/*---------------------------------------------------------------------------------*/
Cinematic::Cinematic(int _w, int _h)
{
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
	InsertKey = 0;
	ShiftKey = false;
	AltKey = false;

	m_flIntensityRND = 0.f;
}

/*-------------------------------------------------------------------*/
Cinematic::~Cinematic()
{
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
	KeyTemp.idsound[LSoundChoose>>8] = idsound;
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
	
	Camera.size = Anglef(160.f, 60.f, 60.f);
	Camera.pos.x = 900.f;
	Camera.pos.y = -160.f;
	Camera.pos.z = 4340.f;
	Camera.angle.a = 3.f;
	Camera.angle.b = 268.f;
	Camera.angle.g = 0.f;
	Camera.clip.left = 0;
	Camera.clip.top = 0;
	Camera.clip.right = LargeurRender;
	Camera.clip.bottom = HauteurRender;
	Camera.clipz0 = 0.f;
	Camera.clipz1 = 2.999f;
	Camera.centerx = LargeurRender / 2;
	Camera.centery = HauteurRender / 2;
	Camera.AddX = 320.f;
	Camera.AddY = 240.f;
	Camera.focal = 350.f;
	Camera.Zdiv = 3000.f;
	Camera.Zmul = 1.f / Camera.Zdiv;
	Camera.clip3D = 60;
	Camera.type = CAM_SUBJVIEW;
	Camera.bkgcolor = Color::none;
	
	numbitmap = -1;
	numbitmapsuiv = -1;
	fx = -1;
	changekey = true;
	idsound = -1;
	key = NULL;
	
	projectload = false;
	InsertKey = 0;
	KeyCopy = NULL;
	
	LeftButton = RightButton = false;
	
	DeleteAllBitmap();
	DeleteAllSound();
	
	DeleteTrack();
	
	FlashBlancEnCours = false;
	SpecialFadeEnCours = false;
	
	LSoundChoose = C_KEY::English << 8;
	
	m_flIntensityRND = 0.f;
	
}

void Cinematic::New() {
	
	projectload = false;

	numbitmap = -1;
	numbitmapsuiv = -1;
	fx = -1;
	key = NULL;
	InsertKey = 0;
	KeyCopy = NULL;
	LeftButton = RightButton = false;

	DeleteTrack();
	DeleteAllBitmap();
	DeleteAllSound();

	AllocTrack(0, 100, 30.f);
	FillKeyTemp(&pos, angz, 0, -1, -1, INTERP_BEZIER, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 1.f, -1, 1, NULL, &posgrille, angzgrille, 1.f);
	AddKey(&KeyTemp, true, true, true);
	FillKeyTemp(&pos, angz, 100, -1, -1, INTERP_BEZIER, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 1.f, -1, 1, NULL, &posgrille, angzgrille, 1.f);
	AddKey(&KeyTemp, true, true, true);
	this->lightd = this->lightchoose = this->light;

	InitUndo();

	SetCurrFrame(GetStartFrame());

	projectload = true;

	FlashBlancEnCours = false;
	SpecialFadeEnCours = false;

	ProjectModif = false;

	LSoundChoose = C_KEY::English << 8;
	
}

void Cinematic::DeleteAllBitmap()
{
	for(std::vector<CinematicBitmap*>::iterator it = m_bitmaps.begin(); it != m_bitmaps.end(); ++it)
	{
		delete *it;
	}

	m_bitmaps.clear();
}

//*************************************************************************************
// InitDeviceObjects()
// Sets RenderStates
//*************************************************************************************
void Cinematic::InitDeviceObjects() {
	
	GRenderer->SetRenderState(Renderer::DepthTest, false);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetCulling(Renderer::CullNone);
	GRenderer->GetTextureStage(0)->SetWrapMode(TextureStage::WrapClamp);

	GRenderer->GetTextureStage(0)->SetMipMapLODBias(0);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetRenderState(Renderer::Fog, false);

	EditLight = false;
}

void Cinematic::DeleteDeviceObjects() {
	
	GRenderer->SetRenderState(Renderer::DepthTest, true);
	GRenderer->SetRenderState(Renderer::DepthWrite, true);
	GRenderer->SetCulling(Renderer::CullCCW);
	GRenderer->GetTextureStage(0)->SetWrapMode(TextureStage::WrapRepeat);
	
	GRenderer->GetTextureStage(0)->SetMipMapLODBias(0);
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	GRenderer->SetRenderState(Renderer::Fog, true);

	GRenderer->GetTextureStage(0)->SetColorOp(TextureStage::OpModulate, TextureStage::ArgTexture, TextureStage::ArgDiffuse);
	GRenderer->GetTextureStage(0)->SetAlphaOp(TextureStage::OpModulate, TextureStage::ArgTexture, TextureStage::ArgDiffuse);
	
}

float LightRND;

/*---------------------------------------------------------------*/

int CalculLight(CinematicLight * light, float x, float y, int col)
{
	float	ra = (float)sqrt((light->pos.x - x) * (light->pos.x - x) + (light->pos.y - y) * (light->pos.y - y));

	if(ra > light->fallout) {
		return (Color::fromBGRA(col) * LightRND).toBGRA();
	}
	else
	{
		float r, g, b;

		if (ra < light->fallin)
		{
			r = light->r * LightRND;
			g = light->g * LightRND;
			b = light->b * LightRND;
		}
		else
		{
			ra = (light->fallout - ra) / (light->fallout - light->fallin);
			float t = LightRND * ra;
			r = light->r * t;
			g = light->g * t;
			b = light->b * t;
		}


		Color in = Color::fromBGRA(col);
		in.r = min(in.r + (int)r, 255);
		in.g = min(in.g + (int)g, 255);
		in.b = min(in.b + (int)b, 255);
		return in.toBGRA();
	}
}
/*---------------------------------------------------------------*/
Vec3f	LocalPos;
float		LocalSin, LocalCos;
void TransformLocalVertex(Vec3f * vbase, TexturedVertex * d3dv)
{
	d3dv->p.x = vbase->x * LocalCos + vbase->y * LocalSin + LocalPos.x;
	d3dv->p.y = vbase->x * -LocalSin + vbase->y * LocalCos + LocalPos.y;
	d3dv->p.z = vbase->z + LocalPos.z;
}
/*---------------------------------------------------------------*/
void DrawGrille(CinematicGrid * grille, int col, int fx, CinematicLight * light, Vec3f * posgrille, float angzgrille)
{
	int nb = grille->nbvertexs;
	Vec3f * v = grille->vertexs;
	TexturedVertex * d3dv = AllTLVertex;

	LocalPos = *posgrille;
	LocalSin = (float)sin(radians(angzgrille));
	LocalCos = (float)cos(radians(angzgrille));

	if ((fx & 0x0000FF00) == FX_DREAM)
	{
		if (light)
		{
			float * dream = DreamTable;

			while (nb--)
			{
				TexturedVertex vtemp;
				Vec3f t;
				t.x = v->x + *dream++;
				t.y = v->y + *dream++;
				t.z = v->z;
				TransformLocalVertex(&t, &vtemp);
				EE_RTP(&vtemp, d3dv);
				d3dv->color = CalculLight(light, d3dv->p.x, d3dv->p.y, col);
				d3dv->p.x = ADJUSTX(d3dv->p.x);
				d3dv->p.y = ADJUSTY(d3dv->p.y);
				v++;
				d3dv++;
			}
		}
		else
		{
			float * dream = DreamTable;

			while (nb--)
			{
				TexturedVertex vtemp;
				Vec3f t;
				t.x = v->x + *dream++;
				t.y = v->y + *dream++;
				t.z = v->z;
				TransformLocalVertex(&t, &vtemp);
				EE_RTP(&vtemp, d3dv);
				d3dv->p.x = ADJUSTX(d3dv->p.x);
				d3dv->p.y = ADJUSTY(d3dv->p.y);
				d3dv->color = col;
				v++;
				d3dv++;
			}
		}
	}
	else
	{
		if (light)
		{
			while (nb--)
			{
				TexturedVertex vtemp;
				TransformLocalVertex(v, &vtemp);
				EE_RTP(&vtemp, d3dv);
				d3dv->color = CalculLight(light, d3dv->p.x, d3dv->p.y, col);
				d3dv->p.x = ADJUSTX(d3dv->p.x);
				d3dv->p.y = ADJUSTY(d3dv->p.y);
				v++;
				d3dv++;
			}
		}
		else
		{
			while (nb--)
			{
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

		while (nb2--)
		{
			AllTLVertex[uvs->indvertex].uv.x = uvs->uv.x;
			AllTLVertex[uvs->indvertex].uv.y = uvs->uv.y;
			uvs++;
		}

		GRenderer->drawIndexed(Renderer::TriangleList, AllTLVertex, grille->nbvertexs,
		                       ((unsigned short *)grille->inds) + mat->startind, mat->nbind);
	}
}
/*---------------------------------------------------------------*/
void Cinematic::Render(float FDIFF) {
	
	CinematicBitmap * tb;

	LargeurRender = DANAESIZX;
	HauteurRender = DANAESIZY;

	if (projectload)
	{
		GRenderer->Clear(Renderer::ColorBuffer);
		GRenderer->BeginScene();
		InRender = true;

		if (InsertKey && m_bitmaps.size() > 0)
		{
			FillKeyTemp(&pos, angz, GetCurrentFrame(), numbitmap, fx, ti, colorchoose, colorchoosed, colorflashchoose, speedchoose, idsound, force, &light, &posgrille, angzgrille, speedtrack);
			AddDiffKey(this, &KeyTemp, true, true, true);

			InsertKey = 0;
		}

		GereTrack(this, FDIFF);

		//sound
		if (changekey)
		{
			if (idsound >= 0)
			{
				PlaySoundKeyFramer(idsound);
			}
		}

		//draw
		GRenderer->SetBlendFunc(Renderer::BlendSrcAlpha, Renderer::BlendInvSrcAlpha);

		GRenderer->GetTextureStage(0)->SetColorOp(TextureStage::OpModulate, TextureStage::ArgTexture, TextureStage::ArgDiffuse);
		GRenderer->GetTextureStage(0)->SetAlphaOp(TextureStage::OpModulate, TextureStage::ArgTexture, TextureStage::ArgDiffuse);

		GRenderer->GetTextureStage(1)->DisableAlpha();
		
		//image key
		tb = m_bitmaps[numbitmap];

		//fx
		int col = 0x00FFFFFF;

		switch (fx & 0x000000FF)
		{
			case FX_FADEIN:
				col = FX_FadeIN(a, color, colord);
				break;
			case FX_FADEOUT:
				col = FX_FadeOUT(a, color, colord);
				break;
			case FX_BLUR:
				FX_Blur(this, tb);
				break;
			default:
				break;
		}

		//fx precalculation
		switch (fx & 0x0000ff00)
		{
			case FX_DREAM:

				if ((this->fxsuiv & 0x0000ff00) == FX_DREAM)
					FX_DreamPrecalc(tb, 15.f, (FPS > 1.f) ? GetTrackFPS() / FPS : 0.f);
				else
					FX_DreamPrecalc(tb, 15.f * a, (FPS > 1.f) ? GetTrackFPS() / FPS : 0.f);

				break;
			default:
				break;
		}

		Camera.pos = pos;
		SetTargetCamera(&Camera, Camera.pos.x, Camera.pos.y, 0.f);
		Camera.angle.b = 0;
		Camera.angle.g = angz;
		Camera.centerx = LargeurRender >> 1;
		Camera.centery = HauteurRender >> 1;
		Camera.clip.right = LargeurRender;
		Camera.clip.bottom = HauteurRender;
		PrepareCamera(&Camera);
		SetActiveCamera(&Camera);

		int alpha = ((int)(a * 255.f)) << 24;

		int stopline = tb->nbx;

		if (stopline & 1) stopline++;

		if (force ^ 1) alpha = 0xFF000000;

		col |= alpha;

		CinematicLight lightt, *l = NULL;

		if ((this->light.intensity >= 0.f) &&
		        (this->lightd.intensity >= 0.f))
		{
			lightt = this->light;
			lightt.pos.x += (float)(LargeurRender >> 1);
			lightt.pos.y += (float)(HauteurRender >> 1);

			#define SPEEDINTENSITYRND (10.f)
			float flIntensityRNDToReach = lightt.intensiternd * rnd();
			m_flIntensityRND += (flIntensityRNDToReach - m_flIntensityRND) * FDIFF * SPEEDINTENSITYRND;
			m_flIntensityRND = m_flIntensityRND < 0.f ? 0.f : m_flIntensityRND > 1.f ? 1.f : m_flIntensityRND;

			LightRND = lightt.intensity + (lightt.intensiternd * rnd());

			if (LightRND > 1.f) LightRND = 1.f;

			l = &lightt;
		}

		if (tb->grid.nbvertexs) DrawGrille(&tb->grid, col, fx, l, &posgrille, angzgrille);

		//PASS #2
		if (force & 1)
		{
			switch (ti)
			{
				case INTERP_NO:
					Camera.pos = possuiv;
					SetTargetCamera(&Camera, Camera.pos.x, Camera.pos.y, 0.f);
					Camera.angle.b = 0;
					Camera.angle.g = angzsuiv;
					PrepareCamera(&Camera);
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

			if ((this->light.intensity >= 0.f) &&
			        (this->lightd.intensity >= 0.f))
			{
				lightt = this->lightd;
				lightt.pos.x += (float)(LargeurRender >> 1);
				lightt.pos.y += (float)(HauteurRender >> 1);
				LightRND = lightt.intensity + (lightt.intensiternd * rnd());

				if (LightRND > 1.f) LightRND = 1.f;

				l = &lightt;
			}

			if (tb->grid.nbvertexs) DrawGrille(&tb->grid, col, fx, l, &posgrillesuiv, angzgrillesuiv);
		}

		//effets qui continuent avec le temps
		if ((FlashBlancEnCours) && ((fx & 0x00FF0000) != FX_FLASH))
		{
			speed = OldSpeedFlashBlanc;
			colorflash = OldColorFlashBlanc;

			if (fx < 0) fx = FX_FLASH;
			else fx |= FX_FLASH;
		}
		else
		{
			if (changekey)
			{
				FlashAlpha = 0.f;
			}

			OldSpeedFlashBlanc = speed;
			OldColorFlashBlanc = colorflash;
		}

		if ((SpecialFadeEnCours) &&
		        (((fx & 0x00FF0000) != FX_APPEAR) && ((fx & 0x00FF0000) != FX_APPEAR2))
		   )
		{
			speed = OldSpeedSpecialFade;

			if (fx < 0) fx = OldFxSpecialFade;
			else fx |= OldFxSpecialFade;
		}
		else
		{
			if (changekey)
			{
				SpecialFadeDx = 0.f;
			}

			OldSpeedSpecialFade = speed;
			OldFxSpecialFade = fx & 0x00FF0000;
		}

		if (changekey)
		{
			changekey = false;
		}

		//post fx
		switch (fx & 0x00FF0000)
		{
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

		CalcFPS();
		InRender = false;
	}
	
}
