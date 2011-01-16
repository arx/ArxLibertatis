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
#include <d3d.h>
#include "Danae.h"
#include "ARX_ViewImage.h"
#include "ARX_Menu2.h"
#include "arx_time.h"
#include "EERIETexture.h"
#include "EERIEDraw.h"
#include "Hermesmain.h"


//-----------------------------------------------------------------------------
extern LPDIRECT3DDEVICE7 GDevice;
extern long DANAESIZX;
extern long DANAESIZY;
extern CDirectInput * pGetInfoDirectInput;
extern PakManager * pPakManager;

//-----------------------------------------------------------------------------
 

//-----------------------------------------------------------------------------
ViewImage::ViewImage(char * _pcDir, char * _pExt)
{
	vListImage.clear();

	char tTxt[256];
	int iNum = 0;

	while (1)
	{
		sprintf(tTxt, "%squit%d.bmp", _pcDir + strlen(Project.workingdir), iNum);

		if (pPakManager->ExistFile(tTxt))
		{
			char * pCopy = strdup(tTxt);
			pCopy = strupr(pCopy);
			vListImage.push_back(pCopy);
			iNum++;
		}
		else
		{
			sprintf(tTxt, "%squit%d.jpg", _pcDir + strlen(Project.workingdir), iNum);

			if (pPakManager->ExistFile(tTxt))
			{
				char * pCopy = strdup(tTxt);
				pCopy = strupr(pCopy);
				vListImage.push_back(pCopy);
				iNum++;
			}
			else
				break;
		}
	}

	pTexCurr = NULL;
}

//-----------------------------------------------------------------------------
ViewImage::~ViewImage()
{
	int iI = vListImage.size();

	while (iI--)
	{
		free((void *)vListImage[iI]);
	}

	vListImage.clear();
}

//-----------------------------------------------------------------------------
void ViewImage::DrawAllImage()
{
	int iI = vListImage.size();

	if (iI)
	{
		TextureContainer	* pTex	= NULL;
		int					iTime	= 0;
		int					iJ		= 0;
		bool				bEnd	= true;
		float				fColor	= 0.f;
		bool				bSens	= false;
		bool				bActiveFade	= true;
		int					iAction	= 0;

		while (bEnd)
		{
			float iCurrentTime = ARX_TIME_Get();

			switch (iAction)
			{
				case 0:
					bSens		= true;
					bActiveFade	= true;
					iAction++;
					break;
				case 1:
					ARX_WARN("bSens set to false by default.");
					iAction++;
					break;
				case 2:
					ARX_WARN("bSens set to false by default.");

					if (!bActiveFade)
					{

						float fCTime	= iCurrentTime + 60000 ;
						ARX_CHECK_INT(fCTime);

						iTime	= ARX_CLEAN_WARN_CAST_INT(fCTime);


						iAction++;
					}

					break;
				case 3:
				{
					bool bEnd = false;

					if (pGetInfoDirectInput)
					{
						pGetInfoDirectInput->GetInput();

						for (int i = 0 ; i < 256 ; i++)
						{
							if (pGetInfoDirectInput->iOneTouch[i] > 0)
							{
								bEnd = true;
							}
						}
					}

					if (((iTime - iCurrentTime) < 0) ||
					        bEnd)
					{
						bSens		= false;
						bActiveFade	= true;
						iAction++;
					}
					else
					{
						ARX_WARN("bSens set to false by default.");
					}
				}
				break;
				case 4:
					ARX_WARN("bSens set to false by default.");

					if (!bActiveFade)
					{
						iAction		= 0;
					}

					break;
			}

			if (iAction == 1)
			{
				if (pTex)
				{
					delete pTex;
				}

				if (iJ >= iI) break;

				char * pName = vListImage[iJ];
				pTex = MakeTCFromFile(pName, 0);
				iJ++;
			}

			if (!danaeApp.DANAEStartRender()) continue;


			float fDepX = ARX_CLEAN_WARN_CAST_FLOAT(__max(0, ((DANAESIZX - pTex->m_dwWidth) >> 1)));
			float fDepY = ARX_CLEAN_WARN_CAST_FLOAT(__max(0, ((DANAESIZY - pTex->m_dwHeight) >> 1)));




			ARX_CHECK_NOT_NEG(DANAESIZX);
			ARX_CHECK_NOT_NEG(DANAESIZY);
			EERIEDrawBitmap(GDevice,
			                fDepX,
			                fDepY,
			                ARX_CLEAN_WARN_CAST_FLOAT(__min(pTex->m_dwWidth, ARX_CAST_ULONG(DANAESIZX))),
			                ARX_CLEAN_WARN_CAST_FLOAT(__min(pTex->m_dwHeight, ARX_CAST_ULONG(DANAESIZY))),
			                0.f,
			                pTex,
			                D3DRGB(fColor, fColor, fColor));



			danaeApp.DANAEEndRender();
			danaeApp.m_pFramework->ShowFrame();

			if (bActiveFade)
			{

				float fTGet = ARX_TIME_Get() - iCurrentTime ;
				ARX_CHECK_INT(fTGet);

				int iFrameTime	= ARX_CLEAN_WARN_CAST_INT(fTGet);


				float fIncFade = ((float)iFrameTime) * (.1f / 100.f);

				if (bSens)
				{
					fColor += fIncFade;

					if (fColor > 1.f)
					{
						fColor		= 1.f;
						bActiveFade	= false;
					}
				}
				else
				{
					fColor -= fIncFade;

					if (fColor < 0.f)
					{
						fColor		= 0.f;
						bActiveFade	= false;
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
void StartImageDemo()
{
	char tTxt[256];
	sprintf(tTxt, Project.workingdir);
	strcat(tTxt, "graph\\interface\\misc\\");
	ViewImage * pViewImage = new ViewImage(tTxt, "*.bmp");

	if (!pViewImage) return;

	danaeApp.DANAEEndRender();
	pViewImage->DrawAllImage();

	delete pViewImage;
}
