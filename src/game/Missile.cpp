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
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved

#include "game/Missile.h"

#include <stddef.h>

#include "core/GameTime.h"
#include "core/Core.h"

#include "game/Damage.h"
#include "game/Player.h"

#include "graphics/Color.h"
#include "graphics/GraphicsTypes.h"
#include "graphics/Math.h"
#include "graphics/data/Mesh.h"
#include "graphics/particle/ParticleEffects.h"

#include "math/Vector3.h"

#include "platform/Flags.h"

#include "scene/Light.h"
#include "scene/Interactive.h"
#include "scene/GameSound.h"

class TextureContainer;

struct ARX_MISSILE
{
	ARX_SPELLS_MISSILE_TYPE type;
	Vec3f startpos;
	Vec3f velocity;
	Vec3f lastpos;
	unsigned long timecreation;
	unsigned long lastupdate;
	unsigned long tolive;
	long		longinfo;
	long		owner;
};

const size_t MAX_MISSILES = 100;
ARX_MISSILE missiles[MAX_MISSILES];

// Gets a Free Projectile Slot
long ARX_MISSILES_GetFree() {
	
	for(size_t i = 0; i < MAX_MISSILES; i++) {
		if(missiles[i].type == MISSILE_NONE) {
			return i;
		}
	}
	
	return -1;
}

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
		case MISSILE_NONE: break;
	}

	missiles[i].type = MISSILE_NONE;
}

//-----------------------------------------------------------------------------
// Clear all missiles
void ARX_MISSILES_ClearAll() {
	for(size_t i = 0; i < MAX_MISSILES; i++) {
		ARX_MISSILES_Kill(i);
	}
}

//-----------------------------------------------------------------------------
// Spawns a Projectile using type, starting position/TargetPosition
void ARX_MISSILES_Spawn(INTERACTIVE_OBJ * io, ARX_SPELLS_MISSILE_TYPE type, const Vec3f * startpos, const Vec3f * targetpos) {
	
	long i(ARX_MISSILES_GetFree());

	if (i == -1) return;

	missiles[i].owner = GetInterNum(io);
	missiles[i].type = type;
	missiles[i].lastpos = missiles[i].startpos = *startpos;

	float dist;

	dist = 1.0F / fdist(*startpos, *targetpos);
	missiles[i].velocity = (*targetpos - *startpos) * dist;
	missiles[i].lastupdate = missiles[i].timecreation = (unsigned long)(arxtime);

	switch (type)
	{
		case MISSILE_NONE: break;
		case MISSILE_FIREBALL:
		{
			missiles[i].tolive = 6000;
			missiles[i].velocity *= 0.8f;
			missiles[i].longinfo = GetFreeDynLight();

			if (missiles[i].longinfo != -1)
			{
				DynLight[missiles[i].longinfo].intensity = 1.3f;
				DynLight[missiles[i].longinfo].exist = 1;
				DynLight[missiles[i].longinfo].fallend = 420.f;
				DynLight[missiles[i].longinfo].fallstart = 250.f;
				DynLight[missiles[i].longinfo].rgb = Color3f(1.f, .8f, .6f);
				DynLight[missiles[i].longinfo].pos = *startpos;
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
	long framediff, framediff3;
	Vec3f orgn, dest, hit;
	TextureContainer * tc = TC_fire; 
	EERIEPOLY *tp = NULL;
	unsigned long tim = (unsigned long)(arxtime);

	for (unsigned long i(0); i < MAX_MISSILES; i++) 
	{
		if (missiles[i].type == MISSILE_NONE) continue;

		framediff = missiles[i].timecreation + missiles[i].tolive - tim;

		if (framediff < 0)
		{
			ARX_MISSILES_Kill(i);
			continue;
		}

		framediff3 = tim - missiles[i].timecreation;

		switch (missiles[i].type)
		{
			case MISSILE_NONE: break;
			case MISSILE_FIREBALL: {
				Vec3f pos;

				pos = missiles[i].startpos + missiles[i].velocity * framediff3;

				if (missiles[i].longinfo != -1)
				{
					DynLight[missiles[i].longinfo].pos = pos;
				}

#ifdef BUILD_EDITOR
				if (USE_COLLISIONS)
#endif
				{
					orgn = missiles[i].lastpos;
					dest = pos;
					
					EERIEPOLY *ep;
					EERIEPOLY *epp;
					Vec3f tro;
					tro.x = 70.0F;
					tro.y = 70.0F;
					tro.z = 70.0F;

					CURRENTINTER = NULL;
					ep = GetMinPoly(dest.x, dest.y, dest.z);
					epp = GetMaxPoly(dest.x, dest.y, dest.z);

					if(closerThan(player.pos, pos, 200.f)) {
						ARX_MISSILES_Kill(i);
						ARX_BOOMS_Add(&pos);
						Add3DBoom(&pos);
						DoSphericDamage(&dest, 180.0F, 200.0F, DAMAGE_AREAHALF, DAMAGE_TYPE_FIRE | DAMAGE_TYPE_MAGICAL);
						break;
					}

					if (ep  && ep->center.y < dest.y)
					{
						ARX_MISSILES_Kill(i);
						ARX_BOOMS_Add(&dest);
						Add3DBoom(&dest);
						DoSphericDamage(&dest, 180.0F, 200.0F, DAMAGE_AREAHALF, DAMAGE_TYPE_FIRE | DAMAGE_TYPE_MAGICAL);
						break;
					}

					if (epp && epp->center.y > dest.y)
					{
						ARX_MISSILES_Kill(i);
						ARX_BOOMS_Add(&dest);
						Add3DBoom(&dest);
						DoSphericDamage(&dest, 180.0F, 200.0F, DAMAGE_AREAHALF, DAMAGE_TYPE_FIRE | DAMAGE_TYPE_MAGICAL);
						break;
					}

					if (EERIELaunchRay3(&orgn, &dest, &hit, tp, 1))
					{
						ARX_MISSILES_Kill(i);
						ARX_BOOMS_Add(&hit);
						Add3DBoom(&hit);
						DoSphericDamage(&dest, 180.0F, 200.0F, DAMAGE_AREAHALF, DAMAGE_TYPE_FIRE | DAMAGE_TYPE_MAGICAL);
						break;
					}

					if ( !EECheckInPoly(&dest) || EEIsUnderWater(&dest) )
					{
						ARX_MISSILES_Kill(i);
						ARX_BOOMS_Add(&dest);
						Add3DBoom(&dest);
						DoSphericDamage(&dest, 180.0F, 200.0F, DAMAGE_AREAHALF, DAMAGE_TYPE_FIRE | DAMAGE_TYPE_MAGICAL);
						break;
					}

					long ici = IsCollidingAnyInter(dest.x, dest.y, dest.z, &tro);

					if (ici != -1 && ici != missiles[i].owner)
					{
						ARX_MISSILES_Kill(i);
						ARX_BOOMS_Add(&dest);
						Add3DBoom(&dest);
						DoSphericDamage(&dest, 180.0F, 200.0F, DAMAGE_AREAHALF, DAMAGE_TYPE_FIRE | DAMAGE_TYPE_MAGICAL);
						break;
					}
				}

				long j = ARX_PARTICLES_GetFree();

				if (j != -1 && !arxtime.is_paused())
				{
					ParticleCount++;
					particle[j].exist = true;
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
					particle[j].siz = 12.0F * (float)(missiles[i].tolive - framediff3) * ( 1.0f / 4000 );
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
