/*
 * Copyright 2011-2021 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_ANIMATION_SKELETON_H
#define ARX_ANIMATION_SKELETON_H

#include <string>
#include <vector>

#include <glm/gtc/quaternion.hpp>

#include "graphics/BaseGraphicsTypes.h"
#include "math/Types.h"
#include "math/Angle.h"
#include "platform/Platform.h"
#include "util/HandleContainer.h"


struct VertexGroup {
	
	std::string name;
	VertexId origin = VertexId(0);
	std::vector<VertexId> indexes;
	float m_blobShadowSize = 0.f;
	
};

struct BoneTransform {
	
	glm::quat quat = quat_identity();
	Vec3f trans = Vec3f(0.f);
	Vec3f scale = Vec3f(0.f); // TODO should this be 1.f?
	
	constexpr BoneTransform() arx_noexcept_default
	
	[[nodiscard]] constexpr Vec3f operator()(const Vec3f & vec) const noexcept {
		return trans + ( quat * vec ) * scale;
	}
	
};

struct Bone {
	
	long father = 0;
	
	BoneTransform anim;
	BoneTransform last;
	BoneTransform init;
	
	Vec3f transinit_global = Vec3f(0.f);
	
	constexpr Bone() arx_noexcept_default
	
};

struct Skeleton {
	
	util::HandleVector<VertexGroupId, Bone> bones;
	
};

#endif // ARX_ANIMATION_SKELETON_H
