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
// EERIEAnchors
///////////////////////////////////////////////////////////////////////////////
//
// Description:
//		Anchors funcs
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved
///////////////////////////////////////////////////////////////////////////////

#include <EERIEAnchors.h>
#include "eerieapp.h"
#include "EERIEPathfinder.h"
#include "EERIEMath.h"
#include "HermesMain.h"
#include "danae.h"
#include "ARX_Text.h"

extern float MAX_ALLOWED_PER_SECOND;
extern BOOL DIRECT_PATH;
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>


EERIEPOLY * ANCHOR_CheckInPolyPrecis(float x, float y, float z)
{
	long px, pz;
	F2L(x * ACTIVEBKG->Xmul, &px);

	if (px >= ACTIVEBKG->Xsize - 1)
	{
		return NULL;
	}

	if (px <= 0)
	{
		return NULL;
	}

	F2L(z * ACTIVEBKG->Zmul, &pz);

	if (pz >= ACTIVEBKG->Zsize - 1)
	{
		return NULL;
	}

	if (pz <= 0)
	{
		return NULL;
	}

	EERIEPOLY * ep;
	FAST_BKG_DATA * feg;
	EERIEPOLY * found = NULL;
	float foundY = 9999999.f;

	for (long j = pz - 1; j <= pz + 1; j++)
		for (long i = px - 1; i <= px + 1; i++)
		{
			feg = &ACTIVEBKG->fastdata[i][j];

			for (long k = 0; k < feg->nbpolyin; k++)
			{
				ep = feg->polyin[k];

				if (!(ep->type & (POLY_WATER | POLY_TRANS | POLY_NOCOL))
				        &&	(PointIn2DPolyXZ(ep, x, z))
				   )
				{
					EERIE_3D poss;
					poss.x = x;
					poss.y = y;
					poss.z = z;
					float yy;

					if ((GetTruePolyY(ep, &poss, &yy))
					        &&	(yy >= y)
					        &&	((found == NULL) || ((found != NULL) && (yy <= foundY)))
					   )
					{
						found = ep;
						foundY = yy;
					}
				}
			}
		}

	return found;
}

EERIEPOLY * ANCHOR_CheckInPoly(float x, float y, float z)
{
	long px, pz;
	F2L(x * ACTIVEBKG->Xmul, &px);

	if (px >= ACTIVEBKG->Xsize)
	{
		return NULL;
	}

	if (px < 0)
	{
		return NULL;
	}

	F2L(z * ACTIVEBKG->Zmul, &pz);

	if (pz >= ACTIVEBKG->Zsize)
	{
		return NULL;
	}

	if (pz < 0)
	{
		return NULL;
	}

	EERIEPOLY * ep;
	FAST_BKG_DATA * feg;
	EERIEPOLY * found = NULL;

	feg = &ACTIVEBKG->fastdata[px][pz];

	for (long k = 0; k < feg->nbpolyin; k++)
	{
		ep = feg->polyin[k];

		if (!(ep->type & (POLY_WATER | POLY_TRANS | POLY_NOCOL))
		        &&	(ep->max.y >= y)
		        &&	(ep != found)
		        &&	((ep->norm.y < 0.f) || ((ep->type & POLY_QUAD) && (ep->norm2.y < 0.f)))
		        &&	(PointIn2DPolyXZ(ep, x, z)))
		{
			if ((found == NULL) || ((found != NULL) && (ep->min.y < found->min.y)))
				found = ep;
		}
	}

	if (!found) return CheckInPolyPrecis(x, y, z);

	return found;
}
 

extern EERIE_3D vector2D;
__inline float ANCHOR_IsPolyInCylinder(EERIEPOLY * ep, EERIE_CYLINDER * cyl, long flags)
{
	if (!(flags & CFLAG_EXTRA_PRECISION))
	{
		if (ep->area < 100.f) return 999999.f;
	}

	if (PointInCylinder(cyl, &ep->center)) 
	{
		if (ep->norm.y < 0.5f)
			return ep->min.y;

		return ep->center.y;
	}

	float min = __min(cyl->origin.y, cyl->origin.y + cyl->height);
	float max = __max(cyl->origin.y, cyl->origin.y + cyl->height);

	if (min > ep->max.y) return 999999.f;

	if (max < ep->min.y) return 999999.f;

	long to;

	if (ep->type & POLY_QUAD) to = 4;
	else to = 3;

	long r = to - 1;
	float anything = 999999.f;
	EERIE_3D center;
	long n;

	for (n = 0; n < to; n++)
	{
		if (flags & CFLAG_EXTRA_PRECISION)
		{
			for (long o = 0; o < 5; o++)
			{
				float p = (float)o * DIV5;
				center.x = (ep->v[n].sx * p + ep->center.x * (1.f - p));
				center.y = (ep->v[n].sy * p + ep->center.y * (1.f - p));
				center.z = (ep->v[n].sz * p + ep->center.z * (1.f - p));

				if (PointInCylinder(cyl, &center)) 
				{
					anything = __min(anything, center.y);
					return anything;
				}
			}
		}


		if ((ep->area > 2000.f)
		        || (flags & CFLAG_EXTRA_PRECISION)
		   )
		{
			center.x = (ep->v[n].sx + ep->v[r].sx) * DIV2;
			center.y = (ep->v[n].sy + ep->v[r].sy) * DIV2;
			center.z = (ep->v[n].sz + ep->v[r].sz) * DIV2;

			if (PointInCylinder(cyl, &center)) 
			{
				anything = __min(anything, center.y);
				return anything;
			}

			if ((ep->area > 4000.f) || (flags & CFLAG_EXTRA_PRECISION))
			{
				center.x = (ep->v[n].sx + ep->center.x) * DIV2;
				center.y = (ep->v[n].sy + ep->center.y) * DIV2;
				center.z = (ep->v[n].sz + ep->center.z) * DIV2;

				if (PointInCylinder(cyl, &center)) 
				{
					anything = __min(anything, center.y);
					return anything;
				}
			}

			if ((ep->area > 6000.f) || (flags & CFLAG_EXTRA_PRECISION))
			{
				center.x = (center.x + ep->v[n].sx) * DIV2;
				center.y = (center.y + ep->v[n].sy) * DIV2;
				center.z = (center.z + ep->v[n].sz) * DIV2;

				if (PointInCylinder(cyl, &center)) 
				{
					anything = __min(anything, center.y);
					return anything;
				}
			}
		}

		if (PointInCylinder(cyl, (EERIE_3D *)&ep->v[n])) 
		{
			anything = __min(anything, ep->v[n].sy);
			return anything;
		}

		r++;

		if (r >= to) r = 0;

	}

	if ((anything != 999999.f) && (ep->norm.y < 0.1f) && (ep->norm.y > -0.1f))
		anything = __min(anything, ep->min.y);

	return anything;
}


//-----------------------------------------------------------------------------
// Returns 0 if nothing in cyl
// Else returns Y Offset to put cylinder in a proper place
float ANCHOR_CheckAnythingInCylinder(EERIE_CYLINDER * cyl, INTERACTIVE_OBJ * ioo, long flags)
{
	long rad;
	F2L((cyl->radius + 230)*ACTIVEBKG->Xmul, &rad); 

	long px, pz;
	F2L(cyl->origin.x * ACTIVEBKG->Xmul, &px);

	if (px > ACTIVEBKG->Xsize - 2 - rad)
		return 0.f;

	if (px < 1 + rad)
		return 0.f;

	F2L(cyl->origin.z * ACTIVEBKG->Zmul, &pz);

	if (pz > ACTIVEBKG->Zsize - 2 - rad)
		return 0.f;

	if (pz < 1 + rad)
		return 0.f;

	float anything = 999999.f; 

	EERIEPOLY * ep;
	FAST_BKG_DATA * feg;

	/* TO KEEP...
		EERIE_BKG_INFO * eg=&ACTIVEBKG->Backg[px+pz*ACTIVEBKG->Xsize];
		if (	(cyl->origin.y+cyl->height < eg->tile_miny)
				&& (cyl->origin.y > eg->tile_miny)
			//||	(cyl->origin.y > eg->tile_maxy)
			)
		{
			return 999999.f;
		}
		*/

	for (long j = pz - rad; j <= pz + rad; j++)
		for (long i = px - rad; i <= px + rad; i++)
		{
			feg = &ACTIVEBKG->fastdata[i][j];

			for (long k = 0; k < feg->nbpoly; k++)
			{
				ep = &feg->polydata[k];

				if (ep->type & (POLY_WATER | POLY_TRANS | POLY_NOCOL)) continue;

				if (ep->min.y < anything)
				{
					float minanything = __min(anything, ANCHOR_IsPolyInCylinder(ep, cyl, flags));

					if (anything != minanything)
					{
						anything = minanything;
					}
				}
			}
		}

	ep = ANCHOR_CheckInPolyPrecis(cyl->origin.x, cyl->origin.y + cyl->height, cyl->origin.z);

	if (ep) anything = __min(anything, ep->min.y);

	float tempo;

	if ((ep) && (GetTruePolyY(ep, &cyl->origin, &tempo)))
		anything = __min(anything, tempo);

	anything = anything - cyl->origin.y;
	return anything;
}
extern long MOVING_CYLINDER;
BOOL ANCHOR_AttemptValidCylinderPos(EERIE_CYLINDER * cyl, INTERACTIVE_OBJ * io, long flags)
{
	float anything = ANCHOR_CheckAnythingInCylinder(cyl, io, flags);

	if ((flags & CFLAG_LEVITATE) && (anything == 0.f)) return TRUE;

	if (anything >= 0.f) // Falling Cylinder but valid pos !
	{
		if (flags & CFLAG_RETURN_HEIGHT)
			cyl->origin.y += anything;

		return TRUE;
	}

	EERIE_CYLINDER tmp;

	if (!(flags & CFLAG_ANCHOR_GENERATION))
	{

		memcpy(&tmp, cyl, sizeof(EERIE_CYLINDER));

		while (anything < 0.f)
		{
			tmp.origin.y += anything;
			anything = ANCHOR_CheckAnythingInCylinder(&tmp, io, flags);
		}

		anything = tmp.origin.y - cyl->origin.y;
	}

	if (MOVING_CYLINDER)
	{
		if (flags & CFLAG_NPC)
		{
			float tolerate;

			if ((io) && (io->ioflags & IO_NPC) && (io->_npcdata->pathfind.listnb > 0) && (io->_npcdata->pathfind.listpos < io->_npcdata->pathfind.listnb))
			{
				tolerate = -80;
			}
			else
				tolerate = -45;

			if (anything < tolerate) return FALSE;
		}

		if (io && (!(flags & CFLAG_JUST_TEST)))
		{
			if ((flags & CFLAG_PLAYER) && (anything < 0.f))
			{
				if (player.jumpphase)
				{
					io->_npcdata->climb_count = MAX_ALLOWED_PER_SECOND;
					return FALSE;
				}

				float dist;
				dist = __max(TRUEVector_Magnitude(&vector2D), 1.f);
				float pente;
				pente = EEfabs(anything) / dist * DIV2; 
				io->_npcdata->climb_count += pente;

				if (io->_npcdata->climb_count > MAX_ALLOWED_PER_SECOND)
				{
					io->_npcdata->climb_count = MAX_ALLOWED_PER_SECOND;
					return FALSE;
				}

				if (anything < -55) 
				{
					io->_npcdata->climb_count = MAX_ALLOWED_PER_SECOND;
					return FALSE;
				}
			}
		}
	}
	else if (anything < -45) return FALSE;

	if ((flags & CFLAG_SPECIAL) && (anything < -40))
	{
		if (flags & CFLAG_RETURN_HEIGHT)
			cyl->origin.y += anything;

		return FALSE;
	}

	memcpy(&tmp, cyl, sizeof(EERIE_CYLINDER));
	tmp.origin.y += anything;
	anything = ANCHOR_CheckAnythingInCylinder(&tmp, io, flags); 

	if (anything < 0.f)
	{
		if (flags & CFLAG_RETURN_HEIGHT)
		{
			while (anything < 0.f)
			{
				tmp.origin.y += anything;
				anything = ANCHOR_CheckAnythingInCylinder(&tmp, io, flags);
			}

			cyl->origin.y = tmp.origin.y; 
		}

		return FALSE;
	}

	cyl->origin.y = tmp.origin.y;
	return TRUE;
}



BOOL ANCHOR_ARX_COLLISION_Move_Cylinder(IO_PHYSICS * ip, INTERACTIVE_OBJ * io, float MOVE_CYLINDER_STEP, long flags)
{
	MOVING_CYLINDER = 1;
	DIRECT_PATH = TRUE;
	IO_PHYSICS test;

	if (ip == NULL)
	{
		MOVING_CYLINDER = 0;
		return FALSE;
	}

	float distance = TRUEEEDistance3D(&ip->startpos, &ip->targetpos);

	if (distance <= 0.f)
	{
		MOVING_CYLINDER = 0;
		return TRUE; 
	}

	float onedist = 1.f / distance;
	EERIE_3D mvector;
	mvector.x = (ip->targetpos.x - ip->startpos.x) * onedist;
	mvector.y = (ip->targetpos.y - ip->startpos.y) * onedist;
	mvector.z = (ip->targetpos.z - ip->startpos.z) * onedist;

	while (distance > 0.f)
	{
		// First We compute current increment
		float curmovedist = __min(distance, MOVE_CYLINDER_STEP);

		distance -= curmovedist;
		//CUR_FRAME_SLICE=curmovedist*onedist;
		// Store our cylinder desc into a test struct
		memcpy(&test, ip, sizeof(IO_PHYSICS));

		// uses test struct to simulate movement.
		test.cyl.origin.x += mvector.x * curmovedist;
		test.cyl.origin.y += mvector.y * curmovedist;
		test.cyl.origin.z += mvector.z * curmovedist;

		vector2D.x = mvector.x * curmovedist;
		vector2D.y = 0.f;
		vector2D.z = mvector.z * curmovedist;

		if ((flags & CFLAG_CHECK_VALID_POS)
		        && (CylinderAboveInvalidZone(&test.cyl)))
			return FALSE;

		if (ANCHOR_AttemptValidCylinderPos(&test.cyl, io, flags))
		{
			memcpy(ip, &test, sizeof(IO_PHYSICS));

		}
		else
		{
			if (flags & CFLAG_CLIMBING)
			{
				memcpy(&test.cyl, &ip->cyl, sizeof(EERIE_CYLINDER));
				test.cyl.origin.y += mvector.y * curmovedist;

				if (ANCHOR_AttemptValidCylinderPos(&test.cyl, io, flags))
				{
					memcpy(ip, &test, sizeof(IO_PHYSICS));
					goto oki;
				}
			}

			DIRECT_PATH = FALSE;
			// Must Attempt To Slide along collisions
			register EERIE_3D	vecatt;
			EERIE_3D			rpos		= { 0, 0, 0 };
			EERIE_3D			lpos		= { 0, 0, 0 };
			long				RFOUND		=	0;
			long				LFOUND		=	0;
			long				maxRANGLE	=	90;
			long				maxLANGLE	=	270;
			float				ANGLESTEPP;

			if (flags & CFLAG_EASY_SLIDING)    // player sliding in fact...
			{
				ANGLESTEPP	=	10.f;
				maxRANGLE	=	70;
				maxLANGLE	=	290;
			}
			else ANGLESTEPP	=	30.f;

			register float rangle	=	ANGLESTEPP;
			register float langle	=	360.f - ANGLESTEPP;


			while (rangle <= maxRANGLE)   //tries on the Right and Left sides
			{
				memcpy(&test.cyl, &ip->cyl, sizeof(EERIE_CYLINDER)); 
				float t = DEG2RAD(MAKEANGLE(rangle));
				_YRotatePoint(&mvector, &vecatt, EEcos(t), EEsin(t));
				test.cyl.origin.x	+=	vecatt.x * curmovedist;
				test.cyl.origin.y	+=	vecatt.y * curmovedist;
				test.cyl.origin.z	+=	vecatt.z * curmovedist;

				if (ANCHOR_AttemptValidCylinderPos(&test.cyl, io, flags))
				{
					memcpy(&rpos, &test.cyl.origin, sizeof(EERIE_3D));
					RFOUND = 1;
				}

				rangle += ANGLESTEPP;

				memcpy(&test.cyl, &ip->cyl, sizeof(EERIE_CYLINDER));   
				t = DEG2RAD(MAKEANGLE(langle));
				_YRotatePoint(&mvector, &vecatt, EEcos(t), EEsin(t));
				test.cyl.origin.x	+=	vecatt.x * curmovedist;
				test.cyl.origin.y	+=	vecatt.y * curmovedist;
				test.cyl.origin.z	+=	vecatt.z * curmovedist;

				if (ANCHOR_AttemptValidCylinderPos(&test.cyl, io, flags))
				{
					memcpy(&lpos, &test.cyl.origin, sizeof(EERIE_3D));
					LFOUND = 1;
				}

				langle -= ANGLESTEPP;

				if ((RFOUND) || (LFOUND)) break;
			}

			if ((LFOUND) && (RFOUND))
			{
				langle = 360.f - langle;

				if (langle < rangle)
				{
					memcpy(&ip->cyl.origin, &lpos, sizeof(EERIE_3D));
					distance -= curmovedist;
				}
				else
				{
					memcpy(&ip->cyl.origin, &rpos, sizeof(EERIE_3D));
					distance -= curmovedist;
				}
			}
			else if (LFOUND)
			{
				memcpy(&ip->cyl.origin, &lpos, sizeof(EERIE_3D));
				distance -= curmovedist;
			}
			else if (RFOUND)
			{
				memcpy(&ip->cyl.origin, &rpos, sizeof(EERIE_3D));
				distance -= curmovedist;
			}
			else  //stopped
			{
				ip->velocity.x = ip->velocity.y = ip->velocity.z = 0.f;
				MOVING_CYLINDER = 0;
				return FALSE;
			}
		}

	oki:
		;
	}

	MOVING_CYLINDER = 0;
	return TRUE;
}


//*************************************************************************************
// Clears all Anchor data from a Background
//*************************************************************************************
void AnchorData_ClearAll(EERIE_BACKGROUND * eb)
{
	//	EERIE_PATHFINDER_Release();
	EERIE_PATHFINDER_Clear();
	EERIE_BKG_INFO * eg;

	for (long j = 0; j < eb->Zsize; j++)
		for (long i = 0; i < eb->Xsize; i++)
		{
			eg = &eb->Backg[i+j*eb->Xsize];

			if ((eg->nbianchors) && (eg->ianchors))
				free(eg->ianchors);

			eg->nbianchors = 0;
			eg->ianchors = NULL;
		}

	if ((eb->anchors) && (eb->nbanchors))
	{
		for (int j = 0; j < eb->nbanchors; j++)
		{
			if ((eb->anchors[j].nblinked) &&
			        (eb->anchors[j].linked))
			{
				free((void *)eb->anchors[j].linked);
				eb->anchors[j].linked = NULL;
			}
		}

		free((void *)eb->anchors);
	}

	eb->anchors = NULL;
	eb->nbanchors = 0;
}
#define INC_HEIGHT 20
#define INC_RADIUS 10

bool CylinderAboveInvalidZone(EERIE_CYLINDER * cyl)
{
	float count = 0;
	float failcount = 0;

	for (float rad = 0; rad < cyl->radius; rad += 10.f)
		for (float ang = 0; ang < 360; ang += 45)
		{
			if (rad == 0) ang = 360;

			EERIE_3D pos;
			pos.x = cyl->origin.x - EEsin(DEG2RAD(ang)) * rad;
			pos.y = cyl->origin.y - 20.f;
			pos.z = cyl->origin.z + EEcos(DEG2RAD(ang)) * rad;
			EERIEPOLY * ep = ANCHOR_CheckInPoly(pos.x, pos.y, pos.z);

			if (!ep) continue;

			if (ep->type & POLY_NOPATH) return true;

			count += 1.f;
			float vy;
			GetTruePolyY(ep, &pos, &vy);

			if (EEfabs(vy - cyl->origin.y) > 160.f)
				failcount++;
		}

	float failratio = failcount / count;

	if (failratio > 0.75f) return true;

	return false;
}
//*************************************************************************************
// Adds an Anchor... and tries to generate the best possible cylinder for it
//*************************************************************************************
#define MUST_BE_BIG 1

bool DirectAddAnchor_Original_Method(EERIE_BACKGROUND * eb, EERIE_BKG_INFO * eg, EERIE_3D * pos, long flags)
{
	long found = 0;
	long best = 0;
	long stop_radius = 0;
	float best_dist = 99999999999.f;
	float v_dist = 99999999999.f;

	EERIE_CYLINDER testcyl;
	EERIE_CYLINDER currcyl;
	EERIE_CYLINDER bestcyl;

	bestcyl.height = 0;
	bestcyl.radius = 0;
	currcyl.radius = 40;
	currcyl.height = -165.f;
	currcyl.origin.x = pos->x;
	currcyl.origin.y = pos->y;
	currcyl.origin.z = pos->z;

	stop_radius = 0;
	found = 0;
	long climb = 0;

	while (stop_radius != 1)
	{
		memcpy(&testcyl, &currcyl, sizeof(EERIE_CYLINDER));
		testcyl.radius += INC_RADIUS;

		if (ANCHOR_AttemptValidCylinderPos(&testcyl, NULL, CFLAG_NO_INTERCOL | CFLAG_EXTRA_PRECISION | CFLAG_ANCHOR_GENERATION))
		{
			memcpy(&currcyl, &testcyl, sizeof(EERIE_CYLINDER));
			found = 1;
		}
		else
		{
			if ((testcyl.origin.y != currcyl.origin.y)
			        && (EEfabs(testcyl.origin.y - pos->y) < 50))
			{
				testcyl.radius -= INC_RADIUS;
				memcpy(&currcyl, &testcyl, sizeof(EERIE_CYLINDER));
				climb++;
			}
			else
				stop_radius = 1;
		}

		if (climb > 4) stop_radius = 1;

		if (currcyl.radius >= 50.f) stop_radius = 1;

	}

	if (found)
	{
		float dist = TRUEEEDistance3D(pos, &currcyl.origin);
		float vd = EEfabs(pos->y - currcyl.origin.y);

		if ((currcyl.radius >= bestcyl.radius))
		{
			if (((best_dist > dist) && (currcyl.radius == bestcyl.radius))
			        || (currcyl.radius > bestcyl.radius))
			{
				memcpy(&bestcyl, &currcyl, sizeof(EERIE_CYLINDER));
				best_dist = dist;
				v_dist = vd;
				best = 1;
			}
		}
	}

	if (!best) return FALSE;

	if (CylinderAboveInvalidZone(&bestcyl)) return FALSE;

	for (long k = 0; k < eb->nbanchors; k++)
	{
		_ANCHOR_DATA * ad = &eb->anchors[k];

		if (TRUEEEDistance3D(&ad->pos, &bestcyl.origin) < 50.f) return FALSE;

		if (TRUEDistance2D(ad->pos.x, ad->pos.z, bestcyl.origin.x, bestcyl.origin.z) < 45.f)
		{
			if (EEfabs(ad->pos.y - bestcyl.origin.y) < 90.f) return FALSE;

			EERIEPOLY * ep = ANCHOR_CheckInPolyPrecis(ad->pos.x, ad->pos.y, ad->pos.z);
			EERIEPOLY * ep2 = ANCHOR_CheckInPolyPrecis(ad->pos.x, bestcyl.origin.y, ad->pos.z);

			if (ep2 == ep) return FALSE;
		}

	}

	eg->ianchors = (long *)realloc(eg->ianchors, sizeof(long) * (eg->nbianchors + 1));

	if (!eg->ianchors) HERMES_Memory_Emergency_Out();

	eg->ianchors[eg->nbianchors] = eb->nbanchors;
	eg->nbianchors++;

	eb->anchors = (_ANCHOR_DATA *)realloc(eb->anchors, sizeof(_ANCHOR_DATA) * (eb->nbanchors + 1));

	if (!eb->anchors) HERMES_Memory_Emergency_Out();

	_ANCHOR_DATA * ad = &eb->anchors[eb->nbanchors];
	ad->pos.x = bestcyl.origin.x; 
	ad->pos.y = bestcyl.origin.y;
	ad->pos.z = bestcyl.origin.z; 
	ad->height = bestcyl.height; 
	ad->radius = bestcyl.radius; 
	ad->linked = NULL;
	ad->nblinked = 0;
	ad->flags = 0;
	eb->nbanchors++;
	return TRUE;
}
bool AddAnchor_Original_Method(EERIE_BACKGROUND * eb, EERIE_BKG_INFO * eg, EERIE_3D * pos, long flags)
{
	long found = 0;
	long best = 0;
	long stop_radius = 0;
	float best_dist = 99999999999.f;
	float v_dist = 99999999999.f;

	EERIE_CYLINDER testcyl;
	EERIE_CYLINDER currcyl;
	EERIE_CYLINDER bestcyl;

	bestcyl.height = 0;
	bestcyl.radius = 0;

	
	for (long rad = 0; rad < 20; rad += 10) 
		for (long ang = 0; ang < 360; ang += 45) // 45
		{
			float t = DEG2RAD((float)ang);
			// We set our current position depending on given position, radius & angle.
			currcyl.radius = 40; 
			currcyl.height = -165; 
			currcyl.origin.x = pos->x - EEsin(t) * (float)rad;
			currcyl.origin.y = pos->y;
			currcyl.origin.z = pos->z + EEcos(t) * (float)rad;

			stop_radius = 0;
			found = 0;
			long climb = 0;

			while ((stop_radius != 1))
			{
				memcpy(&testcyl, &currcyl, sizeof(EERIE_CYLINDER));
				testcyl.radius += INC_RADIUS;

				if (ANCHOR_AttemptValidCylinderPos(&testcyl, NULL, CFLAG_NO_INTERCOL | CFLAG_EXTRA_PRECISION | CFLAG_ANCHOR_GENERATION))
				{
					memcpy(&currcyl, &testcyl, sizeof(EERIE_CYLINDER));
					found = 1;
				}
				else
				{
					if ((testcyl.origin.y != currcyl.origin.y)
					        && (EEfabs(testcyl.origin.y - pos->y) < 50))
					{
						testcyl.radius -= INC_RADIUS;
						memcpy(&currcyl, &testcyl, sizeof(EERIE_CYLINDER));
						climb++;
					}
					else
						stop_radius = 1;
				}

				if (climb > 4) stop_radius = 1;

				if (currcyl.radius >= 50.f) stop_radius = 1;
			}

			if (found)
			{
				float dist = TRUEEEDistance3D(pos, &currcyl.origin);
				float vd = EEfabs(pos->y - currcyl.origin.y);

				if (currcyl.radius >= bestcyl.radius)	
				{
					if (((best_dist > dist) && (currcyl.radius == bestcyl.radius))
					        || (currcyl.radius > bestcyl.radius))
					{
						memcpy(&bestcyl, &currcyl, sizeof(EERIE_CYLINDER));
						best_dist = dist;
						v_dist = vd;
						best = 1;
					}
				}
			}
		}

	if (!best) return FALSE;

	if (CylinderAboveInvalidZone(&bestcyl)) return FALSE;

	if (flags == MUST_BE_BIG)
	{
		if (bestcyl.radius < 60) return FALSE;
	}

	// avoid to recreate same anchor twice...
	if (0)
		for (long k = 0; k < eb->nbanchors; k++)
		{
			_ANCHOR_DATA * ad = &eb->anchors[k];

			if ((ad->pos.x == bestcyl.origin.x)
			        &&	(ad->pos.y == bestcyl.origin.y)
			        &&	(ad->pos.z == bestcyl.origin.z))
			{
				if (ad->radius >= bestcyl.radius)
					return FALSE;

				if (ad->radius <= bestcyl.radius)
				{
					ad->height = bestcyl.height;
					ad->radius = bestcyl.radius;
					return FALSE;
				}
			}
		}

	eg->ianchors = (long *)realloc(eg->ianchors, sizeof(long) * (eg->nbianchors + 1));

	if (!eg->ianchors) HERMES_Memory_Emergency_Out();

	eg->ianchors[eg->nbianchors] = eb->nbanchors;
	eg->nbianchors++;

	eb->anchors = (_ANCHOR_DATA *)realloc(eb->anchors, sizeof(_ANCHOR_DATA) * (eb->nbanchors + 1));

	if (!eb->anchors) HERMES_Memory_Emergency_Out();

	_ANCHOR_DATA * ad = &eb->anchors[eb->nbanchors];
	ad->pos.x = bestcyl.origin.x; 
	ad->pos.y = bestcyl.origin.y; 
	ad->pos.z = bestcyl.origin.z; 
	ad->height = bestcyl.height; 
	ad->radius = bestcyl.radius; 
	ad->linked = NULL;
	ad->nblinked = 0;
	ad->flags = 0;
	eb->nbanchors++;
	return TRUE;
}

//**********************************************************************************************
// Adds an Anchor Link to an Anchor
//**********************************************************************************************
void AddAnchorLink(EERIE_BACKGROUND * eb, long anchor, long linked) 
{
	// Avoid to store Already existing Links
	for (long i = 0; i < eb->anchors[anchor].nblinked; i++)
		if (eb->anchors[anchor].linked[i] == linked) return;

	// Realloc & fill data
	eb->anchors[anchor].linked = (long *)realloc(eb->anchors[anchor].linked, sizeof(long) * (eb->anchors[anchor].nblinked + 1));

	if (!eb->anchors[anchor].linked) HERMES_Memory_Emergency_Out();

	eb->anchors[anchor].linked[eb->anchors[anchor].nblinked] = linked;
	eb->anchors[anchor].nblinked++;
}

//**********************************************************************************************
// Generates Links between Anchors for a background
//**********************************************************************************************

void AnchorData_Create_Links_Original_Method(EERIE_BACKGROUND * eb)
{
	EERIE_BKG_INFO * eg;
	EERIE_BKG_INFO * eg2;
	long ii, ia, ji, ja;
	EERIE_3D p1, p2; 
	char text[256];
	long count = 0;
	long per;
	long lastper = -1;
	long total = eb->Zsize * eb->Xsize;

	for (long j = 0; j < eb->Zsize; j++)
		for (long i = 0; i < eb->Xsize; i++)
		{
			F2L((float)count / (float)total * 100.f, &per);

			if (per != lastper)
			{
				sprintf(text, "Anchor Links Generation: %d%%", per);
				lastper = per;
				_ShowText(text);
			}

			danaeApp.WinManageMess();
			count++;
			eg = &eb->Backg[i+j*eb->Xsize];
			long precise = 0;

			for (long kkk = 0; kkk < eg->nbpolyin; kkk++)
			{
				EERIEPOLY * ep = eg->polyin[kkk];

				if (ep->type & POLY_PRECISE_PATH)
				{
					precise = 1;
					break;
				}
			}


			for (long k = 0; k < eg->nbianchors; k++)
			{
				ii = i - 2;
				ia = i + 2;
				ji = j - 2;
				ja = j + 2;
				FORCERANGE(ii, 0, eb->Xsize - 1);
				FORCERANGE(ia, 0, eb->Xsize - 1);
				FORCERANGE(ji, 0, eb->Zsize - 1);
				FORCERANGE(ja, 0, eb->Zsize - 1);

				for (long j2 = ji; j2 <= ja; j2++)
					for (long i2 = ii; i2 <= ia; i2++)
					{
						eg2 = &eb->Backg[i2+j2*eb->Xsize];
						long precise2 = 0;

						for (long kkk = 0; kkk < eg2->nbpolyin; kkk++)
						{
							EERIEPOLY * ep2 = eg2->polyin[kkk];

							if (ep2->type & POLY_PRECISE_PATH)
							{
								precise2 = 1;
								break;
							}
						}

						for (long k2 = 0; k2 < eg2->nbianchors; k2++)
						{
							// don't treat currently treated anchor
							if (eg->ianchors[k] == eg2->ianchors[k2]) continue;

							memcpy(&p1, &eb->anchors[eg->ianchors[k]].pos, sizeof(EERIE_3D));
							memcpy(&p2, &eb->anchors[eg2->ianchors[k2]].pos, sizeof(EERIE_3D));
							p1.y += 10.f;
							p2.y += 10.f;
							long _onetwo = 0;
							BOOL treat = TRUE;
							float dist = TRUEEEDistance3D(&p1, &p2);
							float dd = TRUEDistance2D(p1.x, p1.z, p2.x, p2.z);

							if (dd < 5.f) continue;

							if (dd > 200.f) continue; 

							if (precise || precise2)
							{
								if (dist > 120.f) continue;
							}
							else	if (dist > 200.f) continue;

							if (EEfabs(p1.y - p2.y) > dd * 0.9f) continue;

					
							IO_PHYSICS ip;
							ip.startpos.x = ip.cyl.origin.x = p1.x;
							ip.startpos.y = ip.cyl.origin.y = p1.y;
							ip.startpos.z = ip.cyl.origin.z = p1.z;
							ip.targetpos.x = p2.x;
							ip.targetpos.y = p2.y;
							ip.targetpos.z = p2.z;
						
							ip.cyl.height = eb->anchors[eg->ianchors[k]].height; 
							ip.cyl.radius = eb->anchors[eg->ianchors[k]].radius;
							EERIE_3D vect;
							vect.x = p2.x - p1.x;
							vect.y = p2.y - p1.y;
							vect.z = p2.z - p1.z;

							long t = 2;

							if (ANCHOR_ARX_COLLISION_Move_Cylinder(&ip, NULL, 20, CFLAG_CHECK_VALID_POS | CFLAG_NO_INTERCOL | CFLAG_EASY_SLIDING | CFLAG_NPC | CFLAG_JUST_TEST | CFLAG_EXTRA_PRECISION)) //CFLAG_SPECIAL
							{
								if (TRUEDistance2D(ip.cyl.origin.x, ip.cyl.origin.z, ip.targetpos.x, ip.targetpos.z) > 25) 
									t--;
								else _onetwo = 1;
							}
							else t--;

							if (t == 1)
							{
								ip.startpos.x = ip.cyl.origin.x = p2.x;
								ip.startpos.y = ip.cyl.origin.y = p2.y;
								ip.startpos.z = ip.cyl.origin.z = p2.z;
								ip.targetpos.x = p1.x;
								ip.targetpos.y = p1.y;
								ip.targetpos.z = p1.z;

								ip.cyl.height = eb->anchors[eg2->ianchors[k2]].height;
								ip.cyl.radius = eb->anchors[eg2->ianchors[k2]].radius; 

								if (ANCHOR_ARX_COLLISION_Move_Cylinder(&ip, NULL, 20, CFLAG_CHECK_VALID_POS | CFLAG_NO_INTERCOL | CFLAG_EASY_SLIDING | CFLAG_NPC | CFLAG_JUST_TEST | CFLAG_EXTRA_PRECISION | CFLAG_RETURN_HEIGHT)) //CFLAG_SPECIAL
								{
									if (TRUEDistance2D(ip.cyl.origin.x, ip.cyl.origin.z, ip.targetpos.x, ip.targetpos.z) > 25) 
										t--;
									else _onetwo |= 2;
								}
								else t--;
							}
							else t--;

							if (t <= 0)
								treat = FALSE;
							else treat = TRUE;

							if (treat)
							{
								if (_onetwo)
								{
									AddAnchorLink(eb, eg->ianchors[k], eg2->ianchors[k2]);
									AddAnchorLink(eb, eg2->ianchors[k2], eg->ianchors[k]);
								}

							}
						}
					}
			}
		}

	EERIE_PATHFINDER_Create(eb);
}

void AnchorData_Create_Phase_II_Original_Method(EERIE_BACKGROUND * eb)
{
	char text[256];
	EERIE_BKG_INFO * eg;
	EERIE_3D pos;
	long k;
	float count = 0;


	long lastper	=	-1;
	long per;
	float total		=	ARX_CLEAN_WARN_CAST_FLOAT(eb->Zsize * eb->Xsize);

	for (long j = 0; j < eb->Zsize; j++)
		for (long i = 0; i < eb->Xsize; i++)
		{
			float current_y = 99999999999.f;
			F2L((float)count / total * 100.f, &per);

			if (per != lastper)
			{
				sprintf(text, "Anchor Generation: %d%% (Pass II)", per);
				lastper = per;
				_ShowText(text);
			}

			count += 1.f;
			danaeApp.WinManageMess();

			eg = &eb->Backg[i+j*eb->Xsize];
			pos.x = (float)((float)((float)i) * (float)eb->Xdiv);
			pos.y = 0.f;
			pos.z = (float)((float)((float)j) * (float)eb->Zdiv);
			k = 0;
			EERIE_CYLINDER currcyl;
			currcyl.radius = 30;
			currcyl.height = -150.f;
			currcyl.origin.x = pos.x;
			currcyl.origin.y = pos.y;
			currcyl.origin.z = pos.z;

			if (eg->nbpolyin)
			{
				long ok = 0;

				for (long kkk = 0; kkk < eg->nbpolyin; kkk++)
				{
					EERIEPOLY * ep = eg->polyin[kkk];

					if (ep->type & POLY_PRECISE_PATH)
					{
						ok = 1;
						break;
					}
				}

				if (!ok) continue;

				float roof = GetTileMinY(i, j); 
				current_y = GetTileMaxY(i, j); 

				while (current_y > roof)
				{
					long added = 0;

					for (float pposz = 0.f; pposz < 1.f; pposz += 0.1f)
						for (float pposx = 0.f; pposx < 1.f; pposx += 0.1f)
						{
							currcyl.origin.x = pos.x + pposx * eb->Xdiv;
							currcyl.origin.z = pos.z + pposz * eb->Zdiv;
							currcyl.origin.y = current_y;

							EERIEPOLY * ep2 = ANCHOR_CheckInPolyPrecis(currcyl.origin.x, currcyl.origin.y - 10.f, currcyl.origin.z);

							if (!ep2)
								continue;

							if (!(ep2->type & POLY_DOUBLESIDED) && (ep2->norm.y > 0.f))
								continue;

							if (ep2->type & POLY_NOPATH)
								continue;

							if (ANCHOR_AttemptValidCylinderPos(&currcyl, NULL, CFLAG_NO_INTERCOL | CFLAG_EXTRA_PRECISION | CFLAG_RETURN_HEIGHT | CFLAG_ANCHOR_GENERATION))
							{
								EERIEPOLY * ep2 = ANCHOR_CheckInPolyPrecis(currcyl.origin.x, currcyl.origin.y - 10.f, currcyl.origin.z);

								if (!ep2)
									continue;

								if (!(ep2->type & POLY_DOUBLESIDED) && (ep2->norm.y > 0.f))
									continue;

								if (ep2->type & POLY_NOPATH)
									continue;

								if (DirectAddAnchor_Original_Method(eb, eg, &currcyl.origin, 0))
								{
									added = 1;
								}
							}
						}


					if (added)
						current_y -= 160.f; 

					current_y -= 50.f; 
				}
			}
		}

}

void AnchorData_Create_Original_Method(EERIE_BACKGROUND * eb)
{
	char text[256];
	AnchorData_ClearAll(eb);
	EERIE_BKG_INFO * eg;
	EERIEPOLY * ep;
	EERIE_3D pos;
#define DECALLL 20.f
	long k;
	float count = 0;
	long lastper	=	-1;
	long per;
	float total		=	ARX_CLEAN_WARN_CAST_FLOAT(eb->Zsize * eb->Xsize * 9);

	for (long j = 0; j < eb->Zsize; j++)
		for (long i = 0; i < eb->Xsize; i++)
		{
			long LASTFOUND = 0;

			for (long divv = 0; divv < 9; divv++)
			{
				long divvx, divvy;

				switch (divv)
				{
					case 0:
						divvx = 0;
						divvy = 0;
						break;
					case 1:
						divvx = 1;
						divvy = 0;
						break;
					case 2:
						divvx = 2;
						divvy = 0;
						break;
					case 3:
						divvx = 0;
						divvy = 1;
						break;
					case 4:
						divvx = 1;
						divvy = 1;
						break;
					case 5:
						divvx = 2;
						divvy = 1;
						break;
					case 6:
						divvx = 0;
						divvy = 2;
						break;
					case 7:
						divvx = 1;
						divvy = 2;
						break;
					case 8:
						divvx = 2;
						divvy = 2;
						break;

				}

				float current_y = 99999999999.f;
				F2L((float)count / total * 100.f, &per);

				if (per != lastper)
				{
					sprintf(text, "Anchor Generation: %d%%", per);
					lastper = per;
					_ShowText(text);
				}

				count += 1.f;
				danaeApp.WinManageMess();

				if (LASTFOUND) break;

				eg = &eb->Backg[i+j*eb->Xsize];
				pos.x = (float)((float)((float)i + 0.33f * (float)divvx) * (float)eb->Xdiv);
				pos.y = 0.f;
				pos.z = (float)((float)((float)j + 0.33f * (float)divvy) * (float)eb->Zdiv);
				ep = GetMinPoly(pos.x, pos.y, pos.z);
				k = 0;
				EERIE_CYLINDER currcyl;
				currcyl.radius = 20 - (4.f * divv);
				currcyl.height = -120.f;
				currcyl.origin.x = pos.x;
				currcyl.origin.y = pos.y;
				currcyl.origin.z = pos.z;

				if (ep)
				{

					EERIEPOLY * epmax;
					epmax = GetMaxPoly(pos.x, pos.y, pos.z);
					float roof = 9999999.f;

					if (ep) roof = ep->min.y - 300;

					if (epmax) roof = epmax->min.y - 300;

					current_y = ep->max.y;

					while (current_y > roof)
					{
						currcyl.origin.y = current_y;
						EERIEPOLY * ep2 = ANCHOR_CheckInPolyPrecis(currcyl.origin.x, currcyl.origin.y - 30.f, currcyl.origin.z);

						if (ep2 && !(ep2->type & POLY_DOUBLESIDED) && (ep2->norm.y > 0.f))
							ep2 = NULL;

						if ((ep2) && !(ep2->type & POLY_NOPATH))
						{
							BOOL bval = ANCHOR_AttemptValidCylinderPos(&currcyl, NULL, CFLAG_NO_INTERCOL | CFLAG_EXTRA_PRECISION | CFLAG_RETURN_HEIGHT | CFLAG_ANCHOR_GENERATION);

							if ((bval)
							        && (currcyl.origin.y - 10.f <= current_y))
							{
								EERIEPOLY * ep2 = ANCHOR_CheckInPolyPrecis(currcyl.origin.x, currcyl.origin.y - 38.f, currcyl.origin.z);

								if (ep2 && !(ep2->type & POLY_DOUBLESIDED) && (ep2->norm.y > 0.f))
								{
									current_y -= 10.f;
								}
								else if ((ep2) && (ep2->type & POLY_NOPATH))
								{
									current_y -= 10.f;
								}
								else if (AddAnchor_Original_Method(eb, eg, &currcyl.origin, 0))
								{
									LASTFOUND++;
									current_y = currcyl.origin.y + currcyl.height;
								}
								else current_y -= 10.f;
							}
							else current_y -= 10.f;
						}
						else current_y -= 10.f;
					}
				}
			}
		}

	AnchorData_Create_Phase_II_Original_Method(eb);
	AnchorData_Create_Links_Original_Method(eb);
}

////////////////////////////////////////////////////////////////////////////////////
//					ALTERNATIVE METHOD
void AnchorData_Create_Alternative_Method_I(EERIE_BACKGROUND * eb)
{
	char text[256];
	AnchorData_ClearAll(eb);
	EERIE_BKG_INFO * eg;
	EERIEPOLY * ep;
	EERIE_3D pos;
	long k;
	float count = 0;

	long lastper	=	-1;
	long per;
	float total		=	ARX_CLEAN_WARN_CAST_FLOAT(eb->Zsize * eb->Xsize * 4);

	for (long j = 0; j < eb->Zsize; j++)
		for (long i = 0; i < eb->Xsize; i++)
		{
			long LASTFOUND = 0;

			for (long divv = 0; divv < 4; divv++)
			{
				long divvx, divvy;

				switch (divv)
				{
					case 0:
						divvx = 0;
						divvy = 0;
						break;
					case 1:
						divvx = 1;
						divvy = 1;
						break;
					case 2:
						divvx = 0;
						divvy = 1;
						break;
					case 3:
						divvx = 1;
						divvy = 0;
						break;
				}

				float current_y = 99999999999.f;
				F2L((float)count / total * 100.f, &per);

				if (per != lastper)
				{
					sprintf(text, "Anchor Generation: %d%%", per);
					lastper = per;
					_ShowText(text);
				}

				count += 1.f;
				danaeApp.WinManageMess();

				if (LASTFOUND) break;

				eg = &eb->Backg[i+j*eb->Xsize];
				pos.x = (float)((float)((float)i + 0.5f * (float)divvx) * (float)eb->Xdiv);
				pos.y = 0.f;
				pos.z = (float)((float)((float)j + 0.5f * (float)divvy) * (float)eb->Zdiv);
				ep = GetMinPoly(pos.x, pos.y, pos.z);
				k = 0;
				EERIE_CYLINDER currcyl;
				currcyl.radius = 20 - (4.f * divv);
				currcyl.height = -120.f;
				currcyl.origin.x = pos.x;
				currcyl.origin.y = pos.y;
				currcyl.origin.z = pos.z;

				if (ep)
				{

					EERIEPOLY * epmax;
					epmax = GetMaxPoly(pos.x, pos.y, pos.z);
					float roof = 9999999.f;

					if (ep) roof = ep->min.y - 300;

					if (epmax) roof = epmax->min.y - 300;

					current_y = ep->max.y;

					while (current_y > roof)
					{
						currcyl.origin.y = current_y;
						EERIEPOLY * ep2 = ANCHOR_CheckInPolyPrecis(currcyl.origin.x, currcyl.origin.y - 30.f, currcyl.origin.z);

						if (ep2 && !(ep2->type & POLY_DOUBLESIDED) && (ep2->norm.y > 0.f))
							ep2 = NULL;

						if ((ep2) && !(ep2->type & POLY_NOPATH))
						{
							BOOL bval = ANCHOR_AttemptValidCylinderPos(&currcyl, NULL, CFLAG_NO_INTERCOL | CFLAG_EXTRA_PRECISION | CFLAG_RETURN_HEIGHT | CFLAG_ANCHOR_GENERATION);

							if ((bval)
							        && (currcyl.origin.y - 10.f <= current_y))
							{
								EERIEPOLY * ep2 = ANCHOR_CheckInPolyPrecis(currcyl.origin.x, currcyl.origin.y - 38.f, currcyl.origin.z);

								if (ep2 && !(ep2->type & POLY_DOUBLESIDED) && (ep2->norm.y > 0.f))
								{
									current_y -= 10.f;
								}
								else if ((ep2) && (ep2->type & POLY_NOPATH))
								{
									current_y -= 10.f;
								}
								else if (AddAnchor_Original_Method(eb, eg, &currcyl.origin, 0))
								{
									LASTFOUND++;
									current_y = currcyl.origin.y + currcyl.height;
								}
								else current_y -= 10.f;
							}
							else current_y -= 10.f;
						}
						else current_y -= 10.f;

					}
				}
			}
		}

	AnchorData_Create_Phase_II_Original_Method(eb);
	AnchorData_Create_Links_Original_Method(eb);

	/* TO KEEP
		// Parses Anchors to refine anchor creation...
		long ii,ia,ji,ja;
		EERIE_3D p1,p2;
		EERIE_BKG_INFO * eg2;
		count=0;


		total=eb->Zsize*eb->Xsize;
		long usable=0;
		if (0)
		for (j=0;j<eb->Zsize;j++)
		for (long i=0;i<eb->Xsize;i++)
		{
			F2L((float)count/(float)total*100.f,&per);
			if (per!=lastper)
			{
				sprintf(text,"Anchor Generation Pass II: %d%% Suitable %d",per,usable);
				lastper=per;
				_ShowText(text);
			}
			count++;
			eg=&eb->Backg[i+j*eb->Xsize];
			for (long k=0;k<eg->nbianchors;k++)
			{
				ii=i-2;
				ia=i+2;
				ji=j-2;
				ja=j+2;
				FORCERANGE(ii,0,eb->Xsize-1);
				FORCERANGE(ia,0,eb->Xsize-1);
				FORCERANGE(ji,0,eb->Zsize-1);
				FORCERANGE(ja,0,eb->Zsize-1);
				for (long j2=ji;j2<=ja;j2++)
				for (long i2=ii;i2<=ia;i2++)
				{
					eg2=&eb->Backg[i2+j2*eb->Xsize];
					for (long k2=0;k2<eg2->nbianchors;k2++)
					{
						// don't treat currently treated anchor
						if (eg->ianchors[k] == eg2->ianchors[k2]) continue;
						memcpy(&p1,&eb->anchors[eg->ianchors[k]].pos,sizeof(EERIE_3D));
						memcpy(&p2,&eb->anchors[eg2->ianchors[k2]].pos,sizeof(EERIE_3D));
						p1.y+=10.f;
						p2.y+=10.f;
						float dist=TRUEEEDistance3D(&p1,&p2);
						if (dist>120.f) continue;
						if (EEfabs(p1.y-p2.y)>80.f) continue;
						if ((eb->anchors[eg->ianchors[k]].radius>=40)
							&& (eb->anchors[eg2->ianchors[k2]].radius>=40)) 
							continue;
						{
							// found 2 usable anchors
							EERIE_3D pos;
							pos.x=(p1.x+p2.x)*DIV2;
							pos.y=(p1.y+p2.y)*DIV2;
							pos.z=(p1.z+p2.z)*DIV2;
							if (AddAnchor(eb,eg,&pos,MUST_BE_BIG))
								usable++;
						}
					}
				}
			}
		}*/

}
long Method = 0;
void AnchorData_Create(EERIE_BACKGROUND * eb)
{
	switch (Method)
	{
		case 0:
			AnchorData_Create_Original_Method(eb);
			break;
		case 1:
			AnchorData_Create_Alternative_Method_I(eb);
			break;
	}
}
