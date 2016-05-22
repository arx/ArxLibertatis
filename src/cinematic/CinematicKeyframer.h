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

#ifndef ARX_CINEMATIC_CINEMATICKEYFRAMER_H
#define ARX_CINEMATIC_CINEMATICKEYFRAMER_H

#include "cinematic/Cinematic.h" // for CinematicLight
#include "math/Vector.h"

static const int INTERP_NO = -1;
static const int INTERP_BEZIER = 0;
static const int INTERP_LINEAR = 1;

struct CinematicKeyframe {
	
	int frame;
	int numbitmap;
	int fx; // associated fx
	short typeinterp;
	short force;
	Vec3f pos;
	float angz;
	Color color;
	Color colord;
	Color colorf;
	float speed;
	CinematicLight light;
	Vec3f posgrille;
	float angzgrille;
	float speedtrack;
	int idsound;
};

struct CinematicTrack {
	int startframe;
	int endframe;
	float currframe;
	float fps;
	int nbkey;
	int pause;
	CinematicKeyframe * key;
};

bool DeleteTrack();
bool AllocTrack(int sf, int ef, float fps);
bool AddKey(const CinematicKeyframe & key);
bool AddKeyLoad(const CinematicKeyframe & key);
void GereTrack(Cinematic * c, float fpscurr, bool resized, bool play);

void PlayTrack(Cinematic * c);
int GetStartFrame();
int GetEndFrame();
void SetCurrFrame(int frame);
float GetTrackFPS();

CinematicKeyframe * GetKey(int f, int * num);
CinematicKeyframe * SearchKey(int f, int * num);

float GetTimeKeyFramer();
void UpDateAllKeyLight();

#endif // ARX_CINEMATIC_CINEMATICKEYFRAMER_H
