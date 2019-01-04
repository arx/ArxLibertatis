/*
 * Copyright 2011-2017 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_TESTS_MATH_ASSERTIONTRAITS_H
#define ARX_TESTS_MATH_ASSERTIONTRAITS_H

#include <sstream>
#include <iomanip>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/ext.hpp>

#include <cppunit/TestAssert.h>

#include "graphics/Math.h"

namespace CppUnit {
	
	template <>
	struct assertion_traits<glm::quat> {
		static bool equal(const glm::quat & v, const glm::quat & other) {
			
			glm::vec4 a = glm::vec4(v.w, v.x, v.y, v.z);
			glm::vec4 b = glm::vec4(other.w, other.x, other.y, other.z);
			
			return
			   glm::all(glm::epsilonEqual(a, b, 0.001f))
			|| glm::all(glm::epsilonEqual(a, b * -1.f, 0.001f));
		}
		
		static std::string toString(const glm::quat quat) {
			std::ostringstream ost;
			ost << std::endl << std::fixed << std::setprecision(4);
			
			ost << "glm::quat(";
			ost << quat.w << ", ";
			ost << quat.x << ", ";
			ost << quat.y << ", ";
			ost << quat.z << ")" << std::endl;
			return ost.str();
		}
	};
	
	template <>
	struct assertion_traits<Vec2s> {
		static bool equal(const Vec2s & v, const Vec2s & other) {
			return glm::all(glm::equal(v, other));
		}
		
		static std::string toString(const Vec2s &v) {
			std::ostringstream ost;
			ost << std::fixed << std::setprecision(4);
			ost << "Vec2s(";
			ost << v.x << ", ";
			ost << v.y << ")";
			return ost.str();
		}
	};
	
	template <>
	struct assertion_traits<Vec2i> {
		static bool equal(const Vec2i & v, const Vec2i & other) {
			return glm::all(glm::equal(v, other));
		}
		
		static std::string toString(const Vec2i &v) {
			std::ostringstream ost;
			ost << std::fixed << std::setprecision(4);
			ost << "Vec2i(" << v.x << ", " << v.y << ")";
			return ost.str();
		}
	};
	
	template <>
	struct assertion_traits<Vec3f> {
		static bool equal(const Vec3f & v, const Vec3f & other) {
			return glm::all(glm::epsilonEqual(v, other, 0.001f));
		}
		
		static std::string toString(const Vec3f &v) {
			std::ostringstream ost;
			ost << std::fixed << std::setprecision(4);
			ost << "Vec3f(";
			ost << v.x << ", ";
			ost << v.y << ", ";
			ost << v.z << ")";
			return ost.str();
		}
	};
	
	template <>
	struct assertion_traits<Vec4f> {
		static bool equal(const Vec4f & v, const Vec4f & other) {
			return glm::all(glm::epsilonEqual(v, other, 0.001f));
		}
		
		static std::string toString(const Vec4f &v) {
			std::ostringstream ost;
			ost << std::fixed << std::setprecision(4);
			ost << "Vec4f(";
			ost << v.x << ", ";
			ost << v.y << ", ";
			ost << v.z << ", ";
			ost << v.w << ")";
			return ost.str();
		}
	};
	
	template <>
	struct assertion_traits<glm::mat4x4> {
		static bool equal(const glm::mat4x4 & mat, const glm::mat4x4 & other) {
			
			for(int i = 0; i < 4; i++) {
				if(! glm::all(glm::epsilonEqual(glm::row(mat, i), glm::row(other, i), 0.001f))) {
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
	
	template <>
	struct assertion_traits<Color> {
		static bool equal(const Color & col, const Color & other) {
			return col.r == other.r && col.g == other.g && col.b == other.b && col.a == other.a;
		}
		
		static std::string toString(const Color &col) {
			OStringStream ost;
			ost << std::setw(3) << int(col.r) << "r ";
			ost << std::setw(3) << int(col.g) << "g ";
			ost << std::setw(3) << int(col.b) << "b ";
			ost << std::setw(3) << int(col.a) << "a";
			return ost.str();
		}
	};
	
} // namespace CppUnit

#endif // ARX_TESTS_MATH_ASSERTIONTRAITS_H
