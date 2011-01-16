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
///////////////////////////////////////////////////////////////////////////////
//
// ARX_Missile.cpp
// ARX Missile Management
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved
//
///////////////////////////////////////////////////////////////////////////////

#include <ARX_Missile.h>

#include <EERIELight.h>
#include <EERIEMath.h>
#include <EERIETexture.h>

#include <ARX_Damages.h>
#include <ARX_Interactive.h>
#include <ARX_Particles.h>
#include <ARX_Physics.h>
#include <ARX_Sound.h>
#include <ARX_Time.h>

#include <stdio.h>
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

//-----------------------------------------------------------------------------
typedef struct
{
	long	type;
	EERIE_3D startpos;
	EERIE_3D velocity;
	EERIE_3D lastpos;
	unsigned long timecreation;
	unsigned long lastupdate;
	unsigned long tolive;
	long		longinfo;
	long		owner;
} ARX_MISSILE;

//-----------------------------------------------------------------------------
const unsigned long MAX_MISSILES(100);
ARX_MISSILE missiles[MAX_MISSILES];

//-----------------------------------------------------------------------------
// Gets a Free Projectile Slot
long ARX_MISSILES_GetFree()
{
	for (long i(0); i < MAX_MISSILES; i++)
		if (missiles[i].type == MISSILE_NONE) return i;

	return -1;
}

//-----------------------------------------------------------------------------
// Kills a missile
void ARX_MISSILES_Kill(long i)
{
	switch (missiles[i].type)
	{
		case MISSILE_FIREBALL :

			if (missiles[i].longinfo != -1)
			{
				DynLight[missiles[i].longinfo].duration = 150;
			}

			break;
	}

	missiles[i].type = MISSILE_NONE;
}

//-----------------------------------------------------------------------------
// Clear all missiles
void ARX_MISSILES_ClearAll()
{
	for (long i(0); i < MAX_MISSILES; i++) ARX_MISSILES_Kill(i);
}

//-----------------------------------------------------------------------------
// Spawns a Projectile using type, starting position/TargetPosition
void ARX_MISSILES_Spawn(INTERACTIVE_OBJ *io, const long &type, const EERIE_3D *startpos, const EERIE_3D *targetpos)
{
	long i(ARX_MISSILES_GetFree());

	if (i == -1) return;

	missiles[i].owner = GetInterNum(io);
	missiles[i].type = type;
	missiles[i].lastpos.x = missiles[i].startpos.x = startpos->x;
	missiles[i].lastpos.y = missiles[i].startpos.y = startpos->y;
	missiles[i].lastpos.z = missiles[i].startpos.z = startpos->z;

	float dist;

	dist = 1.0F / Distance3D(startpos->x, startpos->y, startpos->z, targetpos->x, targetpos->y,targetpos->z);
	missiles[i].velocity.x = (targetpos->x - startpos->x) * dist;
	missiles[i].velocity.y = (targetpos->y - startpos->y) * dist;
	missiles[i].velocity.z = (targetpos->z - startpos->z) * dist;
	missiles[i].lastupdate = missiles[i].timecreation = ARXTimeUL();

	switch (type)
	{
		case MISSILE_FIREBALL:
		{
			missiles[i].tolive = 6000;
			missiles[i].velocity.x *= 0.8f;
			missiles[i].velocity.y *= 0.8f;
			missiles[i].velocity.z *= 0.8f;
			missiles[i].longinfo = GetFreeDynLight();

			if (missiles[i].longinfo != -1)
			{
				DynLight[missiles[i].longinfo].intensity = 1.3f;
				DynLight[missiles[i].longinfo].exist = 1;
				DynLight[missiles[i].longinfo].fallend = 420.f;
				DynLight[missiles[i].longinfo].fallstart = 250.f;
				DynLight[missiles[i].longinfo].rgb.r = 1.f;
				DynLight[missiles[i].longinfo].rgb.g = 0.8f;
				DynLight[missiles[i].longinfo].rgb.b = 0.6f;
				DynLight[missiles[i].longinfo].pos.x = startpos->x;
				DynLight[missiles[i].longinfo].pos.y = startpos->y;
				DynLight[missiles[i].longinfo].pos.z = startpos->z;
			}

			ARX_SOUND_PlaySFX(SND_SPELL_FIRE_WIND, &missiles[i].startpos, 2.0F);
			ARX_SOUND_PlaySFX(SND_SPELL_FIRE_LAUNCH, &missiles[i].startpos, 2.0F);
		}
	}
}

//-----------------------------------------------------------------------------
// Updates all currently launched projectiles
void ARX_MISSILES_Update()
{
	long framediff, framediff2, framediff3;
	EERIE_3D orgn, dest, hit;
	TextureContainer * tc = TC_fire; 
	EERIEPOLY *tp = NULL;
	unsigned long tim = ARXTimeUL();

	for (unsigned long i(0); i < MAX_MISSILES; i++) 
	{
		if (missiles[i].type == MISSILE_NONE) continue;

		framediff = missiles[i].timecreation + missiles[i].tolive - tim;

		if (framediff < 0)
		{
			ARX_MISSILES_Kill(i);
			continue;
		}

		framediff2 = missiles[i].timecreation + missiles[i].tolive - missiles[i].lastupdate;
		framediff3 = tim - missiles[i].timecreation;

		switch (missiles[i].type)
		{
			case MISSILE_FIREBALL :
			{
				EERIE_3D pos;

				pos.x = missiles[i].startpos.x + missiles[i].velocity.x * framediff3;
				pos.y = missiles[i].startpos.y + missiles[i].velocity.y * framediff3;
				pos.z = missiles[i].startpos.z + missiles[i].velocity.z * framediff3;

				if (missiles[i].longinfo != -1)
				{
					DynLight[missiles[i].longinfo].pos.x = pos.x;
					DynLight[missiles[i].longinfo].pos.y = pos.y;
					DynLight[missiles[i].longinfo].pos.z = pos.z;
				}

				if (USE_COLLISIONS) 
				{
					orgn.x = missiles[i].lastpos.x;
					orgn.y = missiles[i].lastpos.y;
					orgn.z = missiles[i].lastpos.z;					
					dest.x = pos.x;
					dest.y = pos.y;
					dest.z = pos.z;					
					
					EERIEPOLY *ep;
					EERIEPOLY *epp;
					EERIE_3D tro;
					tro.x = 70.0F;
					tro.y = 70.0F;
					tro.z = 70.0F;

					CURRENTINTER = NULL;
					ep = GetMinPoly(dest.x, dest.y, dest.z);
					epp = GetMaxPoly(dest.x, dest.y, dest.z);

					if (Distance3D(player.pos.x, player.pos.y, player.pos.z, pos.x, pos.y, pos.z) < 200.0F)
					{
						ARX_MISSILES_Kill(i);
						ARX_BOOMS_Add(&pos);
						Add3DBoom(&pos, NULL);
						DoSphericDamage(&dest, 180.0F, 200.0F, DAMAGE_AREAHALF, DAMAGE_TYPE_FIRE | DAMAGE_TYPE_MAGICAL);
						break;
					}

					if (ep  && ep->center.y < dest.y)
					{
						ARX_MISSILES_Kill(i);
						ARX_BOOMS_Add(&dest);
						Add3DBoom(&dest, NULL);
						DoSphericDamage(&dest, 180.0F, 200.0F, DAMAGE_AREAHALF, DAMAGE_TYPE_FIRE | DAMAGE_TYPE_MAGICAL);
						break;
					}

					if (epp && epp->center.y > dest.y)
					{
						ARX_MISSILES_Kill(i);
						ARX_BOOMS_Add(&dest);
						Add3DBoom(&dest, NULL);
						DoSphericDamage(&dest, 180.0F, 200.0F, DAMAGE_AREAHALF, DAMAGE_TYPE_FIRE | DAMAGE_TYPE_MAGICAL);
						break;
					}

					if (EERIELaunchRay3(&orgn, &dest, &hit, tp, 1))
					{
						ARX_MISSILES_Kill(i);
						ARX_BOOMS_Add(&hit);
						Add3DBoom(&hit, NULL);
						DoSphericDamage(&dest, 180.0F, 200.0F, DAMAGE_AREAHALF, DAMAGE_TYPE_FIRE | DAMAGE_TYPE_MAGICAL);
						break;
					}

					if ( !EECheckInPoly(&dest) || EEIsUnderWaterFast(&dest) ) //ARX: jycorbel (2010-08-20) - rendering issues with bGATI8500: optimize time to render;
					{
						ARX_MISSILES_Kill(i);
						ARX_BOOMS_Add(&dest);
						Add3DBoom(&dest, NULL);
						DoSphericDamage(&dest, 180.0F, 200.0F, DAMAGE_AREAHALF, DAMAGE_TYPE_FIRE | DAMAGE_TYPE_MAGICAL);
						break;
					}

					long ici = IsCollidingAnyInter(dest.x, dest.y, dest.z, &tro);

					if (ici != -1 && ici != missiles[i].owner)
					{
						ARX_MISSILES_Kill(i);
						ARX_BOOMS_Add(&dest);
						Add3DBoom(&dest, NULL);
						DoSphericDamage(&dest, 180.0F, 200.0F, DAMAGE_AREAHALF, DAMAGE_TYPE_FIRE | DAMAGE_TYPE_MAGICAL);
						break;
					}
				}

				long j = ARX_PARTICLES_GetFree();

				if (j != -1 && !ARXPausedTimer)
				{
					ParticleCount++;
					particle[j].exist = TRUE;
					particle[j].zdec = 0;
					particle[j].ov.x = pos.x;
					particle[j].ov.y = pos.y;
					particle[j].ov.z = pos.z;
					particle[j].move.x = missiles[i].velocity.x + 3.0f - 6.0F * rnd();
					particle[j].move.y = missiles[i].velocity.y + 4.0F - 12.0F * rnd();
					particle[j].move.z = missiles[i].velocity.z + 3.0F - 6.0F * rnd();
					particle[j].timcreation = tim;
					particle[j].tolive = 500 + (unsigned long)(rnd() * 500.f);
					particle[j].tc = tc;
					particle[j].siz = 12.0F * (float)(missiles[i].tolive - framediff3) * DIV4000;
					particle[j].scale.x = 15.0F + rnd() * 5.0F;
					particle[j].scale.y = 15.0F + rnd() * 5.0F;
					particle[j].scale.z = 15.0F + rnd() * 5.0F;
					particle[j].special = FIRE_TO_SMOKE;
				}

				missiles[i].lastpos.x = pos.x;
				missiles[i].lastpos.y = pos.y;
				missiles[i].lastpos.z = pos.z;

				break;
			}
		}

		missiles[i].lastupdate = tim;
	}
}
