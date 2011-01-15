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
#include "ARX_Interface.h"
#include "ARX_Levels.h"
#include "ARX_Menu.h"
#include "ARX_Sound.h"
#include "ARX_Time.h"

#include "EERIEDraw.h"
#include "EERIEMath.h"
#include "EERIETexture.h"

//-----------------------------------------------------------------------------
extern float	PROGRESS_BAR_TOTAL;
extern float	PROGRESS_BAR_COUNT;
extern float	OLD_PROGRESS_BAR_COUNT;
extern long		MOULINEX;


//-----------------------------------------------------------------------------
TextureContainer	* FISHTANK_img = NULL;
TextureContainer	* ARKANE_img = NULL;

//-----------------------------------------------------------------------------
void LoadScreen(LPDIRECT3DDEVICE7 pd3dDevice)
{
	if (pd3dDevice)
	{
		pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0f, 0L);

		if (danaeApp.DANAEStartRender())
		{
			danaeApp.DANAEEndRender();
			danaeApp.m_pFramework->ShowFrame();
		}
	}
}

//-----------------------------------------------------------------------------
void ARX_INTERFACE_KillFISHTANK()
{
	D3DTextr_KillTexture(FISHTANK_img);
	FISHTANK_img = NULL;
}

//-----------------------------------------------------------------------------
void ARX_INTERFACE_KillARKANE()
{
	D3DTextr_KillTexture(ARKANE_img);
	ARKANE_img = NULL;
}
//-----------------------------------------------------------------------------
void DrawCenteredImage(LPDIRECT3DDEVICE7 pd3dDevice, TextureContainer * tc, bool _bRatio = true, float _fFade = 1.f)
{
	DANAESIZX = danaeApp.m_pFramework->m_dwRenderWidth;
	DANAESIZY = danaeApp.m_pFramework->m_dwRenderHeight;

	if (danaeApp.m_pDeviceInfo->bWindowed)
		DANAESIZY -= danaeApp.m_pFramework->Ystart;

	DANAECENTERX = DANAESIZX >> 1;
	DANAECENTERY = DANAESIZY >> 1;


	if (_bRatio)
	{
		EERIEDrawBitmap2(pd3dDevice,
		                 DANAECENTERX - (tc->m_dwWidth * 0.5f)*Xratio,
		                 DANAECENTERY - (tc->m_dwHeight * 0.5f)*Yratio,
		                 ARX_CLEAN_WARN_CAST_FLOAT((int)(tc->m_dwWidth * Xratio)),
		                 ARX_CLEAN_WARN_CAST_FLOAT((int)(tc->m_dwHeight * Yratio)),
		                 0.001f, tc, D3DRGB(_fFade, _fFade, _fFade));
	}
	else
	{
		EERIEDrawBitmap2(pd3dDevice,
		                 DANAECENTERX - (tc->m_dwWidth * 0.5f),
		                 DANAECENTERY - (tc->m_dwHeight * 0.5f),
		                 ARX_CLEAN_WARN_CAST_FLOAT((int)(tc->m_dwWidth)),
		                 ARX_CLEAN_WARN_CAST_FLOAT((int)(tc->m_dwHeight)),
		                 0.001f, tc, D3DRGB(_fFade, _fFade, _fFade));
	}


}

//-----------------------------------------------------------------------------
void ARX_INTERFACE_ShowFISHTANK(LPDIRECT3DDEVICE7 pd3dDevice)
{
	if (pd3dDevice)
	{
		Project.vsync = 0;
		pd3dDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFP_POINT);
		pd3dDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFP_POINT);
		pd3dDevice->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_CLAMP); 
		pd3dDevice->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, FALSE);

		pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0f, 0L);

		if (danaeApp.DANAEStartRender())
		{
			if (FISHTANK_img == NULL) FISHTANK_img = MakeTCFromFile("misc\\logo.bmp");

			if (FISHTANK_img != NULL)
			{
				pd3dDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, false);
				DrawCenteredImage(pd3dDevice, FISHTANK_img, false);
			}

			danaeApp.DANAEEndRender();
			danaeApp.m_pFramework->ShowFrame();
		}

		Project.vsync = 1;

		pd3dDevice->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_WRAP);
		pd3dDevice->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, TRUE);
		pd3dDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFP_LINEAR);
		pd3dDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFP_LINEAR);
	}
}

//-----------------------------------------------------------------------------
void ARX_INTERFACE_ShowARKANE(LPDIRECT3DDEVICE7 pd3dDevice)
{
	if (pd3dDevice)
	{
		pd3dDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFP_POINT);
		pd3dDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFP_POINT);
		pd3dDevice->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_CLAMP); 
		pd3dDevice->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, FALSE);
		SETZWRITE(pd3dDevice, true);

		danaeApp.EnableZBuffer();

		Project.vsync = 0;

		pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0f, 0L);

		if (danaeApp.DANAEStartRender())
		{
			if (ARKANE_img == NULL)
				ARKANE_img = MakeTCFromFile("Graph\\Interface\\misc\\Arkane.bmp");

			if (ARKANE_img != NULL)
			{
				pd3dDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, false);
				DrawCenteredImage(GDevice, ARKANE_img, false);
			}

			danaeApp.DANAEEndRender();
			danaeApp.m_pFramework->ShowFrame();
		}

		Project.vsync = 1;

		SETZWRITE(pd3dDevice, TRUE);
		pd3dDevice->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_WRAP);
		pd3dDevice->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, TRUE);
		pd3dDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFP_LINEAR);
		pd3dDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFP_LINEAR);
	}
}

static long lastloadednum = -1;
static TextureContainer * tc = NULL;
static TextureContainer * pbar = NULL;
static long nopbar = -1;
static long lastnum = -1;
 
static float fFadeSens = 0.f;
static float fFadeColor = 0.f;

//-----------------------------------------------------------------------------
void LoadLevelScreen(LPDIRECT3DDEVICE7 _pd3dDevice, long num, float ratio)
{
	Project.vsync = 0;

	LPDIRECT3DDEVICE7 pd3dDevice = NULL;

	if (GDevice)
	{
		GDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFP_POINT);
		GDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFP_POINT);
	}

	if (num < -1) // resets status
	{
		if (tc)
		{
			D3DTextr_KillTexture(tc);
			tc = NULL;
		}

		lastloadednum = -1;
		lastnum = -1;
		PROGRESS_BAR_TOTAL = 0;
		Project.vsync = 1;
		return;
	}

	if (num == -1)
		num = lastnum;

	pd3dDevice = GDevice;

	lastnum = num;

	if (num < 0)
	{
		Project.vsync = 1;
		return;
	}


	if (MOULINEX)
	{
		Project.vsync = 1;
		return;
	}

	if ((OLD_PROGRESS_BAR_COUNT <= 0.f) &&
	        (fFadeColor <= 0.f))
	{
		fFadeSens = .01f;
		fFadeColor = 0.f;
	}

	while ((OLD_PROGRESS_BAR_COUNT < PROGRESS_BAR_COUNT) ||
	        ((fFadeSens < 0.f) && (fFadeColor > 0.f)))
	{
		if ((fFadeSens < 0.f) &&				//sentinel
		        (fFadeColor == 0.f)) break;

		if (PROGRESS_BAR_TOTAL > 0.f)
			ratio = OLD_PROGRESS_BAR_COUNT / PROGRESS_BAR_TOTAL;
		else
			ratio = 0;

		if (ratio > 1.f) ratio = 1.f;
		else if (ratio < 0.f) ratio = 0.f;

		if (pd3dDevice)
		{
			pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0f, 0L);

			if (danaeApp.DANAEStartRender())
			{

				GDevice->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, false);

				danaeApp.EnableZBuffer();
				GDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
				SETZWRITE(GDevice, true);
				GDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, false);
				SETALPHABLEND(GDevice, false);

				long old = GLOBAL_EERIETEXTUREFLAG_LOADSCENE_RELEASE;
				GLOBAL_EERIETEXTUREFLAG_LOADSCENE_RELEASE = -1;

				if (num == 10)
					pbar = MakeTCFromFile("Graph\\interface\\menus\\load_full.bmp");
				else
					pbar = MakeTCFromFile("Graph\\interface\\menus\\load_full_level.bmp");

				nopbar = 1;
				GLOBAL_EERIETEXTUREFLAG_LOADSCENE_RELEASE = old;
				
				if (num != lastloadednum)
				{
					long old = GLOBAL_EERIETEXTUREFLAG_LOADSCENE_RELEASE;
					GLOBAL_EERIETEXTUREFLAG_LOADSCENE_RELEASE = -1;

					if (tc)
					{
						D3DTextr_KillTexture(tc);
						tc = NULL;
					}

					lastloadednum = num;
					char temp[256];
					char tx[256];
					GetLevelNameByNum(num, tx);
					sprintf(temp, "Graph\\Levels\\Level%s\\loading.bmp", tx);
					tc = MakeTCFromFile(temp);
					GLOBAL_EERIETEXTUREFLAG_LOADSCENE_RELEASE = old;
				}

				if (tc)
				{
					DrawCenteredImage(pd3dDevice, tc, true, fFadeColor);
				}

				if (pbar)
				{
					GDevice->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, true);

					if (num == 10)
					{
						float px, py, px2, py2;
						int ipx = (640 - 200)	/ 2;
						int ipy = 461;
						px = ipx * Xratio;
						py = ipy * Yratio;
						px2 = (ratio * pbar->m_dwWidth) * Xratio;
						py2 = pbar->m_dwHeight * Yratio;
						EERIEDrawBitmap_uv(GDevice, px, py, px2, py2, 0.f, pbar, D3DRGB(fFadeColor, fFadeColor, fFadeColor), pbar->m_hdx, pbar->m_hdy, ratio, 1.f);
					}
					else
					{
						float px, py, px2, py2;


						int ipx = ((640 - 320) / 2) + 60;
						int ipy = ((480 - 390) / 2) + 230;


						px = ipx * Xratio;
						py = ipy * Yratio;
						px2 = (ratio * pbar->m_dwWidth) * Xratio;
						py2 = pbar->m_dwHeight * Yratio;
						EERIEDrawBitmap_uv(GDevice, px, py, px2, py2, 0.f, pbar, D3DRGB(fFadeColor, fFadeColor, fFadeColor), pbar->m_hdx, pbar->m_hdy, ratio, 1);
					}

					GDevice->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, false);
				}

				danaeApp.DANAEEndRender();
				danaeApp.m_pFramework->ShowFrame();
			}
		}

		if (fFadeSens > 0.f)
		{
			if (fFadeColor >= 1.f)
				OLD_PROGRESS_BAR_COUNT++;
		}

		if ((PROGRESS_BAR_TOTAL) &&
		        (PROGRESS_BAR_COUNT >= PROGRESS_BAR_TOTAL) &&
		        (OLD_PROGRESS_BAR_COUNT >= PROGRESS_BAR_TOTAL))
		{
			if ((fFadeColor >= 1.f) &&
			        (fFadeSens > 0.f))
				fFadeSens = -fFadeSens;
		}

		fFadeColor += fFadeSens;
		fFadeColor = __min(1.f, fFadeColor);
		fFadeColor = __max(0.f, fFadeColor);
	}

	OLD_PROGRESS_BAR_COUNT = PROGRESS_BAR_COUNT;

	Project.vsync = 1;
}

//-----------------------------------------------------------------------------
void ARX_INTERFACE_EndIntro()
{
	ARX_SOUND_MixerStop(ARX_SOUND_MixerGame);
	ARX_MENU_Launch(GDevice);
}
