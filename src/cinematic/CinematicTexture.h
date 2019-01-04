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

#ifndef ARX_CINEMATIC_CINEMATICTEXTURE_H
#define ARX_CINEMATIC_CINEMATICTEXTURE_H

#include <vector>

#include <stddef.h>

#include "math/Types.h"
#include "math/Vector.h"

class Texture;
namespace res { class path; }

// TODO better name
struct C_INDEXED {
	
	Vec2i bitmapdep;
	Vec2i bitmap;
	int nbvertexs;
	Texture * tex;
	int startind;
	int nbind;
	
	C_INDEXED()
		: bitmapdep(0)
		, bitmap(0)
		, nbvertexs(0)
		, tex(NULL)
		, startind(0)
		, nbind(0)
	{ }
	
};

// TODO better name
struct C_IND {
	unsigned short i1;
	unsigned short i2;
	unsigned short i3;
};

struct C_UV {
	
	Vec2f uv;
	int indvertex;
	
	C_UV()
		: uv(0.f)
		, indvertex(0)
	{ }
	
};

struct CinematicGrid {
	
	size_t m_nbvertexs;
	std::vector<Vec2f> m_vertexs;
	std::vector<C_UV> m_uvs;
	std::vector<C_IND> m_inds;
	std::vector<C_INDEXED> m_mats;
	Vec2i m_count;
	int m_scale;
	
	CinematicGrid()
		: m_nbvertexs(0)
		, m_count(0)
		, m_scale(0)
	{ }
	
	bool AllocGrille(Vec2i nb, Vec2f t, Vec2f d, int scale);
	void FreeGrille();
	
	void AddQuadUVs(Vec2i depc, Vec2i tc, Vec2i bitmappos, Vec2i bitmapw, Texture * tex);
	void ReajustUV();
	
private:
	
	void GetIndNumCube(int cx, int cy, int * i1, int * i2, int * i3, int * i4);
	size_t AddMaterial(Texture * tex);
	void AddPoly(size_t matIdx, int i0, int i1, int i2);
	
};

class CinematicBitmap {
	
public:
	
	CinematicBitmap()
		: m_size(0)
		, m_count(0)
	{ }
	
	~CinematicBitmap();
	
	Vec2i m_size;
	Vec2i m_count;
	CinematicGrid grid;
	
};

CinematicBitmap * CreateCinematicBitmap(const res::path & path, int scale);

#endif // ARX_CINEMATIC_CINEMATICTEXTURE_H
