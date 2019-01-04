/*
 * Copyright 2015-2018 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GRAPHICS_EFFECTS_ROTATINGCONE_H
#define ARX_GRAPHICS_EFFECTS_ROTATINGCONE_H

#include "graphics/effects/SpellEffects.h"

class RotatingCone {
	
public:
	
	RotatingCone();
	
	void Init(float rbase, float rhaut, float hauteur);
	void Update(GameDuration timeDelta, Vec3f pos, float coneScale);
	void Render();
	
private:
	
	enum Constants {
		Def = 16,
		VertexCount = Def * 2 + 2,
		FaceCount = Def * 2 + 2,
	};
	
	Vec3f m_pos;
	GameDuration m_currdurationang;
	float m_ang;
	float m_coneScale;
	TextureContainer * m_tsouffle;
	
	Vec3f conevertex[VertexCount];
	TexturedVertexUntransformed coned3d[VertexCount];
	unsigned short coneind[VertexCount];
	
};

#endif // ARX_GRAPHICS_EFFECTS_ROTATINGCONE_H
