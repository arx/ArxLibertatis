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
// Code: Cyril Meynier
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved

#include "graphics/particle/ParticleEffects.h"

#include <algorithm>

#include "core/Application.h"
#include "core/Config.h"
#include "core/Core.h"
#include "core/GameTime.h"

#include "game/Damage.h"
#include "game/Player.h"

#include "gui/Interface.h"

#include "graphics/Math.h"
#include "graphics/Draw.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/effects/SpellEffects.h"

#include "input/Input.h"

#include "physics/Collisions.h"

#include "scene/GameSound.h"
#include "scene/Interactive.h"
#include "scene/Light.h"

using std::max;

//TODO(lubosz): extern globals :(
extern float fZFogEnd;
extern Color ulBKGColor;

struct OBJFX {
	Vec3f pos;
	Vec3f move;
	Vec3f scale;
	Color3f fade;
	EERIE_3DOBJ * obj;
	long special;
	Anglef spe[8];
	Anglef speinc[8];
	unsigned long tim_start;
	unsigned long duration;
	bool exist;
	long dynlight;
};

FLARETC			flaretc;
PARTICLE_DEF	particle[MAX_PARTICLES];
BOOM			booms[MAX_BOOMS];
FLARES			flare[MAX_FLARES];
TexturedVertex		g_Lumignon[4];
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
void SpawnMetalShine(Vec3f * pos,long r,long g,long b,long num)
{
	return;

	if (num<0) return;

	long j=ARX_PARTICLES_GetFree();

	if ((j!=-1) && (!ARXPausedTimer))
	{
		ParticleCount++;
		PARTICLE_DEF * pd=&particle[j];
		pd->exist		=	true;
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
		pd->rgb = Color3f(r * (1.f/64), g * (1.f/64), b * (1.f/64));
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
		pd->exist=true;
		pd->zdec=0;
		float f=radians(player.angle.b);
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
void ARX_PARTICLES_Spawn_Lava_Burn(Vec3f * poss, INTERACTIVE_OBJ * io) {
	
	Vec3f pos;
	pos.x=poss->x;
	pos.y=poss->y;
	pos.z=poss->z;

	if (	(io)
		&&	(io->obj)
		&&	!io->obj->facelist.empty()	)
	{
		long notok	=	10;
		size_t num	=	0;

		while ( notok-- )
		{
			num = rnd() * io->obj->facelist.size();
			arx_assert(num < io->obj->facelist.size());

			if ( io->obj->facelist[num].facetype & POLY_HIDE ) continue;

			if ( EEfabs( pos.y-io->obj->vertexlist3[io->obj->facelist[num].vid[0]].v.y ) > 50.f )
				continue;

			notok = 0;
		}

		if ((notok >= 0) && 
			( notok < 10 ) )
		{
			Vec3f * v	=	&io->obj->vertexlist3[io->obj->facelist[num].vid[0]].v;
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
		pd->exist			=	true;
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

static void ARX_PARTICLES_Spawn_Rogue_Blood(const Vec3f & pos, float dmgs, Color col) {
	
	long j = ARX_PARTICLES_GetFree();

	if(j == -1 || ARXPausedTimer) {
		return;
	}
	
	float power = (dmgs * (1.f/60)) + .9f;
	
	ParticleCount++;
	PARTICLE_DEF * pd = &particle[j];
	pd->exist = true;
	pd->zdec = 0;
	pd->ov = pos;
	pd->siz = 3.1f * power;
	pd->scale.z = pd->scale.y = pd->scale.x = -pd->siz * (1.f/4);
	pd->timcreation = lARXTime;
	pd->special = PARTICLE_SUB2 | SUBSTRACT | GRAVITY | ROTATING | MODULATE_ROTATION | SPLAT_GROUND;
	pd->tolive = 1600;
	pd->delay = 0;
	pd->move = Vec3f(rnd() * 60.f - 30.f, rnd() * -10.f - 15.f, rnd() * 60.f - 30.f);
	pd->rgb = col.to<float>();
	long num = clamp((long)(rnd() * 6.f), 0l, 5l);
	pd->tc = bloodsplat[num];
	pd->fparam = rnd() * (1.f/10) - .05f;
	
}

static void ARX_PARTICLES_Spawn_Blood3(const Vec3f & pos, float dmgs, Color col, long flags = 0) {
	
	long j = ARX_PARTICLES_GetFree();
	
	if(j != -1 && !ARXPausedTimer) {
		
		float power = (dmgs * (1.f/60)) + .9f;
		ParticleCount++;
		PARTICLE_DEF * pd = &particle[j];
		pd->exist = true;
		pd->zdec = 0;
		pd->ov = pos + Vec3f(-sin(ARXTime * (1.f/1000)), sin(ARXTime * (1.f/1000)), cos(ARXTime * (1.f/1000))) * 30.f;
		pd->siz = 3.5f * power + sin(ARXTime * (1.f/1000));
		if(!config.misc.gore) {
			pd->siz *= (1.f/6);
		}
		pd->scale.z = pd->scale.y = pd->scale.x = -pd->siz * (1.f/2); 
		pd->timcreation	=	lARXTime;
		pd->special = PARTICLE_SUB2 | SUBSTRACT | GRAVITY | ROTATING | MODULATE_ROTATION | flags;
		pd->tolive = 1100;
		pd->delay=0;
		pd->move = Vec3f::ZERO;
		pd->rgb = col.to<float>();
		pd->tc = bloodsplatter;
		pd->fparam = rnd() * (1.f/10) - .05f;
	}
	
	if(rnd() > .90f) {
		ARX_PARTICLES_Spawn_Rogue_Blood(pos, dmgs, col);
	}
	
}
#define SPLAT_MULTIPLY 1.f

void ARX_POLYSPLAT_Add(Vec3f * poss, Color3f * col, float size, long flags) {
	
	if (BoomCount > (MAX_POLYBOOM >> 2) - 30) return;

	if ((BoomCount>250.f) 
		&& (size<10)) return;

	float splatsize=90;

	if (size>40.f) size=40.f;	

	size*=0.75f;

	if ((!config.misc.gore) && (!(flags & 2)))
		size*=( 1.0f / 5 );


	switch (config.video.levelOfDetail)
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
	TheoricalSplat.v[0].p.x=-splatsize*SPLAT_MULTIPLY;
	TheoricalSplat.v[0].p.y = py; 
	TheoricalSplat.v[0].p.z=-splatsize*SPLAT_MULTIPLY;

	TheoricalSplat.v[1].p.x=-splatsize*SPLAT_MULTIPLY;
	TheoricalSplat.v[1].p.y = py; 
	TheoricalSplat.v[1].p.z=+splatsize*SPLAT_MULTIPLY;

	TheoricalSplat.v[2].p.x=+splatsize*SPLAT_MULTIPLY;
	TheoricalSplat.v[2].p.y = py; 
	TheoricalSplat.v[2].p.z=+splatsize*SPLAT_MULTIPLY;

	TheoricalSplat.v[3].p.x=+splatsize*SPLAT_MULTIPLY;
	TheoricalSplat.v[3].p.y = py; 
	TheoricalSplat.v[3].p.z=-splatsize*SPLAT_MULTIPLY;
	TheoricalSplat.type=POLY_QUAD;

	Vec3f RealSplatStart;
	RealSplatStart.x=-size;
	RealSplatStart.y=py;
	RealSplatStart.z=-size;

	TheoricalSplat.v[0].p.x+=poss->x;
	TheoricalSplat.v[0].p.z+=poss->z;

	TheoricalSplat.v[1].p.x+=poss->x;
	TheoricalSplat.v[1].p.z+=poss->z;

	TheoricalSplat.v[2].p.x+=poss->x;
	TheoricalSplat.v[2].p.z+=poss->z;

	TheoricalSplat.v[3].p.x+=poss->x;
	TheoricalSplat.v[3].p.z+=poss->z;
	
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

	x0 = static_cast<long>(poss->x * ACTIVEBKG->Xmul);
	z0 = static_cast<long>(poss->z * ACTIVEBKG->Zmul);
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
	float vratio=size*( 1.0f / 40 );


	(void)checked_range_cast<short>(z0);
	(void)checked_range_cast<short>(x0);
	(void)checked_range_cast<short>(z1);
	(void)checked_range_cast<short>(x1);



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
				if ((PointIn2DPolyXZ(&TheoricalSplat, ep->v[k].p.x, ep->v[k].p.z))
					&& ((float)fabs(ep->v[k].p.y-py) < 100.f) )
				{
					 oki=1;
					break;
				}

				if ((PointIn2DPolyXZ(&TheoricalSplat, (ep->v[k].p.x+ep->center.x)*( 1.0f / 2 ), (ep->v[k].p.z+ep->center.z)*( 1.0f / 2 )))
					&& ((float)fabs(ep->v[k].p.y-py) < 100.f) )
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
					long num = static_cast<long>(rnd() * 6.f);

					if (num<0) num=0;
					else if (num>5) num=5;

					pb->tc=bloodsplat[num];

					float fRandom = rnd() * 2;
					
					int t = checked_range_cast<int>(fRandom);

					if (flags & 2)
						pb->tc = water_splat[t];

					pb->tolive=(long)(float)(16000*vratio);

					if (flags & 2)
						pb->tolive=1500;
					
					pb->timecreation=tim;

					pb->tx = static_cast<short>(i);
					pb->tz = static_cast<short>(j);

					pb->rgb = *col;

					for (int k=0;k<nbvert;k++) 
					{
						float vdiff=EEfabs(ep->v[k].p.y-RealSplatStart.y);
						pb->u[k]=(ep->v[k].p.x-RealSplatStart.x)*hdiv;

						if (pb->u[k]<0.5f)
							pb->u[k]-=vdiff*hdiv;
						else pb->u[k]+=vdiff*hdiv;

						pb->v[k]=(ep->v[k].p.z-RealSplatStart.z)*vdiv;

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

void SpawnGroundSplat(EERIE_SPHERE * sp, Color3f * rgb, float size, long flags) {
	ARX_POLYSPLAT_Add(&sp->origin, rgb, size, flags);
}


long TOTAL_BODY_CHUNKS_COUNT=0;

void ARX_PARTICLES_Spawn_Blood2(const Vec3f & pos, float dmgs, Color col, INTERACTIVE_OBJ * io) {
	
	bool isNpc = io && (io->ioflags & IO_NPC);
	
	if(isNpc && io->_npcdata->SPLAT_TOT_NB) {
		
		if(io->_npcdata->SPLAT_DAMAGES < 3) {
			return;
		}
		
		float power = (io->_npcdata->SPLAT_DAMAGES * (1.f/60)) + .9f;
		
		Vec3f vect = pos - io->_npcdata->last_splat_pos;
		float dist = vect.normalize();
		long nb = long(dist / 4.f * power);
		if(nb == 0) {
			nb = 1;
		}
		
		long MAX_GROUND_SPLATS;
		switch(config.video.levelOfDetail) {
			case 2:
				MAX_GROUND_SPLATS = 10;
			break;
			case 1:
				MAX_GROUND_SPLATS = 5;
			break;
			default:
				MAX_GROUND_SPLATS = 1;
			break;
		}
		
		for(long k = 0; k < nb; k++) {
			Vec3f posi = io->_npcdata->last_splat_pos + vect * k * 4.f * power;
			io->_npcdata->SPLAT_TOT_NB++;
			if(io->_npcdata->SPLAT_TOT_NB > MAX_GROUND_SPLATS) {
				ARX_PARTICLES_Spawn_Blood3(posi, io->_npcdata->SPLAT_DAMAGES, col, SPLAT_GROUND);
				io->_npcdata->SPLAT_TOT_NB=1;
			} else {
				ARX_PARTICLES_Spawn_Blood3(posi, io->_npcdata->SPLAT_DAMAGES, col);
			}
		}
		
	} else {
		if(isNpc) {
			io->_npcdata->SPLAT_DAMAGES = (short)dmgs;
		}
		ARX_PARTICLES_Spawn_Blood3(pos, dmgs, col, SPLAT_GROUND);
		if(isNpc) {
			io->_npcdata->SPLAT_TOT_NB = 1;
		}
	}
	
	if(isNpc) {
		io->_npcdata->last_splat_pos = pos;
	}
}

//-----------------------------------------------------------------------------
void ARX_PARTICLES_Spawn_Blood(Vec3f * pos,float dmgs,long source)
{
	if (source<0) return;

	float nearest_dist = std::numeric_limits<float>::max();
	long nearest=-1;
	long count=inter.iobj[source]->obj->nbgroups;

	for (long i=0;i<count;i+=2)
	{
		float dist = distSqr(*pos, inter.iobj[source]->obj->vertexlist3[inter.iobj[source]->obj->grouplist[i].origin].v);

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
			pd->exist=true;
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
			pd->rgb = Color3f(.9f, 0.f, 0.f);
			pd->tc			=	bloodsplatter;
			pd->fparam		=	rnd()*( 1.0f / 10 )-0.05f;
		}
	}
}
long SPARK_COUNT=0;
//-----------------------------------------------------------------------------
// flag & 1 punch failed
// flag & 2 punch success
//-----------------------------------------------------------------------------
void ARX_PARTICLES_Spawn_Spark(Vec3f * pos,float dmgs,long flags)
{
	if (!pos) return;

	long spawn_nb = dmgs;
	
	
	if (SPARK_COUNT<1000)
	{
		SPARK_COUNT+=spawn_nb*25;
	}
	else
	{
		SPARK_COUNT	-=	static_cast<long>(FrameDiff);
		return;
	}
	
	for (long k=0;k<spawn_nb;k++)
	{
		long j=ARX_PARTICLES_GetFree();

		if ((j!=-1) && (!ARXPausedTimer))
		{
			ParticleCount++;
			PARTICLE_DEF * pd=&particle[j];
			pd->exist=true;
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
			float len		=	(float)spawn_nb*( 1.0f / 3 );

			if ( len > 8 ) len	=	8;

			if ( len < 3 ) len	=	3;

			pd->tolive		=	(unsigned long)(float)(len * 90 + (float)spawn_nb);

			if(flags==0) {
				pd->rgb = Color3f(.3f, .3f, 0.f);
			} else if(flags & 1) {
				pd->rgb = Color3f(.2f, .2f, .1f);
			} else if(flags & 2) {
				pd->rgb = Color3f(.45f, .1f, 0.f);
			}

			pd->tc=NULL;
			pd->fparam = len + rnd() * len; // Spark Tail Length
		}
	}
}

//-----------------------------------------------------------------------------
void MakeCoolFx(Vec3f * pos)
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
		long num = rnd()*((io->obj->vertexlist.size()>>2)-1);
		num=(num<<2)+1;

			long j=ARX_PARTICLES_GetFree();

			if (	(j!=-1)
				&&	(!ARXPausedTimer)	)
			{
				ParticleCount++;
				PARTICLE_DEF * pd=&particle[j];
				pd->exist		=	true;
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
				pd->rgb = Color3f(.3f, .3f, .34f);
				pd->tc			=	smokeparticle;//tc2;
				pd->fparam		=	0.001f;//-rnd()*0.0002f;
			}
		}
	}

// flag 1 = randomize pos
void ARX_PARTICLES_Add_Smoke(Vec3f * pos, long flags, long amount, Color3f * rgb) {

	Vec3f mod;
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
			pd->exist=true;
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

			pd->rgb = (rgb) ? *rgb : Color3f(.3f, .3f, .34f);

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
		el->rgb = Color3f(Project.torch.r - rr * .1f, Project.torch.g - rr * .1f, Project.torch.b - rr * .1f);
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
		el->rgb = Color3f(1.f, .5f, .8f);
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
				el->rgb = Color3f(.01f * count, .009f * count, .008f * count);
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
void ARX_MAGICAL_FLARES_Draw(long FRAMETICKS)
{
	/////////FLARE
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);

	shinum++;

	if (shinum>=10) shinum=1;

	long TICKS	=	lARXTime - FRAMETICKS;

	if (TICKS<0) 
		return;

	float z,s,r,g,b;

	TextureContainer * surf;
	bool key=!GInput->actionPressed(CONTROLS_CUST_MAGICMODE);

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

					if(flare[i].type == 1) { 
						s=flare[i].size*2*z;
					} else if(flare[i].type == 4) {
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
						if(flare[i].type == 1 && z < 0.6f)  {
							z = 0.6f;
						}

						g = flare[i].rgb.g * z;
						r = flare[i].rgb.r * z;
						b = flare[i].rgb.b * z;
						
						flare[i].tv.color = Color3f(r, g, b).toBGR();
						flare[i].v.p.x=flare[i].tv.p.x;
						flare[i].v.p.y=flare[i].tv.p.y;
						flare[i].v.p.z=flare[i].tv.p.z;
								
						DynLight[0].rgb.r = std::max(DynLight[0].rgb.r, r);
						DynLight[0].rgb.g = std::max(DynLight[0].rgb.g, g);
						DynLight[0].rgb.b = std::max(DynLight[0].rgb.b, b);

						if (ValidDynLight(flare[i].dynlight)) 
						{
							EERIE_LIGHT * el=&DynLight[flare[i].dynlight];
							el->pos.x=flare[i].v.p.x;
							el->pos.y=flare[i].v.p.y;
							el->pos.z=flare[i].v.p.z;
							el->rgb = Color3f(r, g, b);
						}

						if (!flare[i].io) 
						{
							GRenderer->SetRenderState(Renderer::DepthTest, false);
						}
						else
						{
							GRenderer->SetRenderState(Renderer::DepthTest, true);
						}

						if(flare[i].bDrawBitmap)
						{
							s*=2.f;
							EERIEDrawBitmap(flare[i].v.p.x, flare[i].v.p.y, s, s, flare[i].v.p.z, surf, Color::fromBGRA(flare[i].tv.color));
						}
						else
						{
							EERIEDrawSprite(&flare[i].v,(float)s*0.025f+1.f, surf, Color::fromBGRA(flare[i].tv.color), 2.f);
						}
					}
				}
				
			}
					
		}
	}

	if (DynLight[0].rgb.r>1.f) DynLight[0].rgb.r=1.f;

	if (DynLight[0].rgb.g>1.f) DynLight[0].rgb.g=1.f;

	if (DynLight[0].rgb.b>1.f) DynLight[0].rgb.b=1.f;

	GRenderer->SetRenderState(Renderer::DepthWrite, true);
	GRenderer->SetRenderState(Renderer::DepthTest, true); 
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
void ARX_BOOMS_Add(Vec3f * poss,long type)
{
	static TextureContainer * tc1=TextureContainer::Load("graph/particles/fire_hit");
	static TextureContainer * tc2=TextureContainer::Load("graph/particles/boom");
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
		pd->exist=true;
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

		if(type == 1) {
			pd->rgb = Color3f(.4f, .4f, 1.f);
		}
	
		j=ARX_PARTICLES_GetFree();

		if (j!=-1)
		{
			ParticleCount++;
			PARTICLE_DEF * pd=&particle[j];
			pd->exist=true;
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

			if(type == 1) {
				pd->rgb = Color3f(.4f, .4f, 1.f);
			}
		}
	}

	typ=0;
	x0 = poss->x * ACTIVEBKG->Xmul;
	z0 = poss->z * ACTIVEBKG->Zmul;
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
	

	(void)checked_range_cast<short>(z0);
	(void)checked_range_cast<short>(x0);
	(void)checked_range_cast<short>(z1);
	(void)checked_range_cast<short>(x1);

	//We never add BOOMS particle with this flag to prevent any render issues. TO DO check for blending of DrawPrimitve and DrawPrimitiveVB
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

			if((ddd = fdist(ep->v[0].p, *poss)) < BOOM_RADIUS) {
				temp_u1[0]=(0.5f-((ddd/BOOM_RADIUS)*0.5f));
				temp_v1[0]=(0.5f-((ddd/BOOM_RADIUS)*0.5f));

				for(long k=1;k<nbvert;k++) {
					
					ddd = fdist(ep->v[k].p, *poss);

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

						pb->tx = static_cast<short>(i);
						pb->tz = static_cast<short>(j);


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

//-----------------------------------------------------------------------------
void Add3DBoom(Vec3f * position) {
	
	float dist = fdist(player.pos - Vec3f(0, 160.f, 0.f), *position);
	Vec3f poss = *position;
	ARX_SOUND_PlaySFX(SND_SPELL_FIRE_HIT, &poss);

	if (dist<300)
	{
		float onedist=1.f/dist;
		Vec3f vect;
		vect.x=(player.pos.x-position->x)*onedist; 
		vect.y=(player.pos.y-160.f-position->y)*onedist; 
		vect.z=(player.pos.z-position->z)*onedist;
		float power=(300.f-dist)*( 1.0f / 80 );
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
						float dist = fdist(inter.iobj[i]->obj->pbox->vert[k].pos, *position);

						if (dist<300.f)
						{
							inter.iobj[i]->obj->pbox->active=1;
							inter.iobj[i]->obj->pbox->stopcount=0;
							float onedist=1.f/dist;
							Vec3f vect;
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
void UpdateObjFx() {

	unsigned long framediff;
	float val,aa,bb;
	float t1,t2,t3;
	long p;
	Vec3f pos;

	TexturedVertex v[3];
	v[0] = TexturedVertex(Vec3f(0, 0, 0.001f), 1.f, Color::white.toBGR(), 1, Vec2f::ZERO);
	v[1] = TexturedVertex(Vec3f(0, 0, 0.001f), 1.f, Color::white.toBGR(), 1, Vec2f::X_AXIS);
	v[2] = TexturedVertex(Vec3f(0, 0, 0.001f), 1.f, Color::white.toBGR(), 1, Vec2f(1.f, 1.f));
	
	TexturedVertex v2[3];
	v[0] = TexturedVertex(Vec3f(0, 0, 0.001f), 1.f, Color::white.toBGR(), 1, Vec2f::ZERO);
	v[1] = TexturedVertex(Vec3f(0, 0, 0.001f), 1.f, Color::white.toBGR(), 1, Vec2f::X_AXIS);
	v[2] = TexturedVertex(Vec3f(0, 0, 0.001f), 1.f, Color::white.toBGR(), 1, Vec2f(1.f, 1.f));
	
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);

	for (long i=0;i<MAX_OBJFX;i++)
	{
		if (objfx[i].exist)
		{
			framediff = ARXTimeUL() - objfx[i].tim_start; 

			if (framediff>objfx[i].duration) 
			{
				objfx[i].exist=false;

				if (ValidDynLight(objfx[i].dynlight))
					DynLight[objfx[i].dynlight].exist=0;

				objfx[i].dynlight=-1;
				continue;
			}

			val=(float)framediff/(float)objfx[i].duration;
			pos.x=objfx[i].pos.x+objfx[i].move.x*val;
			pos.y=objfx[i].pos.y+objfx[i].move.y*val;
			pos.z=objfx[i].pos.z+objfx[i].move.z*val;

			Anglef angle;
			Vec3f scale;
			Color3f color;
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

			DrawEERIEObjEx(objfx[i].obj,
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

				GRenderer->SetCulling(Renderer::CullNone);

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
						v[p].p.x=pos.x;
						v[p].p.y=pos.y;
						v[p].p.z=pos.z;
					}
						
					t1=100.f*scale.x;
					t2=100.f*scale.y;
					t3=100.f*scale.z;
					v[1].p.x-=EEsin(radians(objfx[i].spe[k].b))*t1;
					v[1].p.y+=EEsin(radians(objfx[i].spe[k].a))*t2;
					v[1].p.z+=EEcos(radians(objfx[i].spe[k].b))*t3;
					v[2].p.x=v[0].p.x-EEsin(radians(MAKEANGLE(objfx[i].spe[k].b+bb)))*t1;
					v[2].p.y=v[0].p.y+EEsin(radians(MAKEANGLE(objfx[i].spe[k].a+aa)))*t2;
					v[2].p.z=v[0].p.z+EEcos(radians(MAKEANGLE(objfx[i].spe[k].b+bb)))*t3;
					EE_RTP(&v[0],&v2[0]);
					EE_RTP(&v[1],&v2[1]);
					EE_RTP(&v[2],&v2[2]);

					if(Project.improve) {
						for(p = 0; p < 3; p++) {
							v2[p].color = Color3f(color.r/(3.f + (float)p), 0.f, color.b/(5.f+(float)p)).toBGR();
						}
					} else {
						for(p = 0; p < 3; p++) {
							v2[p].color = Color3f(color.r/(3.f + (float)p), color.g/(4.f + (float)p), color.b/(5.f + (float)p)).toBGR();
						}
					}

					GRenderer->ResetTexture(0);
					EERIEDRAWPRIM(Renderer::TriangleFan, v2);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
void ARX_PARTICLES_FirstInit() {
	smokeparticle=TextureContainer::Load("graph/particles/smoke");
	
	// TODO bloodsplat and water_splat cannot use mipmapping because they need a constant color border pixel
	// this may also apply to other textures
	
	bloodsplat[0] = TextureContainer::Load("graph/particles/new_blood", TextureContainer::NoMipmap);
	bloodsplat[1] = TextureContainer::Load("graph/particles/new_blood_splat1", TextureContainer::NoMipmap);
	bloodsplat[2] = TextureContainer::Load("graph/particles/new_blood_splat2", TextureContainer::NoMipmap);
	bloodsplat[3] = TextureContainer::Load("graph/particles/new_blood_splat3", TextureContainer::NoMipmap);
	bloodsplat[4] = TextureContainer::Load("graph/particles/new_blood_splat4", TextureContainer::NoMipmap);
	bloodsplat[5] = TextureContainer::Load("graph/particles/new_blood_splat5", TextureContainer::NoMipmap);
	bloodsplatter = bloodsplat[0];
	
	water_splat[0] = TextureContainer::Load("graph/particles/[fx]_water01", TextureContainer::NoMipmap);
	water_splat[1] = TextureContainer::Load("graph/particles/[fx]_water02", TextureContainer::NoMipmap);
	water_splat[2] = TextureContainer::Load("graph/particles/[fx]_water03", TextureContainer::NoMipmap);
	
	water_drop[0]=TextureContainer::Load("graph/particles/[fx]_water_drop01");
	water_drop[1]=TextureContainer::Load("graph/particles/[fx]_water_drop02");
	water_drop[2]=TextureContainer::Load("graph/particles/[fx]_water_drop03");
	healing=TextureContainer::Load("graph/particles/heal_0005");
	tzupouf=TextureContainer::Load("graph/obj3d/textures/(fx)_tsu_greypouf");
	fire2=TextureContainer::Load("graph/particles/fire2");
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
	for ( size_t i = 0 ; i < MAX_PARTICLES ; i++)
		if ( !particle[i].exist )
		{
			PARTICLE_DEF *pd	= &particle[i];
			pd->type			= 0;
			pd->rgb = Color3f::white;
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
		pd->exist			=	true;
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
		pd->rgb = Color3f::magenta;
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
			pd->exist		=	true;
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
			pd->rgb = Color3f::magenta;
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
			pd->exist		=	true;
			pd->zdec		=	0;

			pd->ov.x		=	posx - i * 2; 
			pd->ov.y		=	posy - i * 2; 
			pd->ov.z		=	posz;
			pd->move.x		=	-(float)(i)*( 1.0f / 2 );
			pd->move.y		=	-(float)(i)*( 1.0f / 2 );
			pd->move.z		=	0.f;
			pd->scale.y		=	pd->scale.x			=	(float)(i*10);			
			pd->scale.z		=	0.f;
			pd->timcreation	=	lARXTime;
			pd->tolive		=	1200+(unsigned long)(rnd()*400.f);
			pd->tc			=	ITC.Get("book");
			pd->rgb = Color3f(1.f - i * .1f, i * .1f, .5f - i * .1f);
			pd->siz			=	32.f+i*4;
			pd->type		=	PARTICLE_2D;
		}
	}

	NewSpell=1;	
}

//-----------------------------------------------------------------------------
int ARX_GenereOneEtincelle(Vec3f *pos,Vec3f *dir)
{
	int	i;
	
	i=ARX_PARTICLES_GetFree();

	if(i<0) return -1;
	
	ParticleCount++;
	PARTICLE_DEF * pd=&particle[i];
	pd->exist		=	true;
	pd->type		=	PARTICLE_ETINCELLE;
	pd->special		=	GRAVITY;
	pd->ov			=	pd->oldpos	=	*pos;
	pd->move		=	*dir;
	pd->tolive		=	1000+(int)(rnd()*500.f);
	pd->timcreation	=	lARXTime;
	pd->rgb = Color3f(ET_R, ET_G, ET_B);
	pd->tc			=	GTC;
	pd->mask		=	ET_MASK;
	return i;
}

//-----------------------------------------------------------------------------
void ARX_GenereSpheriqueEtincelles(Vec3f *pos,float r,TextureContainer *tc,float rr,float g,float b,int mask)
{
	GTC=tc;
	ET_R=rr;
	ET_G=g;
	ET_B=b;
	ET_MASK=mask;
	int nb=rand()&0x1F;

	while(nb)
	{
		Vec3f dir;

		float a = radians(rnd()*360.f);
		float b = radians(rnd()*360.f);
		dir.x=(float) r*EEsin(a)*EEcos(b);
		dir.z=(float) r*EEsin(a)*EEsin(b);
		dir.y=(float) r*EEcos(a);

		ARX_GenereOneEtincelle(pos,&dir);
		nb--;
	}
}

void ARX_PARTICLES_Spawn_Splat(const Vec3f & pos, float dmgs, Color col) {
	
	float power = (dmgs * (1.f/60)) + .9f;
	
	for(long kk = 0; kk < 20; kk++) {
		
		long j = ARX_PARTICLES_GetFree();
		if(j != -1) {
			ParticleCount++;
			PARTICLE_DEF * pd=&particle[j];
			pd->special = PARTICLE_SUB2 | SUBSTRACT | GRAVITY;
			pd->exist = true;
			pd->ov = pos;
			pd->move = Vec3f((rnd() * 10 - 5.f) * 2.3f, (rnd() * 10 - 5.f) * 2.3f, (rnd() * 10 - 5.f) * 2.3f);
			pd->timcreation = lARXTime;
			pd->tolive = (unsigned long)(1000 + dmgs*3);
			pd->tc = blood_splat;
			pd->siz = .3f + .01f*power;
			pd->scale = Vec3f(.2f + .3f*power, .2f + .3f*power, .2f + .3f*power);
			pd->zdec = 1;
			pd->rgb = col.to<float>();
		}
	}
}

void ARX_PARTICLES_SpawnWaterSplash(const Vec3f * _ePos)
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

			pd->exist = true;
			pd->ov.x = _ePos->x + rnd()*30;
			pd->ov.y = _ePos->y - rnd()*20;
			pd->ov.z = _ePos->z + rnd()*30;
			pd->move.x = (frand2()*5)*1.3f;
			pd->move.y = -(rnd()*5)*2.3f;
			pd->move.z = (frand2()*5)*1.3f;
			pd->timcreation = lARXTime;
			pd->tolive = (unsigned long)(1000+rnd()*300);
			
			float fRandom = rnd() * 2;
			
			int t = checked_range_cast<int>(fRandom);
			
			pd->tc=water_drop[t];
			pd->siz = 0.4f;
			float s = rnd();
			pd->scale.x = 1;
			pd->scale.y = 1;
			pd->scale.z = 1;
			pd->zdec = 1;
			pd->rgb = Color3f::gray(s);
		}
	}
}

//-----------------------------------------------------------------------------
void SpawnFireballTail(Vec3f * poss,Vec3f * vecto,float level,long flags)
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
			pd->exist		=	true;
			pd->ov.x		=	poss->x;
			pd->ov.y		=	poss->y;
			pd->ov.z		=	poss->z;
			pd->move.x		=	0;
			pd->move.y		=	-rnd() * 3.f; 
			pd->move.z		=	0;
			pd->timcreation	=	lARXTime;
			pd->tc			=	explo[0];
			pd->rgb = Color3f::gray(.7f);
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

void LaunchFireballBoom(Vec3f * poss, float level, Vec3f * direction, Color3f * rgb) {
	
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
			pd->exist=true;
			pd->ov = *poss;

			if (direction)
				pd->move = *direction;
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

			if(rgb) {
				pd->rgb = *rgb;
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
void ARX_PARTICLES_Render(EERIE_CAMERA * cam) 
{
	if (!ACTIVEBKG) return;

	TreatBackgroundActions();

	if 	(ParticleCount==0) return;

	long xx,yy;
	unsigned  long tim;
	long framediff;
	long framediff2;
	long t;
	TexturedVertex in,inn,out;
	Color color;
	float siz,siz2,r;
	float val;
	float fd;
	float rott;
	
	tim = ARXTimeUL();//treat warning C4244 conversion from 'float' to 'unsigned long'	
	
	GRenderer->SetCulling(Renderer::CullNone);

	GRenderer->SetFogColor(Color::none);

	TextureContainer * tc=NULL;
	long pcc=ParticleCount;

	for(size_t i = 0; i < MAX_PARTICLES; i++) {
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
					Vec3f vector;
					vector.x=part->ov.x-target->pos.x;
					vector.y=(part->ov.y-target->pos.y)*( 1.0f / 2 );
					vector.z=part->ov.z-target->pos.z;
					vector.normalize();

					part->move.x = vector.x * 18 + rnd() - 0.5f; 
					part->move.y = vector.y * 5 + rnd() - 0.5f; 
					part->move.z = vector.z * 18 + rnd() - 0.5f; 
					
				}				

				continue;
			}

			if (!(part->type & PARTICLE_2D))
			{
				xx = part->ov.x * ACTIVEBKG->Xmul;
				yy = part->ov.z * ACTIVEBKG->Zmul;

				if ((xx<0) || (yy<0) || (xx>ACTIVEBKG->Xsize) || (yy>ACTIVEBKG->Zsize))
				{
					part->exist=false;
					ParticleCount--;
					continue;
				}

				FAST_BKG_DATA * feg=(FAST_BKG_DATA *)&ACTIVEBKG->fastdata[xx][yy];

				if (!feg->treat)
				{
					part->exist=false;
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

					part->rgb = Color3f::gray(.45f);
					part->move.x*=( 1.0f / 2 );
					part->move.y*=( 1.0f / 2 );
					part->move.z*=( 1.0f / 2 );
					part->siz*=( 1.0f / 3 );			
					part->special&=~FIRE_TO_SMOKE;
					part->timcreation=tim;
					part->tc = smokeparticle; 
					
					framediff=part->timcreation+part->tolive-tim;
				}
				else 
				{
					part->exist=false;
					ParticleCount--;
					continue;
				}
			}
			
			
			if((part->special & FIRE_TO_SMOKE2)
			   && framediff2 > long(part->tolive - (part->tolive >> 2))) {
				
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

					pd->rgb = Color3f::white; 
					pd->move *= (1.f/2);
					pd->siz *= (1.f/3);
				}
			}
			
			val=(part->tolive-framediff)*( 1.0f / 100 );
			
			if ((part->special & FOLLOW_SOURCE) && (part->sourceionum>=0) && (inter.iobj[part->sourceionum]))
			{
				inn.p.x=in.p.x=part->source->x;
				inn.p.y=in.p.y=part->source->y;
				inn.p.z=in.p.z=part->source->z;
			}
			else if ((part->special & FOLLOW_SOURCE2) && (part->sourceionum>=0) && (inter.iobj[part->sourceionum]))
			{
				inn.p.x=in.p.x=part->source->x+part->move.x*val;
				inn.p.y=in.p.y=part->source->y+part->move.y*val;
				inn.p.z=in.p.z=part->source->z+part->move.z*val;
			}
			else
			{
				inn.p.x=in.p.x=part->ov.x+part->move.x*val;
				inn.p.y=in.p.y=part->ov.y+part->move.y*val;
				inn.p.z=in.p.z=part->ov.z+part->move.z*val;
			}

			if (part->special & GRAVITY)
			{
				inn.p.y += 0.98f * 1.5f * val * val;
				in.p.y=inn.p.y;
			}
			
			if (part->special & PARTICLE_NOZBUFFER) 
			{
				GRenderer->SetRenderState(Renderer::DepthTest, false);
			}
			else
			{
				GRenderer->SetRenderState(Renderer::DepthTest, true);
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
				sp.origin.x=in.p.x;
				sp.origin.y=in.p.y;
				sp.origin.z=in.p.z;
				EERIETreatPoint(&inn,&out);			

				if (out.rhw<0) continue;

				if (out.p.z>cam->cdepth*fZFogEnd) continue;

				if (part->special & PARTICLE_SPARK)
				{
					if (part->special & NO_TRANS)
					{
						GRenderer->SetRenderState(Renderer::AlphaBlending, false);
					}
					else
					{
						GRenderer->SetRenderState(Renderer::AlphaBlending, true);

						if (part->special & SUBSTRACT) 
						{
							GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);
						}
						else
						{
							GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
						}
					}

					GRenderer->SetCulling(Renderer::CullNone);
					Vec3f vect = part->oldpos - in.p;
					fnormalize(vect);
					TexturedVertex tv[3];
					tv[0].color = part->rgb.toBGR();
					tv[1].color=0xFF666666;
					tv[2].color = 0xFF000000; 
					tv[0].p.x=out.p.x;
					tv[0].p.y=out.p.y;
					tv[0].p.z=out.p.z;
					tv[0].rhw=out.rhw;
					TexturedVertex temp;
					temp.p.x=in.p.x+rnd()*0.5f;
					temp.p.y=in.p.y+0.8f;
					temp.p.z=in.p.z+rnd()*0.5f;
					EERIETreatPoint(&temp,&tv[1]);
					temp.p.x=in.p.x+vect.x*part->fparam;
					temp.p.y=in.p.y+vect.y*part->fparam;
					temp.p.z=in.p.z+vect.z*part->fparam;

					EERIETreatPoint(&temp,&tv[2]);
					GRenderer->ResetTexture(0);

					EERIEDRAWPRIM(Renderer::TriangleStrip, tv);
					if(!ARXPausedTimer)
					{
						part->oldpos.x=in.p.x;
						part->oldpos.y=in.p.y;
						part->oldpos.z=in.p.z;
					}

					continue;
				}

				if (part->special & SPLAT_GROUND)
				{
					siz=part->siz+part->scale.x*fd;			
					sp.radius=siz*10;

					if(CheckAnythingInSphere(&sp,0,CAS_NO_NPC_COL)) {
						
						Color3f rgb = part->rgb;

						if (rnd()<0.9f)
							SpawnGroundSplat(&sp,&rgb,sp.radius,0);

						part->exist=false;
						ParticleCount--;
						continue;
					}
				}

				if (part->special & SPLAT_WATER)
				{
					siz=part->siz+part->scale.x*fd;
					sp.radius=siz*(10 + rnd()*20);

					if (CheckAnythingInSphere(&sp,0,CAS_NO_NPC_COL))
					{
						Color3f rgb(part->rgb.r * 0.5f, part->rgb.g * 0.5f, part->rgb.b * 0.5f);

						if (rnd()<0.9f)
							SpawnGroundSplat(&sp,&rgb,sp.radius,2);

						part->exist=false;
						ParticleCount--;
						continue;
					}
				}
				
			}

			if ((part->special & DISSIPATING) && (out.p.z<0.05f))
			{
				out.p.z=(out.p.z)*20.f;
				r*=out.p.z;
			}

			if (r>0.f) 
			{
				if (part->special & NO_TRANS)
				{
					GRenderer->SetRenderState(Renderer::AlphaBlending, false);
				}
				else
				{
					GRenderer->SetRenderState(Renderer::AlphaBlending, true);

					if (part->special & SUBSTRACT) 
					{
						GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);
					}
					else
					{
						GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
					}
				}
				
				Vec3f op=part->oldpos;

				if(!ARXPausedTimer)
				{
					part->oldpos.x=in.p.x;
					part->oldpos.y=in.p.y;
					part->oldpos.z=in.p.z; 
				}

				if (part->special & PARTICLE_GOLDRAIN)
				{
					float v=(rnd()-0.5f)*( 1.0f / 5 );

					if((part->rgb.r+v<=1.f) && (part->rgb.r+v>0.f)
						&& (part->rgb.g+v<=1.f) && (part->rgb.g+v>0.f)
						&& (part->rgb.b+v<=1.f) && (part->rgb.b+v>0.f) ) {
						part->rgb.r += v;
						part->rgb.g += v;
						part->rgb.b += v;
					}
				}

				if(Project.improve) {
					color = Color3f(part->rgb.r * r, 0.f, part->rgb.b * r).to<u8>();
				} else {
					color = Color3f(part->rgb.r * r, part->rgb.g * r, part->rgb.b * r).to<u8>();
				}

				tc=part->tc;

				if ((tc==explo[0]) && (part->special & PARTICLE_ANIMATED))
				{
					long animrange=part->cval2-part->cval1;
					float tt=(float)framediff2/(float)part->tolive*animrange;
					long num = tt;

					if (num<part->cval1) num=part->cval1;

					if (num>part->cval2) num=part->cval2;

					tc=explo[num];
				}

				siz=part->siz+part->scale.x*fd;			

				if (part->special & ROTATING) 
				{
					if (part->special & MODULATE_ROTATION) rott=MAKEANGLE((float)(tim+framediff2)*part->fparam);
					else rott=(MAKEANGLE((float)(tim+framediff2*2)*( 1.0f / 4 )));

					if (part->type & PARTICLE_2D) 
					{}
					else 
					{
						float temp;

						if (part->zdec) temp=0.0001f;
						else temp=2.f;
						
						if (part->special & PARTICLE_SUB2) 
						{
							TexturedVertex in2;
							memcpy(&in2,&in,sizeof(TexturedVertex));
							GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);						
							EERIEDrawRotatedSprite(&in,siz,tc,color,temp,rott);
							GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);						
							EERIEDrawRotatedSprite(&in2,siz,tc,Color::white,temp,rott);
						}
						else
							EERIEDrawRotatedSprite(&in,siz,tc,color,temp,rott);
					}					
				}
				else if (part->type & PARTICLE_2D) 
				{
					siz2=part->siz+part->scale.y*fd;
					
					if (part->special & PARTICLE_SUB2) 
					{
						TexturedVertex in2;
						memcpy(&in2,&in,sizeof(TexturedVertex));
						GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);						
						EERIEDrawBitmap(in.p.x, in.p.y, siz, siz2, in.p.z, tc, color);
						GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);						
						EERIEDrawBitmap(in2.p.x, in.p.y, siz, siz2, in.p.z, tc, Color::white);
					}
					else
						EERIEDrawBitmap(in.p.x, in.p.y, siz, siz2, in.p.z, tc,color);
				}
				else 
				{
					if(part->type & PARTICLE_ETINCELLE)
					{
						Vec3f pos,end;
						pos.x=in.p.x;
						pos.y=in.p.y;
						pos.z=in.p.z;
						Color col = Color3f(part->rgb.r * r, part->rgb.g * r, part->rgb.b * r).to<u8>();
						end.x=pos.x-(pos.x-op.x)*2.5f;
						end.y=pos.y-(pos.y-op.y)*2.5f;
						end.z=pos.z-(pos.z-op.z)*2.5f;
						Draw3DLineTex2(end,pos,2.f, Color::fromBGRA(col.toBGRA() & part->mask), col);
						
						EERIEDrawSprite(&in, .7f, tc, col, 2.f);
						
					}
					else
					{
						float temp;

						if (part->zdec) temp=0.0001f;
						else temp=2.f;
						
						if (part->special & PARTICLE_SUB2) 
						{
							TexturedVertex in2;
							memcpy(&in2,&in,sizeof(TexturedVertex));
							GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
							EERIEDrawSprite(&in,siz,tc,color,temp);				
							GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);
							EERIEDrawSprite(&in2,siz,tc,Color::white,temp);				
						}
						else 
							EERIEDrawSprite(&in,siz,tc,color,temp);				
					}
				}
			}

			pcc--;

			if (pcc<=0)
			{
				GRenderer->SetFogColor(ulBKGColor);
				return;
			}
		}	
	}

	GRenderer->SetFogColor(ulBKGColor);
	GRenderer->SetRenderState(Renderer::DepthTest, true);
}

void RestoreAllLightsInitialStatus() {
	for(size_t i = 0; i < MAX_LIGHTS; i++) {
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

	for(size_t i = 0; i < MAX_LIGHTS; i++) {
		EERIE_LIGHT * gl=GLight[i];

		if (gl==NULL) continue;
		
		float dist = distSqr(gl->pos,	ACTIVECAM->pos);

		if(dist > square(fZFar)) // Out of Treat Range
		{
			ARX_SOUND_Stop(gl->sample);
			gl->sample = audio::INVALID_ID;
			continue;
		}

		
		if ((gl->extras & EXTRAS_SPAWNFIRE) &&	(gl->status))
			{
				long id=ARX_DAMAGES_GetFree();

				if (id!=-1)
				{
				damages[id].radius = gl->ex_radius; 
				damages[id].damages = gl->ex_radius * ( 1.0f / 7 ); 
					damages[id].area=DAMAGE_FULL;
					damages[id].duration=1;
					damages[id].source=-5;
				damages[id].flags = 0; 
					damages[id].type=DAMAGE_TYPE_MAGICAL | DAMAGE_TYPE_FIRE | DAMAGE_TYPE_NO_FIX;
					damages[id].exist=true;
					damages[id].pos.x=gl->pos.x;
					damages[id].pos.y=gl->pos.y;
					damages[id].pos.z=gl->pos.z;
				}
			}

		if ((  (gl->extras & EXTRAS_SPAWNFIRE) 
			|| (gl->extras & EXTRAS_SPAWNSMOKE))
			&& (gl->status))
		{
			if (gl->sample == audio::INVALID_ID)
			{
				gl->sample = SND_FIREPLACE;
					ARX_SOUND_PlaySFX(gl->sample, &gl->pos, 0.95F + 0.1F * rnd(), ARX_SOUND_PLAY_LOOPED);
			}
			else
			{
				ARX_SOUND_RefreshPosition(gl->sample, &gl->pos);
			}


			long count;

			if(dist < square(ACTIVECAM->cdepth) * square(1.0f / 8)) count=4;
			else if(dist < square(ACTIVECAM->cdepth) * square(1.0f / 6)) count=4;
			else if(dist > square(ACTIVECAM->cdepth) * square(1.0f / 3)) count=3;
			else count=2;

			for (n=0;n<count;n++)
			{
				j=ARX_PARTICLES_GetFree();

				if ((j!=-1) && (!ARXPausedTimer) && ((rnd()<gl->ex_frequency)))
				{
					ParticleCount++;
					PARTICLE_DEF * pd=&particle[j];
					pd->exist=true;
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
					
					pd->rgb = (gl->extras & EXTRAS_COLORLEGACY) ? gl->rgb : Color3f::white;
				}

				if ((gl->extras & EXTRAS_SPAWNFIRE) && (rnd()>0.95f))
				{
					j=ARX_PARTICLES_GetFree();

					if ((j!=-1) && (!ARXPausedTimer) && ((rnd()<gl->ex_frequency)))
					{
						ParticleCount++;
						PARTICLE_DEF * pd=&particle[j];
						pd->exist=true;
						pd->zdec=0;
						sy = rnd() * 3.14159f * 2.f - 3.14159f; 
						sx=EEsin(sy);
						sz=EEcos(sy);
						sy=EEsin(sy);
						pd->ov.x=gl->pos.x+gl->ex_radius*sx*rnd();
						pd->ov.y=gl->pos.y+gl->ex_radius*sy*rnd();
						pd->ov.z=gl->pos.z+gl->ex_radius*sz*rnd();
						Vec3f vect = (pd->ov - gl->pos).getNormalized();

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
						
						pd->rgb = (gl->extras & EXTRAS_COLORLEGACY) ? gl->rgb : Color3f::white;
					}
				}
			}
			
		}
		else
		{
			if ((!gl->status) && (gl->sample != audio::INVALID_ID))
			{
				ARX_SOUND_Stop(gl->sample);
				gl->sample = audio::INVALID_ID;
			}
		}
	}	
}

//-----------------------------------------------------------------------------
void ARX_MAGICAL_FLARES_FirstInit()
{	
	flarenum=0;

	for(long i=0;i<MAX_FLARES;i++) flare[i].exist=0; 

	g_Lumignon[0] = TexturedVertex(Vec3f(0, 0, .5f), .5f, 0xFFFFFFFF, 0, Vec2f::Y_AXIS);
	g_Lumignon[1] = TexturedVertex(Vec3f(0, 0, .5f), .5f, 0xFFFFFFFF, 0, Vec2f::ZERO);
	g_Lumignon[2] = TexturedVertex(Vec3f(0, 0, .5f), .5f, 0xFFFFFFFF, 0, Vec2f(1, 1));
	g_Lumignon[3] = TexturedVertex(Vec3f(0, 0, .5f), .5f, 0xFFFFFFFF, 0, Vec2f::X_AXIS);
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
void AddFlare(Vec2s * pos,float sm,short typ,INTERACTIVE_OBJ * io)
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
			fl->v.p.x+=ka.pos.x;
			fl->v.p.y+=ka.pos.y;
			fl->v.p.z+=ka.pos.z;
			EE_RTT(&fl->tv,&fl->v);
			fl->v.p.x+=ka.pos.x;
			fl->v.p.y+=ka.pos.y;
			fl->v.p.z+=ka.pos.z;
			
			vx=-(fl->x-subj.centerx);
			vy=(fl->y-subj.centery);
			vx*=0.2173913f;///=4.6f;
			vy*=0.1515151515151515f;///=6.6f;
			
			if (io)
			{
				fl->v.p.x=io->pos.x-(float)EEsin(radians(MAKEANGLE(io->angle.b+vx)))*100.f;
				fl->v.p.y=io->pos.y+(float)EEsin(radians(MAKEANGLE(io->angle.a+vy)))*100.f-150.f;
				fl->v.p.z=io->pos.z+(float)EEcos(radians(MAKEANGLE(io->angle.b+vx)))*100.f;
				
			}
			else
			{
				fl->v.p.z=75.f;
				fl->v.p.x=((float)(pos->x-(DANAESIZX>>1)))*fl->v.p.z*2.f/(float)DANAESIZX;
				fl->v.p.y=((float)(pos->y-(DANAESIZY>>1)))*fl->v.p.z*2.f/((float)DANAESIZY*((float)DANAESIZX/(float)DANAESIZY));

				ka=*oldcam;
				SetActiveCamera(&ka);
				PrepareCamera(&ka);

				float temp=(fl->v.p.y*-ka.Xsin) + (fl->v.p.z*ka.Xcos);
				fl->v.p.y = (fl->v.p.y*ka.Xcos) - (-fl->v.p.z*ka.Xsin);
				fl->v.p.z =(temp*ka.Ycos) - (-fl->v.p.x*ka.Ysin);
				fl->v.p.x = (temp*-ka.Ysin) + (fl->v.p.x*ka.Ycos);	

				fl->v.p.x+=oldcam->pos.x;
				fl->v.p.y+=oldcam->pos.y;
				fl->v.p.z+=oldcam->pos.z;
			}
			
			fl->tv.p.x=fl->v.p.x;
			fl->tv.p.y=fl->v.p.y;
			fl->tv.p.z=fl->v.p.z;
			SetActiveCamera(oldcam);
			
			switch(PIPOrgb) {
			case 0:
				fl->rgb = Color3f(rnd() * (2.f/3) + .4f, rnd() * (2.f/3), rnd() * (2.f/3) + .4f);
				break;
			case 1:
				fl->rgb = Color3f(rnd() * .625f + .5f, rnd() * .625f + .5f, rnd() * .55f);
				break;
			case 2:
				fl->rgb = Color3f(rnd() * (2.f/3) + .4f, rnd() * .55f, rnd() * .55f);
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

						pd->exist		=	true;
						pd->zdec		=	0;
						pd->ov.x		=	fl->v.p.x+rnd()*10.f-5.f;
						pd->ov.y		=	fl->v.p.y+rnd()*10.f-5.f;
						pd->ov.z		=	fl->v.p.z+rnd()*10.f-5.f;
						
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
							pd->siz = 1.f + (float)(kk) * ( 1.0f / 2 ); 
						}
						else pd->siz=1.f+rnd()*1.f;

						pd->rgb = Color3f(fl->rgb.r * (2.f/3), fl->rgb.g * (2.f/3), fl->rgb.b * (2.f/3));
						pd->fparam=1.2f;
					}
				}
			}
			return;					
		}
	}
}

//-----------------------------------------------------------------------------
void AddFlare2(Vec2s * pos,float sm,short typ,INTERACTIVE_OBJ * io)
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
			
			fl->tv.p.x=fl->x;
			fl->tv.p.y=fl->y;
			fl->tv.p.z = 0.001f; 

			switch (PIPOrgb)  {
			case 0:
				fl->rgb = Color3f(rnd() * (2.f/3) + .4f, rnd() * (2.f/3), rnd() * (2.f/3) + .4f);
				break;
			case 1:
				fl->rgb = Color3f(rnd() * .625f + .5f, rnd() * .625f + .5f, rnd() * .55f);
				break;
			case 2:
				fl->rgb = Color3f(rnd() * (2.f/3) + .4f, rnd() * .55f, rnd() * .55f);
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
						pd->exist		=	true;
						pd->zdec		=	0;
						pd->ov.x		=	fl->v.p.x+rnd()*10.f-5.f;
						pd->ov.y		=	fl->v.p.y+rnd()*10.f-5.f;
						pd->ov.z		=	fl->v.p.z+rnd()*10.f-5.f;
						
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
							pd->siz = 1.f + (float)(kk) * ( 1.0f / 2 ); 
						}
						else pd->siz=1.f+rnd()*1.f;

						pd->rgb = Color3f(fl->rgb.r * (2.f/3), fl->rgb.g * (2.f/3), fl->rgb.b * (2.f/3));
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
	Vec2s pos;
	pos.x=(short)x;
	pos.y=(short)y;
	AddFlare(&pos,0.45f,1,io);	
}

//-----------------------------------------------------------------------------
void FlareLine(Vec2s * pos0, Vec2s * pos1, INTERACTIVE_OBJ * io) 
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
			z = x1;
			x1 = x0;
			x0 = z;
			z = y1;
			y0 = z;
		}

		if (x0<x1) 
		{
			m=dy/dx;
		
			i = x0;

			while(i<x1) 
			{
				z = rnd()*FLARELINERND;
				z+=FLARELINESTEP;
				i+=z;
				y0+=m*z;
				AddLFlare((float)i,y0,io);				
			}
		}
		else 
		{
			m = dy / dx;
			i = x1;

			while(i<x0) 
			{
				z = rnd()*FLARELINERND;
				z+=FLARELINESTEP;
				i+=z;
				y0+=m*z;
				AddLFlare((float)i,y0,io);				
			}
		}				
	}
	else 
	{
		if (y0>y1) 
		{
			z = x1;
			x0=z;
			z = y1;
			y1=y0;
			y0=z;
		}

		if (y0<y1) 
		{
			m = dx/dy;
			i = y0;

			while(i<y1) 
			{
				z = rnd()*FLARELINERND;
			    z+=FLARELINESTEP;
				i+=z;
				x0+=m*z;
				AddLFlare(x0,(float)i,io);				
			}			
		} 
		else 
		{
			m=dx/dy;
			i = y1;

			while(i<y0) 
			{
				z = rnd()*FLARELINERND;
				z+=FLARELINESTEP;
				i+=z;
				x0+=m*z;
				AddLFlare(x0,(float)i,io);				
			} 
		}	
	}
}
