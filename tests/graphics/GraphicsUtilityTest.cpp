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

#include <iomanip>
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

		static std::string toString(const EERIEMATRIX &m) {
			OStringStream ost;
						
			static const int l = 14;
			ost << std::endl << std::fixed << std::setprecision(5);
			ost << std::setw(l) << m._11 << std::setw(l) << m._12 << std::setw(l) << m._13 << std::setw(l) << m._14 << std::endl;
			ost << std::setw(l) << m._21 << std::setw(l) << m._22 << std::setw(l) << m._23 << std::setw(l) << m._24 << std::endl;
			ost << std::setw(l) << m._31 << std::setw(l) << m._32 << std::setw(l) << m._33 << std::setw(l) << m._34 << std::endl;
			ost << std::setw(l) << m._41 << std::setw(l) << m._42 << std::setw(l) << m._43 << std::setw(l) << m._44 << std::endl;
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
	CPPUNIT_ASSERT_EQUAL(expected, matrix);
}

void GraphicsUtilityTest::edgeCase1()
{
	transform.pos = Vec3f(0.f, 0.f, 0.f);
	transform.updateFromAngle(Anglef(90.f, 90.f, 0.f));
	Util_SetViewMatrix(matrix, transform);
	expected.setToIdentity();
	expected._22 =  0.f;
	expected._23 = -1.f;
	expected._32 =  1.f;
	expected._33 =  0.f;
	CPPUNIT_ASSERT_EQUAL(expected, matrix);
}

void GraphicsUtilityTest::edgeCase2()
{
	transform.updateFromAngle(Anglef(-90.f, -90.f, 0.f));
	Util_SetViewMatrix(matrix, transform);
	expected.setToIdentity();
	expected._11 = -1.f;
	expected._22 =  0.f;
	expected._23 =  1.f;
	expected._32 =  1.f;
	expected._33 =  0.f;
	CPPUNIT_ASSERT_EQUAL(expected, matrix);
}

void GraphicsUtilityTest::translation()
{
	transform.pos = Vec3f(10.f, 200.f, 3000.f);
	transform.updateFromAngle(Anglef(0.f, 0.f, 0.f));
	Util_SetViewMatrix(matrix, transform);
	expected.setToIdentity();
	expected._41 = -10.f;
	expected._42 = 200.f;
	expected._43 = -3000.f;
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
	expected._21 = 0.f;
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
