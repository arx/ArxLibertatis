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
//////////////////////////////////////////////////////////////////////////////////////
//   @@        @@@        @@@                @@                           @@@@@     //
//   @@@       @@@@@@     @@@     @@        @@@@                         @@@  @@@   //
//   @@@       @@@@@@@    @@@    @@@@       @@@@      @@                @@@@        //
//   @@@       @@  @@@@   @@@  @@@@@       @@@@@@     @@@               @@@         //
//  @@@@@      @@  @@@@   @@@ @@@@@        @@@@@@@    @@@            @  @@@         //
//  @@@@@      @@  @@@@  @@@@@@@@         @@@@ @@@    @@@@@         @@ @@@@@@@      //
//  @@ @@@     @@  @@@@  @@@@@@@          @@@  @@@    @@@@@@        @@ @@@@         //
// @@@ @@@    @@@ @@@@   @@@@@            @@@@@@@@@   @@@@@@@      @@@ @@@@         //
// @@@ @@@@   @@@@@@@    @@@@@@           @@@  @@@@   @@@ @@@      @@@ @@@@         //
// @@@@@@@@   @@@@@      @@@@@@@@@@      @@@    @@@   @@@  @@@    @@@  @@@@@        //
// @@@  @@@@  @@@@       @@@  @@@@@@@    @@@    @@@   @@@@  @@@  @@@@  @@@@@        //
//@@@   @@@@  @@@@@      @@@      @@@@@@ @@     @@@   @@@@   @@@@@@@    @@@@@ @@@@@ //
//@@@   @@@@@ @@@@@     @@@@        @@@  @@      @@   @@@@   @@@@@@@    @@@@@@@@@   //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@      @@@@@      //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@       @@        //
//@@@    @@@  @@@ @@@@@                          @@            @@@                  //
//            @@@ @@@                           @@             @@        STUDIOS    //
//////////////////////////////////////////////////////////////////////////////////////                                                                                     
//////////////////////////////////////////////////////////////////////////////////////
// ARX_Scene
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		ARX Scene Management
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////
#include "ARX_Scene.h"
#include "ARX_Spells.h"
#include "ARX_Sound.h"
#include "ARX_Particles.h"
#include "ARX_Draw.h"
#include "ARX_player.h"
#include "ARX_paths.h"
#include "ARX_interface.h"
#include "ARX_time.h"
#include "ARX_HWTransform.h"
#include "ARX_menu2.h"

#include <HERMESMain.h>
#include <EERIELight.h>
#include <EERIEDraw.h>
#include <EERIEAnim.h>
#include <EERIEUtil.h>
#include <EERIEMath.h>

#define DIRECTINPUT_VERSION 0x0700
#include <dinput.h>

#include <stdio.h>
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
extern long USE_LIGHT_OPTIM;


using namespace std;

//-----------------------------------------------------------------------------
#define MAX_OUT 4 
#define VAL_THRESHOLD 100.f
#define PASSS 0.5f 
#define PASS 50.f 

#define MAX_DIST_BUMP			(400.f)

//-----------------------------------------------------------------------------
extern HANDLE LIGHTTHREAD;
extern EERIE_3DOBJ * eyeballobj;
extern long NEED_TEST_TEXT;
extern long DEBUGCODE;
extern long VF_CLIP_IO;
extern long DEBUG_FRUSTRUM;
extern long HIDEANCHORS;
extern long EXTERNALVIEW;
extern long USE_D3DFOG;
extern long WATERFX;
extern long REFLECTFX;
extern bool ARXPausedTimer;
extern bool bUSE_D3DFOG_INTER;
long LAST_PORTALS_COUNT=0;
//-----------------------------------------------------------------------------
extern TextureContainer *enviro;
extern long ZMAPMODE;
extern bool bRenderInterList;
extern bool bALLOW_BUMP;
extern unsigned long ulBKGColor;
extern float fZFogStartWorld;
extern float fZFogEndWorld;
extern CDirectInput *pGetInfoDirectInput;
extern CMenuConfig *pMenuConfig;
//-----------------------------------------------------------------------------
EERIEPOLY VF_Center;
EERIEPOLY VF_Top;
EERIEPOLY VF_Bottom;
EERIEPOLY VF_Front;

EERIEPOLY * TransPol[MAX_TRANSPOL];
D3DTLVERTEX InterTransPol[MAX_INTERTRANSPOL][4];
EERIE_FACE * InterTransFace[MAX_INTERTRANSPOL];
TextureContainer * InterTransTC[MAX_INTERTRANSPOL];

float WATEREFFECT=0.f;
long INTERTRANSPOLYSPOS=0;

long TRANSPOLYSPOS=0;
long D3DTRANSFORM=0;
long SHOWSHADOWS=1;
long TESTMODE=1;
long FRAME_COUNT=0;

long LAST_ROOM=-1;

float fZFogStart=0.3f;
float fZFogEnd=.5f;

long iTotPoly;
unsigned long FrameCount;

CMY_DYNAMIC_VERTEXBUFFER *pDynamicVertexBuffer;
CMY_DYNAMIC_VERTEXBUFFER *pDynamicVertexBufferBump;			// VB based on pDynamicVertexBuffer (for BUMP only).
CMY_DYNAMIC_VERTEXBUFFER *pDynamicVertexBufferTransform;
CMY_DYNAMIC_VERTEXBUFFER *pDynamicVertexBuffer_TLVERTEX;	// VB using TLVERTEX format.
CMY_DYNAMIC_VERTEXBUFFER *pDynamicVertexBuffer_D3DVERTEX3_T;// VB using D3DVERTEX3_T format (for BUMP).

EERIE_FRUSTRUM_PLANE efpPlaneNear;
EERIE_FRUSTRUM_PLANE efpPlaneFar;

vector<EERIEPOLY*> vPolyWater;
vector<EERIEPOLY*> vPolyLava;
vector<EERIEPOLY*> vPolyVoodooMetal;

bool bOLD_CLIPP=false;

void PopAllTriangleListTransparency();

extern long TSU_TEST;
extern bool bGATI8500;
extern bool bSoftRender;

//-----------------------------------------------------------------------------
CMY_DYNAMIC_VERTEXBUFFER::CMY_DYNAMIC_VERTEXBUFFER(unsigned short _ussMaxVertex,unsigned long _uslFormat)
{
	uslFormat=_uslFormat;
	ussMaxVertex=_ussMaxVertex;
	ussNbVertex=0;
	ussNbIndice=0;

	D3DVERTEXBUFFERDESC d3dvbufferdesc;
	d3dvbufferdesc.dwSize=sizeof(D3DVERTEXBUFFERDESC);

	int iFlag=D3DVBCAPS_WRITEONLY;

	if(!(danaeApp.m_pDeviceInfo->ddDeviceDesc.dwDevCaps&D3DDEVCAPS_HWTRANSFORMANDLIGHT))
	{
		iFlag|=D3DVBCAPS_SYSTEMMEMORY;
	}

	d3dvbufferdesc.dwCaps=iFlag;
	d3dvbufferdesc.dwFVF=uslFormat;
	d3dvbufferdesc.dwNumVertices=ussMaxVertex;
	
	pVertexBuffer=NULL;
	danaeApp.m_pD3D->CreateVertexBuffer(	&d3dvbufferdesc,
											&pVertexBuffer,
											0);

	pussIndice=(unsigned short*)malloc(ussMaxVertex*4*2);
}

//-----------------------------------------------------------------------------
CMY_DYNAMIC_VERTEXBUFFER::~CMY_DYNAMIC_VERTEXBUFFER()
{
	if(pVertexBuffer) pVertexBuffer->Release();

	if(pussIndice) free((void*)pussIndice);
}

//-----------------------------------------------------------------------------
void* CMY_DYNAMIC_VERTEXBUFFER::Lock(unsigned int uiFlag)
{
	void		*pVertex;

	if(FAILED(pVertexBuffer->Lock(	DDLOCK_WRITEONLY|uiFlag,
												&pVertex				,
												NULL					) ) )
	{
		return NULL;
	}

	return pVertex;
}

//-----------------------------------------------------------------------------
bool CMY_DYNAMIC_VERTEXBUFFER::UnLock()
{
	pVertexBuffer->Unlock();
	return true;
}

 
//-----------------------------------------------------------------------------
void PopOneTriangleListClipp(D3DTLVERTEX *_pVertex,int *_piNbVertex)
{

		D3DTLVERTEX *pD3DVertex=_pVertex;
	
	while(*_piNbVertex)
	{
		int iOldNbVertex=pDynamicVertexBufferTransform->ussNbVertex;
		pDynamicVertexBufferTransform->ussNbIndice=0;

		SMY_D3DVERTEX *pVertex;


		int iMin  = min(*_piNbVertex,pDynamicVertexBufferTransform->ussMaxVertex);
		ARX_CHECK_USHORT(iMin);

		unsigned short iNbVertex=ARX_CLEAN_WARN_CAST_USHORT(iMin); 	

		
		pDynamicVertexBufferTransform->ussNbVertex+=iNbVertex;

		if(pDynamicVertexBufferTransform->ussNbVertex>=pDynamicVertexBufferTransform->ussMaxVertex)
		{
			pVertex=(SMY_D3DVERTEX*)pDynamicVertexBufferTransform->Lock(DDLOCK_DISCARDCONTENTS);
			pDynamicVertexBufferTransform->ussNbVertex=iNbVertex;
			iOldNbVertex=0;
		} 
		else
		{
			pVertex=(SMY_D3DVERTEX*)pDynamicVertexBufferTransform->Lock(DDLOCK_NOOVERWRITE);
			pVertex+=iOldNbVertex;
		}
		
		*_piNbVertex-=iNbVertex;
		iNbVertex/=3;

		while(iNbVertex--)
		{
			pVertex->x=pD3DVertex->sx;
			pVertex->y=-pD3DVertex->sy;
			pVertex->z=pD3DVertex->sz;
			pVertex->color=pD3DVertex->color;
			pVertex->tu=pD3DVertex->tu;
			pVertex->tv=pD3DVertex->tv;
			pVertex++;
			pD3DVertex++;
			pVertex->x=pD3DVertex->sx;
			pVertex->y=-pD3DVertex->sy;
			pVertex->z=pD3DVertex->sz;
			pVertex->color=pD3DVertex->color;
			pVertex->tu=pD3DVertex->tu;
			pVertex->tv=pD3DVertex->tv;
			pVertex++;
			pD3DVertex++;
			pVertex->x=pD3DVertex->sx;
			pVertex->y=-pD3DVertex->sy;
			pVertex->z=pD3DVertex->sz;
			pVertex->color=pD3DVertex->color;
			pVertex->tu=pD3DVertex->tu;
			pVertex->tv=pD3DVertex->tv;
			pVertex++;
			pD3DVertex++;
		}
		
		pDynamicVertexBufferTransform->UnLock();
		
		GDevice->DrawPrimitiveVB(	D3DPT_TRIANGLELIST,
									pDynamicVertexBufferTransform->pVertexBuffer,
									iOldNbVertex,
									pDynamicVertexBufferTransform->ussNbVertex-iOldNbVertex,
									0 );
	}
}

//------------------------------------------------------------------------------
//	This function will use appropriate VB depending on vertex format. (using template)
/************************************************************************/
/*  HRESULT ARX_DrawPrimitiveVB(	
 *	LPDIRECT3DDEVICE7	_d3dDevice,			: pointer to the DX7 device
 *	D3DPRIMITIVETYPE	_dptPrimitiveType,	: primitive type to draw
 *	DWORD				_dwVertexTypeDesc	: vertex format.
 *	_LPVERTEX_			_pVertex,			: pointer to the first vertex to render
 *	int*				_piNbVertex,		: number of vertices to render (must be positive)
 *	DWORD				_dwFlags            : optionally flag for DrawPrimitiveVB.
 *	CMY_DYNAMIC_VERTEXBUFFER*	_pDynamicVB	: mandatory : dynamicVertexBuffer to use for rendering.
 *	
 *	@return S_OK if function exit correctly.
/************************************************************************/
template<class _LPVERTEX_>
__declspec(dllexport) HRESULT ARX_DrawPrimitiveVB(	LPDIRECT3DDEVICE7			_d3dDevice, 
													D3DPRIMITIVETYPE			_dptPrimitiveType, 
													DWORD						_dwVertexTypeDesc,
													_LPVERTEX_					_pVertex, 
													int*						_piNbVertex, 
													DWORD						_dwFlags,
													CMY_DYNAMIC_VERTEXBUFFER*	_pDynamicVB)
{
	_LPVERTEX_					pD3DVertex	=	_pVertex;
	HRESULT						h_result	=	S_OK;
	CMY_DYNAMIC_VERTEXBUFFER*	pDVB		=	_pDynamicVB;

	ARX_CHECK( pDVB );

	while( *_piNbVertex )
	{
		_LPVERTEX_		pVertex			=	NULL;
		int				iOldNbVertex	=	pDVB->ussNbVertex;
		pDVB->ussNbIndice				=	0;
		unsigned short iNbVertex		=	(unsigned short) min( *_piNbVertex, pDVB->ussMaxVertex ); //don't overload VB

		pDVB->ussNbVertex				+=	iNbVertex;

		if( pDVB->ussNbVertex >= pDVB->ussMaxVertex )
		{
			pVertex						=	(_LPVERTEX_)pDVB->Lock( DDLOCK_DISCARDCONTENTS );
			pDVB->ussNbVertex			=	iNbVertex;
			iOldNbVertex				=	0;
		} 
		else
		{
			pVertex						=	(_LPVERTEX_)pDVB->Lock( DDLOCK_NOOVERWRITE );
			pVertex						+=	iOldNbVertex;
		}

		*_piNbVertex					-=	iNbVertex;

		if( D3DPT_TRIANGLELIST == _dptPrimitiveType )
		{
			iNbVertex		/=	3;
			while( iNbVertex-- )
			{
				(*pVertex++)			=	(*pD3DVertex++); //structure copy (3 times)
				(*pVertex++)			=	(*pD3DVertex++);
 				(*pVertex++)			=	(*pD3DVertex++);
			}
		}
		else
		{
			(*pVertex++)				=	(*pD3DVertex++); //structure copy
		}

		pDVB->UnLock();

		HRESULT	h_resultDPVB = _d3dDevice->DrawPrimitiveVB(	_dptPrimitiveType,
															pDVB->pVertexBuffer,
															iOldNbVertex,
															pDVB->ussNbVertex - iOldNbVertex,
															_dwFlags );
		if( h_resultDPVB != S_OK )
			h_result = h_resultDPVB; //Getting last error for return in case of bad DrawPrimitiveVB result
	}

	return h_result;
}

//------------------------------------------------------------------------------
// Add function manager to call template-function.
/************************************************************************/
/*  HRESULT ARX_DrawPrimitiveVB(	
 *	LPDIRECT3DDEVICE7	_d3dDevice,			: pointer to the DX7 device
 *	D3DPRIMITIVETYPE	_dptPrimitiveType,	: primitive type to draw
 *	DWORD				_dwVertexTypeDesc	: vertex format.
 *	LPVOID				_pVertex,			: pointer to the first vertex to render
 *	int*				_piNbVertex,		: number of vertices to render (must be positive)
 *	DWORD				_dwFlags )          : optionally flag for DrawPrimitiveVB.
 *	
 *	@return S_OK if function exit correctly.
/************************************************************************/
HRESULT ARX_DrawPrimitiveVB(	LPDIRECT3DDEVICE7	_d3dDevice, 
								D3DPRIMITIVETYPE	_dptPrimitiveType, 
								DWORD				_dwVertexTypeDesc,
								LPVOID				_pVertex, 
								int*				_piNbVertex, 
								DWORD				_dwFlags )
{
	HRESULT h_result = S_FALSE;

	if( _pVertex )
	{
		switch( _dwVertexTypeDesc )
		{
		case FVF_D3DVERTEX:
			h_result	=	ARX_DrawPrimitiveVB(	_d3dDevice,
													_dptPrimitiveType,
													_dwVertexTypeDesc,
													(SMY_D3DVERTEX*) _pVertex,
													_piNbVertex,
													_dwFlags,
													pDynamicVertexBufferTransform);
			break;
		case D3DFVF_TLVERTEX:
			h_result	=	ARX_DrawPrimitiveVB(	_d3dDevice,
													_dptPrimitiveType,
													_dwVertexTypeDesc,
													(D3DTLVERTEX*) _pVertex,
													_piNbVertex,
													_dwFlags,
													pDynamicVertexBuffer_TLVERTEX);
			break;
		case FVF_D3DVERTEX3:
			h_result	=	ARX_DrawPrimitiveVB(	_d3dDevice,
													_dptPrimitiveType,
													_dwVertexTypeDesc,
													(SMY_D3DVERTEX3*) _pVertex,
													_piNbVertex,
													_dwFlags,
													pDynamicVertexBuffer);
			break;
		case FVF_D3DVERTEX3_T:
			h_result	=	ARX_DrawPrimitiveVB(	_d3dDevice,
													_dptPrimitiveType,
													_dwVertexTypeDesc,
													(SMY_D3DVERTEX3_T*) _pVertex,
													_piNbVertex,
													_dwFlags,
													pDynamicVertexBuffer_D3DVERTEX3_T);
			break;
		default:
			ARX_LOG_ERROR("FVF is not supported by ARX_DrawPrimitiveVB");
			ARX_CHECK(false && "FVF is not supported by ARX_DrawPrimitiveVB");
		}
	}

	return h_result;
}

//*************************************************************************************
//*************************************************************************************
void ApplyWaterFXToVertex(EERIE_3D * odtv,D3DTLVERTEX * dtv,float power)
{
	power=power*0.05f;
	dtv->tu+=EEsin((WATEREFFECT+odtv->x))*power;
	dtv->tv+=EEcos((WATEREFFECT+odtv->z))*power;
}

void ApplyLavaGlowToVertex(EERIE_3D * odtv,D3DTLVERTEX * dtv,float power)
{
	register float f;
	register long lr,lg,lb;
	power=1.f-(EEsin((WATEREFFECT+odtv->x+odtv->z))*0.05f)*power;	
	f=(float)(long)((dtv->color>>16) & 255)*power;
	F2L(f,&lr);
	lr=clipByte(lr);
	
	f=(float)(long)((dtv->color>>8) & 255)*power;
	F2L(f,&lg);
	lg=clipByte(lg);

	f=(float)(long)((dtv->color) & 255)*power;
	F2L(f,&lb);
	lb=clipByte(lb);

	dtv->color=0xFF000000L | (lr << 16) | (lg << 8) | lb;
}

void ComputeFogVertex(D3DTLVERTEX *v)
{
	float VertexDist=1.f/v->rhw;

	if(VertexDist<fZFogStartWorld)
	{
		v->specular=0xFF000000;
	}
	else
	{
		if(VertexDist>fZFogEndWorld)
		{
			v->specular=0;
		}
		else
		{
			//LINEAR
			float fDist=(fZFogEndWorld-VertexDist)*255.f/(fZFogEndWorld-fZFogStartWorld);
			long lAlpha;
			F2L(fDist,&lAlpha);
			v->specular=((lAlpha)<<24);
		}
	}
}

void ComputeSingleFogVertex(D3DTLVERTEX *v)
{
	if(bUSE_D3DFOG_INTER) return;

	ComputeFogVertex(v);
}

void ComputeFog(D3DTLVERTEX * v, long to)
{

	if(bUSE_D3DFOG_INTER) return;

	for (long k=0;k<to;k++) 
	{
		ComputeFogVertex(&v[k]);
			}
}

__forceinline void ManageLavaWater(EERIEPOLY * ep, const long to, const unsigned long tim)
{
	if ((ep->type & POLY_WATER) || (ep->type & POLY_LAVA) )
	{
		for (long k=0;k<to;k++) 
		{
			ep->tv[k].tu=ep->v[k].tu;
			ep->tv[k].tv=ep->v[k].tv;
			
			
			if (ep->type & POLY_LAVA)
			{
				ApplyWaterFXToVertex((EERIE_3D *)&ep->v[k], &ep->tv[k], 0.35f); 
				ApplyLavaGlowToVertex((EERIE_3D *)&ep->v[k],&ep->tv[k],0.6f);

				if (rnd()>0.995f) 
				{
					if (ep->type & POLY_FALL)
					{
						EERIE_3D pos;
						pos.x=ep->v[k].sx+ep->norm.x*20.f;
						pos.y=ep->v[k].sy+ep->norm.y*20.f;
						pos.z=ep->v[k].sz+ep->norm.z*20.f;
					}

					
				}
			}
			else ApplyWaterFXToVertex((EERIE_3D *)&ep->v[k],&ep->tv[k],0.35f);
		}					
	}	

	if (ep->type & POLY_FALL)
	{
		if (ep->type & POLY_LAVA)
			for (long k=0;k<to;k++) 
			{
				ep->tv[k].tv-=(float)(tim)*DIV12000;
			}
			else
				for (long k=0;k<to;k++) 
				{
					ep->tv[k].tv-=(float)(tim)*DIV1000;
				}
	}
}

void ManageWater_VertexBuffer(EERIEPOLY * ep, const long to, const unsigned long tim,SMY_D3DVERTEX *_pVertex)
{
	for (long k=0;k<to;k++) 
	{
		ep->tv[k].tu=ep->v[k].tu;
		ep->tv[k].tv=ep->v[k].tv;
		
		ApplyWaterFXToVertex((EERIE_3D *)&ep->v[k],&ep->tv[k],0.35f);
			
		if(ep->type&POLY_FALL)
		{
			ep->tv[k].tv-=(float)(tim)*DIV1000;
		}
		
		_pVertex[ep->uslInd[k]].tu=ep->tv[k].tu;
		_pVertex[ep->uslInd[k]].tv=ep->tv[k].tv;
	}					
}

void ManageLava_VertexBuffer(EERIEPOLY * ep, const long to, const unsigned long tim,SMY_D3DVERTEX *_pVertex)
{
	for (long k=0;k<to;k++) 
	{
		ep->tv[k].tu=ep->v[k].tu;
		ep->tv[k].tv=ep->v[k].tv;
		
		ApplyWaterFXToVertex((EERIE_3D *)&ep->v[k],&ep->tv[k],0.35f); //0.25f
		ApplyLavaGlowToVertex((EERIE_3D *)&ep->v[k],&ep->tv[k],0.6f);
			
		if(ep->type&POLY_FALL)
		{
			ep->tv[k].tv-=(float)(tim)*DIV12000;
		}
		
		_pVertex[ep->uslInd[k]].tu=ep->tv[k].tu;
		_pVertex[ep->uslInd[k]].tv=ep->tv[k].tv;
	}					
}



extern D3DMATRIX ProjectionMatrix;
void specialEE_RTP2(D3DTLVERTEX *in,D3DTLVERTEX *out)
{
	register EERIE_TRANSFORM * et=(EERIE_TRANSFORM *)&ACTIVECAM->transform;
	out->sx = in->sx - et->posx;	
	out->sy = in->sy - et->posy;
	out->sz = in->sz - et->posz;

	register float temp =(out->sz*et->ycos) - (out->sx*et->ysin);
	out->sx = (out->sz*et->ysin) + (out->sx*et->ycos);	
	out->sz = (out->sy*et->xsin) + (temp*et->xcos);	
	out->sy = (out->sy*et->xcos) - (temp*et->xsin);

	float fZTemp = 1.f / out->sz;
		
	out->sz = fZTemp * ProjectionMatrix._33 + ProjectionMatrix._43; 
		out->sx=out->sx*ProjectionMatrix._11*fZTemp+et->xmod;
		out->sy=out->sy*ProjectionMatrix._22*fZTemp+et->ymod;
		out->rhw=fZTemp;
	
}

__forceinline long EERIERTPPoly2(EERIEPOLY *ep)
{
	specialEE_RTP2(&ep->v[0],&ep->tv[0]);	
	specialEE_RTP2(&ep->v[1],&ep->tv[1]);
	specialEE_RTP2(&ep->v[2],&ep->tv[2]);	

	if (ep->type & POLY_QUAD) specialEE_RTP2(&ep->v[3],&ep->tv[3]);	
	else ep->tv[3].sz=1.f;

	if ((ep->tv[0].sz<=0.f) &&
		(ep->tv[1].sz<=0.f) &&
		(ep->tv[2].sz<=0.f) &&
		(ep->tv[3].sz<=0.f) ) return 0;

	return 1;
}


BOOL IsSphereInFrustrum(float radius,EERIE_3D * point,EERIE_FRUSTRUM * frustrum);
bool FrustrumsClipSphere(EERIE_FRUSTRUM_DATA * frustrums,EERIE_SPHERE * sphere)
{
	float dists=sphere->origin.x*efpPlaneNear.a + sphere->origin.y*efpPlaneNear.b + sphere->origin.z*efpPlaneNear.c + efpPlaneNear.d;

	if (dists+sphere->radius>0)
	{	
		for (long i=0;i<frustrums->nb_frustrums;i++)
		{
			if (IsSphereInFrustrum(sphere->radius,(EERIE_3D *)&sphere->origin,&frustrums->frustrums[i]))
				return FALSE;
		}
	}

	return TRUE;
}
bool	EEVisibleSphere(EERIE_3D * pos,float radius)
{
	if (EEDistance3D(pos,&ACTIVECAM->pos)-radius>ACTIVECAM->cdepth*0.5f)
		return false;

	long room_num=ARX_PORTALS_GetRoomNumForPosition(pos);

	if (room_num>=0)
	{
		EERIE_SPHERE sphere;
		sphere.origin.x=pos->x;
		sphere.origin.y=pos->y;
		sphere.origin.z=pos->z;
		sphere.radius=radius;
							
		EERIE_FRUSTRUM_DATA * frustrums=&RoomDraw[room_num].frustrum;

		if (FrustrumsClipSphere(frustrums,&sphere))
			return false;
	}

	return true;
}
bool VisibleSphere(float x,float y,float z,float radius)
{
	EERIE_3D pos;
	pos.x=x;
	pos.y=y;
	pos.z=z;
	return EEVisibleSphere(&pos,radius);
}
BOOL IsInFrustrum(EERIE_3D * point,EERIE_FRUSTRUM * frustrum);
BOOL IsBBoxInFrustrum(EERIE_3D_BBOX * bbox,EERIE_FRUSTRUM * frustrum)
{
	EERIE_3D point;
	point.x=bbox->min.x;
	point.y=bbox->min.y;
	point.z=bbox->min.z;

	if (!IsInFrustrum(&point,frustrum))
	{
		EERIE_3D point;
		point.x=bbox->max.x;
		point.y=bbox->min.y;
		point.z=bbox->min.z;

		if (!IsInFrustrum(&point,frustrum))
		{
			EERIE_3D point;
			point.x=bbox->max.x;
			point.y=bbox->max.y;
			point.z=bbox->min.z;

			if (!IsInFrustrum(&point,frustrum))
			{
				EERIE_3D point;
				point.x=bbox->min.x;
				point.y=bbox->max.y;
				point.z=bbox->min.z;

				if (!IsInFrustrum(&point,frustrum))
				{
					EERIE_3D point;
					point.x=bbox->min.x;
					point.y=bbox->min.y;
					point.z=bbox->max.z;

					if (!IsInFrustrum(&point,frustrum))
					{
						EERIE_3D point;
						point.x=bbox->max.x;
						point.y=bbox->min.y;
						point.z=bbox->max.z;

						if (!IsInFrustrum(&point,frustrum))
						{
							EERIE_3D point;
							point.x=bbox->max.x;
							point.y=bbox->max.y;
							point.z=bbox->max.z;

							if (!IsInFrustrum(&point,frustrum))
							{
								EERIE_3D point;
								point.x=bbox->min.x;
								point.y=bbox->max.y;
								point.z=bbox->max.z;

								if (!IsInFrustrum(&point,frustrum))
								{
									return	FALSE;
								}
							}
						}
					}	
				}
			}
		}
	}	

	return TRUE;
}

bool FrustrumsClipBBox3D(EERIE_FRUSTRUM_DATA * frustrums,EERIE_3D_BBOX * bbox)
{
	for (long i=0;i<frustrums->nb_frustrums;i++)
	{
		if (IsBBoxInFrustrum(bbox,&frustrums->frustrums[i]))
			return FALSE;
	}

	return FALSE;
}
PORTAL_ROOM_DRAW * RoomDraw=NULL;
long NbRoomDraw=0;
long * RoomDrawList=NULL;
long NbRoomDrawList=0;
long TotalRoomDrawList=0;

BOOL ARX_SCENE_PORTAL_Basic_ClipIO(INTERACTIVE_OBJ * io)
{
	if (EDITMODE) return FALSE;

	if (io==inter.iobj[0]) return FALSE;

	if ((io) && (io->ioflags & IO_FORCEDRAW)) return FALSE;

	if (USE_PORTALS && portals)
	{
		EERIE_3D posi;
		posi.x=io->pos.x;
		posi.y=io->pos.y-20;
		posi.z=io->pos.z;

		if (io->room_flags & 1)
			UpdateIORoom(io);

		long room_num = io->room; 

		{
			if (room_num==-1)
			{
				posi.y=io->pos.y-120;
				room_num=ARX_PORTALS_GetRoomNumForPosition(&posi);
			}

			if (	(room_num>=0) 
				&&	(RoomDraw)
				&&	(RoomDraw[room_num].count))				
			{

			switch (USE_PORTALS)
			{
				case 2:
				case 3:
				case 4:
					EERIE_SPHERE sphere;

					if (io->ioflags & IO_ITEM)
					{
						sphere.origin.x=io->pos.x;
						sphere.origin.y=io->pos.y-40.f;
						sphere.origin.z=io->pos.z;

						if (io->ioflags & IO_MOVABLE)
							sphere.radius=160.f;
							else sphere.radius = 75.f; 
					}
					else if (io->ioflags & IO_FIX)
					{
						sphere.origin.x=io->pos.x;
						sphere.origin.y=io->pos.y-60.f;
						sphere.origin.z=io->pos.z;
						sphere.radius=340.f;
					}
					else if (io->ioflags & IO_NPC)
					{
						sphere.origin.x=io->pos.x;
						sphere.origin.y=io->pos.y-120.f;
						sphere.origin.z=io->pos.z;
						sphere.radius=120.f;
					}
						
					EERIE_FRUSTRUM_DATA * frustrums=&RoomDraw[room_num].frustrum;

					if (FrustrumsClipSphere(frustrums,&sphere))
					{
						if (io)
						{
							io->bbox1.x=(short)-1;
							io->bbox2.x=(short)-1;
							io->bbox1.y=(short)-1;
							io->bbox2.y=(short)-1;		
						}

						return TRUE;
					}
			}
			}
			else return FALSE;
		}
	}

	return FALSE;
}
//*********************************************************************************************************************
// BOOL ARX_SCENE__PORTAL_ClipIO(INTERACTIVE_OBJ * io,EERIE_3DOBJ * eobj,EERIE_3D * position,EERIE_3D * bboxmin,EERIE_3D * bboxmax)
//---------------------------------------------------------------------------------------------------------------------
// USAGE/FUNCTION
//   io can be NULL if io is valid io->bbox3D contains 3D world-bbox
//   bboxmin & bboxmax ARE in fact 2D-screen BBOXes using only (x,y).
// RETURN:
//   return TRUE if IO cannot be seen, FALSE if visible
//---------------------------------------------------------------------------------------------------------------------
// TODO:
//   Implement all Portal Methods
//   Return a reduced clipbox which can be used for polys clipping in the case of partial visibility
//*********************************************************************************************************************
BOOL ARX_SCENE_PORTAL_ClipIO(INTERACTIVE_OBJ * io,EERIE_3DOBJ * eobj,EERIE_3D * position,EERIE_3D * bboxmin,EERIE_3D * bboxmax)
{
	if (EDITMODE) return FALSE;

	if (io==inter.iobj[0]) return FALSE;

	if ((io) && (io->ioflags & IO_FORCEDRAW)) return FALSE;

	if (USE_PORTALS && portals)
	{
		EERIE_3D posi;
		posi.x=position->x;
		posi.y=position->y-60; //20
		posi.z=position->z;
		long room_num;

		if (io)
		{
			if (io->room_flags & 1)
				UpdateIORoom(io);

			room_num=io->room;//
		}
		else
		{
			room_num=ARX_PORTALS_GetRoomNumForPosition(&posi);
		}

		if (room_num==-1)
		{
			posi.y=position->y-120;
			room_num=ARX_PORTALS_GetRoomNumForPosition(&posi);
		}

		if ((room_num >= 0) && (RoomDraw)) 
		{
			if (RoomDraw[room_num].count==0)
			{
				if (io)
				{
					io->bbox1.x=(short)-1;
					io->bbox2.x=(short)-1;
					io->bbox1.y=(short)-1;
					io->bbox2.y=(short)-1;		
				}

				return TRUE;
			}

			switch (USE_PORTALS)
			{
				case 1: // 2D portal
				{
					EERIE_2D_BBOX * bbox=&RoomDraw[room_num].bbox;

					if (	bbox->min.x > BBOXMAX.x || BBOXMIN.x > bbox->max.x
						||	bbox->min.y > BBOXMAX.y || BBOXMIN.y > bbox->max.y)
					{
						if (io)
						{
							io->bbox1.x=(short)-1;
							io->bbox2.x=(short)-1;
							io->bbox1.y=(short)-1;
							io->bbox2.y=(short)-1;		
						}

						return TRUE;
					}
				}
				break;
				case 2:
				case 3:
				case 4:

					if (io)
					{						
						EERIE_SPHERE sphere;
						sphere.origin.x=(io->bbox3D.min.x+io->bbox3D.max.x)*DIV2;
						sphere.origin.y=(io->bbox3D.min.y+io->bbox3D.max.y)*DIV2;
						sphere.origin.z=(io->bbox3D.min.z+io->bbox3D.max.z)*DIV2;
						sphere.radius=TRUEDistance3D(sphere.origin.x,sphere.origin.y,sphere.origin.z,
											io->bbox3D.min.x,io->bbox3D.min.y,io->bbox3D.min.z)+10.f;
						
						EERIE_FRUSTRUM_DATA * frustrums=&RoomDraw[room_num].frustrum;

						if (FrustrumsClipSphere(frustrums,&sphere))
						{
							if (io)
							{
								io->bbox1.x=(short)-1;
								io->bbox2.x=(short)-1;
								io->bbox1.y=(short)-1;
								io->bbox2.y=(short)-1;		
							}

							return TRUE;
						}

						if (FrustrumsClipBBox3D(frustrums,&io->bbox3D))
						{
							if (io)
							{
								io->bbox1.x=(short)-1;
								io->bbox2.x=(short)-1;
								io->bbox1.y=(short)-1;
								io->bbox2.y=(short)-1;		
							}

							return TRUE;
						}
					}

				break;
			}
		}
	}

	return FALSE;
}

long ARX_PORTALS_GetRoomNumForPosition2(EERIE_3D * pos,long flag,float * height)
{
	
	EERIEPOLY * ep; 

	if (flag & 1)
	{
		ep=CheckInPolyPrecis(pos->x,pos->y-150.f,pos->z);

		if (!ep)
			ep=CheckInPolyPrecis(pos->x,pos->y-1.f,pos->z);
	}
	else
		ep=CheckInPoly(pos->x,pos->y,pos->z);

	if ((ep) && (ep->room>-1))	
	{
		if (height) *height=ep->center.y;

		return ep->room;
	}

	// Security... ?
	ep=GetMinPoly(pos->x,pos->y,pos->z);

	if ((ep) && (ep->room>-1))	
	{
		if (height) *height=ep->center.y;

		return ep->room;
	}
	else if (!(flag & 1))
	{
		ep=CheckInPolyPrecis(pos->x,pos->y,pos->z);

		if ((ep) && (ep->room>-1))	
		{
			if (height) *height=ep->center.y;

			return ep->room;
		}
	}

	if (flag & 2)
	{
		float off=20.f;
		ep=CheckInPolyPrecis(pos->x-off,pos->y-off,pos->z);

		if ((ep) && (ep->room>-1))	
		{
			if (height) *height=ep->center.y;

			return ep->room;
		}

		ep=CheckInPolyPrecis(pos->x-off,pos->y-20,pos->z-off);

		if ((ep) && (ep->room>-1))	
		{
			if (height) *height=ep->center.y;

			return ep->room;
		}

		ep=CheckInPolyPrecis(pos->x-off,pos->y-20,pos->z+off);

		if ((ep) && (ep->room>-1))	
		{
			if (height) *height=ep->center.y;

			return ep->room;
		}

		ep=CheckInPolyPrecis(pos->x+off,pos->y-20,pos->z);

		if ((ep) && (ep->room>-1))	
		{
			if (height) *height=ep->center.y;

			return ep->room;
		}

		ep=CheckInPolyPrecis(pos->x+off,pos->y-20,pos->z+off);

		if ((ep) && (ep->room>-1))	
		{
			if (height) *height=ep->center.y;

			return ep->room;
		}

		ep=CheckInPolyPrecis(pos->x+off,pos->y-20,pos->z-off);

		if ((ep) && (ep->room>-1))	
		{
			if (height) *height=ep->center.y;

			return ep->room;
		}

	}

	return -1;
}
long ARX_PORTALS_GetRoomNumForCamera(float * height)
{
	EERIEPOLY * ep; 
	ep=CheckInPolyPrecis(ACTIVECAM->pos.x,ACTIVECAM->pos.y,ACTIVECAM->pos.z);

	if ((ep) && (ep->room>-1))	
	{
		if (height) *height=ep->center.y;

		return ep->room;
	}

	ep=GetMinPoly(ACTIVECAM->pos.x,ACTIVECAM->pos.y,ACTIVECAM->pos.z);

	if ((ep) && (ep->room>-1))	
	{
		if (height) *height=ep->center.y;

		return ep->room;
	}

	float dist=0.f;

	while (dist<=20.f)
	{		
		float vvv=DEG2RAD(ACTIVECAM->angle.b);
		ep=CheckInPolyPrecis(	ACTIVECAM->pos.x+EEsin(vvv)*dist,
								ACTIVECAM->pos.y,
								ACTIVECAM->pos.z-EEcos(vvv)*dist);

		if ((ep) && (ep->room>-1))	
		{
			if (height) *height=ep->center.y;

			return ep->room;
		}

		dist+=5.f;
	}

	return -1;
}
// flag==1 for player
long ARX_PORTALS_GetRoomNumForPosition(EERIE_3D * pos,long flag)
{
	long num;
	float height;

	if (flag & 1)
		num=ARX_PORTALS_GetRoomNumForCamera(&height);
	else
		num=ARX_PORTALS_GetRoomNumForPosition2(pos,flag,&height);

	if (num > -1)
	{
		long nearest=-1;
		float nearest_dist=99999.f;

		for (long n=0;n<portals->nb_rooms;n++)
		{
			for (long lll=0;lll<portals->room[n].nb_portals;lll++)
			{
				EERIE_PORTALS * po=	&portals->portals[portals->room[n].portals[lll]];
				EERIEPOLY *		epp=&po->poly;

				if (PointIn2DPolyXZ(epp, pos->x, pos->z)) 
				{
					float yy;

					if (GetTruePolyY(epp,pos,&yy))
					{
						if (height>yy)
						{
							if ((yy>=pos->y) && (yy-pos->y<nearest_dist))
							{
								if (epp->norm.y>0)
									nearest=po->room_2;
								else
									nearest=po->room_1;

								nearest_dist=yy-pos->y;						
							}
						}
					}
				}
			}
		}

		if (nearest>-1)
		num=nearest;
	}
	
	return num;
			}
			
void ARX_PORTALS_InitDrawnRooms()
{
	if (!portals) return;

	EERIE_PORTALS *ep = &portals->portals[0];

	for (long i=0;i<portals->nb_total;i++)
	{
		ep->useportal=0;
		*ep++;
	}


	if ((RoomDraw==NULL) || (NbRoomDraw<portals->nb_rooms+1))
	{
		RoomDraw=(PORTAL_ROOM_DRAW *)realloc(RoomDraw,sizeof(PORTAL_ROOM_DRAW)*(portals->nb_rooms+1));

		if (RoomDraw)
		{
			NbRoomDraw=portals->nb_rooms+1;
		}
	}

	if (RoomDraw)
	{
		for (long i=0;i<NbRoomDraw;i++)
		{
			RoomDraw[i].count=0;		
			RoomDraw[i].flags=0;
			RoomDraw[i].frustrum.nb_frustrums=0;
		}
	}

	vPolyWater.clear();
	vPolyLava.clear();
	vPolyVoodooMetal.clear();

	iTotPoly=0;

	if (pDynamicVertexBuffer)
	{
		pDynamicVertexBuffer->Lock(DDLOCK_DISCARDCONTENTS);
		pDynamicVertexBuffer->UnLock();
		pDynamicVertexBuffer->ussNbVertex=0;
	}

	if (pDynamicVertexBufferTransform)
	{
		pDynamicVertexBufferTransform->Lock(DDLOCK_DISCARDCONTENTS);
		pDynamicVertexBufferTransform->UnLock();
		pDynamicVertexBufferTransform->ussNbVertex=0;
	}

	if (pDynamicVertexBuffer_TLVERTEX)
	{
		pDynamicVertexBuffer_TLVERTEX->Lock(DDLOCK_DISCARDCONTENTS);
		pDynamicVertexBuffer_TLVERTEX->UnLock();
		pDynamicVertexBuffer_TLVERTEX->ussNbVertex=0;
	}

	if (pDynamicVertexBufferBump)
	{
		pDynamicVertexBufferBump->Lock(DDLOCK_DISCARDCONTENTS);
		pDynamicVertexBufferBump->UnLock();
		pDynamicVertexBufferBump->ussNbVertex=0;
}

	if (pDynamicVertexBuffer_D3DVERTEX3_T)
	{
		pDynamicVertexBuffer_D3DVERTEX3_T->Lock(DDLOCK_DISCARDCONTENTS);
		pDynamicVertexBuffer_D3DVERTEX3_T->UnLock();
		pDynamicVertexBuffer_D3DVERTEX3_T->ussNbVertex=0;
	}
}
bool BBoxClipPoly(EERIE_2D_BBOX * bbox,EERIEPOLY * ep)
{
	EERIE_2D_BBOX n_bbox;
	long nbv;

	if (ep->type & POLY_QUAD)
		nbv=4;
	else
		nbv=3;

	n_bbox.max.x=n_bbox.min.x=ep->tv[0].sx;
	n_bbox.max.y=n_bbox.min.y=ep->tv[0].sy;	

	for (long i=1;i<nbv;i++)
	{
		n_bbox.min.x=__min(n_bbox.min.x , ep->tv[i].sx);
		n_bbox.min.y=__min(n_bbox.min.y , ep->tv[i].sy);
		n_bbox.max.x=__max(n_bbox.max.x , ep->tv[i].sx);
		n_bbox.max.y=__max(n_bbox.max.y , ep->tv[i].sy);
	}

	if (	bbox->min.x > n_bbox.max.x || n_bbox.min.x > bbox->max.x
		||	bbox->min.y > n_bbox.max.y || n_bbox.min.y > bbox->max.y)
		return true;

	return false;

}
BOOL IsInFrustrum(EERIE_3D * point,EERIE_FRUSTRUM * frustrum)
{
	if (	((point->x*frustrum->plane[0].a + point->y*frustrum->plane[0].b + point->z*frustrum->plane[0].c + frustrum->plane[0].d)>0)
		&&	((point->x*frustrum->plane[1].a + point->y*frustrum->plane[1].b + point->z*frustrum->plane[1].c + frustrum->plane[1].d)>0)
		&&	((point->x*frustrum->plane[2].a + point->y*frustrum->plane[2].b + point->z*frustrum->plane[2].c + frustrum->plane[2].d)>0)
		&&	((point->x*frustrum->plane[3].a + point->y*frustrum->plane[3].b + point->z*frustrum->plane[3].c + frustrum->plane[3].d)>0) )
		return TRUE;

	return FALSE;
}


BOOL IsSphereInFrustrum(float radius,EERIE_3D * point,EERIE_FRUSTRUM * frustrum)
{
	float dists[4];
	dists[0]=point->x*frustrum->plane[0].a + point->y*frustrum->plane[0].b + point->z*frustrum->plane[0].c + frustrum->plane[0].d;
	dists[1]=point->x*frustrum->plane[1].a + point->y*frustrum->plane[1].b + point->z*frustrum->plane[1].c + frustrum->plane[1].d;
	dists[2]=point->x*frustrum->plane[2].a + point->y*frustrum->plane[2].b + point->z*frustrum->plane[2].c + frustrum->plane[2].d;
	dists[3]=point->x*frustrum->plane[3].a + point->y*frustrum->plane[3].b + point->z*frustrum->plane[3].c + frustrum->plane[3].d;

	if (	(dists[0]+radius>0)
		&&	(dists[1]+radius>0)
		&&	(dists[2]+radius>0)
		&&	(dists[3]+radius>0) )
		return TRUE;

	return FALSE;
	
}

bool FrustrumsClipPoly(EERIE_FRUSTRUM_DATA * frustrums,EERIEPOLY * ep)
{
	long nbv;

	if (ep->type & POLY_QUAD)
		nbv=4;
	else
		nbv=3;

 

	for (long i=0;i<frustrums->nb_frustrums;i++)
	{
		if (IsSphereInFrustrum(ep->v[0].rhw,(EERIE_3D *)&ep->center,&frustrums->frustrums[i]))
			return FALSE;
			}

	return TRUE;
}
 
 
void ARX_PORTALS_BlendBBox(long room_num,EERIE_2D_BBOX * bbox)
{
	if (RoomDraw[room_num].count==0)
	{
		RoomDraw[room_num].bbox.min.x=bbox->min.x;
		RoomDraw[room_num].bbox.min.y=bbox->min.y;
		RoomDraw[room_num].bbox.max.x=bbox->max.x;
		RoomDraw[room_num].bbox.max.y=bbox->max.y;		
	}
	else
	{
		RoomDraw[room_num].bbox.min.x=__min(RoomDraw[room_num].bbox.min.x, bbox->min.x);
		RoomDraw[room_num].bbox.min.y=__min(RoomDraw[room_num].bbox.min.y, bbox->min.y);
		RoomDraw[room_num].bbox.max.x=__max(RoomDraw[room_num].bbox.max.x, bbox->max.x);
		RoomDraw[room_num].bbox.max.y=__max(RoomDraw[room_num].bbox.max.y, bbox->max.y);		
	}
}
void Frustrum_Set(EERIE_FRUSTRUM * fr,long plane,float a,float b,float c,float d)
{
	fr->plane[plane].a=a;
	fr->plane[plane].b=b;
	fr->plane[plane].c=c;
	fr->plane[plane].d=d;
}

void CreatePlane(EERIE_FRUSTRUM * frustrum,long numplane,EERIE_3D * orgn,EERIE_3D * pt1,EERIE_3D * pt2)
{
	register float Ax,Ay,Az,Bx,By,Bz,epnlen;
	Ax=pt1->x-orgn->x;
	Ay=pt1->y-orgn->y;
	Az=pt1->z-orgn->z;

	Bx=pt2->x-orgn->x;
	By=pt2->y-orgn->y;
	Bz=pt2->z-orgn->z;

	frustrum->plane[numplane].a=Ay*Bz-Az*By;
	frustrum->plane[numplane].b=Az*Bx-Ax*Bz;
	frustrum->plane[numplane].c=Ax*By-Ay*Bx;

	epnlen = (float)sqrt(	frustrum->plane[numplane].a * frustrum->plane[numplane].a
						+	frustrum->plane[numplane].b * frustrum->plane[numplane].b
						+	frustrum->plane[numplane].c * frustrum->plane[numplane].c	);
	epnlen=1.f/epnlen;
	frustrum->plane[numplane].a*=epnlen;
	frustrum->plane[numplane].b*=epnlen;
	frustrum->plane[numplane].c*=epnlen;
	frustrum->plane[numplane].d=-(	orgn->x * frustrum->plane[numplane].a +
									orgn->y * frustrum->plane[numplane].b +
									orgn->z * frustrum->plane[numplane].c		);

	
}
void CreateScreenFrustrum(EERIE_FRUSTRUM * frustrum);
void CreateFrustrum(EERIE_FRUSTRUM * frustrum,EERIEPOLY * ep,long cull)
{

	long to;

	if (ep->type & POLY_QUAD)
		to=4;
	else to=3;

	if (cull)
	{
		CreatePlane(frustrum,0,&ACTIVECAM->pos,(EERIE_3D *)&ep->v[0],(EERIE_3D *)&ep->v[1]);
		CreatePlane(frustrum,1,&ACTIVECAM->pos,(EERIE_3D *)&ep->v[3],(EERIE_3D *)&ep->v[2]);
		CreatePlane(frustrum,2,&ACTIVECAM->pos,(EERIE_3D *)&ep->v[1],(EERIE_3D *)&ep->v[3]);
		CreatePlane(frustrum,3,&ACTIVECAM->pos,(EERIE_3D *)&ep->v[2],(EERIE_3D *)&ep->v[0]);
	}
	else
	{
		CreatePlane(frustrum,0,&ACTIVECAM->pos,(EERIE_3D *)&ep->v[1],(EERIE_3D *)&ep->v[0]);
		CreatePlane(frustrum,1,&ACTIVECAM->pos,(EERIE_3D *)&ep->v[2],(EERIE_3D *)&ep->v[3]);
		CreatePlane(frustrum,2,&ACTIVECAM->pos,(EERIE_3D *)&ep->v[3],(EERIE_3D *)&ep->v[1]);
		CreatePlane(frustrum,3,&ACTIVECAM->pos,(EERIE_3D *)&ep->v[0],(EERIE_3D *)&ep->v[2]);
	}

	frustrum->nb=to;
}

void CreateScreenFrustrum(EERIE_FRUSTRUM * frustrum)
{

	D3DVECTOR vEyePt    = D3DVECTOR( ACTIVECAM->pos.x , -ACTIVECAM->pos.y, ACTIVECAM->pos.z );

	EERIE_3D target,tout;
	tout.x=0.f;
	tout.y=0.f;
	tout.z=10000.f;
	target.y =- (tout.z*ACTIVECAM->Xsin);
	target.z =-(tout.z*ACTIVECAM->Xcos);
	target.x= (target.z*ACTIVECAM->Ysin);
	target.z=-(target.z*ACTIVECAM->Ycos);
	target.x+=ACTIVECAM->pos.x;
	target.y-=ACTIVECAM->pos.y;
	target.z+=ACTIVECAM->pos.z;

	D3DVECTOR vLookatPt = D3DVECTOR(target.x,target.y,target.z);
	D3DVECTOR vUpVec    = D3DVECTOR( 0.f, 1.f, 0.f );

	// Set the app view matrix for normal viewing
	D3DMATRIX matView,matProj;
	D3DUtil_SetViewMatrix( matView, vEyePt, vLookatPt, vUpVec );
	GDevice->SetTransform( D3DTRANSFORMSTATE_VIEW, &matView );

	GDevice->GetTransform(D3DTRANSFORMSTATE_PROJECTION,&matProj);
	D3DMATRIX matres;
	MatrixMultiply((EERIEMATRIX *)&matres,(EERIEMATRIX *)&matView,(EERIEMATRIX *)&matProj);

	float a,b,c,d,n;
	a=matres._14-matres._11;
	b=matres._24-matres._21;
	c=matres._34-matres._31;
	d=matres._44-matres._41;
 b=-b;
	n = (float)(1.f /sqrt(a*a+b*b+c*c));

	Frustrum_Set(frustrum,0,a*n,b*n,c*n,d*n);
	a=matres._14+matres._11;
	b=matres._24+matres._21;
	c=matres._34+matres._31;
	d=matres._44+matres._41;
 b=-b;
	n = (float)(1.f/sqrt(a*a+b*b+c*c));

	Frustrum_Set(frustrum,1,a*n,b*n,c*n,d*n);
	a=matres._14-matres._12;
	b=matres._24-matres._22;
	c=matres._34-matres._32;
	d=matres._44-matres._42;
 b=-b;
	n = (float)(1.f/sqrt(a*a+b*b+c*c));

	Frustrum_Set(frustrum,2,a*n,b*n,c*n,d*n);
	a=matres._14+matres._12;
	b=matres._24+matres._22;
	c=matres._34+matres._32;
	d=matres._44+matres._42;
 b=-b;
	n = (float)(1.f/sqrt(a*a+b*b+c*c));

	Frustrum_Set(frustrum,3,a*n,b*n,c*n,d*n);
 
	frustrum->nb=4;

	//Ajout du plan Near & Far
	a=matres._14-matres._13;
	b=matres._24-matres._23;
	c=matres._34-matres._33;
	d=matres._44-matres._43;
	b=-b;
 	n = (float)(1.f/sqrt(a*a+b*b+c*c));
	efpPlaneFar.a=a*n;
	efpPlaneFar.b=b*n;
	efpPlaneFar.c=c*n;
	efpPlaneFar.d=d*n;
	
	a=matres._14+matres._13;
	b=matres._24+matres._23;
	c=matres._34+matres._33;
	d=matres._44+matres._43;
 b=-b;
	n = (float)(1.f/sqrt(a*a+b*b+c*c));
	efpPlaneNear.a=a*n;
	efpPlaneNear.b=b*n;
	efpPlaneNear.c=c*n;
	efpPlaneNear.d=d*n;
}

void RoomDrawRelease()
{
	if (RoomDrawList)
		free(RoomDrawList);

	RoomDrawList=NULL;

	if (RoomDraw)
		free(RoomDraw);

	RoomDraw=NULL;
}
void RoomDrawListAdd(long num)
{
	if (TotalRoomDrawList<=NbRoomDrawList)
	{
		RoomDrawList=(long *)realloc(RoomDrawList,sizeof(long)*(NbRoomDrawList+1));
		TotalRoomDrawList=NbRoomDrawList+1;
	}

	RoomDrawList[NbRoomDrawList]=num;	
	NbRoomDrawList++;
}
void RoomFrustrumAdd(long num,EERIE_FRUSTRUM * fr)
{
	if (RoomDraw[num].frustrum.nb_frustrums<MAX_FRUSTRUMS-1)
	{
		memcpy(&RoomDraw[num].frustrum.frustrums
			[RoomDraw[num].frustrum.nb_frustrums],fr,sizeof(EERIE_FRUSTRUM));		
		RoomDraw[num].frustrum.nb_frustrums++;
		
	}	
}
void ARX_PORTALS_RenderRoom(long room_num,EERIE_2D_BBOX * bbox,long prec,long tim);
void ARX_PORTALS_RenderRooms(long prec,long tim)
{
	for (long i=0;i<NbRoomDrawList;i++)
	{
		ARX_PORTALS_RenderRoom(RoomDrawList[i],&RoomDraw[RoomDrawList[i]].bbox,prec,tim);
	}

	NbRoomDrawList=0;
}
void ARX_PORTALS_Frustrum_RenderRoom(long room_num,EERIE_FRUSTRUM_DATA * frustrums,long prec,long tim);
void ARX_PORTALS_Frustrum_RenderRooms(long prec,long tim)
{
	for (long i=0;i<NbRoomDrawList;i++)
	{
		ARX_PORTALS_Frustrum_RenderRoom(RoomDrawList[i],&RoomDraw[RoomDrawList[i]].frustrum,prec,tim);
	}

	NbRoomDrawList=0;
}

void ARX_PORTALS_Frustrum_RenderRoomT(long room_num,EERIE_FRUSTRUM_DATA * frustrums,long prec,long tim);
void ARX_PORTALS_Frustrum_RenderRoomsT(long prec,long tim)
{
	for (long i=0;i<NbRoomDrawList;i++)
	{
		ARX_PORTALS_Frustrum_RenderRoomT(RoomDrawList[i],&RoomDraw[RoomDrawList[i]].frustrum,prec,tim);
	}
}
void ARX_PORTALS_Frustrum_RenderRoom_TransparencyT(long room_num);
void ARX_PORTALS_Frustrum_RenderRoom_TransparencyTSoftCull(long room_num);
void ARX_PORTALS_Frustrum_RenderRooms_TransparencyT()
{
	GDevice->SetRenderState(D3DRENDERSTATE_FOGCOLOR,0);

	GDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,TRUE);

	SETCULL(GDevice,D3DCULL_NONE);
	SETZWRITE(GDevice,FALSE);

	for (long i=0;i<NbRoomDrawList;i++)
	{
		if(USE_PORTALS==4)
		{
			ARX_PORTALS_Frustrum_RenderRoom_TransparencyTSoftCull(RoomDrawList[i]);
		}
		else
		{
			ARX_PORTALS_Frustrum_RenderRoom_TransparencyT(RoomDrawList[i]);
		}
	}

	NbRoomDrawList=0;


	SetZBias(GDevice,8);

	SETZWRITE(GDevice, FALSE);

	//render all fx!!
	SETCULL(GDevice,D3DCULL_CW);

	if(danaeApp.m_pDeviceInfo->wNbTextureSimultaneous>3) danaeApp.m_pDeviceInfo->wNbTextureSimultaneous=3;

	unsigned short iNbIndice = 0;
	int iNb=vPolyWater.size();

	if(iNb)
	{
		int iOldNbVertex=pDynamicVertexBuffer->ussNbVertex;
		pDynamicVertexBuffer->ussNbIndice=0;

		SMY_D3DVERTEX3 *pVertex=(SMY_D3DVERTEX3*)pDynamicVertexBuffer->Lock(DDLOCK_NOOVERWRITE);
		pVertex+=iOldNbVertex;
		
		GDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND,  D3DBLEND_DESTCOLOR );
		GDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE );
		SETTC(GDevice,enviro);

		int iNbTextureSim=danaeApp.m_pDeviceInfo->wNbTextureSimultaneous;

		switch(iNbTextureSim)
		{
		case 3:
			GDevice->SetTexture(2,enviro?enviro->m_pddsSurface?enviro->m_pddsSurface:NULL:NULL);
		case 2:
			GDevice->SetTexture(1,enviro?enviro->m_pddsSurface?enviro->m_pddsSurface:NULL:NULL);
			break;
		}

		unsigned short *pussInd=pDynamicVertexBuffer->pussIndice;

		while(iNb--)
		{
			EERIEPOLY *ep=vPolyWater[iNb];
			
			unsigned short iNbVertex = (ep->type & POLY_QUAD) ? 4 : 3;

			pDynamicVertexBuffer->ussNbVertex		+=	iNbVertex;

			if( pDynamicVertexBuffer->ussNbVertex > pDynamicVertexBuffer->ussMaxVertex )
			{
				pDynamicVertexBuffer->UnLock();
				pDynamicVertexBuffer->ussNbVertex-=iNbVertex;

				if(pDynamicVertexBuffer->ussNbIndice)
				{
					switch(iNbTextureSim)
					{
					case 1:
						GDevice->DrawIndexedPrimitiveVB(	D3DPT_TRIANGLELIST,
							pDynamicVertexBuffer->pVertexBuffer,
							iOldNbVertex,
							pDynamicVertexBuffer->ussNbVertex-iOldNbVertex,
							pDynamicVertexBuffer->pussIndice,
							pDynamicVertexBuffer->ussNbIndice,
							0 );
						
						GDevice->SetTextureStageState(0,D3DTSS_TEXCOORDINDEX,1);
						GDevice->DrawIndexedPrimitiveVB(	D3DPT_TRIANGLELIST,
							pDynamicVertexBuffer->pVertexBuffer,
							iOldNbVertex,
							pDynamicVertexBuffer->ussNbVertex-iOldNbVertex,
							pDynamicVertexBuffer->pussIndice,
							pDynamicVertexBuffer->ussNbIndice,
							0 );
						
						GDevice->SetTextureStageState(0,D3DTSS_TEXCOORDINDEX,2);
						GDevice->DrawIndexedPrimitiveVB(	D3DPT_TRIANGLELIST,
							pDynamicVertexBuffer->pVertexBuffer,
							iOldNbVertex,
							pDynamicVertexBuffer->ussNbVertex-iOldNbVertex,
							pDynamicVertexBuffer->pussIndice,
							pDynamicVertexBuffer->ussNbIndice,
							0 );
						break;
					case 2:
						GDevice->SetTextureStageState(1,D3DTSS_TEXCOORDINDEX,1);
						GDevice->SetTextureStageState(1,D3DTSS_COLORARG1,D3DTA_TEXTURE);
						GDevice->SetTextureStageState(1,D3DTSS_COLORARG2,D3DTA_CURRENT);
						GDevice->SetTextureStageState(1,D3DTSS_COLOROP,D3DTOP_MODULATE4X);
						GDevice->SetTextureStageState(1,D3DTSS_ALPHAOP,D3DTOP_DISABLE);
						GDevice->SetTextureStageState(2,D3DTSS_COLOROP,D3DTOP_DISABLE);
						
						GDevice->DrawIndexedPrimitiveVB(	D3DPT_TRIANGLELIST,
							pDynamicVertexBuffer->pVertexBuffer,
							iOldNbVertex,
							pDynamicVertexBuffer->ussNbVertex-iOldNbVertex,
							pDynamicVertexBuffer->pussIndice,
							pDynamicVertexBuffer->ussNbIndice,
							0 );
						
						GDevice->SetTextureStageState(0,D3DTSS_TEXCOORDINDEX,2);
						GDevice->SetTextureStageState(1,D3DTSS_COLOROP,D3DTOP_DISABLE);
						GDevice->DrawIndexedPrimitiveVB(	D3DPT_TRIANGLELIST,
							pDynamicVertexBuffer->pVertexBuffer,
							iOldNbVertex,
							pDynamicVertexBuffer->ussNbVertex-iOldNbVertex,
							pDynamicVertexBuffer->pussIndice,
							pDynamicVertexBuffer->ussNbIndice,
							0 );
						break;
					case 3:
						GDevice->SetTextureStageState(1,D3DTSS_TEXCOORDINDEX,1);
						GDevice->SetTextureStageState(1,D3DTSS_COLORARG1,D3DTA_TEXTURE);
						GDevice->SetTextureStageState(1,D3DTSS_COLORARG2,D3DTA_CURRENT);
						GDevice->SetTextureStageState(1,D3DTSS_COLOROP,D3DTOP_MODULATE4X);
						GDevice->SetTextureStageState(1,D3DTSS_ALPHAOP,D3DTOP_DISABLE);
						
						GDevice->SetTextureStageState(2,D3DTSS_TEXCOORDINDEX,2);
						GDevice->SetTextureStageState(2,D3DTSS_COLORARG1,D3DTA_TEXTURE);
						GDevice->SetTextureStageState(2,D3DTSS_COLORARG2,D3DTA_CURRENT);
						GDevice->SetTextureStageState(2,D3DTSS_COLOROP,D3DTOP_MODULATE);
						GDevice->SetTextureStageState(2,D3DTSS_ALPHAOP,D3DTOP_DISABLE);
						GDevice->SetTextureStageState(3,D3DTSS_COLOROP,D3DTOP_DISABLE);
						
						GDevice->DrawIndexedPrimitiveVB(	D3DPT_TRIANGLELIST,
							pDynamicVertexBuffer->pVertexBuffer,
							iOldNbVertex,
							pDynamicVertexBuffer->ussNbVertex-iOldNbVertex,
							pDynamicVertexBuffer->pussIndice,
							pDynamicVertexBuffer->ussNbIndice,
							0 );
						break;
					}
					
					GDevice->SetTextureStageState(0,D3DTSS_TEXCOORDINDEX,0);
					GDevice->SetTextureStageState(1,D3DTSS_COLOROP,D3DTOP_DISABLE);
				}

				pVertex=(SMY_D3DVERTEX3*)pDynamicVertexBuffer->Lock(DDLOCK_DISCARDCONTENTS);
				pDynamicVertexBuffer->ussNbVertex=iNbVertex;
				iOldNbVertex = iNbIndice = pDynamicVertexBuffer->ussNbIndice = 0;
				pussInd=pDynamicVertexBuffer->pussIndice;

				//iNbVertex = 3 or 4 but be sure to Assert in Debug if overflow
				ARX_CHECK( pDynamicVertexBuffer->ussNbVertex <= pDynamicVertexBuffer->ussMaxVertex );
			}
			
			pVertex->x=ep->v[0].sx;
			pVertex->y=-ep->v[0].sy;
			pVertex->z=ep->v[0].sz;
			pVertex->color=0xFF505050;
			float fTu=ep->v[0].sx*DIV1000+EEsin((ep->v[0].sx)*DIV200+(float)FrameTime*DIV1000)*DIV32;
			float fTv=ep->v[0].sz*DIV1000+EEcos((ep->v[0].sz)*DIV200+(float)FrameTime*DIV1000)*DIV32;

			if(ep->type&POLY_FALL) fTv+=(float)FrameTime*DIV4000;

			pVertex->tu=fTu;
			pVertex->tv=fTv;
			fTu=(ep->v[0].sx+30.f)*DIV1000+EEsin((ep->v[0].sx+30)*DIV200+(float)FrameTime*DIV1000)*DIV28;
			fTv=(ep->v[0].sz+30.f)*DIV1000-EEcos((ep->v[0].sz+30)*DIV200+(float)FrameTime*DIV1000)*DIV28;

			if (ep->type & POLY_FALL) fTv+=(float)FrameTime*DIV4000;

			pVertex->tu2=fTu;
			pVertex->tv2=fTv;
			fTu=(ep->v[0].sx+60.f)*DIV1000-EEsin((ep->v[0].sx+60)*DIV200+(float)FrameTime*DIV1000)*DIV40;
			fTv=(ep->v[0].sz+60.f)*DIV1000-EEcos((ep->v[0].sz+60)*DIV200+(float)FrameTime*DIV1000)*DIV40;

			if (ep->type & POLY_FALL) fTv+=(float)FrameTime*DIV4000;

			pVertex->tu3=fTu;
			pVertex->tv3=fTv;
			pVertex++;
			pVertex->x=ep->v[1].sx;
			pVertex->y=-ep->v[1].sy;
			pVertex->z=ep->v[1].sz;
			pVertex->color=0xFF505050;
			fTu=ep->v[1].sx*DIV1000+EEsin((ep->v[1].sx)*DIV200+(float)FrameTime*DIV1000)*DIV32;
			fTv=ep->v[1].sz*DIV1000+EEcos((ep->v[1].sz)*DIV200+(float)FrameTime*DIV1000)*DIV32;

			if(ep->type&POLY_FALL) fTv+=(float)FrameTime*DIV4000;

			pVertex->tu=fTu;
			pVertex->tv=fTv;
			fTu=(ep->v[1].sx+30.f)*DIV1000+EEsin((ep->v[1].sx+30)*DIV200+(float)FrameTime*DIV1000)*DIV28;
			fTv=(ep->v[1].sz+30.f)*DIV1000-EEcos((ep->v[1].sz+30)*DIV200+(float)FrameTime*DIV1000)*DIV28;

			if (ep->type & POLY_FALL) fTv+=(float)FrameTime*DIV4000;

			pVertex->tu2=fTu;
			pVertex->tv2=fTv;
			fTu=(ep->v[1].sx+60.f)*DIV1000-EEsin((ep->v[1].sx+60)*DIV200+(float)FrameTime*DIV1000)*DIV40;
			fTv=(ep->v[1].sz+60.f)*DIV1000-EEcos((ep->v[1].sz+60)*DIV200+(float)FrameTime*DIV1000)*DIV40;

			if (ep->type & POLY_FALL) fTv+=(float)FrameTime*DIV4000;

			pVertex->tu3=fTu;
			pVertex->tv3=fTv;
			pVertex++;
			pVertex->x=ep->v[2].sx;
			pVertex->y=-ep->v[2].sy;
			pVertex->z=ep->v[2].sz;
			pVertex->color=0xFF505050;
			fTu=ep->v[2].sx*DIV1000+EEsin((ep->v[2].sx)*DIV200+(float)FrameTime*DIV1000)*DIV32;
			fTv=ep->v[2].sz*DIV1000+EEcos((ep->v[2].sz)*DIV200+(float)FrameTime*DIV1000)*DIV32;

			if(ep->type&POLY_FALL) fTv+=(float)FrameTime*DIV4000;

			pVertex->tu=fTu;
			pVertex->tv=fTv;
			fTu=(ep->v[2].sx+30.f)*DIV1000+EEsin((ep->v[2].sx+30)*DIV200+(float)FrameTime*DIV1000)*DIV28;
			fTv=(ep->v[2].sz+30.f)*DIV1000-EEcos((ep->v[2].sz+30)*DIV200+(float)FrameTime*DIV1000)*DIV28;

			if (ep->type & POLY_FALL) fTv+=(float)FrameTime*DIV4000;

			pVertex->tu2=fTu;
			pVertex->tv2=fTv;
			fTu=(ep->v[2].sx+60.f)*DIV1000-EEsin((ep->v[2].sx+60)*DIV200+(float)FrameTime*DIV1000)*DIV40;
			fTv=(ep->v[2].sz+60.f)*DIV1000-EEcos((ep->v[2].sz+60)*DIV200+(float)FrameTime*DIV1000)*DIV40;

			if (ep->type & POLY_FALL) fTv+=(float)FrameTime*DIV4000;

			pVertex->tu3=fTu;
			pVertex->tv3=fTv;
			pVertex++;

			*pussInd++ = iNbIndice++; 
			*pussInd++ = iNbIndice++; 
			*pussInd++ = iNbIndice++; 
			pDynamicVertexBuffer->ussNbIndice+=3;

			if(iNbVertex&4)
			{
				pVertex->x=ep->v[3].sx;
				pVertex->y=-ep->v[3].sy;
				pVertex->z=ep->v[3].sz;
				pVertex->color=0xFF505050;
				fTu=ep->v[3].sx*DIV1000+EEsin((ep->v[3].sx)*DIV200+(float)FrameTime*DIV1000)*DIV32;
				fTv=ep->v[3].sz*DIV1000+EEcos((ep->v[3].sz)*DIV200+(float)FrameTime*DIV1000)*DIV32;

				if(ep->type&POLY_FALL) fTv+=(float)FrameTime*DIV4000;

				pVertex->tu=fTu;
				pVertex->tv=fTv;
				fTu=(ep->v[3].sx+30.f)*DIV1000+EEsin((ep->v[3].sx+30)*DIV200+(float)FrameTime*DIV1000)*DIV28;
				fTv=(ep->v[3].sz+30.f)*DIV1000-EEcos((ep->v[3].sz+30)*DIV200+(float)FrameTime*DIV1000)*DIV28;

				if (ep->type & POLY_FALL) fTv+=(float)FrameTime*DIV4000;

				pVertex->tu2=fTu;
				pVertex->tv2=fTv;
				fTu=(ep->v[3].sx+60.f)*DIV1000-EEsin((ep->v[3].sx+60)*DIV200+(float)FrameTime*DIV1000)*DIV40;
				fTv=(ep->v[3].sz+60.f)*DIV1000-EEcos((ep->v[3].sz+60)*DIV200+(float)FrameTime*DIV1000)*DIV40;

				if (ep->type & POLY_FALL) fTv+=(float)FrameTime*DIV4000;

				pVertex->tu3=fTu;
				pVertex->tv3=fTv;
				pVertex++;

				*pussInd++ = iNbIndice++; 
				*pussInd++ = iNbIndice - 2; 
				*pussInd++ = iNbIndice - 3; 
				pDynamicVertexBuffer->ussNbIndice+=3;
			}
		}
		
		pDynamicVertexBuffer->UnLock();
		if(pDynamicVertexBuffer->ussNbIndice)
		{
			switch(iNbTextureSim)
			{
			case 1:
				GDevice->DrawIndexedPrimitiveVB(	D3DPT_TRIANGLELIST,
													pDynamicVertexBuffer->pVertexBuffer,
													iOldNbVertex,
													pDynamicVertexBuffer->ussNbVertex-iOldNbVertex,
													pDynamicVertexBuffer->pussIndice,
													pDynamicVertexBuffer->ussNbIndice,
													0 );
			
				GDevice->SetTextureStageState(0,D3DTSS_TEXCOORDINDEX,1);
				GDevice->DrawIndexedPrimitiveVB(	D3DPT_TRIANGLELIST,
													pDynamicVertexBuffer->pVertexBuffer,
													iOldNbVertex,
													pDynamicVertexBuffer->ussNbVertex-iOldNbVertex,
													pDynamicVertexBuffer->pussIndice,
													pDynamicVertexBuffer->ussNbIndice,
													0 );
				
				GDevice->SetTextureStageState(0,D3DTSS_TEXCOORDINDEX,2);
				GDevice->DrawIndexedPrimitiveVB(	D3DPT_TRIANGLELIST,
													pDynamicVertexBuffer->pVertexBuffer,
													iOldNbVertex,
													pDynamicVertexBuffer->ussNbVertex-iOldNbVertex,
													pDynamicVertexBuffer->pussIndice,
													pDynamicVertexBuffer->ussNbIndice,
													0 );
				break;
			case 2:
				GDevice->SetTextureStageState(1,D3DTSS_TEXCOORDINDEX,1);
				GDevice->SetTextureStageState(1,D3DTSS_COLORARG1,D3DTA_TEXTURE);
				GDevice->SetTextureStageState(1,D3DTSS_COLORARG2,D3DTA_CURRENT);
				GDevice->SetTextureStageState(1,D3DTSS_COLOROP,D3DTOP_MODULATE4X);
				GDevice->SetTextureStageState(1,D3DTSS_ALPHAOP,D3DTOP_DISABLE);
				GDevice->SetTextureStageState(2,D3DTSS_COLOROP,D3DTOP_DISABLE);

				GDevice->DrawIndexedPrimitiveVB(	D3DPT_TRIANGLELIST,
													pDynamicVertexBuffer->pVertexBuffer,
													iOldNbVertex,
													pDynamicVertexBuffer->ussNbVertex-iOldNbVertex,
													pDynamicVertexBuffer->pussIndice,
													pDynamicVertexBuffer->ussNbIndice,
													0 );
				
				GDevice->SetTextureStageState(0,D3DTSS_TEXCOORDINDEX,2);
				GDevice->SetTextureStageState(1,D3DTSS_COLOROP,D3DTOP_DISABLE);
				GDevice->DrawIndexedPrimitiveVB(	D3DPT_TRIANGLELIST,
													pDynamicVertexBuffer->pVertexBuffer,
													iOldNbVertex,
													pDynamicVertexBuffer->ussNbVertex-iOldNbVertex,
													pDynamicVertexBuffer->pussIndice,
													pDynamicVertexBuffer->ussNbIndice,
													0 );
				break;
			case 3:
				GDevice->SetTextureStageState(1,D3DTSS_TEXCOORDINDEX,1);
				GDevice->SetTextureStageState(1,D3DTSS_COLORARG1,D3DTA_TEXTURE);
				GDevice->SetTextureStageState(1,D3DTSS_COLORARG2,D3DTA_CURRENT);
				GDevice->SetTextureStageState(1,D3DTSS_COLOROP,D3DTOP_MODULATE4X);
				GDevice->SetTextureStageState(1,D3DTSS_ALPHAOP,D3DTOP_DISABLE);
				
				GDevice->SetTextureStageState(2,D3DTSS_TEXCOORDINDEX,2);
				GDevice->SetTextureStageState(2,D3DTSS_COLORARG1,D3DTA_TEXTURE);
				GDevice->SetTextureStageState(2,D3DTSS_COLORARG2,D3DTA_CURRENT);
				GDevice->SetTextureStageState(2,D3DTSS_COLOROP,D3DTOP_MODULATE);
				GDevice->SetTextureStageState(2,D3DTSS_ALPHAOP,D3DTOP_DISABLE);
				GDevice->SetTextureStageState(3,D3DTSS_COLOROP,D3DTOP_DISABLE);

				GDevice->DrawIndexedPrimitiveVB(	D3DPT_TRIANGLELIST,
													pDynamicVertexBuffer->pVertexBuffer,
													iOldNbVertex,
													pDynamicVertexBuffer->ussNbVertex-iOldNbVertex,
													pDynamicVertexBuffer->pussIndice,
													pDynamicVertexBuffer->ussNbIndice,
													0 );
				break;
			}

			GDevice->SetTextureStageState(1,D3DTSS_COLOROP,D3DTOP_DISABLE);
			GDevice->SetTextureStageState(0,D3DTSS_TEXCOORDINDEX,0);
		}

		vPolyWater.clear();
	}

	iNbIndice=0;
	iNb=vPolyLava.size();

	if(iNb)
	{
		int iOldNbVertex=pDynamicVertexBuffer->ussNbVertex;
		pDynamicVertexBuffer->ussNbIndice=0;

		SMY_D3DVERTEX3 *pVertex=(SMY_D3DVERTEX3*)pDynamicVertexBuffer->Lock(DDLOCK_NOOVERWRITE);
		pVertex+=iOldNbVertex;
		
		GDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND,  D3DBLEND_DESTCOLOR );
		GDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE );
		SETTC(GDevice,enviro);

		switch(danaeApp.m_pDeviceInfo->wNbTextureSimultaneous)
		{
		case 3:
			GDevice->SetTexture(2,enviro?enviro->m_pddsSurface?enviro->m_pddsSurface:NULL:NULL);
		case 2:
			GDevice->SetTexture(1,enviro?enviro->m_pddsSurface?enviro->m_pddsSurface:NULL:NULL);
			break;
		}

		unsigned short *pussInd=pDynamicVertexBuffer->pussIndice;

		while(iNb--)
		{
			EERIEPOLY *ep=vPolyLava[iNb];
			
			unsigned short iNbVertex = (ep->type & POLY_QUAD) ? 4 : 3;

			pDynamicVertexBuffer->ussNbVertex+=iNbVertex;

			if(pDynamicVertexBuffer->ussNbVertex>pDynamicVertexBuffer->ussMaxVertex)
			{
				pDynamicVertexBuffer->UnLock();
				pDynamicVertexBuffer->ussNbVertex-=iNbVertex;

				GDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND,  D3DBLEND_DESTCOLOR );
				GDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE );
				GDevice->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_MODULATE2X);

				if(pDynamicVertexBuffer->ussNbIndice)
				{
					switch(danaeApp.m_pDeviceInfo->wNbTextureSimultaneous)
					{
					case 1:
						GDevice->DrawIndexedPrimitiveVB(	D3DPT_TRIANGLELIST,
							pDynamicVertexBuffer->pVertexBuffer,
							iOldNbVertex,
							pDynamicVertexBuffer->ussNbVertex-iOldNbVertex,
							pDynamicVertexBuffer->pussIndice,
							pDynamicVertexBuffer->ussNbIndice,
							0 );
						
						GDevice->SetTextureStageState(0,D3DTSS_TEXCOORDINDEX,1);
						GDevice->DrawIndexedPrimitiveVB(	D3DPT_TRIANGLELIST,
							pDynamicVertexBuffer->pVertexBuffer,
							iOldNbVertex,
							pDynamicVertexBuffer->ussNbVertex-iOldNbVertex,
							pDynamicVertexBuffer->pussIndice,
							pDynamicVertexBuffer->ussNbIndice,
							0 );
						
						GDevice->SetTextureStageState(0,D3DTSS_TEXCOORDINDEX,2);
						GDevice->DrawIndexedPrimitiveVB(	D3DPT_TRIANGLELIST,
							pDynamicVertexBuffer->pVertexBuffer,
							iOldNbVertex,
							pDynamicVertexBuffer->ussNbVertex-iOldNbVertex,
							pDynamicVertexBuffer->pussIndice,
							pDynamicVertexBuffer->ussNbIndice,
							0 );
						
						GDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ZERO );
						GDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCCOLOR);
						GDevice->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_MODULATE);
						GDevice->DrawIndexedPrimitiveVB(	D3DPT_TRIANGLELIST,
							pDynamicVertexBuffer->pVertexBuffer,
							iOldNbVertex,
							pDynamicVertexBuffer->ussNbVertex-iOldNbVertex,
							pDynamicVertexBuffer->pussIndice,
							pDynamicVertexBuffer->ussNbIndice,
							0 );
						break;
					case 2:
						GDevice->SetTextureStageState(1,D3DTSS_TEXCOORDINDEX,1);
						GDevice->SetTextureStageState(1,D3DTSS_COLORARG1,D3DTA_TEXTURE);
						GDevice->SetTextureStageState(1,D3DTSS_COLORARG2,D3DTA_CURRENT);
						GDevice->SetTextureStageState(1,D3DTSS_COLOROP,D3DTOP_MODULATE4X);
						GDevice->SetTextureStageState(1,D3DTSS_ALPHAOP,D3DTOP_DISABLE);
						GDevice->SetTextureStageState(2,D3DTSS_COLOROP,D3DTOP_DISABLE);
						
						GDevice->DrawIndexedPrimitiveVB(	D3DPT_TRIANGLELIST,
							pDynamicVertexBuffer->pVertexBuffer,
							iOldNbVertex,
							pDynamicVertexBuffer->ussNbVertex-iOldNbVertex,
							pDynamicVertexBuffer->pussIndice,
							pDynamicVertexBuffer->ussNbIndice,
							0 );
						
						GDevice->SetTextureStageState(0,D3DTSS_TEXCOORDINDEX,2);
						GDevice->SetTextureStageState(1,D3DTSS_COLOROP,D3DTOP_DISABLE);
						GDevice->DrawIndexedPrimitiveVB(	D3DPT_TRIANGLELIST,
							pDynamicVertexBuffer->pVertexBuffer,
							iOldNbVertex,
							pDynamicVertexBuffer->ussNbVertex-iOldNbVertex,
							pDynamicVertexBuffer->pussIndice,
							pDynamicVertexBuffer->ussNbIndice,
							0 );
						
						GDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ZERO );
						GDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCCOLOR);
						GDevice->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_MODULATE);
						GDevice->DrawIndexedPrimitiveVB(	D3DPT_TRIANGLELIST,
							pDynamicVertexBuffer->pVertexBuffer,
							iOldNbVertex,
							pDynamicVertexBuffer->ussNbVertex-iOldNbVertex,
							pDynamicVertexBuffer->pussIndice,
							pDynamicVertexBuffer->ussNbIndice,
							0 );
						break;
					case 3:
						GDevice->SetTextureStageState(1,D3DTSS_TEXCOORDINDEX,1);
						GDevice->SetTextureStageState(1,D3DTSS_COLORARG1,D3DTA_TEXTURE);
						GDevice->SetTextureStageState(1,D3DTSS_COLORARG2,D3DTA_CURRENT);
						GDevice->SetTextureStageState(1,D3DTSS_COLOROP,D3DTOP_MODULATE4X);
						GDevice->SetTextureStageState(1,D3DTSS_ALPHAOP,D3DTOP_DISABLE);
						
						GDevice->SetTextureStageState(2,D3DTSS_TEXCOORDINDEX,2);
						GDevice->SetTextureStageState(2,D3DTSS_COLORARG1,D3DTA_TEXTURE);
						GDevice->SetTextureStageState(2,D3DTSS_COLORARG2,D3DTA_CURRENT);
						GDevice->SetTextureStageState(2,D3DTSS_COLOROP,D3DTOP_MODULATE);
						GDevice->SetTextureStageState(2,D3DTSS_ALPHAOP,D3DTOP_DISABLE);
						GDevice->SetTextureStageState(3,D3DTSS_COLOROP,D3DTOP_DISABLE);
						
						GDevice->DrawIndexedPrimitiveVB(	D3DPT_TRIANGLELIST,
							pDynamicVertexBuffer->pVertexBuffer,
							iOldNbVertex,
							pDynamicVertexBuffer->ussNbVertex-iOldNbVertex,
							pDynamicVertexBuffer->pussIndice,
							pDynamicVertexBuffer->ussNbIndice,
							0 );
						
						GDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ZERO );
						GDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCCOLOR);
						GDevice->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_MODULATE);
						GDevice->DrawIndexedPrimitiveVB(	D3DPT_TRIANGLELIST,
							pDynamicVertexBuffer->pVertexBuffer,
							iOldNbVertex,
							pDynamicVertexBuffer->ussNbVertex-iOldNbVertex,
							pDynamicVertexBuffer->pussIndice,
							pDynamicVertexBuffer->ussNbIndice,
							0 );
						break;
					}

					GDevice->SetTextureStageState(1,D3DTSS_COLOROP,D3DTOP_DISABLE);
					GDevice->SetTextureStageState(0,D3DTSS_TEXCOORDINDEX,0);
				}

				pVertex=(SMY_D3DVERTEX3*)pDynamicVertexBuffer->Lock(DDLOCK_DISCARDCONTENTS);
				pDynamicVertexBuffer->ussNbVertex=iNbVertex;
				iOldNbVertex = iNbIndice = pDynamicVertexBuffer->ussNbIndice = 0;
				pussInd=pDynamicVertexBuffer->pussIndice;

				//iNbVertex = 3 or 4 but be sure to Assert in Debug if overflow
				ARX_CHECK( pDynamicVertexBuffer->ussNbVertex <= pDynamicVertexBuffer->ussMaxVertex );
			}
			
			pVertex->x=ep->v[0].sx;
			pVertex->y=-ep->v[0].sy;
			pVertex->z=ep->v[0].sz;
			pVertex->color=0xFF666666;
			float fTu=ep->v[0].sx*DIV1000+EEsin((ep->v[0].sx)*DIV200+(float)FrameTime*DIV2000)*DIV20;
			float fTv=ep->v[0].sz*DIV1000+EEcos((ep->v[0].sz)*DIV200+(float)FrameTime*DIV2000)*DIV20;
			pVertex->tu=fTu;
			pVertex->tv=fTv;
			fTu=ep->v[0].sx*DIV1000+EEsin((ep->v[0].sx)*DIV100+(float)FrameTime*DIV2000)*DIV10;
			fTv=ep->v[0].sz*DIV1000+EEcos((ep->v[0].sz)*DIV100+(float)FrameTime*DIV2000)*DIV10;
			pVertex->tu2=fTu;
			pVertex->tv2=fTv;
			fTu=ep->v[0].sx*DIV600+EEsin((ep->v[0].sx)*DIV160+(float)FrameTime*DIV2000)*DIV11;
			fTv=ep->v[0].sz*DIV600+EEcos((ep->v[0].sz)*DIV160+(float)FrameTime*DIV2000)*DIV11;

			pVertex->tu3=fTu;
			pVertex->tv3=fTv;
			pVertex++;
			pVertex->x=ep->v[1].sx;
			pVertex->y=-ep->v[1].sy;
			pVertex->z=ep->v[1].sz;
			pVertex->color=0xFF666666;
			fTu=ep->v[1].sx*DIV1000+EEsin((ep->v[1].sx)*DIV200+(float)FrameTime*DIV2000)*DIV20;
			fTv=ep->v[1].sz*DIV1000+EEcos((ep->v[1].sz)*DIV200+(float)FrameTime*DIV2000)*DIV20;
			pVertex->tu=fTu;
			pVertex->tv=fTv;
			fTu=ep->v[1].sx*DIV1000+EEsin((ep->v[1].sx)*DIV100+(float)FrameTime*DIV2000)*DIV10;
			fTv=ep->v[1].sz*DIV1000+EEcos((ep->v[1].sz)*DIV100+(float)FrameTime*DIV2000)*DIV10;
			pVertex->tu2=fTu;
			pVertex->tv2=fTv;
			fTu=ep->v[1].sx*DIV600+EEsin((ep->v[1].sx)*DIV160+(float)FrameTime*DIV2000)*DIV11;
			fTv=ep->v[1].sz*DIV600+EEcos((ep->v[1].sz)*DIV160+(float)FrameTime*DIV2000)*DIV11;

			pVertex->tu3=fTu;
			pVertex->tv3=fTv;
			pVertex++;
			pVertex->x=ep->v[2].sx;
			pVertex->y=-ep->v[2].sy;
			pVertex->z=ep->v[2].sz;
			pVertex->color=0xFF666666;
			fTu=ep->v[2].sx*DIV1000+EEsin((ep->v[2].sx)*DIV200+(float)FrameTime*DIV2000)*DIV20;
			fTv=ep->v[2].sz*DIV1000+EEcos((ep->v[2].sz)*DIV200+(float)FrameTime*DIV2000)*DIV20;
			pVertex->tu=fTu;
			pVertex->tv=fTv;
			fTu=ep->v[2].sx*DIV1000+EEsin((ep->v[2].sx)*DIV100+(float)FrameTime*DIV2000)*DIV10;
			fTv=ep->v[2].sz*DIV1000+EEcos((ep->v[2].sz)*DIV100+(float)FrameTime*DIV2000)*DIV10;
			pVertex->tu2=fTu;
			pVertex->tv2=fTv;
			fTu=ep->v[2].sx*DIV600+EEsin((ep->v[2].sx)*DIV160+(float)FrameTime*DIV2000)*DIV11;
			fTv=ep->v[2].sz*DIV600+EEcos((ep->v[2].sz)*DIV160+(float)FrameTime*DIV2000)*DIV11;
	
			pVertex->tu3=fTu;
			pVertex->tv3=fTv;
			pVertex++;

			*pussInd++ = iNbIndice++; 
			*pussInd++ = iNbIndice++; 
			*pussInd++ = iNbIndice++; 
			pDynamicVertexBuffer->ussNbIndice+=3;

			if(iNbVertex&4)
			{
				pVertex->x=ep->v[3].sx;
				pVertex->y=-ep->v[3].sy;
				pVertex->z=ep->v[3].sz;
				pVertex->color=0xFF666666;
				fTu=ep->v[3].sx*DIV1000+EEsin((ep->v[3].sx)*DIV200+(float)FrameTime*DIV2000)*DIV20;
				fTv=ep->v[3].sz*DIV1000+EEcos((ep->v[3].sz)*DIV200+(float)FrameTime*DIV2000)*DIV20;
				pVertex->tu=fTu;
				pVertex->tv=fTv;
				fTu=ep->v[3].sx*DIV1000+EEsin((ep->v[3].sx)*DIV100+(float)FrameTime*DIV2000)*DIV10;
				fTv=ep->v[3].sz*DIV1000+EEcos((ep->v[3].sz)*DIV100+(float)FrameTime*DIV2000)*DIV10;
				pVertex->tu2=fTu;
				pVertex->tv2=fTv;
				fTu=ep->v[3].sx*DIV600+EEsin((ep->v[3].sx)*DIV160+(float)FrameTime*DIV2000)*DIV11;
				fTv=ep->v[3].sz*DIV600+EEcos((ep->v[3].sz)*DIV160+(float)FrameTime*DIV2000)*DIV11;
		
				pVertex->tu3=fTu;
				pVertex->tv3=fTv;
				pVertex++;

				*pussInd++ = iNbIndice++; 
				*pussInd++ = iNbIndice - 2; 
				*pussInd++ = iNbIndice - 3; 
				pDynamicVertexBuffer->ussNbIndice+=3;
			}
		}
		
		pDynamicVertexBuffer->UnLock();

		if(pDynamicVertexBuffer->ussNbIndice)
		{
			GDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND,  D3DBLEND_DESTCOLOR );
			GDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE );
			GDevice->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_MODULATE2X);

			switch(danaeApp.m_pDeviceInfo->wNbTextureSimultaneous)
			{
			case 1:
				GDevice->DrawIndexedPrimitiveVB(	D3DPT_TRIANGLELIST,
													pDynamicVertexBuffer->pVertexBuffer,
													iOldNbVertex,
													pDynamicVertexBuffer->ussNbVertex-iOldNbVertex,
													pDynamicVertexBuffer->pussIndice,
													pDynamicVertexBuffer->ussNbIndice,
													0 );
			
				GDevice->SetTextureStageState(0,D3DTSS_TEXCOORDINDEX,1);
				GDevice->DrawIndexedPrimitiveVB(	D3DPT_TRIANGLELIST,
													pDynamicVertexBuffer->pVertexBuffer,
													iOldNbVertex,
													pDynamicVertexBuffer->ussNbVertex-iOldNbVertex,
													pDynamicVertexBuffer->pussIndice,
													pDynamicVertexBuffer->ussNbIndice,
													0 );
				
				GDevice->SetTextureStageState(0,D3DTSS_TEXCOORDINDEX,2);
				GDevice->DrawIndexedPrimitiveVB(	D3DPT_TRIANGLELIST,
													pDynamicVertexBuffer->pVertexBuffer,
													iOldNbVertex,
													pDynamicVertexBuffer->ussNbVertex-iOldNbVertex,
													pDynamicVertexBuffer->pussIndice,
													pDynamicVertexBuffer->ussNbIndice,
													0 );

				GDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ZERO );
				GDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCCOLOR);
				GDevice->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_MODULATE);
				GDevice->DrawIndexedPrimitiveVB(	D3DPT_TRIANGLELIST,
													pDynamicVertexBuffer->pVertexBuffer,
													iOldNbVertex,
													pDynamicVertexBuffer->ussNbVertex-iOldNbVertex,
													pDynamicVertexBuffer->pussIndice,
													pDynamicVertexBuffer->ussNbIndice,
													0 );
				break;
			case 2:
				GDevice->SetTextureStageState(1,D3DTSS_TEXCOORDINDEX,1);
				GDevice->SetTextureStageState(1,D3DTSS_COLORARG1,D3DTA_TEXTURE);
				GDevice->SetTextureStageState(1,D3DTSS_COLORARG2,D3DTA_CURRENT);
				GDevice->SetTextureStageState(1,D3DTSS_COLOROP,D3DTOP_MODULATE4X);
				GDevice->SetTextureStageState(1,D3DTSS_ALPHAOP,D3DTOP_DISABLE);
				GDevice->SetTextureStageState(2,D3DTSS_COLOROP,D3DTOP_DISABLE);

				GDevice->DrawIndexedPrimitiveVB(	D3DPT_TRIANGLELIST,
													pDynamicVertexBuffer->pVertexBuffer,
													iOldNbVertex,
													pDynamicVertexBuffer->ussNbVertex-iOldNbVertex,
													pDynamicVertexBuffer->pussIndice,
													pDynamicVertexBuffer->ussNbIndice,
													0 );
				
				GDevice->SetTextureStageState(0,D3DTSS_TEXCOORDINDEX,2);
				GDevice->SetTextureStageState(1,D3DTSS_COLOROP,D3DTOP_DISABLE);
				GDevice->DrawIndexedPrimitiveVB(	D3DPT_TRIANGLELIST,
													pDynamicVertexBuffer->pVertexBuffer,
													iOldNbVertex,
													pDynamicVertexBuffer->ussNbVertex-iOldNbVertex,
													pDynamicVertexBuffer->pussIndice,
													pDynamicVertexBuffer->ussNbIndice,
													0 );

				GDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ZERO );
				GDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCCOLOR);
				GDevice->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_MODULATE);
				GDevice->DrawIndexedPrimitiveVB(	D3DPT_TRIANGLELIST,
													pDynamicVertexBuffer->pVertexBuffer,
													iOldNbVertex,
													pDynamicVertexBuffer->ussNbVertex-iOldNbVertex,
													pDynamicVertexBuffer->pussIndice,
													pDynamicVertexBuffer->ussNbIndice,
													0 );
				break;
			case 3:
				GDevice->SetTextureStageState(1,D3DTSS_TEXCOORDINDEX,1);
				GDevice->SetTextureStageState(1,D3DTSS_COLORARG1,D3DTA_TEXTURE);
				GDevice->SetTextureStageState(1,D3DTSS_COLORARG2,D3DTA_CURRENT);
				GDevice->SetTextureStageState(1,D3DTSS_COLOROP,D3DTOP_MODULATE4X);
				GDevice->SetTextureStageState(1,D3DTSS_ALPHAOP,D3DTOP_DISABLE);
				
				GDevice->SetTextureStageState(2,D3DTSS_TEXCOORDINDEX,2);
				GDevice->SetTextureStageState(2,D3DTSS_COLORARG1,D3DTA_TEXTURE);
				GDevice->SetTextureStageState(2,D3DTSS_COLORARG2,D3DTA_CURRENT);
				GDevice->SetTextureStageState(2,D3DTSS_COLOROP,D3DTOP_MODULATE);
				GDevice->SetTextureStageState(2,D3DTSS_ALPHAOP,D3DTOP_DISABLE);
				GDevice->SetTextureStageState(3,D3DTSS_COLOROP,D3DTOP_DISABLE);

				GDevice->DrawIndexedPrimitiveVB(	D3DPT_TRIANGLELIST,
													pDynamicVertexBuffer->pVertexBuffer,
													iOldNbVertex,
													pDynamicVertexBuffer->ussNbVertex-iOldNbVertex,
													pDynamicVertexBuffer->pussIndice,
													pDynamicVertexBuffer->ussNbIndice,
													0 );

				GDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ZERO );
				GDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCCOLOR);
				GDevice->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_MODULATE);
				GDevice->DrawIndexedPrimitiveVB(	D3DPT_TRIANGLELIST,
													pDynamicVertexBuffer->pVertexBuffer,
													iOldNbVertex,
													pDynamicVertexBuffer->ussNbVertex-iOldNbVertex,
													pDynamicVertexBuffer->pussIndice,
													pDynamicVertexBuffer->ussNbIndice,
													0 );
				break;
			}

			GDevice->SetTextureStageState(1,D3DTSS_COLOROP,D3DTOP_DISABLE);
			GDevice->SetTextureStageState(0,D3DTSS_TEXCOORDINDEX,0);
		}

		vPolyLava.clear();
	}


	SetZBias(GDevice,0);
	GDevice->SetRenderState(D3DRENDERSTATE_FOGCOLOR,ulBKGColor);
	GDevice->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_MODULATE);
	SETALPHABLEND(GDevice,FALSE);
}

void ARX_PORTALS_Frustrum_RenderRoomTCullSoft(long room_num,EERIE_FRUSTRUM_DATA * frustrums,long prec,long tim);
void ARX_PORTALS_Frustrum_RenderRoomsTCullSoft(long prec,long tim)
{
	GDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ZERO );
	GDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCCOLOR );	

	for (long i=0;i<NbRoomDrawList;i++)
	{
		ARX_PORTALS_Frustrum_RenderRoomTCullSoft(RoomDrawList[i],&RoomDraw[RoomDrawList[i]].frustrum,prec,tim);
	}
}
void ARX_PORTALS_RenderRoom(long room_num,EERIE_2D_BBOX * bbox,long prec,long tim)
{
	
	if (RoomDraw[room_num].count)
	{
		EERIEDraw2DRect(GDevice, bbox->min.x,bbox->min.y,bbox->max.x,bbox->max.y,0.0001f, 0xFF0000FF);

	for (long  lll=0;lll<portals->room[room_num].nb_polys;lll++)
	{
			
		FAST_BKG_DATA * feg;
		feg=&ACTIVEBKG->fastdata[portals->room[room_num].epdata[lll].px][portals->room[room_num].epdata[lll].py];

		if (!feg->treat)
			continue;

		EERIEPOLY * ep=&feg->polydata[portals->room[room_num].epdata[lll].idx];

		if (ep->type & (POLY_IGNORE | POLY_NODRAW))		
			continue;
			
			// GO for 3D Backface Culling
			if (ep->type & POLY_DOUBLESIDED)
				SETCULL( GDevice, D3DCULL_NONE );
			else
			{
				EERIE_3D nrm;
				nrm.x=ep->v[2].sx-ACTIVECAM->pos.x;
				nrm.y=ep->v[2].sy-ACTIVECAM->pos.y;
				nrm.z=ep->v[2].sz-ACTIVECAM->pos.z;

				if ( ep->type & POLY_QUAD) 
				{
					if ( (DOTPRODUCT( ep->norm , nrm )>0.f) &&
						 (DOTPRODUCT( ep->norm2 , nrm )>0.f) )	
						continue;
				}
				else if ( DOTPRODUCT( ep->norm , nrm )>0.f)
						continue;

				SETCULL( GDevice, D3DCULL_CW );
			}
			 
			if (!EERIERTPPoly(ep)) // RotTransProject Vertices
				continue; 

			if (BBoxClipPoly(bbox,ep))
				continue;

			long to;

			if ( ep->type & POLY_QUAD) 
			{
				if (FRAME_COUNT<=0) 
					ep->tv[3].color=ep->v[3].color;	

				to=4;
			}
			else to=3;

			if (FRAME_COUNT<=0)
			{
				if(!USE_D3DFOG) ComputeFog(ep->tv,to);							
			}

			if (ep->type & POLY_TRANS) 
			{
				ManageLavaWater(ep,to,tim);
				TransPol[TRANSPOLYSPOS++]=ep;

				if (TRANSPOLYSPOS>=MAX_TRANSPOL) TRANSPOLYSPOS=MAX_TRANSPOL-1;

				if (ViewMode)
				{
					if (ViewMode & VIEWMODE_WIRE) 
						EERIEPOLY_DrawWired(GDevice,ep);
					
					if (ViewMode & VIEWMODE_NORMALS) 
						EERIEPOLY_DrawNormals(GDevice,ep);
				}	

				continue;
			}

			if (!Project.improve)  // Normal View...			
			{
				if (ep->type & POLY_GLOW) 
					ep->tv[0].color=ep->tv[1].color=ep->tv[2].color=ep->tv[3].color=0xFFFFFFFF;
				else 
				{
					if (FRAME_COUNT<=0)
					{
						if (ModeLight & MODE_DYNAMICLIGHT) 	ApplyDynLight(ep);					
						else
						{
							ep->tv[0].color=ep->v[0].color;	
							ep->tv[1].color=ep->v[1].color;	
							ep->tv[2].color=ep->v[2].color;		
						}						
					}
				}

				ManageLavaWater(ep,to,tim);
				
				Delayed_EERIEDRAWPRIM(ep);

				if (ViewMode)
				{
					if (ViewMode & VIEWMODE_WIRE) 
						EERIEPOLY_DrawWired(GDevice,ep);
					
					if (ViewMode & VIEWMODE_NORMALS) 
						EERIEPOLY_DrawNormals(GDevice,ep);
				}	
			}
			else // Improve Vision Activated
			{			
				if (FRAME_COUNT<=0)
				{
					if ( ModeLight & MODE_DYNAMICLIGHT ) 	ApplyDynLight(ep);										
					else 
					{	
						ep->tv[0].color=ep->v[0].color;	
						ep->tv[1].color=ep->v[1].color;	
						ep->tv[2].color=ep->v[2].color;				
					}
				

					for (long k=0;k<to;k++) 
					{
						long lfr,lfb;
						float fr,fb;
						long lr=(ep->tv[k].color>>16) & 255;
						float ffr=(float)(lr);
							
						float dd=(ep->tv[k].sz*prec);

						if (dd>1.f) dd=1.f;

						if (dd<0.f) dd=0.f;
						
						fb=((1.f-dd)*6.f + (EEfabs(ep->nrml[k].x)+EEfabs(ep->nrml[k].y)))*0.125f;
						fr=((0.6f-dd)*6.f + (EEfabs(ep->nrml[k].z)+EEfabs(ep->nrml[k].y)))*0.125f;//(1.f-dd);						

						if (fr<0.f) fr=0.f;
						else fr=__max(ffr,fr*255.f);

						fb*=255.f;
						F2L(fr,&lfr);
						F2L(fb,&lfb);
						F2L(fr,&lfr);
						ep->tv[k].color=( 0xff001E00L | ( (lfr & 255) << 16) | (lfb & 255) );
						//GG component locked at 0x1E
				}
			}

			Delayed_EERIEDRAWPRIM(ep);
		}			
	}			
	}
}
void ARX_PORTALS_Frustrum_RenderRoom(long room_num,EERIE_FRUSTRUM_DATA * frustrums,long prec,long tim)
{
	
	if (RoomDraw[room_num].count)
	{
		
	for (long  lll=0;lll<portals->room[room_num].nb_polys;lll++)
	{
		
		FAST_BKG_DATA * feg;
		feg=&ACTIVEBKG->fastdata[portals->room[room_num].epdata[lll].px][portals->room[room_num].epdata[lll].py];

		if (!feg->treat)
			continue;

		EERIEPOLY * ep=&feg->polydata[portals->room[room_num].epdata[lll].idx];

		if (ep->type & (POLY_IGNORE | POLY_NODRAW))		
			continue;
			
		if (FrustrumsClipPoly(frustrums,ep))
				continue;

			// GO for 3D Backface Culling
			if (ep->type & POLY_DOUBLESIDED)
				SETCULL( GDevice, D3DCULL_NONE );
			else
			{
				EERIE_3D nrm;
				nrm.x=ep->v[2].sx-ACTIVECAM->pos.x;
				nrm.y=ep->v[2].sy-ACTIVECAM->pos.y;
				nrm.z=ep->v[2].sz-ACTIVECAM->pos.z;

				if ( ep->type & POLY_QUAD) 
				{
					if ( (DOTPRODUCT( ep->norm , nrm )>0.f) &&
						 (DOTPRODUCT( ep->norm2 , nrm )>0.f) )	
						continue;
				}
				else if ( DOTPRODUCT( ep->norm , nrm )>0.f)
						continue;

				SETCULL( GDevice, D3DCULL_CW );
			}
			 
			if (!EERIERTPPoly(ep)) // RotTransProject Vertices
				continue; 
			
			long to;

			if ( ep->type & POLY_QUAD) 
			{
				if (FRAME_COUNT<=0) ep->tv[3].color=ep->v[3].color;	

				to=4;
			}
			else to=3;

			if (FRAME_COUNT<=0)
			{
				if(!USE_D3DFOG) ComputeFog(ep->tv,to);							
			}

			if (ep->type & POLY_TRANS) 
			{
				ManageLavaWater(ep,to,tim);
				TransPol[TRANSPOLYSPOS++]=ep;

				if (TRANSPOLYSPOS>=MAX_TRANSPOL) TRANSPOLYSPOS=MAX_TRANSPOL-1;

				if (ViewMode)
				{
					if (ViewMode & VIEWMODE_WIRE) 
						EERIEPOLY_DrawWired(GDevice,ep);
					
					if (ViewMode & VIEWMODE_NORMALS) 
						EERIEPOLY_DrawNormals(GDevice,ep);
				}	

				continue;
			}

			if (!Project.improve)  // Normal View...			
			{
				if (ep->type & POLY_GLOW) ep->tv[0].color=ep->tv[1].color=ep->tv[2].color=ep->tv[3].color=0xFFFFFFFF;
				else 
				{
					if (FRAME_COUNT<=0)
					{
						if (ModeLight & MODE_DYNAMICLIGHT) 	ApplyDynLight(ep);					
						else
						{
							ep->tv[0].color=ep->v[0].color;	
							ep->tv[1].color=ep->v[1].color;	
							ep->tv[2].color=ep->v[2].color;		
						}						
					}
				}

				ManageLavaWater(ep,to,tim);
				
				Delayed_EERIEDRAWPRIM(ep);

				if (ViewMode)
				{
					if (ViewMode & VIEWMODE_WIRE) 
						EERIEPOLY_DrawWired(GDevice,ep);
					
					if (ViewMode & VIEWMODE_NORMALS) 
						EERIEPOLY_DrawNormals(GDevice,ep);
				}	
			}
			else // Improve Vision Activated
			{			
				if (FRAME_COUNT<=0)
				{
					if ( ModeLight & MODE_DYNAMICLIGHT ) 	ApplyDynLight(ep);										
					else 
					{	
						ep->tv[0].color=ep->v[0].color;	
						ep->tv[1].color=ep->v[1].color;	
						ep->tv[2].color=ep->v[2].color;				
					}
				

					for (long k=0;k<to;k++) 
					{
						long lfr,lfb;
						float fr,fb;
						long lr=(ep->tv[k].color>>16) & 255;
						float ffr=(float)(lr);
							
						float dd=(ep->tv[k].sz*prec);

						if (dd>1.f) dd=1.f;

						if (dd<0.f) dd=0.f;
						
						fb=((1.f-dd)*6.f + (EEfabs(ep->nrml[k].x)+EEfabs(ep->nrml[k].y)))*0.125f;
						fr=((0.6f-dd)*6.f + (EEfabs(ep->nrml[k].z)+EEfabs(ep->nrml[k].y)))*0.125f;//(1.f-dd);						

						if (fr<0.f) fr=0.f;
						else fr=__max(ffr,fr*255.f);

						fb*=255.f;
						F2L(fr,&lfr);
						F2L(fb,&lfb);
						F2L(fr,&lfr);
						ep->tv[k].color=( 0xff001E00L | ( (lfr & 255) << 16) | (lfb & 255) );
						//GG component locked at 0x1E
				}
			}

			Delayed_EERIEDRAWPRIM(ep);
		}			
	}			
	}
}

void ApplyDynLight_VertexBuffer(EERIEPOLY *ep,SMY_D3DVERTEX *_pVertex,unsigned short _usInd0,unsigned short _usInd1,unsigned short _usInd2,unsigned short _usInd3);
void ApplyDynLight_VertexBuffer_2(EERIEPOLY *ep,short x,short y,SMY_D3DVERTEX *_pVertex,unsigned short _usInd0,unsigned short _usInd1,unsigned short _usInd2,unsigned short _usInd3);
//-----------------------------------------------------------------------------
void ARX_PORTALS_Frustrum_RenderRoomT(long room_num,EERIE_FRUSTRUM_DATA * frustrums,long prec,long tim)
{

					}
				
//-----------------------------------------------------------------------------
void ARX_PORTALS_Frustrum_RenderRoom_TransparencyT(long room_num)
{

}

TILE_LIGHTS tilelights[MAX_BKGX][MAX_BKGZ];

void InitTileLights()
{
	for (long j=0;j<MAX_BKGZ;j++)
	for (long i=0;i<MAX_BKGZ;i++)
	{
		tilelights[i][j].el=NULL;
		tilelights[i][j].max=0;
		tilelights[i][j].num=0;
	}
}

void ComputeTileLights(short x,short z)
{
	tilelights[x][z].num=0;
	float xx=((float)x+0.5f)*ACTIVEBKG->Xdiv;
	float zz=((float)z+0.5f)*ACTIVEBKG->Zdiv;

	for (long i=0;i<TOTPDL;i++)
	{
		float d2d=Distance2D(xx,zz,PDL[i]->pos.x,PDL[i]->pos.z);

		if (d2d<PDL[i]->fallend+60.f)
		{
			if (tilelights[x][z].num>=tilelights[x][z].max)
			{
				tilelights[x][z].max++;
				tilelights[x][z].el=(EERIE_LIGHT **)realloc(tilelights[x][z].el,sizeof(EERIE_LIGHT *)*(tilelights[x][z].max));				
			}

			tilelights[x][z].el[tilelights[x][z].num]=PDL[i];
			tilelights[x][z].num++;
		}
	}
}

void ClearTileLights()
{
	for (long j=0;j<MAX_BKGZ;j++)
	for (long i=0;i<MAX_BKGZ;i++)
	{
		tilelights[i][j].max=0;
		tilelights[i][j].num=0;

		if (tilelights[i][j].el != NULL)
		{
			free (tilelights[i][j].el);
			tilelights[i][j].el=NULL;
		}
	}
}

//-----------------------------------------------------------------------------

void ARX_PORTALS_Frustrum_RenderRoomTCullSoft(long room_num,EERIE_FRUSTRUM_DATA * frustrums,long prec,long tim)
{
SMY_D3DVERTEX *pMyVertex;
	
	if (RoomDraw[room_num].count)
	{
		bool bNoModulate2X=(pMenuConfig->bForceMetalTwoPass);

		if(!portals->room[room_num].pVertexBuffer)
		{
			char tTxt[256];
			sprintf(tTxt,"portals %d - Zero Polys",room_num);
			MessageBox(	NULL,
						tTxt,
						"Error Portals",
						MB_OK|MB_ICONERROR );
			return;
		}
		
		if( FAILED( portals->room[room_num].pVertexBuffer->Lock(	DDLOCK_WRITEONLY|DDLOCK_NOOVERWRITE/*|DDLOCK_WAIT*/	,
																	(void**)&pMyVertex									,
																	NULL												) ) ) 
		{
			ARX_LOG_ERROR("ARX_PORTALS_Frustrum_RenderRoomTCullSoft Render Error : Cannot Lock Buffer.");
			return;
		}
		
		unsigned short *pIndices=portals->room[room_num].pussIndice;

		FAST_BKG_DATA * feg;
		EERIEPOLY * ep;
		EP_DATA *pEPDATA = &portals->room[room_num].epdata[0];

		float fDistBump=__min(__max(0.f,(ACTIVECAM->cdepth*fZFogStart)-200.f),MAX_DIST_BUMP);

		for (long  lll=0; lll<portals->room[room_num].nb_polys; lll++, *pEPDATA++)
		{

			feg = &ACTIVEBKG->fastdata[pEPDATA->px][pEPDATA->py];

			if (!feg->treat)
			{
				long ix=__max(0,pEPDATA->px-1);
				long ax=__min(ACTIVEBKG->Xsize-1,pEPDATA->px+1);
				long iz=__max(0,pEPDATA->py-1);
				long az=__min(ACTIVEBKG->Zsize-1,pEPDATA->py+1);
				

				ARX_CHECK_SHORT(iz);
				ARX_CHECK_SHORT(ix);
				ARX_CHECK_SHORT(az);
				ARX_CHECK_SHORT(ax);

				for (long nz=iz;nz<=az;nz++)
				for (long nx=ix;nx<=ax;nx++)

				{
					FAST_BKG_DATA * feg2 = &ACTIVEBKG->fastdata[nx][nz];

					if (!feg2->treat)
					{
						feg2->treat=1;

						if (USE_LIGHT_OPTIM) 
								ComputeTileLights(ARX_CLEAN_WARN_CAST_SHORT(nx), ARX_CLEAN_WARN_CAST_SHORT(nz));
					}
				}
			}

			ep=&feg->polydata[pEPDATA->idx];

			if	(!ep->tex)
			{
				continue;
			}

			if (ep->type & (POLY_IGNORE | POLY_NODRAW| POLY_HIDE))
			{
				continue;
			}
			
			if (FrustrumsClipPoly(frustrums,ep))
			{
				continue;
			}

			//Clipp ZNear + Distance pour les ZMapps et Bump!!!
			float fDist=(ep->center.x*efpPlaneNear.a + ep->center.y*efpPlaneNear.b + ep->center.z*efpPlaneNear.c + efpPlaneNear.d);

			if(ep->v[0].rhw<-fDist)
			{
				continue;
			}

			fDist-=ep->v[0].rhw;

			EERIE_3D nrm;
			nrm.x=ep->v[2].sx-ACTIVECAM->pos.x;
			nrm.y=ep->v[2].sy-ACTIVECAM->pos.y;
			nrm.z=ep->v[2].sz-ACTIVECAM->pos.z;

			int to;

			if(ep->type&POLY_QUAD)
			{
				if(	(!(ep->type&POLY_DOUBLESIDED))&&
					(DOTPRODUCT( ep->norm , nrm )>0.f)&&
					(DOTPRODUCT( ep->norm2 , nrm )>0.f) )
				{
					continue;
				}

				to=4;
			}
			else
			{
				if(	(!(ep->type&POLY_DOUBLESIDED))&&
					(DOTPRODUCT( ep->norm , nrm )>0.f) )
				{
					continue;
				}

				to=3;
			}

			unsigned short *pIndicesCurr;
			unsigned long *pNumIndices;

			if(ep->type&POLY_TRANS)
			{
				if(ep->transval>=2.f)  //MULTIPLICATIVE
				{
					pIndicesCurr=pIndices+ep->tex->tMatRoom[room_num].uslStartCull_TMultiplicative+ep->tex->tMatRoom[room_num].uslNbIndiceCull_TMultiplicative;
					pNumIndices=&ep->tex->tMatRoom[room_num].uslNbIndiceCull_TMultiplicative;
				}
				else
				{
					if(ep->transval>=1.f) //ADDITIVE
					{
						pIndicesCurr=pIndices+ep->tex->tMatRoom[room_num].uslStartCull_TAdditive+ep->tex->tMatRoom[room_num].uslNbIndiceCull_TAdditive;
						pNumIndices=&ep->tex->tMatRoom[room_num].uslNbIndiceCull_TAdditive;
					}
					else
					{
						if(ep->transval>0.f) //NORMAL TRANS
						{
							pIndicesCurr=pIndices+ep->tex->tMatRoom[room_num].uslStartCull_TNormalTrans+ep->tex->tMatRoom[room_num].uslNbIndiceCull_TNormalTrans;
							pNumIndices=&ep->tex->tMatRoom[room_num].uslNbIndiceCull_TNormalTrans;
						}
						else
						{
							//SUBTRACTIVE
							pIndicesCurr=pIndices+ep->tex->tMatRoom[room_num].uslStartCull_TSubstractive+ep->tex->tMatRoom[room_num].uslNbIndiceCull_TSubstractive;
							pNumIndices=&ep->tex->tMatRoom[room_num].uslNbIndiceCull_TSubstractive;
						}
					}
				}
			}
			else
			{
				pIndicesCurr=pIndices+ep->tex->tMatRoom[room_num].uslStartCull+ep->tex->tMatRoom[room_num].uslNbIndiceCull;
				pNumIndices=&ep->tex->tMatRoom[room_num].uslNbIndiceCull;
				
				if(ZMAPMODE)
				{
					if((fDist<200)&&(ep->tex->TextureRefinement))
					{
						ep->tex->TextureRefinement->vPolyZMap.push_back(ep);
					}
				}

				if(	(bNoModulate2X)&&
					(ep->tex->userflags&POLY_METAL) )
				{
					vPolyVoodooMetal.push_back(ep);
				}
			}

			SMY_D3DVERTEX *pMyVertexCurr;

				*pIndicesCurr++=ep->uslInd[0];
				*pIndicesCurr++=ep->uslInd[1];
				*pIndicesCurr++=ep->uslInd[2];

				if(to&4)
				{
					*pIndicesCurr++=ep->uslInd[3];
					*pIndicesCurr++=ep->uslInd[2];
					*pIndicesCurr++=ep->uslInd[1];
					*pNumIndices+=6;
				}
				else
				{
					*pNumIndices+=3;
				}

				pMyVertexCurr=&pMyVertex[ep->tex->tMatRoom[room_num].uslStartVertex];
		

			if (!Project.improve)  // Normal View...			
			{
				if(ep->type&POLY_GLOW) 
				{
					pMyVertexCurr[ep->uslInd[0]].color=pMyVertexCurr[ep->uslInd[1]].color=pMyVertexCurr[ep->uslInd[2]].color=0xFFFFFFFF;

					if(to&4)
					{
						pMyVertexCurr[ep->uslInd[3]].color=0xFFFFFFFF;
					}
				}
				else 
				{
					if(ep->type&POLY_LAVA)
					{
						if((FRAME_COUNT<=0)&&(!(ep->type&POLY_TRANS)))
						{
							if(ModeLight & MODE_DYNAMICLIGHT)
							{
								ApplyDynLight(ep);
							}
							else
							{
								ep->tv[0].color=ep->v[0].color;	
								ep->tv[1].color=ep->v[1].color;	
								ep->tv[2].color=ep->v[2].color;		

								if(to&4)
								{
									ep->tv[3].color=ep->v[3].color;
								}
							}
						}

						ManageLava_VertexBuffer(ep,to,tim,pMyVertexCurr);

						vPolyLava.push_back(ep);

						pMyVertexCurr[ep->uslInd[0]].color=ep->tv[0].color;
						pMyVertexCurr[ep->uslInd[1]].color=ep->tv[1].color;
						pMyVertexCurr[ep->uslInd[2]].color=ep->tv[2].color;

						if(to&4)
						{
							pMyVertexCurr[ep->uslInd[3]].color=ep->tv[3].color;
						}
					}
					else
					{
						if((FRAME_COUNT<=0)&&(!(ep->type&POLY_TRANS)))
						{
							if(ModeLight & MODE_DYNAMICLIGHT)
							{
																		
									if (USE_LIGHT_OPTIM)
									ApplyDynLight_VertexBuffer_2(	ep,pEPDATA->px,pEPDATA->py,
																pMyVertexCurr,
																ep->uslInd[0],
																ep->uslInd[1],
																ep->uslInd[2],
																ep->uslInd[3]);
																
									else
									ApplyDynLight_VertexBuffer(	ep,
																pMyVertexCurr,
																ep->uslInd[0],
																ep->uslInd[1],
																ep->uslInd[2],
																ep->uslInd[3]);
								
								}
							else
							{
										pMyVertexCurr[ep->uslInd[0]].color=ep->v[0].color;
										pMyVertexCurr[ep->uslInd[1]].color=ep->v[1].color;
										pMyVertexCurr[ep->uslInd[2]].color=ep->v[2].color;

									if(to&4)
									{
											pMyVertexCurr[ep->uslInd[3]].color=ep->v[3].color;
										}
									}
								}

						if(ep->type&POLY_WATER)
						{
							ManageWater_VertexBuffer(ep,to,tim,pMyVertexCurr);
							vPolyWater.push_back(ep);
						}
					}
				}

				if ((ViewMode & VIEWMODE_WIRE))
				{
					if (EERIERTPPoly(ep))
						EERIEPOLY_DrawWired(GDevice,ep);
				}

					}
			else // Improve Vision Activated
			{			
				//!!!!!!!!! NOT OPTIMIZED T&L !!!!!!!!!!
				if ((FRAME_COUNT<=0)&&(!(ep->type&POLY_TRANS)))
				{
					if (!EERIERTPPoly(ep)) // RotTransProject Vertices
					{
						continue; 
					}

					if ( ModeLight & MODE_DYNAMICLIGHT ) 	ApplyDynLight(ep);										
					else 
					{	
						ep->tv[0].color=ep->v[0].color;	
						ep->tv[1].color=ep->v[1].color;	
						ep->tv[2].color=ep->v[2].color;				

						if(to&4)
						{
							ep->tv[3].color=ep->v[3].color;
						}
					}
				
					for (long k=0;k<to;k++) 
					{
						long lfr,lfb;
						float fr,fb;
						long lr=(ep->tv[k].color>>16) & 255;
						float ffr=(float)(lr);
						
						float dd=(ep->tv[k].rhw*prec);

						if (dd>1.f) dd=1.f;

						if (dd<0.f) dd=0.f;
						
						fb=((1.f-dd)*6.f + (EEfabs(ep->nrml[k].x)+EEfabs(ep->nrml[k].y)))*0.125f;
						fr = ((0.6f - dd) * 6.f + (EEfabs(ep->nrml[k].z) + EEfabs(ep->nrml[k].y))) * 0.125f;

						if (fr<0.f) fr=0.f;
						else fr=__max(ffr,fr*255.f);

						fr=__min(fr,255.f);
						fb*=255.f;
						fb=__min(fb,255.f);
						F2L(fr,&lfr);
						F2L(fb,&lfb);
				
						ep->tv[k].color=( 0xff001E00L | ( (lfr & 255) << 16) | (lfb & 255) );
					
					}

					pMyVertexCurr[ep->uslInd[0]].color=ep->tv[0].color;
					pMyVertexCurr[ep->uslInd[1]].color=ep->tv[1].color;
					pMyVertexCurr[ep->uslInd[2]].color=ep->tv[2].color;

					if(to&4)
					{
						pMyVertexCurr[ep->uslInd[3]].color=ep->tv[3].color;
					}
				}
			}

			if( bALLOW_BUMP )
			{
				if( ( fDist < fDistBump ) && ( ep->tex->m_pddsBumpMap ) )
				{
					ep->tex->vPolyBump.push_back(ep);
				}
			}
		}

		portals->room[room_num].pVertexBuffer->Unlock();
	
		//render opaque
		SETCULL(GDevice,D3DCULL_NONE);
		int iNbTex=portals->room[room_num].usNbTextures;
		TextureContainer **ppTexCurr=portals->room[room_num].ppTextureContainer;

		while(iNbTex--)
		{
			TextureContainer *pTexCurr=*ppTexCurr;

			if (ViewMode & VIEWMODE_FLAT) 
				SETTC(GDevice,NULL);
			else
				SETTC(GDevice,pTexCurr);

			if(	(pTexCurr->userflags&POLY_METAL)&&
				(!bNoModulate2X) )
			{
				GDevice->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_MODULATE2X);
			}
			else
			{
				GDevice->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_MODULATE);
			}
			
			if(pTexCurr->tMatRoom[room_num].uslNbIndiceCull)
			{
				GDevice->DrawIndexedPrimitiveVB(	D3DPT_TRIANGLELIST,
					portals->room[room_num].pVertexBuffer,
					pTexCurr->tMatRoom[room_num].uslStartVertex,
					pTexCurr->tMatRoom[room_num].uslNbVertex,
					&portals->room[room_num].pussIndice[pTexCurr->tMatRoom[room_num].uslStartCull],
					pTexCurr->tMatRoom[room_num].uslNbIndiceCull,
					0 );
				EERIEDrawnPolys+=pTexCurr->tMatRoom[room_num].uslNbIndiceCull;
				pTexCurr->tMatRoom[room_num].uslNbIndiceCull=0;
						}
						
			ppTexCurr++;
		}

		//////////////////////////////
		//ZMapp
		GDevice->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_MODULATE);

		GDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,TRUE);
		GDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,FALSE);

		iNbTex=portals->room[room_num].usNbTextures;
		ppTexCurr=portals->room[room_num].ppTextureContainer;

		if( bSoftRender )
		{
		while(iNbTex--)
		{
			TextureContainer *pTexCurr=*ppTexCurr;
		
			if(	(pTexCurr->TextureRefinement)&&
				(pTexCurr->TextureRefinement->vPolyZMap.size()) )
			{
					SETTC( GDevice, pTexCurr->TextureRefinement );

					SMY_D3DVERTEX3		pVertex[6];
					vector<EERIEPOLY *>::iterator it=	pTexCurr->TextureRefinement->vPolyZMap.begin();

					for ( ; it != pTexCurr->TextureRefinement->vPolyZMap.end() ; ++it )
					{
						EERIEPOLY *		ep			=	*it;
						unsigned short	iNbVertex	= (ep->type & POLY_QUAD) ? 4 : 3;

						//---------------------------------------------------------------------------
						//																	   CALCUL
						float			tu[4];
						float			tv[4];
						float			_fTransp[4];
						unsigned short	nu,	nuu;
						long			nrm			=	0;

						if	(		(EEfabs( ep->nrml[0].y ) >= 0.9f )
								||	(EEfabs( ep->nrml[1].y ) >= 0.9f )
								||	(EEfabs( ep->nrml[2].y ) >= 0.9f ) )
							nrm						=	1;

						for ( nu = 0, nuu = iNbVertex - 1 ; nu < iNbVertex ; nuu = nu++ )
						{
							if ( nrm )
							{
								tu[nu]				=	(ep->v[nu].sx * DIV50);
								tv[nu]				=	(ep->v[nu].sz * DIV50);
							}
							else
							{
								tu[nu]				=	ep->v[nu].tu * 4.f;
								tv[nu]				=	ep->v[nu].tv * 4.f;
							}

							float			t		=	__max( 10, EEDistance3D(&ACTIVECAM->pos, (EERIE_3D *)&ep->v[nu]) - 80.f );

							_fTransp[nu] = (150.f - t) * 0.006666666f;

							if (_fTransp[nu] < 0.f)
								_fTransp[nu]		=	0.f;
							// t cannot be greater than 1.f (b should be negative for that)
						}

						//---------------------------------------------------------------------------
						//																	FILL DATA
						for ( int idx = 0  ; idx < iNbVertex ; ++idx )
						{
							pVertex[idx].x				=	ep->v[idx].sx;
							pVertex[idx].y				=	- ep->v[idx].sy;
							pVertex[idx].z				=	ep->v[idx].sz;
							pVertex[idx].color			=	D3DRGB( _fTransp[idx], _fTransp[idx], _fTransp[idx]);
							pVertex[idx].tu				=	tu[idx]; 
							pVertex[idx].tv				=	tv[idx]; 
						}
						if ( iNbVertex & 4 )
						{
							pVertex[4]					=	pVertex[2];
							pVertex[5]					=	pVertex[1];
						}

						//Draw current prim
						EERIEDRAWPRIM(GDevice, D3DPT_TRIANGLELIST, FVF_D3DVERTEX3, pVertex, (iNbVertex&4)?6:3,  0, EERIE_NOCOUNT );
					}

					//---------------------------------------------------------------------------
					//														   CLEAR CURRENT ZMAP
					pTexCurr->TextureRefinement->vPolyZMap.clear();
				}
				//MAJ POINTER -------------------------------------------------------------------
				ppTexCurr++;
			}//END  while ( iNbTex-- ) ----------------------------------------------------------
		}
		else
		{
			while ( iNbTex-- ) //For each tex in portals->room[room_num]
			{
				TextureContainer * pTexCurr					=	*ppTexCurr;

				if (	( pTexCurr->TextureRefinement ) &&
						( pTexCurr->TextureRefinement->vPolyZMap.size() ) )
				{
					//---------------------------------------------------------------------------
					//																		 INIT
				int iOldNbVertex=pDynamicVertexBuffer->ussNbVertex;
				pDynamicVertexBuffer->ussNbIndice=0;
				
				SMY_D3DVERTEX3 *pVertex=(SMY_D3DVERTEX3*)pDynamicVertexBuffer->Lock(DDLOCK_NOOVERWRITE);
				pVertex+=iOldNbVertex;
				
				SETTC(GDevice,pTexCurr->TextureRefinement);
				
				unsigned short *pussInd=pDynamicVertexBuffer->pussIndice;
				unsigned short iNbIndice = 0;

					vector<EERIEPOLY *>::iterator it		=	pTexCurr->TextureRefinement->vPolyZMap.begin();
				
					//---------------------------------------------------------------------------
					//																		 LOOP
				for (; it != pTexCurr->TextureRefinement->vPolyZMap.end(); ++it)
				{
					EERIEPOLY * ep = *it;
					unsigned short iNbVertex = (ep->type & POLY_QUAD) ? 4 : 3;
					
					pDynamicVertexBuffer->ussNbVertex+=iNbVertex;
					
						//-----------------------------------------------------------------------
						//																	FLUSH
					if(pDynamicVertexBuffer->ussNbVertex>pDynamicVertexBuffer->ussMaxVertex)
					{
						pDynamicVertexBuffer->UnLock();
						pDynamicVertexBuffer->ussNbVertex-=iNbVertex;
						
						if(pDynamicVertexBuffer->ussNbIndice)
						{
							GDevice->DrawIndexedPrimitiveVB(	D3DPT_TRIANGLELIST,
																pDynamicVertexBuffer->pVertexBuffer,
																iOldNbVertex,
																pDynamicVertexBuffer->ussNbVertex-iOldNbVertex,
																pDynamicVertexBuffer->pussIndice,
																pDynamicVertexBuffer->ussNbIndice,
																0 );
						}
						
							//INIT --------------------------------------------------------------
						pVertex=(SMY_D3DVERTEX3*)pDynamicVertexBuffer->Lock(DDLOCK_DISCARDCONTENTS);
						pDynamicVertexBuffer->ussNbVertex=iNbVertex;
						iOldNbVertex = iNbIndice = pDynamicVertexBuffer->ussNbIndice = 0;
						pussInd=pDynamicVertexBuffer->pussIndice;

							//iNbVertex = 3 or 4 but be sure to Assert in Debug if overflow
						ARX_CHECK( pDynamicVertexBuffer->ussNbVertex <= pDynamicVertexBuffer->ussMaxVertex );
					}
					
						//-----------------------------------------------------------------------
						//																PRECALCUL
					float tu[4];
					float tv[4];
					float _fTransp[4];
					unsigned short nu, nuu;
					long nrm=0;

					if	(	(EEfabs(ep->nrml[0].y)>=0.9f)
						||	(EEfabs(ep->nrml[1].y)>=0.9f)
						||	(EEfabs(ep->nrml[2].y)>=0.9f)	)
						nrm=1;

					for (nu=0,nuu=iNbVertex-1;nu<iNbVertex;nuu=nu++)
					{
						if (nrm)
						{
							tu[nu]=(ep->v[nu].sx*DIV50);
							tv[nu]=(ep->v[nu].sz*DIV50);
						}
						else
						{
							tu[nu]=ep->v[nu].tu*4.f;
							tv[nu]=ep->v[nu].tv*4.f;						
						}

							float			t		=	__max( 10, EEDistance3D(&ACTIVECAM->pos, (EERIE_3D *)&ep->v[nu]) - 80.f );
							//if (t < 10.f)	t		=	10.f;

						_fTransp[nu] = (150.f - t) * 0.006666666f;

							if (_fTransp[nu] < 0.f)
								_fTransp[nu]		=	0.f;
							// t cannot be greater than 1.f (b should be negative for that)
					}
					
						//-----------------------------------------------------------------------
						//																FILL DATA
						for ( int idx = 0  ; idx < iNbVertex ; ++idx )
						{
							pVertex->x				=	ep->v[idx].sx;
							pVertex->y				=	- ep->v[idx].sy;
							pVertex->z				=	ep->v[idx].sz;
							pVertex->color			=	D3DRGB( _fTransp[idx], _fTransp[idx], _fTransp[idx]);
							pVertex->tu				=	tu[idx]; 
							pVertex->tv				=	tv[idx]; 
							pVertex++;
				
							*pussInd++				=	iNbIndice++;
							pDynamicVertexBuffer->ussNbIndice++;
						}
					
					if(iNbVertex&4)
					{
						*pussInd++=iNbIndice-2;
						*pussInd++=iNbIndice-3;
							pDynamicVertexBuffer->ussNbIndice	+=	2;//3;
					}
				}

					//---------------------------------------------------------------------------
					//														   CLEAR CURRENT ZMAP
				pTexCurr->TextureRefinement->vPolyZMap.clear();
					pDynamicVertexBuffer->UnLock();

				if(pDynamicVertexBuffer->ussNbIndice)
				{
					GDevice->DrawIndexedPrimitiveVB(	D3DPT_TRIANGLELIST,
						pDynamicVertexBuffer->pVertexBuffer,
						iOldNbVertex,
						pDynamicVertexBuffer->ussNbVertex-iOldNbVertex,
						pDynamicVertexBuffer->pussIndice,
						pDynamicVertexBuffer->ussNbIndice,
						0 );
				}	
			}
				//MAJ POINTER -------------------------------------------------------------------
			ppTexCurr++;
			}//END  while ( iNbTex-- ) ----------------------------------------------------------
		}

		//METAL(Voodoo grand gourou)
		if(vPolyVoodooMetal.size())
		{
			GDevice->SetRenderState(D3DRENDERSTATE_FOGCOLOR,0);

			unsigned long ulMetalColor=0x00101010;
			GDevice->SetTexture(0,NULL);
			
			int iOldNbVertex=pDynamicVertexBuffer->ussNbVertex;
			pDynamicVertexBuffer->ussNbIndice=0;
			
			SMY_D3DVERTEX3 *pVertex=(SMY_D3DVERTEX3*)pDynamicVertexBuffer->Lock(DDLOCK_NOOVERWRITE);
			pVertex+=iOldNbVertex;
			
			unsigned short *pussInd=pDynamicVertexBuffer->pussIndice;
			unsigned short iNbIndice = 0;
			
			GDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,D3DBLEND_ONE);
			GDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND,D3DBLEND_ONE);	
			
			vector<EERIEPOLY*>::iterator iT;

			for(iT=vPolyVoodooMetal.begin();iT<vPolyVoodooMetal.end();iT++)
			{
				EERIEPOLY *pPoly=*iT;
				unsigned short iNbVertex = (pPoly->type & POLY_QUAD) ? 4 : 3;
				
				pDynamicVertexBuffer->ussNbVertex+=iNbVertex;

				if(pDynamicVertexBuffer->ussNbVertex>pDynamicVertexBuffer->ussMaxVertex)
				{
					pDynamicVertexBuffer->UnLock();
					pDynamicVertexBuffer->ussNbVertex-=iNbVertex;
					
					if(pDynamicVertexBuffer->ussNbIndice)
					{
						GDevice->DrawIndexedPrimitiveVB(	D3DPT_TRIANGLELIST,
							pDynamicVertexBuffer->pVertexBuffer,
							iOldNbVertex,
							pDynamicVertexBuffer->ussNbVertex-iOldNbVertex,
							pDynamicVertexBuffer->pussIndice,
							pDynamicVertexBuffer->ussNbIndice,
							0 );
					}
					
					pVertex=(SMY_D3DVERTEX3*)pDynamicVertexBuffer->Lock(DDLOCK_DISCARDCONTENTS);
					pDynamicVertexBuffer->ussNbVertex=iNbVertex;
					iOldNbVertex = iNbIndice = pDynamicVertexBuffer->ussNbIndice = 0;
					pussInd=pDynamicVertexBuffer->pussIndice;

					//iNbVertex = 3 or 4 but be sure to Assert in Debug if overflow
					ARX_CHECK( pDynamicVertexBuffer->ussNbVertex <= pDynamicVertexBuffer->ussMaxVertex );
				}
				
				pVertex->x=pPoly->v[0].sx;
				pVertex->y=-pPoly->v[0].sy;
				pVertex->z=pPoly->v[0].sz;
				pVertex->color=ulMetalColor;
				pVertex++;
				pVertex->x=pPoly->v[1].sx;
				pVertex->y=-pPoly->v[1].sy;
				pVertex->z=pPoly->v[1].sz;
				pVertex->color=ulMetalColor;
				pVertex++;
				pVertex->x=pPoly->v[2].sx;
				pVertex->y=-pPoly->v[2].sy;
				pVertex->z=pPoly->v[2].sz;
				pVertex->color=ulMetalColor;
				pVertex++;
				
				*pussInd++=iNbIndice++;
				*pussInd++=iNbIndice++;
				*pussInd++=iNbIndice++;
				pDynamicVertexBuffer->ussNbIndice+=3;
				
				if(iNbVertex&4)
				{
					pVertex->x=pPoly->v[3].sx;
					pVertex->y=-pPoly->v[3].sy;
					pVertex->z=pPoly->v[3].sz;
					pVertex->color=ulMetalColor;
					pVertex++;
					
					*pussInd++=iNbIndice++;
					*pussInd++=iNbIndice-2;
					*pussInd++=iNbIndice-3;
					pDynamicVertexBuffer->ussNbIndice+=3;
				}
			}

			pDynamicVertexBuffer->UnLock();

			if(pDynamicVertexBuffer->ussNbIndice)
			{
				GDevice->DrawIndexedPrimitiveVB(	
					D3DPT_TRIANGLELIST,
					pDynamicVertexBuffer->pVertexBuffer,
					iOldNbVertex,
					pDynamicVertexBuffer->ussNbVertex-iOldNbVertex,
					pDynamicVertexBuffer->pussIndice,
					pDynamicVertexBuffer->ussNbIndice,
					0 );
			}
			
			GDevice->SetRenderState(D3DRENDERSTATE_FOGCOLOR,ulBKGColor);
			GDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ZERO );
			GDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCCOLOR );	
			vPolyVoodooMetal.clear();
		}

		GDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,TRUE);
		GDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,FALSE);
	}
}

void CalculTriangleBump( const D3DTLVERTEX& v0, const D3DTLVERTEX& v1, const D3DTLVERTEX& v2, float *du, float *dv );

//-----------------------------------------------------------------------------

void ARX_PORTALS_Frustrum_RenderRoom_TransparencyTSoftCull(long room_num)
{
	if (RoomDraw[room_num].count)
	{
		//render transparency
		int iNbTex=portals->room[room_num].usNbTextures;
		TextureContainer **ppTexCurr=portals->room[room_num].ppTextureContainer;

		while(iNbTex--)
		{
			TextureContainer *pTexCurr=*ppTexCurr;

			//BUMP
			if( ( bALLOW_BUMP ) && ( pTexCurr->vPolyBump.size() ) )
			{
					//----------------------------------------------------------------------------------
					//																		Initializing
					CMY_DYNAMIC_VERTEXBUFFER* pDVB	=	pDynamicVertexBufferBump;
					SetZBias( GDevice, 0 );
		
					int				iOldNbVertex	=	pDVB->ussNbVertex;
					pDVB->ussNbIndice				=	0;
					SMY_D3DVERTEX3*	pVertex			=	(SMY_D3DVERTEX3*)pDVB->Lock( DDLOCK_NOOVERWRITE );
					pVertex							+=	iOldNbVertex;
					
					unsigned short *pussInd			=	pDVB->pussIndice;
				unsigned short iNbIndice = 0;
					
					GDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_DESTCOLOR );
					GDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_SRCCOLOR );	
					
					GDevice->SetTexture( 0, pTexCurr->m_pddsBumpMap );
					int iSimultaneousTexture		=	danaeApp.m_pDeviceInfo->wNbTextureSimultaneous;

					switch( iSimultaneousTexture )
					{
					default:
						GDevice->SetTexture( 1, pTexCurr->m_pddsBumpMap );
						GDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 1 );

						if( bGATI8500 && !bSoftRender )
	 					{
	 						GDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE | D3DTA_COMPLEMENT );
							GDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	 					}
	 					else
						{
							GDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
							GDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE | D3DTA_COMPLEMENT );
						}

						GDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
						GDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );
						GDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_ADDSIGNED );
						GDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
						GDevice->SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE );

						break;
					case 1:
						GDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
						GDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
						GDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
						break;
					}
					
					//----------------------------------------------------------------------------------
					//																				Loop
					for( vector<EERIEPOLY*>::iterator iT = pTexCurr->vPolyBump.begin() ; iT < pTexCurr->vPolyBump.end() ; iT++ )
					{
						EERIEPOLY *pPoly			=	*iT;

						if(	!pPoly->tv[0].color &&
							!pPoly->tv[1].color &&
							!pPoly->tv[2].color) continue;

						float fDu,fDv;
						EERIERTPPoly(pPoly);

						CalculTriangleBump( pPoly->tv[0], pPoly->tv[1], pPoly->tv[2], &fDu, &fDv );
						
						fDu							*=	.8f;
						fDv							*=	.8f;
						const unsigned short iNbVertex	=	( pPoly->type & POLY_QUAD )?4:3;
						pDVB->ussNbVertex			+=	iNbVertex;
						
						//----------------------------------------------------------------------------------
						//																			Flushing
						if( pDVB->ussNbVertex > pDVB->ussMaxVertex )
						{
							pDVB->UnLock();
							pDVB->ussNbVertex		-=	iNbVertex;
							
							if( pDVB->ussNbIndice )
							{
							switch (iSimultaneousTexture) 
								{
								case 1:
									GDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
									GDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
									
									GDevice->DrawIndexedPrimitiveVB(	D3DPT_TRIANGLELIST,
																		pDVB->pVertexBuffer,
																		iOldNbVertex,
																		pDVB->ussNbVertex - iOldNbVertex,
																		pDVB->pussIndice,
																		pDVB->ussNbIndice,
																		0 );
									
									GDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 1 );
									GDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE | D3DTA_COMPLEMENT );
								default:
									GDevice->DrawIndexedPrimitiveVB(	D3DPT_TRIANGLELIST,
																		pDVB->pVertexBuffer,
																		iOldNbVertex,
																		pDVB->ussNbVertex-iOldNbVertex,
																		pDVB->pussIndice,
																		pDVB->ussNbIndice,
																		0 );
									break;
								}
							}

							pVertex				=	(SMY_D3DVERTEX3*)pDVB->Lock(DDLOCK_DISCARDCONTENTS);
							pDVB->ussNbVertex	=	iNbVertex;
							iOldNbVertex		=	iNbIndice			=	pDVB->ussNbIndice	=	0;
							pussInd				=	pDVB->pussIndice;

							ARX_CHECK( pDVB->ussNbVertex <= pDVB->ussMaxVertex );
						}
						
						//----------------------------------------------------------------------------------
						//																		  Filling VB
						//Add 3 Vertices
						for( short int idx = 0 ; idx < 3 ; ++idx )
						{
							pVertex->x			=	pPoly->v[idx].sx;
							pVertex->y			=	-pPoly->v[idx].sy;
							pVertex->z			=	pPoly->v[idx].sz;
							pVertex->color		=	ARX_OPAQUE_WHITE;
							pVertex->tu			=	pPoly->v[idx].tu;
							pVertex->tv			=	pPoly->v[idx].tv;
							pVertex->tu2		=	pPoly->v[idx].tu + fDu;
							pVertex->tv2		=	pPoly->v[idx].tv + fDv;
							pVertex++;
						}
						//Add Triangle 0-1-2 in Indices Tab
						*pussInd++				=	iNbIndice++;
						*pussInd++				=	iNbIndice++;
						*pussInd++				=	iNbIndice++;
						pDVB->ussNbIndice		+=	3;
						
						if(iNbVertex&4)
						{
							//Add vertex
							pVertex->x			=	pPoly->v[3].sx;
							pVertex->y			=	-pPoly->v[3].sy;
							pVertex->z			=	pPoly->v[3].sz;
							pVertex->color		=	ARX_OPAQUE_WHITE;//pPoly->tv[3].color;
							pVertex->tu			=	pPoly->v[3].tu;
							pVertex->tv			=	pPoly->v[3].tv;
							pVertex->tu2		=	pPoly->v[3].tu + fDu;
							pVertex->tv2		=	pPoly->v[3].tv + fDv;
							pVertex++;
							
							//Add 2nd triangle 1-2-3
							*pussInd++=iNbIndice++;
							*pussInd++=iNbIndice-2;
							*pussInd++=iNbIndice-3;
							pDVB->ussNbIndice	+=	3;
						}
					}
			
					//----------------------------------------------------------------------------------
					//																			 Drawing
					pDVB->UnLock();
					if( pDVB->ussNbIndice )
					{
						switch( iSimultaneousTexture )
						{
						case 1:
							GDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
							GDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
							
							GDevice->DrawIndexedPrimitiveVB(	
																D3DPT_TRIANGLELIST,
																pDVB->pVertexBuffer,
																iOldNbVertex,
																pDVB->ussNbVertex-iOldNbVertex,
																pDVB->pussIndice,
																pDVB->ussNbIndice,
																0 );
							
							GDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 1 );
							GDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE | D3DTA_COMPLEMENT );
						default:
							GDevice->DrawIndexedPrimitiveVB(
																D3DPT_TRIANGLELIST,
																pDVB->pVertexBuffer,
																iOldNbVertex,
																pDVB->ussNbVertex-iOldNbVertex,
																pDVB->pussIndice,
																pDVB->ussNbIndice,
																0 );
							break;
						}
					}

					//----------------------------------------------------------------------------------
					//																			  Ending
					GDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
					GDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
					GDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
					GDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
					GDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );

				//Flushing vector
				pTexCurr->vPolyBump.clear();

			} // END BUMP

			SETTC(GDevice,pTexCurr);

			//NORMAL TRANS
			if(pTexCurr->tMatRoom[room_num].uslNbIndiceCull_TNormalTrans)
			{
				SetZBias(GDevice,2);
				GDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND,  D3DBLEND_SRCCOLOR);
				GDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND,  D3DBLEND_DESTCOLOR);
			
				GDevice->DrawIndexedPrimitiveVB(	D3DPT_TRIANGLELIST,
													portals->room[room_num].pVertexBuffer,
													pTexCurr->tMatRoom[room_num].uslStartVertex,
													pTexCurr->tMatRoom[room_num].uslNbVertex,
													&portals->room[room_num].pussIndice[pTexCurr->tMatRoom[room_num].uslStartCull_TNormalTrans],
													pTexCurr->tMatRoom[room_num].uslNbIndiceCull_TNormalTrans,
													0 );
				EERIEDrawnPolys+=pTexCurr->tMatRoom[room_num].uslNbIndiceCull_TNormalTrans;
				pTexCurr->tMatRoom[room_num].uslNbIndiceCull_TNormalTrans=0;
			}

			//MULTIPLICATIVE
			if(pTexCurr->tMatRoom[room_num].uslNbIndiceCull_TMultiplicative)
			{
				SetZBias(GDevice,2);
				GDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ONE);
				GDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND,  D3DBLEND_ONE);
				GDevice->DrawIndexedPrimitiveVB(	D3DPT_TRIANGLELIST,
													portals->room[room_num].pVertexBuffer,
													pTexCurr->tMatRoom[room_num].uslStartVertex,
													pTexCurr->tMatRoom[room_num].uslNbVertex,
													&portals->room[room_num].pussIndice[pTexCurr->tMatRoom[room_num].uslStartCull_TMultiplicative],
													pTexCurr->tMatRoom[room_num].uslNbIndiceCull_TMultiplicative,
													0 );
				EERIEDrawnPolys+=pTexCurr->tMatRoom[room_num].uslNbIndiceCull_TMultiplicative;
				pTexCurr->tMatRoom[room_num].uslNbIndiceCull_TMultiplicative=0;
			}

			//ADDITIVE
			if(pTexCurr->tMatRoom[room_num].uslNbIndiceCull_TAdditive)
			{
				SetZBias(GDevice,2);
				GDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ONE);
				GDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND,  D3DBLEND_ONE);
				GDevice->DrawIndexedPrimitiveVB(	D3DPT_TRIANGLELIST,
													portals->room[room_num].pVertexBuffer,
													pTexCurr->tMatRoom[room_num].uslStartVertex,
													pTexCurr->tMatRoom[room_num].uslNbVertex,
													&portals->room[room_num].pussIndice[pTexCurr->tMatRoom[room_num].uslStartCull_TAdditive],
													pTexCurr->tMatRoom[room_num].uslNbIndiceCull_TAdditive,
													0 );
				EERIEDrawnPolys+=pTexCurr->tMatRoom[room_num].uslNbIndiceCull_TAdditive;
				pTexCurr->tMatRoom[room_num].uslNbIndiceCull_TAdditive=0;
			}

			//SUBSTRACTIVE
			if(pTexCurr->tMatRoom[room_num].uslNbIndiceCull_TSubstractive)
			{
				if(danaeApp.m_pFramework->bitdepth==16)
					SetZBias(GDevice,1);
				else
					SetZBias(GDevice,8);

				GDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ZERO);
				GDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND,  D3DBLEND_INVSRCCOLOR);	
				GDevice->DrawIndexedPrimitiveVB(	D3DPT_TRIANGLELIST,
													portals->room[room_num].pVertexBuffer,
													pTexCurr->tMatRoom[room_num].uslStartVertex,
													pTexCurr->tMatRoom[room_num].uslNbVertex,
													&portals->room[room_num].pussIndice[pTexCurr->tMatRoom[room_num].uslStartCull_TSubstractive],
													pTexCurr->tMatRoom[room_num].uslNbIndiceCull_TSubstractive,
													0 );
				EERIEDrawnPolys+=pTexCurr->tMatRoom[room_num].uslNbIndiceCull_TSubstractive;
				pTexCurr->tMatRoom[room_num].uslNbIndiceCull_TSubstractive=0;
			}

			ppTexCurr++;
		}
	}
}


void ARX_PORTALS_ComputeRoom(long room_num,EERIE_2D_BBOX * bbox,long prec,long tim)
{
	if (portals==NULL) return;

	if (bbox->min.x>=DANAESIZX) return;

	if (bbox->min.y>=DANAESIZY) return;

	if (bbox->max.x<0) return;

	if (bbox->max.y<0) return;

	if (bbox->min.x>=bbox->max.x) return;

	if (bbox->min.y>=bbox->max.y) return;

	if (RoomDraw[room_num].count==0)
		RoomDrawListAdd(room_num);

	ARX_PORTALS_BlendBBox(room_num,bbox);
	RoomDraw[room_num].count++;
	
		// Now Checks For room Portals !!!
	for (long lll=0;lll<portals->room[room_num].nb_portals;lll++)
	{
		if (portals->portals[portals->room[room_num].portals[lll]].useportal) continue;

		EERIE_PORTALS * po=&portals->portals[portals->room[room_num].portals[lll]];
		EERIEPOLY * epp=&po->poly;

	
		float dist1=EEDistance3D(&ACTIVECAM->pos,(EERIE_3D *)&epp->v[0]);
		float dist2=EEDistance3D(&ACTIVECAM->pos,(EERIE_3D *)&epp->v[2]);
		float distc=EEDistance3D(&ACTIVECAM->pos,(EERIE_3D *)&epp->center);
		float threshold=ACTIVECAM->cdepth*fZFogEnd;

		if (	(dist1 > ACTIVECAM->cdepth-threshold)
			&&	(dist2 > ACTIVECAM->cdepth-threshold) 
			&&	(distc > ACTIVECAM->cdepth-threshold) )
		{
			portals->portals[portals->room[room_num].portals[lll]].useportal=2;
			continue;
		}
		
		if (!EERIERTPPoly2(epp)) continue;

		
		int Cull=BackFaceCull2D(epp->tv);

		
		EERIE_2D_BBOX n_bbox;
		n_bbox.max.x=n_bbox.min.x=epp->tv[0].sx;
		n_bbox.max.y=n_bbox.min.y=epp->tv[0].sy;
		long to;

		if (epp->type & POLY_QUAD)
			to=4;
		else
			to=3;

		float minz=epp->tv[0].sz;
		float maxz=epp->tv[0].sz;

		for (long nn=1;nn<to;nn++)
		{
			n_bbox.min.x=__min(n_bbox.min.x , epp->tv[nn].sx);
			n_bbox.min.y=__min(n_bbox.min.y , epp->tv[nn].sy);
			n_bbox.max.x=__max(n_bbox.max.x , epp->tv[nn].sx);
			n_bbox.max.y=__max(n_bbox.max.y , epp->tv[nn].sy);
			minz=__min(minz,epp->tv[0].sz);
			maxz=__max(maxz,epp->tv[0].sz);
		}

		if (minz>0.5f) continue;

		if (	bbox->min.x > n_bbox.max.x || n_bbox.min.x > bbox->max.x
			||	bbox->min.y > n_bbox.max.y || n_bbox.min.y > bbox->max.y)
			continue;

		if (Cull)
			EERIEPOLY_DrawWired(GDevice,epp,0xFFFF0000);
		else
			EERIEPOLY_DrawWired(GDevice,epp,0xFF00FF00);
		
		n_bbox.min.x=__max(n_bbox.min.x,bbox->min.x);
		n_bbox.min.y=__max(n_bbox.min.y,bbox->min.y);
		n_bbox.max.x=__min(n_bbox.max.x,bbox->max.x);
		n_bbox.max.y=__min(n_bbox.max.y,bbox->max.y);
		
		if (po->room_1==room_num)
		{
			if (!Cull)
			{
				portals->portals[portals->room[room_num].portals[lll]].useportal=1;
				ARX_PORTALS_ComputeRoom(po->room_2,&n_bbox,prec,tim);
			}
		}
		else if (po->room_2==room_num)
		{
			if (Cull)
			{
				portals->portals[portals->room[room_num].portals[lll]].useportal=1;
				ARX_PORTALS_ComputeRoom(po->room_1,&n_bbox,prec,tim);
			}			
		}	
	}
}

long ARX_PORTALS_Frustrum_ComputeRoom(long room_num,EERIE_FRUSTRUM * frustrum,long prec,long tim)
{
	long portals_count=0;

	if (portals==NULL) return 0;

	if (RoomDraw[room_num].count==0)
	{
		RoomDrawListAdd(room_num);
	}

	RoomFrustrumAdd(room_num,frustrum);
	RoomDraw[room_num].count++;

	float fClippZFar=ACTIVECAM->cdepth*(fZFogEnd*1.1f);

	// Now Checks For room Portals !!!
	for (long lll=0;lll<portals->room[room_num].nb_portals;lll++)
	{
		if (portals->portals[portals->room[room_num].portals[lll]].useportal) continue;

		EERIE_PORTALS * po=&portals->portals[portals->room[room_num].portals[lll]];
		EERIEPOLY * epp=&po->poly;
	
		//clipp NEAR & FAR
		unsigned char ucVisibilityNear=0;
		unsigned char ucVisibilityFar=0;
		float fDist0=(efpPlaneNear.a*epp->v[0].sx)+(efpPlaneNear.b*epp->v[0].sy)+(efpPlaneNear.c*epp->v[0].sz)+efpPlaneNear.d;

		if(fDist0<0.f) ucVisibilityNear++;
		else
		{
			if(fDist0>fClippZFar) ucVisibilityFar++;
		}

		fDist0=(efpPlaneNear.a*epp->v[1].sx)+(efpPlaneNear.b*epp->v[1].sy)+(efpPlaneNear.c*epp->v[1].sz)+efpPlaneNear.d;

		if(fDist0<0.f) ucVisibilityNear++;
		else
		{
			if(fDist0>fClippZFar) ucVisibilityFar++;
		}

		fDist0=(efpPlaneNear.a*epp->v[2].sx)+(efpPlaneNear.b*epp->v[2].sy)+(efpPlaneNear.c*epp->v[2].sz)+efpPlaneNear.d;

		if(fDist0<0.f) ucVisibilityNear++;
		else
		{
			if(fDist0>fClippZFar) ucVisibilityFar++;
		}

		fDist0=(efpPlaneNear.a*epp->v[3].sx)+(efpPlaneNear.b*epp->v[3].sy)+(efpPlaneNear.c*epp->v[3].sz)+efpPlaneNear.d;

		if(fDist0<0.f) ucVisibilityNear++;
		else
		{
			if(fDist0>fClippZFar) ucVisibilityFar++;
		}

		if(	(ucVisibilityFar&4)||(ucVisibilityNear&4) )
		{
			portals->portals[portals->room[room_num].portals[lll]].useportal=2;
			continue;
		}

		EERIE_3D pos;
		pos.x=epp->center.x-ACTIVECAM->pos.x;
		pos.y=epp->center.y-ACTIVECAM->pos.y;
		pos.z=epp->center.z-ACTIVECAM->pos.z;
		float fRes = Vector_DotProduct(&pos, &epp->norm);
		long to;

		if (epp->type & POLY_QUAD)
			to=4;
		else
			to=3;

		long ret=1;

		if(IsSphereInFrustrum(epp->v[0].rhw,&epp->center,frustrum))
		{
			ret=0;
		}

		if (ret) 
		{
			EERIERTPPoly2(epp);

			if (NEED_TEST_TEXT)
				EERIEPOLY_DrawWired(GDevice,epp,0xFFFF00FF);

			continue;
		}

		portals_count++;

		EERIERTPPoly2(epp);

		int Cull;

		if (fRes<0.f) Cull=0;
		else Cull=1;

		
		EERIE_FRUSTRUM fd;
		CreateFrustrum(&fd,epp,Cull);
		
		if (NEED_TEST_TEXT)
		{
			if (Cull)
				EERIEPOLY_DrawWired(GDevice,epp,0xFFFF0000);
			else
				EERIEPOLY_DrawWired(GDevice,epp,0xFF00FF00);
		}
		

		if (po->room_1==room_num)
		{
			if (!Cull)
			{
				portals->portals[portals->room[room_num].portals[lll]].useportal=1;
				ARX_PORTALS_Frustrum_ComputeRoom(po->room_2,&fd,prec,tim);
			}
		}
		else if (po->room_2==room_num)
		{
			if (Cull)
			{
				portals->portals[portals->room[room_num].portals[lll]].useportal=1;
				ARX_PORTALS_Frustrum_ComputeRoom(po->room_1,&fd,prec,tim);
			}			
		}	
	}

	return portals_count; 
			}

		
BOOL Clip_Visible(const EERIE_3D * orgn, EERIE_3D * dest)
{
	register float dx,dy,dz,adx,ady,adz,ix,iy,iz;
	register float x0,y0,z0;
	register float forr,temp;
 
	dx=(dest->x-orgn->x);
	adx=EEfabs(dx);
	dy=(dest->y-orgn->y);
	ady=EEfabs(dy);
	dz=(dest->z-orgn->z);
	adz=EEfabs(dz);

	x0=orgn->x;
	y0=orgn->y;
	z0=orgn->z;

	if ( (adx>=ady) && (adx>=adz)) 
	{
		if (adx != dx)
		{
			ix = -1.f * PASS;
		}
		else
		{
			ix = 1.f * PASS;
		}

		forr=adx;
		temp=1.f/(adx/PASS);
		iy=dy*temp;
		iz=dz*temp;
	}
	else if ( (ady>=adx) && (ady>=adz)) 
	{
		if (ady != dy)
		{
			iy = -1.f * PASS;
		}
		else
		{
			iy = 1.f * PASS;
		}

		forr=ady;
		temp=1.f/(ady/PASS);
		ix=dx*temp;
		iz=dz*temp;
	}
	else 
	{
		if (adz != dz)
		{
			iz = -1.f * PASS;
		}
		else
		{
			iz = 1.f * PASS;
		}

		forr=adz;
		temp=1.f/(adz/PASS);
		ix=dx*temp;
		iy=dy*temp;
	}
	

long curpixel;
	long tot;
	tot=0;
	
	long x,y;
	FAST_BKG_DATA * LAST_eg=NULL;
	curpixel=2;
	x0+=ix*2;
		y0+=iy*2;
		z0+=iz*2;
		forr-=PASS*2;		

	while (forr>PASSS)
	{
			FAST_BKG_DATA * feg;
			F2L(x0*ACTIVEBKG->Xmul,&x);
			F2L(z0*ACTIVEBKG->Zmul,&y);
			feg=&ACTIVEBKG->fastdata[x][y];

		if (feg!=LAST_eg)
		{
				
			LAST_eg=feg;
			
			if (feg->nothing)	tot += 2; 
		
			if (tot>MAX_OUT) return FALSE;
		}

		float v=(float)curpixel*DIV5;

		if (v<1.f) v=1.f;

		x0+=ix*v;
		y0+=iy*v;
		z0+=iz*v;
		forr-=PASS*v;		
	}

	return TRUE;//hard;
}



BOOL spGetTruePolyY(const EERIEPOLY * ep, const EERIE_3D * pos, float * ret)
	{
		
	register EERIE_3D n,s21,s31;

	s21.x=ep->v[1].sx-ep->v[0].sx;
	s21.y=ep->v[1].sy-ep->v[0].sy;
	s21.z=ep->v[1].sz-ep->v[0].sz;
	s31.x=ep->v[2].sx-ep->v[0].sx;
	s31.y=ep->v[2].sy-ep->v[0].sy;
	s31.z=ep->v[2].sz-ep->v[0].sz;

	n.y=(s21.z*s31.x)-(s21.x*s31.z);
	n.x=(s21.y*s31.z)-(s21.z*s31.y);
	n.z=(s21.x*s31.y)-(s21.y*s31.x);

	// uses s21.x instead of d
	s21.x=ep->v[0].sx*n.x+ep->v[0].sy*n.y+ep->v[0].sz*n.z;

	s21.x=(s21.x-(n.x*pos->x)-(n.z*pos->z))/n.y;
	*ret=s21.x;
	return TRUE;

}

extern long SPECIAL_DRAGINTER_RENDER;
long MAX_FRAME_COUNT=0;
//*************************************************************************************
// Main Background Rendering Proc.
// ie: Big Mess
//*************************************************************************************
///////////////////////////////////////////////////////////
void ARX_SCENE_Render(LPDIRECT3DDEVICE7 pd3dDevice, long flag, long param) 
{
	FrameCount++;

	FRAME_COUNT++;

	if (FRAME_COUNT>MAX_FRAME_COUNT) FRAME_COUNT=0;

	if (!TESTMODE) FRAME_COUNT=0;

	if (EDITMODE) FRAME_COUNT=0;

	if ((player.Interface & INTER_MAP ) &&  (!(player.Interface & INTER_COMBATMODE))) 
		FRAME_COUNT=0;

	if (D3DTRANSFORM) 
	{
		ARX_HWTransform_Render();
		return;
	}
		
	static EERIE_3D lastpos;
	static EERIE_3D lastangle;
	static float lastfocal;
	long to;
	static long x0=0;
	static long x1=0;
	static long z0=0;
	static long z1=0;
	long i,j,k;
	float xx,yy;
	float fr, fb, ffr; 
	EERIEPOLY * ep;
	FAST_BKG_DATA * feg;
 
 
 
	long temp;
	float dd;
	long lr;
	unsigned long tim = ARXTimeUL();
	
	WATEREFFECT+=0.0005f*_framedelay;

	// First if scene camera hasn't moved we set MODIF to 0
	// This allows us not to Clip/Rotate/Translate/Project again the scene
	BOOL MODIF=TRUE;

	if (flag == 3)
	{
		lastpos.x = 0.f;
		return;
	}

	if (	(lastpos.x == ACTIVECAM->pos.x) &&
			(lastpos.y == ACTIVECAM->pos.y) &&
			(lastpos.z == ACTIVECAM->pos.z) &&
			(lastangle.a == ACTIVECAM->angle.a) &&
			(lastangle.b == ACTIVECAM->angle.b) &&
			(lastangle.g == ACTIVECAM->angle.g) &&
			(lastfocal == ACTIVECAM->focal ) ) MODIF=0;
	else 
	{
		lastpos.x = ACTIVECAM->pos.x;
		lastpos.y = ACTIVECAM->pos.y;
		lastpos.z = ACTIVECAM->pos.z;
		lastangle.a = ACTIVECAM->angle.a;
		lastangle.b = ACTIVECAM->angle.b;
		lastangle.g = ACTIVECAM->angle.g;
		lastfocal = ACTIVECAM->focal;
		MODIF=1;
	}
    
	// If LightThread is running we suspend it to avoid too much performance
	// decrease and eventual interference 
	if (LIGHTTHREAD) 
		SuspendThread(LIGHTTHREAD);
	
	float cval=(float)ACTIVECAM->clip3D+4;
	long lcval;
	F2L(cval,&lcval);

	{

		PrepareActiveCamera();
		xx=(float)(ACTIVECAM->pos.x*ACTIVEBKG->Xmul);
		yy=(float)(ACTIVECAM->pos.z*ACTIVEBKG->Zmul);
		
		F2L(xx,&ACTIVECAM->Xsnap);
		F2L(yy,&ACTIVECAM->Zsnap);
		FORCERANGE(ACTIVECAM->Xsnap,0,ACTIVEBKG->Xsize-1);
		FORCERANGE(ACTIVECAM->Zsnap,0,ACTIVEBKG->Zsize-1);
		
		x0=ACTIVECAM->Xsnap-lcval;
		x1=ACTIVECAM->Xsnap+lcval;
		z0=ACTIVECAM->Zsnap-lcval;
		z1=ACTIVECAM->Zsnap+lcval;
		FORCERANGE(x0,0,ACTIVEBKG->Xsize-1);
		FORCERANGE(x1,0,ACTIVEBKG->Xsize-1);
		FORCERANGE(z0,0,ACTIVEBKG->Zsize-2);
		FORCERANGE(z1,0,ACTIVEBKG->Xsize-2);
			
			
		ACTIVEBKG->Backg[ACTIVECAM->Xsnap+ACTIVECAM->Zsnap * ACTIVEBKG->Xsize].treat = 1;
		float prec = 1.f / (ACTIVECAM->cdepth * ACTIVECAM->Zmul);

			

	long lll;
	#ifdef USE_RTS
	{
		ResetWorlds();
		CreatePWorld(x0,x1,z0,z1);
		ComputeSworld();
	}
	#endif

	DRAWLATER_ReInit();
	
	// Temporary Hack...
	long LAST_FC=FRAME_COUNT;
	FRAME_COUNT=0;

	if ((FRAME_COUNT<=0) && (ModeLight & MODE_DYNAMICLIGHT)) PrecalcDynamicLighting(x0,z0,x1,z1);

	float temp0=DEG2RAD(ACTIVECAM->angle.b);
	ACTIVECAM->norm.x=-(float)EEsin(temp0);
	ACTIVECAM->norm.y= (float)EEsin(DEG2RAD(ACTIVECAM->angle.a));
	ACTIVECAM->norm.z= (float)EEcos(temp0);
	float dddd=1.f/EEsqrt(ACTIVECAM->norm.x*ACTIVECAM->norm.x+ACTIVECAM->norm.y*ACTIVECAM->norm.y+ACTIVECAM->norm.z*ACTIVECAM->norm.z);
	ACTIVECAM->norm.x*=dddd;
	ACTIVECAM->norm.y*=dddd;
	ACTIVECAM->norm.z*=dddd;
	// Go for a growing-square-spirallike-render around the camera position
	// (To maximize Z-Buffer efficiency)
	temp0=0;
	EERIE_3D nrm;

	long lfr,lfb;
	long zsnap=ACTIVECAM->Zsnap;
	zsnap=__min(zsnap,ACTIVEBKG->Zsize-1);
	zsnap=__max(zsnap,1);
	long xsnap=ACTIVECAM->Xsnap;
	xsnap=__min(xsnap,ACTIVEBKG->Xsize-1);
	xsnap=__max(xsnap,1);

 

	if (!USE_LIGHT_OPTIM)
	{
 		for (j=0;j<ACTIVEBKG->Zsize;j++)
		{
			feg=&ACTIVEBKG->fastdata[0][j];

			for (i=0; i<ACTIVEBKG->Xsize; i++, *feg++)
			{
				if (feg->treat)
					feg->treat=0;			
			}
		}
	}
	else
	{
		for (j=z0;j<=z1;j++)
		{			
			for (i=x0; i<x1; i++)
			{
				feg=&ACTIVEBKG->fastdata[i][j];
				feg->treat=0;												
			}
		}

		for (j=0;j<ACTIVEBKG->Zsize;j++)
		for (i=0; i<ACTIVEBKG->Xsize; i++)
		{
			if (tilelights[i][j].num)
				tilelights[i][j].num=0;
		}
			}

	

	if ((!HIDEANCHORS) || DEBUG_FRUSTRUM)
	for (long n=0;n<=lcval;n++)
	{
		temp0+=100.f;

		for (j=zsnap-n;j<=zsnap+n;j++)
		{
			for (i=xsnap-n;i<=xsnap+n;i++)
			{
				if ( (i!=xsnap-n) && (i!=xsnap+n) && (j!=zsnap-n) && (j!=zsnap+n) )
					continue;

				if ( (i<0) || (j<0) || (i>=ACTIVEBKG->Xsize) || (j>=ACTIVEBKG->Zsize) ) continue;

				if (i<x0) continue;

				if (i>x1) continue;
	
						feg = &ACTIVEBKG->fastdata[i][j]; 
			
				if (!HIDEANCHORS)
				for (lll=0;lll<feg->nbianchors;lll++)
				{
					_ANCHOR_DATA * ad=&ACTIVEBKG->anchors[feg->ianchors[lll]];
					ad->pos.y-=10;			

					if (ad->nblinked==0) DebugSphere(ad->pos.x,ad->pos.y,ad->pos.z,3.f,90,0xFF00FF00);
					else 
					{
						if (ad->flags & ANCHOR_FLAG_BLOCKED)
							DebugSphere(ad->pos.x,ad->pos.y,ad->pos.z,3.f,90,0xFF00FFFF);
						else if (ad->flags & 1)
							DebugSphere(ad->pos.x,ad->pos.y,ad->pos.z,3.f,90,0xFF00FF00);
						else DebugSphere(ad->pos.x,ad->pos.y,ad->pos.z,3.f,90,0xFFFF0000);
					}

					for (long k=0;k<ad->nblinked;k++)
					{
						
						_ANCHOR_DATA * ad2=&ACTIVEBKG->anchors[ad->linked[k]];
						ad2->pos.y-=10;

						if ((ad->flags & 1) && (ad2->flags & 1))
							EERIEDrawTrue3DLine(pd3dDevice,&ad->pos,&ad2->pos,0xFF00FF00);
						else EERIEDrawTrue3DLine(pd3dDevice,&ad->pos,&ad2->pos,0xFFFF0000);

						ad2->pos.y+=10;
					}

					ad->pos.y+=10;

					if (DEBUGNPCMOVE)
					{
						EERIE_CYLINDER cyl;
						cyl.origin.x=ad->pos.x;
						cyl.origin.y=ad->pos.y;
						cyl.origin.z=ad->pos.z;
						cyl.radius=ad->radius;
						cyl.height=ad->height;			
						EERIEDraw3DCylinderBase(GDevice,&cyl,0xFFFFFF00);  
					}
				}

				if (DEBUG_FRUSTRUM)
				{
					DebugSphere(i*ACTIVEBKG->Xdiv+50.f,feg->frustrum_maxy,j*ACTIVEBKG->Zdiv+50.f,3.f,90,0xFFFF00FF);
					DebugSphere(i*ACTIVEBKG->Xdiv+50.f,feg->frustrum_miny,j*ACTIVEBKG->Zdiv+50.f,3.f,90,0xFFFFFF00);
				}
			}
		}
	}

if (USE_PORTALS && portals)
{
	long room_num=ARX_PORTALS_GetRoomNumForPosition(&ACTIVECAM->pos,1);
	LAST_ROOM=room_num;

	if (room_num>-1)
	{
		ARX_PORTALS_InitDrawnRooms();
		

		ARX_CHECK_LONG( prec );
		long lprec = ARX_CLEAN_WARN_CAST_LONG( prec );

		switch (USE_PORTALS)
		{
			case 1:
				EERIE_2D_BBOX bbox;
				bbox.min.x=0;
				bbox.min.y=0;

				bbox.max.x = ARX_CLEAN_WARN_CAST_FLOAT( DANAESIZX );
				bbox.max.y = ARX_CLEAN_WARN_CAST_FLOAT( DANAESIZY );

				ARX_PORTALS_ComputeRoom(room_num,&bbox,lprec,tim);
				ARX_PORTALS_RenderRooms(lprec,tim);
				break;
			case 2:
				EERIE_FRUSTRUM frustrum;
				CreateScreenFrustrum(&frustrum);
				LAST_PORTALS_COUNT=ARX_PORTALS_Frustrum_ComputeRoom(room_num,&frustrum,lprec,tim);
				ARX_PORTALS_Frustrum_RenderRooms(lprec,tim);
				break;
			case 3:
				CreateScreenFrustrum(&frustrum);
				LAST_PORTALS_COUNT=ARX_PORTALS_Frustrum_ComputeRoom(room_num,&frustrum,lprec,tim);
				ARX_PORTALS_Frustrum_RenderRoomsT(lprec,tim);
				break;
			case 4:
				CreateScreenFrustrum(&frustrum);
				LAST_PORTALS_COUNT=ARX_PORTALS_Frustrum_ComputeRoom(room_num,&frustrum,lprec,tim);
				ARX_PORTALS_Frustrum_RenderRoomsTCullSoft(lprec,tim);
				break;
			break;
		}


		//ARX_SCENE_DilateBackground();
	}
}
else
{
	for (long n=0;n<=lcval;n++)
	{
		temp0+=100.f;

	for (j=zsnap-n;j<=zsnap+n;j++)
	{
	for (i=xsnap-n;i<=xsnap+n;i++)
	{
		if ( (i!=xsnap-n) && (i!=xsnap+n) && (j!=zsnap-n) && (j!=zsnap+n) )
		{
			continue;
		}

		if ( (i<0) || (j<0) || (i>=ACTIVEBKG->Xsize) || (j>=ACTIVEBKG->Zsize) ) continue;

		if (i<x0) continue;

		if (i>x1) continue;
	
						feg = &ACTIVEBKG->fastdata[i][j]; 

		if (!feg->treat) continue;		

		for ( lll=0;lll<feg->nbpoly;lll++)
		{
			//SPECIFIC INTEL COMPILER  
			ep=&feg->polydata[lll];

			if (ep->type & (POLY_IGNORE | POLY_NODRAW))		
				continue;

			if ((ep->min.y > feg->frustrum_maxy)	
				|| (ep->max.y < feg->frustrum_miny))
				continue;
			
			// GO for 3D Backface Culling
			if (ep->type & POLY_DOUBLESIDED)
				SETCULL( pd3dDevice, D3DCULL_NONE );
			else
			{
				
				nrm.x=ep->v[2].sx-ACTIVECAM->pos.x;
				nrm.y=ep->v[2].sy-ACTIVECAM->pos.y;
				nrm.z=ep->v[2].sz-ACTIVECAM->pos.z;

				if ( ep->type & POLY_QUAD) 
				{
					if ( (DOTPRODUCT( ep->norm , nrm )>0.f) &&
						 (DOTPRODUCT( ep->norm2 , nrm )>0.f) )	
						continue;
				}
				else if ( DOTPRODUCT( ep->norm , nrm )>0.f)
						continue;

				SETCULL( pd3dDevice, D3DCULL_CW );
			}
			 
							if (!EERIERTPPoly(ep)) 
				continue; 

			if ( ep->type & POLY_QUAD) 
			{
				if (FRAME_COUNT<=0) ep->tv[3].color=ep->v[3].color;	

				to=4;
			}
			else to=3;

	
			if (FRAME_COUNT<=0)
			{
								if (!USE_D3DFOG) ComputeFog(ep->tv,to);
			}

			if (ep->type & POLY_TRANS) 
			{
				ManageLavaWater(ep,to,tim);
				TransPol[TRANSPOLYSPOS++]=ep;

				if (TRANSPOLYSPOS>=MAX_TRANSPOL) TRANSPOLYSPOS=MAX_TRANSPOL-1;

				if (ViewMode)
				{
					if (ViewMode & VIEWMODE_WIRE) 
						EERIEPOLY_DrawWired(pd3dDevice,ep);
					
					if (ViewMode & VIEWMODE_NORMALS) 
						EERIEPOLY_DrawNormals(pd3dDevice,ep);
				}	

				continue;
			}

			if (!Project.improve)  // Normal View...			
			{
				if (ep->type & POLY_GLOW) ep->tv[0].color=ep->tv[1].color=ep->tv[2].color=ep->tv[3].color=0xFFFFFFFF;
				else 
				{
					if (FRAME_COUNT<=0)
					{
						if (ModeLight & MODE_DYNAMICLIGHT) 	ApplyDynLight(ep);					
						else
						{
							ep->tv[0].color=ep->v[0].color;	
							ep->tv[1].color=ep->v[1].color;	
							ep->tv[2].color=ep->v[2].color;		
						}
						
					}
				}

				ManageLavaWater(ep,to,tim);
				
				Delayed_EERIEDRAWPRIM(ep);

				if (ViewMode)
				{
		
					if (ViewMode & VIEWMODE_WIRE) 
						EERIEPOLY_DrawWired(pd3dDevice,ep);
					
					if (ViewMode & VIEWMODE_NORMALS) 
						EERIEPOLY_DrawNormals(pd3dDevice,ep);
				}	
			}
			else // Improve Vision Activated
			{			
				if (FRAME_COUNT<=0)
				{
					if ( ModeLight & MODE_DYNAMICLIGHT ) 	ApplyDynLight(ep);										
					else 
					{	
						ep->tv[0].color=ep->v[0].color;	
						ep->tv[1].color=ep->v[1].color;	
						ep->tv[2].color=ep->v[2].color;				
					}
				

					for (k=0;k<to;k++) 
					{
						lr=(ep->tv[k].color>>16) & 255;
						ffr=(float)(lr);
							
						dd=(ep->tv[k].sz*prec);

						if (dd>1.f) dd=1.f;

						if (dd<0.f) dd=0.f;
						
						fb=((1.f-dd)*6.f + (EEfabs(ep->nrml[k].x)+EEfabs(ep->nrml[k].y)))*0.125f;
						fr=((0.6f-dd)*6.f + (EEfabs(ep->nrml[k].z)+EEfabs(ep->nrml[k].y)))*0.125f;//(1.f-dd);						

						if (fr<0.f) fr=0.f;
						else fr=__max(ffr,fr*255.f);

						fb*=255.f;
						F2L(fr,&lfr);
						F2L(fb,&lfb);
						F2L(fr,&lfr);
						ep->tv[k].color=( 0xff001E00L | ( (lfr & 255) << 16) | (lfb & 255) );
						//GG component locked at 0x1E
					}
								}

				Delayed_EERIEDRAWPRIM(ep);
			}			
			}			
		}
	}
	}
}

	if(pGetInfoDirectInput->IsVirtualKeyPressedNowPressed(DIK_J))
		bOLD_CLIPP=!bOLD_CLIPP;

	if ((SHOWSHADOWS) && (!Project.improve))
		ARXDRAW_DrawInterShadows(pd3dDevice);

	FRAME_COUNT=LAST_FC;

	if(USE_PORTALS<3)
			Delayed_FlushAll(pd3dDevice);

		
		ARX_CHECK_ULONG(FrameDiff);
		ARX_THROWN_OBJECT_Manage(ARX_CLEAN_WARN_CAST_ULONG(FrameDiff));
		
		VF_CLIP_IO=1;
		RenderInter(pd3dDevice, 0.f, 3200.f);
		
		VF_CLIP_IO=0;
		

	if (DRAGINTER) // To render Dragged objs
	{
		SPECIAL_DRAGINTER_RENDER=1;
		ARX_INTERFACE_RenderCursor();

		if(USE_PORTALS<3)
			Delayed_FlushAll(pd3dDevice);

		SPECIAL_DRAGINTER_RENDER=0;
	}

	PopAllTriangleList(true);
	
					}
					
	
	if (EXTERNALVIEW)
		ARXDRAW_DrawExternalView(pd3dDevice);


	DRAWLATER_Render(pd3dDevice);

	if (DEBUGCODE) ForceSendConsole("RenderBackground - Boom",1,0,(HWND)1);	

	if (ACTIVECAM->type!=CAM_TOPVIEW) 
	{
		
		if ((eyeball.exist!=0) && eyeballobj)
			ARXDRAW_DrawEyeBall(pd3dDevice);

		
		SETZWRITE(pd3dDevice, FALSE );

		if (BoomCount) 
			ARXDRAW_DrawPolyBoom(pd3dDevice);

		if (INTERTRANSPOLYSPOS&&(!bRenderInterList))
			ARXDRAW_DrawAllInterTransPolyPos(pd3dDevice);

		PopAllTriangleListTransparency();
		
		if(	(USE_PORTALS>2)&&
			(portals) )
		{
			ARX_PORTALS_Frustrum_RenderRooms_TransparencyT();
		}
		else
		{
			if (TRANSPOLYSPOS)
				ARXDRAW_DrawAllTransPolysPos(pd3dDevice,MODIF);
		}
	}

if (HALOCUR>0)
{
	SETTC(pd3dDevice,NULL);
	pd3dDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND,  D3DBLEND_SRCCOLOR );
	pd3dDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE );	
	SETALPHABLEND(pd3dDevice,TRUE);			
	SETCULL(pd3dDevice,D3DCULL_NONE);
	SETZWRITE(pd3dDevice,FALSE);

	for (i=0;i<HALOCUR;i++)
	{
		//blue halo rendering (keyword : BLUE HALO RENDERING HIGHLIGHT AURA)
		D3DTLVERTEX * vert=&LATERDRAWHALO[(i<<2)];

		if (vert[2].color == 0)
		{
			pd3dDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ZERO );
			pd3dDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCCOLOR );									
			vert[2].color =0xFF000000;
			EERIEDRAWPRIM(pd3dDevice,D3DPT_TRIANGLEFAN, D3DFVF_TLVERTEX| D3DFVF_DIFFUSE , vert, 4,  0, 0 );//>>> DO NOT USE EERIE_USEVB FOR HALO <<<
			pd3dDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND,  D3DBLEND_SRCCOLOR );
			pd3dDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE );	
		}
		else EERIEDRAWPRIM(pd3dDevice,D3DPT_TRIANGLEFAN, D3DFVF_TLVERTEX| D3DFVF_DIFFUSE , vert, 4,  0, 0 );//>>> DO NOT USE EERIE_USEVB FOR HALO <<<
	}

		 HALOCUR = 0; 
	SETALPHABLEND(pd3dDevice,FALSE);			
}

	SETCULL(pd3dDevice,D3DCULL_CCW);
	SETALPHABLEND(pd3dDevice,FALSE);	
	SETZWRITE(pd3dDevice, TRUE );

	if (EDITION==EDITION_LIGHTS)
		ARXDRAW_DrawAllLights(pd3dDevice,x0,z0,x1,z1);

	if (LIGHTTHREAD)
		ResumeThread(LIGHTTHREAD);
}

