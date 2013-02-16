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

#include <cppunit/TestAssert.h>

#include "GraphicsUtilityTest.h"

#include "graphics/Math.h"


CPPUNIT_TEST_SUITE_REGISTRATION(GraphicsUtilityTest);

bool checkFloat(float a, float b) {
	return std::fabs(a-b) < 0.001f;
}

namespace CppUnit {

	template<>
	struct assertion_traits<EERIEMATRIX> {
		static bool equal(const EERIEMATRIX &mat, const EERIEMATRIX &other) {
			return  checkFloat(mat._11,other._11) && checkFloat(mat._12,other._12) && checkFloat(mat._13,other._13) && checkFloat(mat._14,other._14) &&
					checkFloat(mat._21,other._21) && checkFloat(mat._22,other._22) && checkFloat(mat._23,other._23) && checkFloat(mat._24,other._24) &&
					checkFloat(mat._31,other._31) && checkFloat(mat._32,other._32) && checkFloat(mat._33,other._33) && checkFloat(mat._34,other._34) &&
					checkFloat(mat._41,other._41) && checkFloat(mat._42,other._42) && checkFloat(mat._43,other._43) && checkFloat(mat._44,other._44);
		}

		static std::string toString(const EERIEMATRIX &matrix) {
			OStringStream ost;
			ost << std::endl << std::fixed;
			ost << matrix._11 << "\t" << matrix._12 << "\t" << matrix._13 << "\t" << matrix._14 << std::endl;
			ost << matrix._21 << "\t" << matrix._22 << "\t" << matrix._23 << "\t" << matrix._24 << std::endl;
			ost << matrix._31 << "\t" << matrix._32 << "\t" << matrix._33 << "\t" << matrix._34 << std::endl;
			ost << matrix._41 << "\t" << matrix._42 << "\t" << matrix._43 << "\t" << matrix._44 << std::endl;
			return ost.str();
		}
	};
}

void GraphicsUtilityTest::front() {
	transform.pos = Vec3f(0.f, 0.f, 0.f);
	transform.updateFromAngle(Anglef(0.f, 0.f, 0.f));
	Util_SetViewMatrix(matrix, transform);
	expected.setToIdentity();
	CPPUNIT_ASSERT_EQUAL(expected, matrix);
}

void GraphicsUtilityTest::back() {
	transform.updateFromAngle(Anglef(180.f, 180.f, 0.f));
	Util_SetViewMatrix(matrix, transform);
	expected.setToIdentity();
	expected._12 = 7.64274186e-15f;
	expected._13 = -8.74227766e-08f;
	expected._23 = 8.74227766e-08f;
	expected._31 = 8.74227766e-08f;
	expected._32 = -8.74227766e-08f;
	CPPUNIT_ASSERT_EQUAL(expected, matrix);
}

void GraphicsUtilityTest::edgeCase1()
{
	transform.pos = Vec3f(0.f, 0.f, 0.f);
	transform.updateFromAngle(Anglef(90.f, 90.f, 0.f));
	Util_SetViewMatrix(matrix, transform);
	expected.setToIdentity();
	expected._12 = -8.35187172e-23f;
	expected._13 = 4.37113883e-08f;
	expected._21 = 4.37113883e-08f;
	expected._22 = 1.91068547e-15f;
	expected._23 = -1.f;
	expected._32 = 1.f;
	expected._33 = 1.91068547e-15f;
	CPPUNIT_ASSERT_EQUAL(expected, matrix);
}

void GraphicsUtilityTest::edgeCase2()
{
	transform.updateFromAngle(Anglef(-90.f, -90.f, 0.f));
	Util_SetViewMatrix(matrix, transform);
	expected.setToIdentity();
	expected._11 = -1.f;
	expected._12 = 8.35187172e-23f;
	expected._13 = -4.37113883e-08f;
	expected._21 = -4.37113883e-08f;
	expected._22 = -1.91068547e-15f;
	expected._23 = 1.f;
	expected._32 = 1.f;
	expected._33 = 1.91068547e-15f;
	CPPUNIT_ASSERT_EQUAL(expected, matrix);
}

void GraphicsUtilityTest::translation()
{
	transform.pos = Vec3f(10.f, 200.f, 3000.f);
	transform.updateFromAngle(Anglef(0.f, 0.f, 0.f));
	Util_SetViewMatrix(matrix, transform);
	expected.setToIdentity();
	expected._41 = -10;
	expected._42 = 200;
	expected._43 = -3000;
	CPPUNIT_ASSERT_EQUAL(expected, matrix);
}

void GraphicsUtilityTest::combined()
{
	transform.pos = Vec3f(10.f, 200.f, 3000.f);
	transform.updateFromAngle(Anglef(45.f, -45.f, 0.f));
	Util_SetViewMatrix(matrix, transform);
	expected._11 = 0.707106709f;
	expected._12 = 0.5f;
	expected._13 = 0.5f;
	expected._14 = 0.f;
	expected._21 = 1.49011612e-08f;
	expected._22 = 0.707106769f;
	expected._23 = -0.707106769f;
	expected._24 = 0.f;
	expected._31 = -0.707106769f;
	expected._32 = 0.5f;
	expected._33 = 0.5f;
	expected._34 = 0.f;
	expected._41 = 2114.24927f;
	expected._42 = -1363.57849f;
	expected._43 = -1646.42126f;
	expected._44 = 1.f;
	CPPUNIT_ASSERT_EQUAL(expected, matrix);
}
