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

#ifndef ARX_GRAPHICS_DATA_CINEMATICTEXTURE_H
#define ARX_GRAPHICS_DATA_CINEMATICTEXTURE_H

#include <vector>

#include "math/Types.h"
#include "math/Vector.h"

class Texture2D;
namespace res { class path; }

// TODO better name
struct C_INDEXED {
	int bitmapdepx;
	int bitmapdepy;
	int bitmapw;
	int bitmaph;
	int nbvertexs;
	Texture2D * tex;
	int startind;
	int nbind;
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
};

struct CinematicGrid {
	int nbvertexs;
	int nbfaces;
	int nbinds;
	int nbindsmalloc;
	int nbuvs;
	int nbuvsmalloc;
	Vec3f * vertexs;
	C_UV * uvs;
	C_IND * inds;
	std::vector<C_INDEXED> mats;
	float dx;
	float dy;
	int nbx;
	int nby;
	int echelle;
};

class CinematicBitmap {
public:
	~CinematicBitmap();

public:
	int w, h;
	int nbx, nby;
	CinematicGrid grid;
	int dreaming;
};

CinematicBitmap * CreateCinematicBitmap(const res::path & path, int scale);

#endif // ARX_GRAPHICS_DATA_CINEMATICTEXTURE_H
