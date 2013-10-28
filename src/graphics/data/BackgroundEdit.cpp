/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
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

#include "graphics/data/BackgroundEdit.h"

#include "graphics/GraphicsTypes.h"
#include "graphics/Math.h"

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

	if(ep2->type & POLY_QUAD)
		return false;

	if(ep->tex != ep2->tex)
		return false;

	long typ1=ep->type&(~POLY_QUAD);
	long typ2=ep2->type&(~POLY_QUAD);

	if(typ1!=typ2)
		return false;

	if((ep->type & POLY_TRANS) && ep->transval != ep2->transval)
		return false;

	CalcFaceNormal(ep,ep->v);

	if(fabs(glm::dot(ep->norm, ep2->norm)) < 1.f - tolerance)
		return false;

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
		ep2->v[3].p = ep->v[ep_notcommon].p;
		ep2->tv[3].uv = ep2->v[3].uv = ep->v[ep_notcommon].uv;
		ep2->tv[3].color = ep2->v[3].color = Color::white.toBGR();
		ep2->tv[3].rhw = ep2->v[3].rhw = 1.f;

	DeclareEGInfo(ep->v[3].p.x, ep->v[3].p.z);

	ep2->center = (ep2->v[0].p + ep2->v[1].p + ep2->v[2].p + ep2->v[3].p) * 0.25f;
	ep2->max = glm::max(ep2->max, ep2->v[3].p);
	ep2->min = glm::min(ep2->min, ep2->v[3].p);

	ep2->norm2 = ep->norm;

	ep2->area += fdist((ep2->v[1].p + ep2->v[2].p) * .5f, ep2->v[3].p)
				 * fdist(ep2->v[3].p, ep2->v[1].p)*.5f; // should this be v[2] instead of v[3]?

		return true;
	}

	return false;
}

#define TYPE_ROOM	2
bool TryToQuadify(EERIEPOLY * ep,EERIE_3DOBJ * eobj)
{
	float cx = (ep->v[0].p.x + ep->v[1].p.x + ep->v[2].p.x);
	float cz = (ep->v[0].p.z + ep->v[1].p.z + ep->v[2].p.z);
	long posx = cx * (1.0f/3) * ACTIVEBKG->Xmul;
	long posz = cz * (1.0f/3) * ACTIVEBKG->Zmul;

	long dx = std::max(0L, posx - 1);
	long fx = std::min(posx + 1, ACTIVEBKG->Xsize - 1L);
	long dz = std::max(0L, posz - 1);
	long fz = std::min(posz + 1, ACTIVEBKG->Zsize - 1L);
	float tolerance=0.1f;

	for(long kl = 0; kl < 2; kl++)
	for(long zz = dz; zz <= fz; zz++)
	for(long xx = dx; xx <= fx; xx++) {
		long val1 = 0;

		long type, val2;
		if(!GetNameInfo(eobj->name, type, val1, val2))
			return false;

		if(type != TYPE_ROOM)
			return false;

		EERIE_BKG_INFO *eg = (EERIE_BKG_INFO *)&ACTIVEBKG->Backg[xx+zz*ACTIVEBKG->Xsize];

		if(eg)
		for(long n = 0; n < eg->nbpoly; n++) {
			EERIEPOLY *ep2 = (EERIEPOLY *)&eg->polydata[n];

			if(ep2->room != val1)
				continue;

			if(ep == ep2)
				continue;

			if(kl == 0 && (ep2->type & POLY_QUAD)) {
				if(Quadable(ep, ep2, tolerance))
					return true;
			} else if(kl == 1 && !(ep2->type & POLY_QUAD)) {
				if(Quadable(ep, ep2, tolerance))
					return true;
			}
		}
	}

	return false;
}
