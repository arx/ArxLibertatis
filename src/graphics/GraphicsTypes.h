/*
 * Copyright 2011-2022 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GRAPHICS_GRAPHICSTYPES_H
#define ARX_GRAPHICS_GRAPHICSTYPES_H

#include <cmath>
#include <memory>
#include <vector>

#include <boost/array.hpp>

#include "animation/Skeleton.h"
#include "audio/AudioTypes.h"
#include "graphics/BaseGraphicsTypes.h"
#include "graphics/Color.h"
#include "graphics/Vertex.h"

#include "io/resource/ResourcePath.h"

#include "math/Vector.h"
#include "math/Angle.h"

#include "platform/Platform.h"

#include "util/Flags.h"


struct EERIE_3DOBJ;
class TextureContainer;
class Entity;
struct EERIE_LIGHT;
struct PHYSICS_BOX_DATA;

typedef HandleType<struct ActionPointTag,  s32,   -1> ActionPoint;
typedef HandleType<struct ObjSelectionTag, long,  -1> ObjSelection;
typedef HandleType<struct ObjVertGroupTag, short, -1> ObjVertGroup;
typedef HandleType<struct ObjVertHandleTag, s32,  -1> ObjVertHandle;

struct EERIE_TRI {
	
	Vec3f v[3] = { Vec3f(0.f), Vec3f(0.f), Vec3f(0.f) };
	
	constexpr EERIE_TRI() arx_noexcept_default
	
};

enum PolyTypeFlag {
	POLY_NO_SHADOW    = 1 << 0,
	POLY_DOUBLESIDED  = 1 << 1,
	POLY_TRANS        = 1 << 2,
	POLY_WATER        = 1 << 3,
	POLY_GLOW         = 1 << 4,
	POLY_IGNORE       = 1 << 5,
	POLY_QUAD         = 1 << 6,
	POLY_TILED        = 1 << 7,
	POLY_METAL        = 1 << 8,
	POLY_HIDE         = 1 << 9,
	POLY_STONE        = 1 << 10,
	POLY_WOOD         = 1 << 11,
	POLY_GRAVEL       = 1 << 12,
	POLY_EARTH        = 1 << 13,
	POLY_NOCOL        = 1 << 14,
	POLY_LAVA         = 1 << 15,
	POLY_CLIMB        = 1 << 16,
	POLY_FALL         = 1 << 17,
	POLY_NOPATH       = 1 << 18,
	POLY_NODRAW       = 1 << 19,
	POLY_PRECISE_PATH = 1 << 20,
	POLY_NO_CLIMB     = 1 << 21,
	POLY_ANGULAR      = 1 << 22,
	POLY_ANGULAR_IDX0 = 1 << 23,
	POLY_ANGULAR_IDX1 = 1 << 24,
	POLY_ANGULAR_IDX2 = 1 << 25,
	POLY_ANGULAR_IDX3 = 1 << 26,
	POLY_LATE_MIP     = 1 << 27
};
DECLARE_FLAGS(PolyTypeFlag, PolyType)
DECLARE_FLAGS_OPERATORS(PolyType)

struct EERIEPOLY {
	
	PolyType type;
	Vec3f min = Vec3f(0.f);
	Vec3f max = Vec3f(0.f);
	Vec3f norm = Vec3f(0.f);
	Vec3f norm2 = Vec3f(0.f);
	TexturedVertex v[4];
	ColorRGBA color[4] = { };
	Vec3f nrml[4] = { Vec3f(0.f), Vec3f(0.f), Vec3f(0.f), Vec3f(0.f) };
	TextureContainer * tex = nullptr;
	Vec3f center = Vec3f(0.f);
	float transval = 0.f;
	float area = 0.f;
	short room = 0;
	unsigned short uslInd[4] = { 0, 0, 0, 0 };
	
	constexpr EERIEPOLY() arx_noexcept_default
	
};

#define IOPOLYVERT 3
struct EERIE_FACE {
	
	PolyType facetype;
	short texid = 0; //!< index into the objects texture list
	unsigned short vid[IOPOLYVERT] = { 0, 0, 0 };
	float u[IOPOLYVERT] = { 0.f, 0.f, 0.f };
	float v[IOPOLYVERT] = { 0.f, 0.f, 0.f };
	
	float transval = 0.f;
	Vec3f norm = Vec3f(0.f);
	
	constexpr EERIE_FACE() arx_noexcept_default
	
};

struct EERIE_ACTIONLIST {
	
	std::string name;
	ActionPoint idx;
	
};

struct EERIE_LINKED {
	
	ObjVertGroup lgroup;
	ActionPoint lidx;
	ActionPoint lidx2;
	EERIE_3DOBJ * obj = nullptr;
	Entity * io = nullptr;
	
};

struct EERIE_SELECTIONS {
	
	std::string name;
	std::vector<size_t> selected;
	
};

struct EERIE_FASTACCESS {
	
	ActionPoint view_attach;
	ActionPoint primary_attach;
	ActionPoint left_attach;
	ActionPoint weapon_attach;
	ActionPoint secondary_attach;
	ObjVertHandle head_group_origin;
	ObjVertGroup head_group;
	ActionPoint fire;
	ObjSelection sel_head;
	ObjSelection sel_chest;
	ObjSelection sel_leggings;
	
};

struct EERIE_3DOBJ {
	
	void clear();
	
	~EERIE_3DOBJ();
	
	res::path file;
	size_t origin = 0;
	std::vector<Vec3f> vertexlocal;
	std::vector<EERIE_VERTEX> vertexlist;
	std::vector<EERIE_VERTEX> vertexWorldPositions;
	std::vector<Vec4f> vertexClipPositions;
	std::vector<ColorRGBA> vertexColors;

	std::vector<EERIE_FACE> facelist;
	std::vector<VertexGroup> grouplist;
	std::vector<EERIE_ACTIONLIST> actionlist;
	std::vector<EERIE_SELECTIONS> selections;
	std::vector<TextureContainer *> texturecontainer;

	std::vector<res::path> originaltextures;
	std::vector<EERIE_LINKED> linked;
	
	PHYSICS_BOX_DATA * pbox = nullptr;
	EERIE_FASTACCESS fastaccess;
	
	Skeleton * m_skeleton = nullptr;
	std::vector<std::vector<u32>> m_boneVertices; // TODO use u16 here ?
	
};

struct Plane {
	
	Vec3f normal = Vec3f(0.f);
	float offset = 0.f; // dist to origin
	
	constexpr Plane() arx_noexcept_default
	
};

struct EERIE_FRUSTRUM {
	
	Plane plane[4];
	
};

#endif // ARX_GRAPHICS_GRAPHICSTYPES_H
