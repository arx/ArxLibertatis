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

#include <sstream>
#include <iomanip>
#include <cppunit/TestAssert.h>
#include <glm/gtx/epsilon.hpp>

#include "GraphicsUtilityTest.h"

#include "graphics/Math.h"

CPPUNIT_TEST_SUITE_REGISTRATION(GraphicsUtilityTest);

#include <glm/gtc/matrix_access.hpp>

namespace CppUnit {
	
	template<>
	struct assertion_traits<glm::mat4x4> {
		static bool equal(const glm::mat4x4 & mat, const glm::mat4x4 & other) {
			
			for(int i = 0; i < 4; i++) {
				if(! glm::all(glm::gtx::epsilon::equalEpsilon(glm::row(mat, i), glm::row(other, i), 0.001f))) {
					return false;
				}
			}
			return true;
		}
		
		static std::string toString(const glm::mat4x4 &m) {
			OStringStream ost;
			ost << std::endl << std::fixed << std::setprecision(5);
			
			for(int i = 0; i < 4; i++) {
				for(int u = 0; u < 4; u++) {
					// Print columns as rows !
					ost << std::setw(14) << glm::column(m, i)[u];
				}
				ost << std::endl;
			}
			
			return ost.str();
		}
	};
}

glm::mat4x4 fromEerieMatrix(const EERIEMATRIX & other) {
	glm::mat4x4 result;
	
	result[0][0] = other._11;
	result[0][1] = other._12;
	result[0][2] = other._13;
	result[0][3] = other._14;
	
	result[1][0] = other._21;
	result[1][1] = other._22;
	result[1][2] = other._23;
	result[1][3] = other._24;
	
	result[2][0] = other._31;
	result[2][1] = other._32;
	result[2][2] = other._33;
	result[2][3] = other._34;
	
	result[3][0] = other._41;
	result[3][1] = other._42;
	result[3][2] = other._43;
	result[3][3] = other._44;
	
	return result;
}

inline void checkMatrix(Vec3f pos, Anglef angle, glm::mat4x4 expected) {
	EERIE_TRANSFORM transform;
	transform.pos = pos;
	transform.updateFromAngle(angle);
	
	EERIEMATRIX matrix;
	Util_SetViewMatrix(matrix, transform);
	
	glm::mat4x4 result = fromEerieMatrix(matrix);
	
	std::ostringstream ost;
	ost << std::endl << std::fixed << std::setprecision(3);
	ost << "Input:" << std::endl;
	ost << std::setw(8) << pos.x;
	ost << std::setw(8) << pos.y;
	ost << std::setw(8) << pos.z;
	ost << std::endl;
	ost << std::setw(8) << angle.getYaw();
	ost << std::setw(8) << angle.getPitch();
	ost << std::setw(8) << angle.getRoll();
	ost << std::endl;
	
	CPPUNIT_ASSERT_EQUAL_MESSAGE(ost.str(), expected, result);
}

void GraphicsUtilityTest::front() {
	checkMatrix(
		Vec3f(0.f, 0.f, 0.f),
		Anglef(0.f, 0.f, 0.f),
		glm::mat4x4(
		 1, 0, 0, 0,
		 0, 1, 0, 0,
		 0, 0, 1, 0,
		 0, 0, 0, 1)
	);
}

void GraphicsUtilityTest::back() {
	checkMatrix(
		Vec3f(0.f, 0.f, 0.f),
		Anglef(180.f, 180.f, 0.f),
		glm::mat4x4(
		 1, 0, 0, 0,
		 0, 1, 0, 0,
		 0, 0, 1, 0,
		 0, 0, 0, 1)
	);
}

void GraphicsUtilityTest::translation()
{
	checkMatrix(
		Vec3f(10.f, 200.f, 3000.f),
		Anglef(0.f, 0.f, 0.f),
		glm::mat4x4(
		 1, 0, 0, 0,
		 0, 1, 0, 0,
		 0, 0, 1, 0,
		 -10, 200, -3000, 1)
	);
}

void GraphicsUtilityTest::testMatrix1() {
	checkMatrix(
		Vec3f(0.f, 0.f, 0.f),
		Anglef(90.f, 0.f, 0.f),
		glm::mat4x4(
		 1, 0, 0, 0,
		 0, 0,-1, 0,
		 0, 1, 0, 0,
		 0, 0, 0, 1)
	);
}

void GraphicsUtilityTest::testMatrix2() {
	checkMatrix(
		Vec3f(0.f, 0.f, 0.f),
		Anglef(0.f, 90.f, 0.f),
		glm::mat4x4(
		 0, 0,-1, 0,
		 0, 1, 0, 0,
		 1, 0, 0, 0,
		 0, 0, 0, 1)
	);
}

void GraphicsUtilityTest::testMatrix3() {
	checkMatrix(
		Vec3f(0.f, 0.f, 0.f),
		Anglef(0.f, 0.f, 90.f),
		glm::mat4x4(
		 1, 0, 0, 0,
		 0, 1, 0, 0,
		 0, 0, 1, 0,
		 0, 0, 0, 1)
	);
	
	checkMatrix(
		Vec3f(0.f, 0.f, 0.f),
		Anglef(0.f, 0.f, -180.f),
		glm::mat4x4(
		 1, 0, 0, 0,
		 0, 1, 0, 0,
		 0, 0, 1, 0,
		 0, 0, 0, 1)
	);
}

void GraphicsUtilityTest::testMatrix4() {
	checkMatrix(
		Vec3f(0.f, 0.f, 0.f),
		Anglef(90.f, 90.f, 0.f),
		glm::mat4x4(
		 1, 0, 0, 0,
		 0, 0,-1, 0,
		 0, 1, 0, 0,
		 0, 0, 0, 1)
	);
}

void GraphicsUtilityTest::testMatrix5() {
	checkMatrix(
		Vec3f(0.f, 0.f, 0.f),
		Anglef(-90.f, -90.f, 0.f),
		glm::mat4x4(
		-1, 0, 0, 0,
		 0, 0, 1, 0,
		 0, 1, 0, 0,
		 0, 0, 0, 1)
	);
}

void GraphicsUtilityTest::testMatrix6() {
	checkMatrix(
		Vec3f(1.f, 0.f, 0.f),
		Anglef(90.f, 0.f, 0.f),
		glm::mat4x4(
		 1, 0, 0, 0,
		 0, 0,-1, 0,
		 0, 1, 0, 0,
		-1, 0, 0, 1)
	);
}

void GraphicsUtilityTest::testMatrix7() {
	checkMatrix(
		Vec3f(10.f, 200.f, 3000.f),
		Anglef(45.f, -45.f, 0.f),
		glm::mat4x4(
		    0.70711,     0.5    ,     0.5    , 0,
		    0      ,     0.70711,    -0.70711, 0,
		   -0.70711,     0.5    ,     0.5    , 0,
		 2114.24927, -1363.57849, -1646.42126, 1)
	);
}
