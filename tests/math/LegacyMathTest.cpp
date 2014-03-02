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

#include "LegacyMathTest.h"

#include "graphics/Math.h"

#include "AssertionTraits.h"

CPPUNIT_TEST_SUITE_REGISTRATION(LegacyMathTest);

void LegacyMathTest::quaternionTests() {
	std::vector<glm::quat> testQuats;
	
	// Identity (no rotation)
	testQuats.push_back(glm::quat( 1.f, 0.f, 0.f, 0.f));
	testQuats.push_back(glm::quat(-1.f, 0.f, 0.f, 0.f));
	
	// 180 degrees about x-axis
	testQuats.push_back(glm::quat(0.f,  1.f, 0.f, 0.f));
	testQuats.push_back(glm::quat(0.f, -1.f, 0.f, 0.f));
	
	//  180 degrees about y-axis
	testQuats.push_back(glm::quat(0.f, 0.f,  1.f, 0.f));
	testQuats.push_back(glm::quat(0.f, 0.f, -1.f, 0.f));
	
	// 180 degrees about z-axis
	testQuats.push_back(glm::quat(0.f, 0.f, 0.f,  1.f));
	testQuats.push_back(glm::quat(0.f, 0.f, 0.f, -1.f));
	
	std::vector<glm::quat>::iterator it;
	for(it = testQuats.begin(); it != testQuats.end(); ++it) {
		
		glm::quat A = *it;	
		glm::quat B = *it;
	
		CPPUNIT_ASSERT_EQUAL(A, B);
		
		glm::quat inverseA = A;
		Quat_Reverse(&inverseA);
		glm::quat inverseB = glm::gtc::quaternion::inverse(B);
		
		CPPUNIT_ASSERT_EQUAL(inverseA, inverseB);
	
		Vec3f vecA = TransformVertexQuat(A, Vec3f(1.f, 0.5f, 0.1f));
		Vec3f vecB = B * Vec3f(1.f, 0.5f, 0.1f);
		
		CPPUNIT_ASSERT_EQUAL(vecA, vecB);
	}
}

void LegacyMathTest::quatMuliplyTest() {
	glm::quat A = glm::quat(0.f,  1.f, 0.f, 0.f);
	glm::quat B = glm::quat(0.f,  0.f, 1.f, 0.f);
	
	CPPUNIT_ASSERT_EQUAL(A * B, Quat_Multiply(A, B));
}
