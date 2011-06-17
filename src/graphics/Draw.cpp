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

#include "graphics/Draw.h"

#include "core/Application.h"

#include "graphics/VertexBuffer.h"
#include "graphics/GraphicsEnum.h"
#include "graphics/data/Mesh.h"

using std::min;
using std::max;

TextureContainer * EERIE_DRAW_sphere_particle=NULL;
TextureContainer * EERIE_DRAW_square_particle=NULL;

long ZMAPMODE=1;
TextureContainer * Zmap;
Vec3f SPRmins;
Vec3f SPRmaxs;

extern long REFLECTFX;
extern long WATERFX;
extern TextureContainer * enviro;
extern float FrameTime;

void CopyVertices(EERIEPOLY * ep,long to, long from) {
	ep->v[to] = ep->v[from];
	ep->tv[to] = ep->tv[from];
	ep->nrml[to] = ep->nrml[from];
}
 
bool NearlyEqual(float a,float b)
{
	if (EEfabs(a-b)<0.01f) return true;

	if (EEfabs(b-a)<0.01f) return true;

	return false;
}

bool Quadable(EERIEPOLY * ep, EERIEPOLY * ep2, float tolerance)
{
	
	long count=0;
	long common=-1;
	long common2=-1;
	
	
	long ep_notcommon=-1;
	long ep2_notcommon=-1;
	
	if (ep2->type & POLY_QUAD)
					{
	return false;
}
	
	if (ep->tex != ep2->tex) return false;

	long typ1=ep->type&(~POLY_QUAD);
	long typ2=ep2->type&(~POLY_QUAD);

	if (typ1!=typ2) return false;

	if ((ep->type & POLY_TRANS) && (ep->transval!=ep2->transval)) return false;

	CalcFaceNormal(ep,ep->v);

	if (EEfabs(ep->norm dot ep2->norm) < 1.f - tolerance) return false;
	
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
		ep2->tv[3].color=ep2->v[3].color=Color::white.toBGR();
		ep2->tv[3].rhw=ep2->v[3].rhw=1.f;

	DeclareEGInfo(ep->v[3].sx, ep->v[3].sz);

	ep2->center.x=(ep2->v[0].sx+ep2->v[1].sx+ep2->v[2].sx+ep2->v[3].sx)*( 1.0f / 4 );
	ep2->center.y=(ep2->v[0].sy+ep2->v[1].sy+ep2->v[2].sy+ep2->v[3].sy)*( 1.0f / 4 );
	ep2->center.z=(ep2->v[0].sz+ep2->v[1].sz+ep2->v[2].sz+ep2->v[3].sz)*( 1.0f / 4 );
	ep2->max.x=max(ep2->max.x,ep2->v[3].sx);
	ep2->min.x=min(ep2->min.x,ep2->v[3].sx);
	ep2->max.y=max(ep2->max.y,ep2->v[3].sy);
	ep2->min.y=min(ep2->min.y,ep2->v[3].sy);
	ep2->max.z=max(ep2->max.z,ep2->v[3].sz);
	ep2->min.z=min(ep2->min.z,ep2->v[3].sz);

	ep2->norm2.x=ep->norm.x;
	ep2->norm2.y=ep->norm.y;
	ep2->norm2.z=ep->norm.z;
	
	ep2->area+=Distance3D(	(ep2->v[1].sx+ep2->v[2].sx)*( 1.0f / 2 ),
							(ep2->v[1].sy+ep2->v[2].sy)*( 1.0f / 2 ),
							(ep2->v[1].sz+ep2->v[2].sz)*( 1.0f / 2 ),
							ep2->v[3].sx,ep2->v[3].sy,ep2->v[3].sz)
			*Distance3D(	ep2->v[3].sx,ep2->v[3].sy,ep2->v[3].sz,
							ep2->v[1].sx,ep2->v[1].sy,ep2->v[1].sz)*( 1.0f / 2 );

		return true;
	}

	return false;
}

extern long COMPUTE_PORTALS;
#define TYPE_ROOM	2
bool TryToQuadify(EERIEPOLY * ep,EERIE_3DOBJ * eobj)
{
	long posx,posz;
	float cx,cz;
	EERIE_BKG_INFO * eg;
	cx=(ep->v[0].sx+ep->v[1].sx+ep->v[2].sx);
	cz=(ep->v[0].sz+ep->v[1].sz+ep->v[2].sz);
	posx = cx*( 1.0f / 3 )*ACTIVEBKG->Xmul;
	posz = cz*( 1.0f / 3 )*ACTIVEBKG->Zmul;
	
	long dx,dz,fx,fz;
	dx=std::max(0L,posx-1);
	fx=std::min(posx+1,ACTIVEBKG->Xsize-1L);
	dz=std::max(0L,posz-1);
	fz=std::min(posz+1,ACTIVEBKG->Zsize-1L);
	float tolerance=0.1f;

	for (long kl = 0; kl < 2; kl++)
	for (long zz=dz;zz<=fz;zz++)
	for (long xx=dx;xx<=fx;xx++)
	{
		long val1;

		if (COMPUTE_PORTALS)
		{
			long type, val2;
			if (!GetNameInfo(eobj->name, type, val1, val2))
				return false;

			if (type!=TYPE_ROOM)
				return false;
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
				if (Quadable(ep,ep2,tolerance)) return true;
			}
			else if ((kl==1) && !(ep2->type & POLY_QUAD) )
			{
				if (Quadable(ep,ep2,tolerance)) return true;
			}
		}
	}

	return false;
}

void EERIE_DRAW_SetTextureZMAP(TextureContainer * Z_map)
{
	Zmap=Z_map;
}

CircularVertexBuffer<D3DTLVERTEX> * pDynamicVertexBuffer_TLVERTEX;

void EERIEDRAWPRIM(Renderer::Primitive primitive, const D3DTLVERTEX * vertices, size_t count, bool nocount) {
	
	if(!nocount) {
		EERIEDrawnPolys++;
	}
	
	pDynamicVertexBuffer_TLVERTEX->draw(primitive, vertices, count);
}

void Delayed_FlushAll() {

	TextureContainer* ptcTexture = GetTextureList();

    while( ptcTexture )
    {
		if ((ptcTexture->delayed_nb) && ptcTexture->delayed)
		{
			long to;

			if (ViewMode & VIEWMODE_FLAT)
				GRenderer->ResetTexture(0);
			else 
				GRenderer->SetTexture(0, ptcTexture);

			DELAYED_PRIM * del=(DELAYED_PRIM *)ptcTexture->delayed;

			for (long i=0;i<ptcTexture->delayed_nb;i++)
			{				
				EERIEPOLY * ep=del[i].data;

				if (!(ep->type & POLY_DOUBLESIDED))
					GRenderer->SetCulling(Renderer::CullCW);
				else 
					GRenderer->SetCulling(Renderer::CullNone);
				
				if (ep->type & POLY_QUAD)
					to=4;
				else to=3;

				EERIEDRAWPRIM(Renderer::TriangleStrip, ep->tv, to, true);
					
				if ( ptcTexture->userflags & POLY_METAL)
				{
					GRenderer->SetBlendFunc(Renderer::BlendDstColor, Renderer::BlendOne);
					GRenderer->SetRenderState(Renderer::AlphaBlending, true);
					GRenderer->ResetTexture(0); 

					EERIEDRAWPRIM(Renderer::TriangleStrip, ep->tv, to);
				}

				if ( (ep->type & POLY_LAVA) || (ep->type & POLY_WATER) )
				{
					GRenderer->SetBlendFunc(Renderer::BlendDstColor, Renderer::BlendOne);	
					GRenderer->SetRenderState(Renderer::AlphaBlending, true);	
					D3DTLVERTEX verts[4];
					GRenderer->SetTexture(0, enviro);

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
							verts[i].tu=ep->v[i].sx*( 1.0f / 1000 )+EEsin((ep->v[i].sx)*( 1.0f / 200 )+(float)FrameTime*( 1.0f / 2000 ))*( 1.0f / 20 );
							verts[i].tv=ep->v[i].sz*( 1.0f / 1000 )+EEcos((ep->v[i].sz)*( 1.0f / 200 )+(float)FrameTime*( 1.0f / 2000 ))*( 1.0f / 20 );
						}
						else
						{
							GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
							verts[i].tu=ep->v[i].sx*( 1.0f / 800 )+EEsin((ep->v[i].sx)*( 1.0f / 600 )+(float)FrameTime*( 1.0f / 1000 ))*( 1.0f / 9 );
							verts[i].tv=ep->v[i].sz*( 1.0f / 800 )+EEcos((ep->v[i].sz)*( 1.0f / 600 )+(float)FrameTime*( 1.0f / 1000 ))*( 1.0f / 9 );

							if (ep->type & POLY_FALL) verts[i].tv-=(float)(FrameTime)*( 1.0f / 200 );
						}
					}

					EERIEDRAWPRIM(Renderer::TriangleStrip, verts, to, true);

					if (ep->type & POLY_WATER)
					{
						for (long i=0;i<to;i++)
						{
							verts[i].tu=ep->v[i].sx*( 1.0f / 1000 )+EEsin((ep->v[i].sy)*( 1.0f / 200 )+(float)FrameTime*( 1.0f / 600 )*( 1.0f / 3 ))*( 1.0f / 10 );
							verts[i].tv=ep->v[i].sz*( 1.0f / 1000 )+EEcos((ep->v[i].sz+ep->v[i].sx)*( 1.0f / 200 )+(float)FrameTime*( 1.0f / 600 )*( 1.0f / 3 ))*( 1.0f / 10 );

							if (ep->type & POLY_FALL) 
							{
								verts[i].tv-=(float)(FrameTime)*( 1.0f / 200 );
							}
						}	

						EERIEDRAWPRIM(Renderer::TriangleStrip, verts, to);
					}

					if (ep->type & POLY_LAVA)
					{
						for (long i=0;i<to;i++)
						{
							verts[i].tu=ep->v[i].sx*( 1.0f / 1000 )+EEsin((ep->v[i].sx)*( 1.0f / 100 )+(float)FrameTime*( 1.0f / 2000 ))*( 1.0f / 10 );
							verts[i].tv=ep->v[i].sz*( 1.0f / 1000 )+EEcos((ep->v[i].sz)*( 1.0f / 100 )+(float)FrameTime*( 1.0f / 2000 ))*( 1.0f / 10 );
						}	
						EERIEDRAWPRIM(Renderer::TriangleStrip, verts, to);
						for ( int i=0;i<to;i++)
						{
							verts[i].tu=ep->v[i].sx*( 1.0f / 600 )+EEsin((ep->v[i].sx)*( 1.0f / 160 )+(float)FrameTime*( 1.0f / 2000 ))*( 1.0f / 11 );
							verts[i].tv=ep->v[i].sz*( 1.0f / 600 )+EEcos((ep->v[i].sz)*( 1.0f / 160 )+(float)FrameTime*( 1.0f / 2000 ))*( 1.0f / 11 );
							verts[i].color=0xFF666666;
						}	

						GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);
						EERIEDRAWPRIM(Renderer::TriangleStrip, verts, to);
					}
				}
					
				GRenderer->SetTexture(0, ptcTexture);				
				GRenderer->SetRenderState(Renderer::AlphaBlending, false);	
			}				
			
			if (ZMAPMODE)
			{				
				if (ptcTexture->TextureRefinement==NULL) 
					ptcTexture->delayed_nb=0;
				
				if (ptcTexture->delayed_nb)
				{
					GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);	
					GRenderer->SetRenderState(Renderer::DepthWrite, false); 
					GRenderer->SetRenderState(Renderer::AlphaBlending, true);					
					D3DTLVERTEX verts[4];
					GRenderer->SetTexture(0, ptcTexture->TextureRefinement); 

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
							GRenderer->SetCulling(Renderer::CullCW);			
						else GRenderer->SetCulling(Renderer::CullNone);
						
						
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
									tmp = val*10200;
									verts[j].color=0xFF000000 | (tmp<<16) | (tmp<<8) | tmp;
								}			
							}				
						}

						EERIEDRAWPRIM(Renderer::TriangleStrip, verts, to, true);
					}
				}

				EERIEDrawnPolys+=ptcTexture->delayed_nb;
				GRenderer->SetRenderState(Renderer::AlphaBlending, false); 
				GRenderer->SetRenderState(Renderer::DepthWrite, true);
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
		tc->delayed=(DELAYED_PRIM *)realloc(tc->delayed,sizeof(DELAYED_PRIM)*(tc->delayed_nb+1));
		tc->delayed_max=tc->delayed_nb+1;
	}

	DELAYED_PRIM * del=(DELAYED_PRIM *)tc->delayed;
	del[tc->delayed_nb].data=ep;
	tc->delayed_nb++;
}

void EERIEDraw2DLine(float x0, float y0, float x1, float y1, float z, Color col) {
	
	D3DTLVERTEX v[2];
	v[0].sx = x0;
	v[0].sy = y0;
	v[0].sz = v[1].sz = z;
	v[1].sx = x1;
	v[1].sy = y1;
	v[1].color = v[0].color = col.toBGRA();
	v[1].rhw = v[0].rhw = 1.f;
	
	GRenderer->ResetTexture(0);
	EERIEDRAWPRIM(Renderer::LineList, v, 2);
}

void EERIEDraw2DRect(float x0, float y0, float x1, float y1, float z, Color col) {
	
	D3DTLVERTEX v[5];
	v[4].sx = v[3].sx = v[0].sx = x0;
	v[4].sy = v[1].sy = v[0].sy = y0;
	v[2].sx = v[1].sx = x1;
	v[3].sy = v[2].sy = y1;
	v[4].sz = v[3].sz = v[2].sz = v[1].sz = v[0].sz = z;
	v[4].color = v[3].color = v[2].color = v[1].color = v[0].color = col.toBGRA();
	v[4].rhw = v[3].rhw = v[2].rhw = v[1].rhw = v[0].rhw = 1.f;
	
	GRenderer->ResetTexture(0);
	EERIEDRAWPRIM(Renderer::LineStrip, v, 5);
}

void EERIEDrawFill2DRectDegrad(float x0, float y0, float x1, float y1, float z, Color cold, Color cole) {
	
	D3DTLVERTEX v[4];
	v[0].sx = v[2].sx = x0;
	v[0].sy = v[1].sy = y0;
	v[1].sx = v[3].sx = x1;
	v[2].sy = v[3].sy = y1;
	v[0].color = v[1].color = cold.toBGRA();
	v[2].color = v[3].color = cole.toBGRA();
	v[0].sz = v[1].sz = v[2].sz = v[3].sz = z;
	v[3].rhw = v[2].rhw = v[1].rhw = v[0].rhw = 1.f;
	
	GRenderer->ResetTexture(0);
	EERIEDRAWPRIM(Renderer::TriangleStrip, v, 4);
}

void EERIEDraw3DCylinder(const EERIE_CYLINDER & cyl, Color col) {
	
	#define STEPCYL 16
	for(long i = 0; i < 360 - STEPCYL; i += STEPCYL) {
		
		float es = sin(radians(MAKEANGLE((float)i))) * cyl.radius;
		float ec = cos(radians(MAKEANGLE((float)i))) * cyl.radius;
		float es2 = sin(radians(MAKEANGLE((float)(i + STEPCYL)))) * cyl.radius;
		float ec2 = cos(radians(MAKEANGLE((float)(i + STEPCYL)))) * cyl.radius;
		
		// Draw low pos
		EERIEDraw3DLine(cyl.origin + Vec3f(es, 0.f, ec), cyl.origin + Vec3f(es2, 0.f, ec2),  col);
		// Draw vertical 
		Vec3f from = cyl.origin + Vec3f(es, 0.f, ec);
		EERIEDraw3DLine(from, from + Vec3f(0.f, cyl.height, 0.f),  col);
		// Draw high pos
		Vec3f from2 = cyl.origin + Vec3f(es, cyl.height, ec);
		Vec3f to = cyl.origin + Vec3f(es2, cyl.height, ec2);
		EERIEDraw3DLine(from2, to,  col);
	}
}

void EERIEDraw3DCylinderBase(const EERIE_CYLINDER & cyl, Color col) {
	
	#define STEPCYL 16
	for(long i = 0; i < 360 - STEPCYL; i += STEPCYL) {
		
		float es = sin(radians(MAKEANGLE((float)i))) * cyl.radius;
		float ec = cos(radians(MAKEANGLE((float)i))) * cyl.radius;
		float es2 = sin(radians(MAKEANGLE((float)(i + STEPCYL)))) * cyl.radius;
		float ec2 = cos(radians(MAKEANGLE((float)(i + STEPCYL)))) * cyl.radius;
		
		// Draw low pos
		EERIEDraw3DLine(cyl.origin + Vec3f(es, 0.f, ec), cyl.origin + Vec3f(es2, 0.f, ec2),  col);
	}
}

void EERIEDrawCircle(float x0, float y0, float r, Color col, float z) {
	
	register float lx = x0;
	register float ly = y0 + r;
	GRenderer->ResetTexture(0);
	
	for(long i = 0; i < 361; i += 10) {
		float t = radians((float)i);
		float x = x0 - sin(t) * r;
		float y = y0 + cos(t) * r;
		EERIEDraw2DLine(lx, ly, x, y, z, col);
		lx = x;
		ly = y;
	}
}

//*************************************************************************************
//*************************************************************************************

void EERIEDrawTrue3DLine(const Vec3f & orgn, const Vec3f & dest, Color col) {
	
	Vec3f vect = dest - orgn;
	float m=Vector_Magnitude(&vect);

	if (m<=0) return;

	float om=1.f/m;
	vect *= om;
	Vec3f cpos = orgn;

	while (m>0)
	{
		float dep=std::min(m,30.f);
		Vec3f tpos = cpos + (vect * dep);
		EERIEDraw3DLine(cpos, tpos, col);
		cpos = tpos;
		m-=dep;
	}
}
//*************************************************************************************
//*************************************************************************************

void EERIEDraw3DLine(const Vec3f & orgn, const Vec3f & dest, Color col) {
	
	D3DTLVERTEX v[2];
	D3DTLVERTEX in;
	
	in.sx = orgn.x;
	in.sy = orgn.y;
	in.sz = orgn.z;
	EE_RTP(&in,&v[0]);
	if(v[0].sz < 0.f) {
		return;
	}
	
	in.sx = dest.x;
	in.sy = dest.y;
	in.sz = dest.z;
	EE_RTP(&in,&v[1]);
	if(v[1].sz<0.f) {
		return;
	}
	
	GRenderer->ResetTexture(0);
	v[1].color = v[0].color = col.toBGRA();
	
	EERIEDRAWPRIM(Renderer::LineList, v, 2);
}
#define BASICFOCAL 350.f
//*************************************************************************************
//*************************************************************************************

void EERIEDrawSprite(D3DTLVERTEX * in, float siz, TextureContainer * tex, Color color, float Zpos) {
	
	D3DTLVERTEX out;
	
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

		SPRmaxs.x=out.sx+t;
		SPRmins.x=out.sx-t;
		SPRmaxs.y=out.sy+t;
		SPRmins.y=out.sy-t;

		D3DCOLOR col = color.toBGRA();
		D3DTLVERTEX v[4];
		v[0]= D3DTLVERTEX( D3DVECTOR( SPRmins.x, SPRmins.y, out.sz), out.rhw, col, out.specular, 0.f, 0.f);
		v[1]= D3DTLVERTEX( D3DVECTOR( SPRmaxs.x, SPRmins.y, out.sz), out.rhw, col, out.specular, 1.f, 0.f);
		v[2]= D3DTLVERTEX( D3DVECTOR( SPRmins.x, SPRmaxs.y, out.sz), out.rhw, col, out.specular, 0.f, 1.f);
		v[3]= D3DTLVERTEX( D3DVECTOR( SPRmaxs.x, SPRmaxs.y, out.sz), out.rhw, col, out.specular, 1.f, 1.f);

		GRenderer->SetTexture(0, tex);
		EERIEDRAWPRIM(Renderer::TriangleStrip, v, 4);
	}
	else SPRmaxs.x=-1;
}

//*************************************************************************************
//*************************************************************************************

void EERIEDrawRotatedSprite(D3DTLVERTEX * in, float siz, TextureContainer * tex, Color color,
                            float Zpos, float rot) {
	
	D3DTLVERTEX out;
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

		D3DCOLOR col = color.toBGRA();
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

		for(long i=0;i<4;i++) {
			float tt = radians(MAKEANGLE(rot+90.f*i+45+90));
			v[i].sx = EEsin(tt) * t + out.sx;
			v[i].sy = EEcos(tt) * t + out.sy;
		}

		GRenderer->SetTexture(0, tex);
		EERIEDRAWPRIM(Renderer::TriangleFan, v, 4);
	}
	else SPRmaxs.x=-1;
}

//*************************************************************************************
//*************************************************************************************

void EERIEPOLY_DrawWired(EERIEPOLY * ep, Color color) {
	
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

	GRenderer->ResetTexture(0);

	D3DCOLOR col = color.toBGRA();
	if (col)
	 ltv[0].color=ltv[1].color=ltv[2].color=ltv[3].color=ltv[4].color=col;
	else if (to==4)
			ltv[0].color=ltv[1].color=ltv[2].color=ltv[3].color=ltv[4].color=0xFF00FF00;
	else ltv[0].color=ltv[1].color=ltv[2].color=ltv[3].color=0xFFFFFF00;
	
	EERIEDRAWPRIM(Renderer::LineStrip, ltv, to + 1);
}
					
void EERIEPOLY_DrawNormals(EERIEPOLY *ep)
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
	GRenderer->ResetTexture(0);
	ltv[1].color=ltv[0].color=0xFFFF0000;

	if ((ltv[1].sz>0.f) && (ltv[0].sz>0.f))
		EERIEDRAWPRIM(Renderer::LineList, ltv, 3);

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
		GRenderer->ResetTexture(0);
		ltv[1].color=ltv[0].color=Color::yellow.toBGR();

		if ((ltv[1].sz>0.f) &&  (ltv[0].sz>0.f))
			EERIEDRAWPRIM(Renderer::LineList, ltv, 3);
	}
}


//-----------------------------------------------------------------------------

void EERIEDrawBitmap(float x, float y, float sx, float sy, float z, TextureContainer * tex, Color color) {
	
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

	D3DCOLOR col = color.toBGRA();
	D3DTLVERTEX v[4];
	v[0]= D3DTLVERTEX( D3DVECTOR( x,	y,		z ), 1.f, col, 0xFF000000, smu,		smv);
	v[1]= D3DTLVERTEX( D3DVECTOR( x+sx, y,		z ), 1.f, col, 0xFF000000, fEndu,	smv);
	v[2]= D3DTLVERTEX( D3DVECTOR( x,	y+sy,	z ), 1.f, col, 0xFF000000, smu,		fEndv);
	v[3]= D3DTLVERTEX( D3DVECTOR( x+sx,	y+sy,	z ), 1.f, col, 0xFF000000, fEndu,	fEndv);
	
	GRenderer->SetTexture(0, tex);
	EERIEDRAWPRIM(Renderer::TriangleStrip, v, 4);
}

void EERIEDrawBitmap_uv(float x, float y, float sx, float sy, float z, TextureContainer * tex,
                        Color color, float u0, float v0, float u1, float v1) {
	
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

	D3DCOLOR col = color.toBGRA();
	D3DTLVERTEX v[4];
	u0=smu+(fEndu-smu+fDecalU)*u0;
	u1=smu+(fEndu-smu+fDecalU)*u1;
	v0=smv+(fEndv-smv+fDecalV)*v0;
	v1=smv+(fEndv-smv+fDecalV)*v1;
	v[0]= D3DTLVERTEX( D3DVECTOR( x, y, z ), 1.f, col, 0xFF000000, u0, v0);
	v[1]= D3DTLVERTEX( D3DVECTOR( x+sx, y, z ), 1.f, col, 0xFF000000, u1, v0);
	v[2]= D3DTLVERTEX( D3DVECTOR( x+sx, y+sy, z ), 1.f, col, 0xFF000000, u1, v1);
	v[3]= D3DTLVERTEX( D3DVECTOR( x, y+sy, z ), 1.f, col, 0xFF000000, u0, v1);

	GRenderer->SetTexture(0, tex);
	EERIEDRAWPRIM(Renderer::TriangleFan, v, 4);
}

void EERIEDrawBitmapUVs(float x, float y, float sx, float sy, float z, TextureContainer * tex,
                        Color color, float u0, float v0, float u1, float v1, float u2, float v2,
	                      float u3, float v3) {
	
	register float smu,smv;

	if (tex)
	{
		smu=tex->m_hdx;
		smv=tex->m_hdy;
	}
	else
	{
		smu=smv=0.f;
	}

	D3DCOLOR col = color.toBGRA();
	D3DTLVERTEX v[4];
	v[0]= D3DTLVERTEX( D3DVECTOR( x,	y,		z ), 1.f, col, 0xFF000000, smu+u0,	smv+v0);
	v[1]= D3DTLVERTEX( D3DVECTOR( x+sx, y,		z ), 1.f, col, 0xFF000000, smu+u1,	smv+v1);
	v[2]= D3DTLVERTEX( D3DVECTOR( x,	y+sy,	z ), 1.f, col, 0xFF000000, smu+u2,	smv+v2);
	v[3]= D3DTLVERTEX( D3DVECTOR( x+sx,	y+sy,	z ), 1.f, col, 0xFF000000, smu+u3,	smv+v3);
	
	GRenderer->SetTexture(0, tex);
	EERIEDRAWPRIM(Renderer::TriangleStrip, v, 4);	
}

void EERIEDrawBitmap2(float x, float y, float sx, float sy, float z, TextureContainer * tex, Color color) {
	
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

	D3DCOLOR col = color.toBGRA();
	D3DTLVERTEX v[4];
	float fZMinus=1.f-z;
	v[0]= D3DTLVERTEX( D3DVECTOR( x, y, z ), fZMinus, col, 0xFF000000, smu, smv);
	v[1]= D3DTLVERTEX( D3DVECTOR( x+sx, y, z ), fZMinus, col, 0xFF000000, fEndu, smv);
	v[2]= D3DTLVERTEX( D3DVECTOR( x, y+sy, z ), fZMinus, col, 0xFF000000, smu, fEndv);
	v[3]= D3DTLVERTEX( D3DVECTOR( x+sx, y+sy, z ), fZMinus, col, 0xFF000000, fEndu, fEndv);

	GRenderer->SetTexture(0, tex);
	EERIEDRAWPRIM(Renderer::TriangleStrip, v, 4);
}

void EERIEDrawBitmap2DecalY(float x, float y, float sx, float sy, float z, TextureContainer * tex,
                            Color color, float _fDeltaY) {
	
	float smu;
	float fDepv;
	float fEndu,fEndv;

	if (tex)
	{
		smu=tex->m_hdx;
		fEndu=tex->m_dx;
		fEndv=tex->m_dy;
		fDepv=tex->m_hdy+(tex->m_dy-tex->m_hdy)*_fDeltaY;
	}
	else
	{
		smu=0.f;
		fDepv=0.f;
		fEndu=fEndv=0.f;
	}

	D3DCOLOR col = color.toBGRA();
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

	GRenderer->SetTexture(0, tex);
	EERIEDRAWPRIM(Renderer::TriangleFan, v, 4);	
}
