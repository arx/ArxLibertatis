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

#include "physics/Physics.h"

#include <stddef.h>

#include "graphics/GraphicsTypes.h"
#include "graphics/data/Mesh.h"
#include "math/Vector3.h"

EERIEPOLY * BCCheckInPoly(float x, float y, float z)
{
	long px, pz;
	px = x * ACTIVEBKG->Xmul;

	if ((px >= ACTIVEBKG->Xsize)
	        ||	(px < 0))
		return NULL;

	pz = z * ACTIVEBKG->Zmul;

	if ((pz >= ACTIVEBKG->Zsize)
	        ||	(pz < 0))
		return NULL;

	EERIEPOLY * ep;
	EERIE_BKG_INFO * eg;
	EERIEPOLY * found = NULL;

	eg = (EERIE_BKG_INFO *)&ACTIVEBKG->Backg[px+pz*ACTIVEBKG->Xsize];

	for (long k = 0; k < eg->nbpolyin; k++)
	{
		ep = eg->polyin[k];

		if (!(ep->type & POLY_WATER) &&  !(ep->type & POLY_TRANS))
		{
			if (ep->min.y > y)
			{
				if (PointIn2DPolyXZ(ep, x, z))
				{
					if (found == NULL) found = ep;
					else if (ep->min.y < found->min.y) found = ep;
				}
			}
			else if (ep->min.y + 45.f > y)
				if (PointIn2DPolyXZ(ep, x, z))
				{
					return NULL;
				}
		}
	}

	if (found)
	{
		eg = (EERIE_BKG_INFO *)&ACTIVEBKG->Backg[px+pz*ACTIVEBKG->Xsize];

		for (long k = 0; k < eg->nbpolyin; k++)
		{
			ep = eg->polyin[k];

			if (!(ep->type & POLY_WATER) &&  !(ep->type & POLY_TRANS))
			{
				if (ep != found)
					if (ep->min.y < found->min.y)
						if (ep->min.y > found->min.y - 160.f)
						{
							if (PointIn2DPolyXZ(ep, x, z))
							{
								return NULL;
							}
						}
			}
		}
	}

	return found;
}
