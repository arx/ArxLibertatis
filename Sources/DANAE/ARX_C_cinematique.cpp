/*
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
#include <stdlib.h>
#include "danae.h"
#include "arx_c_cinematique.h"
#include "Resource.h"

#include "EERIEUtil.h"
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define ADJUSTX(a) (((((a)-(LARGEURS>>1))*((float)LargeurRender/(float)LARGEURS))+(LARGEURS>>1)))*(640.f/(float)LargeurRender) //*((float)LARGEURS/(float)LargeurRender)
#define ADJUSTY(a) (((((a)-(HAUTEURS>>1))*((float)HauteurRender/(float)HAUTEURS))+(HAUTEURS>>1)))*(480.f/(float)HauteurRender)  //*((float)HAUTEURS/(float)HauteurRender)

/*---------------------------------------------------------------------------------*/

EERIE_CAMERA	Camera;
char			AllTxt[32767];
BOOL			LeftButton, RightButton;
int				 InsertKey;
C_KEY		*	KeyCopy;
int				LargeurRender, HauteurRender;
TextureContainer * FxTexture[2];
BOOL			InRender;
BOOL			ProjectModif;

//vertex
D3DTLVERTEX		AllD3DTLVertex[40000];

extern float DreamTable[];

C_KEY			KeyTemp;
BOOL			EditLight;
BOOL			DrawLine;
BOOL			ShiftKey;
BOOL			AltKey;

BOOL			FlashBlancEnCours;
BOOL			SpecialFadeEnCours;
float			OldSpeedFlashBlanc;
float			OldSpeedSpecialFade;
int				OldColorFlashBlanc;
int				OldFxSpecialFade;
int				LSoundChoose;

char			DirectoryAbs[256];

/*---------------------------------------------------------------------------------*/
 
/*---------------------------------------------------------------------------------*/
extern C_BITMAP	TabBitmap[];
extern float	FlashAlpha;
extern char FileNameDirLoad[];
extern char FileNameDirSave[];
extern char FileNameChoose[];
extern int UndoPile;
extern int NbBitmap;
extern float SpecialFadeDx;
extern C_SOUND		TabSound[MAX_SOUND];
extern long DANAESIZX;
extern long DANAESIZY;
extern DANAE danaeApp;
/*---------------------------------------------------------------*/
void GetPathDirectory(char * dirfile)
{
	char	* n ;
	int				i ;

	if (!(i = strlen(dirfile))) return;

	n = dirfile + i;

	while (i && (*n != '\\'))
	{
		n--;
		i--;
	}

	n++;

	if (i) *n = 0;
}
/*---------------------------------------------------------------*/
void ClearDirectory(char * dirfile)
{
	char	* n ;
	int				i ;

	if (!(i = strlen(dirfile))) return ;

	n = dirfile + i ;

	while (i && (*n != '\\'))
	{
		n-- ;
		i-- ;
	}

	if (i)
	{
		strcpy(FileNameChoose, n + 1) ;
	}
}
/*---------------------------------------------------------------*/
void ClearAbsDirectory(char * pT, char * d)
{
	char * pTcopy = pT;
	int i = strlen(pT);

	while (i--)
	{
		if (!strnicmp(pT, d, strlen(d)))
		{
			pT += strlen(d);
			memmove(pTcopy, pT, strlen(pT - strlen(d)) + 1);
			break;
		}

		pT++;
	}
}

/*---------------------------------------------------------------*/
void AddDirectory(char * pT, char * dir)
{
	char pTCopy[256];
	strcpy(pTCopy, pT);
	strcpy(pT, dir);
	strcat(pT, pTCopy);
}

/*---------------------------------------------------------------------------------*/
CINEMATIQUE::CINEMATIQUE(LPDIRECT3DDEVICE7 _m_pd3dDevice, int _w, int _h)
{
	LargeurRender = _w;
	HauteurRender = _h;

	m_pd3dDevice = _m_pd3dDevice;

	numbitmap = -1;
	numbitmapsuiv = -1;
	fx = -1;
	changekey = TRUE;
	idsound = -1;
	key = NULL;
	projectload = FALSE; 
	ti = tichoose = INTERP_BEZIER;
	speedchoose = 1.f;
	InsertKey = 0;
	ShiftKey = FALSE;
	AltKey = FALSE;

	m_flIntensityRND = 0.f;

	strcpy(DirectoryAbs, Project.workingdir);
}
/*-------------------------------------------------------------------*/
void FillKeyTemp(EERIE_3D * pos, float az, int frame, int numbitmap, int numfx, short ti, int color, int colord, int colorf, float speed, int idsound, short force, C_LIGHT * light, EERIE_3D * posgrille, float azgrille, float speedtrack)
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
		KeyTemp.light.intensite = -2.f;
	}
}

/* Recreation d'une mapp */
void CINEMATIQUE::ReInitMapp(int id)
{
	if (id < 0) return;

	if (TabBitmap[id].actif)
	{
		ReCreateAllMapsForBitmap(id, TabBitmap[id].grille.echelle, this, GDevice);
	}
}

/* Reinit */
HRESULT CINEMATIQUE::OneTimeSceneReInit()
{
	Camera.size.y = 160.f;
	Camera.size.x = 60.f;
	Camera.size.z = 60.f;
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
	Camera.bkgcolor = 0x00000000;

	numbitmap = -1;
	numbitmapsuiv = -1;
	fx = -1;
	changekey = TRUE;
	idsound = -1;
	key = NULL;

	projectload = FALSE;
	InsertKey = 0;
	KeyCopy = NULL;

	LeftButton = RightButton = FALSE;

	DeleteAllBitmap(GDevice);
	DeleteAllSound();

	InitMapLoad(this);
	InitSound(this);
	DeleteTrack();

	FlashBlancEnCours = FALSE;
	SpecialFadeEnCours = FALSE;

	LSoundChoose = C_LANGUAGE_ENGLISH << 8;

	m_flIntensityRND = 0.f;

	return S_OK;
}
HRESULT CINEMATIQUE::New()
{
	projectload = FALSE;

	numbitmap = -1;
	numbitmapsuiv = -1;
	fx = -1;
	key = NULL;
	InsertKey = 0;
	KeyCopy = NULL;
	LeftButton = RightButton = FALSE;

	DeleteTrack();
	DeleteAllBitmap(GDevice);
	DeleteAllSound();

	AllocTrack(0, 100, 30.f);
	FillKeyTemp(&pos, angz, 0, -1, -1, INTERP_BEZIER, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 1.f, -1, 1, NULL, &posgrille, angzgrille, 1.f);
	AddKey(&KeyTemp, TRUE, TRUE, TRUE);
	FillKeyTemp(&pos, angz, 100, -1, -1, INTERP_BEZIER, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 1.f, -1, 1, NULL, &posgrille, angzgrille, 1.f);
	AddKey(&KeyTemp, TRUE, TRUE, TRUE);
	this->lightd = this->lightchoose = this->light;

	InitMapLoad(this);
	InitSound(this);
	InitUndo();

	SetCurrFrame(GetStartFrame());

	projectload = TRUE;

	FlashBlancEnCours = FALSE;
	SpecialFadeEnCours = FALSE;

	ProjectModif = FALSE;

	LSoundChoose = C_LANGUAGE_ENGLISH << 8;

	return S_OK;
}
//*************************************************************************************
// InitDeviceObjects()
// Sets RenderStates
//*************************************************************************************
HRESULT CINEMATIQUE::InitDeviceObjects()
{
	m_pd3dDevice = GDevice;

	D3DMATERIAL7 mtrl;
	D3DUtil_InitMaterial(mtrl, 1.f, 1.f, 1.f);
	m_pd3dDevice->SetMaterial(&mtrl);

	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE , TRUE);
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, FALSE);
	SETZWRITE(GDevice, false);
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_AMBIENT,  0x0a0a0a0a);
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, TRUE);
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_SPECULARENABLE, FALSE);
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_LASTPIXEL, TRUE); 
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_CLIPPING , TRUE);
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_LIGHTING  , FALSE);
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_CULLMODE , D3DCULL_NONE);
	m_pd3dDevice->SetTextureStageState(0, D3DTSS_ADDRESS , D3DTADDRESS_CLAMP);

	D3DDEVICEDESC7 devicedesc;
	this->m_pd3dDevice->GetCaps(&devicedesc);
	DWORD f;
	bool bAnisotropicOk = false;

	if (devicedesc.dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_MINFANISOTROPIC)
	{
		f = D3DTFG_ANISOTROPIC;
		bAnisotropicOk = true;
	}
	else
	{
		if (devicedesc.dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_MINFLINEAR)
		{
			f = D3DTFG_LINEAR;
		}
		else
		{
			f = D3DTFG_POINT;
		}
	}

	m_pd3dDevice->SetTextureStageState(0, D3DTSS_MINFILTER, f);

	if (devicedesc.dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_MAGFANISOTROPIC)
	{
		f = D3DTFG_ANISOTROPIC;
		bAnisotropicOk = true;
	}
	else
	{
		if (devicedesc.dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_MAGFLINEAR)
		{
			f = D3DTFG_LINEAR;
		}
		else
		{
			f = D3DTFG_POINT;
		}
	}

	m_pd3dDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, f);

	if (bAnisotropicOk)
	{
		m_pd3dDevice->SetTextureStageState(0, D3DTSS_MAXANISOTROPY, 0);
	}

	m_pd3dDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFG_LINEAR);
	m_pd3dDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTFP_LINEAR);
	m_pd3dDevice->SetTextureStageState(0, D3DTSS_MIPMAPLODBIAS, (DWORD)(0));
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE);

	EditLight = FALSE;

	return S_OK;
}

HRESULT CINEMATIQUE::DeleteDeviceObjects()
{
	m_pd3dDevice = GDevice;		

	// Setup Base Material
	D3DMATERIAL7 mtrl;
	D3DUtil_InitMaterial(mtrl, 1.f, 1.f, 1.f);
	m_pd3dDevice->SetMaterial(&mtrl);
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE , TRUE);
	danaeApp.EnableZBuffer();
	SETZWRITE(GDevice, true);
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_AMBIENT,  0x0a0a0a0a);
	// Setup Dither Mode
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, FALSE);
	// Setup Specular RenderState
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_SPECULARENABLE, FALSE);
	// Setup LastPixel RenderState
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_LASTPIXEL, TRUE); 
	// Setup Clipping RenderState
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_CLIPPING , TRUE);
	// Disable Lighting RenderState
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_LIGHTING  , FALSE);

	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_CULLMODE , D3DCULL_CCW);
	m_pd3dDevice->SetTextureStageState(0, D3DTSS_ADDRESS , D3DTADDRESS_WRAP);

	D3DDEVICEDESC7 devicedesc;
	this->m_pd3dDevice->GetCaps(&devicedesc);
	DWORD f;

	if (devicedesc.dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_MINFLINEAR)
	{
		f = D3DTFG_LINEAR;
	}
	else
	{
		f = D3DTFG_POINT;
	}
	
	m_pd3dDevice->SetTextureStageState(0, D3DTSS_MINFILTER, f);

	if (devicedesc.dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_MAGFLINEAR)
	{
		f = D3DTFG_LINEAR;
	}
	else
	{
		f = D3DTFG_POINT;
	}
	
	m_pd3dDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, f);
	m_pd3dDevice->SetTextureStageState(0, D3DTSS_MAXANISOTROPY, 1);

	m_pd3dDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTFP_LINEAR);
	m_pd3dDevice->SetTextureStageState(0, D3DTSS_MIPMAPLODBIAS, (DWORD)(0));

	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, TRUE);

	m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

	return S_OK;
}

float LightRND;

/*---------------------------------------------------------------*/

int CalculLight(C_LIGHT * light, float x, float y, int col)
{
	float	ra = (float)sqrt((light->pos.x - x) * (light->pos.x - x) + (light->pos.y - y) * (light->pos.y - y));

	if (ra > light->fallout)
	{
		int ri = (int)(((float)((col >> 16) & 0xFF)) * LightRND);
		int gi = (int)(((float)((col >> 8) & 0xFF)) * LightRND);
		int bi = (int)(((float)((col) & 0xFF)) * LightRND);
		return RGBA_MAKE(ri, gi, bi, (col >> 24) & 0xFF);
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

		int ri = ((col >> 16) & 0xFF) + ((int)r);
		int gi = ((col >> 8) & 0xFF) + ((int)g);
		int bi = ((col) & 0xFF) + ((int)b);

		if (ri > 255) ri = 255;

		if (gi > 255) gi = 255;

		if (bi > 255) bi = 255;

		return RGBA_MAKE(ri, gi, bi, (col >> 24) & 0xFF);
	}
}
/*---------------------------------------------------------------*/
EERIE_3D	LocalPos;
float		LocalSin, LocalCos;
void TransformLocalVertex(EERIE_3D * vbase, D3DTLVERTEX * d3dv)
{
	d3dv->sx = vbase->x * LocalCos + vbase->y * LocalSin + LocalPos.x;
	d3dv->sy = vbase->x * -LocalSin + vbase->y * LocalCos + LocalPos.y;
	d3dv->sz = vbase->z + LocalPos.z;
}
/*---------------------------------------------------------------*/
void DrawGrille(LPDIRECT3DDEVICE7 device, C_GRILLE * grille, int col, int fx, C_LIGHT * light, EERIE_3D * posgrille, float angzgrille)
{
	int nb = grille->nbvertexs;
	EERIE_3D * v = grille->vertexs;
	D3DTLVERTEX * d3dv = AllD3DTLVertex;

	LocalPos = *posgrille;
	LocalSin = (float)sin(DEG2RAD(angzgrille));
	LocalCos = (float)cos(DEG2RAD(angzgrille));

	if ((fx & 0x0000FF00) == FX_DREAM)
	{
		if (light)
		{
			float * dream = DreamTable;

			while (nb--)
			{
				D3DTLVERTEX vtemp;
				EERIE_3D t;
				t.x = v->x + *dream++;
				t.y = v->y + *dream++;
				t.z = v->z;
				TransformLocalVertex(&t, &vtemp);
				EE_RTP(&vtemp, d3dv);
				d3dv->color = CalculLight(light, d3dv->sx, d3dv->sy, col);
				d3dv->sx = ADJUSTX(d3dv->sx);
				d3dv->sy = ADJUSTY(d3dv->sy);
				v++;
				d3dv++;
			}
		}
		else
		{
			float * dream = DreamTable;

			while (nb--)
			{
				D3DTLVERTEX vtemp;
				EERIE_3D t;
				t.x = v->x + *dream++;
				t.y = v->y + *dream++;
				t.z = v->z;
				TransformLocalVertex(&t, &vtemp);
				EE_RTP(&vtemp, d3dv);
				d3dv->sx = ADJUSTX(d3dv->sx);
				d3dv->sy = ADJUSTY(d3dv->sy);
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
				D3DTLVERTEX vtemp;
				TransformLocalVertex(v, &vtemp);
				EE_RTP(&vtemp, d3dv);
				d3dv->color = CalculLight(light, d3dv->sx, d3dv->sy, col);
				d3dv->sx = ADJUSTX(d3dv->sx);
				d3dv->sy = ADJUSTY(d3dv->sy);
				v++;
				d3dv++;
			}
		}
		else
		{
			while (nb--)
			{
				D3DTLVERTEX vtemp;
				TransformLocalVertex(v, &vtemp);
				EE_RTP(&vtemp, d3dv);
				d3dv->sx = ADJUSTX(d3dv->sx);
				d3dv->sy = ADJUSTY(d3dv->sy);
				d3dv->color = col;
				v++;
				d3dv++;
			}
		}
	}

	C_INDEXED	* mat = grille->mats;
	C_UV	*	uvs = grille->uvs;
	nb = grille->nbmat;

	while (nb--)
	{
		if (mat->tex)
			SETTC(device, mat->tex);
		else
			SETTC(device, NULL);

		int	nb2 = mat->nbvertexs;

		while (nb2--)
		{
			AllD3DTLVertex[uvs->indvertex].tu = uvs->uv.x;
			AllD3DTLVertex[uvs->indvertex].tv = uvs->uv.y;
			uvs++;
		}

		if (FAILED(device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,
		                                        D3DFVF_TLVERTEX,
		                                        AllD3DTLVertex,
		                                        grille->nbvertexs,
		                                        ((unsigned short *)grille->inds) + mat->startind,
		                                        mat->nbind,
		                                        0)))
		{
			ARX_DEAD_CODE();
		}

		if (DrawLine)
		{
			SETTC(device, NULL);
			device->SetRenderState(D3DRENDERSTATE_FILLMODE, D3DFILL_WIREFRAME);
			device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,
			                             D3DFVF_TLVERTEX,
			                             AllD3DTLVertex,
			                             grille->nbvertexs,
			                             ((unsigned short *)grille->inds) + mat->startind,
			                             mat->nbind,
			                             0);
			device->SetRenderState(D3DRENDERSTATE_FILLMODE, D3DFILL_SOLID);
		}

		mat++;
	}
}
/*---------------------------------------------------------------*/
HRESULT CINEMATIQUE::Render(float FDIFF)
{
	int			nb, col;
	C_BITMAP	* tb;


	m_pd3dDevice = GDevice;

	LargeurRender = DANAESIZX;
	HauteurRender = DANAESIZY;

	if (projectload)
	{
		m_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, 0x00000000, 1.0f, 0L);
		danaeApp.DANAEStartRender();
		InRender = TRUE;

		if (InsertKey && NbBitmap)
		{
			FillKeyTemp(&pos, angz, GetCurrentFrame(), numbitmap, fx, ti, colorchoose, colorchoosed, colorflashchoose, speedchoose, idsound, force, &light, &posgrille, angzgrille, speedtrack);
			AddDiffKey(this, &KeyTemp, TRUE, TRUE, TRUE);

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
		m_pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
		m_pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);

		m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
		m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
		m_pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

		//image key
		tb = &TabBitmap[numbitmap];

		//fx
		col = 0x00FFFFFF;

		switch (fx & 0x000000FF)
		{
			case FX_FADEIN:
				col = FX_FadeIN(a, color, colord);
				break;
			case FX_FADEOUT:
				col = FX_FadeOUT(a, color, colord);
				break;
			case FX_BLUR:
				FX_Blur(this, m_pd3dDevice, tb);
				nb = 0;
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

		if ((numbitmap < 0) || (numbitmapsuiv < 0))
		{
			nb = 0;
		}

		float * dream, *dreambase;
		dream = dreambase = DreamTable;
 
		int stopline = tb->nbx;

		if (stopline & 1) stopline++;

		if (force ^ 1) alpha = 0xFF000000;

		col |= alpha;

		C_LIGHT lightt, *l = NULL;

		if ((this->light.intensite >= 0.f) &&
		        (this->lightd.intensite >= 0.f))
		{
			lightt = this->light;
			lightt.pos.x += (float)(LargeurRender >> 1);
			lightt.pos.y += (float)(HauteurRender >> 1);

			#define SPEEDINTENSITYRND (10.f)
			float flIntensityRNDToReach = lightt.intensiternd * rnd();
			m_flIntensityRND += (flIntensityRNDToReach - m_flIntensityRND) * FDIFF * SPEEDINTENSITYRND;
			m_flIntensityRND = m_flIntensityRND < 0.f ? 0.f : m_flIntensityRND > 1.f ? 1.f : m_flIntensityRND;

			LightRND = lightt.intensite + (lightt.intensiternd * rnd());

			if (LightRND > 1.f) LightRND = 1.f;

			l = &lightt;
		}

		if (tb->grille.nbvertexs) DrawGrille(this->m_pd3dDevice, &tb->grille, col, fx, l, &posgrille, angzgrille);

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

			tb = &TabBitmap[numbitmapsuiv];

			alpha = 0xFF000000 - alpha;
			col &= 0x00FFFFFF;
			col |= alpha;

			l = NULL;

			if ((this->light.intensite >= 0.f) &&
			        (this->lightd.intensite >= 0.f))
			{
				lightt = this->lightd;
				lightt.pos.x += (float)(LargeurRender >> 1);
				lightt.pos.y += (float)(HauteurRender >> 1);
				LightRND = lightt.intensite + (lightt.intensiternd * rnd());

				if (LightRND > 1.f) LightRND = 1.f;

				l = &lightt;
			}

			if (tb->grille.nbvertexs) DrawGrille(this->m_pd3dDevice, &tb->grille, col, fx, l, &posgrillesuiv, angzgrillesuiv);
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
			changekey = FALSE;
		}

		//post fx
		switch (fx & 0x00FF0000)
		{
			case FX_FLASH:
				FlashBlancEnCours = FX_FlashBlanc(m_pd3dDevice, (float)LargeurRender, (float)HauteurRender, speed, colorflash, GetTrackFPS(), FPS);
				break;
			case FX_APPEAR:

				if (FxTexture[0]) SpecialFadeEnCours = SpecialFade(m_pd3dDevice, FxTexture[0], (float)LargeurRender, (float)HauteurRender, speed, GetTrackFPS(), FPS);

				break;
			case FX_APPEAR2:

				if (FxTexture[0]) SpecialFadeEnCours = SpecialFadeR(m_pd3dDevice, FxTexture[0], (float)LargeurRender, (float)HauteurRender, speed, GetTrackFPS(), FPS);

				break;
			default:
				break;
		}

		CalcFPS();
		InRender = FALSE;
	}

	return S_OK;
}
