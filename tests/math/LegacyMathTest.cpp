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

#include <glm/gtx/euler_angles.hpp>

#include "graphics/Math.h"

#include "AssertionTraits.h"
#include "LegacyMath.h"

CPPUNIT_TEST_SUITE_REGISTRATION(LegacyMathTest);

struct TestRotation {
	glm::quat quat;
	Anglef angle;
	glm::mat3 mat;
	
	TestRotation(glm::quat quat, Anglef angle, glm::mat3 mat)
		: quat(quat)
		, angle(angle)
		, mat(mat)
	{}
};

std::vector<TestRotation> rotations;

static void addTestData(glm::quat quat, Anglef angle, glm::mat3 mat) {
	rotations.push_back(TestRotation(quat, angle, glm::transpose(mat)));
}

void LegacyMathTest::setUp() {
	
	using glm::quat;
	using glm::mat3;
	
	// Data from:
	// http://www.euclideanspace.com/maths/geometry/rotations/conversions/eulerToQuaternion/steps/index.htm
	// http://www.euclideanspace.com/maths/algebra/matrix/transforms/examples/index.htm
	
	// Identity (no rotation)
	addTestData(quat(    1.f,    0.0f,    0.0f,    0.0f), Anglef(  0,  0,  0), mat3( 1, 0, 0,  0, 1, 0,  0, 0, 1));
	// 90 degrees about y axis
	addTestData(quat(0.7071f,    0.0f, 0.7071f,    0.0f), Anglef( 90,  0,  0), mat3( 0, 0, 1,  0, 1, 0, -1, 0, 0));
	// 180 degrees about y axis
	addTestData(quat(   0.0f,    0.0f,     1.f,    0.0f), Anglef(180,  0,  0), mat3(-1, 0, 0,  0, 1, 0,  0, 0,-1));
	// 270 degrees about y axis
	addTestData(quat(0.7071f,    0.0f,-0.7071f,    0.0f), Anglef(-90,  0,  0), mat3( 0, 0,-1,  0, 1, 0,  1, 0, 0));
	
	addTestData(quat(0.7071f,    0.0f,    0.0f, 0.7071f), Anglef(  0, 90,  0), mat3( 0,-1, 0,  1, 0, 0,  0, 0, 1));
	addTestData(quat(   0.5f,    0.5f,    0.5f,    0.5f), Anglef( 90, 90,  0), mat3( 0, 0, 1,  1, 0, 0,  0, 1, 0));
	addTestData(quat(   0.0f, 0.7071f, 0.7071f,    0.0f), Anglef(180, 90,  0), mat3( 0, 1, 0,  1, 0, 0,  0, 0,-1));
	addTestData(quat(   0.5f,   -0.5f,   -0.5f,    0.5f), Anglef(-90, 90,  0), mat3( 0, 0,-1,  1, 0, 0,  0,-1, 0));
	
	addTestData(quat(0.7071f,    0.0f,    0.0f,-0.7071f), Anglef(  0,-90,  0), mat3( 0, 1, 0, -1, 0, 0,  0, 0, 1));
	addTestData(quat(   0.5f,   -0.5f,    0.5f,   -0.5f), Anglef( 90,-90,  0), mat3( 0, 0, 1, -1, 0, 0,  0,-1, 0));
	addTestData(quat(   0.0f,-0.7071f, 0.7071f,    0.0f), Anglef(180,-90,  0), mat3( 0,-1, 0, -1, 0, 0,  0, 0,-1));
	addTestData(quat(   0.5f,    0.5f,   -0.5f,   -0.5f), Anglef(-90,-90,  0), mat3( 0, 0,-1, -1, 0, 0,  0, 1, 0));
	
	addTestData(quat(0.7071f, 0.7071f,    0.0f,    0.0f), Anglef(  0,  0, 90), mat3( 1, 0, 0,  0, 0,-1,  0, 1, 0));
	addTestData(quat(   0.5f,    0.5f,    0.5f,   -0.5f), Anglef( 90,  0, 90), mat3( 0, 1, 0,  0, 0,-1, -1, 0, 0));
	addTestData(quat(   0.0f,    0.0f, 0.7071f,-0.7071f), Anglef(180,  0, 90), mat3(-1, 0, 0,  0, 0,-1,  0,-1, 0));
	addTestData(quat(   0.5f,    0.5f,   -0.5f,    0.5f), Anglef(-90,  0, 90), mat3( 0,-1, 0,  0, 0,-1,  1, 0, 0));
	
	addTestData(quat(   0.0f,    1.0f,    0.0f,    0.0f), Anglef(  0,  0,180), mat3( 1, 0, 0,  0,-1, 0,  0, 0,-1));
	addTestData(quat(   0.0f, 0.7071f,    0.0f,-0.7071f), Anglef( 90,  0,180), mat3( 0, 0,-1,  0,-1, 0, -1, 0, 0));
	addTestData(quat(   0.0f,    0.0f,    0.0f,    1.0f), Anglef(180,  0,180), mat3(-1, 0, 0,  0,-1, 0,  0, 0, 1));
	addTestData(quat(   0.0f, 0.7071f,    0.0f, 0.7071f), Anglef(-90,  0,180), mat3( 0, 0, 1,  0,-1, 0,  1, 0, 0));
	
	addTestData(quat(0.7071f,-0.7071f,    0.0f,    0.0f), Anglef(  0,  0,-90), mat3( 1, 0, 0,  0, 0, 1,  0,-1, 0));
	addTestData(quat(   0.5f,   -0.5f,    0.5f,    0.5f), Anglef( 90,  0,-90), mat3( 0,-1, 0,  0, 0, 1, -1, 0, 0));
	addTestData(quat(   0.0f,    0.0f, 0.7071f, 0.7071f), Anglef(180,  0,-90), mat3(-1, 0, 0,  0, 0, 1,  0, 1, 0));
	addTestData(quat(   0.5f,   -0.5f,   -0.5f,   -0.5f), Anglef(-90,  0,-90), mat3( 0, 1, 0,  0, 0, 1,  1, 0, 0));
}

void LegacyMathTest::tearDown() {
	rotations.clear();
}

void LegacyMathTest::rotationTestDataTest() {
	typedef std::vector<TestRotation>::iterator Itr;
	for(Itr i = rotations.begin(); i != rotations.end(); ++i) {
		
		CPPUNIT_ASSERT_EQUAL(i->quat, glm::toQuat(i->mat));
		CPPUNIT_ASSERT_EQUAL(glm::toMat4(i->quat), glm::mat4(i->mat));
	}
}

void LegacyMathTest::quaternionTests() {
	
	std::vector<TestRotation>::iterator it;
	for(it = rotations.begin(); it != rotations.end(); ++it) {
		
		glm::quat A = it->quat;	
		glm::quat B = it->quat;
	
		CPPUNIT_ASSERT_EQUAL(A, B);
		
		glm::quat inverseA = A;
		Quat_Reverse(&inverseA);
		glm::quat inverseB = glm::inverse(B);
		
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

void LegacyMathTest::quatMuliplyTest() {
	glm::quat A = glm::quat(0.f,  1.f, 0.f, 0.f);
	glm::quat B = glm::quat(0.f,  0.f, 1.f, 0.f);
	
	CPPUNIT_ASSERT_EQUAL(A * B, Quat_Multiply(A, B));
}

// TODO this does not seem to cover all edge cases
//void LegacyMathTest::quatSlerpTest() {
//	
//	typedef std::vector<TestRotation>::iterator itr;
//	for(itr iA = rotations.begin(); iA != rotations.end(); ++iA) {
//		for(itr iB = rotations.begin(); iB != rotations.end(); ++iB) {
//			for(int a = 0; a <= 10; a++) {
//				glm::quat expected = glm::slerp(iA->quat, iB->quat, a * 0.1f);
//				glm::quat result = Quat_Slerp(iA->quat, iB->quat, a * 0.1f);
//				
//				CPPUNIT_ASSERT_EQUAL(expected, result);
//			}
//		}
//	}
//}

void LegacyMathTest::quatTransformVectorTest() {
	
	typedef std::vector<TestRotation>::iterator Itr;
	
	for(Itr it = rotations.begin(); it != rotations.end(); ++it) {
		const Vec3f testVert(1.f, 0.5f, 0.1f);
		
		Vec3f vecA = TransformVertexQuat(it->quat, testVert);
		Vec3f vecB = it->quat * testVert;
		
		CPPUNIT_ASSERT_EQUAL(vecA, vecB);
		
		Vec3f vecC;
		TransformInverseVertexQuat(it->quat, testVert, vecC);
		
		Vec3f vecD = glm::inverse(it->quat) * testVert;
		
		CPPUNIT_ASSERT_EQUAL(vecC, vecD);
	}
}

void LegacyMathTest::quatMatrixConversionTest() {
	
	typedef std::vector<TestRotation>::iterator Itr;
	
	for(Itr it = rotations.begin(); it != rotations.end(); ++it) {
		CPPUNIT_ASSERT_EQUAL(glm::toMat4(it->quat), glm::mat4(it->mat));
		
		glm::quat q;
		QuatFromMatrix(q, glm::mat4(it->mat));
		CPPUNIT_ASSERT_EQUAL(glm::toQuat(it->mat), q);
	}
}

void LegacyMathTest::vecMatrixConversionTest() {
	
	typedef std::vector<TestRotation>::iterator Itr;
	
	for(Itr it = rotations.begin(); it != rotations.end(); ++it) {
		Vec3f front(0, 0, 1);
		Vec3f up(0, 1, 0);
		
		front = it->quat * front;
		up = it->quat * up;
		
		glm::mat4 mat;
		MatrixSetByVectors(mat, front, up);
		
		CPPUNIT_ASSERT_EQUAL(glm::mat4(it->mat), mat);
	}
}

void LegacyMathTest::angleTest()
{
	typedef std::vector<TestRotation>::iterator Itr;
	
	for(Itr it = rotations.begin(); it != rotations.end(); ++it) {
		Vec3f testVert(1.f, 0.5f, 0.1f);
		
		EERIE_TRANSFORM trans;
		trans.updateFromAngle(it->angle);
		
		Vec3f out, temp, temp2;
		YRotatePoint(&testVert, &temp, trans.ycos, trans.ysin);
		XRotatePoint(&temp, &temp2, trans.xcos, trans.xsin);
		ZRotatePoint(&temp2, &out, trans.zcos, trans.zsin);
		
		Vec3f vecA = out;
		Vec3f vecB = Vec3f(trans.worldToView * Vec4f(testVert, 1.f));
		
		CPPUNIT_ASSERT_EQUAL(vecA, vecB);
	}
}

void LegacyMathTest::angleConversionTest()
{
	typedef std::vector<TestRotation>::iterator Itr;
	for(Itr it = rotations.begin(); it != rotations.end(); ++it) {
		
		glm::quat q = toNonNpcRotation(it->angle);
		
		glm::quat q2 = glm::toQuat(toRotationMatrix(it->angle));
		
		CPPUNIT_ASSERT_EQUAL(q, q2);
	}
}

void LegacyMathTest::cameraRotationTest() {
	
	const Vec3f testVert(100.f, 200.f, 300.f);
	
	typedef std::vector<TestRotation>::iterator Itr;
	for(Itr it = rotations.begin(); it != rotations.end(); ++it) {
		
		EERIE_TRANSFORM trans;
		trans.pos = Vec3f(3.f, 6.f, 9.f);
		trans.updateFromAngle(it->angle);
		
		Vec3f vecA = camEE_RT(testVert, trans);
		Vec3f vecB = Vec3f(trans.worldToView * Vec4f(testVert, 1.f));
		
		CPPUNIT_ASSERT_EQUAL(vecA, vecB);
	}
}

// TODO copy-paste
Vec2s inventorySizeFromTextureSize(Vec2i size) {
	return Vec2s(glm::clamp((size + Vec2i(31, 31)) / Vec2i(32, 32), Vec2i(1, 1), Vec2i(3, 3)));
}

void LegacyMathTest::inventorySizeTest() {
	
	for(short i = 0; i < 100; ++i)
	for(short j = 0; j < 100; ++j) {
		Vec2i size(i, j);
		Vec2s expected = inventorySizeFromTextureSize(size);
		
		Vec2s result1 = inventorySizeFromTextureSize_1(i, j);
		Vec2s result2 = inventorySizeFromTextureSize_2(i, j);
		
		CPPUNIT_ASSERT_EQUAL_MESSAGE(glm::to_string(Vec2i(i, j)), expected, result1);
		CPPUNIT_ASSERT_EQUAL_MESSAGE(glm::to_string(Vec2i(i, j)), expected, result2);
	}
}

void LegacyMathTest::angleToVectorXZ_Test() {
	
	for(float angle = -1000; angle < 1000; angle += 0.01) {
		Vec3f expected = angleToVectorXZ(angle);
		Vec3f result = angleToVectorXZ_180offset(angle + 180);
		
		Vec3f result2 = VRotateY(Vec3f(0.f, 0.f, 1.f), float(360 - angle));
		
		CPPUNIT_ASSERT_EQUAL(expected, result);
		CPPUNIT_ASSERT_EQUAL(expected, result2);
	}
}

void LegacyMathTest::vectorRotateTest() {
	
	for(float angle = 0; angle < 720; angle += 10) {
		
		Vec3f foo = Vec3f(0.f, 0.f, 1.f);
		
		Vec3f result;
		VectorRotateY(foo, result, glm::radians(angle));
		
		Vec3f result2 = VRotateY(foo, angle);
		
		CPPUNIT_ASSERT_EQUAL(result, result2);
	}
	
	for(float angle = 0; angle < 720; angle += 10) {
		
		Vec3f foo = Vec3f(1.f, 0.f, 0.f);
		
		Vec3f result;
		VectorRotateZ(foo, result, glm::radians(angle));
		
		Vec3f result2 = VRotateZ(foo, angle);
		
		//std::cout << angle << "; " << glm::to_string(result) << glm::to_string(result2) << "\n";
		
		CPPUNIT_ASSERT_EQUAL(result, result2);
	}
}

void LegacyMathTest::focalToFovTest() {
	
	for(float focal = 100; focal < 800; focal += 0.1f) {
		float expected = glm::radians(focalToFovLegacy(focal));
		float result = focalToFov(focal);
		
		std::string msg = "In: " + glm::to_string(focal) + " Expected: " + glm::to_string(expected) + ", Result: " + glm::to_string(result);
		CPPUNIT_ASSERT_MESSAGE(msg, glm::epsilonEqual(expected, result, 2.f));
	}
}

void LegacyMathTest::pointInerpolationTest() {
	
	for(int i = 0; i < 500; i++) {
		Vec3f v0 = glm::linearRand(Vec3f(-10), Vec3f(10));
		Vec3f v1 = glm::linearRand(Vec3f(-10), Vec3f(10));
		Vec3f v2 = glm::linearRand(Vec3f(-10), Vec3f(10));
		Vec3f v3 = glm::linearRand(Vec3f(-10), Vec3f(10));
		
		for(int u = 0; u < 1000; u++) {
			float f = u / 1000.f;
			
			Vec3f res1 = interpolatePos(f, v0, v1, v2, v3);
			Vec3f res2 = glm::catmullRom(v0, v1, v2, v3, f);
			
			CPPUNIT_ASSERT_EQUAL(res1, res2);
		}
	}
}
