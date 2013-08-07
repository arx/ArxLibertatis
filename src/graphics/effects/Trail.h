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

#ifndef ARX_GRAPHICS_EFFECTS_TRAIL_H
#define ARX_GRAPHICS_EFFECTS_TRAIL_H

#include "graphics/BaseGraphicsTypes.h"
#include "graphics/Color.h"
#include "io/resource/ResourcePath.h"
#include "math/MathFwd.h"
#include "math/Vector3.h"
#include "platform/Flags.h"

class Trail {

private:

	short key;
	int duration;
	int currduration;
	int iNumThrow;

	struct T_RUBAN {
		int actif;
		Vec3f pos;
		int next;
	};
	T_RUBAN truban[2048];

	struct T_RUBAN_DEF {
		int first;
		int origin;
		float size;
		int dec;
		float r, g, b;
		float r2, g2, b2;
	};

	int nbrubandef;
	T_RUBAN_DEF trubandef[256];

	int GetFreeRuban(void);
	void AddRuban(int * f, int dec);
	void DrawRuban(int num, float size, int dec, float r, float g, float b, float r2, float g2, float b2);

public:

	void AddRubanDef(int origin, float size, int dec, float r, float g, float b, float r2, float g2, float b2);
	void Create(int numinteractive, int duration);

	void Update();
	void Render();
};

#endif // ARX_GRAPHICS_EFFECTS_TRAIL_H
