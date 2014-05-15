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

#ifndef ARX_GRAPHICS_PARTICLE_PARTICLEPARAMS_H
#define ARX_GRAPHICS_PARTICLE_PARTICLEPARAMS_H

#include "graphics/Draw.h"
#include "math/Vector.h"

class ParticleParams {
	
public:
	
	Vec3f m_pos;
	Vec3f m_direction;
	Vec3f m_gravity;
	int   m_nbMax;
	int   m_freq;
	bool  m_rotationRandomDirection;
	bool  m_rotationRandomStart;
	float m_life;
	float m_lifeRandom;
	float m_angle;
	float m_speed;
	float m_speedRandom;
	float m_flash;
	float m_rotation;
	
	bool  m_texInfo;
	bool  m_texLoop;
	int   m_texNb;
	int   m_texTime;
	RenderMaterial::BlendType m_blendMode;
	char * m_texName;
	
	float m_startSize;
	float m_startSizeRandom;
	Color4f m_startColor;
	Color4f m_startColorRandom;
	
	float m_endSize;
	float m_endSizeRandom;
	Color4f m_endColor;
	Color4f m_endColorRandom;
	
	ParticleParams()
		: m_freq(-1)
		, m_texInfo(false)
	{}
};

#endif // ARX_GRAPHICS_PARTICLE_PARTICLEPARAMS_H
