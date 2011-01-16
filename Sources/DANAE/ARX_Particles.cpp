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
// ARX_Particles
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		ARX Particles Management
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

//#include "danae.h"
#include "ARX_Particles.h"
#include "ARX_Input.h"
#include "ARX_CSpellFx.h"
#include "ARX_Sound.h"
#include "ARX_Collisions.h"
#include "ARX_Interface.h" // -> à virer
#include "ARX_Menu2.h"
#include "ARX_MenuPublic.h"
#include "ARX_Paths.h"
#include "ARX_Time.h"
#include "ARX_Damages.h"
#include "ARX_Scene.h"

#include "EERIEMath.h"
#include "EERIEDraw.h"
#include "EERIEObject.h"
#include "EERIEPhysicsBox.h"
extern CMenuConfig *pMenuConfig;
#include <stdio.h>
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

extern float fZFogEnd;
extern unsigned long ulBKGColor;
extern bool bSoftRender;

//-----------------------------------------------------------------------------
typedef struct
{
	EERIE_3D	pos;
	EERIE_3D	move;
	EERIE_3D	scale;
	EERIE_RGB	fade;
	EERIE_3DOBJ * obj;
	long		special;
	EERIE_3D	spe[8];
	EERIE_3D	speinc[8];
	unsigned long	tim_start;
	unsigned long	duration;
	BOOL		exist;
	long		dynlight;
} OBJFX;

FLARETC			flaretc;
PARTICLE_DEF	particle[MAX_PARTICLES];
BOOM			booms[MAX_BOOMS];
FLARES			flare[MAX_FLARES];
D3DTLVERTEX		g_Lumignon[4];
TextureContainer *bloodsplat[6];
TextureContainer *water_splat[3];
TextureContainer *water_drop[3];
TextureContainer * smokeparticle=NULL;
TextureContainer * bloodsplatter=NULL;
TextureContainer * healing=NULL;
TextureContainer * tzupouf=NULL;
TextureContainer * fire2=NULL;

OBJFX			objfx[MAX_OBJFX];
long			BoomCount=0;
 
long			flarenum=0;
long			ParticleCount=0;
short			OPIPOrgb=0;
short			PIPOrgb=0;
short			shinum=1;
long			NewSpell=0;
TextureContainer	*GTC;
float				ET_R,ET_G,ET_B;
int					ET_MASK;
long			HIDEMAGICDUST=0;

//-----------------------------------------------------------------------------
long ARX_BOOMS_GetFree()
{
	for(long i=0;i<MAX_POLYBOOM;i++) 
	{
		if (!polyboom[i].exist) 
			return i;
	}

	return -1;
}

//-----------------------------------------------------------------------------
void SpawnMetalShine(EERIE_3D * pos,long r,long g,long b,long num)
{
	return;

	if (num<0) return;

	long j=ARX_PARTICLES_GetFree();

	if ((j!=-1) && (!ARXPausedTimer))
	{
		ParticleCount++;
		PARTICLE_DEF * pd=&particle[j];
		pd->exist		=	TRUE;
		pd->zdec		=	0;
		pd->ov.x		=	pos->x;
		pd->ov.y		=	pos->y;
		pd->ov.z		=	pos->z;
		pd->move.x		=	0.f;
		pd->move.y		=	0.f;
		pd->move.z		=	0.f;
		pd->timcreation	=	lARXTime;
		pd->tolive		=	(unsigned long)(500.0F + rnd() * 400.0F);
		long num		=	(long)(rnd() * 10.f);

		if (num<1) num = 1;

		if (num>9) num = 9;

		pd->tc			=	flaretc.shine[num];
		pd->siz			=	4.f+rnd()*2.f;
		pd->scale.x		=	-15.f;
		pd->scale.y		=	-15.f;
		pd->scale.z		=	-15.f;
		pd->r			=	(float)r*DIV64;
		pd->g			=	(float)g*DIV64;
		pd->b			=	(float)b*DIV64;
		pd->special		=	ROTATING | MODULATE_ROTATION | FADE_IN_AND_OUT | FOLLOW_SOURCE;
		pd->fparam		=	0.001f;
		pd->source		=	pos;
		pd->sourceionum	=	num;
	}
}

//-----------------------------------------------------------------------------
void LaunchDummyParticle()
{
	long j=ARX_PARTICLES_GetFree();

	if (	(j!=-1)
		&&	(!ARXPausedTimer)	)
	{
		ParticleCount++;
		PARTICLE_DEF * pd=&particle[j];
		pd->exist=TRUE;
		pd->zdec=0;
		float f=DEG2RAD(player.angle.b);
		pd->ov.x		=	player.pos.x+EEsin(f)*100.f;
		pd->ov.y		=	player.pos.y;
		pd->ov.z		=	player.pos.z-EEcos(f)*100.f;
		pd->move.x		=	0.f;
		pd->move.y		=	0.f;
		pd->move.z		=	0.f;
		pd->timcreation	=	lARXTime;
		pd->tolive		=	600;
		pd->tc			=	smokeparticle;
		pd->siz			=	15.f;
		pd->scale.x		=	-15.f+rnd()*5.f;
		pd->scale.y		=	-15.f+rnd()*5.f;
		pd->scale.z		=	-15.f+rnd()*5.f;
		pd->special		=	FIRE_TO_SMOKE;
	}
}

//-----------------------------------------------------------------------------
void ARX_PARTICLES_Spawn_Lava_Burn(EERIE_3D * poss,float power,INTERACTIVE_OBJ * io)
{
	EERIE_3D pos;
	pos.x=poss->x;
	pos.y=poss->y;
	pos.z=poss->z;

	if (	(io)
		&&	(io->obj)
		&&	(io->obj->nbfaces)	)
	{
		long notok	=	10;
		long num	=	0;

		while ( notok-- )
		{
			F2L( (float)( rnd() * (float)io->obj->nbfaces ), &num );

			if ( ( num >= 0 ) && ( num < io->obj->nbfaces ) )
			{
				if ( io->obj->facelist[num].facetype & POLY_HIDE ) continue;

				if ( EEfabs( pos.y-io->obj->vertexlist3[io->obj->facelist[num].vid[0]].v.y ) > 50.f )
					continue;

				notok = 0;
			}
		}

		if ((notok >= 0) && 
			( notok < 10 ) )
		{
			EERIE_3D * v	=	&io->obj->vertexlist3[io->obj->facelist[num].vid[0]].v;
			pos.x			=	v->x;
			pos.y			=	v->y;
			pos.z			=	v->z;
		}
	}

	long j = ARX_PARTICLES_GetFree();

	if (	( j != -1 )
		&&	( !ARXPausedTimer )	)
	{
		ParticleCount++;
		PARTICLE_DEF * pd	=	&particle[j];
		pd->exist			=	TRUE;
		pd->zdec			=	0;
		pd->ov.x			=	pos.x;
		pd->ov.y			=	pos.y;
		pd->ov.z			=	pos.z;
		pd->move.x			=	rnd() * 2 - 4.f;
		pd->move.y			=	rnd() * - 12.f - 15;
		pd->move.z			=	rnd() * 2 - 4.f;
		pd->timcreation		=	lARXTime;
		pd->tolive			=	800;
		pd->tc				=	smokeparticle;
		pd->siz				=	15.f;
		pd->scale.x			=	15.f + rnd() * 5.f;
		pd->scale.y			=	15.f + rnd() * 5.f;
		pd->scale.z			=	15.f + rnd() * 5.f;
		pd->special			=	FIRE_TO_SMOKE;

		if ( rnd() > 0.5f ) pd->special |= SUBSTRACT;
	}
}

extern long GORE_MODE;
//-----------------------------------------------------------------------------
void ARX_PARTICLES_Spawn_Rogue_Blood(EERIE_3D * pos,float dmgs,D3DCOLOR col)
{
	long j = ARX_PARTICLES_GetFree();

	if (	(j!=-1)
		&&	(!ARXPausedTimer)	)
	{
		float power;
		power=(dmgs*DIV60)+0.9f;
		float r,g,b;
		r=(float)((long)((col>>16) & 255))*DIV255;
		g=(float)((long)((col>>8) & 255))*DIV255;
		b=(float)((long)((col) & 255))*DIV255;
		ParticleCount++;
		PARTICLE_DEF * pd=&particle[j];
		pd->exist=TRUE;
		pd->zdec=0;
		pd->ov.x=pos->x;
		pd->ov.y=pos->y;
		pd->ov.z=pos->z;
			
		pd->siz			=	3.1f*power;
		pd->scale.z		=	pd->scale.y		=	pd->scale.x		=	-pd->siz*DIV4;			
		pd->timcreation	=	lARXTime;
		pd->special		=	PARTICLE_SUB2 | SUBSTRACT | GRAVITY | ROTATING | MODULATE_ROTATION;
		pd->special		|=	SPLAT_GROUND;
		pd->tolive		=	1600;
		pd->delay		=	0;
		pd->move.x		=	rnd()*60.f-30.f;
		pd->move.y		=	rnd()*-10.f-15.f;
		pd->move.z		=	rnd()*60.f-30.f;
		pd->r			=	r;
		pd->g			=	g;
		pd->b			=	b;
		long num;
		F2L((float)(rnd()*6.f),&num);

		if (num<0) num=0;
		else if (num>5) num=5;

		pd->tc=bloodsplat[num];
		pd->fparam=rnd()*DIV10-0.05f;
	}
	
}
//-----------------------------------------------------------------------------
void ARX_PARTICLES_Spawn_Blood3(EERIE_3D * pos,float dmgs,D3DCOLOR col,long vert,INTERACTIVE_OBJ * io,long flags)
{
	long j = ARX_PARTICLES_GetFree();
	
		if (	(j!=-1)
			&&	(!ARXPausedTimer)	)
		{
			float power;
			power=(dmgs*DIV60)+0.9f;
			float r,g,b;
			r=(float)((long)((col>>16) & 255))*DIV255;
			g=(float)((long)((col>>8) & 255))*DIV255;
			b=(float)((long)((col) & 255))*DIV255;
			ParticleCount++;
			PARTICLE_DEF * pd=&particle[j];
			pd->exist=TRUE;
			pd->zdec=0;
		pd->ov.x = pos->x - EEsin((float)ARXTime * DIV1000) * 30.f; 
		pd->ov.y = pos->y + EEsin((float)ARXTime * DIV1000) * 30.f; 
		pd->ov.z = pos->z + EEcos((float)ARXTime * DIV1000) * 30.f; 

		pd->siz = 3.5f * power + EEsin((float)ARXTime * DIV1000); 
				
			if (!GORE_MODE)
				pd->siz*=DIV6;

			
		pd->scale.z = pd->scale.y = pd->scale.x = -pd->siz * DIV2; 
				
				
		pd->timcreation	=	lARXTime;
		pd->special		=	PARTICLE_SUB2 | SUBSTRACT | GRAVITY | ROTATING | MODULATE_ROTATION;

			if (flags & 1) 
				pd->special|=SPLAT_GROUND;

		pd->tolive = 1100; 
			pd->delay=0;
			pd->move.x=0;
			pd->move.y=0;
			pd->move.z=0;
			pd->r=r;
			pd->g=g;
			pd->b=b;
			pd->tc=bloodsplatter;
			pd->fparam=rnd()*DIV10-0.05f;
		}

		if (rnd()>0.90f) ARX_PARTICLES_Spawn_Rogue_Blood(pos,dmgs,col);
	
}
#define SPLAT_MULTIPLY 1.f

//-----------------------------------------------------------------------------
void ARX_POLYSPLAT_Add(EERIE_3D * poss,long type,EERIE_RGB * col,float size,long flags)
{
	if (BoomCount > (MAX_POLYBOOM >> 2) - 30) return;

	if ((BoomCount>250.f) 
		&& (size<10)) return;

	float splatsize=90;

	if (size>40.f) size=40.f;	

	size*=0.75f;

	if ((!GORE_MODE) && (!(flags & 2)))
		size*=DIV5;


	switch (pMenuConfig->iLevelOfDetails)
	{
		case 2:

			if (BoomCount>160.f)  return;

			splatsize=90;
			size*=1.f;
		break;
		case 1:

			if (BoomCount>60.f)  return;

			splatsize=60;
			size*=0.5f;
		break;
		default:

			if (BoomCount>10.f)  return;

			splatsize=30;
			size*=0.25f;
		break;
	}


	float py;
	EERIEPOLY * ep=CheckInPoly(poss->x,poss->y-40,poss->z,&py);

	if (!ep) return;
	

	if (flags & 1) py=poss->y;

	EERIEPOLY TheoricalSplat; // clockwise
	TheoricalSplat.v[0].sx=-splatsize*SPLAT_MULTIPLY;
	TheoricalSplat.v[0].sy = py; 
	TheoricalSplat.v[0].sz=-splatsize*SPLAT_MULTIPLY;

	TheoricalSplat.v[1].sx=-splatsize*SPLAT_MULTIPLY;
	TheoricalSplat.v[1].sy = py; 
	TheoricalSplat.v[1].sz=+splatsize*SPLAT_MULTIPLY;

	TheoricalSplat.v[2].sx=+splatsize*SPLAT_MULTIPLY;
	TheoricalSplat.v[2].sy = py; 
	TheoricalSplat.v[2].sz=+splatsize*SPLAT_MULTIPLY;

	TheoricalSplat.v[3].sx=+splatsize*SPLAT_MULTIPLY;
	TheoricalSplat.v[3].sy = py; 
	TheoricalSplat.v[3].sz=-splatsize*SPLAT_MULTIPLY;
	TheoricalSplat.type=POLY_QUAD;

	EERIE_3D RealSplatStart;
	RealSplatStart.x=-size;
	RealSplatStart.y=py;
	RealSplatStart.z=-size;

	TheoricalSplat.v[0].sx+=poss->x;
	TheoricalSplat.v[0].sz+=poss->z;

	TheoricalSplat.v[1].sx+=poss->x;
	TheoricalSplat.v[1].sz+=poss->z;

	TheoricalSplat.v[2].sx+=poss->x;
	TheoricalSplat.v[2].sz+=poss->z;

	TheoricalSplat.v[3].sx+=poss->x;
	TheoricalSplat.v[3].sz+=poss->z;
	
	RealSplatStart.x+=poss->x;
	RealSplatStart.z+=poss->z;


	float hdiv,vdiv;
	hdiv=vdiv=1.f/(size*2);


	long x0,x1;
	long z0,z1,i,j;
	unsigned long tim;
	long n;
	EERIE_BKG_INFO * eg;
	tim = ARXTimeUL();

	for (i=0;i<MAX_POLYBOOM;i++)
	{
		if (polyboom[i].exist)
		{
			polyboom[i].type|=128;
		}
	}

	F2L((poss->x*ACTIVEBKG->Xmul),&x0);
	F2L((poss->z*ACTIVEBKG->Zmul),&z0);
	x1 = x0 + 3; 
	x0 = x0 - 3; 
	z1 = z0 + 3; 
	z0 = z0 - 3; 

	if (x0<0) x0=0;

	if (x0>=ACTIVEBKG->Xsize) x0=ACTIVEBKG->Xsize-1;

	if (x1<0) x1=0;

	if (x1>=ACTIVEBKG->Xsize) x1=ACTIVEBKG->Xsize-1;

	if (z0<0) z0=0;

	if (z0>=ACTIVEBKG->Zsize) z0=ACTIVEBKG->Zsize-1;

	if (z1<0) z1=0;

	if (z1>=ACTIVEBKG->Zsize) z1=ACTIVEBKG->Zsize-1;

	long nbvert;
	float vratio=size*DIV40;


	ARX_CHECK_SHORT(z0);
	ARX_CHECK_SHORT(x0);
	ARX_CHECK_SHORT(z1);
	ARX_CHECK_SHORT(x1);



	for (j=z0;j<=z1;j++) 		
	for (i=x0;i<=x1;i++) 
	{
		eg=(EERIE_BKG_INFO *)&ACTIVEBKG->Backg[i+j*ACTIVEBKG->Xsize];

			for (long l = 0; l < eg->nbpolyin; l++) 
		{
				ep = eg->polyin[l]; 

			if (flags & 2)
			{
				if (!(ep->type & POLY_WATER)) continue;
			}

			if ((ep->type & POLY_TRANS) && !(ep->type & POLY_WATER)) continue;

			if (ep->type & POLY_QUAD) nbvert=4;
			else nbvert=3;

			long oki=0;

			for (long k=0;k<nbvert;k++)
			{
				if ((PointIn2DPolyXZ(&TheoricalSplat, ep->v[k].sx, ep->v[k].sz))
					&& (EEfabs(ep->v[k].sy-py)<100.f) )
				{
					 oki=1;
					break;
				}

				if ((PointIn2DPolyXZ(&TheoricalSplat, (ep->v[k].sx+ep->center.x)*DIV2, (ep->v[k].sz+ep->center.z)*DIV2))
					&& (EEfabs(ep->v[k].sy-py)<100.f) )
				{
					 oki=1;
					break;
				}
			}

			if (!oki && (PointIn2DPolyXZ(&TheoricalSplat, ep->center.x, ep->center.z))
					&& (EEfabs(ep->center.y-py)<100.f) )
					oki=1;

			
			if (oki)
			{
				n=ARX_BOOMS_GetFree();

				if (n>=0) 
				{
					BoomCount++;
					POLYBOOM * pb=&polyboom[n];
					pb->type=1;	

					if (flags & 2)
						pb->type=2;	

					pb->exist=1;
					pb->ep=ep;
					long num;
					F2L((float)(rnd()*6.f),&num);

					if (num<0) num=0;
					else if (num>5) num=5;

					pb->tc=bloodsplat[num];


					float fRandom = rnd()*2;
					ARX_CHECK_INT(fRandom);
					
					int t	= ARX_CLEAN_WARN_CAST_INT(fRandom);


					if (flags & 2)
						pb->tc = water_splat[t];

					pb->tolive=(long)(float)(16000*vratio);

					if (flags & 2)
						pb->tolive=1500;
					
					pb->timecreation=tim;

					pb->tx=ARX_CLEAN_WARN_CAST_SHORT(i);
					pb->tz=ARX_CLEAN_WARN_CAST_SHORT(j);

					pb->rgb.r=col->r;
					pb->rgb.g=col->g;
					pb->rgb.b=col->b;

					for (int k=0;k<nbvert;k++) 
					{
						float vdiff=EEfabs(ep->v[k].sy-RealSplatStart.y);
						pb->u[k]=(ep->v[k].sx-RealSplatStart.x)*hdiv;

						if (pb->u[k]<0.5f)
							pb->u[k]-=vdiff*hdiv;
						else pb->u[k]+=vdiff*hdiv;

						pb->v[k]=(ep->v[k].sz-RealSplatStart.z)*vdiv;

						if (pb->v[k]<0.5f)
							pb->v[k]-=vdiff*vdiv;
						else pb->v[k]+=vdiff*vdiv;

					}

					pb->nbvert=(short)nbvert;
				}
			}
		}			
	}	
}

//-----------------------------------------------------------------------------
void SpawnGroundSplat(EERIE_SPHERE * sp,EERIE_RGB * rgb,float size,long flags)
{
		ARX_POLYSPLAT_Add(&sp->origin,0,rgb,size,flags);
}


long TOTAL_BODY_CHUNKS_COUNT=0;
	
//-----------------------------------------------------------------------------
void ARX_PARTICLES_Spawn_Blood2(EERIE_3D * pos,float dmgs,D3DCOLOR col,long vert,INTERACTIVE_OBJ * io)
{
	if (	(io)
		&&	(io->ioflags & IO_NPC)
		&&	(io->_npcdata->SPLAT_TOT_NB)	)
	{
		if (io->_npcdata->SPLAT_DAMAGES<3) return;

		float power;

		power=((float)(io->_npcdata->SPLAT_DAMAGES)*DIV60)+0.9f;

		EERIE_3D vect;
		vect.x=pos->x-io->_npcdata->last_splat_pos.x;
		vect.y=pos->y-io->_npcdata->last_splat_pos.y;
		vect.z=pos->z-io->_npcdata->last_splat_pos.z;
		float dist=TRUEVector_Magnitude(&vect);
		float div=1.f/dist;
		vect.x*=div;
		vect.y*=div;
		vect.z*=div;
		long nb;
		F2L((float)(dist/(4.f*power)),&nb);

		if (nb==0) nb=1;

		long MAX_GROUND_SPLATS;

		switch (pMenuConfig->iLevelOfDetails)
		{
			case 2:
				MAX_GROUND_SPLATS=10;
			break;
			case 1:
				MAX_GROUND_SPLATS=5;
			break;
			default:
				MAX_GROUND_SPLATS=1;
			break;
		}


		for (long k=0;k<nb;k++)
		{
			EERIE_3D posi;
			posi.x=io->_npcdata->last_splat_pos.x+vect.x*(float)k*4.f*power;
			posi.y=io->_npcdata->last_splat_pos.y+vect.y*(float)k*4.f*power;
			posi.z=io->_npcdata->last_splat_pos.z+vect.z*(float)k*4.f*power;
			io->_npcdata->SPLAT_TOT_NB++;

			if (io->_npcdata->SPLAT_TOT_NB>MAX_GROUND_SPLATS)
			{
				ARX_PARTICLES_Spawn_Blood3(&posi,io->_npcdata->SPLAT_DAMAGES,col,vert,io,1);	
				io->_npcdata->SPLAT_TOT_NB=1;
			}
			else ARX_PARTICLES_Spawn_Blood3(&posi,io->_npcdata->SPLAT_DAMAGES,col,vert,io,0);	
		}
	}
	else 
	{
		if (	(io)
			&&	(io->ioflags & IO_NPC)	)
			io->_npcdata->SPLAT_DAMAGES=(short)dmgs;

		ARX_PARTICLES_Spawn_Blood3(pos,dmgs,col,vert,io,1);			

		if (	(io)
			&&	(io->ioflags & IO_NPC)	)
			io->_npcdata->SPLAT_TOT_NB=1;
	}

	if (	(io)
		&&	(io->ioflags & IO_NPC)	)
		Vector_Copy(&io->_npcdata->last_splat_pos,pos);
}

//-----------------------------------------------------------------------------
void ARX_PARTICLES_Spawn_Blood(EERIE_3D * pos,EERIE_3D * vect,float dmgs,long source)
{
	if (source<0) return;

	float nearest_dist=999999999.f;
	long nearest=-1;
	long count=inter.iobj[source]->obj->nbgroups;

	for (long i=0;i<count;i+=2)
	{
		float dist=EEDistance3D(pos,&inter.iobj[source]->obj->vertexlist3[inter.iobj[source]->obj->grouplist[i].origin].v);

		if (dist<nearest_dist)
		{
			nearest_dist=dist;
			nearest=i;
		}
	}

	// Decides number of blood particles...
	long spawn_nb;	
	spawn_nb = (long)(dmgs * 2.0F);

	if (spawn_nb<5) spawn_nb=5;
	else if (spawn_nb>26) spawn_nb=26;

	long totdelay=0;

	if (nearest>=0)
	for (long k=0;k<spawn_nb;k++)
	{
		long j=ARX_PARTICLES_GetFree();

		if ((j!=-1) && (!ARXPausedTimer))
		{
			ParticleCount++;
			PARTICLE_DEF * pd=&particle[j];
			pd->exist=TRUE;
			pd->zdec=0;
			pd->siz=0.f;
			pd->scale.x=(float)spawn_nb;
			pd->scale.y=(float)spawn_nb;
			pd->scale.z=(float)spawn_nb;
			
				pd->timcreation	=	lARXTime;
			pd->special		=	GRAVITY | ROTATING | MODULATE_ROTATION | DELAY_FOLLOW_SOURCE;
			pd->source		=	&inter.iobj[source]->obj->vertexlist3[nearest].v;
			pd->sourceionum	=	source;
			pd->tolive		=	1200+spawn_nb*5;
			totdelay		+=	(long)(45.f + rnd() * (150.f - spawn_nb));
			pd->delay		=	totdelay;
			pd->r			=	0.9f;
			pd->g			=	0.f;
			pd->b			=	0.f;
			pd->tc			=	bloodsplatter;
			pd->fparam		=	rnd()*DIV10-0.05f;			
		}
	}
}
long SPARK_COUNT=0;
//-----------------------------------------------------------------------------
// flag & 1 punch failed
// flag & 2 punch success
//-----------------------------------------------------------------------------
void ARX_PARTICLES_Spawn_Spark(EERIE_3D * pos,float dmgs,long flags)
{
	if (!pos) return;

	long spawn_nb;	
	F2L(dmgs,&spawn_nb);
	
	
	if (SPARK_COUNT<1000)
	{
		SPARK_COUNT+=spawn_nb*25;
	}
	else
	{
		SPARK_COUNT	-=	ARX_CLEAN_WARN_CAST_LONG(FrameDiff);
		return;
	}
	
	for (long k=0;k<spawn_nb;k++)
	{
		long j=ARX_PARTICLES_GetFree();

		if ((j!=-1) && (!ARXPausedTimer))
		{
			ParticleCount++;
			PARTICLE_DEF * pd=&particle[j];
			pd->exist=TRUE;
			pd->zdec=0;
			pd->oldpos.x=pd->ov.x=pos->x+rnd()*10.f-5.f;
			pd->oldpos.y=pd->ov.y=pos->y+rnd()*10.f-5.f;
			pd->oldpos.z=pd->ov.z=pos->z+rnd()*10.f-5.f;
			
			pd->siz			=	2.f;
			pd->move.x		=	6 - 12.f * rnd(); 
			pd->move.y		=	6 - 12.f * rnd(); 
			pd->move.z		=	6 - 12.f * rnd(); 
				
			pd->timcreation	=	lARXTime;
			pd->special		=	PARTICLE_SPARK;
			float len		=	(float)spawn_nb*DIV3;

			if ( len > 8 ) len	=	8;

			if ( len < 3 ) len	=	3;

			pd->tolive		=	(unsigned long)(float)(len * 90 + (float)spawn_nb);

			if (flags==0)
			{
				pd->r=0.3f;
				pd->g=0.3f;
				pd->b=0.f;
			}
			else if (flags & 1)
			{
				pd->r=0.2f;
				pd->g=0.2f;
				pd->b=0.1f;
			}
			else if (flags & 2)
			{
				pd->r=0.45f;
				pd->g=0.1f;
				pd->b=0.f;
			}

			pd->tc=NULL;
			pd->fparam = len + rnd() * len; // Spark Tail Length
		}
	}
}

//-----------------------------------------------------------------------------
void MakeCoolFx(EERIE_3D * pos)
{
	ARX_BOOMS_Add(pos,1);		
}

//-----------------------------------------------------------------------------
void MakePlayerAppearsFX(INTERACTIVE_OBJ * io)
{
	if (io)
	{
		MakeCoolFx(&io->pos);		
		MakeCoolFx(&io->pos);		
		AddRandomSmoke(io,30);
		ARX_PARTICLES_Add_Smoke(&io->pos,1 | 2,20); // flag 1 = randomize pos
	}
}

//-----------------------------------------------------------------------------
void AddRandomSmoke(INTERACTIVE_OBJ * io,long amount)
{
	if (!io) return;

	while (amount--)
	{
		long num;
		F2L(rnd()*((io->obj->nbvertex>>2)-1),&num);
		num=(num<<2)+1;

			long j=ARX_PARTICLES_GetFree();

			if (	(j!=-1)
				&&	(!ARXPausedTimer)	)
			{
				ParticleCount++;
				PARTICLE_DEF * pd=&particle[j];
				pd->exist		=	TRUE;
				pd->zdec		=	0;
				pd->ov.x		=	io->obj->vertexlist3[num].v.x+rnd()*10.f-5.f;
				pd->ov.y		=	io->obj->vertexlist3[num].v.y+rnd()*10.f-5.f;
				pd->ov.z		=	io->obj->vertexlist3[num].v.z+rnd()*10.f-5.f;
				pd->siz			=	rnd()*8.f;

				if (pd->siz<4.f) pd->siz=4.f;

				pd->scale.x		=	10.f;
				pd->scale.y		=	10.f;
				pd->scale.z		=	10.f;
			pd->timcreation	=	lARXTime;
				pd->special		=	ROTATING | MODULATE_ROTATION | FADE_IN_AND_OUT;
				pd->tolive		=	900+(unsigned long)(rnd()*400.f);
				pd->move.x		=	0.25f-0.5f*rnd();
				pd->move.y		=	-1.f*rnd()+0.3f;
				pd->move.z		=	0.25f-0.5f*rnd();
				pd->r			=	0.3f;
				pd->g			=	0.3f;
				pd->b			=	0.34f;
				pd->tc			=	smokeparticle;//tc2;
				pd->fparam		=	0.001f;//-rnd()*0.0002f;
			}
		}
	}

//-----------------------------------------------------------------------------
void ARX_PARTICLES_Add_Smoke(EERIE_3D * pos,long flags,long amount,EERIE_RGB * rgb) // flag 1 = randomize pos
{

	EERIE_3D mod;
	mod.x = mod.y = mod.z = 0.f;

	if (flags & 1)
	{
		mod.x=rnd()*100.f-50.f;
		mod.y=rnd()*100.f-50.f;
		mod.z=rnd()*100.f-50.f;
	}

	while(amount)
	{
		amount--;
		long j=ARX_PARTICLES_GetFree();

		if (	(j!=-1)
			&&	(!ARXPausedTimer)	)
		{
			ParticleCount++;
			PARTICLE_DEF * pd=&particle[j];
			pd->exist=TRUE;
			pd->zdec=0;
			pd->ov.x=pos->x+mod.x;
			pd->ov.y=pos->y+mod.y;
			pd->ov.z=pos->z+mod.z;

			if (flags & 2)
			{
				pd->siz=rnd()*20.f+15.f;
				pd->scale.x=40.f+rnd()*15;
				pd->scale.y=40.f+rnd()*15;
				pd->scale.z=40.f+rnd()*15;
			}
			else
			{
				pd->siz=rnd()*8.f+5.f;

				if (pd->siz<4.f) pd->siz=4.f;

				pd->scale.x=10.f+rnd()*5;
				pd->scale.y=10.f+rnd()*5;
				pd->scale.z=10.f+rnd()*5;
			}
			
			pd->timcreation	=	lARXTime;
			pd->special		=	ROTATING | MODULATE_ROTATION | FADE_IN_AND_OUT;
			pd->tolive		=	1100+(unsigned long)(rnd()*400.f);
			pd->delay		=	(unsigned long)(amount * 120.0F + rnd() * 100.0F);
			pd->move.x		=	0.25f-0.5f*rnd();
			pd->move.y		=	-1.f*rnd()+0.3f;
			pd->move.z		=	0.25f-0.5f*rnd();

			if (rgb)
			{
				pd->r = rgb->r;
				pd->g = rgb->g;
				pd->b = rgb->b;
			}
			else
			{
				pd->r=0.3f;
				pd->g=0.3f;
				pd->b=0.34f;
			}

			pd->tc = smokeparticle; 
			pd->fparam = 0.01f; 
		}
	}
}

extern long cur_mr;
//-----------------------------------------------------------------------------
void ManageTorch()
{
	EERIE_LIGHT * el=&DynLight[0];
	
	if (SHOW_TORCH) 
	{
		float rr=rnd();
		el->pos.x = player.pos.x; 
		el->pos.y=player.pos.y;
		el->pos.z = player.pos.z; 
		el->intensity=1.6f;
		el->fallstart=280.f+rr*20.f;
		el->fallend = el->fallstart + 280.f; 
		el->exist=1;	
		el->rgb.r = Project.torch.r - rr * 0.1f; 
		el->rgb.g = Project.torch.g - rr * 0.1f; 
		el->rgb.b = Project.torch.b - rr * 0.1f;
		el->duration=0;
		el->extras=0;		
	}
	else if (cur_mr==3)
	{		
		el->pos.x = player.pos.x;
		el->pos.y=player.pos.y;
		el->pos.z = player.pos.z; 
		el->intensity=1.8f;
		el->fallstart=480.f;
		el->fallend = el->fallstart + 480.f; 
		el->exist=1;	
		el->rgb.r = 1.f;
		el->rgb.g = 0.5f; 
		el->rgb.b = 0.8f;
		el->duration=0;
		el->extras=0;		
	}
	else 
	{
		if (flarenum == 0) 
			el->exist=0;
		else 
		{
			float rr=rnd();
			long count=0;

			for (long i = 0; i < MAX_FLARES; i++)
			{
				if (flare[i].exist)
				{
					if (flare[i].flags==0) count++;
				}
			}

			if (count)
			{
				el->pos.x=player.pos.x;
				el->pos.y=player.pos.y;
				el->pos.z=player.pos.z;
				el->fallstart=140.f+(float)count*0.333333f+rr*5.f;
				el->fallend=220.f+(float)count*0.5f+rr*5.f;
				el->intensity=1.6f;
				el->exist=1;	
				el->rgb.r=0.01f*count;	
				el->rgb.g=0.009f*count;
				el->rgb.b=0.008f*count;
			}
		}
	}

	if (inter.iobj && inter.iobj[0] && inter.iobj[0]->obj
		&& (inter.iobj[0]->obj->fastaccess.head_group_origin>-1))
	{
		el->pos.y=inter.iobj[0]->obj->vertexlist3[inter.iobj[0]->obj->fastaccess.head_group_origin].v.y;
	}
}
#define DIV_MAX_FLARELIFE	0.00025f
//-----------------------------------------------------------------------------
void ARX_MAGICAL_FLARES_Draw(LPDIRECT3DDEVICE7  m_pd3dDevice,long FRAMETICKS)
{
	/////////FLARE
	SETZWRITE(m_pd3dDevice, FALSE );
	m_pd3dDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND,   D3DBLEND_ONE );
	m_pd3dDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND,  D3DBLEND_ONE );

	SETALPHABLEND(m_pd3dDevice,TRUE);
	shinum++;

	if (shinum>=10) shinum=1;

	long TICKS	=	lARXTime - FRAMETICKS;

	if (TICKS<0) 
		return;

	float zapp,x,y,z,s,r,g,b;

	TextureContainer * surf;
	BOOL key=!ARX_IMPULSE_Pressed(CONTROLS_CUST_MAGICMODE);

	for (long j=1;j<5;j++) 
	{
		switch (j)
		{
		case 2:				
			surf=flaretc.lumignon;
			break;
		case 3:
			surf=flaretc.lumignon2;
			break;
		case 4:
			surf=flaretc.plasm;
			break;
		default:
			surf=flaretc.shine[shinum];
			break;
		}

		for (long i=0;i<MAX_FLARES;i++)
		{
			if ((flare[i].exist)
				&& (flare[i].type==j))
			{
				flare[i].tolive-=(float)TICKS*2;

				if (flare[i].flags & 1)
				{
					flare[i].tolive-=(float)TICKS*4;
				}
				else if (key)						
				{
					flare[i].tolive-=TICKS*6;
				}
				
				if (	(flare[i].tolive <= 0.f ) 
					||	(flare[i].y< -64 )	
					)
				{
					if (flare[i].io)
					{
						if (ValidIOAddress(flare[i].io))
							flare[i].io->flarecount--;
					}					

					if (ValidDynLight(flare[i].dynlight))
						DynLight[flare[i].dynlight].exist=0;

					flare[i].dynlight=-1;
					flare[i].exist=0;
					flarenum--;
				}
				else 
				{
					
					z = (flare[i].tolive * DIV_MAX_FLARELIFE); 

					if ( (flare[i].type==1)  ) 
						s=flare[i].size*2*z;
					else if (flare[i].type==4) 
					{
						s=flare[i].size*2.f*z+10.f;
					}
					else s=flare[i].size;
					
					if ( s<3.f ) 
					{
						if (flare[i].io)
						{
							if (ValidIOAddress(flare[i].io))
								flare[i].io->flarecount--;
						}					

						if (ValidDynLight(flare[i].dynlight))
							DynLight[flare[i].dynlight].exist=0;

						flare[i].dynlight=-1;
						flare[i].exist=0;
						flarenum--;
					}
					else 
					{
						if (( (s<45) && (flare[i].type==1) ) || (flare[i].type==4) ) 
						{
							zapp=((MAX_FLARELIFE-flare[i].tolive)*(MAX_FLARELIFE-flare[i].tolive))*0.000001f;
						}

						if ((flare[i].type==1) ) 
						{
							if (z<0.6f) z=0.6f;						
						}

						g=flare[i].g*z;
						r=flare[i].r*z;
						b=flare[i].b*z;
						
						zapp = 48 - s * 0.5f; 
						x=flare[i].x+zapp;
						y=flare[i].y+zapp;
								
						flare[i].tv.color=D3DRGB(r,g,b);
						flare[i].v.sx=flare[i].tv.sx;
						flare[i].v.sy=flare[i].tv.sy;
						flare[i].v.sz=flare[i].tv.sz;
								
						DynLight[0].rgb.r=__max(DynLight[0].rgb.r,r);
						DynLight[0].rgb.g=__max(DynLight[0].rgb.g,g);
						DynLight[0].rgb.b=__max(DynLight[0].rgb.b,b);

						if (ValidDynLight(flare[i].dynlight)) 
						{
							EERIE_LIGHT * el=&DynLight[flare[i].dynlight];
							el->pos.x=flare[i].v.sx;
							el->pos.y=flare[i].v.sy;
							el->pos.z=flare[i].v.sz;
							el->rgb.r=r;
							el->rgb.g=g;
							el->rgb.b=b;
						}

						if (!flare[i].io) 
						{
							GDevice->SetRenderState(D3DRENDERSTATE_ZENABLE,FALSE);
						}
						else
						{
							GDevice->SetRenderState(D3DRENDERSTATE_ZENABLE,TRUE);
						}

						if(flare[i].bDrawBitmap)
						{
							s*=2.f;
							EERIEDrawBitmap(m_pd3dDevice,flare[i].v.sx,flare[i].v.sy,s,s,flare[i].v.sz,surf,flare[i].tv.color);
						}
						else
						{
							EERIEDrawSprite(m_pd3dDevice,&flare[i].v,(float)s*0.025f+1.f, surf,flare[i].tv.color,2.f);
						}
					}
				}
				
			}
					
		}
	}

	if (DynLight[0].rgb.r>1.f) DynLight[0].rgb.r=1.f;

	if (DynLight[0].rgb.g>1.f) DynLight[0].rgb.g=1.f;

	if (DynLight[0].rgb.b>1.f) DynLight[0].rgb.b=1.f;

	SETZWRITE(m_pd3dDevice, TRUE );
	GDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, TRUE); 
}

//-----------------------------------------------------------------------------
void ARX_BOOMS_ClearAllPolyBooms()
{
	for(long i=0;i<MAX_POLYBOOM;i++) polyboom[i].exist=0;

	BoomCount=0;
}

//-----------------------------------------------------------------------------
void ARX_BOOMS_ClearAll()
{
	for (long i=0;i<MAX_BOOMS;i++) booms[i].exist=0;
}
//-----------------------------------------------------------------------------
void ARX_BOOMS_Add(EERIE_3D * poss,long type)
{
	static TextureContainer * tc1=MakeTCFromFile("Graph\\Particles\\fire_hit.bmp");
	static TextureContainer * tc2=MakeTCFromFile("Graph\\Particles\\boom.bmp");
	long x0,x1;
	long z0,z1,i,j;
	unsigned long tim;
	float ddd;
	long typ;
	long dod,n;
	EERIEPOLY * ep;
	EERIE_BKG_INFO * eg;
	tim = ARXTimeUL();//treat warning C4244 conversion from 'float' to 'unsigned long'	

	j=ARX_PARTICLES_GetFree();

	if (j!=-1)
	{
		ParticleCount++;
		PARTICLE_DEF * pd=&particle[j];
		pd->exist=TRUE;
		pd->ov.x=poss->x;
		pd->ov.y=poss->y;
		pd->ov.z=poss->z;
		pd->move.x=3.f-6.f*rnd();
		pd->move.y=4.f-12.f*rnd();
		pd->move.z=3.f-6.f*rnd();
		pd->timcreation=tim;
		pd->tolive=(unsigned long)(600+rnd()*100);
		pd->tc=tc1;
		pd->siz=100.f+10.f*rnd();

		if (type==1) pd->siz*=2;

		pd->zdec=1;

		if (type==1)
		{
			pd->r=0.4f;
			pd->g=0.4f;
			pd->b=1.f;
		}
	
		j=ARX_PARTICLES_GetFree();

		if (j!=-1)
		{
			ParticleCount++;
			PARTICLE_DEF * pd=&particle[j];
			pd->exist=TRUE;
			pd->ov.x=poss->x;
			pd->ov.y=poss->y;
			pd->ov.z=poss->z;
			pd->move.x=3.f-6.f*rnd();
			pd->move.y=4.f-12.f*rnd();
			pd->move.z=3.f-6.f*rnd();
			pd->timcreation=tim;
			pd->tolive=(unsigned long)(600+rnd()*100);
			pd->tc=tc1;
			pd->siz=40.f+30.f*rnd();

			if (type==1) pd->siz*=2;

			pd->zdec=1;

			if (type==1)
			{
				pd->r=0.4f;
				pd->g=0.4f;
				pd->b=1.f;
			}
		}
	}

	EERIE_3D pos;
	Vector_Copy(&pos,poss);

	typ=0;
	F2L((poss->x*ACTIVEBKG->Xmul),&x0);
	F2L((poss->z*ACTIVEBKG->Zmul),&z0);
	x1=x0+3;
	x0=x0-3;
	z1=z0+3;
	z0=z0-3;

	if (x0<0) x0=0;
	else if (x0>=ACTIVEBKG->Xsize) x0=ACTIVEBKG->Xsize-1;

	if (x1<0) x1=0;
	else if (x1>=ACTIVEBKG->Xsize) x1=ACTIVEBKG->Xsize-1;

	if (z0<0) z0=0;
	else if (z0>=ACTIVEBKG->Zsize) z0=ACTIVEBKG->Zsize-1;

	if (z1<0) z1=0;
	else if (z1>=ACTIVEBKG->Zsize) z1=ACTIVEBKG->Zsize-1;

	long nbvert;
	float temp_u1[4];
	float temp_v1[4];
	

	ARX_CHECK_SHORT(z0);
	ARX_CHECK_SHORT(x0);	
	ARX_CHECK_SHORT(z1);
	ARX_CHECK_SHORT(x1);

	if( !bSoftRender )
	{	//We never add BOOMS particle with this flag to prevent any render issues. TO DO check for blending of DrawPrimitve and DrawPrimitiveVB
		for (j=z0;j<=z1;j++) 		
		for (i=x0;i<=x1;i++) 
		{
			eg=(EERIE_BKG_INFO *)&ACTIVEBKG->Backg[i+j*ACTIVEBKG->Xsize];

				for (long l = 0; l < eg->nbpoly; l++) 
			{
				ep=&eg->polydata[l];

				if (ep->type & POLY_QUAD) nbvert=4;
				else nbvert=3;

				if ((ep->type & POLY_TRANS) && !(ep->type & POLY_WATER)) goto suite;

				dod=1;

				if ((ddd=Distance3D(ep->v[0].sx,ep->v[0].sy,ep->v[0].sz,poss->x,poss->y,poss->z))<BOOM_RADIUS)
				{	
					temp_u1[0]=(0.5f-((ddd/BOOM_RADIUS)*0.5f));
					temp_v1[0]=(0.5f-((ddd/BOOM_RADIUS)*0.5f));

					for (long k=1;k<nbvert;k++) 
					{
						ddd=Distance3D(ep->v[k].sx,ep->v[k].sy,ep->v[k].sz,poss->x,poss->y,poss->z);

						if (ddd>BOOM_RADIUS) dod=0;
						else 
						{
							temp_u1[k]=0.5f-((ddd/BOOM_RADIUS)*0.5f);
							temp_v1[k]=0.5f-((ddd/BOOM_RADIUS)*0.5f);						
						}
					}

					if (dod) 
					{
						n=ARX_BOOMS_GetFree();

						if (n>=0) 
						{
							BoomCount++;
							POLYBOOM * pb=&polyboom[n];
							pb->type=(short)typ;						
							pb->exist=1;
							pb->ep=ep;
							pb->tc=tc2;
							pb->tolive=10000;
							pb->timecreation=tim;

							pb->tx=ARX_CLEAN_WARN_CAST_SHORT(i);
							pb->tz=ARX_CLEAN_WARN_CAST_SHORT(j);


							for (int k=0;k<nbvert;k++) 
							{
								pb->u[k]=temp_u1[k];
								pb->v[k]=temp_v1[k];
							}

							pb->nbvert=(short)nbvert;
						}
					}
				}

				suite:
					;
			}
		}
	}
}

//-----------------------------------------------------------------------------
void Add3DBoom(EERIE_3D * position, EERIE_3DOBJ *pObj3DSphere)
{
	float dist=Distance3D(player.pos.x,player.pos.y-160.f,player.pos.z,position->x,position->y,position->z);
	EERIE_3D poss;
	Vector_Copy(&poss,position);
	ARX_SOUND_PlaySFX(SND_SPELL_FIRE_HIT, &poss);

	if (dist<300)
	{
		float onedist=1.f/dist;
		EERIE_3D vect;
		vect.x=(player.pos.x-position->x)*onedist; 
		vect.y=(player.pos.y-160.f-position->y)*onedist; 
		vect.z=(player.pos.z-position->z)*onedist;
		float power=(300.f-dist)*DIV80;
		player.physics.forces.x+=vect.x*power;
		player.physics.forces.y+=vect.y*power;
		player.physics.forces.z+=vect.z*power;
	}

	for (long i=0;i<inter.nbmax;i++)
	{		
		if ( inter.iobj[i] != NULL )
		{
			if ( inter.iobj[i]->show!=1 ) continue;

			if ( !(inter.iobj[i]->ioflags & IO_ITEM) ) continue;

			if ( inter.iobj[i]->obj)
				if ( inter.iobj[i]->obj->pbox)
				{
					for (long k=0;k<inter.iobj[i]->obj->pbox->nb_physvert;k++)
					{
						float dist=EEDistance3D( &inter.iobj[i]->obj->pbox->vert[k].pos, position);

						if (dist<300.f)
						{
							inter.iobj[i]->obj->pbox->active=1;
							inter.iobj[i]->obj->pbox->stopcount=0;
							float onedist=1.f/dist;
							EERIE_3D vect;
							vect.x=(inter.iobj[i]->obj->pbox->vert[k].pos.x-position->x)*onedist; 
							vect.y=(inter.iobj[i]->obj->pbox->vert[k].pos.y-position->y)*onedist; 
							vect.z=(inter.iobj[i]->obj->pbox->vert[k].pos.z-position->z)*onedist;
							float power = (300.f - dist) * 10.f; 
							inter.iobj[i]->obj->pbox->vert[k].velocity.x+=vect.x*power;
							inter.iobj[i]->obj->pbox->vert[k].velocity.y+=vect.y*power;
							inter.iobj[i]->obj->pbox->vert[k].velocity.z+=vect.z*power;
						}
					}
				}
		}
	}
}

//-----------------------------------------------------------------------------
void UpdateObjFx(LPDIRECT3DDEVICE7 pd3dDevice, EERIE_CAMERA * cam)
		{

	unsigned long framediff;
	float val,aa,bb;
	float t1,t2,t3;
	long p;
	EERIE_3D pos;

	D3DTLVERTEX v[3];
	v[0]= D3DTLVERTEX( D3DVECTOR( 0, 0, 0.001f ), 1.f, D3DRGB(1.f,1.f,1.f), 1, 0.f, 0.f);
	v[1]= D3DTLVERTEX( D3DVECTOR( 0, 0, 0.001f ), 1.f, D3DRGB(1.f,1.f,1.f), 1, 1.f, 0.f);
	v[2]= D3DTLVERTEX( D3DVECTOR( 0, 0, 0.001f ), 1.f, D3DRGB(1.f,1.f,1.f), 1, 1.f, 1.f);
	

	D3DTLVERTEX v2[3];
	v[0]= D3DTLVERTEX( D3DVECTOR( 0, 0, 0.001f ), 1.f, D3DRGB(1.f,1.f,1.f), 1, 0.f, 0.f);
	v[1]= D3DTLVERTEX( D3DVECTOR( 0, 0, 0.001f ), 1.f, D3DRGB(1.f,1.f,1.f), 1, 1.f, 0.f);
	v[2]= D3DTLVERTEX( D3DVECTOR( 0, 0, 0.001f ), 1.f, D3DRGB(1.f,1.f,1.f), 1, 1.f, 1.f);
	
	pd3dDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND,   D3DBLEND_ONE );
	pd3dDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND,  D3DBLEND_ONE );
	SETALPHABLEND(pd3dDevice,TRUE);

	SETZWRITE(pd3dDevice, FALSE );
	

	for (long i=0;i<MAX_OBJFX;i++)
	{
		if (objfx[i].exist)
		{
			framediff = ARXTimeUL() - objfx[i].tim_start; 

			if (framediff>objfx[i].duration) 
			{
				objfx[i].exist=FALSE;

				if (ValidDynLight(objfx[i].dynlight))
					DynLight[objfx[i].dynlight].exist=0;

				objfx[i].dynlight=-1;
				continue;
			}

			val=(float)framediff/(float)objfx[i].duration;
			pos.x=objfx[i].pos.x+objfx[i].move.x*val;
			pos.y=objfx[i].pos.y+objfx[i].move.y*val;
			pos.z=objfx[i].pos.z+objfx[i].move.z*val;

			EERIE_3D angle;
			EERIE_3D scale;
			EERIE_RGB color;			
			scale.x=1.f+objfx[i].scale.x*val;
			scale.y=1.f+objfx[i].scale.y*val;
			scale.z=1.f+objfx[i].scale.z*val;

			if (Project.improve)
			{
				color.r=1.f-objfx[i].fade.r*val;
				color.g=0.f;
				color.b=1.f-objfx[i].fade.b*val;
			}
			else
			{
				color.r=1.f-objfx[i].fade.r*val;
				color.g=1.f-objfx[i].fade.g*val;
				color.b=1.f-objfx[i].fade.b*val;
			}

			angle.a=0.f;
			angle.b=0.f;
			angle.g=0.f;

			DrawEERIEObjEx(pd3dDevice,objfx[i].obj,
					&angle,&pos,&scale,&color);

			if (objfx[i].dynlight!=-1)
			{
				DynLight[objfx[i].dynlight].fallend=250.f+450.f*val;
				DynLight[objfx[i].dynlight].fallstart=150.f+50.f*val;
				DynLight[objfx[i].dynlight].rgb.r=1.f-val;
				DynLight[objfx[i].dynlight].rgb.g=0.9f-val*0.9f;
				DynLight[objfx[i].dynlight].rgb.b=0.5f-val*0.5f;
				DynLight[objfx[i].dynlight].pos.x=pos.x;
				DynLight[objfx[i].dynlight].pos.y=pos.y;
				DynLight[objfx[i].dynlight].pos.z=pos.z;
			}

			if (objfx[i].special & SPECIAL_RAYZ)
			{

				SETCULL(pd3dDevice,D3DCULL_NONE);

				for (long k=0;k<8;k++)
				{
					aa=objfx[i].spe[k].g;
					bb=objfx[i].speinc[k].g;

					if (ARXPausedTimer)
					{						
					}
					else
					{
						objfx[i].spe[k].a+=objfx[i].speinc[k].a;
						objfx[i].spe[k].b+=objfx[i].speinc[k].b;
					}

					for (p=0;p<3;p++)
					{
						v[p].sx=pos.x;
						v[p].sy=pos.y;
						v[p].sz=pos.z;
					}
						
					t1=100.f*scale.x;
					t2=100.f*scale.y;
					t3=100.f*scale.z;
					v[1].sx-=EEsin(DEG2RAD(objfx[i].spe[k].b))*t1;
					v[1].sy+=EEsin(DEG2RAD(objfx[i].spe[k].a))*t2;
					v[1].sz+=EEcos(DEG2RAD(objfx[i].spe[k].b))*t3;
					v[2].sx=v[0].sx-EEsin(DEG2RAD(MAKEANGLE(objfx[i].spe[k].b+bb)))*t1;
					v[2].sy=v[0].sy+EEsin(DEG2RAD(MAKEANGLE(objfx[i].spe[k].a+aa)))*t2;
					v[2].sz=v[0].sz+EEcos(DEG2RAD(MAKEANGLE(objfx[i].spe[k].b+bb)))*t3;
					EE_RTP(&v[0],&v2[0]);
					EE_RTP(&v[1],&v2[1]);
					EE_RTP(&v[2],&v2[2]);

					if (Project.improve)
					{
						for (p=0;p<3;p++)
							v2[p].color=D3DRGB(color.r/(3.f+(float)p),0.f,color.b/(5.f+(float)p));
					}
					else
					{
						for (p=0;p<3;p++)
							v2[p].color=D3DRGB(color.r/(3.f+(float)p),color.g/(4.f+(float)p),color.b/(5.f+(float)p));
					}

					SETTC(pd3dDevice,NULL);
					EERIEDRAWPRIM(pd3dDevice,D3DPT_TRIANGLEFAN, D3DFVF_TLVERTEX| D3DFVF_DIFFUSE , v2, 3,  0, bSoftRender?EERIE_USEVB:0  );
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
void ARX_PARTICLES_FirstInit() 
{
	smokeparticle=MakeTCFromFile("Graph\\Particles\\smoke.bmp");
    bloodsplat[0]=bloodsplatter=MakeTCFromFile("Graph\\Particles\\new_blood.bmp");	
	bloodsplat[1]=MakeTCFromFile("Graph\\Particles\\new_blood_splat1.bmp");	
	bloodsplat[2]=MakeTCFromFile("Graph\\Particles\\new_blood_splat2.bmp");	
	bloodsplat[3]=MakeTCFromFile("Graph\\Particles\\new_blood_splat3.bmp");	
	bloodsplat[4]=MakeTCFromFile("Graph\\Particles\\new_blood_splat4.bmp");	
	bloodsplat[5]=MakeTCFromFile("Graph\\Particles\\new_blood_splat5.bmp");	
	water_splat[0]=MakeTCFromFile("Graph\\Particles\\[fx]_Water01.bmp");
	water_splat[1]=MakeTCFromFile("Graph\\Particles\\[fx]_Water02.bmp");
	water_splat[2]=MakeTCFromFile("Graph\\Particles\\[fx]_Water03.bmp");
	water_drop[0]=MakeTCFromFile("Graph\\Particles\\[fx]_Water_drop01.bmp");
	water_drop[1]=MakeTCFromFile("Graph\\Particles\\[fx]_Water_drop02.bmp");
	water_drop[2]=MakeTCFromFile("Graph\\Particles\\[fx]_Water_drop03.bmp");
	healing=MakeTCFromFile("Graph\\Particles\\heal_0005.bmp");
	tzupouf=MakeTCFromFile("Graph\\Obj3D\\Textures\\(FX)_tsu_greypouf.bmp");
	fire2=MakeTCFromFile("Graph\\Particles\\fire2.bmp");
}

//-----------------------------------------------------------------------------
void ARX_PARTICLES_ClearAll() 
{
	memset(particle,0,sizeof(PARTICLE_DEF)*MAX_PARTICLES);
	ParticleCount=0;
}

//-----------------------------------------------------------------------------
long ARX_PARTICLES_GetFree()
{	
	for ( long i = 0 ; i < MAX_PARTICLES ; i++)
		if ( !particle[i].exist )
		{
			PARTICLE_DEF *pd	= &particle[i];
			pd->type			= 0;
			pd->r				= 1.f;
			pd->g				= 1.f;
			pd->b				= 1.f;
			pd->tc				= NULL;
			pd->special			= 0;
			pd->source			= NULL;
			pd->delay			= 0;

			return i;
		}

	return -1;
}

//-----------------------------------------------------------------------------
void MagFX(float posx,float posy,float posz)
{
	long j;
	j=ARX_PARTICLES_GetFree();

	if ((j!=-1) && (!ARXPausedTimer))
	{
		ParticleCount++;
		PARTICLE_DEF * pd	=	&particle[j];
		pd->exist			=	TRUE;
		pd->zdec			=	0;
		pd->ov.x			=	posx+rnd()*6.f-rnd()*12.f;
		pd->ov.y			=	posy+rnd()*6.f-rnd()*12.f;
		pd->ov.z			=	posz;
		pd->move.x			=	6.f-rnd()*12.f;
		pd->move.y			=	-8.f+rnd()*16.f;
		pd->move.z			=	0.f;
		pd->scale.x			=	4.4f;
		pd->scale.y			=	4.4f;
		pd->scale.z			=	1.f;
		pd->timcreation		=	lARXTime;
		pd->tolive			=	1500+(unsigned long)(rnd()*900.f);
		pd->tc				=	healing;
		pd->r				=	1.f;
		pd->g				=	0.f;
		pd->b				=	1.f;
		pd->siz				=	56.f;
		pd->type			=	PARTICLE_2D;
	}
}

//-----------------------------------------------------------------------------
void MakeBookFX(float posx,float posy,float posz)
{
	long j;

	for (long i=0;i<12;i++)
	{
		j=ARX_PARTICLES_GetFree();

		if ((j!=-1) && (!ARXPausedTimer))
		{
			ParticleCount++;
			PARTICLE_DEF * pd=&particle[j];
			pd->exist		=	TRUE;
			pd->zdec		=	0;
			pd->ov.x		=	posx+rnd()*6.f-rnd()*12.f;
			pd->ov.y		=	posy+rnd()*6.f-rnd()*12.f;
			pd->ov.z		=	posz;
			pd->move.x		=	6.f-rnd()*12.f;
			pd->move.y		=	-8.f+rnd()*16.f;
			pd->move.z		=	0.f;
			pd->scale.x		=	4.4f;
			pd->scale.y		=	4.4f;
			pd->scale.z		=	1.f;
			pd->timcreation	=	lARXTime;
			pd->tolive		=	1500+(unsigned long)(rnd()*900.f);
			pd->tc			=	healing;
			pd->r			=	1.f;
			pd->g			=	0.f;
			pd->b			=	1.f;
			pd->siz			=	56.f;
			pd->type			=	PARTICLE_2D;
		}
	}

	for (int i=0;i<5;i++)
	{
		j=ARX_PARTICLES_GetFree();

		if ((j!=-1) && (!ARXPausedTimer))
		{
			ParticleCount++;
			PARTICLE_DEF * pd=&particle[j];
			pd->exist		=	TRUE;
			pd->zdec		=	0;

			pd->ov.x		=	posx - i * 2; 
			pd->ov.y		=	posy - i * 2; 
			pd->ov.z		=	posz;
			pd->move.x		=	-(float)(i)*DIV2;
			pd->move.y		=	-(float)(i)*DIV2;
			pd->move.z		=	0.f;
			pd->scale.y		=	pd->scale.x			=	(float)(i*10);			
			pd->scale.z		=	0.f;
			pd->timcreation	=	lARXTime;
			pd->tolive		=	1200+(unsigned long)(rnd()*400.f);
			pd->tc			=	ITC.book;
			pd->r			=	1.f-(float)i*0.1f;
			pd->g			=	(float)i*0.1f;
			pd->b			=	0.5f-(float)i*0.1f;
			pd->siz			=	32.f+i*4;
			pd->type		=	PARTICLE_2D;
		}
	}

	NewSpell=1;	
}

//-----------------------------------------------------------------------------
int ARX_GenereOneEtincelle(EERIE_3D *pos,EERIE_3D *dir)
{
	int	i;
	
	i=ARX_PARTICLES_GetFree();

	if(i<0) return -1;
	
	ParticleCount++;
	PARTICLE_DEF * pd=&particle[i];
	pd->exist		=	TRUE;
	pd->type		=	PARTICLE_ETINCELLE;
	pd->special		=	GRAVITY;
	pd->ov			=	pd->oldpos	=	*pos;
	pd->move		=	*dir;
	pd->tolive		=	1000+(int)(rnd()*500.f);
	pd->timcreation	=	lARXTime;
	pd->r			=	ET_R;
	pd->g			=	ET_G;
	pd->b			=	ET_B;
	pd->tc			=	GTC;
	pd->mask		=	ET_MASK;
	return i;
}

//-----------------------------------------------------------------------------
void ARX_GenereSpheriqueEtincelles(EERIE_3D *pos,float r,TextureContainer *tc,float rr,float g,float b,int mask)
{
	GTC=tc;
	ET_R=rr;
	ET_G=g;
	ET_B=b;
	ET_MASK=mask;
	int nb=rand()&0x1F;

	while(nb)
	{
		EERIE_3D dir;

		float a = DEG2RAD(rnd()*360.f);
		float b = DEG2RAD(rnd()*360.f);
		dir.x=(float) r*EEsin(a)*EEcos(b);
		dir.z=(float) r*EEsin(a)*EEsin(b);
		dir.y=(float) r*EEcos(a);

		ARX_GenereOneEtincelle(pos,&dir);
		nb--;
	}
}

//-----------------------------------------------------------------------------
// flags & 1 = spawn_body_chunk
void ARX_PARTICLES_Spawn_Splat(EERIE_3D * pos,float dmgs,D3DCOLOR col,long vert,INTERACTIVE_OBJ * io,long flags)
{
	float power;
	power=(dmgs*DIV60)+0.9f;

	float r,g,b;
	r=(float)((long)((col>>16) & 255))*DIV255;
	g=(float)((long)((col>>8) & 255))*DIV255;
	b=(float)((long)((col) & 255))*DIV255;

	for (long kk=0;kk<20;kk++)
	{
		long j=ARX_PARTICLES_GetFree();

		if (j!=-1)
		{
			ParticleCount++;
			PARTICLE_DEF * pd=&particle[j];
			pd->special		=	PARTICLE_SUB2 | SUBSTRACT | GRAVITY;
			pd->exist		=	TRUE;
			pd->ov.x		=	pos->x;
			pd->ov.y		=	pos->y;
			pd->ov.z		=	pos->z;
			pd->move.x		=	(rnd()*10-5.f)*2.3f;
			pd->move.y		=	(rnd()*10-5.f)*2.3f;
			pd->move.z		=	(rnd()*10-5.f)*2.3f;
			pd->timcreation	=	lARXTime;
			pd->tolive		=	(unsigned long)(1000+dmgs*3);
			pd->tc			=	blood_splat;
			pd->siz			=	(float)0.3f+0.01f*power;
			pd->scale.x		=	0.2f+(float)0.3f*power;
			pd->scale.y		=	0.2f+(float)0.3f*power;
			pd->scale.z		=	0.2f+(float)0.3f*power;
			pd->zdec		=	1;	
			pd->b			=	b;
			pd->g			=	g;
			pd->r			=	r;
		}
	}
}


//-----------------------------------------------------------------------------
void ARX_PARTICLES_SpawnWaterSplash(EERIE_3D *_ePos)
{
	for (long kk=0;kk<rnd()*15+20;kk++)
	{
		long j=ARX_PARTICLES_GetFree();

		if (j!=-1)
		{
			ParticleCount++;
			PARTICLE_DEF * pd=&particle[j];
			pd->special = FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING | GRAVITY;
			pd->special |= SPLAT_WATER;

			pd->exist		=	TRUE;
			pd->ov.x 		=	_ePos->x + rnd()*30;
			pd->ov.y 		=	_ePos->y - rnd()*20;
			pd->ov.z 		=	_ePos->z + rnd()*30;
			pd->move.x 		=	(frand2()*5)*1.3f;
			pd->move.y 		=	-(rnd()*5)*2.3f;
			pd->move.z 		=	(frand2()*5)*1.3f;
			pd->timcreation	=	lARXTime;
			pd->tolive		=	(unsigned long)(1000+rnd()*300);
			

			float fRandom	=	 rnd()*2;
			ARX_CHECK_INT(fRandom);
			
			int t = ARX_CLEAN_WARN_CAST_INT(fRandom);

			
			pd->tc=water_drop[t];
			pd->siz = 0.4f; 
			float s = rnd();
			pd->scale.x = 1;
			pd->scale.y = 1; 
			pd->scale.z = 1; 
			pd->zdec=1;	
			pd->b = s;
			pd->g = s; 
			pd->r = s;
		}
	}
}

//-----------------------------------------------------------------------------
void SpawnFireballTail(EERIE_3D * poss,EERIE_3D * vecto,float level,long flags)
{
	if (explo[0]==NULL) return;

	level*=2.f;

	for (long nn=0;nn<2;nn++)
	{
		long j=ARX_PARTICLES_GetFree();

		if (j!=-1)
		{
			ParticleCount++;
			PARTICLE_DEF * pd=&particle[j];
			pd->special		=	FIRE_TO_SMOKE | FADE_IN_AND_OUT | PARTICLE_ANIMATED | ROTATING | MODULATE_ROTATION;
			pd->fparam		=	0.02f-rnd()*0.02f;
			pd->exist		=	TRUE;
			pd->ov.x		=	poss->x;
			pd->ov.y		=	poss->y;
			pd->ov.z		=	poss->z;
			pd->move.x		=	0;
			pd->move.y		=	-rnd() * 3.f; 
			pd->move.z		=	0;
			pd->timcreation	=	lARXTime;
			pd->tc			=	explo[0];
			pd->r			=	0.7f;
			pd->g			=	0.7f;
			pd->b			=	0.7f;			
			pd->siz			=	level+2.f*rnd();

			if (flags & 1)
			{
				pd->tolive	=	(unsigned long)(400+rnd()*100);
				pd->siz		*=	0.7f;
				pd->scale.z=pd->scale.y=pd->scale.x=level*0.7f;				
			}
			else
			{				
				pd->scale.x=level;
				pd->scale.y=level;
				pd->scale.z=level;
				pd->tolive=(unsigned long)(800+rnd()*100);
			}

			pd->cval1=0;
			pd->cval2=MAX_EXPLO-1;

			if (nn==1)
			{
				pd->delay=(unsigned long)(float)(150+rnd()*100.f);
				pd->ov.x=poss->x+vecto->x*pd->delay;
				pd->ov.y=poss->y+vecto->y*pd->delay;
				pd->ov.z=poss->z+vecto->z*pd->delay;
			}
		}
	}
}

//-----------------------------------------------------------------------------
void LaunchFireballBoom(EERIE_3D * poss,float level,EERIE_3D * direction,EERIE_RGB * rgb)
{
	level*=1.6f;

	if (explo[0]==NULL) return;

	for (long nn=0;nn<1;nn++)
	{
		long j=ARX_PARTICLES_GetFree();

		if (j!=-1)
		{
			ParticleCount++;
			PARTICLE_DEF * pd=&particle[j];
			pd->special=FIRE_TO_SMOKE | FADE_IN_AND_OUT | PARTICLE_ANIMATED;
			pd->exist=TRUE;
			pd->ov.x=poss->x;
			pd->ov.y=poss->y;
			pd->ov.z=poss->z;

			if (direction)
				Vector_Copy(&pd->move,direction);
			else
			{
				pd->move.x = 0.f; 
				pd->move.y = -rnd() * 5.f; 
				pd->move.z = 0.f; 
			}

			pd->timcreation	=	lARXTime;
			pd->tolive		=	(unsigned long)(1600+rnd()*600);
			pd->tc			=	explo[0];
			pd->siz			=	level*3.f+2.f*rnd();
			pd->scale.x		=	level*3.f;
			pd->scale.y		=	level*3.f;
			pd->scale.z		=	level*3.f;
			pd->zdec		=	1;
			pd->cval1		=	0;
			pd->cval2		=	MAX_EXPLO-1;

			if (rgb)
			{
				pd->r=rgb->r;
				pd->g=rgb->g;
				pd->b=rgb->b;
			}

			if (nn==1)
			{
				pd->delay=300;
				pd->tolive/=2;				
				pd->scale.x*=2;
				pd->scale.y*=2;
				pd->scale.z*=2;
			}
		}
	}
}

//-----------------------------------------------------------------------------
void ARX_PARTICLES_Render(LPDIRECT3DDEVICE7 pd3dDevice,EERIE_CAMERA * cam) 
{
	if (!ACTIVEBKG) return;

	TreatBackgroundActions();

	if 	(ParticleCount==0) return;

	register long i;
	register long xx,yy;
	register unsigned  long tim;
	long framediff;
	long framediff2;
	long t;
	register D3DTLVERTEX in,inn,out;
	register D3DCOLOR color;
	register float siz,siz2,r;
	register float val;
	register float fd;
	register float rott;
	
	tim = ARXTimeUL();//treat warning C4244 conversion from 'float' to 'unsigned long'	
	
	SETCULL(pd3dDevice,D3DCULL_NONE);

	GDevice->SetRenderState(D3DRENDERSTATE_FOGCOLOR,0);

	TextureContainer * tc=NULL;
	long pcc=ParticleCount;

	for (i=0;i<MAX_PARTICLES;i++) 
	{
		PARTICLE_DEF * part=&particle[i];

		if (part->exist) 
		{
			framediff=part->timcreation+part->tolive-tim;
			framediff2=tim-part->timcreation;	

			if (framediff2 < (long)part->delay) continue;

			if (part->delay>0)
			{
				part->timcreation+=part->delay;
				part->delay=0;

				if ((part->special & DELAY_FOLLOW_SOURCE)  && (part->sourceionum>=0) && (inter.iobj[part->sourceionum]))
				{
					part->ov.x = part->source->x; 
					part->ov.y = part->source->y; 
					part->ov.z = part->source->z; 
					INTERACTIVE_OBJ * target=inter.iobj[part->sourceionum];
					EERIE_3D vector;
					vector.x=part->ov.x-target->pos.x;
					vector.y=(part->ov.y-target->pos.y)*DIV2;
					vector.z=part->ov.z-target->pos.z;
					float t=1.f/TRUEVector_Magnitude(&vector);
					vector.x*=t;
					vector.y*=t;
					vector.z*=t;

					part->move.x = vector.x * 18 + rnd() - 0.5f; 
					part->move.y = vector.y * 5 + rnd() - 0.5f; 
					part->move.z = vector.z * 18 + rnd() - 0.5f; 
					
				}				

				continue;
			}

			if (!(part->type & PARTICLE_2D))
			{
				F2L((part->ov.x*ACTIVEBKG->Xmul),&xx);			
				F2L((part->ov.z*ACTIVEBKG->Zmul),&yy);

				if ((xx<0) || (yy<0) || (xx>ACTIVEBKG->Xsize) || (yy>ACTIVEBKG->Zsize))
				{
					part->exist=FALSE;
					ParticleCount--;
					continue;
				}

				FAST_BKG_DATA * feg=(FAST_BKG_DATA *)&ACTIVEBKG->fastdata[xx][yy];

				if (!feg->treat)
				{
					part->exist=FALSE;
					ParticleCount--;
					continue;
				}
			}	
			
			if (framediff<=0) 
			{
				if ((part->special & FIRE_TO_SMOKE) && (rnd()>0.7f))
				{
					part->ov.x+=part->move.x;
					part->ov.y+=part->move.y;
					part->ov.z+=part->move.z;
					part->tolive += (part->tolive >> 2) + (part->tolive >> 3);
					part->special&=~FIRE_TO_SMOKE;
					part->timcreation=tim;
					part->tc = smokeparticle;
					part->scale.x*=2.4f;
					part->scale.y*=2.4f;
					part->scale.z*=2.4f;

					if (part->scale.x<0.f) part->scale.x*=-1.f;

					if (part->scale.y<0.f) part->scale.y*=-1.f;

					if (part->scale.z<0.f) part->scale.z*=-1.f;

					part->r=0.45f;
					part->g=0.45f;
					part->b=0.45f;
					part->move.x*=DIV2;
					part->move.y*=DIV2;
					part->move.z*=DIV2;
					part->siz*=DIV3;			
					part->special&=~FIRE_TO_SMOKE;
					part->timcreation=tim;
					part->tc = smokeparticle; 
					
					framediff=part->timcreation+part->tolive-tim;
				}
				else 
				{
					part->exist=FALSE;
					ParticleCount--;
					continue;
				}
			}
			
			
			if((part->special & FIRE_TO_SMOKE2)&&(framediff2>(long)(part->tolive-(part->tolive>>2))))
			{
				part->special&=~FIRE_TO_SMOKE2;
				int j=ARX_PARTICLES_GetFree();

				if(j>=0)
				{
					ParticleCount++;
					PARTICLE_DEF * pd=&particle[j];
					particle[j]=particle[i]; 
					pd->exist=1;
					pd->zdec=0;
					pd->special|=SUBSTRACT;					
					pd->ov.x=part->oldpos.x;
					pd->ov.y=part->oldpos.y;
					pd->ov.z=part->oldpos.z;
					pd->timcreation=tim;
					pd->tc = tzupouf; 
					pd->scale.x*=4.f;
					pd->scale.y*=4.f;
					pd->scale.z*=4.f;

					if (pd->scale.x<0.f) pd->scale.x*=-1.f;

					if (pd->scale.y<0.f) pd->scale.y*=-1.f;

					if (pd->scale.z<0.f) pd->scale.z*=-1.f;

					pd->r = 1.f; 
					pd->g = 1.f; 
					pd->b = 1.f; 
					pd->move.x*=DIV2;
					pd->move.y*=DIV2;
					pd->move.z*=DIV2;
					pd->siz*=DIV3;			
				}
			}
			
			val=(part->tolive-framediff)*DIV100;
			
			if ((part->special & FOLLOW_SOURCE) && (part->sourceionum>=0) && (inter.iobj[part->sourceionum]))
			{
				inn.sx=in.sx=part->source->x;
				inn.sy=in.sy=part->source->y;
				inn.sz=in.sz=part->source->z;
			}
			else if ((part->special & FOLLOW_SOURCE2) && (part->sourceionum>=0) && (inter.iobj[part->sourceionum]))
			{
				inn.sx=in.sx=part->source->x+part->move.x*val;
				inn.sy=in.sy=part->source->y+part->move.y*val;
				inn.sz=in.sz=part->source->z+part->move.z*val;
			}
			else
			{
				inn.sx=in.sx=part->ov.x+part->move.x*val;
				inn.sy=in.sy=part->ov.y+part->move.y*val;
				inn.sz=in.sz=part->ov.z+part->move.z*val;
			}

			if (part->special & GRAVITY)
			{
				inn.sy += 0.98f * 1.5f * val * val;
				in.sy=inn.sy;
			}
			
			if (part->special & PARTICLE_NOZBUFFER) 
			{
				GDevice->SetRenderState(D3DRENDERSTATE_ZENABLE,FALSE);
			}
			else
			{
				GDevice->SetRenderState(D3DRENDERSTATE_ZENABLE,TRUE);
			}

			if (part->special & FADE_IN_AND_OUT) 
			{
				t=part->tolive>>1;

				if (framediff2<=t)
				{
					r=((float)framediff2/(float)t);
				}
				else 
				{
					fd=((float)(framediff2-t)/(float)t);
					r=1.f-fd;
				}

				fd=((float)framediff2/(float)part->tolive);
			}
			else
			{
				fd=((float)framediff2/(float)part->tolive);
				r=1.f-fd;
			}

			if (!(part->type & PARTICLE_2D))
			{
				EERIE_SPHERE sp;
				sp.origin.x=in.sx;
				sp.origin.y=in.sy;
				sp.origin.z=in.sz;
				EERIETreatPoint(&inn,&out);			

				if (out.rhw<0) continue;

				if (out.sz>cam->cdepth*fZFogEnd) continue;

				if (part->special & PARTICLE_SPARK)
				{
					if (part->special & NO_TRANS)
					{
						SETALPHABLEND(pd3dDevice,FALSE);
					}
					else
					{
						SETALPHABLEND(pd3dDevice,TRUE);

						if (part->special & SUBSTRACT) 
						{
							pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ZERO);
							pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCCOLOR);
						}
						else
						{
							pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ONE);
							pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);
						}
					}

					SETCULL(pd3dDevice,D3DCULL_NONE);
					EERIE_3D vect;
					vect.x=part->oldpos.x-in.sx;
					vect.y=part->oldpos.y-in.sy;
					vect.z=part->oldpos.z-in.sz;
					Vector_Normalize(&vect);
					D3DTLVERTEX tv[3];
					tv[0].color=D3DRGB(part->r,part->g,part->b);					
					tv[1].color=0xFF666666;
					tv[2].color = 0xFF000000; 
					tv[0].sx=out.sx;
					tv[0].sy=out.sy;
					tv[0].sz=out.sz;
					tv[0].rhw=out.rhw;					
					D3DTLVERTEX temp;
					temp.sx=in.sx+rnd()*0.5f;
					temp.sy=in.sy+0.8f;
					temp.sz=in.sz+rnd()*0.5f;
					EERIETreatPoint(&temp,&tv[1]);
					temp.sx=in.sx+vect.x*part->fparam;
					temp.sy=in.sy+vect.y*part->fparam;
					temp.sz=in.sz+vect.z*part->fparam;

					EERIETreatPoint(&temp,&tv[2]);
					SETTC(pd3dDevice,NULL);
					ComputeFogVertex(tv);

					EERIEDRAWPRIM(pd3dDevice,D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX| D3DFVF_DIFFUSE , tv, 3,  0, bSoftRender?EERIE_USEVB:0 );
					if(!ARXPausedTimer)
					{
						part->oldpos.x=in.sx;
						part->oldpos.y=in.sy;
						part->oldpos.z=in.sz;
					}

					continue;
				}

				if (part->special & SPLAT_GROUND)
				{
					siz=part->siz+part->scale.x*fd;			
					sp.radius=siz*10;

					if (CheckAnythingInSphere(&sp,0,1))
					{
						EERIE_RGB rgb;
						rgb.r=part->r;
						rgb.g=part->g;
						rgb.b=part->b;

						if (rnd()<0.9f)
							SpawnGroundSplat(&sp,&rgb,sp.radius,0);

						part->exist=FALSE;
						ParticleCount--;
						continue;
					}
				}

				if (part->special & SPLAT_WATER)
				{
					siz=part->siz+part->scale.x*fd;			
					sp.radius=siz*(10 + rnd()*20);

					if (CheckAnythingInSphere(&sp,0,1))
					{
						EERIE_RGB rgb;
						rgb.r=part->r*0.5f;
						rgb.g=part->g*0.5f;
						rgb.b=part->b*0.5f;

						if (rnd()<0.9f)
							SpawnGroundSplat(&sp,&rgb,sp.radius,2);

						part->exist=FALSE;
						ParticleCount--;
						continue;
					}
				}
				
			}

			if ((part->special & DISSIPATING) && (out.sz<0.05f))
			{
				out.sz=(out.sz)*20.f;
				r*=out.sz;
			}

			if (r>0.f) 
			{
				if (part->special & NO_TRANS)
				{
					SETALPHABLEND(pd3dDevice,FALSE);
				}
				else
				{
					SETALPHABLEND(pd3dDevice,TRUE);

					if (part->special & SUBSTRACT) 
					{
						pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ZERO);
						pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCCOLOR);
					}
					else
					{
						pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ONE);
						pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);
					}
				}
				
				EERIE_3D op=part->oldpos;

				if(!ARXPausedTimer)
				{
					part->oldpos.x=in.sx;
					part->oldpos.y=in.sy;
					part->oldpos.z=in.sz; 
				}

				if (part->special & PARTICLE_GOLDRAIN)
				{
					float v=(rnd()-0.5f)*DIV5;

					if (	(part->r+v<=1.f) && (part->r+v>0.f)
						&&	(part->g+v<=1.f) && (part->g+v>0.f)
						&&	(part->b+v<=1.f) && (part->b+v>0.f) )
					{
						part->r+=v;
						part->g+=v;
						part->b+=v;
					}
				}

				if (Project.improve) 
				{
					color=D3DRGB(part->r*r,0.f,part->b*r);
				}
				else	color=D3DRGB(part->r*r,part->g*r,part->b*r);

				tc=part->tc;

				if ((tc==explo[0]) && (part->special & PARTICLE_ANIMATED))
				{
					long animrange=part->cval2-part->cval1;
					float tt=(float)framediff2/(float)part->tolive*animrange;
					long num;
					F2L(tt,&num);

					if (num<part->cval1) num=part->cval1;

					if (num>part->cval2) num=part->cval2;

					tc=explo[num];
				}

				siz=part->siz+part->scale.x*fd;			

				if (part->special & ROTATING) 
				{
					if (part->special & MODULATE_ROTATION) rott=MAKEANGLE((float)(tim+framediff2)*part->fparam);
					else rott=(MAKEANGLE((float)(tim+framediff2*2)*DIV4));

					if (part->type & PARTICLE_2D) 
					{}
					else 
					{
						float temp;

						if (part->zdec) temp=0.0001f;
						else temp=2.f;
						
						if (part->special & PARTICLE_SUB2) 
						{
							D3DTLVERTEX in2;
							memcpy(&in2,&in,sizeof(D3DTLVERTEX));
							pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ONE);
							pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);						
							EERIEDrawRotatedSprite(pd3dDevice,&in,siz,tc,color,temp,rott);
							pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ZERO);
							pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCCOLOR);						
							EERIEDrawRotatedSprite(pd3dDevice,&in2,siz,tc,0xFFFFFFFF,temp,rott);
						}
						else
							EERIEDrawRotatedSprite(pd3dDevice,&in,siz,tc,color,temp,rott);
					}					
				}
				else if (part->type & PARTICLE_2D) 
				{
					siz2=part->siz+part->scale.y*fd;
					
					if (part->special & PARTICLE_SUB2) 
					{
						D3DTLVERTEX in2;
						memcpy(&in2,&in,sizeof(D3DTLVERTEX));
						pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ONE);
						pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);						
						EERIEDrawBitmap(pd3dDevice,in.sx,in.sy,siz,siz2,in.sz,tc,color);
						pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ZERO);
						pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCCOLOR);						
						EERIEDrawBitmap(pd3dDevice,in2.sx,in.sy,siz,siz2,in.sz,tc,0xFFFFFFFF);
					}
					else
						EERIEDrawBitmap(pd3dDevice,in.sx,in.sy,siz,siz2,in.sz,tc,color);
				}
				else 
				{
					if(part->type & PARTICLE_ETINCELLE)
					{
						EERIE_3D pos,end;
						pos.x=in.sx;
						pos.y=in.sy;
						pos.z=in.sz;
						int color1=RGBA_MAKE((int)(part->r*255.f*r),(int)(part->g*255.f*r),(int)(part->b*255.f*r),255);
						end.x=pos.x-(pos.x-op.x)*2.5f;
						end.y=pos.y-(pos.y-op.y)*2.5f;
						end.z=pos.z-(pos.z-op.z)*2.5f;
						Draw3DLineTex2(pd3dDevice,end,pos,2.f,color1 & part->mask,color1);
						
						EERIEDrawSprite(pd3dDevice,&in,.7f,tc,color1,2.f);				
						
					}
					else
					{
						float temp;

						if (part->zdec) temp=0.0001f;
						else temp=2.f;
						
						if (part->special & PARTICLE_SUB2) 
						{
							D3DTLVERTEX in2;
							memcpy(&in2,&in,sizeof(D3DTLVERTEX));
							pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ONE);
							pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);
							EERIEDrawSprite(pd3dDevice,&in,siz,tc,color,temp);				
							pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ZERO);
							pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCCOLOR);
							EERIEDrawSprite(pd3dDevice,&in2,siz,tc,0xFFFFFFFF,temp);				
						}
						else 
							EERIEDrawSprite(pd3dDevice,&in,siz,tc,color,temp);				
					}
				}
			}

			pcc--;

			if (pcc<=0)
			{
				GDevice->SetRenderState(D3DRENDERSTATE_FOGCOLOR,ulBKGColor);
				return;
			}
		}	
	}

	GDevice->SetRenderState(D3DRENDERSTATE_FOGCOLOR,ulBKGColor);
	GDevice->SetRenderState(D3DRENDERSTATE_ZENABLE,TRUE);
}

//-----------------------------------------------------------------------------
void RestoreAllLightsInitialStatus()
{
	for (long i=0;i<MAX_LIGHTS;i++)
	{
		if ( (GLight[i]!=NULL) )
		{
			GLight[i]->status=1;

			if (GLight[i]->extras & EXTRAS_STARTEXTINGUISHED) GLight[i]->status=0;

			if (GLight[i]->status==0)
			{
				if (ValidDynLight(GLight[i]->tl))
				{
					DynLight[GLight[i]->tl].exist=0;
				}

				GLight[i]->tl=-1;
			}
		}
	}
}
extern long FRAME_COUNT;
//-----------------------------------------------------------------------------
// Draws Flame Particles
//-----------------------------------------------------------------------------
void TreatBackgroundActions()
{
	if (FRAME_COUNT>0) return;

	long n,j;
	float sx,sy,sz;


	float fZFar=ACTIVECAM->cdepth*fZFogEnd*1.3f;	

	for (long i=0;i<MAX_LIGHTS;i++)
	{
		EERIE_LIGHT * gl=GLight[i];

		if (gl==NULL) continue;
		
		float dist=EEDistance3D(&gl->pos,	&ACTIVECAM->pos);

		if ( dist>fZFar ) // Out of Treat Range
		{
			ARX_SOUND_Stop(gl->sample);
			gl->sample = ARX_SOUND_INVALID_RESOURCE;
			continue;
		}

		
		if ((gl->extras & EXTRAS_SPAWNFIRE) &&	(gl->status))
			{
				long id=ARX_DAMAGES_GetFree();

				if (id!=-1)
				{
				damages[id].radius = gl->ex_radius; 
				damages[id].damages = gl->ex_radius * DIV7; 
					damages[id].area=DAMAGE_FULL;
					damages[id].duration=1;
					damages[id].source=-5;
				damages[id].flags = 0; 
					damages[id].type=DAMAGE_TYPE_MAGICAL | DAMAGE_TYPE_FIRE | DAMAGE_TYPE_NO_FIX;
					damages[id].exist=TRUE;
					damages[id].pos.x=gl->pos.x;
					damages[id].pos.y=gl->pos.y;
					damages[id].pos.z=gl->pos.z;
				}
			}

		if ((  (gl->extras & EXTRAS_SPAWNFIRE) 
			|| (gl->extras & EXTRAS_SPAWNSMOKE))
			&& (gl->status))
		{
			if (gl->sample == ARX_SOUND_INVALID_RESOURCE)
			{
				gl->sample = SND_FIREPLACE;
					ARX_SOUND_PlaySFX(gl->sample, &gl->pos, 0.95F + 0.1F * rnd(), ARX_SOUND_PLAY_LOOPED);
			}
			else
			{
				ARX_SOUND_RefreshPosition(gl->sample, &gl->pos);
			}


			long count;

				if (dist<ACTIVECAM->cdepth*DIV8) count=4;
			else if (dist<ACTIVECAM->cdepth*DIV6) count=4;
			else if (dist>ACTIVECAM->cdepth*DIV3) count=3;
			else count=2;

			for (n=0;n<count;n++)
			{
				j=ARX_PARTICLES_GetFree();

				if ((j!=-1) && (!ARXPausedTimer) && ((rnd()<gl->ex_frequency)))
				{
					ParticleCount++;
					PARTICLE_DEF * pd=&particle[j];
					pd->exist=TRUE;
					pd->zdec=0;
					sy=rnd()*3.14159f;
					sx=EEsin(sy);
					sz=EEcos(sy);
					sy=EEsin(sy);
					pd->ov.x=gl->pos.x+gl->ex_radius*sx*rnd();
					pd->ov.y=gl->pos.y+gl->ex_radius*sy*rnd();
					pd->ov.z=gl->pos.z+gl->ex_radius*sz*rnd();
					pd->move.x=(2.f-4.f*rnd())*gl->ex_speed;
					pd->move.y=(2.f-22.f*rnd())*gl->ex_speed;
					pd->move.z=(2.f-4.f*rnd())*gl->ex_speed;
					pd->siz=7.f*gl->ex_size;
					pd->tolive=500+(unsigned long)(rnd()*1000.f*gl->ex_speed);

					if ((gl->extras & EXTRAS_SPAWNFIRE) && (gl->extras & EXTRAS_SPAWNSMOKE))
						pd->special=FIRE_TO_SMOKE;
					else pd->special=0;

					if (gl->extras & EXTRAS_SPAWNFIRE) 
						pd->tc = fire2; 
					else 
						pd->tc = smokeparticle;
					
					pd->special		|=	ROTATING | MODULATE_ROTATION;
					pd->fparam		=	0.1f-rnd()*0.2f*gl->ex_speed;
					pd->scale.x		=	-8.f;
					pd->scale.y		=	-8.f;
					pd->scale.z		=	-8.f;
					pd->timcreation	=	lARXTime;
					
					if (gl->extras & EXTRAS_COLORLEGACY)
					{
						pd->r=gl->rgb.r;
						pd->g=gl->rgb.g;
						pd->b=gl->rgb.b;
					}
					else pd->r=pd->g=pd->b=1.f;
				}

				if ((gl->extras & EXTRAS_SPAWNFIRE) && (rnd()>0.95f))
				{
					j=ARX_PARTICLES_GetFree();

					if ((j!=-1) && (!ARXPausedTimer) && ((rnd()<gl->ex_frequency)))
					{
						ParticleCount++;
						PARTICLE_DEF * pd=&particle[j];
						pd->exist=TRUE;
						pd->zdec=0;
						sy = rnd() * 3.14159f * 2.f - 3.14159f; 
						sx=EEsin(sy);
						sz=EEcos(sy);
						sy=EEsin(sy);
						pd->ov.x=gl->pos.x+gl->ex_radius*sx*rnd();
						pd->ov.y=gl->pos.y+gl->ex_radius*sy*rnd();
						pd->ov.z=gl->pos.z+gl->ex_radius*sz*rnd();
						EERIE_3D vect;
						vect.x=pd->ov.x-gl->pos.x;
						vect.y=pd->ov.y-gl->pos.y;
						vect.z=pd->ov.z-gl->pos.z;
						TRUEVector_Normalize(&vect);

						if (gl->extras & EXTRAS_FIREPLACE)
						{
							pd->move.x = vect.x * 6.f * gl->ex_speed; 
							pd->move.y=(-10.f-8.f*rnd())*gl->ex_speed;
							pd->move.z = vect.z * 6.f * gl->ex_speed; 
						}
						else
						{
							pd->move.x = vect.x * 4.f * gl->ex_speed; 
							pd->move.y=(-10.f-8.f*rnd())*gl->ex_speed;
							pd->move.z = vect.z * 4.f * gl->ex_speed; 
						}

						pd->siz=4.f*gl->ex_size*0.3f;
						pd->tolive=1200+(unsigned long)(rnd()*500.f*gl->ex_speed);
						pd->special=0;//FIRE_TO_SMOKE;
						pd->tc = fire2; 
						
						pd->special		|=	ROTATING | MODULATE_ROTATION | GRAVITY;
						pd->fparam		=	0.1f-rnd()*0.2f*gl->ex_speed;
						pd->scale.x		=	-3.f;
						pd->scale.y		=	-3.f;
						pd->scale.z		=	-3.f;
						pd->timcreation	=	lARXTime;
						
						if (gl->extras & EXTRAS_COLORLEGACY)
						{
							pd->r=gl->rgb.r;
							pd->g=gl->rgb.g;
							pd->b=gl->rgb.b;
						}
						else pd->r=pd->g=pd->b=1.f;
					}
				}
			}
			
		}
		else
		{
			if ((!gl->status) && (gl->sample != ARX_SOUND_INVALID_RESOURCE))
			{
				ARX_SOUND_Stop(gl->sample);
				gl->sample = ARX_SOUND_INVALID_RESOURCE;
			}
		}
	}	
}

//-----------------------------------------------------------------------------
void ARX_MAGICAL_FLARES_FirstInit()
{	
	flarenum=0;

	for(long i=0;i<MAX_FLARES;i++) flare[i].exist=0; 

	g_Lumignon[0] = D3DTLVERTEX( D3DVECTOR( 0, 0, 0.5 ), 0.5, 0xFFFFFFFF, 0, 0, 1 );
    g_Lumignon[1] = D3DTLVERTEX( D3DVECTOR( 0, 0, 0.5 ), 0.5, 0xFFFFFFFF, 0, 0, 0 );
    g_Lumignon[2] = D3DTLVERTEX( D3DVECTOR( 0, 0, 0.5 ), 0.5, 0xFFFFFFFF, 0, 1, 1 );
    g_Lumignon[3] = D3DTLVERTEX( D3DVECTOR( 0, 0, 0.5 ), 0.5, 0xFFFFFFFF, 0, 1, 0 );	
}
void ARX_MAGICAL_FLARES_KillAll()
{
	for (long i=0;i<MAX_FLARES;i++)
	{
		if (flare[i].exist)
		{
			if (flare[i].io)
			{
				flare[i].io->flarecount--;
			}

			flare[i].exist=0;
			flare[i].tolive=0;
			flarenum--;

			if (ValidDynLight(flare[i].dynlight!=-1))
				DynLight[flare[i].dynlight].exist=0;

			flare[i].dynlight=-1;
		}
	}

	flarenum=0;
}
 

//-----------------------------------------------------------------------------
void AddFlare(EERIE_S2D * pos,float sm,short typ,INTERACTIVE_OBJ * io)
{
	long i;
	float vx,vy;
	EERIE_CAMERA ka;
	float zz;

	for (i=0;i<MAX_FLARES;i++) 
	{
		if ( !flare[i].exist ) 
		{
			FLARES * fl=&flare[i];
			fl->exist=1;
			fl->bDrawBitmap=0;

			if (io)
			{
				fl->flags=1;
				fl->io=io;
				io->flarecount++;
			}
			else
			{
				fl->flags=0;
				fl->io=NULL;
			}

			flarenum++;
			fl->x=pos->x-rnd()*4;
			fl->y=pos->y-rnd()*4-50;
			
			fl->tv.rhw=fl->v.rhw=1.f;
			fl->tv.specular=fl->v.specular=1;
			memcpy(&ka,Kam,sizeof(EERIE_CAMERA));
			
			ka.angle.a=360.f-ka.angle.a;
			ka.angle.b=360.f-ka.angle.b;
			ka.angle.g=360.f-ka.angle.g;
			EERIE_CAMERA * oldcam=ACTIVECAM;
			SetActiveCamera(&ka);
			PrepareCamera(&ka);
			fl->v.sx+=ka.pos.x;
			fl->v.sy+=ka.pos.y;
			fl->v.sz+=ka.pos.z;
			EE_RTT(&fl->tv,&fl->v);
			fl->v.sx+=ka.pos.x;
			fl->v.sy+=ka.pos.y;
			fl->v.sz+=ka.pos.z;
			
			vx=-(fl->x-subj.centerx);
			vy=(fl->y-subj.centery);
			vx*=0.2173913f;///=4.6f;
			vy*=0.1515151515151515f;///=6.6f;
			
			if (io)
			{
				fl->v.sx=io->pos.x-(float)EEsin(DEG2RAD(MAKEANGLE(io->angle.b+vx)))*100.f;
				fl->v.sy=io->pos.y+(float)EEsin(DEG2RAD(MAKEANGLE(io->angle.a+vy)))*100.f-150.f;
				fl->v.sz=io->pos.z+(float)EEcos(DEG2RAD(MAKEANGLE(io->angle.b+vx)))*100.f;
				
			}
			else
			{
				fl->v.sz=75.f;
				fl->v.sx=((float)(pos->x-(DANAESIZX>>1)))*fl->v.sz*2.f/(float)DANAESIZX;
				fl->v.sy=((float)(pos->y-(DANAESIZY>>1)))*fl->v.sz*2.f/((float)DANAESIZY*((float)DANAESIZX/(float)DANAESIZY));

				ka=*oldcam;
				SetActiveCamera(&ka);
				PrepareCamera(&ka);

				float temp=(fl->v.sy*-ka.Xsin) + (fl->v.sz*ka.Xcos);
				fl->v.sy = (fl->v.sy*ka.Xcos) - (-fl->v.sz*ka.Xsin);
				fl->v.sz =(temp*ka.Ycos) - (-fl->v.sx*ka.Ysin);
				fl->v.sx = (temp*-ka.Ysin) + (fl->v.sx*ka.Ycos);	

				fl->v.sx+=oldcam->pos.x;
				fl->v.sy+=oldcam->pos.y;
				fl->v.sz+=oldcam->pos.z;
			}
			
			fl->tv.sx=fl->v.sx;
			fl->tv.sy=fl->v.sy;
			fl->tv.sz=fl->v.sz;
			SetActiveCamera(oldcam);
			
			switch (PIPOrgb) 
			{
			case 0:
				fl->g=rnd()*DEUXTIERS;
				fl->r=(rnd()*DEUXTIERS)+0.4f;
				fl->b=(rnd()*DEUXTIERS)+0.4f;
				break;
			case 1:
				fl->b=rnd()*0.55f;
				fl->g=(rnd()*0.625f)+0.5f;
				fl->r=(rnd()*0.625f)+0.5f;
				break;
			case 2:
				fl->b=rnd()*0.55f;
					fl->g = rnd() * 0.55f;
				fl->r=(rnd()*DEUXTIERS)+0.4f;
				break;			
			}
			
			if (typ == -1 ) 
			{
				if (EERIEMouseButton & 1) zz = 0.29f;
				else if (sm>0.5f) zz=rnd();
				else zz=1.f;

				if ( zz < 0.2f ) 
				{
					fl->type=2;
					fl->size=((rnd()*42)+42.f);
					fl->tolive=((800.f+rnd()*800.f)*(float)FLARE_MUL);
				}
				else if ( zz < 0.5f ) 
				{
					fl->type=3;
					fl->size=((rnd()*52)+16.f);
					fl->tolive=((800.f+rnd()*800.f)*(float)FLARE_MUL);
				}
				else 
				{
					fl->type=1;
					fl->size=((rnd()*24)+32.f) *sm;
					fl->tolive=((1700.f+rnd()*500.f)*(float)FLARE_MUL);
				}
			}
			else
			{
				zz=rnd();

				if (zz>0.8f) fl->type=1;
				else fl->type=4;

				fl->size=((rnd()*38)+64.f) *sm;
				fl->tolive=((1700.f+rnd()*500.f)*(float)FLARE_MUL);
			}

			fl->dynlight=-1;
			fl->move=OPIPOrgb;
			
			if (!HIDEMAGICDUST)
			{
				for (long kk=0;kk<3;kk++)
				{
					long j=ARX_PARTICLES_GetFree();

					if ((j!=-1) && (!ARXPausedTimer) && (rnd()*100.f<50.f))
					{
						ParticleCount++;
						PARTICLE_DEF * pd=&particle[j];
						pd->special=FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;

						if(!io)
						{
							pd->special|=PARTICLE_NOZBUFFER;
						}

						pd->exist		=	TRUE;
						pd->zdec		=	0;
						pd->ov.x		=	fl->v.sx+rnd()*10.f-5.f;
						pd->ov.y		=	fl->v.sy+rnd()*10.f-5.f;
						pd->ov.z		=	fl->v.sz+rnd()*10.f-5.f;
						
						pd->move.x		=	0.f;
						pd->move.y		=	5.f;
						pd->move.z		=	0.f;
						
						pd->scale.x		=	-2.f;
						pd->scale.y		=	-2.f;
						pd->scale.z		=	-2.f;
						pd->timcreation	=	lARXTime;
						pd->tolive		=	(unsigned long)(float)(1300.f+rnd()*800.f+kk*100.f);
						
						pd->tc			=	fire2;

						if (kk==1) 
						{
							pd->move.y = 5.f - (float)kk; 
							pd->siz = 1.f + (float)(kk) * DIV2; 
						}
						else pd->siz=1.f+rnd()*1.f;

						pd->r=fl->r*DEUXTIERS;
						pd->g=fl->g*DEUXTIERS;
						pd->b=fl->b*DEUXTIERS;
						pd->fparam=1.2f;
					}
				}
			}
			return;					
		}
	}
}

//-----------------------------------------------------------------------------
void AddFlare2(EERIE_S2D * pos,float sm,short typ,INTERACTIVE_OBJ * io)
{
	long i;
	float zz;

	for (i=0;i<MAX_FLARES;i++) 
	{
		if ( !flare[i].exist ) 
		{
			FLARES * fl=&flare[i];
			fl->exist=1;
			fl->bDrawBitmap=1;

			if (io)
			{
				fl->flags=1;
				fl->io=io;
				io->flarecount++;
			}
			else
			{
				fl->flags=0;
				fl->io=NULL;
			}

			flarenum++;
			fl->x=pos->x-rnd()*4;
			fl->y=pos->y-rnd()*4-50;
			
			fl->tv.rhw=fl->v.rhw=1.f;
			fl->tv.specular=fl->v.specular=1;
			
			fl->tv.sx=fl->x;
			fl->tv.sy=fl->y;
			fl->tv.sz = 0.001f; 

			switch (PIPOrgb) 
			{
			case 0:
				fl->g=rnd()*DEUXTIERS;
				fl->r=(rnd()*DEUXTIERS)+0.4f;
				fl->b=(rnd()*DEUXTIERS)+0.4f;
				break;
			case 1:
				fl->b=rnd()*0.55f;
				fl->g=(rnd()*0.625f)+0.5f;
				fl->r=(rnd()*0.625f)+0.5f;
				break;
			case 2:
				fl->b=rnd()*0.55f;
					fl->g = rnd() * 0.55f; 
				fl->r=(rnd()*DEUXTIERS)+0.4f;
				break;			
			}
			
			if (typ == -1 ) 
			{
				if (EERIEMouseButton & 1) zz = 0.29f;
				else if (sm>0.5f) zz=rnd();
				else zz=1.f;

				if ( zz < 0.2f ) 
				{
					fl->type=2;
					fl->size=((rnd()*42)+42.f);
					fl->tolive=((800.f+rnd()*800.f)*(float)FLARE_MUL);
				}
				else if ( zz < 0.5f ) 
				{
					fl->type=3;
					fl->size=((rnd()*52)+16.f);
					fl->tolive=((800.f+rnd()*800.f)*(float)FLARE_MUL);
				}
				else 
				{
					fl->type=1;
					fl->size=((rnd()*24)+32.f) *sm;
					fl->tolive=((1700.f+rnd()*500.f)*(float)FLARE_MUL);
				}
			}
			else
			{
				zz=rnd();

				if (zz>0.8f) fl->type=1;
				else fl->type=4;

				fl->size=((rnd()*38)+64.f) *sm;
				fl->tolive=((1700.f+rnd()*500.f)*(float)FLARE_MUL);
			}

			fl->dynlight=-1;
			fl->move=OPIPOrgb;
			
			if (!HIDEMAGICDUST)
				for (long kk=0;kk<3;kk++)
				{
					long j=ARX_PARTICLES_GetFree();

					if ((j!=-1) && (!ARXPausedTimer) && (rnd()*100.f<50.f))
					{
					
						ParticleCount++;
						PARTICLE_DEF * pd=&particle[j];
						pd->special		=	FADE_IN_AND_OUT;
						pd->exist		=	TRUE;
						pd->zdec		=	0;
						pd->ov.x		=	fl->v.sx+rnd()*10.f-5.f;
						pd->ov.y		=	fl->v.sy+rnd()*10.f-5.f;
						pd->ov.z		=	fl->v.sz+rnd()*10.f-5.f;
						
						pd->move.x		=	0.f;
						pd->move.y		=	5.f;
						pd->move.z		=	0.f;
						
						pd->scale.x		=	-2.f;
						pd->scale.y		=	-2.f;
						pd->scale.z		=	-2.f;
						pd->timcreation	=	lARXTime;
						pd->tolive		=	(unsigned long)(float)(1300.f+rnd()*800.f+kk*100.f);
						
						pd->tc			=	fire2;

						if (kk==1) 
						{
							pd->move.y = 5.f - (float)kk; 
							pd->siz = 1.f + (float)(kk) * DIV2; 
						}
						else pd->siz=1.f+rnd()*1.f;

						pd->r=fl->r*DEUXTIERS;
						pd->g=fl->g*DEUXTIERS;
						pd->b=fl->b*DEUXTIERS;
						pd->fparam=1.2f;
						pd->type=PARTICLE_2D;
					}
				}

				i=MAX_FLARES + 1;
		}
	}
}

//-----------------------------------------------------------------------------
void AddLFlare(float x, float y,INTERACTIVE_OBJ * io) 
{
	EERIE_S2D pos;
	pos.x=(short)x;
	pos.y=(short)y;
	AddFlare(&pos,0.45f,1,io);	
}

//-----------------------------------------------------------------------------
void FlareLine(EERIE_S2D * pos0, EERIE_S2D * pos1, INTERACTIVE_OBJ * io) 
{
	float dx,dy,adx,ady,m;
	long i;
	long z;
	float x0=pos0->x;
	float x1=pos1->x;
	float y0=pos0->y;
	float y1=pos1->y;
	dx=(x1-x0);
	adx=EEfabs(dx);
	dy=(y1-y0);
	ady=EEfabs(dy);

	if (adx>ady) 
	{
		if (x0>x1) 
		{
			F2L(x1,&z);
			x1=x0;
			x0=(float)z;
			F2L(y1,&z);
			y1=y0;
			y0=(float)z;
		}

		if (x0<x1) 
		{
			m=dy/dx;
		
			F2L(x0,&i);

			while(i<x1) 
			{
				F2L(rnd()*FLARELINERND,&z);
				z+=FLARELINESTEP;
				i+=z;
				y0+=m*(float)z;
				AddLFlare((float)i,y0,io);				
			}
		}
		else 
		{
			m=dy/dx;
			F2L(x1,&i);

			while(i<x0) 
			{
				F2L(rnd()*FLARELINERND,&z);
				z+=FLARELINESTEP;
				i+=z;
				y0+=m*(float)z;
				AddLFlare((float)i,y0,io);				
			}
		}				
	}
	else 
	{
		if (y0>y1) 
		{
			F2L(x1,&z);
			x1=x0;
			x0=(float)z;			
			F2L(y1,&z);
			y1=y0;
			y0=(float)z;
		}

		if (y0<y1) 
		{
			m=dx/dy;
			F2L(y0,&i);

			while(i<y1) 
			{
				F2L(rnd()*FLARELINERND,&z);
			    z+=FLARELINESTEP;
				i+=z;
				x0+=m*(float)z;
				AddLFlare(x0,(float)i,io);				
			}			
		} 
		else 
		{
			m=dx/dy;
			F2L(y1,&i);

			while(i<y0) 
			{
				F2L(rnd()*FLARELINERND,&z);
				z+=FLARELINESTEP;
				i+=z;
				x0+=m*(float)z;
				AddLFlare(x0,(float)i,io);				
			} 
		}	
	}
}
