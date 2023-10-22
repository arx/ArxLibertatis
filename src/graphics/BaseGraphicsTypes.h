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

#ifndef ARX_GRAPHICS_BASEGRAPHICSTYPES_H
#define ARX_GRAPHICS_BASEGRAPHICSTYPES_H

#include "math/Rectangle.h"
#include "math/Vector.h"
#include "platform/Platform.h"
#include "util/HandleType.h"


typedef util::HandleType<struct ObjSelectionTag, u16> VertexSelectionId;
typedef util::HandleType<struct ObjVertGroupTag, u16> VertexGroupId;
typedef util::HandleType<struct VertexIdTag, u16> VertexId;
typedef util::HandleType<struct MaterialIdTag, u16> MaterialId;
typedef util::HandleType<struct RoomHandleTag, u32> RoomHandle;
typedef util::HandleType<struct PortalHandleTag, u32> PortalHandle;
typedef util::HandleType<struct AreaIdTag, u32> AreaId;
typedef util::HandleType<struct MapLevelTag, u32> MapLevel;
typedef util::HandleType<struct FogHandleTag, u32> FogHandle;

struct Cylinder {
	
	Vec3f origin = Vec3f(0.f);
	float radius = 0.f;
	float height = 0.f;
	
	constexpr Cylinder() arx_noexcept_default
	
	constexpr Cylinder(const Vec3f & origin_, float radius_, float height_) noexcept
		: origin(origin_)
		, radius(radius_)
		, height(height_)
	{ }
	
};

struct Sphere {
	
	Vec3f origin = Vec3f(0.f);
	float radius = 0.f;
	
	constexpr Sphere() arx_noexcept_default
	
	constexpr Sphere(const Vec3f & origin_, float radius_) noexcept
		: origin(origin_)
		, radius(radius_)
	{ }
	
	[[nodiscard]] bool contains(const Vec3f & pos) const noexcept {
		return closerThan(pos, origin, radius);
	}
	
};

struct EERIE_2D_BBOX {
	
	Vec2f min = Vec2f(0.f);
	Vec2f max = Vec2f(0.f);
	
	constexpr EERIE_2D_BBOX() arx_noexcept_default
	
	constexpr void reset() noexcept {
		min = Vec2f(32000);
		max = Vec2f(-32000);
	}
	
	constexpr void add(const Vec3f & pos) noexcept {
		min = glm::min(min, Vec2f(pos));
		max = glm::max(max, Vec2f(pos));
	}
	
	[[nodiscard]] constexpr bool valid() const noexcept {
		return (min.x <= max.x && min.y <= max.y);
	}
	
	[[nodiscard]] constexpr Rectf toRect() const noexcept {
		return Rectf(min.x, min.y, max.x, max.y);
	}
	
};

struct EERIE_3D_BBOX {
	
	Vec3f min = Vec3f(0.f);
	Vec3f max = Vec3f(0.f);
	
	constexpr EERIE_3D_BBOX() arx_noexcept_default
	
	constexpr EERIE_3D_BBOX(const Vec3f & min_, const Vec3f & max_) noexcept : min(min_), max(max_) { }
	
	constexpr void reset() noexcept {
		min = Vec3f(99999999.f);
		max = Vec3f(-99999999.f);
	}
	
	constexpr void add(const Vec3f & pos) noexcept {
		min = glm::min(min, pos);
		max = glm::max(max, pos);
	}
	
	[[nodiscard]] constexpr bool valid() const noexcept {
		return (min.x <= max.x && min.y <= max.y && min.z <= max.z);
	}
	
};

#endif // ARX_GRAPHICS_BASEGRAPHICSTYPES_H
