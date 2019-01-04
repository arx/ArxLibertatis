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

#ifndef ARX_CINEMATIC_CINEMATIC_H
#define ARX_CINEMATIC_CINEMATIC_H

#include <stddef.h>
#include <vector>

#include "core/TimeTypes.h"
#include "game/Camera.h"
#include "graphics/Color.h"
#include "math/RandomFlicker.h"
#include "math/Vector.h"
#include "platform/Alignment.h"

struct CinematicKeyframe;
struct CinematicGrid;
class CinematicBitmap;

class CinematicLight {
	
public:
	
	Vec3f pos;
	float fallin;
	float fallout;
	Color3f color;
	float intensity;
	float intensiternd;
	CinematicKeyframe * prev;
	CinematicKeyframe * next;
	
	CinematicLight()
		: pos(0.f)
		, fallin(100.f)
		, fallout(200.f)
		, color(Color3f::white * 255.f)
		, intensity(1.f)
		, intensiternd(0.2f)
		, prev(NULL)
		, next(NULL)
	{}
};

struct CinematicFadeOut {
	float top;
	float bottom;
	float left;
	float right;
	explicit CinematicFadeOut(float v = 0.f) : top(v), bottom(v), left(v), right(v) { }
};

class Cinematic {
	
public:
	
	Vec3f m_pos;
	float angz;
	Vec3f m_nextPos; // in the case of a non-fade interpolation
	float m_nextAngz;
	int numbitmap;
	int m_nextNumbitmap;
	float a;
	int fx;
	int m_nextFx;
	bool changekey;
	CinematicKeyframe * m_key;
	bool projectload;
	short ti;
	short force;
	Color color;
	Color colord;
	Color colorflash;
	float speed;
	int idsound;
	CinematicLight m_light;
	math::RandomFlicker flicker;
	CinematicLight m_lightd;
	math::RandomFlicker flickerd;
	Vec3f posgrille;
	float angzgrille;
	CinematicFadeOut fadegrille;
	Vec3f m_nextPosgrille;
	float m_nextAngzgrille;
	CinematicFadeOut m_nextFadegrille;
	float speedtrack;
	PlatformDuration flTime;
	std::vector<CinematicBitmap *> m_bitmaps;
	
	CinematicFadeOut fadeprev;
	CinematicFadeOut fadenext;
	
	Vec2i cinRenderSize;
	
	explicit Cinematic(Vec2i size);
	~Cinematic();
	
	void OneTimeSceneReInit();
	void Render(PlatformDuration frameDuration);
	
	void DeleteAllBitmap();
	
private:
	
	Camera m_camera;
	
public:
	
	ARX_USE_ALIGNED_NEW(Cinematic) // for m_camera
	
};

void DrawGrille(CinematicBitmap * bitmap, Color col, int fx, CinematicLight * light,
                const Vec3f & pos, float angle, const CinematicFadeOut & fade);

ARX_USE_ALIGNED_ALLOCATOR(Cinematic) // for m_camera

#endif // ARX_CINEMATIC_CINEMATIC_H
