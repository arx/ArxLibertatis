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

#include <ARX_Draw.h>

#include <ARX_Interactive.h>
#include <ARX_Particles.h>
#include <ARX_Spells.h>
#include <ARX_time.h>
#include "ARX_menu2.h"

#include <EERIEDraw.h>
#include <EERIELight.h>
#include "EERIEPoly.h"
#include "EERIEapp.h"

#include <stdio.h>
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

// Some external defs needing to be cleaned...
extern long DANAESIZX;
extern EERIE_3D SPRmins;
extern EERIE_3D SPRmaxs;
extern long MOVETYPE;
extern ANIM_HANDLE * herowait;
extern ANIM_HANDLE * herowalk;
extern ANIM_HANDLE * herorun;
extern EERIE_3DOBJ * eyeballobj; 
extern TextureContainer * Boom;
extern bool bZBUFFER;

POLYBOOM polyboom[MAX_POLYBOOM];

extern unsigned long ulBKGColor;
extern CMenuConfig *pMenuConfig;
extern bool bSoftRender;

void EE_RT2(D3DTLVERTEX*,D3DTLVERTEX*);
bool ARX_DrawPrimitive_SoftClippZ(D3DTLVERTEX*,D3DTLVERTEX*,D3DTLVERTEX*,float _fAdd=0.f);

//***********************************************************************************************
// hum... to be checked again for performance and result quality.
//-----------------------------------------------------------------------------------------------
// VERIFIED (Cyril 2001/10/15)
//***********************************************************************************************
void ARXDRAW_DrawInterShadows(LPDIRECT3DDEVICE7 pd3dDevice)
{	
	bool bNoVB						=	false;
	if( bSoftRender )
	{
		bNoVB						=	GET_FORCE_NO_VB();
		SET_FORCE_NO_VB( true );
	}

	GDevice->SetRenderState(D3DRENDERSTATE_FOGCOLOR,0);
	SetZBias(pd3dDevice,1);

	long k;
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
			F2L((io->pos.x)*ACTIVEBKG->Xmul,&xx);
			F2L((io->pos.z)*ACTIVEBKG->Zmul,&yy);

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
				D3DTLVERTEX in;			
				
				D3DTLVERTEX ltv[4];
				ltv[0]=	D3DTLVERTEX( D3DVECTOR( 0, 0, 0.001f ), 1.f, 0, 1, 0.3f, 0.3f) ;
				ltv[1]=	D3DTLVERTEX( D3DVECTOR( 0, 0, 0.001f ), 1.f, 0, 1, 0.7f, 0.3f) ;
				ltv[2]=	D3DTLVERTEX( D3DVECTOR( 0, 0, 0.001f ), 1.f, 0, 1, 0.7f, 0.7f) ;		
				ltv[3]=	D3DTLVERTEX( D3DVECTOR( 0, 0, 0.001f ), 1.f, 0, 1, 0.3f, 0.7f) ;
				
				float s1=16.f*io->scale;
				float s2=s1 * DIV2;	

				if (io->obj->nbgroups<=1)
				{
					for (k=0;k<io->obj->nbvertex;k+=9)
					{
						ep=EECheckInPoly(&io->obj->vertexlist3[k].v);

						if (ep!=NULL)
						{
							in.sy=ep->min.y-3.f;
							float r=0.5f-((float)EEfabs(io->obj->vertexlist3[k].v.y-in.sy))*DIV500;
							r-=io->invisibility;
							r*=io->scale;

							if (r<=0.f) continue;
							
							in.sx=io->obj->vertexlist3[k].v.x-s2;						
							in.sz=io->obj->vertexlist3[k].v.z-s2;

							long lv;
							r*=255.f;
							F2L(r,&lv);
							ltv[0].color=ltv[1].color=ltv[2].color=ltv[3].color=0xFF000000 | lv<<16 | lv<<8 | lv;
							
							if (first)
							{
								first=0;
								SETZWRITE(pd3dDevice, FALSE );
								SETBLENDMODE(pd3dDevice,D3DBLEND_ZERO,D3DBLEND_INVSRCCOLOR);
								SETALPHABLEND(pd3dDevice,TRUE);
								SETTC(pd3dDevice,Boom);
							}

							EE_RT2(&in,&ltv[0]);
							in.sx+=s1;
							EE_RT2(&in,&ltv[1]);
							in.sz+=s1;
							EE_RT2(&in,&ltv[2]);
							in.sx-=s1;
							EE_RT2(&in,&ltv[3]);

							if ((ltv[0].sz>0.f) && (ltv[1].sz>0.f) && (ltv[2].sz>0.f))
							{
								ARX_DrawPrimitive_SoftClippZ(	&ltv[0],
															&ltv[1],
															&ltv[2],
															50.f);
								ARX_DrawPrimitive_SoftClippZ(	&ltv[0],
															&ltv[2],
															&ltv[3],
															50.f);
							}
						}
					}	
				}
				else 
				{
					for (k=0;k<io->obj->nbgroups;k++)
					{
						long origin=io->obj->grouplist[k].origin;
						ep=EECheckInPoly(	&io->obj->vertexlist3[origin].v );

						if (ep!=NULL)
						{
							in.sy=ep->min.y-3.f;
							float r=0.8f-((float)EEfabs(io->obj->vertexlist3[origin].v.y-in.sy))*DIV500;
							r*=io->obj->grouplist[k].siz;
							r-=io->invisibility;

							if (r<=0.f) continue;

							float s1=io->obj->grouplist[k].siz*44.f;
							float s2=s1*DIV2;
							in.sx=io->obj->vertexlist3[origin].v.x-s2;						
							in.sz=io->obj->vertexlist3[origin].v.z-s2;

							long lv;
							r*=255.f;
							F2L(r,&lv);						
							ltv[0].color=	ltv[1].color	=	ltv[2].color	=	ltv[3].color	=	0xFF000000 | lv<<16 | lv<<8 | lv;

							if (first)
							{
								first=0;
								SETZWRITE(pd3dDevice, FALSE );
								SETBLENDMODE(pd3dDevice,D3DBLEND_ZERO,D3DBLEND_INVSRCCOLOR);
								SETALPHABLEND(pd3dDevice,TRUE);
								SETTC(pd3dDevice,Boom);
							}

							EE_RT2(&in,&ltv[0]);
							in.sx+=s1;
							EE_RT2(&in,&ltv[1]);
							in.sz+=s1;
							EE_RT2(&in,&ltv[2]);
							in.sx-=s1;
							EE_RT2(&in,&ltv[3]);
							ARX_DrawPrimitive_SoftClippZ(	&ltv[0],
															&ltv[1],
															&ltv[2],
															50.f);
							ARX_DrawPrimitive_SoftClippZ(	&ltv[0],
															&ltv[2],
															&ltv[3],
															50.f);
						}
					}
				}
			}
		
		}

	SETALPHABLEND(pd3dDevice,FALSE);
	SETZWRITE(pd3dDevice, TRUE );
	SetZBias(pd3dDevice,0);
	GDevice->SetRenderState(D3DRENDERSTATE_FOGCOLOR,ulBKGColor);
	
	if( bSoftRender ) SET_FORCE_NO_VB( bNoVB );
}

//***********************************************************************************************
// Draws a light source for EDITOR purpose...
//-----------------------------------------------------------------------------------------------
// VERIFIED (Cyril 2001/10/15)
//***********************************************************************************************
void EERIEDrawLight(LPDIRECT3DDEVICE7 pd3dDevice,EERIE_LIGHT * el) 
{
//	long i;
 
	D3DTLVERTEX in;
	D3DTLVERTEX center;
	SETCULL(pd3dDevice,D3DCULL_NONE);
	
	if (el!=NULL)
	if (el->treat) 
	{
		el->mins.x=999999999.f;
		in.sx=el->pos.x;
		in.sy=el->pos.y;
		in.sz=el->pos.z;

		if (ACTIVECAM->type==CAM_TOPVIEW)
			EERIEDrawSprite(pd3dDevice,&in,11.f,lightsource_tc,EERIERGB(el->rgb.r,el->rgb.g,el->rgb.b),2.f);
		else 
		{
			EERIEDrawSprite(pd3dDevice,&in,11.f,lightsource_tc,EERIERGB(el->rgb.r,el->rgb.g,el->rgb.b),2.f);
			memcpy(&el->mins,&SPRmins,sizeof(EERIE_3D));
			memcpy(&el->maxs,&SPRmaxs,sizeof(EERIE_3D));

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
						float t=(1.f-center.sz)*ACTIVECAM->use_focal*DIV3000;
						float rad=el->fallstart*t;
						EERIEDrawCircle(center.sx,center.sy,rad,0xFFFFFF00,0.0001f);
						rad=el->fallend*t;
						EERIEDrawCircle(center.sx,center.sy,rad,0xFFFF0000,0.0001f);
						rad=el->intensity*200.f*t;
						EERIEDrawCircle(center.sx,center.sy,rad,0xFF00FF00,0.0001f);
					}
				}
			}			
		}
	}
}

//*************************************************************************************
//*************************************************************************************
void ARXDRAW_DrawAllLights(LPDIRECT3DDEVICE7 pd3dDevice,long x0,long z0,long x1,long z1)
{
	long i,tx,tz;

	for (i=0;i<MAX_LIGHTS;i++) 
	{	
		if (GLight[i]!=NULL)
		{
			
			F2L(GLight[i]->pos.x*ACTIVEBKG->Xmul,&tx);
			F2L(GLight[i]->pos.z*ACTIVEBKG->Zmul,&tz);
			GLight[i]->mins.x=9999999999.f;

			if ((tx>=x0) && (tx<=x1) &&
				(tz>=z0) && (tz<=z1)) 
			{
				GLight[i]->treat=1;

				if (ACTIVECAM->type!=CAM_TOPVIEW)
				{
					 EERIEDrawLight(pd3dDevice,GLight[i]);
				}
				else EERIEDrawLight(pd3dDevice,GLight[i]); 
			}
		}
	}
}
extern INTERACTIVE_OBJ * CAMERACONTROLLER;
extern long ARX_CONVERSATION;
//*************************************************************************************
//*************************************************************************************
void ARXDRAW_DrawExternalView(LPDIRECT3DDEVICE7 pd3dDevice)
{
}
//*************************************************************************************
//*************************************************************************************
void ARXDRAW_DrawEyeBall(LPDIRECT3DDEVICE7 pd3dDevice)
{
	EERIE_3D angle;
	EERIE_3D pos;
	EERIE_3D scale;
	EERIE_RGB rgb;
	
	float d;

	if (eyeball.exist<0) 
	{
		d=(float)(-eyeball.exist)*DIV100;
		eyeball.exist++;		
	}
	else if (eyeball.exist>2) 
	{		
		d=(float)(eyeball.exist)*DIV100;
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
	rgb.r=d;
	rgb.g=d;
	rgb.b=d;
	SETBLENDMODE(pd3dDevice,D3DBLEND_ONE,D3DBLEND_ONE);
	SETALPHABLEND(pd3dDevice,TRUE);
	DrawEERIEObjEx(pd3dDevice,eyeballobj,&angle,&pos,&scale,&rgb);	
}
//*************************************************************************************
//*************************************************************************************
void IncrementPolyWithNormalOutput(EERIEPOLY *_pPoly,float _fFactor,D3DTLVERTEX *_pOut)
{
	if(	(pMenuConfig)&&
		(pMenuConfig->bForceZBias) )
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
void EE_P2(D3DTLVERTEX *in,D3DTLVERTEX *out);
void ARXDRAW_DrawPolyBoom(LPDIRECT3DDEVICE7 pd3dDevice)
{
	D3DTLVERTEX ltv[4];

	long i,k;
	float tt;

	SetZBias(pd3dDevice,8);
	GDevice->SetRenderState(D3DRENDERSTATE_FOGCOLOR,0);
	unsigned long tim = ARXTimeUL(); 	
	SETALPHABLEND(pd3dDevice,TRUE);

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

					for (k=0;k<polyboom[i].nbvert;k++) 
					{
						ltv[k].tu=polyboom[i].u[k];
						ltv[k].tv=polyboom[i].v[k];

						if (Project.improve) ltv[k].color=EERIERGB(tt*DIV2,0.f,0.f);
						else ltv[k].color=_EERIERGB(tt);

						ltv[k].specular=0xFF000000;
					}				

						if (Project.improve) 
						{
							SETBLENDMODE(pd3dDevice,D3DBLEND_ONE,D3DBLEND_ONE);
						}
						else  
						{
							SETBLENDMODE(pd3dDevice,D3DBLEND_ZERO,D3DBLEND_INVSRCCOLOR);
						}

						SETTC(pd3dDevice,Boom);
						ARX_DrawPrimitive_SoftClippZ(	&ltv[0],
														&ltv[1],
														&ltv[2]);

						if(polyboom[i].nbvert&4)
						{
							EE_RT2(&ltv[3],&ltv[3]);
							ARX_DrawPrimitive_SoftClippZ(	&ltv[1],
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

						D3DCOLOR col=EERIERGB(polyboom[i].rgb.r*tt,polyboom[i].rgb.g*tt,polyboom[i].rgb.b*tt);

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
								
								SETTEXTUREWRAPMODE(pd3dDevice,D3DTADDRESS_CLAMP);
								SETBLENDMODE(pd3dDevice,D3DBLEND_ONE,D3DBLEND_ONE);
							SETTC(pd3dDevice, polyboom[i].tc); 

 									ARX_DrawPrimitive_SoftClippZ(		&ltv[0],
 																		&ltv[1],
 																		&ltv[2]);

 									if(polyboom[i].nbvert&4)
 									{
  										ARX_DrawPrimitive_SoftClippZ(	&ltv[1],
  																		&ltv[2],
 																		&ltv[3]);
									}


								col=_EERIERGB(tt);
								ltv[0].color=ltv[1].color=ltv[2].color=ltv[3].color=col;								
								
								SETBLENDMODE(pd3dDevice,D3DBLEND_ZERO,D3DBLEND_INVSRCCOLOR);

									ARX_DrawPrimitive_SoftClippZ(	&ltv[0],
																	&ltv[1],
																	&ltv[2]);

									if(polyboom[i].nbvert&4)
									{
										ARX_DrawPrimitive_SoftClippZ(	&ltv[1],
																		&ltv[2],
																		&ltv[3]);
									}
								
								SETTEXTUREWRAPMODE(pd3dDevice,D3DTADDRESS_WRAP);
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
						D3DCOLOR col=EERIERGB(	polyboom[i].rgb.r*ttt,
												polyboom[i].rgb.g*ttt,
												polyboom[i].rgb.b*ttt);

						for (k=0;k<polyboom[i].nbvert;k++) 
						{
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
								
								SETTEXTUREWRAPMODE(pd3dDevice,D3DTADDRESS_CLAMP);
								SETBLENDMODE(pd3dDevice,D3DBLEND_INVDESTCOLOR,D3DBLEND_ONE);
						SETTC(pd3dDevice, polyboom[i].tc); 
				
								ARX_DrawPrimitive_SoftClippZ(	&ltv[0],
																&ltv[1],
																&ltv[2]);

								if(polyboom[i].nbvert&4)
								{
									EE_RT2(&ltv[3],&ltv[3]);
									ARX_DrawPrimitive_SoftClippZ(	&ltv[1],
																	&ltv[2],
																	&ltv[3]);
								}

						SETTEXTUREWRAPMODE(pd3dDevice, D3DTADDRESS_WRAP);
								
							}
					break;
				}
			}			
		}	
	}

	SetZBias(pd3dDevice,0);
	GDevice->SetRenderState(D3DRENDERSTATE_FOGCOLOR,ulBKGColor);
}
extern long TRANSPOLYSPOS;
extern EERIEPOLY * TransPol[MAX_TRANSPOL];

extern long INTERTRANSPOLYSPOS;
extern D3DTLVERTEX InterTransPol[MAX_INTERTRANSPOL][4];
extern EERIE_FACE * InterTransFace[MAX_INTERTRANSPOL];
extern TextureContainer * InterTransTC[MAX_INTERTRANSPOL];

void ARXDRAW_DrawAllInterTransPolyPos(LPDIRECT3DDEVICE7 pd3dDevice)
{
	SETALPHABLEND(pd3dDevice,TRUE);
	EERIEDrawnPolys+=INTERTRANSPOLYSPOS;

	for (long i=0;i<INTERTRANSPOLYSPOS;i++) 
	{
		if (!InterTransFace[i]) continue; // Object was destroyed after sending faces...

		if (InterTransFace[i]->texid<0) continue;

		if (InterTransFace[i]->facetype & POLY_DOUBLESIDED) 
				SETCULL(pd3dDevice,D3DCULL_NONE);
		else	SETCULL(pd3dDevice,D3DCULL_CW);

		SETTC(pd3dDevice,InterTransTC[i]);
		EERIE_FACE * ef=InterTransFace[i];
		float ttt=ef->transval;

		if (ttt>=2.f)  //MULTIPLICATIVE
		{
			SETBLENDMODE(pd3dDevice,D3DBLEND_ONE,D3DBLEND_ONE);
			ttt*=DIV2;
			ttt+=0.5f;
			InterTransPol[i][2].color=InterTransPol[i][1].color=InterTransPol[i][0].color=_EERIERGB(ttt);
		}
		else if (ttt>=1.f) //ADDITIVE
		{	
			ttt-=1.f;
			SETBLENDMODE(pd3dDevice,D3DBLEND_ONE,D3DBLEND_ONE);
			InterTransPol[i][2].color=InterTransPol[i][1].color=InterTransPol[i][0].color=_EERIERGB(ttt);
		}
		else if (ttt>0.f)  //NORMAL TRANS
		{
			ttt=1.f-ttt;
			SETBLENDMODE(pd3dDevice,D3DBLEND_DESTCOLOR,D3DBLEND_SRCCOLOR);
			InterTransPol[i][2].color=InterTransPol[i][1].color=InterTransPol[i][0].color=_EERIERGBA(ttt);//ttt);
		}
		else  //SUBTRACTIVE
		{
			SETBLENDMODE(pd3dDevice,D3DBLEND_ZERO,D3DBLEND_INVSRCCOLOR);
			ttt=1.f-ttt;
			InterTransPol[i][2].color=InterTransPol[i][1].color=InterTransPol[i][0].color=_EERIERGB(ttt);
		}

		EERIEDRAWPRIM(pd3dDevice, D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX| D3DFVF_DIFFUSE , InterTransPol[i], 3,  0, EERIE_NOCOUNT | (bSoftRender?EERIE_USEVB:0) );
	}

	INTERTRANSPOLYSPOS=0;
}

extern TextureContainer * enviro;

void ARXDRAW_DrawAllTransPolysPos( LPDIRECT3DDEVICE7 pd3dDevice, long MODIF )
{
	int flg_NOCOUNT_USEVB = EERIE_NOCOUNT | (bSoftRender?EERIE_USEVB:0);
	SetZBias( pd3dDevice, 1 );

	SETALPHABLEND( pd3dDevice, TRUE );

	long i, to = 0; 

	register EERIEPOLY * ep;

	for ( i = 0 ; i < TRANSPOLYSPOS ; i++ ) 
	{
		ep = TransPol[i];

		if ( ( !( Project.hide & HIDE_BACKGROUND ) ) )
		{
			if ( ep->type & POLY_DOUBLESIDED ) SETCULL( pd3dDevice, D3DCULL_NONE );
			else SETCULL( pd3dDevice, D3DCULL_CW );
		
			if ( ViewMode & VIEWMODE_FLAT ) SETTC( pd3dDevice, NULL );
			else	SETTC( pd3dDevice, ep->tex );

			if ( ep->type & POLY_QUAD ) to = 4;
			else to = 3;

			float ttt = ep->transval;

			if ( ttt >= 2.f )  //MULTIPLICATIVE
			{
				SETBLENDMODE( pd3dDevice, D3DBLEND_ONE, D3DBLEND_ONE );
				ttt	*= DIV2;
				ttt	+= 0.5f;
				ep->tv[3].color = ep->tv[2].color = ep->tv[1].color = ep->tv[0].color = _EERIERGB( ttt );
			}
			else if ( ttt >= 1.f ) //ADDITIVE
			{	
				ttt -= 1.f;
				SETBLENDMODE( pd3dDevice, D3DBLEND_ONE, D3DBLEND_ONE );
				ep->tv[3].color = ep->tv[2].color = ep->tv[1].color = ep->tv[0].color = _EERIERGB( ttt );
			}
			else if ( ttt > 0.f )  //NORMAL TRANS
			{
				ttt = 1.f - ttt;
				SETBLENDMODE( pd3dDevice, D3DBLEND_DESTCOLOR, D3DBLEND_SRCCOLOR );
				ep->tv[3].color = ep->tv[2].color = ep->tv[1].color = ep->tv[0].color = _EERIERGBA(ttt);  
			}
			else  //SUBTRACTIVE
			{
				SETBLENDMODE( pd3dDevice, D3DBLEND_ZERO, D3DBLEND_INVSRCCOLOR );
				ttt = 1.f - ttt;
				ep->tv[3].color = ep->tv[2].color = ep->tv[1].color = ep->tv[0].color = _EERIERGB( ttt );
			}

			EERIEDRAWPRIM( pd3dDevice, D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX | D3DFVF_DIFFUSE , ep->tv, to,  0, bSoftRender?EERIE_USEVB:0  );

				if (ep->type & POLY_LAVA)
				{
					pd3dDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND,  D3DBLEND_DESTCOLOR );
					pd3dDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE );	
					SETALPHABLEND( pd3dDevice, TRUE );	
					D3DTLVERTEX verts[4];
					SETTC( pd3dDevice, enviro );

				ARX_CHECK(to > 0);

					for ( long j = 0 ; j < to ; j++ )
					{
						verts[j].sx		= ep->tv[j].sx;
						verts[j].sy		= ep->tv[j].sy;
						verts[j].sz		= ep->tv[j].sz;
						verts[j].rhw	= ep->tv[j].rhw;
						verts[j].color	= 0xFFFFFFFF;
						verts[j].tu		= ep->v[j].sx * DIV1000 + EEsin( ( ep->v[j].sx ) * DIV200 + (float) FrameTime * DIV2000 ) * DIV20;
						verts[j].tv		= ep->v[j].sz * DIV1000 + EEcos( (ep->v[j].sz) * DIV200 + (float) FrameTime * DIV2000 ) * DIV20;
					}	

					EERIEDRAWPRIM(pd3dDevice, D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX | D3DFVF_DIFFUSE, verts, to, 0, flg_NOCOUNT_USEVB );

					for ( i = 0 ; i < to ; i++ )
					{
						verts[i].tu = ep->v[i].sx * DIV1000 + EEsin( ( ep->v[i].sx ) * DIV100 + (float)FrameTime * DIV2000 ) * DIV10;
						verts[i].tv = ep->v[i].sz * DIV1000 + EEcos( ( ep->v[i].sz ) * DIV100 + (float)FrameTime * DIV2000 ) * DIV10;
					}	
					EERIEDRAWPRIM(pd3dDevice, D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX | D3DFVF_DIFFUSE, verts, to, 0, flg_NOCOUNT_USEVB );
					
					for ( i = 0 ; i < to ; i++ )
					{
							verts[i].tu		= ep->v[i].sx * DIV600 + EEsin ( ( ep->v[i].sx ) * DIV160 + (float)FrameTime * DIV2000 ) * DIV11;
							verts[i].tv		= ep->v[i].sz * DIV600 + EEcos ( ( ep->v[i].sz ) * DIV160 + (float)FrameTime * DIV2000 ) * DIV11;
							verts[i].color	= 0xFF666666;
					}	

					pd3dDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ZERO );
					pd3dDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCCOLOR );
					EERIEDRAWPRIM(pd3dDevice, D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX | D3DFVF_DIFFUSE, verts, to, 0, flg_NOCOUNT_USEVB );
				}
			}


		if ( ep->type & POLY_WATER )
		{
				pd3dDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND,  D3DBLEND_DESTCOLOR );
				pd3dDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE );	

				SETALPHABLEND( pd3dDevice, TRUE );	
				
				D3DTLVERTEX verts[4];

				SETTC( pd3dDevice, enviro );

			ARX_CHECK(to > 0);

				for ( long j = 0 ; j < to ; j++ )
				{
					verts[j].sx		= ep->tv[j].sx;
					verts[j].sy		= ep->tv[j].sy;
					verts[j].sz		= ep->tv[j].sz;
					verts[j].rhw	= ep->tv[j].rhw;
					verts[j].color	= 0xFF505050;
					verts[j].tu		= ep->v[j].sx * DIV1000 + EEsin( ( ep->v[j].sx ) * DIV200 + (float)FrameTime * DIV1000 ) * DIV32;
					verts[j].tv		= ep->v[j].sz * DIV1000 + EEcos( ( ep->v[j].sz ) * DIV200 + (float)FrameTime * DIV1000 ) * DIV32;

					if ( ep->type & POLY_FALL ) verts[j].tv += (float)FrameTime * DIV4000;
				}

				EERIEDRAWPRIM(pd3dDevice, D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX | D3DFVF_DIFFUSE, verts, to, 0, flg_NOCOUNT_USEVB );

				for ( i = 0 ; i < to ; i++ )
				{
					verts[i].tu = ( ep->v[i].sx + 30.f ) * DIV1000 + EEsin( ( ep->v[i].sx + 30 ) * DIV200 + (float)FrameTime * DIV1000 ) * DIV28;
					verts[i].tv = ( ep->v[i].sz + 30.f ) * DIV1000 - EEcos( ( ep->v[i].sz + 30 ) * DIV200 + (float)FrameTime * DIV1000 ) * DIV28;

					if ( ep->type & POLY_FALL ) verts[i].tv += (float)FrameTime * DIV4000;
				}

				EERIEDRAWPRIM(pd3dDevice, D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX | D3DFVF_DIFFUSE, verts, to, 0, flg_NOCOUNT_USEVB );

				for ( i = 0 ; i < to ; i++ )
				{
					verts[i].tu = ( ep->v[i].sx + 60.f ) * DIV1000 - EEsin( ( ep->v[i].sx + 60 ) * DIV200 + (float)FrameTime * DIV1000 ) * DIV40;
					verts[i].tv = ( ep->v[i].sz + 60.f ) * DIV1000 - EEcos( ( ep->v[i].sz + 60 ) * DIV200 + (float)FrameTime * DIV1000 ) * DIV40;

					if ( ep->type & POLY_FALL ) verts[i].tv += (float)FrameTime * DIV4000;
				}	
				EERIEDRAWPRIM(pd3dDevice, D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX | D3DFVF_DIFFUSE, verts, to, 0, flg_NOCOUNT_USEVB );
		}
	}

	SetZBias( pd3dDevice, 0 );
}

