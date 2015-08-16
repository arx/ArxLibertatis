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

static void CopyVertices(EERIEPOLY * ep,long to, long from) {
	ep->v[to] = ep->v[from];
	ep->tv[to] = ep->tv[from];
	ep->nrml[to] = ep->nrml[from];
}

static bool NearlyEqual(float a,float b) {
	
	if (glm::abs(a-b)<0.01f) return true;

	if (glm::abs(b-a)<0.01f) return true;

	return false;
}

static bool Quadable(EERIEPOLY * ep, EERIEPOLY * ep2, float tolerance) {

	long count=0;

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

	ep->norm = CalcFaceNormal(ep->v);

	if(glm::abs(glm::dot(ep->norm, ep2->norm)) < 1.f - tolerance)
		return false;

	for (long i=0;i<3;i++)
	{
		long common=-1;
		long common2=-1;

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
		ep2->tv[3].color = ep2->v[3].color = Color::white.toRGB();
		ep2->tv[3].rhw = ep2->v[3].rhw = 1.f;

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
bool TryToQuadify(EERIEPOLY * ep,EERIE_3DOBJ * eobj) {
	
	const float tolerance = 0.1f;
	
	float centerx = (ep->v[0].p.x + ep->v[1].p.x + ep->v[2].p.x) * (1.f/3);
	float centerz = (ep->v[0].p.z + ep->v[1].p.z + ep->v[2].p.z) * (1.f/3);
	
	short tilex = centerx * ACTIVEBKG->Xmul;
	short tilez = centerz * ACTIVEBKG->Zmul;
	short radius = 1;
	
	short minx = std::max(tilex - radius, 0);
	short maxx = std::min(tilex + radius, ACTIVEBKG->Xsize - 1);
	short minz = std::max(tilez - radius, 0);
	short maxz = std::min(tilez + radius, ACTIVEBKG->Zsize - 1);
	
	for(long kl = 0; kl < 2; kl++)
	for(short z = minz; z <= maxz; z++)
	for(short x = minx; x <= maxx; x++) {
		EERIE_BKG_INFO *eg = &ACTIVEBKG->fastdata[x][z];
		
		long val1 = 0;

		long type, val2;
		if(!GetNameInfo(eobj->name, type, val1, val2))
			return false;

		if(type != TYPE_ROOM)
			return false;

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

/*!
 * \brief Checks for angular difference between normals
 * \param norm
 * \param norm2
 * \return
 */
static bool LittleAngularDiff(Vec3f * norm, Vec3f * norm2) {
	return closerThan(*norm, *norm2, 1.41421f);
}

void ARX_PrepareBackgroundNRMLs() {
	
	Vec3f nrml;
	Vec3f cur_nrml;
	float count;
	
	for(short z = 0; z < ACTIVEBKG->Zsize; z++)
	for(short x = 0; x < ACTIVEBKG->Xsize; x++) {
		EERIE_BKG_INFO * eg = &ACTIVEBKG->fastdata[x][z];
		for(short l = 0; l < eg->nbpoly; l++) {
			EERIEPOLY * ep = &eg->polydata[l];
			
			u8 nbvert = (ep->type & POLY_QUAD) ? 4 : 3;
			
			for(u8 k = 0; k < nbvert; k++) {
				float ttt = 1.f;
				
				if(k == 3) {
					nrml = ep->norm2;
					count = 1.f;
				} else if(k > 0 && nbvert > 3) {
					nrml = (ep->norm + ep->norm2);
					count = 2.f;
					ttt = .5f;
				} else {
					nrml = ep->norm;
					count = 1.f;
				}
				
				cur_nrml = nrml * ttt;
				
				short radius = 4;
				
				short minx = std::max(x - radius, 0);
				short maxx = std::min(x + radius, ACTIVEBKG->Xsize - 1);
				short minz = std::max(z - radius, 0);
				short maxz = std::min(z + radius, ACTIVEBKG->Zsize - 1);
				
				for(short j2 = minz; j2 < maxz; j2++)
				for(short i2 = minx; i2 < maxx; i2++) {
					EERIE_BKG_INFO * eg2 = &ACTIVEBKG->fastdata[i2][j2];
					
					for(long kr = 0; kr < eg2->nbpoly; kr++) {
						EERIEPOLY * ep2 = &eg2->polydata[kr];

						u8 nbvert2 = (ep2->type & POLY_QUAD) ? 4 : 3;

						if(ep != ep2)
							for(u8 k2 = 0; k2 < nbvert2; k2++) {
								if(glm::abs(ep2->v[k2].p.x - ep->v[k].p.x) < 2.f
								   && glm::abs(ep2->v[k2].p.y - ep->v[k].p.y) < 2.f
								   && glm::abs(ep2->v[k2].p.z - ep->v[k].p.z) < 2.f
								) {
									if(k2 == 3) {
										if(LittleAngularDiff(&cur_nrml, &ep2->norm2)) {
											nrml += ep2->norm2;
											count += 1.f;
											nrml += cur_nrml;
											count += 1.f;
										}
									} else if(k2 > 0 && nbvert2 > 3) {
										Vec3f tnrml = (ep2->norm + ep2->norm2) * .5f;
										if(LittleAngularDiff(&cur_nrml, &tnrml)) {
											nrml += tnrml * 2.f;
											count += 2.f;
										}
									} else {
										if(LittleAngularDiff(&cur_nrml, &ep2->norm)) {
											nrml += ep2->norm;
											count += 1.f;
										}
									}
								}
							}
					}
				}
				
				count = 1.f / count;
				ep->tv[k].p = nrml * count;
				
			}
		}
	}
	
	for(short z = 0; z < ACTIVEBKG->Zsize; z++)
	for(short x = 0; x < ACTIVEBKG->Xsize; x++) {
		EERIE_BKG_INFO * eg = &ACTIVEBKG->fastdata[x][z];
		for(long l = 0; l < eg->nbpoly; l++) {
			EERIEPOLY * ep = &eg->polydata[l];
			
			u8 nbvert = (ep->type & POLY_QUAD) ? 4 : 3;
			
			for(u8 k = 0; k < nbvert; k++) {
				ep->nrml[k] = ep->tv[k].p;
			}
			
			float d = 0.f;
			
			for(long ii = 0; ii < nbvert; ii++) {
				d = std::max(d, glm::distance(ep->center, ep->v[ii].p));
			}
			
			ep->v[0].rhw = d;
		}
	}

}
