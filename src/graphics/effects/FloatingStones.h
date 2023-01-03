/*
 * Copyright 2015-2022 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GRAPHICS_EFFECTS_FLOATINGSTONES_H
#define ARX_GRAPHICS_EFFECTS_FLOATINGSTONES_H

#include <type_traits>
#include <vector>

#include "graphics/effects/SpellEffects.h"
#include "math/Quantizer.h"


class FloatingStones {
	
public:
	
	void Init(float radius);
	void Update(ShortGameDuration timeDelta, Vec3f pos, bool addStones = true);
	void AddStone(const Vec3f & pos);
	void DrawStone();
	
private:
	
	struct Stone {
		
		Vec3f pos;
		float yvel;
		Anglef ang;
		ShortGameDuration duration;
		Anglef angvel;
		ShortGameDuration elapsed;
		Vec3f scale;
		short numstone;
		
	};
	
	static_assert(std::is_trivially_copyable_v<Stone>);
	
	std::vector<Stone> m_stones;
	ShortGameDuration m_timestone;
	float m_baseRadius = 0.f;
	math::Quantizer m_quantizer;
	
};

#endif // ARX_GRAPHICS_EFFECTS_FLOATINGSTONES_H
