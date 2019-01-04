/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_CINEMATIC_CINEMATICFORMAT_H
#define ARX_CINEMATIC_CINEMATICFORMAT_H

#include "cinematic/Cinematic.h"
#include "graphics/GraphicsFormat.h"
#include "platform/Platform.h"


#pragma pack(push, 1)

static const s32 CINEMATIC_VERSION_1_75 = (1 << 16) | 75;
static const s32 CINEMATIC_VERSION_1_76 = (1 << 16) | 76;
static const s16 INTERP_NO_FADE = 2;

// Version 1.75 structures

struct CinematicLight_1_71 {
	
	SavedVec3 pos;
	f32 fallin;
	f32 fallout;
	SavedColor color;
	f32 intensity;
	f32 intensiternd;
	s32 prev; // ignored
	s32 next; // ignored
	
	operator CinematicLight() {
		CinematicLight l;
		l.pos = pos.toVec3();
		l.fallin = fallin;
		l.fallout = fallout;
		l.color = color;
		l.intensity = intensity;
		l.intensiternd = intensiternd;
		return l;
	}
	
};

// TODO macros
// Effects
const u32 CinematicFxMask = 0x000000ff;
#define FX_FADEIN  1
#define FX_FADEOUT 2
#define FX_BLUR    3
// Pre-effects
const u32 CinematicFxPreMask = 0x0000ff00;
#define FX_DREAM   (1 << 8)
// Post-effects
const u32 CinematicFxPostMask = 0x00ff0000;
#define FX_FLASH   (1 << 16)
#define FX_APPEAR  (2 << 16)
#define FX_APPEAR2 (3 << 16)
// All time
const u32 CinematicFxAllMask = 0xff000000;
#define FX_LIGHT   (1 << 24)

struct C_KEY_1_75 {
	s32 frame;
	s32 numbitmap;
	s32 fx; // associated fx
	s16 typeinterp;
	s16 force;
	SavedVec3 pos;
	f32 angz;
	s32 color;
	s32 colord;
	s32 colorf;
	s32 idsound;
	f32 speed;
	CinematicLight_1_71 light;
	SavedVec3 posgrille;
	f32 angzgrille;
	f32 speedtrack;
};

// Version 1.76 structures

struct C_KEY_1_76 {
	s32 frame;
	s32 numbitmap;
	s32 fx; // associated fx
	s16 typeinterp;
	s16 force;
	SavedVec3 pos;
	f32 angz;
	s32 color;
	s32 colord;
	s32 colorf;
	f32 speed;
	CinematicLight_1_71 light;
	SavedVec3 posgrille;
	f32 angzgrille;
	f32 speedtrack;
	s32 idsound[16];
};


struct SavedCinematicTrack {
	s32 startframe;
	s32 endframe;
	f32 currframe;
	f32 fps;
	s32 nbkey;
	s32 pause;
};


#pragma pack(pop)


#endif // ARX_CINEMATIC_CINEMATICFORMAT_H
