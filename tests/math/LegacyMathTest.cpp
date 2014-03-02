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

struct TestRotation {
	glm::quat quat;
	Anglef angle;
	
	TestRotation(glm::quat quat, Anglef angle)
		: quat(quat)
		, angle(angle)
	{}
};

std::vector<TestRotation> rotations;

static void addTestData(glm::quat quat, Anglef angle) {
	rotations.push_back(TestRotation(quat, angle));
}

void LegacyMathTest::setUp() {
	// Data from: http://www.euclideanspace.com/maths/geometry/rotations/conversions/eulerToQuaternion/steps/index.htm
	
	// Identity (no rotation)
	addTestData(glm::quat(    1.f,    0.0f,    0.0f,    0.0f), Anglef(  0.f,   0.f,   0.f));
	// 90 degrees about y axis
	addTestData(glm::quat(0.7071f,    0.0f, 0.7071f,    0.0f), Anglef( 90.f,   0.f,   0.f));
	// 180 degrees about y axis
	addTestData(glm::quat(   0.0f,    0.0f,     1.f,    0.0f), Anglef(180.f,   0.f,   0.f));
	// 270 degrees about y axis
	addTestData(glm::quat(0.7071f,    0.0f,-0.7071f,    0.0f), Anglef(-90.f,   0.f,   0.f));
	
	addTestData(glm::quat(0.7071f,    0.0f,    0.0f, 0.7071f), Anglef(  0.f,  90.f,   0.f));
	addTestData(glm::quat(   0.5f,    0.5f,    0.5f,    0.5f), Anglef( 90.f,  90.f,   0.f));
	addTestData(glm::quat(   0.0f, 0.7071f, 0.7071f,    0.0f), Anglef(180.f,  90.f,   0.f));
	addTestData(glm::quat(   0.5f,   -0.5f,   -0.5f,    0.5f), Anglef(-90.f,  90.f,   0.f));
	
	addTestData(glm::quat(0.7071f,    0.0f,    0.0f,-0.7071f), Anglef(  0.f, -90.f,   0.f));
	addTestData(glm::quat(   0.5f,   -0.5f,    0.5f,   -0.5f), Anglef( 90.f, -90.f,   0.f));
	addTestData(glm::quat(   0.0f,-0.7071f, 0.7071f,    0.0f), Anglef(180.f, -90.f,   0.f));
	addTestData(glm::quat(   0.5f,    0.5f,   -0.5f,   -0.5f), Anglef(-90.f, -90.f,   0.f));
	
	addTestData(glm::quat(0.7071f, 0.7071f,    0.0f,    0.0f), Anglef(  0.f,   0.f,  90.f));
	addTestData(glm::quat(   0.5f,    0.5f,    0.5f,   -0.5f), Anglef( 90.f,   0.f,  90.f));
	addTestData(glm::quat(   0.0f,    0.0f, 0.7071f,-0.7071f), Anglef(180.f,   0.f,  90.f));
	addTestData(glm::quat(   0.5f,    0.5f,   -0.5f,    0.5f), Anglef(-90.f,   0.f,  90.f));
	
	addTestData(glm::quat(   0.0f,    1.0f,    0.0f,    0.0f), Anglef(  0.f,   0.f, 180.f));
	addTestData(glm::quat(   0.0f, 0.7071f,    0.0f,-0.7071f), Anglef( 90.f,   0.f, 180.f));
	addTestData(glm::quat(   0.0f,    0.0f,    0.0f,    1.0f), Anglef(180.f,   0.f, 180.f));
	addTestData(glm::quat(   0.0f, 0.7071f,    0.0f, 0.7071f), Anglef(-90.f,   0.f, 180.f));
	
	addTestData(glm::quat(0.7071f,-0.7071f,    0.0f,    0.0f), Anglef(  0.f,   0.f, -90.f));
	addTestData(glm::quat(   0.5f,   -0.5f,    0.5f,    0.5f), Anglef( 90.f,   0.f, -90.f));
	addTestData(glm::quat(   0.0f,    0.0f, 0.7071f, 0.7071f), Anglef(180.f,   0.f, -90.f));
	addTestData(glm::quat(   0.5f,   -0.5f,   -0.5f,   -0.5f), Anglef(-90.f,   0.f, -90.f));
}

void LegacyMathTest::tearDown() {
	rotations.clear();
}

void LegacyMathTest::quaternionTests() {
	
	std::vector<TestRotation>::iterator it;
	for(it = rotations.begin(); it != rotations.end(); ++it) {
		
		glm::quat A = it->quat;	
		glm::quat B = it->quat;
	
		CPPUNIT_ASSERT_EQUAL(A, B);
		
		glm::quat inverseA = A;
		Quat_Reverse(&inverseA);
		glm::quat inverseB = glm::gtc::quaternion::inverse(B);
		
		CPPUNIT_ASSERT_EQUAL(inverseA, inverseB);
	
		Vec3f vecA = TransformVertexQuat(A, Vec3f(1.f, 0.5f, 0.1f));
		Vec3f vecB = B * Vec3f(1.f, 0.5f, 0.1f);
		
		CPPUNIT_ASSERT_EQUAL(vecA, vecB);
		
		glm::mat4x4 matrixA;
		MatrixFromQuat(matrixA, A);
		
		glm::mat4x4 matrixB = glm::toMat4(B);
		
		CPPUNIT_ASSERT_EQUAL(matrixA, matrixB);
	}
}

glm::quat Quat_Multiply(const glm::quat & q1, const glm::quat & q2)
{
	/*
	Fast multiplication

	There are some schemes available that reduces the number of internal
	multiplications when doing quaternion multiplication. Here is one:

	   q = (a, b, c, d), p = (x, y, z, w)

	   tmp_00 = (d - c) * (z - w)
	   tmp_01 = (a + b) * (x + y)
	   tmp_02 = (a - b) * (z + w)
	   tmp_03 = (c + d) * (x - y)
	   tmp_04 = (d - b) * (y - z)
	   tmp_05 = (d + b) * (y + z)
	   tmp_06 = (a + c) * (x - w)
	   tmp_07 = (a - c) * (x + w)
	   tmp_08 = tmp_05 + tmp_06 + tmp_07
	   tmp_09 = 0.5 * (tmp_04 + tmp_08)

	   q * p = (tmp_00 + tmp_09 - tmp_05,
	            tmp_01 + tmp_09 - tmp_08,
	            tmp_02 + tmp_09 - tmp_07,
	            tmp_03 + tmp_09 - tmp_06)

	With this method You get 7 less multiplications, but 15 more
	additions/subtractions. Generally, this is still an improvement.
	  */

	return glm::quat(
		q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z,
		q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y,
		q1.w * q2.y + q1.y * q2.w + q1.z * q2.x - q1.x * q2.z,
		q1.w * q2.z + q1.z * q2.w + q1.x * q2.y - q1.y * q2.x
	);
}

void LegacyMathTest::quatMuliplyTest() {
	glm::quat A = glm::quat(0.f,  1.f, 0.f, 0.f);
	glm::quat B = glm::quat(0.f,  0.f, 1.f, 0.f);
	
	CPPUNIT_ASSERT_EQUAL(A * B, Quat_Multiply(A, B));
}
