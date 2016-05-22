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

#include "cinematic/CinematicKeyframer.h"

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <limits>

#include "cinematic/Cinematic.h"
#include "cinematic/CinematicTexture.h"
#include "cinematic/CinematicFormat.h"
#include "core/Config.h"
#include "core/Core.h"
#include "core/GameTime.h"

static const float C_MIN_F32 = 1.175494351e-38F;
inline bool C_NEQUAL_F32(float f1, float f2) {
	return glm::abs(f1 - f2) >= C_MIN_F32;
}

CinematicTrack	* CKTrack;

bool AllocTrack(int sf, int ef, float fps)
{
	if(CKTrack)
		return false;

	CKTrack = (CinematicTrack *)std::malloc(sizeof(CinematicTrack));

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

static CinematicKeyframe * SearchAndMoveKey(int f)
{
	CinematicKeyframe * k = CKTrack->key + CKTrack->nbkey - 1;
	int nb = CKTrack->nbkey;

	while(nb) {
		if(f > k->frame)
			break;

		k--;
		nb--;
	}

	nb = CKTrack->nbkey - nb;

	if(nb) {
		std::memmove((void *)(k + 2), (void *)(k + 1), sizeof(CinematicKeyframe)*nb);
	}

	return k + 1;
}

CinematicKeyframe * SearchKey(int f, int * num)
{
	if(!CKTrack || !CKTrack->nbkey)
		return NULL;

	CinematicKeyframe * k = CKTrack->key;
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
 
static void UpDateKeyLight(int frame) {
	
	CinematicKeyframe *klightprev2, *klightnext2;
	int num;

	CinematicKeyframe * k = SearchKey(frame, &num);
	CinematicKeyframe * klightprev = k;
	CinematicKeyframe * klightnext = k;
	
	CinematicKeyframe * kbase = k;
	int num2 = num;

	//on cherche le range de deux lights
	//prev
	while(num2) {
		k--;

		if((k->fx & CinematicFxAllMask) == FX_LIGHT) {
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

		if((k->fx & CinematicFxAllMask) == FX_LIGHT) {
			klightnext = k;
			break;
		}

		num2++;
	}

	//on crépie le range avec ces deux valeurs
	kbase->light.prev = klightprev;
	kbase->light.next = klightnext;

	if((kbase->fx & CinematicFxAllMask) == FX_LIGHT) {
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

		if((k->fx & CinematicFxAllMask) == FX_LIGHT)
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

		if((k->fx & CinematicFxAllMask) == FX_LIGHT)
			break;

		k->light.next = klightnext;
		k++;
	}
}

void UpDateAllKeyLight(void)
{
	//update les lights
	CinematicKeyframe * kk = CKTrack->key;
	int nb = CKTrack->nbkey;

	while(nb--) {
		UpDateKeyLight(kk->frame);
		kk++;
	}
}

bool AddKey(const CinematicKeyframe & key) {
	int			num;

	if(!CKTrack || (key.frame < CKTrack->startframe) || (key.frame > CKTrack->endframe))
		return false;

	CinematicKeyframe * k = SearchKey(key.frame, &num);
	if(!k) {
		if(!CKTrack->nbkey) {
			CKTrack->key = k = (CinematicKeyframe *)std::malloc(sizeof(CinematicKeyframe));
		} else {
			CKTrack->key = (CinematicKeyframe *)std::realloc(CKTrack->key, sizeof(CinematicKeyframe) * (CKTrack->nbkey + 1));
			k = SearchAndMoveKey(key.frame);
		}

		CKTrack->nbkey++;

		k->frame = key.frame;
	}

	if(key.numbitmap > -2)
		k->numbitmap = key.numbitmap;

	if(key.fx > -2) {
		k->fx = key.fx;
		
		/* TODO what was this code suppesed to achieve
		if((key.fx > 255) && (k->fx > 0)) {
			k->fx |= key.fx;
		} else {
			if((k->fx >= 255) && (key.fx >= 0)) {
				k->fx |= key.fx;
			} else {
				k->fx = key.fx;
			}
		}
		*/
	}

	if(key.speed > -1.f) {
		k->speed = key.speed;
	}
	
	k->color = key.color;
	k->colord = key.colord;
	k->colorf = key.colorf;
	
	if(key.idsound > -2) {
		k->idsound = key.idsound;
	}

	if(key.force > -2)
		k->force = key.force;

	k->frame = key.frame;
	k->pos = key.pos;
	k->angz = key.angz;

	if(key.typeinterp > -2)
		k->typeinterp = key.typeinterp;

	float a = -2.f;

	if(C_NEQUAL_F32(key.light.intensity, a)) {
		k->light = key.light;
	}

	k->posgrille = key.posgrille;
	k->angzgrille = key.angzgrille;
	k->speedtrack = key.speedtrack;

	UpDateAllKeyLight();

	return true;
}

bool AddKeyLoad(const CinematicKeyframe & key) {
	int num;

	if(!CKTrack || (key.frame < CKTrack->startframe) || (key.frame > CKTrack->endframe))
		return false;
	
	CinematicKeyframe * k = SearchKey(key.frame, &num);
	if(!k) {
		if(!CKTrack->nbkey) {
			CKTrack->key = k = (CinematicKeyframe *)std::malloc(sizeof(CinematicKeyframe));
		} else {
			CKTrack->key = (CinematicKeyframe *)std::realloc(CKTrack->key, sizeof(CinematicKeyframe) * (CKTrack->nbkey + 1));
			k = SearchAndMoveKey(key.frame);
		}

		CKTrack->nbkey++;
	}
	
	*k = key;
	
	return true;
}

CinematicKeyframe * GetKey(int f, int * num)
{
	if(!CKTrack || !CKTrack->key)
		return NULL;

	CinematicKeyframe * k = CKTrack->key + CKTrack->nbkey - 1;
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

static float GetAngleInterpolation(float d, float e) {
	
	float da = e - d;

	if(glm::abs(da) > 180.f) {
		if(da > 0.f)
			da -= 360.f;
		else
			da += 360.f;
	}

	return da;
}


static CinematicFadeOut getFadeOut(const Cinematic & c, const CinematicKeyframe & key, const CinematicKeyframe & pos) {
	
	if(key.numbitmap < 0 || size_t(key.numbitmap) >= c.m_bitmaps.size()) {
		return CinematicFadeOut(0.f);
	}
	
	CinematicBitmap * bitmap = c.m_bitmaps[key.numbitmap];
	if(!bitmap) {
		return CinematicFadeOut(0.f);
	}
	
	if(key.angzgrille != 0.f || pos.angz != 0.f) {
		return CinematicFadeOut(1.f);
	}
	
	// The dream effect distorts the bitmap edges, add some buffer room.
	float add = 2.f;
	if(key.fx & FX_DREAM) {
		add = 20.f;
	}
	
	// Project the bitmap corners onto a 640x480 screen.
	Vec3f s = key.posgrille - Vec3f(bitmap->m_size.x, bitmap->m_size.y, 0) * 0.5f;
	Vec3f e = key.posgrille + Vec3f(bitmap->m_size.x, bitmap->m_size.y, 0) * 0.5f;
	float fFOV = glm::radians(69.75);
	float k = glm::cos(fFOV / 2) / glm::sin(fFOV / 2) * 0.5f;
	s -= key.pos;
	s.x = s.x * 0.75f * k * 640 / s.z;
	s.y = s.y * 1.0f  * k * 480 / s.z;
	e -= key.pos;
	e.x = e.x * 0.75f * k * 640 / e.z;
	e.y = e.y * 1.0f  * k * 480 / e.z;
	
	// If an edge is outside the 640x480 screen, but inside ours, fade it.
	CinematicFadeOut fade;
	if(s.x <= -0.5f * 640.f + 5.f && s.x >= -0.5f * g_size.width() / g_sizeRatio.y - add) {
		fade.left = 1.f;
	}
	if(e.x >= 0.5f * 640.f - 5.f && e.x <= 0.5f * g_size.width() / g_sizeRatio.y + add) {
		fade.right = 1.f;
	}
	
	return fade;
}

static void updateFadeOut(Cinematic * c, CinematicTrack * track, int num, float a,
                          bool keyChanged) {
	
	if(config.interface.cinematicWidescreenMode != CinematicFadeEdges) {
		return;
	}
	
	if(float(g_size.width()) / g_size.height()
	   <= 4.f / 3.f + 10 * std::numeric_limits<float>::epsilon()) {
		// No need to fade anything for narrow screens.
		c->fadegrille = c->fadeprev = c->fadenext = CinematicFadeOut(0.f);
		c->m_nextFadegrille = CinematicFadeOut(0.f);
		return;
	}
	
	CinematicKeyframe * current = &track->key[num - 1];
	CinematicKeyframe * next = (num == track->nbkey) ? current : current + 1;
	
	if(keyChanged) {
		c->fadeprev = getFadeOut(*c, *current, *current);
		if(num == track->nbkey || next->numbitmap != current->numbitmap) {
			c->fadenext = c->fadeprev;
		} else {
			c->fadenext = getFadeOut(*c, *next, *next);
		}
		if(current->force) {
			int next = (num == track->nbkey) ? num - 1 : num;
			const CinematicKeyframe & key = track->key[next];
			if(next > 0 && track->key[next - 1].typeinterp != INTERP_NO) {
				c->m_nextFadegrille  = getFadeOut(*c, key, track->key[next - 1]);
			} else {
				c->m_nextFadegrille  = getFadeOut(*c, key, key);
			}
		} else {
			c->m_nextFadegrille = CinematicFadeOut(0.f);
		}
	}
	
	if(current->typeinterp == INTERP_NO) {
		c->fadegrille = c->fadeprev;
	} else {
		c->fadegrille.left   = c->fadenext.left * a   + c->fadeprev.left * (1.f - a);
		c->fadegrille.right  = c->fadenext.right * a  + c->fadeprev.right * (1.f - a);
		c->fadegrille.top    = c->fadenext.top * a    + c->fadeprev.top * (1.f - a);
		c->fadegrille.bottom = c->fadenext.bottom * a + c->fadeprev.bottom * (1.f - a);
	}
	
}

static void interpolateLight(float alight, CinematicKeyframe* lightprec, Cinematic* c) {
	
	float unmoinsalight = 1.0f - alight;
	
	CinematicLight ldep;
	CinematicLight lend;

	if(lightprec->light.intensity < 0.f) {
		c->m_light.intensity = -1.f;
		return;
	} else {
		ldep = lightprec->light;
	}

	if(c->m_lightd.intensity < 0.f) {
		return;
	} else {
		lend = c->m_lightd;
	}

	c->m_light.pos = lend.pos * alight + ldep.pos * unmoinsalight;
	c->m_light.fallin = alight * lend.fallin + unmoinsalight * ldep.fallin;
	c->m_light.fallout = alight * lend.fallout + unmoinsalight * ldep.fallout;
	c->m_light.color = lend.color * alight + ldep.color * unmoinsalight;
	c->m_light.intensity = alight * lend.intensity + unmoinsalight * ldep.intensity;
	c->m_light.intensiternd = alight * lend.intensiternd
							+ unmoinsalight * ldep.intensiternd;
}

void GereTrack(Cinematic * c, float fpscurr, bool resized, bool play) {
	
	if(!CKTrack || !CKTrack->nbkey)
		return;
	
	int num;
	
	if(play && CKTrack->pause)
		return;
	
	if(!play && !CKTrack->pause)
		return;
	
	CinematicKeyframe * current = GetKey((int) CKTrack->currframe, &num);

	if(!current)
		return;

	CinematicKeyframe * next = (num == CKTrack->nbkey) ? current : current + 1;
	
	float a;
	
	if(next->frame != current->frame)
		a = (CKTrack->currframe - (float)current->frame) / ((float)(next->frame - current->frame));
	else
		a = 1.f;
	
	float unmoinsa;
	
	c->a = unmoinsa = 1.0f - a;

	c->numbitmap		= current->numbitmap;
	c->m_nextNumbitmap	= next->numbitmap;
	c->ti				= current->typeinterp;
	c->fx				= current->fx;
	c->m_nextFx			= next->fx;
	c->color			= current->color;
	c->colord			= current->colord;
	c->colorflash		= current->colorf;
	c->speed			= current->speed;
	c->idsound			= current->idsound;
	c->force			= current->force;
	
	CinematicKeyframe * lightprec;
	
	if((current->fx & CinematicFxAllMask) == FX_LIGHT) {
		lightprec = current;
	} else {
		lightprec = current->light.prev;
	}

	CinematicKeyframe * lightnext = current->light.next;
	c->m_lightd = lightnext->light;
	
	float alight = 0;
	
	if(lightprec != lightnext) {
		alight = (CKTrack->currframe - (float)lightprec->frame) / ((float)(lightnext->frame - lightprec->frame));

		if(alight > 1.f)
			alight = 1.f;
	} else {
		if(current == (CKTrack->key + CKTrack->nbkey - 1)) {
			alight			= 1.f;
		} else {
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
			c->m_lightd = current->light;
			lightprec = current;
		}
	}

	c->posgrille = current->posgrille;
	c->angzgrille = current->angzgrille;
	c->m_nextPosgrille = next->posgrille;
	c->m_nextAngzgrille = next->angzgrille;
	
	if(!play)
		if(current->numbitmap < 0 || next->numbitmap < 0)
			return;
	
	switch(current->typeinterp) {
		case INTERP_NO:
			c->pos = current->pos;
			c->angz = current->angz;
			c->m_nextPos = next->pos;
			c->m_nextAngz = next->angz;
			c->m_light = lightprec->light;
			c->speedtrack = current->speedtrack;
			break;
		case INTERP_LINEAR:
			c->pos = next->pos * a + current->pos * unmoinsa;
			c->angz = current->angz + a * GetAngleInterpolation(current->angz, next->angz);
			c->speedtrack = a * next->speedtrack + unmoinsa * current->speedtrack;

			interpolateLight(alight, lightprec, c);
			break;
		case INTERP_BEZIER: {
			if(play)
				c->m_light = current->light;
			
			CinematicKeyframe * ksuivsuiv = ((num + 1) < CKTrack->nbkey) ? next + 1 : next;
			CinematicKeyframe * kprec = (num > 1) ? current - 1 : current;
			
			const Vec3f prevPos = kprec->pos;
			const Vec3f currentPos = current->pos;
			const Vec3f nextPos = next->pos;
			const Vec3f next2Pos = ksuivsuiv->pos;
			
			c->pos = glm::catmullRom(prevPos, currentPos, nextPos, next2Pos, a);
			
			c->angz = current->angz + a * GetAngleInterpolation(current->angz, next->angz);
			
			{ // TODO use glm::catmullRom
			const float t1 = a;
			const float t2 = t1 * t1;
			const float t3 = t2 * t1;
			const float f0 = 2.f * t3 - 3.f * t2 + 1.f;
			const float f1 = -2.f * t3 + 3.f * t2;
			const float f2 = t3 - 2.f * t2 + t1;
			const float f3 = t3 - t2;
			
			const float tempsp = next->speedtrack;
			const float p0sp = 0.5f * (tempsp - kprec->speedtrack);
			const float p1sp = 0.5f * (ksuivsuiv->speedtrack - current->speedtrack);
			c->speedtrack = f0 * current->speedtrack + f1 * tempsp + f2 * p0sp + f3 * p1sp;
			}

			interpolateLight(alight, lightprec, c);
			break;
		}
	}
	
	updateFadeOut(c, CKTrack, num, a, current != c->key || (resized && play));

	if(current != c->key) {
		c->key = current;
		c->changekey = true;
	}
	
	if(play) {
	
	c->flTime += fpscurr;
	CKTrack->currframe = (((float)(c->flTime)) / 1000.f) * ((float)(GetEndFrame() - GetStartFrame())) / (float)GetTimeKeyFramer();
	
	// TODO this assert fails if you pause the gametime before a cinematic starts and unpause after
	arx_assert(CKTrack->currframe >= 0);
	
	if(CKTrack->currframe > (float)CKTrack->endframe) {
		CKTrack->currframe = (float)CKTrack->startframe;
		c->key = NULL;
		arxtime.update();
		c->flTime = arxtime.now_f();
	}
	}
}

void PlayTrack(Cinematic * c)
{
	if(!CKTrack || !CKTrack->pause)
		return;

	CKTrack->pause = false;
	c->flTime = 0; 
}

float GetTimeKeyFramer()
{
	if(!CKTrack)
		return 0.f;

	float t = 0.f;
	CinematicKeyframe * k = CKTrack->key;
	int nb = CKTrack->nbkey - 1;

	while(nb--) {
		CinematicKeyframe * next = k + 1;
		t += ((float)(next->frame - k->frame)) / (CKTrack->fps * k->speedtrack);
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
