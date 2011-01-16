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
#include <stdio.h>
#include "danae.h"
#include "ARX_carte.h"
#include "ARX_levels.h"
#include "EERIEUtil.h"
#include "EERIETexture.h"
#include "Hermesmain.h"
#include "EERIEDraw.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

extern long FINAL_RELEASE;
extern long CURRENTLEVEL;
int iCreateMap = 0; // used to create mini-map bitmap
C_ARX_Carte *ARXCarte=NULL;
char ThisLevelMap[256];

//#############################################################################################
// Constructeur
//#############################################################################################
C_ARX_Carte::C_ARX_Carte(LPDIRECT3DDEVICE7 d,EERIE_BACKGROUND *bkg,int nbpix,int wrender,int hrender)
{
	this->device=d;
	this->background=bkg;
	this->nbpixels=nbpix;
	this->widthrender=wrender;
	this->heightrender=hrender;

	this->maxx=FLT_MIN;
	this->minx=FLT_MAX;
	this->maxz=FLT_MIN;
	this->minz=FLT_MAX;

	for(int j=0;j<bkg->Zsize;j++) 
	{
		for(int i=0;i<bkg->Xsize;i++) 
		{
			EERIE_BKG_INFO *eg=&bkg->Backg[i+j*bkg->Xsize];

			for(int lll=0;lll<eg->nbpoly;lll++)
			{
				EERIEPOLY *ep=&eg->polydata[lll];

				if(ep->type&POLY_IGNORE) continue;

				if(ep->type&POLY_TRANS) continue;
				
				int nb;

				if(ep->type&POLY_QUAD) nb=4;
				else nb=3;

				while(nb--)
				{
					if(ep->v[nb].sx<this->minx) this->minx=ep->v[nb].sx;

					if(ep->v[nb].sz<this->minz) this->minz=ep->v[nb].sz;

					if(ep->v[nb].sx>this->maxx) this->maxx=ep->v[nb].sx;

					if(ep->v[nb].sz>this->maxz) this->maxz=ep->v[nb].sz;
				}
			}
		}
	}

	this->posx=this->minx;
	this->posz=this->minz;

	this->width=this->nbpixels*(int)((this->maxx-this->minx)*this->background->Xmul);
	this->height=this->nbpixels*(int)((this->maxz-this->minz)*this->background->Zmul);

	this->ecx=((float)this->nbpixels)*this->background->Xmul;	//1/100
	this->ecz=((float)this->nbpixels)*this->background->Zmul;

	this->surfacetemp=NULL;
}
//#############################################################################################
// Render de la scene
//#############################################################################################
BOOL C_ARX_Carte::Render(void)
{
	if((!this->device)||(!this->background)) return E_FAIL;

	CalcFPS();
	device->Clear(0,NULL,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,0x00000000,1.0f,0L);

	if(!danaeApp.DANAEStartRender()) return E_FAIL;

	int depx = (int)(this->posx * this->background->Xmul);

	if(depx<0) depx=0;

	int depz=(int)(this->posz*this->background->Zmul);

	if(depz<0) depz=0;

	int	endx=this->widthrender/this->nbpixels;
	int	endz=this->heightrender/this->nbpixels;
	endx+=depx;

	if(endx>this->background->Xsize) endx=this->background->Xsize;

	endz+=depz;

	if(endz>this->background->Zsize) endz=this->background->Zsize;

	float yecran=0.f;
	float zsub=((float)depz)*this->background->Zdiv;

	for(int j=depz;j<endz;j++) 
	{
		float xecran=0.f;
		float xsub=((float)depx)*this->background->Xdiv;

		for(int i=depx;i<endx;i++) 
		{
			EERIE_BKG_INFO *eg=&this->background->Backg[i+j*this->background->Xsize];

			for(int lll=0;lll<eg->nbpoly;lll++)
			{
				EERIEPOLY *ep=&eg->polydata[lll];

				if (ep)
				{
					if(ep->type&POLY_IGNORE) continue;

					if(ep->type&POLY_TRANS) continue;
					
					int nb;

					if(ep->type&POLY_QUAD) nb=4;
					else nb=3;

					for(int k=0;k<nb;k++)
					{
						ep->tv[k].sx=xecran+(ep->v[k].sx-xsub)*this->ecx;
						ep->tv[k].sy=yecran+(ep->v[k].sz-zsub)*this->ecz;
						ep->tv[k].rhw=(1.f/ep->v[k].sy);

						if (ep->tv[k].rhw<0.f) ep->tv[k].rhw=0.f;

						if (ep->tv[k].rhw>1.f) ep->tv[k].rhw=1.f;

						ep->tv[k].sz=1.f-ep->tv[k].rhw;

						if (ep->tv[k].sz<0.f) ep->tv[k].sz=0.f;

						if (ep->tv[k].sz>1.f) ep->tv[k].sz=1.f;

						ep->tv[k].color=0xFFFFFFFF;
						ep->tv[k].tu=ep->v[k].tu;
						ep->tv[k].tv=ep->v[k].tv;
					}

					if(ep->tex)
						device->SetTexture(0,ep->tex->m_pddsSurface);
					else
						device->SetTexture(0,NULL);

					EERIEDRAWPRIM( device,	D3DPT_TRIANGLESTRIP,
											D3DFVF_TLVERTEX|D3DFVF_DIFFUSE,
											ep->tv,
											nb,
											0, EERIE_NOCOUNT );
				}

			}

			xsub+=this->background->Xdiv;
			xecran+=(float)this->nbpixels;
		}

		zsub+=this->background->Zdiv;
		yecran+=(float)this->nbpixels;
	}

	return S_OK;
}

//#############################################################################################
// Deplace la mapp
//#############################################################################################
void C_ARX_Carte::MoveMap(float newposx,float newposz)
{
	this->posx=newposx;
	this->posz=newposz;
}
//#############################################################################################
// Inc deplace mapp
//#############################################################################################
void C_ARX_Carte::IncMoveMap(float incx,float incz)
{
	this->posx+=incx;
	this->posz+=incz;
}
//#############################################################################################
// CreateSurfTemp
//#############################################################################################
BOOL C_ARX_Carte::CreateSurfaceTemp(CD3DFramework7 *framework)
{
	if(this->surfacetemp)
	{
		SAFE_RELEASE(this->surfacetemp);
	}

	LPDIRECTDRAW7 ddraw=framework->GetDirectDraw();

	if(!ddraw) return FALSE;

    // Setup the new surface desc
    DDSURFACEDESC2 ddsd;
    ddsd.dwSize=sizeof(ddsd);
	LPDIRECTDRAWSURFACE7	surfacerender=framework->GetRenderSurface();

	if(!surfacerender)
	{
        ddraw->Release();
        return FALSE;
	}

    surfacerender->GetSurfaceDesc(&ddsd);
    ddsd.dwFlags         = DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|DDSD_PIXELFORMAT;
    ddsd.ddsCaps.dwCaps  = DDSCAPS_SYSTEMMEMORY;
    ddsd.ddsCaps.dwCaps2 = 0L;
    ddsd.dwWidth         = (this->width+3)&~3;
    ddsd.dwHeight        = this->height;

    // Create a new surface for the texture
    HRESULT hr;

    if(FAILED(hr=ddraw->CreateSurface(&ddsd,&this->surfacetemp,NULL)))
    {
        ddraw->Release();
        return FALSE;
    }

	return TRUE;
}
//#############################################################################################
// BltOnSurfTemp
//#############################################################################################
BOOL C_ARX_Carte::BltOnSurfTemp(CD3DFramework7 *framework,int x,int y,int dw,int dh,int largs,int largh)
{
LPDIRECTDRAWSURFACE7	surfacerender;
RECT					src,dst;

	surfacerender=framework->GetRenderSurface();

	if(!surfacerender) return FALSE;

	int n=this->nbpixels>>1;
	src.left=n;
	src.top=n;
	src.right=largs+src.left-dw;
	src.bottom=largh+src.top-dh;
	dst.left=x;
	dst.top=y;
	dst.right=dst.left+largs-dw;
	dst.bottom=dst.top+largh-dh;

	if(FAILED(this->surfacetemp->Blt(&dst,surfacerender,&src,DDBLT_WAIT,NULL))) return FALSE;

	return TRUE;
}
//#############################################################################################
// BuildMap
//#############################################################################################
BOOL C_ARX_Carte::BuildMap(CD3DFramework7 *framework,char *name)
{
	if(!this->CreateSurfaceTemp(framework)) return FALSE;

float oldposx,oldposz;
	oldposx=this->posx;
	oldposz=this->posz;
	this->posx=this->minx;
	this->posz=this->minz;

	int taillex=(this->widthrender-(this->nbpixels>>1))/this->nbpixels;
	taillex--;
	float incx=((float)taillex)*100.f;
	taillex*=this->nbpixels;
	int tailley=(this->heightrender-(this->nbpixels>>1))/this->nbpixels;
	tailley--;
	float incy=((float)tailley)*100.f;
	tailley*=this->nbpixels;
	float incyy=0.f;
	
	int nbx=this->width/taillex;

	if(this->width%taillex) nbx++;

	int nby=this->height/tailley;

	if(this->height%tailley) nby++;

	int cposy=0;

	while(nby--)
	{
		int nbxx=nbx;
		int cposx=0;
		int dh=0;

		if((cposy+tailley)>this->height) dh=cposy+tailley-this->height;

		while(nbxx--)
		{
			this->Render();
			danaeApp.DANAEEndRender();

			int dw=0;

			if((cposx+taillex)>this->width) dw=cposx+taillex-this->width;

			if(!this->BltOnSurfTemp(framework,cposx,cposy,dw,dh,taillex,tailley)) return FALSE;

			cposx+=taillex;

			this->IncMoveMap(incx,0.f);
		}

		cposy+=tailley;
		this->MoveMap(this->minx,this->minz);
		incyy+=incy;
		this->IncMoveMap(0.f,incyy);
	}

	DDSURFACEDESC2 ddsd;
	ddsd.dwSize=sizeof(DDSURFACEDESC2);

	if(FAILED(this->surfacetemp->Lock(NULL,&ddsd,DDLOCK_READONLY|DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT,NULL)))
	{
		SAFE_RELEASE(this->surfacetemp);
		return FALSE;
	}

	if(	(ddsd.ddpfPixelFormat.dwRGBBitCount==4)||
		(ddsd.ddpfPixelFormat.dwRGBBitCount==8)||
		(ddsd.ddpfPixelFormat.dwRGBBitCount==24) )
	{
		MessageBox(NULL,"Desktop not in 16 or 32 bits","Map Generation Error!!!",0);
		SAFE_RELEASE(this->surfacetemp);
		return FALSE;
	}

	int tailleraw;

	if(ddsd.ddpfPixelFormat.dwRGBBitCount==16)
	{
		tailleraw=(ddsd.lPitch>>1)*ddsd.dwHeight*3;
	}
	else
	{
		tailleraw=(ddsd.lPitch>>2)*ddsd.dwHeight*3;
	}

	//enregistre la mapp
	int *mem=(int*)malloc(tailleraw);
	unsigned char *memc=(unsigned char *)mem;

	if(!mem)
	{
		SAFE_RELEASE(this->surfacetemp);
		return FALSE;
	}

	//info
	BITMAPFILEHEADER	bm;
	bm.bfType=(('M'<<8)+'B');
	bm.bfSize=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFO)-4+(tailleraw);
	bm.bfReserved1=0;
	bm.bfReserved2=0;
	bm.bfOffBits=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFO)-4;
	BITMAPINFO			bi;
	bi.bmiHeader.biSize=sizeof(BITMAPINFO)-4;
	bi.bmiHeader.biHeight=ddsd.dwHeight;
	bi.bmiHeader.biPlanes=1;
	bi.bmiHeader.biBitCount=24;
	bi.bmiHeader.biCompression=BI_RGB;
	bi.bmiHeader.biSizeImage=tailleraw;
	bi.bmiHeader.biXPelsPerMeter=0;
	bi.bmiHeader.biYPelsPerMeter=0;
	bi.bmiHeader.biClrUsed=0;
	bi.bmiHeader.biClrImportant=0;

	//crepie
	DWORD dwRMask=ddsd.ddpfPixelFormat.dwRBitMask;
	DWORD dwGMask=ddsd.ddpfPixelFormat.dwGBitMask;
	DWORD dwBMask=ddsd.ddpfPixelFormat.dwBBitMask;
	DWORD dwAMask=ddsd.ddpfPixelFormat.dwRGBAlphaBitMask;
	DWORD dwRShiftR=0;
	DWORD dwGShiftR=0;
	DWORD dwBShiftR=0;
	DWORD dwAShiftR=0;
	DWORD dwRShiftL=8;
	DWORD dwGShiftL=8;
	DWORD dwBShiftL=8;
	DWORD dwAShiftL=8;

	DWORD dwMask;

	for( dwMask=dwRMask; dwMask && !(dwMask&0x1); dwMask>>=1 ) dwRShiftR++;

	for( ; dwMask && 1; dwMask>>=1 ) dwRShiftL--;

	for( dwMask=dwGMask; dwMask && !(dwMask&0x1); dwMask>>=1 ) dwGShiftR++;

	for( ; dwMask && 1; dwMask>>=1 ) dwGShiftL--;

	for( dwMask=dwBMask; dwMask && !(dwMask&0x1); dwMask>>=1 ) dwBShiftR++;

	for( ; dwMask && 1; dwMask>>=1 ) dwBShiftL--;

	for( dwMask=dwAMask; dwMask && !(dwMask&0x1); dwMask>>=1 ) dwAShiftR++;

	for( ; dwMask && 1; dwMask>>=1 ) dwAShiftL--;

	nby=ddsd.dwHeight;

	if(ddsd.ddpfPixelFormat.dwRGBBitCount==16)
	{
		bi.bmiHeader.biWidth=ddsd.lPitch>>1;
	
		short *mems=(short*)ddsd.lpSurface;

		while(nby--)
		{
			nbx=ddsd.lPitch>>1;

			while(nbx--)
			{
				unsigned short col=*mems++;
				*memc++=(unsigned char)(((col&dwBMask)>>dwBShiftR)<<dwBShiftL);
				*memc++=(unsigned char)(((col&dwGMask)>>dwGShiftR)<<dwGShiftL);
				*memc++=(unsigned char)(((col&dwRMask)>>dwRShiftR)<<dwRShiftL);
			}
		}
	}
	else
	{
		bi.bmiHeader.biWidth=ddsd.lPitch>>2;
	
		int *mems=(int*)ddsd.lpSurface;

		while(nby--)
		{
			int nbx=ddsd.lPitch>>2;

			while(nbx--)
			{	
				int col=*mems++;
				*memc++=(unsigned char)((col&dwBMask)>>dwBShiftR);
				*memc++=(unsigned char)((col&dwGMask)>>dwGShiftR);
				*memc++=(unsigned char)((col&dwRMask)>>dwRShiftR);
			}
		}
	}

	this->surfacetemp->Unlock(NULL);

	//sauvegarde
	nby=0;

	FILE	*f;

	f=fopen(name,"wb");

	fwrite((void*)&bm,1,sizeof(BITMAPFILEHEADER),f);
	fwrite((void*)&bi,1,sizeof(BITMAPINFO)-4,f);
	fwrite((void*)mem,1,tailleraw,f);
	fclose(f);

	free((void*)mem);
	SAFE_RELEASE(this->surfacetemp);

	this->posx=oldposx;
	this->posz=oldposz;
	return TRUE;
}

//-----------------------------------------------------------------------------
BOOL NeedMapCreation()
{
	char name[256];
	GetLevelNameByNum(CURRENTLEVEL,name);	
	sprintf(ThisLevelMap,"%s\\Graph\\Levels\\Level%s\\map.bmp",Project.workingdir,name);

	if (PAK_FileExist(ThisLevelMap)) return FALSE;

	return TRUE;
}

//***********************************************************************************************
// EDITOR Feature ONLY !!! Creates a minimap to disk
//***********************************************************************************************
void DANAE_Manage_CreateMap()
{
	if (FINAL_RELEASE) return;

	SETCULL(GDevice,D3DCULL_CCW);
	iCreateMap++;

	if (iCreateMap==1)
	{			
		ARXCarte=new C_ARX_Carte(danaeApp.m_pFramework->GetD3DDevice(),ACTIVEBKG,4,danaeApp.m_pFramework->m_dwRenderWidth,danaeApp.m_pFramework->m_dwRenderHeight);			
	}

	if (iCreateMap==2)
	{
		ARXCarte->BuildMap(danaeApp.m_pFramework,ThisLevelMap);
	}

	if (iCreateMap==3)
	{
		delete ARXCarte;
		ARXCarte=NULL;
		iCreateMap=0;
	}
}
