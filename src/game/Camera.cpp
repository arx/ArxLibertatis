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

#include "game/Camera.h"

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>

void EERIE_TRANSFORM::updateFromAngle(const Anglef &angle) {
	float yaw, pitch, roll;
	yaw = radians(angle.getYaw());
	xcos = std::cos(yaw);
	xsin = std::sin(yaw);
	pitch = radians(angle.getPitch());
	ycos = std::cos(pitch);
	ysin = std::sin(pitch);
	roll = radians(angle.getRoll());
	zcos = std::cos(roll);
	zsin = std::sin(roll);
	
	// 0.9.4.5 and older have a reversed sign in glm::eulerAngleY()
#if GLM_VERSION_MAJOR == 0 \
	&& (GLM_VERSION_MINOR < 9 || (GLM_VERSION_MINOR == 9 \
		&& (GLM_VERSION_PATCH < 4 || (GLM_VERSION_PATCH == 4 \
			&& GLM_VERSION_REVISION < 6 \
		)) \
	))
	pitch = -pitch;
#endif
	
	glm::mat4 translation = glm::translate(-pos);
	glm::mat4 rotateX = glm::eulerAngleX(yaw);
	glm::mat4 rotateY = glm::eulerAngleY(pitch);
	glm::mat4 rotateZ = glm::eulerAngleZ(-roll);
	worldToView = rotateZ * rotateX * rotateY * translation;
}
