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
// ARX_Special.CPP
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		ARX Special ...
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////
#include "ARX_Special.h"
#include "ARX_Interactive.h"
#include "EERIEMath.h"

#include <stdio.h>
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>


typedef struct
{
	long	ionum;  // -1 == not defined
	float	power;
	float	radius;
} ARX_SPECIAL_ATTRACTOR;
#define MAX_ATTRACTORS 16

ARX_SPECIAL_ATTRACTOR attractors[MAX_ATTRACTORS];

void ARX_SPECIAL_ATTRACTORS_Reset()
{
	for (long i = 0; i < MAX_ATTRACTORS; i++)
	{
		attractors[i].ionum = -1;
	}
}

void ARX_SPECIAL_ATTRACTORS_Remove(long ionum)
{
	for (long i = 0; i < MAX_ATTRACTORS; i++)
	{
		if (attractors[i].ionum == ionum)
			attractors[i].ionum = -1;
	}
}

long ARX_SPECIAL_ATTRACTORS_Exist(long ionum)
{
	for (long i = 0; i < MAX_ATTRACTORS; i++)
	{
		if (attractors[i].ionum == ionum)
			return i;
	}

	return -1;
}

bool ARX_SPECIAL_ATTRACTORS_Add(long ionum, float power, float radius)
{
	if (power == 0.f) ARX_SPECIAL_ATTRACTORS_Remove(ionum);

	long tst;

	if ((tst = ARX_SPECIAL_ATTRACTORS_Exist(ionum)) != -1)
	{
		attractors[tst].power = power;
		attractors[tst].radius = radius;
		return FALSE;
	}

	for (long i = 0; i < MAX_ATTRACTORS; i++)
	{
		if (attractors[i].ionum == -1)
		{
			attractors[i].ionum = ionum;
			attractors[i].power = power;
			attractors[i].radius = radius;
			return TRUE;
		}
	}

	return FALSE;
}

void ARX_SPECIAL_ATTRACTORS_ComputeForIO(INTERACTIVE_OBJ * ioo, EERIE_3D * force)
{
	force->x = 0;
	force->y = 0;
	force->z = 0;

	for (long i = 0; i < MAX_ATTRACTORS; i++)
	{
		if (attractors[i].ionum != -1)
		{
			if (ValidIONum(attractors[i].ionum))
			{
				INTERACTIVE_OBJ * io = inter.iobj[attractors[i].ionum];

				if ((io->show == SHOW_FLAG_IN_SCENE)
				        && !(io->ioflags & IO_NO_COLLISIONS)
				        && (io->GameFlags & GFLAG_ISINTREATZONE))
				{
					float power = attractors[i].power;
					EERIE_3D pos;
					pos.x = ioo->pos.x;
					pos.y = ioo->pos.y;
					pos.z = ioo->pos.z;
					float dist = EEDistance3D(&pos, &io->pos);

					if ((dist > ioo->physics.cyl.radius + io->physics.cyl.radius + 10.f)
					        || (power < 0.f))
					{
						float max_radius = attractors[i].radius; 

						if (dist < max_radius)
						{
							float ratio_dist = 1.f - (dist / max_radius);
							EERIE_3D vect;
							vect.x = io->pos.x - pos.x;
							vect.y = io->pos.y - pos.y;
							vect.z = io->pos.z - pos.z;
							Vector_Normalize(&vect);
							power *= ratio_dist * 0.01f;
							force->x = vect.x * power;
							force->y = vect.y * power;
							force->z = vect.z * power;
						}
					}
				}
			}
		}
	}
}

