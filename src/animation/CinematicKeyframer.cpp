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

#include "animation/CinematicKeyframer.h"

#include <cmath>
#include <cstdlib>
#include <cstring>

#include "animation/Cinematic.h"
#include "core/GameTime.h"

static const float C_MIN_F32 = 1.175494351e-38F;
inline bool C_NEQUAL_F32(float f1, float f2) {
	return fabs(f1 - f2) >= C_MIN_F32;
}

using std::malloc;
using std::realloc;
using std::memcpy;
using std::memmove;

CinematicTrack	* CKTrack;

bool AllocTrack(int sf, int ef, float fps)
{
	if(CKTrack)
		return false;

	CKTrack = (CinematicTrack *)malloc(sizeof(CinematicTrack));

	if(!CKTrack)
		return false;

	CKTrack->startframe = sf;
	CKTrack->endframe = ef;
	CKTrack->currframe = 0.f;
	CKTrack->fps = fps;
	CKTrack->nbkey = 0;
	CKTrack->key = NULL;
	CKTrack->pause = true;

	return true;
}

bool DeleteTrack() {
	
	if(!CKTrack)
		return false;
	
	free(CKTrack->key);
	free(CKTrack);
	CKTrack = NULL;
	
	return true;
}

static C_KEY * SearchAndMoveKey(int f)
{
	C_KEY * k = CKTrack->key + CKTrack->nbkey - 1;
	int nb = CKTrack->nbkey;

	while(nb) {
		if(f > k->frame)
			break;

		k--;
		nb--;
	}

	nb = CKTrack->nbkey - nb;

	if(nb) {
		memmove((void *)(k + 2), (void *)(k + 1), sizeof(C_KEY)*nb);
	}

	return k + 1;
}

C_KEY * SearchKey(int f, int * num)
{
	if(!CKTrack || !CKTrack->nbkey)
		return NULL;

	C_KEY * k = CKTrack->key;
	int nb = CKTrack->nbkey;

	while(nb) {
		if(f == k->frame) {
			*num = CKTrack->nbkey - nb;
			return k;
		}

		k++;
		nb--;
	}

	return NULL;
}
 
void UpDateKeyLight(int frame)
{
	C_KEY *klightprev2, *klightnext2;
	int num;

	C_KEY * k = SearchKey(frame, &num);
	C_KEY * klightprev = k;
	C_KEY * klightnext = k;
	
	C_KEY * kbase = k;
	int num2 = num;

	//on cherche le range de deux lights
	//prev
	while(num2) {
		k--;

		if((k->fx & 0xFF000000) == FX_LIGHT) {
			klightprev = k;
			break;
		}

		num2--;
	}

	//next
	k = kbase;
	num2 = num;

	while(num2 < (CKTrack->nbkey - 1)) {
		k++;

		if((k->fx & 0xFF000000) == FX_LIGHT) {
			klightnext = k;
			break;
		}

		num2++;
	}

	//on crépie le range avec ces deux valeurs
	kbase->light.prev = klightprev;
	kbase->light.next = klightnext;

	if((kbase->fx & 0xFF000000) == FX_LIGHT) {
		klightprev2 = klightnext2 = kbase;
	} else {
		kbase->light.intensity = -1.f;
		klightprev2 = klightprev;
		klightnext2 = klightnext;
	}

	//prev
	k = kbase - 1;

	while(k >= CKTrack->key) {
		if(klightprev == kbase) {
			k->light.intensity = -1.f;
		}

		k->light.next = klightnext2;

		if((k->fx & 0xFF000000) == FX_LIGHT)
			break;

		k->light.prev = klightprev;
		k--;
	}

	//next
	k = kbase + 1;

	while(k < (CKTrack->key + CKTrack->nbkey)) {
		if(klightnext == kbase) {
			k->light.intensity = -1.f;
		}

		k->light.prev = klightprev2;

		if((k->fx & 0xFF000000) == FX_LIGHT)
			break;

		k->light.next = klightnext;
		k++;
	}
}

void UpDateAllKeyLight(void)
{
	//update les lights
	C_KEY * kk = CKTrack->key;
	int nb = CKTrack->nbkey;

	while(nb--) {
		UpDateKeyLight(kk->frame);
		kk++;
	}
}

bool AddKey(C_KEY * key, bool writecolor, bool writecolord, bool writecolorf)
{
	int			num;

	if(!CKTrack || (key->frame < CKTrack->startframe) || (key->frame > CKTrack->endframe))
		return false;

	C_KEY * k = SearchKey(key->frame, &num);
	if(!k) {
		if(!CKTrack->nbkey) {
			CKTrack->key = k = (C_KEY *)malloc(sizeof(C_KEY));
		} else {
			CKTrack->key = (C_KEY *)realloc(CKTrack->key, sizeof(C_KEY) * (CKTrack->nbkey + 1));
			k = SearchAndMoveKey(key->frame);
		}

		CKTrack->nbkey++;

		k->frame = key->frame;
	}

	if(key->numbitmap > -2)
		k->numbitmap = key->numbitmap;

	if(key->fx > -2) {
		if((key->fx > 255) && (k->fx > 0)) {
			k->fx |= key->fx;
		} else {
			if((k->fx >= 255) && (key->fx >= 0)) {
				k->fx |= key->fx;
			} else {
				k->fx = key->fx;
			}
		}
	}

	if(key->speed > -1.f) {
		k->speed = key->speed;
	}

	if(writecolor)
		k->color = key->color;

	if(writecolord)
		k->colord = key->colord;

	if(writecolorf)
		k->colorf = key->colorf;

	if(key->idsound > -2) {
		k->idsound = key->idsound;
	}

	if(key->force > -2)
		k->force = key->force;

	k->frame = key->frame;
	k->pos = key->pos;
	k->angz = key->angz;

	if(key->typeinterp > -2)
		k->typeinterp = key->typeinterp;

	float a = -2.f;

	if(C_NEQUAL_F32(key->light.intensity, a)) {
		k->light = key->light;
	}

	k->posgrille = key->posgrille;
	k->angzgrille = key->angzgrille;
	k->speedtrack = key->speedtrack;

	UpDateAllKeyLight();

	return true;
}

bool AddKeyLoad(C_KEY * key)
{
	int num;

	if(!CKTrack || (key->frame < CKTrack->startframe) || (key->frame > CKTrack->endframe))
		return false;
	
	C_KEY * k = SearchKey(key->frame, &num);
	if(!k) {
		if(!CKTrack->nbkey) {
			CKTrack->key = k = (C_KEY *)malloc(sizeof(C_KEY));
		} else {
			CKTrack->key = (C_KEY *)realloc(CKTrack->key, sizeof(C_KEY) * (CKTrack->nbkey + 1));
			k = SearchAndMoveKey(key->frame);
		}

		CKTrack->nbkey++;
	}

	k->numbitmap = key->numbitmap;
	k->fx = key->fx;
	k->speed = key->speed;
	k->color = key->color;
	k->colord = key->colord;
	k->colorf = key->colorf;
	k->frame = key->frame;
	k->pos = key->pos;
	k->angz = key->angz;
	k->typeinterp = key->typeinterp;
	k->idsound = key->idsound;
	k->force = key->force;
	k->light = key->light;
	k->posgrille = key->posgrille;
	k->angzgrille = key->angzgrille;
	k->speedtrack = key->speedtrack;

	return true;
}

C_KEY * GetKey(int f, int * num)
{
	if(!CKTrack || !CKTrack->key)
		return NULL;

	C_KEY * k = CKTrack->key + CKTrack->nbkey - 1;
	int nb = CKTrack->nbkey;

	while(nb) {
		if(f >= k->frame) {
			*num = nb;
			return k;
		}

		k--;
		nb--;
	}

	return NULL;
}

float GetAngleInterpolation(float d, float e)
{
	float da = e - d;

	if(fabs(da) > 180.f) {
		if(da > 0.f)
			da -= 360.f;
		else
			da += 360.f;
	}

	return da;
}

bool GereTrack(Cinematic * c, float fpscurr)
{
	float	a, unmoinsa, alight = 0, unmoinsalight = 0;
	int		num;
	C_KEY	* kprec, *ksuivsuiv;
	float	t1, t2, t3, f0, f1, f2, f3, p0, p1, temp;
	C_KEY	* lightprec, *lightnext;

	if(!CKTrack || !CKTrack->nbkey)
		return false;

	if(CKTrack->pause)
		return true;

	C_KEY * k = GetKey((int)CKTrack->currframe, &num);
	C_KEY * ksuiv = (num == CKTrack->nbkey) ? k : k + 1;

	if(ksuiv->frame != k->frame)
		a = (CKTrack->currframe - (float)k->frame) / ((float)(ksuiv->frame - k->frame));
	else
		a = 1.f;

	c->a = unmoinsa = 1.0f - a;

	c->numbitmap		= k->numbitmap;
	c->numbitmapsuiv	= ksuiv->numbitmap;
	c->ti				= k->typeinterp;
	c->fx				= k->fx;
	c->fxsuiv			= ksuiv->fx;
	c->color			= k->color;
	c->colord			= k->colord;
	c->colorflash		= k->colorf;
	c->speed			= k->speed;
	c->idsound			= k->idsound;
	c->force			= k->force;

	if((k->fx & 0xFF000000) == FX_LIGHT) {
		lightprec = k;
	} else {
		lightprec = k->light.prev;
	}

	lightnext = k->light.next;
	c->lightd = lightnext->light;

	if(lightprec != lightnext) {
		alight = (CKTrack->currframe - (float)lightprec->frame) / ((float)(lightnext->frame - lightprec->frame));

		if(alight > 1.f)
			alight = 1.f;

		unmoinsalight = 1.0f - alight;
	} else {
		if(k == (CKTrack->key + CKTrack->nbkey - 1)) {
			alight			= 1.f;
			unmoinsalight	= 0.f;
		}
		else
		{

			//alight can't be used because it is not initialized
//ARX_BEGIN: jycorbel (2010-07-19) - Set light coeff to 0 to keep null all possibly light created from uninitialyzed var.
/*
alight = unmoinsalight = 0.f; //default values needed when : k->typeinterp == INTERP_BEZIER (0) || k->typeinterp == INTERP_LINEAR (1)
consequences on light :
				c->light : position = (0,0,0);
				c->light : color	= (0,0,0); == BLACK
				c->light : fallin	= fallout		= 0;
				c->light : intensite = intensiternd	= 0;
			arx_assert( k->typeinterp != INTERP_BEZIER && k->typeinterp != INTERP_LINEAR );
*/
//ARX_END: jycorbel (2010-07-19)
			//ARX_END: jycorbel (2010-06-28)
			c->lightd = k->light;
			lightprec = k;
		}
	}

	c->posgrille	  = k->posgrille;
	c->angzgrille	  = k->angzgrille;
	c->posgrillesuiv  = ksuiv->posgrille;
	c->angzgrillesuiv = ksuiv->angzgrille;

	switch(k->typeinterp) {
		case INTERP_NO:
			c->pos = k->pos;
			c->angz = k->angz;
			c->possuiv = ksuiv->pos;
			c->angzsuiv = ksuiv->angz;
			c->light = lightprec->light;
			c->speedtrack = k->speedtrack;
			break;
		case INTERP_LINEAR:
			c->pos = ksuiv->pos * a + k->pos * unmoinsa;
			c->angz = k->angz + a * GetAngleInterpolation(k->angz, ksuiv->angz);
			c->speedtrack = a * ksuiv->speedtrack + unmoinsa * k->speedtrack;

			{
				CinematicLight ldep;
				CinematicLight lend;

				if(lightprec->light.intensity < 0.f) {
					c->light.intensity = -1;
					break;
				} else {
					ldep = lightprec->light;
				}

				if(c->lightd.intensity < 0.f) {
					break;
				} else {
					lend = c->lightd;
				}

				c->light.pos = lend.pos * alight + ldep.pos * unmoinsalight;
				c->light.fallin = alight * lend.fallin + unmoinsalight * ldep.fallin;
				c->light.fallout = alight * lend.fallout + unmoinsalight * ldep.fallout;
				c->light.color = lend.color * alight + ldep.color * unmoinsalight;
				c->light.intensity = alight * lend.intensity + unmoinsalight * ldep.intensity;
				c->light.intensiternd = alight * lend.intensiternd
				                        + unmoinsalight * ldep.intensiternd;
			}
			break;
		case INTERP_BEZIER:
			c->light = k->light;

			ksuivsuiv = ((num + 1) < CKTrack->nbkey) ? ksuiv + 1 : ksuiv;
			kprec = (num > 1) ? k - 1 : k;

			t1 = a;
			t2 = t1 * t1;
			t3 = t2 * t1;
			f0 = 2.f * t3 - 3.f * t2 + 1.f;
			f1 = -2.f * t3 + 3.f * t2;
			f2 = t3 - 2.f * t2 + t1;
			f3 = t3 - t2;

			temp = ksuiv->pos.x;
			p0 = 0.5f * (temp - kprec->pos.x);
			p1 = 0.5f * (ksuivsuiv->pos.x - k->pos.x);
			c->pos.x = f0 * k->pos.x + f1 * temp + f2 * p0 + f3 * p1;

			temp = ksuiv->pos.y;
			p0 = 0.5f * (temp - kprec->pos.y);
			p1 = 0.5f * (ksuivsuiv->pos.y - k->pos.y);
			c->pos.y = f0 * k->pos.y + f1 * temp + f2 * p0 + f3 * p1;

			temp = ksuiv->pos.z;
			p0 = 0.5f * (temp - kprec->pos.z);
			p1 = 0.5f * (ksuivsuiv->pos.z - k->pos.z);
			c->pos.z = f0 * k->pos.z + f1 * temp + f2 * p0 + f3 * p1;

			c->angz = k->angz + a * GetAngleInterpolation(k->angz, ksuiv->angz);

			temp = ksuiv->speedtrack;
			p0 = 0.5f * (temp - kprec->speedtrack);
			p1 = 0.5f * (ksuivsuiv->speedtrack - k->speedtrack);
			c->speedtrack = f0 * k->speedtrack + f1 * temp + f2 * p0 + f3 * p1;

			{
				CinematicLight ldep;
				CinematicLight lend;

				if(lightprec->light.intensity < 0.f) {
					c->light.intensity = -1;
					break;
				} else {
					ldep = lightprec->light;
				}

				if(c->lightd.intensity < 0.f) {
					break;
				} else {
					lend = c->lightd;
				}

				c->light.pos = lend.pos * alight + ldep.pos * unmoinsalight;
				c->light.fallin = alight * lend.fallin + unmoinsalight * ldep.fallin;
				c->light.fallout = alight * lend.fallout + unmoinsalight * ldep.fallout;
				c->light.color = lend.color * alight + ldep.color * unmoinsalight;
				c->light.intensity = alight * lend.intensity + unmoinsalight * ldep.intensity;
				c->light.intensiternd = alight * lend.intensiternd
				                        + unmoinsalight * ldep.intensiternd;
			}
			break;
	}

	if(k != c->key) {
		c->key = k;
		c->changekey = true;
	}

	c->flTime += fpscurr;
	CKTrack->currframe = (((float)(c->flTime)) / 1000.f) * ((float)(GetEndFrame() - GetStartFrame())) / (float)GetTimeKeyFramer();

	if(CKTrack->currframe > (float)CKTrack->endframe) {
		CKTrack->currframe = (float)CKTrack->startframe;
		c->key = NULL;
		c->flTime = arxtime.get_updated();
	}

	return true;
}

bool GereTrackNoPlay(Cinematic * c)
{
	float	a, unmoinsa, alight = 0, unmoinsalight = 0;
	int		num;
	C_KEY	* kprec, *ksuivsuiv;
	float	t1, t2, t3, f0, f1, f2, f3, p0, p1, temp;
	C_KEY	* lightprec, *lightnext;

	if(!CKTrack || !CKTrack->nbkey || !CKTrack->pause)
		return false;

	C_KEY * k = GetKey((int) CKTrack->currframe, &num);

	if(!k)
		return false;

	C_KEY * ksuiv = (num == CKTrack->nbkey) ? k : k + 1;

	if(ksuiv->frame != k->frame)
		a = (CKTrack->currframe - (float)k->frame) / ((float)(ksuiv->frame - k->frame));
	else
		a = 1.f;

	c->a = unmoinsa = 1.0f - a;

	c->numbitmap		= k->numbitmap;
	c->numbitmapsuiv	= ksuiv->numbitmap;
	c->ti				= k->typeinterp;
	c->fx				= k->fx;
	c->fxsuiv			= ksuiv->fx;
	c->color			= k->color;
	c->colord			= k->colord;
	c->colorflash		= k->colorf;
	c->speed			= k->speed;
	c->idsound			= k->idsound;
	c->force			= k->force;

	if((k->fx & 0xFF000000) == FX_LIGHT) {
		lightprec = k;
	} else {
		lightprec = k->light.prev;
	}

	lightnext = k->light.next;
	c->lightd = lightnext->light;

	if(lightprec != lightnext) {
		alight = (CKTrack->currframe - (float)lightprec->frame) / ((float)(lightnext->frame - lightprec->frame));

		if(alight > 1.f)
			alight = 1.f;

		unmoinsalight = 1.0f - alight;
	} else {
		if(k == (CKTrack->key + CKTrack->nbkey - 1)) {
			alight			= 1.f;
			unmoinsalight	= 0.f;
		} else {
			//ARX_BEGIN: jycorbel (2010-06-28) - clean warning 'variable used without having been initialized'. @BUG
			//alight can't be used because it is not initialized but the game used un initialized alight...
//ARX_BEGIN: jycorbel (2010-07-19) - Set light coeff to 0 to keep null all possibly light created from uninitialyzed var.
/*
	alight = unmoinsalight = 0.f; //default values needed when : k->typeinterp == INTERP_BEZIER (0) || k->typeinterp == INTERP_LINEAR (1)
	consequences on light :
			c->light : position = (0,0,0);
			c->light : color	= (0,0,0); == BLACK
			c->light : fallin	= fallout		= 0;
			c->light : intensite = intensiternd	= 0;
			arx_assert( k->typeinterp != INTERP_BEZIER && k->typeinterp != INTERP_LINEAR );
*/
//ARX_END: jycorbel (2010-07-19)
			//ARX_END: jycorbel (2010-06-28)
			c->lightd = k->light;
			lightprec = k;
		}
	}

	c->posgrille		= k->posgrille;
	c->angzgrille		= k->angzgrille;
	c->posgrillesuiv	= ksuiv->posgrille;
	c->angzgrillesuiv	= ksuiv->angzgrille;

	if(k->numbitmap < 0 || ksuiv->numbitmap < 0)
		return false;

	switch(k->typeinterp) {
		case INTERP_NO:
			c->pos		= k->pos;
			c->angz		= k->angz;
			c->possuiv	= ksuiv->pos;
			c->angzsuiv	= ksuiv->angz;

			c->light	= lightprec->light;
			c->speedtrack = k->speedtrack;
			break;
		case INTERP_LINEAR:
			c->pos = ksuiv->pos * a + k->pos * unmoinsa;
			c->angz = k->angz + a * GetAngleInterpolation(k->angz, ksuiv->angz);
			c->speedtrack = a * ksuiv->speedtrack + unmoinsa * k->speedtrack;

			{
				CinematicLight ldep;
				CinematicLight lend;

				if(lightprec->light.intensity < 0.f) {
					c->light.intensity = -1.f;
					break;
				} else {
					ldep = lightprec->light;
				}

				if(c->lightd.intensity < 0.f) {
					break;
				} else {
					lend = c->lightd;
				}

				c->light.pos = lend.pos * alight + ldep.pos * unmoinsalight;
				c->light.fallin = alight * lend.fallin + unmoinsalight * ldep.fallin;
				c->light.fallout = alight * lend.fallout + unmoinsalight * ldep.fallout;
				c->light.color = lend.color * alight + ldep.color * unmoinsalight;
				c->light.intensity = alight * lend.intensity + unmoinsalight * ldep.intensity;
				c->light.intensiternd = alight * lend.intensiternd
				                        + unmoinsalight * ldep.intensiternd;
			}
			break;
			
		case INTERP_BEZIER:
			ksuivsuiv = ((num + 1) < CKTrack->nbkey) ? ksuiv + 1 : ksuiv;
			kprec = (num > 1) ? k - 1 : k;

			t1 = a;
			t2 = t1 * t1;
			t3 = t2 * t1;
			f0 = 2.f * t3 - 3.f * t2 + 1.f;
			f1 = -2.f * t3 + 3.f * t2;
			f2 = t3 - 2.f * t2 + t1;
			f3 = t3 - t2;

			temp = ksuiv->pos.x;
			p0 = 0.5f * (temp - kprec->pos.x);
			p1 = 0.5f * (ksuivsuiv->pos.x - k->pos.x);
			c->pos.x = f0 * k->pos.x + f1 * temp + f2 * p0 + f3 * p1;

			temp = ksuiv->pos.y;
			p0 = 0.5f * (temp - kprec->pos.y);
			p1 = 0.5f * (ksuivsuiv->pos.y - k->pos.y);
			c->pos.y = f0 * k->pos.y + f1 * temp + f2 * p0 + f3 * p1;

			temp = ksuiv->pos.z;
			p0 = 0.5f * (temp - kprec->pos.z);
			p1 = 0.5f * (ksuivsuiv->pos.z - k->pos.z);
			c->pos.z = f0 * k->pos.z + f1 * temp + f2 * p0 + f3 * p1;

			c->angz = k->angz + a * GetAngleInterpolation(k->angz, ksuiv->angz);

			temp = ksuiv->speedtrack;
			p0 = 0.5f * (temp - kprec->speedtrack);
			p1 = 0.5f * (ksuivsuiv->speedtrack - k->speedtrack);
			c->speedtrack = f0 * k->speedtrack + f1 * temp + f2 * p0 + f3 * p1;

			{
				CinematicLight ldep;
				CinematicLight lend;

				if(lightprec->light.intensity < 0.f) {
					c->light.intensity = -1;
					break;
				} else {
					ldep = lightprec->light;
				}

				if(c->lightd.intensity < 0.f) {
					break;
				} else {
					lend = c->lightd;
				}

				c->light.pos = lend.pos * alight + ldep.pos * unmoinsalight;
				c->light.fallin = alight * lend.fallin + unmoinsalight * ldep.fallin;
				c->light.fallout = alight * lend.fallout + unmoinsalight * ldep.fallout;
				c->light.color = lend.color * alight + ldep.color * unmoinsalight;
				c->light.intensity = alight * lend.intensity + unmoinsalight * ldep.intensity;
				c->light.intensiternd = alight * lend.intensiternd
				                        + unmoinsalight * ldep.intensiternd;
			}
			break;
	}

	if(k != c->key) {
		c->key = k;
		c->changekey = true;
	}

	return true;
}

void PlayTrack(Cinematic * c)
{
	if(!CKTrack || !CKTrack->pause)
		return;

	CKTrack->pause = false;
	c->flTime = 0; 
}

int GetCurrentFrame(void)
{
	if(!CKTrack)
		return -1;

	return (int)CKTrack->currframe;
}

float GetTimeKeyFramer()
{
	if(!CKTrack)
		return 0.f;

	float t = 0.f;
	C_KEY * k = CKTrack->key, *ksuiv;
	int nb = CKTrack->nbkey - 1;

	while(nb--) {
		ksuiv = k + 1;
		t += ((float)(ksuiv->frame - k->frame)) / (CKTrack->fps * k->speedtrack);
		k++;
	}

	return t;
}

int GetStartFrame(void)
{
	if(!CKTrack)
		return -1;

	return CKTrack->startframe;
}

int GetEndFrame(void)
{
	if(!CKTrack)
		return -1;

	return CKTrack->endframe;
}

float GetTrackFPS(void)
{
	if(!CKTrack)
		return -1;

	return CKTrack->fps;
}

void SetCurrFrame(int frame)
{
	if(!CKTrack)
		return;

	CKTrack->currframe = (float)CKTrack->startframe + (float)frame;
}
