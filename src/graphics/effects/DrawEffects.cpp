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
// ARX_Draw
//////////////////////////////////////////////////////////////////////////////////////
// 
// Description:
//		ARX-specific Drawing Funcs
//
// Updates:	(date)		(person)	(update)
//			2001/10/14	Cyril		Some Verifications
//
// Initial Code: Cyril Meynier
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#include "graphics/effects/DrawEffects.h"

#include "animation/AnimationRender.h"

#include "core/Application.h"
#include "core/Config.h"
#include "core/Core.h"
#include "core/GameTime.h"

#include "game/Spells.h"

#include "gui/MenuWidgets.h"

#include "graphics/Draw.h"
#include "graphics/Math.h"
#include "graphics/data/Mesh.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/texture/TextureStage.h"

#include "scene/Light.h"
#include "scene/Interactive.h"

// Some external defs needing to be cleaned...
extern long DANAESIZX;
extern Vec3f SPRmins;
extern Vec3f SPRmaxs;
extern long MOVETYPE;
extern EERIE_3DOBJ * eyeballobj; 
extern TextureContainer * Boom;

POLYBOOM polyboom[MAX_POLYBOOM];

extern Color ulBKGColor;

void EE_RT2(TexturedVertex*,TexturedVertex*);

//***********************************************************************************************
// hum... to be checked again for performance and result quality.
//-----------------------------------------------------------------------------------------------
// VERIFIED (Cyril 2001/10/15)
//***********************************************************************************************
void ARXDRAW_DrawInterShadows()
{	
	GRenderer->SetFogColor(Color::none);
	SetZBias(1);

	long first=1;
	
	for (long i=0;i<TREATZONE_CUR;i++) 
	{
		if ((treatio[i].show!=1) || (treatio[i].io==NULL)) continue;

		INTERACTIVE_OBJ * io=treatio[i].io;

		if (	(!io->obj) 
			||	(io->ioflags & IO_JUST_COLLIDE)	)
		{
			continue;
		}

			if ((Project.hide & HIDE_NPC) &&  (io->ioflags & IO_NPC)) continue;

			if ((Project.hide & HIDE_ITEMS) &&  (io->ioflags & IO_ITEM)) continue;

			if ((Project.hide & HIDE_FIXINTER) && (io->ioflags & IO_FIX)) continue;

			long xx,yy;
			xx = io->pos.x * ACTIVEBKG->Xmul;
			yy = io->pos.z * ACTIVEBKG->Zmul;

			if ( (xx>=1) && (yy>=1) && (xx<ACTIVEBKG->Xsize-1) && (yy<ACTIVEBKG->Zsize-1) )
			{
				FAST_BKG_DATA * feg=(FAST_BKG_DATA *)&ACTIVEBKG->fastdata[xx][yy];

				if(!feg->treat) continue;
			}

			if (!( io->ioflags & IO_NOSHADOW ) )
			if ( io->show==SHOW_FLAG_IN_SCENE ) 
			if ( !(io->ioflags & IO_GOLD) ) 
			{
				register EERIEPOLY * ep;
				TexturedVertex in;			
				
				TexturedVertex ltv[4];
				ltv[0]=	TexturedVertex( Vec3f( 0, 0, 0.001f ), 1.f, 0, 1, 0.3f, 0.3f) ;
				ltv[1]=	TexturedVertex( Vec3f( 0, 0, 0.001f ), 1.f, 0, 1, 0.7f, 0.3f) ;
				ltv[2]=	TexturedVertex( Vec3f( 0, 0, 0.001f ), 1.f, 0, 1, 0.7f, 0.7f) ;		
				ltv[3]=	TexturedVertex( Vec3f( 0, 0, 0.001f ), 1.f, 0, 1, 0.3f, 0.7f) ;
				
				float s1=16.f*io->scale;
				float s2=s1 * ( 1.0f / 2 );	

				if (io->obj->nbgroups<=1)
				{
					for (size_t k=0;k<io->obj->vertexlist.size();k+=9)
					{
						ep=EECheckInPoly(&io->obj->vertexlist3[k].v);

						if (ep!=NULL)
						{
							in.sy=ep->min.y-3.f;
							float r=0.5f-((float)EEfabs(io->obj->vertexlist3[k].v.y-in.sy))*( 1.0f / 500 );
							r-=io->invisibility;
							r*=io->scale;

							if (r<=0.f) continue;
							
							in.sx=io->obj->vertexlist3[k].v.x-s2;						
							in.sz=io->obj->vertexlist3[k].v.z-s2;

							r*=255.f;
							long lv = r;
							ltv[0].color=ltv[1].color=ltv[2].color=ltv[3].color=0xFF000000 | lv<<16 | lv<<8 | lv;
							
							if (first)
							{
								first=0;
								GRenderer->SetRenderState(Renderer::DepthWrite, false);
								GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);
								GRenderer->SetRenderState(Renderer::AlphaBlending, true);
								GRenderer->SetTexture(0, Boom);
							}

							EE_RT2(&in,&ltv[0]);
							in.sx+=s1;
							EE_RT2(&in,&ltv[1]);
							in.sz+=s1;
							EE_RT2(&in,&ltv[2]);
							in.sx-=s1;
							EE_RT2(&in,&ltv[3]);

							if(ltv[0].sz > 0.f && ltv[1].sz > 0.f && ltv[2].sz > 0.f) {
								ARX_DrawPrimitive(&ltv[0], &ltv[1], &ltv[2]);
								ARX_DrawPrimitive(&ltv[0], &ltv[2], &ltv[3]);
							}
						}
					}	
				}
				else 
				{
					for (long k=0;k<io->obj->nbgroups;k++)
					{
						long origin=io->obj->grouplist[k].origin;
						ep=EECheckInPoly(	&io->obj->vertexlist3[origin].v );

						if (ep!=NULL)
						{
							in.sy=ep->min.y-3.f;
							float r=0.8f-((float)EEfabs(io->obj->vertexlist3[origin].v.y-in.sy))*( 1.0f / 500 );
							r*=io->obj->grouplist[k].siz;
							r-=io->invisibility;

							if (r<=0.f) continue;

							float s1=io->obj->grouplist[k].siz*44.f;
							float s2=s1*( 1.0f / 2 );
							in.sx=io->obj->vertexlist3[origin].v.x-s2;						
							in.sz=io->obj->vertexlist3[origin].v.z-s2;

							r*=255.f;
							long lv = r;
							ltv[0].color=	ltv[1].color	=	ltv[2].color	=	ltv[3].color	=	0xFF000000 | lv<<16 | lv<<8 | lv;

							if (first)
							{
								first=0;
								GRenderer->SetRenderState(Renderer::DepthWrite, false);
								GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);
								GRenderer->SetRenderState(Renderer::AlphaBlending, true);
								GRenderer->SetTexture(0, Boom);
							}

							EE_RT2(&in,&ltv[0]);
							in.sx+=s1;
							EE_RT2(&in,&ltv[1]);
							in.sz+=s1;
							EE_RT2(&in,&ltv[2]);
							in.sx-=s1;
							EE_RT2(&in,&ltv[3]);
							ARX_DrawPrimitive(&ltv[0], &ltv[1], &ltv[2]);
							ARX_DrawPrimitive(&ltv[0], &ltv[2], &ltv[3]);
						}
					}
				}
			}
		
		}

	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	GRenderer->SetRenderState(Renderer::DepthWrite, true);
	SetZBias(0);
	GRenderer->SetFogColor(ulBKGColor);
}

//***********************************************************************************************
// Draws a light source for EDITOR purpose...
//-----------------------------------------------------------------------------------------------
// VERIFIED (Cyril 2001/10/15)
//***********************************************************************************************
void EERIEDrawLight(EERIE_LIGHT * el) 
{
//	long i;
 
	TexturedVertex in;
	TexturedVertex center;
	GRenderer->SetCulling(Renderer::CullNone);
	
	if (el!=NULL)
	if (el->treat) 
	{
		el->mins.x=999999999.f;
		in.sx=el->pos.x;
		in.sy=el->pos.y;
		in.sz=el->pos.z;

		EERIEDrawSprite(&in, 11.f, lightsource_tc, el->rgb.to<u8>(), 2.f);
		if(ACTIVECAM->type != CAM_TOPVIEW) {
			
			el->mins = SPRmins;
			el->maxs = SPRmaxs;

			if (el->selected)
			{
				if ((el->mins.x>=-200.f) && (el->mins.x<=1000.f))
				if ((el->mins.y>=-200.f) && (el->mins.y<=1000.f))
				{
					in.sx=el->pos.x;
					in.sy=el->pos.y;
					in.sz=el->pos.z;
					EERIETreatPoint(&in,&center);	

					if ((center.sz>0.f) && (center.sz<1000.f))
					{
						float t=(1.f-center.sz)*ACTIVECAM->use_focal*( 1.0f / 3000 );
						float rad=el->fallstart*t;
						EERIEDrawCircle(center.sx, center.sy, rad, Color::yellow, 0.0001f);
						rad=el->fallend*t;
						EERIEDrawCircle(center.sx, center.sy, rad, Color::red, 0.0001f);
						rad=el->intensity*200.f*t;
						EERIEDrawCircle(center.sx, center.sy, rad, Color::green, 0.0001f);
					}
				}
			}			
		}
	}
}

void ARXDRAW_DrawAllLights(long x0,long z0,long x1,long z1) {
	for(size_t i = 0; i < MAX_LIGHTS; i++) {
		if (GLight[i]!=NULL)
		{
			
			long tx = GLight[i]->pos.x * ACTIVEBKG->Xmul;
			long tz = GLight[i]->pos.z * ACTIVEBKG->Zmul;
			GLight[i]->mins.x=9999999999.f;

			if ((tx>=x0) && (tx<=x1) &&
				(tz>=z0) && (tz<=z1)) 
			{
				GLight[i]->treat=1;

				if (ACTIVECAM->type!=CAM_TOPVIEW)
				{
					 EERIEDrawLight(GLight[i]);
				}
				else EERIEDrawLight(GLight[i]); 
			}
		}
	}
}
extern INTERACTIVE_OBJ * CAMERACONTROLLER;
extern long ARX_CONVERSATION;

void ARXDRAW_DrawEyeBall()
{
	Anglef angle;
	Vec3f pos;
	Vec3f scale;
	
	float d;

	if (eyeball.exist<0) 
	{
		d=(float)(-eyeball.exist)*( 1.0f / 100 );
		eyeball.exist++;		
	}
	else if (eyeball.exist>2) 
	{		
		d=(float)(eyeball.exist)*( 1.0f / 100 );
	}
	else return;

	angle.a = eyeball.angle.a; 
	angle.b=MAKEANGLE(180.f-eyeball.angle.b);
	angle.g=eyeball.angle.g;
	pos.x=eyeball.pos.x;
	pos.y=eyeball.pos.y+eyeball.floating;
	pos.z=eyeball.pos.z;
	scale.x=d;
	scale.y=d;
	scale.z=d;
	
	Color3f rgb(d, d, d);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	DrawEERIEObjEx(eyeballobj,&angle,&pos,&scale,&rgb);	
}
//*************************************************************************************
//*************************************************************************************
void IncrementPolyWithNormalOutput(EERIEPOLY *_pPoly,float _fFactor,TexturedVertex *_pOut)
{
	if(config.misc.forceZBias)
	{
		float t0=_pPoly->norm.x*_fFactor;
		float t1=_pPoly->norm.y*_fFactor;
		float t2=_pPoly->norm.z*_fFactor;
		_pOut[0].sx=_pPoly->v[0].sx+t0;
		_pOut[0].sy=_pPoly->v[0].sy+t1;
		_pOut[0].sz=_pPoly->v[0].sz+t2;
		_pOut[1].sx=_pPoly->v[1].sx+t0;
		_pOut[1].sy=_pPoly->v[1].sy+t1;
		_pOut[1].sz=_pPoly->v[1].sz+t2;
		_pOut[2].sx=_pPoly->v[2].sx+t0;
		_pOut[2].sy=_pPoly->v[2].sy+t1;
		_pOut[2].sz=_pPoly->v[2].sz+t2;

		if(_pPoly->type&POLY_QUAD)
		{
			_pOut[3].sx=_pPoly->v[3].sx+_pPoly->norm2.x*_fFactor;
			_pOut[3].sy=_pPoly->v[3].sy+_pPoly->norm2.y*_fFactor;
			_pOut[3].sz=_pPoly->v[3].sz+_pPoly->norm2.z*_fFactor;
		}
	}
	else
	{
		_pOut[0].sx=_pPoly->v[0].sx;
		_pOut[0].sy=_pPoly->v[0].sy;
		_pOut[0].sz=_pPoly->v[0].sz;

		_pOut[1].sx=_pPoly->v[1].sx;
		_pOut[1].sy=_pPoly->v[1].sy;
		_pOut[1].sz=_pPoly->v[1].sz;

		_pOut[2].sx=_pPoly->v[2].sx;
		_pOut[2].sy=_pPoly->v[2].sy;
		_pOut[2].sz=_pPoly->v[2].sz;

		if(_pPoly->type&POLY_QUAD)
		{
			_pOut[3].sx=_pPoly->v[3].sx;
			_pOut[3].sy=_pPoly->v[3].sy;
			_pOut[3].sz=_pPoly->v[3].sz;
		}

	}
}
extern float FrameDiff;
void EE_P2(TexturedVertex *in,TexturedVertex *out);
void ARXDRAW_DrawPolyBoom()
{
	TexturedVertex ltv[4];

	long i,k;
	float tt;

	SetZBias(8);
	GRenderer->SetFogColor(Color::none);
	unsigned long tim = ARXTimeUL();
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);

	for ( i = 0 ; i < MAX_POLYBOOM ; i++ )
	{
		if ( polyboom[i].exist )
		{
			if ( polyboom[i].type & 128 )
			{	
				if (polyboom[i].timecreation - FrameDiff > 0)
				{
					float fCalc	=	polyboom[i].timecreation - FrameDiff;	
					ARX_CHECK_ULONG( fCalc );
					polyboom[i].timecreation	=	ARX_CLEAN_WARN_CAST_ULONG( fCalc );
				}

				if (polyboom[i].timecreation - FrameDiff > 0)
				{
					float fCalc	= 	polyboom[i].timecreation - FrameDiff;	
					ARX_CHECK_ULONG( fCalc );
					polyboom[i].timecreation	=	ARX_CLEAN_WARN_CAST_ULONG( fCalc );
				}

			}

			float t	=	(float)polyboom[i].timecreation + (float)polyboom[i].tolive - (float)tim;

			if ( t <= 0 ) 
			{
				polyboom[i].exist=0;
				BoomCount--;
				continue;
			}
			
			if (Project.hide & HIDE_BACKGROUND) continue;
			else
			{
				long typp	=	polyboom[i].type;
				typp		&=	~128;

				switch (typp) 
				{
					case 0:	
					tt	=	(float)t / (float)polyboom[i].tolive * 0.8f;

					IncrementPolyWithNormalOutput(polyboom[i].ep,2.f,ltv);
					EE_RT2(&ltv[0],&ltv[0]);
					EE_RT2(&ltv[1],&ltv[1]);
					EE_RT2(&ltv[2],&ltv[2]);

					for(k=0;k<polyboom[i].nbvert;k++) {
						ltv[k].tu=polyboom[i].u[k];
						ltv[k].tv=polyboom[i].v[k];
						ltv[k].color = (Project.improve ? (Color3f::red * (tt*.5f)) : Color3f::gray(tt)).toBGR();
						ltv[k].specular = Color::black.toBGR();
					}

						if(Project.improve) {
							GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
						} else {
							GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);
						}

						GRenderer->SetTexture(0, Boom);
						ARX_DrawPrimitive(	&ltv[0],
														&ltv[1],
														&ltv[2]);

						if(polyboom[i].nbvert&4)
						{
							EE_RT2(&ltv[3],&ltv[3]);
							ARX_DrawPrimitive(	&ltv[1],
															&ltv[2],
															&ltv[3]);
						}
					break;
					case 1:	// Blood
					{
						float div=1.f/(float)polyboom[i].tolive;
						tt=(float)t*div;
						float tr = tt * 2 - 0.5f; 

						if (tr<1.f) tr=1.f;

						ColorBGRA col = (polyboom[i].rgb * tt).toBGR();

						for (k=0;k<polyboom[i].nbvert;k++) 
						{
							ltv[k].tu=(polyboom[i].u[k]-0.5f)*(tr)+0.5f;
							ltv[k].tv=(polyboom[i].v[k]-0.5f)*(tr)+0.5f;
							ltv[k].color=col;
							ltv[k].specular=0xFF000000;
						}	


							IncrementPolyWithNormalOutput(polyboom[i].ep,2.f,ltv);
							EE_RT2(&ltv[0],&ltv[0]);
							EE_RT2(&ltv[1],&ltv[1]);
							EE_RT2(&ltv[2],&ltv[2]);

							if(polyboom[i].nbvert&4)
							{
								EE_RT2(&ltv[3],&ltv[3]);
							}

								
							{
								
								GRenderer->GetTextureStage(0)->SetWrapMode(TextureStage::WrapClamp);
								GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
							GRenderer->SetTexture(0, polyboom[i].tc); 

 									ARX_DrawPrimitive(		&ltv[0],
 																		&ltv[1],
 																		&ltv[2]);

 									if(polyboom[i].nbvert&4)
 									{
  										ARX_DrawPrimitive(	&ltv[1],
  																		&ltv[2],
 																		&ltv[3]);
									}


								ltv[0].color = ltv[1].color = ltv[2].color = ltv[3].color = Color::gray(tt).toBGR();
								
								GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);

									ARX_DrawPrimitive(	&ltv[0],
																	&ltv[1],
																	&ltv[2]);

									if(polyboom[i].nbvert&4)
									{
										ARX_DrawPrimitive(	&ltv[1],
																		&ltv[2],
																		&ltv[3]);
									}
								
								GRenderer->GetTextureStage(0)->SetWrapMode(TextureStage::WrapRepeat);
							}
						}
					break;				
					case 2: // WATER
					{
						float div=1.f/(float)polyboom[i].tolive;
						tt=(float)t*div;
						float tr = (tt * 2 - 0.5f);

						if (tr<1.f) tr=1.f;

						float ttt=tt*0.5f;
						ColorBGRA col = (polyboom[i].rgb * ttt).toBGR();

						for(k=0;k<polyboom[i].nbvert;k++) {
							ltv[k].tu=(polyboom[i].u[k]-0.5f)*(tr)+0.5f;
							ltv[k].tv=(polyboom[i].v[k]-0.5f)*(tr)+0.5f;
							ltv[k].color=col;
							ltv[k].specular=0xFF000000;
						}	

						if (	(ltv[0].tu<0.f)
							&&	(ltv[1].tu<0.f)
							&&	(ltv[2].tu<0.f)
							&&	(ltv[3].tu<0.f) )
							break;

						if (	(ltv[0].tv<0.f)
							&&	(ltv[1].tv<0.f)
							&&	(ltv[2].tv<0.f)
							&&	(ltv[3].tv<0.f) )
							break;

						if (	(ltv[0].tu>1.f)
							&&	(ltv[1].tu>1.f)
							&&	(ltv[2].tu>1.f)
							&&	(ltv[3].tu>1.f) )
							break;

						if (	(ltv[0].tv>1.f)
							&&	(ltv[1].tv>1.f)
							&&	(ltv[2].tv>1.f)
							&&	(ltv[3].tv>1.f) )
							break;

							IncrementPolyWithNormalOutput(polyboom[i].ep,2.f,ltv);
							EE_RT2(&ltv[0],&ltv[0]);
							EE_RT2(&ltv[1],&ltv[1]);
							EE_RT2(&ltv[2],&ltv[2]);
								
								GRenderer->GetTextureStage(0)->SetWrapMode(TextureStage::WrapClamp);
								GRenderer->SetBlendFunc(Renderer::BlendInvDstColor, Renderer::BlendOne);
						GRenderer->SetTexture(0, polyboom[i].tc); 
				
								ARX_DrawPrimitive(	&ltv[0],
																&ltv[1],
																&ltv[2]);

								if(polyboom[i].nbvert&4)
								{
									EE_RT2(&ltv[3],&ltv[3]);
									ARX_DrawPrimitive(	&ltv[1],
																	&ltv[2],
																	&ltv[3]);
								}

						GRenderer->GetTextureStage(0)->SetWrapMode(TextureStage::WrapRepeat);
								
							}
					break;
				}
			}			
		}	
	}

	SetZBias(0);
	GRenderer->SetFogColor(ulBKGColor);
}
extern long TRANSPOLYSPOS;
extern EERIEPOLY * TransPol[MAX_TRANSPOL];

extern long INTERTRANSPOLYSPOS;
extern TexturedVertex InterTransPol[MAX_INTERTRANSPOL][4];
extern EERIE_FACE * InterTransFace[MAX_INTERTRANSPOL];
extern TextureContainer * InterTransTC[MAX_INTERTRANSPOL];

void ARXDRAW_DrawAllInterTransPolyPos()
{
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	EERIEDrawnPolys+=INTERTRANSPOLYSPOS;

	for (long i=0;i<INTERTRANSPOLYSPOS;i++) 
	{
		if (!InterTransFace[i]) continue; // Object was destroyed after sending faces...

		if (InterTransFace[i]->texid<0) continue;

		if (InterTransFace[i]->facetype & POLY_DOUBLESIDED) 
				GRenderer->SetCulling(Renderer::CullNone);
		else	GRenderer->SetCulling(Renderer::CullCW);

		GRenderer->SetTexture(0, InterTransTC[i]);
		EERIE_FACE * ef=InterTransFace[i];
		float ttt=ef->transval;

		if (ttt>=2.f)  //MULTIPLICATIVE
		{
			GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
			ttt*=( 1.0f / 2 );
			ttt+=0.5f;
			InterTransPol[i][2].color = InterTransPol[i][1].color = InterTransPol[i][0].color = Color::gray(ttt).toBGR();
		}
		else if (ttt>=1.f) //ADDITIVE
		{	
			ttt-=1.f;
			GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
			InterTransPol[i][2].color = InterTransPol[i][1].color = InterTransPol[i][0].color = Color::gray(ttt).toBGR();
		}
		else if (ttt>0.f)  //NORMAL TRANS
		{
			ttt=1.f-ttt;
			GRenderer->SetBlendFunc(Renderer::BlendDstColor, Renderer::BlendSrcColor);
			InterTransPol[i][2].color = InterTransPol[i][1].color = InterTransPol[i][0].color = Color::gray(ttt).toBGR(Color::Limits::max() * ttt);
		}
		else  //SUBTRACTIVE
		{
			GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);
			ttt=1.f-ttt;
			InterTransPol[i][2].color = InterTransPol[i][1].color = InterTransPol[i][0].color = Color::gray(ttt).toBGR();
		}

		EERIEDRAWPRIM(Renderer::TriangleStrip, InterTransPol[i], 3, true);
	}

	INTERTRANSPOLYSPOS=0;
}

extern TextureContainer * enviro;

void ARXDRAW_DrawAllTransPolysPos() {
	
	SetZBias( 1 );

	GRenderer->SetRenderState(Renderer::AlphaBlending, true);

	long i, to = 0; 

	register EERIEPOLY * ep;

	for ( i = 0 ; i < TRANSPOLYSPOS ; i++ ) 
	{
		ep = TransPol[i];

		if ( ( !( Project.hide & HIDE_BACKGROUND ) ) )
		{
			if ( ep->type & POLY_DOUBLESIDED ) GRenderer->SetCulling(Renderer::CullNone);
			else GRenderer->SetCulling(Renderer::CullCW);
		
			if ( ViewMode & VIEWMODE_FLAT )
				GRenderer->ResetTexture(0);
			else	
				GRenderer->SetTexture(0, ep->tex);

			if ( ep->type & POLY_QUAD ) to = 4;
			else to = 3;

			float ttt = ep->transval;

			if ( ttt >= 2.f )  //MULTIPLICATIVE
			{
				GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
				ttt	*= ( 1.0f / 2 );
				ttt	+= 0.5f;
				ep->tv[3].color = ep->tv[2].color = ep->tv[1].color = ep->tv[0].color = Color::gray(ttt).toBGR();
			}
			else if ( ttt >= 1.f ) //ADDITIVE
			{	
				ttt -= 1.f;
				GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
				ep->tv[3].color = ep->tv[2].color = ep->tv[1].color = ep->tv[0].color = Color::gray(ttt).toBGR();
			}
			else if ( ttt > 0.f )  //NORMAL TRANS
			{
				ttt = 1.f - ttt;
				GRenderer->SetBlendFunc(Renderer::BlendDstColor, Renderer::BlendSrcColor);
				ep->tv[3].color = ep->tv[2].color = ep->tv[1].color = ep->tv[0].color = Color::gray(ttt).toBGR(Color::Limits::max() * ttt);
			}
			else  //SUBTRACTIVE
			{
				GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);
				ttt = 1.f - ttt;
				ep->tv[3].color = ep->tv[2].color = ep->tv[1].color = ep->tv[0].color = Color::gray(ttt).toBGR();
			}

			EERIEDRAWPRIM(Renderer::TriangleStrip, ep->tv, to);

				if (ep->type & POLY_LAVA)
				{
					GRenderer->SetBlendFunc(Renderer::BlendDstColor, Renderer::BlendOne);	
					GRenderer->SetRenderState(Renderer::AlphaBlending, true);	
					TexturedVertex verts[4];
					GRenderer->SetTexture(0, enviro );

				ARX_CHECK(to > 0);

					for ( long j = 0 ; j < to ; j++ )
					{
						verts[j].sx		= ep->tv[j].sx;
						verts[j].sy		= ep->tv[j].sy;
						verts[j].sz		= ep->tv[j].sz;
						verts[j].rhw	= ep->tv[j].rhw;
						verts[j].color	= 0xFFFFFFFF;
						verts[j].tu		= ep->v[j].sx * ( 1.0f / 1000 ) + EEsin( ( ep->v[j].sx ) * ( 1.0f / 200 ) + (float) FrameTime * ( 1.0f / 2000 ) ) * ( 1.0f / 20 );
						verts[j].tv		= ep->v[j].sz * ( 1.0f / 1000 ) + EEcos( (ep->v[j].sz) * ( 1.0f / 200 ) + (float) FrameTime * ( 1.0f / 2000 ) ) * ( 1.0f / 20 );
					}	

					EERIEDRAWPRIM(Renderer::TriangleStrip, verts, to, true);

					for ( i = 0 ; i < to ; i++ )
					{
						verts[i].tu = ep->v[i].sx * ( 1.0f / 1000 ) + EEsin( ( ep->v[i].sx ) * ( 1.0f / 100 ) + (float)FrameTime * ( 1.0f / 2000 ) ) * ( 1.0f / 10 );
						verts[i].tv = ep->v[i].sz * ( 1.0f / 1000 ) + EEcos( ( ep->v[i].sz ) * ( 1.0f / 100 ) + (float)FrameTime * ( 1.0f / 2000 ) ) * ( 1.0f / 10 );
					}	
					EERIEDRAWPRIM(Renderer::TriangleStrip, verts, to, true);
					
					for ( i = 0 ; i < to ; i++ )
					{
							verts[i].tu		= ep->v[i].sx * ( 1.0f / 600 ) + EEsin ( ( ep->v[i].sx ) * ( 1.0f / 160 ) + (float)FrameTime * ( 1.0f / 2000 ) ) * ( 1.0f / 11 );
							verts[i].tv		= ep->v[i].sz * ( 1.0f / 600 ) + EEcos ( ( ep->v[i].sz ) * ( 1.0f / 160 ) + (float)FrameTime * ( 1.0f / 2000 ) ) * ( 1.0f / 11 );
							verts[i].color	= 0xFF666666;
					}	

					GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);
					EERIEDRAWPRIM(Renderer::TriangleStrip, verts, to, true);
				}
			}


		if ( ep->type & POLY_WATER )
		{
				GRenderer->SetBlendFunc(Renderer::BlendDstColor, Renderer::BlendOne);	
				GRenderer->SetRenderState(Renderer::AlphaBlending, true);	
				
				TexturedVertex verts[4];

				GRenderer->SetTexture(0, enviro );

			ARX_CHECK(to > 0);

				for ( long j = 0 ; j < to ; j++ )
				{
					verts[j].sx		= ep->tv[j].sx;
					verts[j].sy		= ep->tv[j].sy;
					verts[j].sz		= ep->tv[j].sz;
					verts[j].rhw	= ep->tv[j].rhw;
					verts[j].color	= 0xFF505050;
					verts[j].tu		= ep->v[j].sx * ( 1.0f / 1000 ) + EEsin( ( ep->v[j].sx ) * ( 1.0f / 200 ) + (float)FrameTime * ( 1.0f / 1000 ) ) * ( 1.0f / 32 );
					verts[j].tv		= ep->v[j].sz * ( 1.0f / 1000 ) + EEcos( ( ep->v[j].sz ) * ( 1.0f / 200 ) + (float)FrameTime * ( 1.0f / 1000 ) ) * ( 1.0f / 32 );

					if ( ep->type & POLY_FALL ) verts[j].tv += (float)FrameTime * ( 1.0f / 4000 );
				}

				EERIEDRAWPRIM(Renderer::TriangleStrip, verts, to, true);

				for ( i = 0 ; i < to ; i++ )
				{
					verts[i].tu = ( ep->v[i].sx + 30.f ) * ( 1.0f / 1000 ) + EEsin( ( ep->v[i].sx + 30 ) * ( 1.0f / 200 ) + (float)FrameTime * ( 1.0f / 1000 ) ) * ( 1.0f / 28 );
					verts[i].tv = ( ep->v[i].sz + 30.f ) * ( 1.0f / 1000 ) - EEcos( ( ep->v[i].sz + 30 ) * ( 1.0f / 200 ) + (float)FrameTime * ( 1.0f / 1000 ) ) * ( 1.0f / 28 );

					if ( ep->type & POLY_FALL ) verts[i].tv += (float)FrameTime * ( 1.0f / 4000 );
				}

				EERIEDRAWPRIM(Renderer::TriangleStrip, verts, to, true);

				for ( i = 0 ; i < to ; i++ )
				{
					verts[i].tu = ( ep->v[i].sx + 60.f ) * ( 1.0f / 1000 ) - EEsin( ( ep->v[i].sx + 60 ) * ( 1.0f / 200 ) + (float)FrameTime * ( 1.0f / 1000 ) ) * ( 1.0f / 40 );
					verts[i].tv = ( ep->v[i].sz + 60.f ) * ( 1.0f / 1000 ) - EEcos( ( ep->v[i].sz + 60 ) * ( 1.0f / 200 ) + (float)FrameTime * ( 1.0f / 1000 ) ) * ( 1.0f / 40 );

					if ( ep->type & POLY_FALL ) verts[i].tv += (float)FrameTime * ( 1.0f / 4000 );
				}	
				EERIEDRAWPRIM(Renderer::TriangleStrip, verts, to, true);
		}
	}

	SetZBias( 0 );
}

