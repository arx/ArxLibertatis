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

public:
	Trail(Vec3f & initialPosition);

	void SetNextPosition(Vec3f & nextPosition);

	void Update();
	void Render();

private:

	Vec3f m_nextPosition;

	int m_first;
	int m_origin;
	float m_size;
	int m_dec;
	float m_r, m_g, m_b;
	float m_r2, m_g2, m_b2;

	struct T_RUBAN {
		int actif;
		Vec3f pos;
		int next;
	};
	T_RUBAN truban[2048];

	int GetFreeRuban(void);
	void AddRuban(int * f, int dec);
	void DrawRuban(int num, float size, int dec, float r, float g, float b, float r2, float g2, float b2);

	void AddRubanDef(int origin, float size, int dec, float r, float g, float b, float r2, float g2, float b2);
	void Create(int numinteractive);
};

#endif // ARX_GRAPHICS_EFFECTS_TRAIL_H
