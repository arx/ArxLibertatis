
#ifndef ARX_MATH_MATHFWD_H
#define ARX_MATH_MATHFWD_H

#include "platform/Platform.h"

template <class T>
class Angle;
typedef Angle<s32> Anglei;
typedef Angle<float> Anglef;
typedef Angle<double> Angled;

template<class T>
class _Rectangle;
typedef _Rectangle<s32> Rect;
typedef _Rectangle<float> Rectf;

template <class T>
class Vector2;
typedef Vector2<s32> Vec2i;
typedef Vector2<short> Vec2s;
typedef Vector2<float> Vec2f;
typedef Vector2<double> Vec2d;

template <class T>
class Vector3;
typedef Vector3<s32> Vec3i;
typedef Vector3<float> Vec3f;
typedef Vector3<double> Vec3d;

// Math constants
#define PI 3.14159265358979323846f

#endif // ARX_MATH_MATHFWD_H
