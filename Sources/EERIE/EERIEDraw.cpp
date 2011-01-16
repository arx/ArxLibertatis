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

#include "EERIEDraw.h" 
#include "EERIEApp.h" 
#include "EERIEPoly.h" 

#include "HERMESMain.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

extern void ComputeSingleFogVertex(D3DTLVERTEX*);


TextureContainer * EERIE_DRAW_sphere_particle=NULL;
TextureContainer * EERIE_DRAW_square_particle=NULL;

typedef struct
{
	EERIEPOLY * ep;
} TODRAWLATER;

long curdrawlater=0;
#define MAX_DRAWLATER 256
TODRAWLATER tdl[MAX_DRAWLATER];
long DBGSETTEXTURE=0;
 

long ZMAPMODE=1;
TextureContainer * Zmap;
EERIE_3D SPRmins;
EERIE_3D SPRmaxs;

extern long REFLECTFX;
extern long WATERFX;
extern TextureContainer * enviro;
extern float FrameTime;
extern EERIE_3D e3dPosBump;
extern bool bALLOW_BUMP;
extern bool bSoftRender;
extern bool bZBUFFER;

/*---------------------------------------------------------------------------------------------------------*/
typedef struct
{
	union
	{
		D3DVALUE sx; 
		D3DVALUE dvSX;     
	};    
	union
	{
		D3DVALUE sy; 
		D3DVALUE dvSY;
	};
	union
	{
		D3DVALUE sz; 
		D3DVALUE dvSZ; 
    };
	union
	{
        D3DVALUE rhw; 
		D3DVALUE dvRHW; 
    }; 
	union
	{
		D3DCOLOR color; 
		D3DCOLOR dcColor;
	}; 
	union
	{
		D3DCOLOR specular; 
		D3DCOLOR dcSpecular; 
    };
	union
	{
        D3DVALUE tu1; 
		D3DVALUE dvTU1;
	};
	union
	{
		D3DVALUE tv1; 
		D3DVALUE dvTV1; 
    };
	union
	{
        D3DVALUE tu2; 
		D3DVALUE dvTU2;
	};
	union
	{
		D3DVALUE tv2; 
		D3DVALUE dvTV2; 
    };
} D3DTLVERTEX2UV;


void CopyVertices(EERIEPOLY * ep,long to, long from)
{
	memcpy(&ep->v[to],&ep->v[from],sizeof(D3DTLVERTEX));
	memcpy(&ep->tv[to],&ep->tv[from],sizeof(D3DTLVERTEX));
	memcpy(&ep->nrml[to],&ep->nrml[from],sizeof(EERIE_3D));
}
 
bool NearlyEqual(float a,float b)
{
	if (EEfabs(a-b)<0.01f) return true;

	if (EEfabs(b-a)<0.01f) return true;

	return false;
}

BOOL Quadable(EERIEPOLY * ep, EERIEPOLY * ep2, float tolerance)
{
	
	long count=0;
	long common=-1;
	long common2=-1;
	
	
	long ep_notcommon=-1;
	long ep2_notcommon=-1;
	
	if (ep2->type & POLY_QUAD)
					{
	return FALSE;
}
	
	if (ep->tex != ep2->tex) return FALSE;

	long typ1=ep->type&(~POLY_QUAD);
	long typ2=ep2->type&(~POLY_QUAD);

	if (typ1!=typ2) return FALSE;

	if ((ep->type & POLY_TRANS) && (ep->transval!=ep2->transval)) return FALSE;

	CalcFaceNormal(ep,ep->v);

	if (EEfabs(Vector_DotProduct(&ep->norm,&ep2->norm))<1.f-tolerance) return FALSE;
	
	for (long i=0;i<3;i++)
	{
		common=-1;
		common2=-1;

		for (long j=0;j<3;j++)
		{
			if (   ( NearlyEqual(ep->v[i].sx,ep2->v[j].sx) )
				&& ( NearlyEqual(ep->v[i].sy,ep2->v[j].sy) )
				&& ( NearlyEqual(ep->v[i].sz,ep2->v[j].sz) )
				&& ( NearlyEqual(ep->v[i].tu,ep2->v[j].tu) )
				&& ( NearlyEqual(ep->v[i].tv,ep2->v[j].tv) )
				)
			{
				count++;
				common=j;
			}

			if (   ( NearlyEqual(ep->v[j].sx,ep2->v[i].sx) )
				&& ( NearlyEqual(ep->v[j].sy,ep2->v[i].sy) )
				&& ( NearlyEqual(ep->v[j].sz,ep2->v[i].sz) )
				&& ( NearlyEqual(ep->v[j].tu,ep2->v[i].tu) )
				&& ( NearlyEqual(ep->v[j].tv,ep2->v[i].tv) )
				)
			{				
				common2=j;
			}		
		}

		if (common2==-1) ep2_notcommon=i;

		if (common==-1) ep_notcommon=i;
	}

	if ((count>=2) && (ep_notcommon!=-1) && (ep2_notcommon!=-1))
	{
		ep2->type |= POLY_QUAD;

		switch (ep2_notcommon)
		{
			case 1:
				CopyVertices(ep2,3,0);
				CopyVertices(ep2,0,1);
				CopyVertices(ep2,1,2);
				CopyVertices(ep2,2,3);				
			break;
		
			case 2:	
				CopyVertices(ep2,3,0);
				CopyVertices(ep2,0,2);				
				CopyVertices(ep2,2,1);
				CopyVertices(ep2,1,3);
			
			break;
		}

		CopyVertices(ep2,3,0);
		ep2->v[3].sx=ep->v[ep_notcommon].sx;
		ep2->v[3].sy=ep->v[ep_notcommon].sy;
		ep2->v[3].sz=ep->v[ep_notcommon].sz;
		ep2->tv[3].tu=ep2->v[3].tu=ep->v[ep_notcommon].tu;
		ep2->tv[3].tv=ep2->v[3].tv=ep->v[ep_notcommon].tv;
		ep2->tv[3].color=ep2->v[3].color=EERIECOLOR_WHITE;
		ep2->tv[3].rhw=ep2->v[3].rhw=1.f;

	DeclareEGInfo(ep->v[3].sx,ep->v[3].sy,ep->v[3].sz);

	ep2->center.x=(ep2->v[0].sx+ep2->v[1].sx+ep2->v[2].sx+ep2->v[3].sx)*DIV4;
	ep2->center.y=(ep2->v[0].sy+ep2->v[1].sy+ep2->v[2].sy+ep2->v[3].sy)*DIV4;
	ep2->center.z=(ep2->v[0].sz+ep2->v[1].sz+ep2->v[2].sz+ep2->v[3].sz)*DIV4;
	ep2->max.x=__max(ep2->max.x,ep2->v[3].sx);
	ep2->min.x=__min(ep2->min.x,ep2->v[3].sx);
	ep2->max.y=__max(ep2->max.y,ep2->v[3].sy);
	ep2->min.y=__min(ep2->min.y,ep2->v[3].sy);
	ep2->max.z=__max(ep2->max.z,ep2->v[3].sz);
	ep2->min.z=__min(ep2->min.z,ep2->v[3].sz);

	ep2->norm2.x=ep->norm.x;
	ep2->norm2.y=ep->norm.y;
	ep2->norm2.z=ep->norm.z;
	
	ep2->area+=Distance3D(	(ep2->v[1].sx+ep2->v[2].sx)*DIV2,
							(ep2->v[1].sy+ep2->v[2].sy)*DIV2,
							(ep2->v[1].sz+ep2->v[2].sz)*DIV2,
							ep2->v[3].sx,ep2->v[3].sy,ep2->v[3].sz)
			*Distance3D(	ep2->v[3].sx,ep2->v[3].sy,ep2->v[3].sz,
							ep2->v[1].sx,ep2->v[1].sy,ep2->v[1].sz)*DIV2;

		return TRUE;
	}

	return FALSE;
}

extern long COMPUTE_PORTALS;
#define TYPE_ROOM	2
BOOL TryToQuadify(EERIEPOLY * ep,EERIE_3DOBJ * eobj)
{
	long posx,posz,posy;
	float cx,cy,cz;
	EERIE_BKG_INFO * eg;
	cx=(ep->v[0].sx+ep->v[1].sx+ep->v[2].sx);
	cy=(ep->v[0].sy+ep->v[1].sy+ep->v[2].sy);
	cz=(ep->v[0].sz+ep->v[1].sz+ep->v[2].sz);
	F2L(cx*DIV3*ACTIVEBKG->Xmul,&posx);
	F2L(cy*DIV3*ACTIVEBKG->Xmul+ACTIVEBKG->Xsize*DIV2,&posy);
	F2L(cz*DIV3*ACTIVEBKG->Zmul,&posz);
	
	long dx,dz,fx,fz;
	dx=__max(0,posx-1);
	fx=__min(posx+1,ACTIVEBKG->Xsize-1);
	dz=__max(0,posz-1);
	fz=__min(posz+1,ACTIVEBKG->Zsize-1);
	float tolerance=0.1f;

	for (long kl = 0; kl < 2; kl++)
	for (long zz=dz;zz<=fz;zz++)
	for (long xx=dx;xx<=fx;xx++)
	{
		long type,val1,val2;

		if (COMPUTE_PORTALS)
		{

			if (!GetNameInfo(eobj->name,&type,&val1,&val2))
				return FALSE;

			if (type!=TYPE_ROOM)
				return FALSE;
		}

		eg=(EERIE_BKG_INFO *)&ACTIVEBKG->Backg[xx+zz*ACTIVEBKG->Xsize];

		if (eg)
		for (long n=0;n<eg->nbpoly;n++)
		{
			EERIEPOLY * ep2=(EERIEPOLY *)&eg->polydata[n];

			if (COMPUTE_PORTALS)
			{
				if (ep2->room!=val1) continue;
			}
			
			if (ep==ep2) continue;

			if ((kl==0) && (ep2->type & POLY_QUAD) )
			{
				if (Quadable(ep,ep2,tolerance)) return TRUE;
			}
			else if ((kl==1) && !(ep2->type & POLY_QUAD) )
			{
				if (Quadable(ep,ep2,tolerance)) return TRUE;
			}
		}
	}

	return FALSE;
}


void DRAWLATER_ReInit()
{
	curdrawlater=0;
}

void EERIE_DRAW_SetTextureZMAP(long num,TextureContainer * Z_map)
{
	Zmap=Z_map;
}
void DRAWLATER_Render(LPDIRECT3DDEVICE7 pd3dDevice)
{
	if (curdrawlater==0) return;

		D3DTLVERTEX verts[4];
		pd3dDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ZERO );
		pd3dDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCCOLOR );	
		SETZWRITE(pd3dDevice,FALSE); 
		SETALPHABLEND(pd3dDevice,TRUE);					
		SETCULL( pd3dDevice, D3DCULL_NONE );
		long to;

		for (long j=0;j<curdrawlater;j++)
		{
			if (tdl[j].ep->tex==NULL) continue;

			if (tdl[j].ep->tex->TextureRefinement==NULL) continue;

			if (tdl[j].ep->tex->TextureRefinement->m_pddsSurface==NULL)
			{
				tdl[j].ep->tex->TextureRefinement->Restore(pd3dDevice);

				if (tdl[j].ep->tex->TextureRefinement->m_pddsSurface==NULL) continue;
			}

			if (tdl[j].ep->type & POLY_QUAD) to=4;
			else to=3;

			long tmp;
			long i=0;

			for (;i<to;i++)
			{
				verts[i].tu=tdl[j].ep->tv[i].tu;
				verts[i].tv=tdl[j].ep->tv[i].tv;
				verts[i].color=tdl[j].ep->tv[i].color;
				verts[i].sz=tdl[j].ep->tv[i].sz;
				tdl[j].ep->tv[i].tu*=4.f;
				tdl[j].ep->tv[i].tv*=4.f;

				float val;

				if(bZBUFFER)
				{
				val = (0.048f - tdl[j].ep->tv[i].sz); 
				}
				else
				{
				val = (0.048f - tdl[j].ep->tv[i].rhw);
				}
				
				if (val<=0.f) 
				{
					tdl[j].ep->tv[i].color=0xFF000000;
				}
				else 
				{
					if (val>0.0175) 
					{
					tdl[j].ep->tv[i].color = 0xFFB2B2B2; 
					}
					else 
					{
						F2L(val*10200,&tmp);
						tdl[j].ep->tv[i].color=0xFF000000 | (tmp<<16) | (tmp<<8) | tmp;
					}

				}				
			}

			tdl[j].ep->tv[i].sz-=0.001f;

			SETTC(pd3dDevice,tdl[j].ep->tex->TextureRefinement);
			EERIEDRAWPRIM( pd3dDevice, D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX | D3DFVF_DIFFUSE, tdl[j].ep, to, 0, EERIE_NOCOUNT | (bSoftRender?EERIE_USEVB:0) );

			for (int i=0;i<to;i++)
			{
				tdl[j].ep->tv[i].tu=verts[i].tu;
				tdl[j].ep->tv[i].tv=verts[i].tv;
				tdl[j].ep->tv[i].color=verts[i].color;	
				tdl[j].ep->tv[i].sz=verts[i].sz;
			}
		}

		EERIEDrawnPolys+=curdrawlater;
		SETALPHABLEND(pd3dDevice,FALSE); 
		SETZWRITE(pd3dDevice,TRUE); 

	SETTC(pd3dDevice,NULL);
}

//------------------------------------------------------------------------------

//Draw primitive using vertex buffer
HRESULT ARX_DrawPrimitiveVB(	LPDIRECT3DDEVICE7	_d3dDevice, 
								D3DPRIMITIVETYPE	_dptPrimitiveType, 
								DWORD				_dwVertexTypeDesc,	// Vertex Format
								LPVOID				_pD3DTLVertex,		// don't specify _pD3DTLVertex type.
								int *				_piNbVertex, 
								DWORD				_dwFlags );

//------------------------------------------------------------------------------

bool bForce_NoVB = false;
void SET_FORCE_NO_VB( const bool& _NoVB )
{
	bForce_NoVB = _NoVB;
}
bool GET_FORCE_NO_VB( void )
{
	return bForce_NoVB;
}

//------------------------------------------------------------------------------

HRESULT EERIEDRAWPRIM(	LPDIRECT3DDEVICE7 pd3dDevice,
						D3DPRIMITIVETYPE dptPrimitiveType,  
						DWORD  dwVertexTypeDesc,            
						LPVOID lpvVertices,                 
						DWORD  dwVertexCount,               
						DWORD  dwFlags,
						long flags,
						EERIEPOLY * ep
					)
{
	if( !( EERIE_NOCOUNT & flags ) )
	{
	EERIEDrawnPolys++;
	}

	if(	!bForce_NoVB &&	flags&EERIE_USEVB )
	{
		return ARX_DrawPrimitiveVB(	pd3dDevice,
									dptPrimitiveType,
									dwVertexTypeDesc, 		//FVF
									lpvVertices,			//No type specified
									(int*)&dwVertexCount,
									dwFlags);				//Same thing throught DrawPrimitiveVB
	}
	else
	{
		return (pd3dDevice->DrawPrimitive(dptPrimitiveType,dwVertexTypeDesc,lpvVertices,dwVertexCount,dwFlags));	
	}
}
		
void Delayed_FlushAll(LPDIRECT3DDEVICE7 pd3dDevice)
{
	int flg_NOCOUNT_USEVB = EERIE_NOCOUNT;

	TextureContainer* ptcTexture = GetTextureList();

    while( ptcTexture )
    {
		if ((ptcTexture->delayed_nb) && ptcTexture->delayed)
		{
			long to;

			if (ViewMode & VIEWMODE_FLAT)
				SETTC(pd3dDevice,NULL);
			else 
				SETTC(pd3dDevice,ptcTexture);

			DELAYED_PRIM * del=(DELAYED_PRIM *)ptcTexture->delayed;

			for (long i=0;i<ptcTexture->delayed_nb;i++)
			{				
				EERIEPOLY * ep=del[i].data;

				if (!(ep->type & POLY_DOUBLESIDED))
					SETCULL( pd3dDevice, D3DCULL_CW );			
				else 
					SETCULL( pd3dDevice, D3DCULL_NONE );
				
				if (ep->type & POLY_QUAD)
					to=4;
				else to=3;

				EERIEDRAWPRIM(	pd3dDevice, D3DPT_TRIANGLESTRIP,
										D3DFVF_TLVERTEX| D3DFVF_DIFFUSE,
								ep->tv,	to,	0 , flg_NOCOUNT_USEVB );
					
				if ( ptcTexture->userflags & POLY_METAL)
				{
					pd3dDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND,  D3DBLEND_DESTCOLOR );
					pd3dDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE );	
					SETALPHABLEND(pd3dDevice,TRUE);	
					SETTC(pd3dDevice,NULL); 

					EERIEDRAWPRIM(	pd3dDevice, D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX| D3DFVF_DIFFUSE, ep->tv,	to,	0, flg_NOCOUNT_USEVB );
					EERIEDrawnPolys++;	
				}

				if ( (ep->type & POLY_LAVA) || (ep->type & POLY_WATER) )
				{
					pd3dDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND,  D3DBLEND_DESTCOLOR );
					pd3dDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE );	
					SETALPHABLEND(pd3dDevice,TRUE);	
					D3DTLVERTEX verts[4];
					SETTC(pd3dDevice,enviro);

					for (long i=0;i<to;i++)
					{
						verts[i].sx=ep->tv[i].sx;
						verts[i].sy=ep->tv[i].sy;
						verts[i].sz=ep->tv[i].sz;
						verts[i].rhw=ep->tv[i].rhw;
						verts[i].color=0xFFFFFFFF;

						// Water
						if (ep->type & POLY_LAVA)
						{
							verts[i].tu=ep->v[i].sx*DIV1000+EEsin((ep->v[i].sx)*DIV200+(float)FrameTime*DIV2000)*DIV20;
							verts[i].tv=ep->v[i].sz*DIV1000+EEcos((ep->v[i].sz)*DIV200+(float)FrameTime*DIV2000)*DIV20;
						}
						else
						{
							pd3dDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ONE );
							verts[i].tu=ep->v[i].sx*DIV800+EEsin((ep->v[i].sx)*DIV600+(float)FrameTime*DIV1000)*DIV9;
							verts[i].tv=ep->v[i].sz*DIV800+EEcos((ep->v[i].sz)*DIV600+(float)FrameTime*DIV1000)*DIV9;

							if (ep->type & POLY_FALL) verts[i].tv-=(float)(FrameTime)*DIV200;
						}
					}

					EERIEDRAWPRIM( pd3dDevice, D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX| D3DFVF_DIFFUSE, verts, to, 0, flg_NOCOUNT_USEVB );

					if (ep->type & POLY_WATER)
					{
						for (long i=0;i<to;i++)
						{
							verts[i].tu=ep->v[i].sx*DIV1000+EEsin((ep->v[i].sy)*DIV200+(float)FrameTime*DIV600*DIV3)*DIV10;
							verts[i].tv=ep->v[i].sz*DIV1000+EEcos((ep->v[i].sz+ep->v[i].sx)*DIV200+(float)FrameTime*DIV600*DIV3)*DIV10;

							if (ep->type & POLY_FALL) 
							{
								verts[i].tv-=(float)(FrameTime)*DIV200;
							}
						}	

						EERIEDRAWPRIM( pd3dDevice, D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX| D3DFVF_DIFFUSE, verts, to, 0, flg_NOCOUNT_USEVB );
						EERIEDrawnPolys++;
					}

					if (ep->type & POLY_LAVA)
					{
						for (long i=0;i<to;i++)
						{
							verts[i].tu=ep->v[i].sx*DIV1000+EEsin((ep->v[i].sx)*DIV100+(float)FrameTime*DIV2000)*DIV10;
							verts[i].tv=ep->v[i].sz*DIV1000+EEcos((ep->v[i].sz)*DIV100+(float)FrameTime*DIV2000)*DIV10;
						}	
						EERIEDRAWPRIM( pd3dDevice, D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX| D3DFVF_DIFFUSE, verts, to, 0, flg_NOCOUNT_USEVB );
						for ( int i=0;i<to;i++)
						{
							verts[i].tu=ep->v[i].sx*DIV600+EEsin((ep->v[i].sx)*DIV160+(float)FrameTime*DIV2000)*DIV11;
							verts[i].tv=ep->v[i].sz*DIV600+EEcos((ep->v[i].sz)*DIV160+(float)FrameTime*DIV2000)*DIV11;
							verts[i].color=0xFF666666;
						}	

						pd3dDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ZERO );
						pd3dDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCCOLOR);
						EERIEDRAWPRIM( pd3dDevice, D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX| D3DFVF_DIFFUSE, verts, to, 0, flg_NOCOUNT_USEVB );
						EERIEDrawnPolys+=2;
					}
				}
					
#define MAX_DIST_BUMP			600.f
#define ONE_ONE_MAX_DIST_BUMP	(1.f/MAX_DIST_BUMP)
#define MAX_BUMP				.8f

				if ((bALLOW_BUMP) &&
						( ep->tex     )					&&
						( ep->tex->m_pddsBumpMap ) )
					{
						float fDx=e3dPosBump.x-ep->center.x;
						float fDy=e3dPosBump.y-ep->center.y;
						float fDz=e3dPosBump.z-ep->center.z;
						float fDist=(fDx*fDx)+(fDy*fDy)+(fDz*fDz);

						if(fDist<=(MAX_DIST_BUMP*MAX_DIST_BUMP))	//6 metres
						{
							fDist=(float)sqrt(fDist);
							EERIE_DrawPolyBump(pd3dDevice,ep,MAX_BUMP*(MAX_DIST_BUMP-fDist)*ONE_ONE_MAX_DIST_BUMP);
							EERIEDrawnPolys++;

						}

					}

					SETTC(pd3dDevice,ptcTexture);				
					SETALPHABLEND(pd3dDevice,FALSE);	
			}				
			
			if (ZMAPMODE)
			{				
				if (ptcTexture->TextureRefinement==NULL) ptcTexture->delayed_nb=0;
				else if (ptcTexture->TextureRefinement->m_pddsSurface==NULL)
				{
					ptcTexture->TextureRefinement->Restore(pd3dDevice);

					if (ptcTexture->TextureRefinement->m_pddsSurface==NULL) ptcTexture->delayed_nb=0;
				}

				if (ptcTexture->delayed_nb)
				{
					pd3dDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ZERO );
					pd3dDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCCOLOR );	
					SETZWRITE(pd3dDevice,FALSE); 
					SETALPHABLEND(pd3dDevice,TRUE);					
					D3DTLVERTEX verts[4];
					SETTC(pd3dDevice,ptcTexture->TextureRefinement); 

					for (long i=0;i<ptcTexture->delayed_nb;i++)
					{
						DELAYED_PRIM * del=(DELAYED_PRIM *)ptcTexture->delayed;
						EERIEPOLY * ep=del[i].data;

						if (ep->type & POLY_QUAD) 
						{
							if ( (ep->tv[0].sz>0.048f) 
								&& (ep->tv[1].sz>0.048f)
								&& (ep->tv[2].sz>0.048f) 
							        && (ep->tv[3].sz > 0.048f)) continue; 

							to=4;
						}
						else 
						{
							if ( (ep->tv[0].sz>0.048f) 
								&& (ep->tv[1].sz>0.048f)
							        && (ep->tv[2].sz > 0.048f)) continue; 

							to=3;	
						}					

						if (!(ep->type & POLY_DOUBLESIDED))
							SETCULL( pd3dDevice, D3DCULL_CW );			
						else SETCULL( pd3dDevice, D3DCULL_NONE );
						
						
						register long tmp;

						for (register long j=0;j<to;j++)
						{
							verts[j].sx=ep->tv[j].sx;
							verts[j].sy=ep->tv[j].sy;
							verts[j].sz=ep->tv[j].sz;
							verts[j].tu=ep->tv[j].tu*4.f;
							verts[j].tv=ep->tv[j].tv*4.f;
							verts[j].rhw=ep->tv[j].rhw;

							float val;

							val = (0.038f - verts[j].sz); 

							if (val<=0.f) 
							{
								verts[j].color=0xFF000000;
							}
							else 
							{
								if (val>0.0175) 
								{
									verts[j].color=0xFFB2B2B2;
								}
								else 
								{
									F2L(val*10200,&tmp);
									verts[j].color=0xFF000000 | (tmp<<16) | (tmp<<8) | tmp;
								}			
							}				
						}

						EERIEDRAWPRIM(	pd3dDevice, D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX| D3DFVF_DIFFUSE, verts, to, 0, flg_NOCOUNT_USEVB );
					}
				}

				EERIEDrawnPolys+=ptcTexture->delayed_nb;
				SETALPHABLEND(pd3dDevice,FALSE); 
				SETZWRITE(pd3dDevice,TRUE);
			}

			EERIEDrawnPolys+=ptcTexture->delayed_nb;
			ptcTexture->delayed_nb=0;
		}

		ptcTexture = ptcTexture->m_pNext;
    }	
}

void Delayed_EERIEDRAWPRIM( EERIEPOLY * ep)
{
	TextureContainer * tc=ep->tex; 

	if (!tc) return;

	if (tc->delayed_nb>=tc->delayed_max)
	{
	retry:
		;
		tc->delayed=(DELAYED_PRIM *)realloc(tc->delayed,sizeof(DELAYED_PRIM)*(tc->delayed_nb+1));

		if (!tc->delayed) 
		{
			if (HERMES_Memory_Emergency_Out(sizeof(DELAYED_PRIM)*(tc->delayed_nb+1),"tc->delayed"))
				goto retry;
		}

		tc->delayed_max=tc->delayed_nb+1;
	}

	DELAYED_PRIM * del=(DELAYED_PRIM *)tc->delayed;
	del[tc->delayed_nb].data=ep;
	tc->delayed_nb++;
}

/* To Keep...
					if ((REFLECTFX) && enviro)
					{
						pd3dDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND,  D3DBLEND_DESTCOLOR );
						pd3dDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE );	
						SETALPHABLEND(pd3dDevice,TRUE);	
						D3DTLVERTEX verts[4];
						SETTC(pd3dDevice,enviro);
						for (long i=0;i<to;i++)
						{
							verts[i].sx=ep->tv[i].sx;
							verts[i].sy=ep->tv[i].sy;
							verts[i].sz=ep->tv[i].sz;
							verts[i].rhw=ep->tv[i].rhw;
							verts[i].color=0xFFFFFFFF;
							verts[i].tu=(ep->v[i].sx-ep->tv[i].sx)*DIV800+(EEsin((ep->v[i].sx-ep->tv[i].sx)*DIV200)*DIV8)*ep->tv[i].rhw;
							verts[i].tv=(ep->v[i].sz-ep->tv[i].sy)*DIV800+(EEcos((ep->v[i].sz-ep->tv[i].sy)*DIV200)*DIV8)*ep->tv[i].rhw;						
						}
						EERIEDRAWPRIM(pd3dDevice,D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX| D3DFVF_DIFFUSE, verts, to, 0,1,ep);
					}*/

/*---------------------------------------------------------------------------------------------------------*/

void CalculTriangleBump(const D3DTLVERTEX& v0,const D3DTLVERTEX& v1,const D3DTLVERTEX& v2,float *du,float *dv)
{
float duab,duac,dvab,dvac,s1,s2,s3;

	duab	=	v1.tu - v0.tu + .3f;
	dvab	=	v1.tv - v0.tv - .3f;
	duac	=	v2.tu - v0.tu + .3f;
	dvac	=	v2.tv - v0.tv - .3f;

	s1		=	( (float)( ( (v0.color >> 16) & 0xFF ) + ( (v0.color >> 8) & 0xFF ) + (v0.color & 0xFF) ) ) * ( .1f / (3.f * 255.f) );
	s2		=	( (float)( ( (v1.color >> 16) & 0xFF ) + ( (v1.color >> 8) & 0xFF ) + (v1.color & 0xFF) ) ) * ( .1f / (3.f * 255.f) );
	s3		=	( (float)( ( (v2.color >> 16) & 0xFF ) + ( (v2.color >> 8) & 0xFF ) + (v2.color & 0xFF) ) ) * ( .1f / (3.f * 255.f) );
	
	s2-=s1;
	s3-=s1;
	
	duab*=s2;
	dvab*=s2;
	duac*=s3;
	dvac*=s3;
	
	duab+=dvac;
	dvab+=dvac;

	*du=duab;
	*dv=dvab;
}

/*---------------------------------------------------------------------------------------------------------*/
void EERIE_DrawPolyBump(LPDIRECT3DDEVICE7 pd3dDevice,EERIEPOLY *ep,float alpha)
{
	int			flg_NOCOUNT_USEVB	=	EERIE_NOCOUNT | (bSoftRender?EERIE_USEVB:0);
float		du,dv;
int			nb;

	if(	(!ep->tv[0].color)&&
		(!ep->tv[1].color)&&
		(!ep->tv[2].color) ) return;

	pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,D3DBLEND_DESTCOLOR);
	pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND,D3DBLEND_SRCCOLOR);	
	SETALPHABLEND(pd3dDevice,TRUE);			
	SETTC(pd3dDevice,ep->tex);

	CalculTriangleBump( ep->tv[0], ep->tv[1], ep->tv[2], &du, &dv );
	du*=alpha;
	dv*=alpha;
	
	if(g_pD3DApp->m_pDeviceInfo->wNbTextureSimultaneous>1)
	{
		D3DTLVERTEX2UV	v[4];
		pd3dDevice->SetTextureStageState(0,D3DTSS_COLORARG1,D3DTA_TEXTURE);
		pd3dDevice->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_SELECTARG1);
		pd3dDevice->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_DISABLE);

		pd3dDevice->SetTexture(1,ep->tex->m_pddsBumpMap);
		pd3dDevice->SetTextureStageState(1,D3DTSS_TEXCOORDINDEX,1);
		pd3dDevice->SetTextureStageState(1,D3DTSS_COLORARG1,D3DTA_TEXTURE|D3DTA_COMPLEMENT);
		pd3dDevice->SetTextureStageState(1,D3DTSS_COLORARG2,D3DTA_CURRENT);
		pd3dDevice->SetTextureStageState(1,D3DTSS_COLOROP,D3DTOP_ADDSIGNED);
		pd3dDevice->SetTextureStageState(1,D3DTSS_ALPHAOP,D3DTOP_DISABLE);
		pd3dDevice->SetTextureStageState(2,D3DTSS_COLOROP,D3DTOP_DISABLE);
		
		*((D3DTLVERTEX*)(v))=ep->tv[0];
		v->tu2=ep->tv[0].tu+du;
		v->tv2=ep->tv[0].tv+dv;
		*((D3DTLVERTEX*)(v+1))=ep->tv[1];
		(v+1)->tu2=ep->tv[1].tu+du;
		(v+1)->tv2=ep->tv[1].tv+dv;
		*((D3DTLVERTEX*)(v+2))=ep->tv[2];
		(v+2)->tu2=ep->tv[2].tu+du;
		(v+2)->tv2=ep->tv[2].tv+dv;

		if(ep->type&POLY_QUAD)
		{
			*((D3DTLVERTEX*)(v+3))=ep->tv[3];
			(v+3)->tu2=ep->tv[3].tu+du;
			(v+3)->tv2=ep->tv[3].tv+dv;
		
				EERIEDRAWPRIM( pd3dDevice, D3DPT_TRIANGLESTRIP, D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX2, v, 4, 0, flg_NOCOUNT_USEVB );

		}
		else
		{
			
				EERIEDRAWPRIM( pd3dDevice, D3DPT_TRIANGLESTRIP, D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX2, v, 3, 0, flg_NOCOUNT_USEVB );
		}

		pd3dDevice->SetTextureStageState(1,D3DTSS_COLOROP,D3DTOP_DISABLE);
	}
	else
	{
		D3DTLVERTEX	v[4];
		pd3dDevice->SetTextureStageState(0,D3DTSS_COLORARG1,D3DTA_TEXTURE);
		pd3dDevice->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_SELECTARG1);
		pd3dDevice->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_DISABLE);
		
		memcpy(v,ep->tv,3*sizeof(D3DTLVERTEX));

		if(ep->type&POLY_QUAD)
		{
			v[3]=ep->tv[3];

				EERIEDRAWPRIM( pd3dDevice, D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX | D3DFVF_DIFFUSE, v, 4, 0, flg_NOCOUNT_USEVB );
			
			v[3].tu+=du;
			v[3].tv+=dv;
			nb=4;
		}
		else
		{
				EERIEDRAWPRIM( pd3dDevice, D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX | D3DFVF_DIFFUSE, ep->tv, 3, 0, flg_NOCOUNT_USEVB );
			nb=3;
		}

		v[0].tu+=du;
		v[0].tv+=dv;
		v[1].tu+=du;
		v[1].tv+=dv;
		v[2].tu+=du;
		v[2].tv+=dv;
		
		pd3dDevice->SetTextureStageState(0,D3DTSS_COLORARG1,D3DTA_TEXTURE|D3DTA_COMPLEMENT);
		pd3dDevice->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_SELECTARG1);
		pd3dDevice->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_DISABLE);
				
			EERIEDRAWPRIM( pd3dDevice, D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX|D3DFVF_DIFFUSE, v, nb, 0, flg_NOCOUNT_USEVB );
	}

	pd3dDevice->SetTextureStageState(0,D3DTSS_COLORARG1,D3DTA_TEXTURE);
	pd3dDevice->SetTextureStageState(0,D3DTSS_COLORARG2,D3DTA_DIFFUSE);
	pd3dDevice->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_MODULATE);
	pd3dDevice->SetTextureStageState(0,D3DTSS_ALPHAARG1,D3DTA_TEXTURE);
	pd3dDevice->SetTextureStageState(0,D3DTSS_ALPHAARG2,D3DTA_DIFFUSE);
	pd3dDevice->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_MODULATE);

	pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,D3DBLEND_ONE);
	pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND,D3DBLEND_ZERO);	
	SETALPHABLEND(pd3dDevice,FALSE); 
	SETZWRITE(pd3dDevice,TRUE); 
}

//*************************************************************************************
//*************************************************************************************

void EERIEDrawLine(float x,float y,float x1,float y1,float z,D3DCOLOR col)
{
	register D3DTLVERTEX tv[2];
	tv[0].sx=x;
	tv[0].sy=y;
	tv[1].sz=tv[0].sz=z;
	tv[1].color=tv[0].color=col;
	tv[1].rhw=tv[0].rhw=1.f;
	tv[1].sx=x1;
	tv[1].sy=y1;	
	SETTC(GDevice,NULL);
	EERIEDRAWPRIM(GDevice,D3DPT_LINELIST ,	D3DFVF_TLVERTEX,tv, 2,  0  );	

}

//*************************************************************************************
//*************************************************************************************

void EERIEDraw2DLine(LPDIRECT3DDEVICE7 pd3dDevice, float x0,float y0,float x1,float y1,float z, D3DCOLOR col)
{
	register D3DTLVERTEX v[2];

	SETTC(pd3dDevice,NULL);
	v[0].sx=x0;
	v[0].sy=y0;
	v[0].sz=v[1].sz=z;
	v[1].sx=x1;
	v[1].sy=y1;
	v[1].color=v[0].color=col;
	v[1].rhw=v[0].rhw=1.f;

	SETTC(pd3dDevice,NULL);
	EERIEDRAWPRIM(pd3dDevice, D3DPT_LINELIST, D3DFVF_TLVERTEX | D3DFVF_DIFFUSE, 
					 v, 2,  0  );	
}

void EERIEDraw2DRect(LPDIRECT3DDEVICE7 pd3dDevice, float x0,float y0,float x1,float y1,float z, D3DCOLOR col)
{
	register D3DTLVERTEX v[5];

	SETTC(pd3dDevice,NULL);
	v[4].sx=v[3].sx=v[0].sx=x0;
	v[4].sy=v[1].sy=v[0].sy=y0;
	v[2].sx=v[1].sx=x1;
	v[3].sy=v[2].sy=y1;
	v[4].sz=v[3].sz=v[2].sz=v[1].sz=v[0].sz=z;
	v[4].color=v[3].color=v[2].color=v[1].color=v[0].color=col;
	v[4].rhw=v[3].rhw=v[2].rhw=v[1].rhw=v[0].rhw=1.f;

	SETTC(pd3dDevice,NULL);
	EERIEDRAWPRIM(pd3dDevice,D3DPT_LINESTRIP, D3DFVF_TLVERTEX| D3DFVF_DIFFUSE, v, 5,  0  );	
}

void EERIEDrawFill2DRectDegrad(LPDIRECT3DDEVICE7 pd3dDevice, float x0,float y0,float x1,float y1,float z, D3DCOLOR cold, D3DCOLOR cole)
{
	D3DTLVERTEX v[4];

	SETTC(pd3dDevice,NULL);
	v[0].sx=v[2].sx=x0;
	v[0].sy=v[1].sy=y0;
	v[1].sx=v[3].sx=x1;
	v[2].sy=v[3].sy=y1;
	v[0].color=v[1].color=cold;
	v[2].color=v[3].color=cole;
	v[0].sz=v[1].sz=v[2].sz=v[3].sz=z;
	v[3].rhw=v[2].rhw=v[1].rhw=v[0].rhw=1.f;
	SETTC(pd3dDevice,NULL);
	EERIEDRAWPRIM(pd3dDevice,D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX| D3DFVF_DIFFUSE, v, 4,  0  );	
}

void EERIEDraw3DCylinder(LPDIRECT3DDEVICE7 pd3dDevice, EERIE_CYLINDER * cyl, D3DCOLOR col)
{
	register EERIE_3D from,to;
	#define STEPCYL 16

	for (long i=0;i<360-STEPCYL;i+=STEPCYL)
	{
		float es =EEsin(DEG2RAD(MAKEANGLE((float)i)))*cyl->radius;
		float ec =EEcos(DEG2RAD(MAKEANGLE((float)i)))*cyl->radius;
		float es2=EEsin(DEG2RAD(MAKEANGLE((float)(i+STEPCYL))))*cyl->radius;
		float ec2=EEcos(DEG2RAD(MAKEANGLE((float)(i+STEPCYL))))*cyl->radius;		
		
		// Draw low pos
		from.x=cyl->origin.x+es;
		from.y=cyl->origin.y;
		from.z=cyl->origin.z+ec;
		to.x=cyl->origin.x+es2;
		to.y=cyl->origin.y;
		to.z=cyl->origin.z+ec2;
		EERIEDraw3DLine(pd3dDevice,&from, &to,  col);
		// Draw vertical 
		from.x=cyl->origin.x+es;
		from.y=cyl->origin.y;
		from.z=cyl->origin.z+ec;
		to.x=from.x;
		to.y=from.y+cyl->height;
		to.z=from.z;
		EERIEDraw3DLine(pd3dDevice,&from, &to,  col);
		// Draw high pos
		from.x=cyl->origin.x+es;
		from.y=cyl->origin.y+cyl->height;
		from.z=cyl->origin.z+ec;
		to.x=cyl->origin.x+es2;
		to.y=from.y;
		to.z=cyl->origin.z+ec2;
		EERIEDraw3DLine(pd3dDevice,&from, &to,  col);
	}
}

void EERIEDraw3DCylinderBase(LPDIRECT3DDEVICE7 pd3dDevice, EERIE_CYLINDER * cyl, D3DCOLOR col)
{
	register EERIE_3D from,to;
	#define STEPCYL 16

	for (long i=0;i<360-STEPCYL;i+=STEPCYL)
	{
		float es =EEsin(DEG2RAD(MAKEANGLE((float)i)))*cyl->radius;
		float ec =EEcos(DEG2RAD(MAKEANGLE((float)i)))*cyl->radius;
		float es2=EEsin(DEG2RAD(MAKEANGLE((float)(i+STEPCYL))))*cyl->radius;
		float ec2=EEcos(DEG2RAD(MAKEANGLE((float)(i+STEPCYL))))*cyl->radius;		
		
		// Draw low pos
		from.x=cyl->origin.x+es;
		from.y=cyl->origin.y;
		from.z=cyl->origin.z+ec;
		to.x=cyl->origin.x+es2;
		to.y=cyl->origin.y;
		to.z=cyl->origin.z+ec2;
		EERIEDraw3DLine(pd3dDevice,&from, &to,  col);	
	}
}

void EERIEDrawCircle(float x0,float y0,float r,D3DCOLOR col,float z)
{
	register float x,y;
	register float lx=x0;
	register float ly=y0+r;
	register float t;
	SETTC(GDevice,NULL);	

	for (long i=0;i<361;i+=10)
	{
		t=DEG2RAD((float)i);
		x=x0-EEsin(t)*r;
		y=y0+EEcos(t)*r;
		EERIEDrawLine(lx,ly,x,y,z,col);
		lx = x;
		ly = y;
		}
}

//*************************************************************************************
//*************************************************************************************

void EERIEDrawTrue3DLine(LPDIRECT3DDEVICE7 pd3dDevice,EERIE_3D * orgn, EERIE_3D * dest, D3DCOLOR col)
{
	EERIE_3D vect;
	vect.x=dest->x-orgn->x;
	vect.y=dest->y-orgn->y;
	vect.z=dest->z-orgn->z;
	float m=Vector_Magnitude(&vect);

	if (m<=0) return;

	float om=1.f/m;
	vect.x*=om;
	vect.y*=om;
	vect.z*=om;
	EERIE_3D cpos;
	cpos.x=orgn->x;
	cpos.y=orgn->y;
	cpos.z=orgn->z;

	while (m>0)
	{
		EERIE_3D tpos;
		float dep=__min(m,30.f);
		tpos.x=cpos.x+vect.x*dep;
		tpos.y=cpos.y+vect.y*dep;
		tpos.z=cpos.z+vect.z*dep;
		EERIEDraw3DLine(pd3dDevice, &cpos,&tpos,col);
		Vector_Copy(&cpos,&tpos);
		m-=dep;
	}
}
//*************************************************************************************
//*************************************************************************************

void EERIEDraw3DLine(LPDIRECT3DDEVICE7 pd3dDevice,EERIE_3D * orgn, EERIE_3D * dest, D3DCOLOR col)
{
	D3DTLVERTEX v[2];
	D3DTLVERTEX in;

	in.sx=orgn->x;
	in.sy=orgn->y;
	in.sz=orgn->z;

	EE_RTP(&in,&v[0]);

	if (v[0].sz<0.f) return;

	in.sx=dest->x;
	in.sy=dest->y;
	in.sz=dest->z;

	EE_RTP(&in,&v[1]);

	if (v[1].sz<0.f) return;

	SETTC(pd3dDevice,NULL);
	v[1].color=v[0].color=col;
	
	EERIEDRAWPRIM(pd3dDevice,D3DPT_LINELIST, D3DFVF_TLVERTEX| D3DFVF_DIFFUSE,v, 2,  0  );	
}
#define BASICFOCAL 350.f
//*************************************************************************************
//*************************************************************************************

void EERIEDrawSprite(LPDIRECT3DDEVICE7 pd3dDevice,D3DTLVERTEX *in,float siz,TextureContainer * tex,D3DCOLOR col,float Zpos)
{
	register D3DTLVERTEX out;
	
	EERIETreatPoint2(in,&out);

	if ((out.sz>0.f) && (out.sz<1000.f)
		&& (out.sx>-1000) && (out.sx<2500.f)
		&& (out.sy>-500) && (out.sy<1800.f))
	{
		float use_focal=BASICFOCAL*Xratio;
		float t;

		if (siz < 0)
		{
			t=-siz;
		}
		else
		{
			t=siz*((out.rhw-1.f)*use_focal*0.001f);

			if (t<=0.f) t=0.00000001f;
		}
		
		if (Zpos<=1.f)
		{
			out.sz = Zpos;
			out.rhw=1.f-out.sz;
		}
		else
		{
			out.rhw*=(1.f/3000.f);
		}

		ComputeSingleFogVertex(&out);

		SPRmaxs.x=out.sx+t;
		SPRmins.x=out.sx-t;
		SPRmaxs.y=out.sy+t;			
		SPRmins.y=out.sy-t;

		D3DTLVERTEX v[4];
		v[0]= D3DTLVERTEX( D3DVECTOR( SPRmins.x, SPRmins.y, out.sz), out.rhw, col, out.specular, 0.f, 0.f);
		v[1]= D3DTLVERTEX( D3DVECTOR( SPRmaxs.x, SPRmins.y, out.sz), out.rhw, col, out.specular, 1.f, 0.f);
		v[2]= D3DTLVERTEX( D3DVECTOR( SPRmins.x, SPRmaxs.y, out.sz), out.rhw, col, out.specular, 0.f, 1.f);
		v[3]= D3DTLVERTEX( D3DVECTOR( SPRmaxs.x, SPRmaxs.y, out.sz), out.rhw, col, out.specular, 1.f, 1.f);

		SETTC(pd3dDevice,tex);
		EERIEDRAWPRIM(pd3dDevice,D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX| D3DFVF_DIFFUSE , v, 4,  0  );		
	}
	else SPRmaxs.x=-1;
}

//*************************************************************************************
//*************************************************************************************

void EERIEDrawRotatedSprite(LPDIRECT3DDEVICE7 pd3dDevice,D3DTLVERTEX *in,float siz,TextureContainer * tex,D3DCOLOR col,float Zpos,float rot)
{
	register D3DTLVERTEX out;
	register float tt;
		
	EERIETreatPoint2(in, &out);
	
	if ((out.sz>0.f) && (out.sz<1000.f))
	{
		float use_focal=BASICFOCAL*Xratio;
	
		float t = siz * ((out.rhw - 1.f) * use_focal * 0.001f); 

		if (t<=0.f) t=0.00000001f;

		if (Zpos<=1.f)
		{
			out.sz = Zpos; 
			out.rhw=1.f-out.sz;
		}
		else
		{
			out.rhw*=(1.f/3000.f);
		}

		ComputeSingleFogVertex(&out);

		D3DTLVERTEX v[4];
		v[0]= D3DTLVERTEX( D3DVECTOR( 0, 0, out.sz ), out.rhw, col, out.specular, 0.f, 0.f);
		v[1]= D3DTLVERTEX( D3DVECTOR( 0, 0, out.sz ), out.rhw, col, out.specular, 1.f, 0.f);
		v[2]= D3DTLVERTEX( D3DVECTOR( 0, 0, out.sz ), out.rhw, col, out.specular, 1.f, 1.f);
		v[3]= D3DTLVERTEX( D3DVECTOR( 0, 0, out.sz ), out.rhw, col, out.specular, 0.f, 1.f);
		
		
		SPRmaxs.x=out.sx+t;
		SPRmins.x=out.sx-t;
		
		SPRmaxs.y=out.sy+t;			
		SPRmins.y=out.sy-t;

		SPRmaxs.z = SPRmins.z = out.sz; 

		for (long i=0;i<4;i++)
		{
			tt=DEG2RAD(MAKEANGLE(rot+90.f*i+45+90));
			v[i].sx=EEsin(tt)*t+out.sx;
			v[i].sy=EEcos(tt)*t+out.sy;
		}		

		SETTC(pd3dDevice,tex);
		EERIEDRAWPRIM(pd3dDevice, D3DPT_TRIANGLEFAN, D3DFVF_TLVERTEX | D3DFVF_DIFFUSE , 
				 v, 4,  0  );		
	}
	else SPRmaxs.x=-1;
}

//*************************************************************************************
//*************************************************************************************

void SETTEXTUREWRAPMODE(LPDIRECT3DDEVICE7 pd3dDevice,DWORD mode)
{
	pd3dDevice->SetTextureStageState( 0,D3DTSS_ADDRESS ,mode);
}

//*************************************************************************************
//*************************************************************************************

void SETCULL(LPDIRECT3DDEVICE7 pd3dDevice,DWORD state)
{
	pd3dDevice->SetRenderState( D3DRENDERSTATE_CULLMODE , state);
}

//*************************************************************************************

void SETZWRITE(LPDIRECT3DDEVICE7 pd3dDevice,DWORD _dwState)
{
		pd3dDevice->SetRenderState( D3DRENDERSTATE_ZWRITEENABLE , _dwState);
}	

//*************************************************************************************
//*************************************************************************************

void SETALPHABLEND(LPDIRECT3DDEVICE7 pd3dDevice,DWORD state)
{
	pd3dDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, state);
}	

void SETBLENDMODE(LPDIRECT3DDEVICE7 pd3dDevice,DWORD srcblend,DWORD destblend)
{
	pd3dDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND,  srcblend  );
	pd3dDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, destblend );
	
}

void EERIEPOLY_DrawWired(LPDIRECT3DDEVICE7 pd3dDevice, EERIEPOLY *ep,long col)
{
	D3DTLVERTEX ltv[5];
	ltv[0]= D3DTLVERTEX( D3DVECTOR( 0, 0, 0.5 ), 1.f, 1, 1, 0.f, 0.f);
	ltv[1]= D3DTLVERTEX( D3DVECTOR( 0, 0, 0.5 ), 1.f, 1, 1, 1.f, 0.f);
	ltv[2]= D3DTLVERTEX( D3DVECTOR( 0, 0, 0.5 ), 1.f, 1, 1, 1.f, 1.f);
	ltv[3]= D3DTLVERTEX( D3DVECTOR( 0, 0, 0.5 ), 1.f, 1, 1, 0.f, 1.f);
	ltv[4]= D3DTLVERTEX( D3DVECTOR( 0, 0, 0.5 ), 1.f, 1, 1, 0.f, 1.f);
	
	long to;

	if (ep->type & POLY_QUAD) to=4;
	else to=3;

	memcpy(ltv,ep->tv,sizeof(D3DTLVERTEX)*to);							
	ltv[0].sz-=0.0002f;
	ltv[1].sz-=0.0002f;
	ltv[2].sz-=0.0002f;
	ltv[3].sz-=0.0002f;

	if (to==4) 
	{
		memcpy(&ltv[2],&ep->tv[3],sizeof(D3DTLVERTEX));
		memcpy(&ltv[3],&ep->tv[2],sizeof(D3DTLVERTEX));
		memcpy(&ltv[4],&ep->tv[0],sizeof(D3DTLVERTEX));
		ltv[4].sz-=0.0002f;
	}
	else memcpy(&ltv[to],&ltv[0],sizeof(D3DTLVERTEX));

	SETTC(pd3dDevice, NULL);

	if (col)
	 ltv[0].color=ltv[1].color=ltv[2].color=ltv[3].color=ltv[4].color=col;
	else if (to==4)
			ltv[0].color=ltv[1].color=ltv[2].color=ltv[3].color=ltv[4].color=0xFF00FF00;
	else ltv[0].color=ltv[1].color=ltv[2].color=ltv[3].color=0xFFFFFF00;
							
	EERIEDRAWPRIM(pd3dDevice,D3DPT_LINESTRIP, D3DFVF_TLVERTEX| D3DFVF_DIFFUSE, ltv, to+1,  0  );
}
					
void EERIEPOLY_DrawNormals(LPDIRECT3DDEVICE7 pd3dDevice, EERIEPOLY *ep)
{
	D3DTLVERTEX ltv[5];
	ltv[0]= D3DTLVERTEX( D3DVECTOR( 0, 0, 0.5 ), 1.f, 1, 1, 0.f, 0.f);
	ltv[1]= D3DTLVERTEX( D3DVECTOR( 0, 0, 0.5 ), 1.f, 1, 1, 1.f, 0.f);
	ltv[2]= D3DTLVERTEX( D3DVECTOR( 0, 0, 0.5 ), 1.f, 1, 1, 1.f, 1.f);
	ltv[3]= D3DTLVERTEX( D3DVECTOR( 0, 0, 0.5 ), 1.f, 1, 1, 0.f, 1.f);
	ltv[4]= D3DTLVERTEX( D3DVECTOR( 0, 0, 0.5 ), 1.f, 1, 1, 0.f, 1.f);
	
	D3DTLVERTEX lv;
	long to;

	if (ep->type & POLY_QUAD) to=4;
	else to=3;

	lv.sx=ep->center.x;
	lv.sy=ep->center.y;
	lv.sz=ep->center.z;
	EE_RTP(&lv,&ltv[0]);
	lv.sx+=ep->norm.x*10.f;
	lv.sy+=ep->norm.y*10.f;
	lv.sz+=ep->norm.z*10.f;
	EE_RTP(&lv,&ltv[1]);					
	SETTC(pd3dDevice,NULL);
	ltv[1].color=ltv[0].color=0xFFFF0000;

	if ((ltv[1].sz>0.f) && (ltv[0].sz>0.f))
		EERIEDRAWPRIM(pd3dDevice,D3DPT_LINELIST, D3DFVF_TLVERTEX| D3DFVF_DIFFUSE, ltv, 3,  0  );								

	for (long h=0;h<to;h++)
	{
		lv.sx=ep->v[h].sx;
		lv.sy=ep->v[h].sy;
		lv.sz=ep->v[h].sz;
		EE_RTP(&lv,&ltv[0]);
		lv.sx+=ep->nrml[h].x*10.f;
		lv.sy+=ep->nrml[h].y*10.f;
		lv.sz+=ep->nrml[h].z*10.f;
		EE_RTP(&lv,&ltv[1]);					
		SETTC(pd3dDevice,NULL);
		ltv[1].color=ltv[0].color=EERIECOLOR_YELLOW;

		if ((ltv[1].sz>0.f) &&  (ltv[0].sz>0.f))
			EERIEDRAWPRIM(pd3dDevice,D3DPT_LINELIST, D3DFVF_TLVERTEX| D3DFVF_DIFFUSE, ltv, 3,  0  );									
	}
}


//-----------------------------------------------------------------------------

void EERIEDrawBitmap(LPDIRECT3DDEVICE7 pd3dDevice,float x,float y,float sx,float sy,float z,TextureContainer * tex,D3DCOLOR col)
{
	register float smu,smv;
	float fEndu,fEndv;

	if (tex)
	{
		smu=tex->m_hdx;
		smv=tex->m_hdy;
		fEndu=tex->m_dx;
		fEndv=tex->m_dy;
	}
	else
	{
		smu=smv=0.f;
		fEndu=fEndv=0.f;
	}

	D3DTLVERTEX v[4];
	v[0]= D3DTLVERTEX( D3DVECTOR( x,	y,		z ), 1.f, col, 0xFF000000, smu,		smv);
	v[1]= D3DTLVERTEX( D3DVECTOR( x+sx, y,		z ), 1.f, col, 0xFF000000, fEndu,	smv);
	v[2]= D3DTLVERTEX( D3DVECTOR( x,	y+sy,	z ), 1.f, col, 0xFF000000, smu,		fEndv);
	v[3]= D3DTLVERTEX( D3DVECTOR( x+sx,	y+sy,	z ), 1.f, col, 0xFF000000, fEndu,	fEndv);
	
	SETTC(pd3dDevice,tex);
	EERIEDRAWPRIM(pd3dDevice,D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX| D3DFVF_DIFFUSE, v, 4, 0  );	
}

void EERIEDrawBitmap_uv(LPDIRECT3DDEVICE7 pd3dDevice,float x,float y,float sx,float sy,float z,TextureContainer * tex,D3DCOLOR col,float u0,float v0,float u1,float v1)
{
	float smu,smv;
	float fEndu,fEndv;
	float fDecalU,fDecalV;

	if (tex)
	{
		smu=tex->m_hdx;
		smv=tex->m_hdy;
		fEndu=tex->m_dx;
		fEndv=tex->m_dy;
		fDecalU=tex->m_hdx;
		fDecalV=tex->m_hdy;
	}
	else
	{
		smu=smv=0.f;
		fEndu=fEndv=1.f;
		fDecalU=fDecalV=0.f;
	}

	D3DTLVERTEX v[4];
	u0=smu+(fEndu-smu+fDecalU)*u0;
	u1=smu+(fEndu-smu+fDecalU)*u1;
	v0=smv+(fEndv-smv+fDecalV)*v0;
	v1=smv+(fEndv-smv+fDecalV)*v1;
	v[0]= D3DTLVERTEX( D3DVECTOR( x, y, z ), 1.f, col, 0xFF000000, u0, v0);
	v[1]= D3DTLVERTEX( D3DVECTOR( x+sx, y, z ), 1.f, col, 0xFF000000, u1, v0);
	v[2]= D3DTLVERTEX( D3DVECTOR( x+sx, y+sy, z ), 1.f, col, 0xFF000000, u1, v1);
	v[3]= D3DTLVERTEX( D3DVECTOR( x, y+sy, z ), 1.f, col, 0xFF000000, u0, v1);

	SETTC(pd3dDevice,tex);
	EERIEDRAWPRIM(pd3dDevice,D3DPT_TRIANGLEFAN, D3DFVF_TLVERTEX| D3DFVF_DIFFUSE, v, 4, 0  );	
}

void EERIEDrawBitmapUVs(LPDIRECT3DDEVICE7 pd3dDevice,float x,float y,float sx,float sy,float z,TextureContainer * tex,D3DCOLOR col
						,float u0,float v0
						,float u1,float v1
						,float u2,float v2
						,float u3,float v3
						)
{
	register float smu,smv;
	float fEndu,fEndv;

	if (tex)
	{
		smu=tex->m_hdx;
		smv=tex->m_hdy;
		fEndu=tex->m_dx;
		fEndv=tex->m_dy;
	}
	else
	{
		smu=smv=0.f;
		fEndu=fEndv=0.f;
	}

	D3DTLVERTEX v[4];
	v[0]= D3DTLVERTEX( D3DVECTOR( x,	y,		z ), 1.f, col, 0xFF000000, smu+u0,	smv+v0);
	v[1]= D3DTLVERTEX( D3DVECTOR( x+sx, y,		z ), 1.f, col, 0xFF000000, smu+u1,	smv+v1);
	v[2]= D3DTLVERTEX( D3DVECTOR( x,	y+sy,	z ), 1.f, col, 0xFF000000, smu+u2,	smv+v2);
	v[3]= D3DTLVERTEX( D3DVECTOR( x+sx,	y+sy,	z ), 1.f, col, 0xFF000000, smu+u3,	smv+v3);
	
	SETTC(pd3dDevice,tex);
	EERIEDRAWPRIM(pd3dDevice,D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX| D3DFVF_DIFFUSE, v, 4, 0  );	
}

//-----------------------------------------------------------------------------

void EERIEDrawBitmap2(LPDIRECT3DDEVICE7 pd3dDevice,float x,float y,float sx,float sy,float z,TextureContainer * tex,D3DCOLOR col)
{
	float smu,smv;
	float fEndu,fEndv;

	if (tex)
	{
		smu=tex->m_hdx;
		smv=tex->m_hdy;
		fEndu=tex->m_dx;
		fEndv=tex->m_dy;
	}
	else
	{
		smu=smv=0.f;
		fEndu=fEndv=0.f;
	}

	D3DTLVERTEX v[4];

	float fZMinus=1.f-z;
	v[0]= D3DTLVERTEX( D3DVECTOR( x, y, z ), fZMinus, col, 0xFF000000, smu, smv);
	v[1]= D3DTLVERTEX( D3DVECTOR( x+sx, y, z ), fZMinus, col, 0xFF000000, fEndu, smv);
	v[2]= D3DTLVERTEX( D3DVECTOR( x, y+sy, z ), fZMinus, col, 0xFF000000, smu, fEndv);
	v[3]= D3DTLVERTEX( D3DVECTOR( x+sx, y+sy, z ), fZMinus, col, 0xFF000000, fEndu, fEndv);

	SETTC(pd3dDevice,tex);
	EERIEDRAWPRIM(pd3dDevice,D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX| D3DFVF_DIFFUSE, v, 4, 0  );	
}

//-----------------------------------------------------------------------------

void EERIEDrawBitmap2DecalY(LPDIRECT3DDEVICE7 pd3dDevice,float x,float y,float sx,float sy,float z,TextureContainer * tex,D3DCOLOR col,float _fDeltaY)
{
	float smu,smv;
	float fDepv;
	float fEndu,fEndv;

	if (tex)
	{
		smu=tex->m_hdx;
		smv=tex->m_hdy;
		fEndu=tex->m_dx;
		fEndv=tex->m_dy;
		fDepv=tex->m_hdy+(tex->m_dy-tex->m_hdy)*_fDeltaY;
	}
	else
	{
		smu=smv=0.f;
		fDepv=0.f;
		fEndu=fEndv=0.f;
	}

	D3DTLVERTEX v[4];
	float fDy=_fDeltaY*sy;

	if(sx<0)
	{
		sx=-sx;
		v[0]= D3DTLVERTEX( D3DVECTOR( x, y+fDy, z ), 1.f, col, 0xFF000000, fEndu, fDepv);
		v[1]= D3DTLVERTEX( D3DVECTOR( x+sx, y+fDy, z ), 1.f, col, 0xFF000000,smu, fDepv);
		v[2]= D3DTLVERTEX( D3DVECTOR( x+sx, y+sy, z ), 1.f, col, 0xFF000000, smu, fEndv);
		v[3]= D3DTLVERTEX( D3DVECTOR( x, y+sy, z ), 1.f, col, 0xFF000000, fEndu, fEndv);
	}
	else
	{
		v[0]= D3DTLVERTEX( D3DVECTOR( x, y+fDy, z ), 1.f, col, 0xFF000000, smu, fDepv);
		v[1]= D3DTLVERTEX( D3DVECTOR( x+sx, y+fDy, z ), 1.f, col, 0xFF000000, fEndu, fDepv);
		v[2]= D3DTLVERTEX( D3DVECTOR( x+sx, y+sy, z ), 1.f, col, 0xFF000000, fEndu, fEndv);
		v[3]= D3DTLVERTEX( D3DVECTOR( x, y+sy, z ), 1.f, col, 0xFF000000, smu, fEndv);
	}

	SETTC(pd3dDevice,tex);
	EERIEDRAWPRIM(pd3dDevice,D3DPT_TRIANGLEFAN, D3DFVF_TLVERTEX| D3DFVF_DIFFUSE, v, 4, 0  );	
}
