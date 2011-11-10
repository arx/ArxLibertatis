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

#include "graphics/Draw.h"

#include "core/Application.h"

#include "graphics/VertexBuffer.h"
#include "graphics/Math.h"
#include "graphics/data/TextureContainer.h"
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

	if (fabs(dot(ep->norm, ep2->norm)) < 1.f - tolerance) return false;
	
	for (long i=0;i<3;i++)
	{
		common=-1;
		common2=-1;

		for (long j=0;j<3;j++)
		{
			if (   ( NearlyEqual(ep->v[i].p.x,ep2->v[j].p.x) )
				&& ( NearlyEqual(ep->v[i].p.y,ep2->v[j].p.y) )
				&& ( NearlyEqual(ep->v[i].p.z,ep2->v[j].p.z) )
				&& ( NearlyEqual(ep->v[i].uv.x,ep2->v[j].uv.x) )
				&& ( NearlyEqual(ep->v[i].uv.y,ep2->v[j].uv.y) )
				)
			{
				count++;
				common=j;
			}

			if (   ( NearlyEqual(ep->v[j].p.x,ep2->v[i].p.x) )
				&& ( NearlyEqual(ep->v[j].p.y,ep2->v[i].p.y) )
				&& ( NearlyEqual(ep->v[j].p.z,ep2->v[i].p.z) )
				&& ( NearlyEqual(ep->v[j].uv.x,ep2->v[i].uv.x) )
				&& ( NearlyEqual(ep->v[j].uv.y,ep2->v[i].uv.y) )
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
		ep2->v[3].p.x=ep->v[ep_notcommon].p.x;
		ep2->v[3].p.y=ep->v[ep_notcommon].p.y;
		ep2->v[3].p.z=ep->v[ep_notcommon].p.z;
		ep2->tv[3].uv.x=ep2->v[3].uv.x=ep->v[ep_notcommon].uv.x;
		ep2->tv[3].uv.y=ep2->v[3].uv.y=ep->v[ep_notcommon].uv.y;
		ep2->tv[3].color=ep2->v[3].color=Color::white.toBGR();
		ep2->tv[3].rhw=ep2->v[3].rhw=1.f;

	DeclareEGInfo(ep->v[3].p.x, ep->v[3].p.z);

	ep2->center.x=(ep2->v[0].p.x+ep2->v[1].p.x+ep2->v[2].p.x+ep2->v[3].p.x)*( 1.0f / 4 );
	ep2->center.y=(ep2->v[0].p.y+ep2->v[1].p.y+ep2->v[2].p.y+ep2->v[3].p.y)*( 1.0f / 4 );
	ep2->center.z=(ep2->v[0].p.z+ep2->v[1].p.z+ep2->v[2].p.z+ep2->v[3].p.z)*( 1.0f / 4 );
	ep2->max.x=max(ep2->max.x,ep2->v[3].p.x);
	ep2->min.x=min(ep2->min.x,ep2->v[3].p.x);
	ep2->max.y=max(ep2->max.y,ep2->v[3].p.y);
	ep2->min.y=min(ep2->min.y,ep2->v[3].p.y);
	ep2->max.z=max(ep2->max.z,ep2->v[3].p.z);
	ep2->min.z=min(ep2->min.z,ep2->v[3].p.z);

	ep2->norm2.x=ep->norm.x;
	ep2->norm2.y=ep->norm.y;
	ep2->norm2.z=ep->norm.z;
	
	ep2->area += fdist((ep2->v[1].p + ep2->v[2].p) * .5f, ep2->v[3].p)
	             * fdist(ep2->v[3].p, ep2->v[1].p)*.5f; // should this be v[2] instead of v[3]?

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
	cx=(ep->v[0].p.x+ep->v[1].p.x+ep->v[2].p.x);
	cz=(ep->v[0].p.z+ep->v[1].p.z+ep->v[2].p.z);
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

CircularVertexBuffer<TexturedVertex> * pDynamicVertexBuffer_TLVERTEX;

void EERIEDRAWPRIM(Renderer::Primitive primitive, const TexturedVertex * vertices, size_t count, bool nocount) {
	
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
					TexturedVertex verts[4];
					GRenderer->SetTexture(0, enviro);

					for (long i=0;i<to;i++)
					{
						verts[i].p.x=ep->tv[i].p.x;
						verts[i].p.y=ep->tv[i].p.y;
						verts[i].p.z=ep->tv[i].p.z;
						verts[i].rhw=ep->tv[i].rhw;
						verts[i].color=0xFFFFFFFF;

						// Water
						if (ep->type & POLY_LAVA)
						{
							verts[i].uv.x=ep->v[i].p.x*( 1.0f / 1000 )+EEsin((ep->v[i].p.x)*( 1.0f / 200 )+(float)FrameTime*( 1.0f / 2000 ))*( 1.0f / 20 );
							verts[i].uv.y=ep->v[i].p.z*( 1.0f / 1000 )+EEcos((ep->v[i].p.z)*( 1.0f / 200 )+(float)FrameTime*( 1.0f / 2000 ))*( 1.0f / 20 );
						}
						else
						{
							GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
							verts[i].uv.x=ep->v[i].p.x*( 1.0f / 800 )+EEsin((ep->v[i].p.x)*( 1.0f / 600 )+(float)FrameTime*( 1.0f / 1000 ))*( 1.0f / 9 );
							verts[i].uv.y=ep->v[i].p.z*( 1.0f / 800 )+EEcos((ep->v[i].p.z)*( 1.0f / 600 )+(float)FrameTime*( 1.0f / 1000 ))*( 1.0f / 9 );

							if (ep->type & POLY_FALL) verts[i].uv.y-=(float)(FrameTime)*( 1.0f / 200 );
						}
					}

					EERIEDRAWPRIM(Renderer::TriangleStrip, verts, to, true);

					if (ep->type & POLY_WATER)
					{
						for (long i=0;i<to;i++)
						{
							verts[i].uv.x=ep->v[i].p.x*( 1.0f / 1000 )+EEsin((ep->v[i].p.y)*( 1.0f / 200 )+(float)FrameTime*( 1.0f / 600 )*( 1.0f / 3 ))*( 1.0f / 10 );
							verts[i].uv.y=ep->v[i].p.z*( 1.0f / 1000 )+EEcos((ep->v[i].p.z+ep->v[i].p.x)*( 1.0f / 200 )+(float)FrameTime*( 1.0f / 600 )*( 1.0f / 3 ))*( 1.0f / 10 );

							if (ep->type & POLY_FALL) 
							{
								verts[i].uv.y-=(float)(FrameTime)*( 1.0f / 200 );
							}
						}	

						EERIEDRAWPRIM(Renderer::TriangleStrip, verts, to);
					}

					if (ep->type & POLY_LAVA)
					{
						for (long i=0;i<to;i++)
						{
							verts[i].uv.x=ep->v[i].p.x*( 1.0f / 1000 )+EEsin((ep->v[i].p.x)*( 1.0f / 100 )+(float)FrameTime*( 1.0f / 2000 ))*( 1.0f / 10 );
							verts[i].uv.y=ep->v[i].p.z*( 1.0f / 1000 )+EEcos((ep->v[i].p.z)*( 1.0f / 100 )+(float)FrameTime*( 1.0f / 2000 ))*( 1.0f / 10 );
						}	
						EERIEDRAWPRIM(Renderer::TriangleStrip, verts, to);
						for ( int i=0;i<to;i++)
						{
							verts[i].uv.x=ep->v[i].p.x*( 1.0f / 600 )+EEsin((ep->v[i].p.x)*( 1.0f / 160 )+(float)FrameTime*( 1.0f / 2000 ))*( 1.0f / 11 );
							verts[i].uv.y=ep->v[i].p.z*( 1.0f / 600 )+EEcos((ep->v[i].p.z)*( 1.0f / 160 )+(float)FrameTime*( 1.0f / 2000 ))*( 1.0f / 11 );
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
					TexturedVertex verts[4];
					GRenderer->SetTexture(0, ptcTexture->TextureRefinement); 

					for (long i=0;i<ptcTexture->delayed_nb;i++)
					{
						DELAYED_PRIM * del=(DELAYED_PRIM *)ptcTexture->delayed;
						EERIEPOLY * ep=del[i].data;

						if (ep->type & POLY_QUAD) 
						{
							if ( (ep->tv[0].p.z>0.048f) 
								&& (ep->tv[1].p.z>0.048f)
								&& (ep->tv[2].p.z>0.048f) 
							        && (ep->tv[3].p.z > 0.048f)) continue; 

							to=4;
						}
						else 
						{
							if ( (ep->tv[0].p.z>0.048f) 
								&& (ep->tv[1].p.z>0.048f)
							        && (ep->tv[2].p.z > 0.048f)) continue; 

							to=3;	
						}					

						if (!(ep->type & POLY_DOUBLESIDED))
							GRenderer->SetCulling(Renderer::CullCW);			
						else GRenderer->SetCulling(Renderer::CullNone);
						
						
						register long tmp;

						for (register long j=0;j<to;j++)
						{
							verts[j].p.x=ep->tv[j].p.x;
							verts[j].p.y=ep->tv[j].p.y;
							verts[j].p.z=ep->tv[j].p.z;
							verts[j].uv.x=ep->tv[j].uv.x*4.f;
							verts[j].uv.y=ep->tv[j].uv.y*4.f;
							verts[j].rhw=ep->tv[j].rhw;

							float val;

							val = (0.038f - verts[j].p.z); 

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
	
	TexturedVertex v[2];
	v[0].p.x = x0;
	v[0].p.y = y0;
	v[0].p.z = v[1].p.z = z;
	v[1].p.x = x1;
	v[1].p.y = y1;
	v[1].color = v[0].color = col.toBGRA();
	v[1].rhw = v[0].rhw = 1.f;
	
	GRenderer->ResetTexture(0);
	EERIEDRAWPRIM(Renderer::LineList, v, 2);
}

void EERIEDraw2DRect(float x0, float y0, float x1, float y1, float z, Color col) {
	
	TexturedVertex v[5];
	v[4].p.x = v[3].p.x = v[0].p.x = x0;
	v[4].p.y = v[1].p.y = v[0].p.y = y0;
	v[2].p.x = v[1].p.x = x1;
	v[3].p.y = v[2].p.y = y1;
	v[4].p.z = v[3].p.z = v[2].p.z = v[1].p.z = v[0].p.z = z;
	v[4].color = v[3].color = v[2].color = v[1].color = v[0].color = col.toBGRA();
	v[4].rhw = v[3].rhw = v[2].rhw = v[1].rhw = v[0].rhw = 1.f;
	
	GRenderer->ResetTexture(0);
	EERIEDRAWPRIM(Renderer::LineStrip, v, 5);
}

void EERIEDrawFill2DRectDegrad(float x0, float y0, float x1, float y1, float z, Color cold, Color cole) {
	
	TexturedVertex v[4];
	v[0].p.x = v[2].p.x = x0;
	v[0].p.y = v[1].p.y = y0;
	v[1].p.x = v[3].p.x = x1;
	v[2].p.y = v[3].p.y = y1;
	v[0].color = v[1].color = cold.toBGRA();
	v[2].color = v[3].color = cole.toBGRA();
	v[0].p.z = v[1].p.z = v[2].p.z = v[3].p.z = z;
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
	float m = ffsqrt(vect.lengthSqr());

	if (m<=0) return;

	vect *= 1 / m;
	
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
	
	TexturedVertex v[2];
	TexturedVertex in;
	
	in.p.x = orgn.x;
	in.p.y = orgn.y;
	in.p.z = orgn.z;
	EE_RTP(&in,&v[0]);
	if(v[0].p.z < 0.f) {
		return;
	}
	
	in.p.x = dest.x;
	in.p.y = dest.y;
	in.p.z = dest.z;
	EE_RTP(&in,&v[1]);
	if(v[1].p.z<0.f) {
		return;
	}
	
	GRenderer->ResetTexture(0);
	v[1].color = v[0].color = col.toBGRA();
	
	EERIEDRAWPRIM(Renderer::LineList, v, 2);
}
#define BASICFOCAL 350.f
//*************************************************************************************
//*************************************************************************************

void EERIEDrawSprite(TexturedVertex * in, float siz, TextureContainer * tex, Color color, float Zpos) {
	
	TexturedVertex out;
	
	EERIETreatPoint2(in,&out);

	if ((out.p.z>0.f) && (out.p.z<1000.f)
		&& (out.p.x>-1000) && (out.p.x<2500.f)
		&& (out.p.y>-500) && (out.p.y<1800.f))
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
			out.p.z = Zpos;
			out.rhw=1.f-out.p.z;
		}
		else
		{
			out.rhw*=(1.f/3000.f);
		}

		SPRmaxs.x=out.p.x+t;
		SPRmins.x=out.p.x-t;
		SPRmaxs.y=out.p.y+t;
		SPRmins.y=out.p.y-t;

		ColorBGRA col = color.toBGRA();
		TexturedVertex v[4];
		v[0] = TexturedVertex(Vec3f(SPRmins.x, SPRmins.y, out.p.z), out.rhw, col, out.specular, Vec2f::ZERO);
		v[1] = TexturedVertex(Vec3f(SPRmaxs.x, SPRmins.y, out.p.z), out.rhw, col, out.specular, Vec2f::X_AXIS);
		v[2] = TexturedVertex(Vec3f(SPRmins.x, SPRmaxs.y, out.p.z), out.rhw, col, out.specular, Vec2f::Y_AXIS);
		v[3] = TexturedVertex(Vec3f(SPRmaxs.x, SPRmaxs.y, out.p.z), out.rhw, col, out.specular, Vec2f(1.f, 1.f));

		GRenderer->SetTexture(0, tex);
		EERIEDRAWPRIM(Renderer::TriangleStrip, v, 4);
	}
	else SPRmaxs.x=-1;
}

//*************************************************************************************
//*************************************************************************************

void EERIEDrawRotatedSprite(TexturedVertex * in, float siz, TextureContainer * tex, Color color,
                            float Zpos, float rot) {
	
	TexturedVertex out;
	EERIETreatPoint2(in, &out);
	
	if ((out.p.z>0.f) && (out.p.z<1000.f))
	{
		float use_focal=BASICFOCAL*Xratio;
	
		float t = siz * ((out.rhw - 1.f) * use_focal * 0.001f); 

		if (t<=0.f) t=0.00000001f;

		if (Zpos<=1.f)
		{
			out.p.z = Zpos; 
			out.rhw=1.f-out.p.z;
		}
		else
		{
			out.rhw*=(1.f/3000.f);
		}

		ColorBGRA col = color.toBGRA();
		TexturedVertex v[4];
		v[0] = TexturedVertex(Vec3f(0, 0, out.p.z), out.rhw, col, out.specular, Vec2f::ZERO);
		v[1] = TexturedVertex(Vec3f(0, 0, out.p.z), out.rhw, col, out.specular, Vec2f::X_AXIS);
		v[2] = TexturedVertex(Vec3f(0, 0, out.p.z), out.rhw, col, out.specular, Vec2f(1.f, 1.f));
		v[3] = TexturedVertex(Vec3f(0, 0, out.p.z), out.rhw, col, out.specular, Vec2f::Y_AXIS);
		
		
		SPRmaxs.x=out.p.x+t;
		SPRmins.x=out.p.x-t;
		
		SPRmaxs.y=out.p.y+t;			
		SPRmins.y=out.p.y-t;

		SPRmaxs.z = SPRmins.z = out.p.z; 

		for(long i=0;i<4;i++) {
			float tt = radians(MAKEANGLE(rot+90.f*i+45+90));
			v[i].p.x = EEsin(tt) * t + out.p.x;
			v[i].p.y = EEcos(tt) * t + out.p.y;
		}

		GRenderer->SetTexture(0, tex);
		EERIEDRAWPRIM(Renderer::TriangleFan, v, 4);
	}
	else SPRmaxs.x=-1;
}

//*************************************************************************************
//*************************************************************************************

void EERIEPOLY_DrawWired(EERIEPOLY * ep, Color color) {
	
	TexturedVertex ltv[5];
	ltv[0] = TexturedVertex(Vec3f(0, 0, 0.5), 1.f, 1, 1, Vec2f::ZERO);
	ltv[1] = TexturedVertex(Vec3f(0, 0, 0.5), 1.f, 1, 1, Vec2f::X_AXIS);
	ltv[2] = TexturedVertex(Vec3f(0, 0, 0.5), 1.f, 1, 1, Vec2f(1.f, 1.f));
	ltv[3] = TexturedVertex(Vec3f(0, 0, 0.5), 1.f, 1, 1, Vec2f::Y_AXIS);
	ltv[4] = TexturedVertex(Vec3f(0, 0, 0.5), 1.f, 1, 1, Vec2f::Y_AXIS);
	
	long to;

	if (ep->type & POLY_QUAD) to=4;
	else to=3;

	memcpy(ltv,ep->tv,sizeof(TexturedVertex)*to);							
	ltv[0].p.z-=0.0002f;
	ltv[1].p.z-=0.0002f;
	ltv[2].p.z-=0.0002f;
	ltv[3].p.z-=0.0002f;

	if (to==4) 
	{
		memcpy(&ltv[2],&ep->tv[3],sizeof(TexturedVertex));
		memcpy(&ltv[3],&ep->tv[2],sizeof(TexturedVertex));
		memcpy(&ltv[4],&ep->tv[0],sizeof(TexturedVertex));
		ltv[4].p.z-=0.0002f;
	}
	else memcpy(&ltv[to],&ltv[0],sizeof(TexturedVertex));

	GRenderer->ResetTexture(0);

	ColorBGRA col = color.toBGRA();
	if (col)
	 ltv[0].color=ltv[1].color=ltv[2].color=ltv[3].color=ltv[4].color=col;
	else if (to==4)
			ltv[0].color=ltv[1].color=ltv[2].color=ltv[3].color=ltv[4].color=0xFF00FF00;
	else ltv[0].color=ltv[1].color=ltv[2].color=ltv[3].color=0xFFFFFF00;
	
	EERIEDRAWPRIM(Renderer::LineStrip, ltv, to + 1);
}

void EERIEPOLY_DrawNormals(EERIEPOLY * ep) {
	TexturedVertex ltv[5];
	ltv[0] = TexturedVertex(Vec3f(0, 0, 0.5), 1.f, 1, 1, Vec2f::ZERO);
	ltv[1] = TexturedVertex(Vec3f(0, 0, 0.5), 1.f, 1, 1, Vec2f::X_AXIS);
	ltv[2] = TexturedVertex(Vec3f(0, 0, 0.5), 1.f, 1, 1, Vec2f(1.f, 1.f));
	ltv[3] = TexturedVertex(Vec3f(0, 0, 0.5), 1.f, 1, 1, Vec2f::Y_AXIS);
	ltv[4] = TexturedVertex(Vec3f(0, 0, 0.5), 1.f, 1, 1, Vec2f::Y_AXIS);
	
	TexturedVertex lv;
	long to;

	if (ep->type & POLY_QUAD) to=4;
	else to=3;

	lv.p.x=ep->center.x;
	lv.p.y=ep->center.y;
	lv.p.z=ep->center.z;
	EE_RTP(&lv,&ltv[0]);
	lv.p.x+=ep->norm.x*10.f;
	lv.p.y+=ep->norm.y*10.f;
	lv.p.z+=ep->norm.z*10.f;
	EE_RTP(&lv,&ltv[1]);					
	GRenderer->ResetTexture(0);
	ltv[1].color=ltv[0].color=0xFFFF0000;

	if ((ltv[1].p.z>0.f) && (ltv[0].p.z>0.f))
		EERIEDRAWPRIM(Renderer::LineList, ltv, 3);

	for (long h=0;h<to;h++)
	{
		lv.p.x=ep->v[h].p.x;
		lv.p.y=ep->v[h].p.y;
		lv.p.z=ep->v[h].p.z;
		EE_RTP(&lv,&ltv[0]);
		lv.p.x+=ep->nrml[h].x*10.f;
		lv.p.y+=ep->nrml[h].y*10.f;
		lv.p.z+=ep->nrml[h].z*10.f;
		EE_RTP(&lv,&ltv[1]);
		GRenderer->ResetTexture(0);
		ltv[1].color=ltv[0].color=Color::yellow.toBGR();

		if ((ltv[1].p.z>0.f) &&  (ltv[0].p.z>0.f))
			EERIEDRAWPRIM(Renderer::LineList, ltv, 3);
	}
}


//-----------------------------------------------------------------------------

void EERIEDrawBitmap(float x, float y, float sx, float sy, float z, TextureContainer * tex, Color color) {
	
	// Match pixel and texel origins.
	x -= .5f, y -= .5f;
	
	Vec2f uv = (tex) ? tex->uv : Vec2f::ZERO;
	
	ColorBGRA col = color.toBGRA();
	TexturedVertex v[4];
	v[0] = TexturedVertex(Vec3f(x,      y,      z), 1.f, col, 0xff000000, Vec2f(0.f,  0.f));
	v[1] = TexturedVertex(Vec3f(x + sx, y,      z), 1.f, col, 0xff000000, Vec2f(uv.x, 0.f));
	v[2] = TexturedVertex(Vec3f(x,      y + sy, z), 1.f, col, 0xff000000, Vec2f(0.f,  uv.y));
	v[3] = TexturedVertex(Vec3f(x + sx, y + sy, z), 1.f, col, 0xff000000, Vec2f(uv.x, uv.y));
	
	GRenderer->SetTexture(0, tex);
	EERIEDRAWPRIM(Renderer::TriangleStrip, v, 4);
}

void EERIEDrawBitmap_uv(float x, float y, float sx, float sy, float z, TextureContainer * tex,
                        Color color, float u0, float v0, float u1, float v1) {
	
	// Match pixel and texel origins.
	x -= .5f, y -= .5f;
	
	Vec2f uv = (tex) ? tex->uv : Vec2f::ONE;
	u0 *= uv.x, u1 *= uv.x, v0 *= uv.y, v1 *= uv.y;

	ColorBGRA col = color.toBGRA();
	TexturedVertex v[4];
	v[0] = TexturedVertex(Vec3f(x,      y,      z), 1.f, col, 0xff000000, Vec2f(u0, v0));
	v[1] = TexturedVertex(Vec3f(x + sx, y,      z), 1.f, col, 0xff000000, Vec2f(u1, v0));
	v[2] = TexturedVertex(Vec3f(x + sx, y + sy, z), 1.f, col, 0xff000000, Vec2f(u1, v1));
	v[3] = TexturedVertex(Vec3f(x,      y + sy, z), 1.f, col, 0xff000000, Vec2f(u0, v1));

	GRenderer->SetTexture(0, tex);
	EERIEDRAWPRIM(Renderer::TriangleFan, v, 4);
}

void EERIEDrawBitmapUVs(float x, float y, float sx, float sy, float z, TextureContainer * tex,
                        Color color, float u0, float v0, float u1, float v1, float u2, float v2,
	                      float u3, float v3) {
	
	// Match pixel and texel origins.
	x -= .5f, y -= .5f;
	
	ColorBGRA col = color.toBGRA();
	TexturedVertex v[4];
	v[0] = TexturedVertex(Vec3f(x,      y,      z), 1.f, col, 0xff000000, Vec2f(u0, v0));
	v[1] = TexturedVertex(Vec3f(x + sx, y,      z), 1.f, col, 0xff000000, Vec2f(u1, v1));
	v[2] = TexturedVertex(Vec3f(x,      y + sy, z), 1.f, col, 0xff000000, Vec2f(u2, v2));
	v[3] = TexturedVertex(Vec3f(x + sx, y + sy, z), 1.f, col, 0xff000000, Vec2f(u3, v3));
	
	GRenderer->SetTexture(0, tex);
	EERIEDRAWPRIM(Renderer::TriangleStrip, v, 4);	
}

void EERIEDrawBitmap2(float x, float y, float sx, float sy, float z, TextureContainer * tex, Color color) {
	
	// Match pixel and texel origins.
	x -= .5f, y -= .5f;
	
	Vec2f uv = (tex) ? tex->uv : Vec2f::ZERO;
	
	ColorBGRA col = color.toBGRA();
	TexturedVertex v[4];
	float rhw = 1.f - z;
	v[0] = TexturedVertex(Vec3f(x,      y,      z), rhw, col, 0xFF000000, Vec2f(0.f,  0.f));
	v[1] = TexturedVertex(Vec3f(x + sx, y,      z), rhw, col, 0xFF000000, Vec2f(uv.x, 0.f));
	v[2] = TexturedVertex(Vec3f(x,      y + sy, z), rhw, col, 0xFF000000, Vec2f(0.f,  uv.y));
	v[3] = TexturedVertex(Vec3f(x + sx, y + sy, z), rhw, col, 0xFF000000, Vec2f(uv.x, uv.y));
	
	GRenderer->SetTexture(0, tex);
	EERIEDRAWPRIM(Renderer::TriangleStrip, v, 4);
}

void EERIEDrawBitmap2DecalY(float x, float y, float sx, float sy, float z, TextureContainer * tex,
                            Color color, float _fDeltaY) {
	
	// Match pixel and texel origins.
	x -= .5f, y -= .5f;
	
	Vec2f uv = (tex) ? tex->uv : Vec2f::ZERO;
	float sv = uv.y * _fDeltaY;
	
	ColorBGRA col = color.toBGRA();
	TexturedVertex v[4];
	float fDy = _fDeltaY * sy;
	if(sx < 0) {
		v[0] = TexturedVertex(Vec3f(x,      y + fDy, z), 1.f, col, 0xFF000000, Vec2f(uv.x, sv));
		v[1] = TexturedVertex(Vec3f(x - sx, y + fDy, z), 1.f, col, 0xFF000000, Vec2f(0.f,  sv));
		v[2] = TexturedVertex(Vec3f(x - sx, y + sy,  z), 1.f, col, 0xFF000000, Vec2f(0.f,  uv.y));
		v[3] = TexturedVertex(Vec3f(x,      y + sy,  z), 1.f, col, 0xFF000000, Vec2f(uv.x, uv.y));
	} else {
		v[0] = TexturedVertex(Vec3f(x,      y + fDy, z), 1.f, col, 0xFF000000, Vec2f(0.f,  sv));
		v[1] = TexturedVertex(Vec3f(x + sx, y + fDy, z), 1.f, col, 0xFF000000, Vec2f(uv.x, sv));
		v[2] = TexturedVertex(Vec3f(x + sx, y + sy,  z), 1.f, col, 0xFF000000, Vec2f(uv.x, uv.y));
		v[3] = TexturedVertex(Vec3f(x,      y + sy,  z), 1.f, col, 0xFF000000, Vec2f(0.f,  uv.y));
	}
	
	GRenderer->SetTexture(0, tex);
	EERIEDRAWPRIM(Renderer::TriangleFan, v, 4);	
}
