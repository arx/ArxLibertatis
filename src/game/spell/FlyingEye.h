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

#ifndef ARX_GAME_SPELL_FLYINGEYE_H
#define ARX_GAME_SPELL_FLYINGEYE_H

#include "graphics/BaseGraphicsTypes.h"
#include "math/Angle.h"

struct EYEBALL_DEF {
	
	long exist;
	Vec3f pos;
	Anglef angle;
	Vec3f size;
	float floating;
	
	EYEBALL_DEF()
		: exist(0)
		, pos(0.f)
		, size(0.f)
		, floating(0.f)
	{ }
	
};

extern float MagicSightFader;
extern EYEBALL_DEF eyeball;

void FlyingEye_Init();
void FlyingEye_Release();

void DrawMagicSightInterface();

void ARXDRAW_DrawEyeBall();

#endif // ARX_GAME_SPELL_FLYINGEYE_H
