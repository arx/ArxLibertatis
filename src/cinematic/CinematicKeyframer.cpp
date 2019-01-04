/*
 * Copyright 2011-2019 Arx Libertatis Team (see the AUTHORS file)
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

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <limits>

#include <boost/foreach.hpp>

#include "cinematic/Cinematic.h"
#include "cinematic/CinematicTexture.h"
#include "cinematic/CinematicFormat.h"
#include "core/Config.h"
#include "core/Core.h"
#include "core/GameTime.h"

CinematicTrack * CKTrack;

CinematicTrack::CinematicTrack(int endframe_, float fps_)
	: endframe(endframe_)
	, fps(fps_)
{
	currframe = 0.f;
	pause = true;
}

void AllocTrack(int ef, float fps) {
	
	if(CKTrack)
		return;
	
	CKTrack = new CinematicTrack(ef, fps);
}

void DeleteTrack() {
	delete CKTrack;
	CKTrack = NULL;
}
 
static void UpDateKeyLight(CinematicKeyframe * kbase) {
	
	CinematicKeyframe * klightprev2, * klightnext2;
	
	CinematicKeyframe * klightprev = kbase;
	CinematicKeyframe * klightnext = kbase;
	
	// Look for the previous and next keyframes that change the light
	
	CinematicKeyframe * k = kbase;
	while(k != &CKTrack->key.front()) {
		k--;
		if((k->fx & CinematicFxAllMask) == FX_LIGHT) {
			klightprev = k;
			break;
		}
	}
	
	k = kbase;
	while(k != &CKTrack->key.back()) {
		k++;
		if((k->fx & CinematicFxAllMask) == FX_LIGHT) {
			klightnext = k;
			break;
		}
	}
	
	kbase->light.prev = klightprev;
	kbase->light.next = klightnext;
	
	if((kbase->fx & CinematicFxAllMask) == FX_LIGHT) {
		klightprev2 = klightnext2 = kbase;
	} else {
		kbase->light.intensity = -1.f;
		klightprev2 = klightprev;
		klightnext2 = klightnext;
	}
	
	k = kbase;
	while(k != &CKTrack->key.front()) {
		k--;
		if(klightprev == kbase) {
			k->light.intensity = -1.f;
		}
		k->light.next = klightnext2;
		if((k->fx & CinematicFxAllMask) == FX_LIGHT) {
			break;
		}
		k->light.prev = klightprev;
	}
	
	k = kbase;
	while(k != &CKTrack->key.back()) {
		k++;
		if(klightnext == kbase) {
			k->light.intensity = -1.f;
		}
		k->light.prev = klightprev2;
		if((k->fx & CinematicFxAllMask) == FX_LIGHT) {
			break;
		}
		k->light.next = klightnext;
	}
	
}

void UpDateAllKeyLight() {
	BOOST_FOREACH(CinematicKeyframe & key, CKTrack->key) {
		UpDateKeyLight(&key);
	}
}

struct HasFrame {
	
	explicit HasFrame(int frame) : m_frame(frame) { }
	
	bool operator()(const CinematicKeyframe & key) {
		return key.frame == m_frame;
	}
	
private:
	
	int m_frame;
	
};

struct AfterFrame {
	
	explicit AfterFrame(int frame) : m_frame(frame) { }
	
	bool operator()(const CinematicKeyframe & key) {
		return key.frame > m_frame;
	}
	
private:
	
	int m_frame;
	
};

void AddKeyLoad(const CinematicKeyframe & key) {
	
	if(!CKTrack || key.frame < 0 || key.frame > CKTrack->endframe) {
		return;
	}
	
	std::vector<CinematicKeyframe>::iterator i;
	i = std::find_if(CKTrack->key.begin(), CKTrack->key.end(), HasFrame(key.frame));
	if(i != CKTrack->key.end()) {
		*i = key;
		return;
	}
	
	i = std::find_if(CKTrack->key.begin(), CKTrack->key.end(), AfterFrame(key.frame));
	i = CKTrack->key.insert(i, key);
	
}

static CinematicKeyframe * GetKey(int frame) {
	
	if(!CKTrack || CKTrack->key.empty()) {
		return NULL;
	}
	
	std::vector<CinematicKeyframe>::iterator i;
	i = std::find_if(CKTrack->key.begin(), CKTrack->key.end(), AfterFrame(frame));
	
	if(i == CKTrack->key.begin()) {
		return NULL;
	}
	
	return &*(i - 1);
}

static float GetAngleInterpolation(float d, float e) {
	
	float da = e - d;
	if(glm::abs(da) > 180.f) {
		if(da > 0.f) {
			da -= 360.f;
		} else {
			da += 360.f;
		}
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
	float fFOV = glm::radians(69.75f);
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

static void updateFadeOut(Cinematic * c, CinematicKeyframe * current, float a,
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
	
	CinematicKeyframe * next = (current == &CKTrack->key.back()) ? current : current + 1;
	
	if(keyChanged) {
		c->fadeprev = getFadeOut(*c, *current, *current);
		if(current == &CKTrack->key.back() || next->numbitmap != current->numbitmap) {
			c->fadenext = c->fadeprev;
		} else {
			c->fadenext = getFadeOut(*c, *next, *next);
		}
		if(current->force) {
			CinematicKeyframe * prev = (current == &CKTrack->key.front()) ? current : current - 1;
			if(prev->typeinterp != INTERP_NO) {
				c->m_nextFadegrille  = getFadeOut(*c, *current, *prev);
			} else {
				c->m_nextFadegrille  = getFadeOut(*c, *current, *current);
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

#if defined(_MSC_VER) && _MSC_VER >= 1800 && _MSC_VER < 1900
/*
 * MSVC 2013 generates a MOVAPS instruction here with an argument that is not 16-byte alligned,
 * resulting in a crash.
 *  https://bugs.arx-libertatis.org/arx/issues/1097
 *
 * This has been fixed in MSVC 2015:
 *  https://connect.microsoft.com/VisualStudio/feedback/details/956733/
 */
#pragma optimize("", off)
#endif
static void interpolateLight(float alight, CinematicKeyframe * lightprec, Cinematic * c) {
	
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
	c->m_light.intensiternd = alight * lend.intensiternd + unmoinsalight * ldep.intensiternd;
	
}
#if defined(_MSC_VER) && _MSC_VER >= 1800 && _MSC_VER < 1900
#pragma optimize("", on)
#endif

void GereTrack(Cinematic * c, PlatformDuration frameDuration, bool resized, bool play) {
	
	if(!CKTrack || CKTrack->key.empty()) {
		return;
	}
	
	if(CKTrack->pause == play) {
		return;
	}
	
	CinematicKeyframe * current = GetKey(int(CKTrack->currframe));
	if(!current) {
		return;
	}
	
	CinematicKeyframe * next = (current == &CKTrack->key.back()) ? current : current + 1;
	
	float a = 1.f;
	if(next->frame != current->frame) {
		a = (CKTrack->currframe - float(current->frame)) / float(next->frame - current->frame);
	}
	
	c->a = 1.f - a;
	c->numbitmap = current->numbitmap;
	c->m_nextNumbitmap = next->numbitmap;
	c->ti = current->typeinterp;
	c->fx = current->fx;
	c->m_nextFx = next->fx;
	c->color = current->color;
	c->colord = current->colord;
	c->colorflash = current->colorf;
	c->speed = current->speed;
	c->idsound = current->idsound;
	c->force = current->force;
	
	CinematicKeyframe * light = current;
	if((current->fx & CinematicFxAllMask) != FX_LIGHT) {
		light = current->light.prev;
	}
	
	CinematicKeyframe * lightnext = current->light.next;
	c->m_lightd = lightnext->light;
	
	float alight = 0.f;
	if(light != lightnext) {
		alight = (CKTrack->currframe - float(light->frame)) / float(lightnext->frame - light->frame);
		if(alight > 1.f) {
			alight = 1.f;
		}
	} else if(current == &CKTrack->key.back()) {
		alight = 1.f;
	} else {
		// alight can't be used because it is not initialized
		c->m_lightd = current->light;
		light = current;
	}
	
	c->posgrille = current->posgrille;
	c->angzgrille = current->angzgrille;
	c->m_nextPosgrille = next->posgrille;
	c->m_nextAngzgrille = next->angzgrille;
	
	if(!play && (current->numbitmap < 0 || next->numbitmap < 0)) {
		return;
	}
	
	switch(current->typeinterp) {
		case INTERP_NO:
			c->m_pos = current->pos;
			c->angz = current->angz;
			c->m_nextPos = next->pos;
			c->m_nextAngz = next->angz;
			c->m_light = light->light;
			c->speedtrack = current->speedtrack;
			break;
		case INTERP_LINEAR:
			c->m_pos = next->pos * a + current->pos * (1.f - a);
			c->angz = current->angz + a * GetAngleInterpolation(current->angz, next->angz);
			c->speedtrack = a * next->speedtrack + (1.f - a) * current->speedtrack;

			interpolateLight(alight, light, c);
			break;
		case INTERP_BEZIER: {
			if(play)
				c->m_light = current->light;
			
			CinematicKeyframe * ksuivsuiv = (next == &CKTrack->key.back()) ? next : next + 1;
			CinematicKeyframe * kprec = (current == &CKTrack->key.front()) ? current : current - 1;
			
			const Vec3f prevPos = kprec->pos;
			const Vec3f currentPos = current->pos;
			const Vec3f nextPos = next->pos;
			const Vec3f next2Pos = ksuivsuiv->pos;
			
			c->m_pos = arx::catmullRom(prevPos, currentPos, nextPos, next2Pos, a);
			
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

			interpolateLight(alight, light, c);
			break;
		}
	}
	
	updateFadeOut(c, current, a, current != c->m_key || (resized && play));

	if(current != c->m_key) {
		c->m_key = current;
		c->changekey = true;
	}
	
	if(play) {
	
	c->flTime = c->flTime + frameDuration;
	CKTrack->currframe = toS(c->flTime) * float(CKTrack->endframe) / GetTimeKeyFramer();
	
	// TODO this assert fails if you pause the gametime before a cinematic starts and unpause after
	arx_assert(CKTrack->currframe >= 0);
	
	if(CKTrack->currframe > float(CKTrack->endframe)) {
		CKTrack->currframe = 0;
		c->m_key = NULL;
		c->flTime = 0;
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

float GetTimeKeyFramer() {
	
	if(!CKTrack) {
		return 0.f;
	}
	
	float t = 0.f;
	
	for(size_t i = 0; i + 1 < CKTrack->key.size(); i++) {
		CinematicKeyframe & k = CKTrack->key[i];
		CinematicKeyframe & next = CKTrack->key[i + 1];
		t += float(next.frame - k.frame) / (CKTrack->fps * k.speedtrack);
	}
	
	return t;
}

float GetTrackFPS() {
	
	if(!CKTrack) {
		return -1;
	}
	
	return CKTrack->fps;
}

void SetCurrFrame(int frame) {
	
	if(!CKTrack) {
		return;
	}
	
	CKTrack->currframe = float(frame);
}
