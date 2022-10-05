/*
 * Copyright 2011-2021 Arx Libertatis Team (see the AUTHORS file)
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
#include <memory>
#include <vector>

#include "core/TimeTypes.h"
#include "game/Camera.h"
#include "graphics/Color.h"
#include "math/RandomFlicker.h"
#include "math/Vector.h"
#include "platform/Platform.h"


struct CinematicKeyframe;
struct CinematicGrid;
class CinematicBitmap;

static inline constexpr int INTERP_NO = -1;
static inline constexpr int INTERP_BEZIER = 0;
static inline constexpr int INTERP_LINEAR = 1;

class CinematicLight {
	
public:
	
	Vec3f pos = Vec3f(0.f);
	float fallin = 100.f;
	float fallout = 200.f;
	Color3f color = Color3f::white * 255.f;
	float intensity = 1.f;
	float intensiternd = 0.2f;
	CinematicKeyframe * prev = nullptr;
	CinematicKeyframe * next = nullptr;
	
	CinematicLight() arx_noexcept_default
	
};

struct CinematicFadeOut {
	
	float top;
	float bottom;
	float left;
	float right;
	
	explicit constexpr CinematicFadeOut(float v = 0.f) noexcept : top(v), bottom(v), left(v), right(v) { }
	
};

class Cinematic {
	
public:
	
	Vec3f m_pos = Vec3f(0.f);
	float angz = 0.f;
	Vec3f m_nextPos = Vec3f(0.f); // in the case of a non-fade interpolation
	float m_nextAngz = 0.f;
	int numbitmap = -1;
	int m_nextNumbitmap = -1;
	float a = 0.f;
	int fx = -1;
	int m_nextFx = 0;
	bool changekey = true;
	CinematicKeyframe * m_key = nullptr;
	bool projectload = false;
	short ti = INTERP_BEZIER;
	short force = 0;
	Color color;
	Color colord;
	Color colorflash;
	float speed = 0.f;
	int idsound = -1;
	CinematicLight m_light;
	math::RandomFlicker flicker;
	CinematicLight m_lightd;
	math::RandomFlicker flickerd;
	Vec3f posgrille = Vec3f(0.f);
	float angzgrille = 0.f;
	CinematicFadeOut fadegrille;
	Vec3f m_nextPosgrille = Vec3f(0.f);
	float m_nextAngzgrille = 0.f;
	CinematicFadeOut m_nextFadegrille;
	float speedtrack = 0.f;
	PlatformDuration flTime;
	std::vector<std::unique_ptr<CinematicBitmap>> m_bitmaps;
	
	CinematicFadeOut fadeprev;
	CinematicFadeOut fadenext;
	
	Vec2i cinRenderSize;
	
	explicit Cinematic(Vec2i size) noexcept;
	~Cinematic();
	
	void OneTimeSceneReInit();
	void Render(PlatformDuration frameDuration);
	
	void DeleteAllBitmap();
	
private:
	
	Camera m_camera;
	
};

void DrawGrille(CinematicBitmap * bitmap, Color col, int fx, CinematicLight * light,
                const Vec3f & pos, float angle, const CinematicFadeOut & fade);

#endif // ARX_CINEMATIC_CINEMATIC_H
