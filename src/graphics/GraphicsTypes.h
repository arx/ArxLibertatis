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

#include "physics/Physics.h"

#include "platform/Platform.h"

#include "util/Flags.h"
#include "util/HandleContainer.h"


struct EERIE_3DOBJ;
class TextureContainer;
class Entity;
struct EERIE_LIGHT;

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
	RoomHandle room = RoomHandle(0);
	unsigned short uslInd[4] = { 0, 0, 0, 0 };
	
	constexpr EERIEPOLY() arx_noexcept_default
	
};

#define IOPOLYVERT 3
struct EERIE_FACE {
	
	PolyType facetype;
	short texid = 0; //!< index into the objects texture list
	VertexId vid[IOPOLYVERT] = { VertexId(0), VertexId(0), VertexId(0) };
	float u[IOPOLYVERT] = { 0.f, 0.f, 0.f };
	float v[IOPOLYVERT] = { 0.f, 0.f, 0.f };
	
	float transval = 0.f;
	Vec3f norm = Vec3f(0.f);
	
	constexpr EERIE_FACE() arx_noexcept_default
	
};

struct EERIE_ACTIONLIST {
	
	std::string name;
	VertexId idx;
	
};

struct EERIE_LINKED {
	
	VertexGroupId lgroup;
	VertexId lidx;
	VertexId lidx2;
	EERIE_3DOBJ * obj = nullptr;
	Entity * io = nullptr;
	
};

struct EERIE_SELECTIONS {
	
	std::string name;
	std::vector<VertexId> selected;
	
};

struct EERIE_FASTACCESS {
	
	VertexId view_attach;
	VertexId primary_attach;
	VertexId left_attach;
	VertexId weapon_attach;
	VertexId fire;
	VertexId head_group_origin;
	VertexGroupId head_group;
	ObjSelection sel_head;
	ObjSelection sel_chest;
	ObjSelection sel_leggings;
	
};

struct EERIE_3DOBJ {
	
	res::path file;
	VertexId origin = VertexId(0);
	util::HandleVector<VertexId, Vec3f> vertexlocal;
	util::HandleVector<VertexId, EERIE_VERTEX> vertexlist;
	util::HandleVector<VertexId, EERIE_VERTEX> vertexWorldPositions;
	util::HandleVector<VertexId, Vec4f> vertexClipPositions;
	util::HandleVector<VertexId, ColorRGBA> vertexColors;
	
	std::vector<EERIE_FACE> facelist;
	std::vector<VertexGroup> grouplist;
	std::vector<EERIE_ACTIONLIST> actionlist;
	std::vector<EERIE_SELECTIONS> selections;
	std::vector<TextureContainer *> texturecontainer;
	
	std::vector<res::path> originaltextures;
	std::vector<EERIE_LINKED> linked;
	
	std::unique_ptr<PHYSICS_BOX_DATA> pbox;
	EERIE_FASTACCESS fastaccess;
	
	std::unique_ptr<Skeleton> m_skeleton;
	std::vector<std::vector<VertexId>> m_boneVertices; // TODO use u16 here ?
	
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
