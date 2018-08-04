// Based on:
/*
Copyright (c) 2005 - 2016 G-Truc Creation

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef ARX_MATH_GTXFUNCTIONS_H
#define ARX_MATH_GTXFUNCTIONS_H

#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>

#include "math/Types.h"

namespace arx {

inline float length2(float x)
{
	return x * x;
}

inline float length2(Vec2f const & v)
{
	return glm::dot(v, v);
}

inline float length2(Vec3f const & v)
{
	return glm::dot(v, v);
}

inline float distance2(float p0, float p1)
{
	return arx::length2(p1 - p0);
}

inline float distance2(Vec2f const & p0, Vec2f const & p1)
{
	return arx::length2(p1 - p0);
}

inline float distance2(Vec3f const & p0, Vec3f const & p1)
{
	return arx::length2(p1 - p0);
}

inline glm::mat4 eulerAngleX
(
	float const & angleX
)
{
	float cosX = glm::cos(angleX);
	float sinX = glm::sin(angleX);

	return glm::mat4(
		1.f, 0.f, 0.f, 0.f,
		0.f, cosX, sinX, 0.f,
		0.f, -sinX, cosX, 0.f,
		0.f, 0.f, 0.f, 1.f);
}

inline glm::mat4 eulerAngleY(float const & angleY) {
	
	float cosY = glm::cos(angleY);
	float sinY = glm::sin(angleY);
	
	return glm::mat4(cosY, 0.f, -sinY, 0.f,
	                 0.f, 1.f, 0.f, 0.f,
	                 sinY, 0.f, cosY, 0.f,
	                 0.f, 0.f, 0.f, 1.f);
}

inline glm::mat4 eulerAngleZ(float const & angleZ) {
	
	float cosZ = glm::cos(angleZ);
	float sinZ = glm::sin(angleZ);
	
	return glm::mat4(cosZ, sinZ, 0.f, 0.f,
	                 -sinZ, cosZ, 0.f, 0.f,
	                 0.f, 0.f, 1.f, 0.f,
	                 0.f, 0.f, 0.f, 1.f);
}

template <typename genType>
GLM_FUNC_QUALIFIER genType pow2(genType const & x)
{
	return x * x;
}

template <typename genType>
GLM_FUNC_QUALIFIER genType pow3(genType const & x)
{
	return x * x * x;
}

template <typename genType>
GLM_FUNC_QUALIFIER genType catmullRom
(
	genType const & v1,
	genType const & v2,
	genType const & v3,
	genType const & v4,
	typename genType::value_type const & s
)
{
	// typename genType::value_type s1 = s; ARX CHANGE !
	typename genType::value_type s2 = pow2(s);
	typename genType::value_type s3 = pow3(s);

	typename genType::value_type f1 = -s3 + typename genType::value_type(2) * s2 - s;
	typename genType::value_type f2 = typename genType::value_type(3) * s3 - typename genType::value_type(5) * s2 + typename genType::value_type(2);
	typename genType::value_type f3 = typename genType::value_type(-3) * s3 + typename genType::value_type(4) * s2 + s;
	typename genType::value_type f4 = s3 - s2;

	return (f1 * v1 + f2 * v2 + f3 * v3 + f4 * v4) / typename genType::value_type(2);

}

// from glm/gtx/intersect.inl

template <typename genType>
GLM_FUNC_QUALIFIER bool intersectLineTriangle
(
	genType const & orig, genType const & dir,
	genType const & vert0, genType const & vert1, genType const & vert2,
	genType & position
)
{
	typename genType::value_type Epsilon = std::numeric_limits<typename genType::value_type>::epsilon();

	genType edge1 = vert1 - vert0;
	genType edge2 = vert2 - vert0;

	genType pvec = glm::cross(dir, edge2);

	float det = glm::dot(edge1, pvec);

	if (det > -Epsilon && det < Epsilon)
		return false;
	float inv_det = typename genType::value_type(1) / det;

	genType tvec = orig - vert0;

	position.y = glm::dot(tvec, pvec) * inv_det;
	if (position.y < typename genType::value_type(0) || position.y > typename genType::value_type(1))
		return false;

	genType qvec = glm::cross(tvec, edge1);

	position.z = glm::dot(dir, qvec) * inv_det;
	if (position.z < typename genType::value_type(0) || position.y + position.z > typename genType::value_type(1))
		return false;

	position.x = glm::dot(edge2, qvec) * inv_det;

	return true;
}

GLM_FUNC_QUALIFIER Vec2f rotate(Vec2f const & v, float const & angle) {
	float const cos(glm::cos(angle));
	float const sin(glm::sin(angle));
	return Vec2f(v.x * cos - v.y * sin, v.x * sin + v.y * cos);
}

GLM_FUNC_QUALIFIER float angle(Vec2f const & x, Vec2f const & y) {
	return glm::acos(glm::clamp(glm::dot(x, y), -1.f, 1.f));
}

GLM_FUNC_QUALIFIER float orientedAngle
(
	Vec2f const & x,
	Vec2f const & y
)
{
	float const Angle(glm::acos(glm::clamp(glm::dot(x, y), -1.f, 1.f)));

	if(glm::all(glm::epsilonEqual(y, arx::rotate(x, Angle), 0.0001f)))
		return Angle;
	else
		return -Angle;
}

} // namespace arx

#endif // ARX_MATH_GTXFUNCTIONS_H
