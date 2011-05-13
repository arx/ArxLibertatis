
#ifndef ARX_GRAPHICS_BASEGRAPHICSTYPES_H
#define ARX_GRAPHICS_BASEGRAPHICSTYPES_H

#include "platform/math/Vector3.h"

struct EERIE_RGB {
	float r;
	float g;
	float b;
};

struct EERIE_QUAT {
	float x;
	float y;
	float z;
	float w;
};

struct EERIEMATRIX {
	float _11, _12, _13, _14;
	float _21, _22, _23, _24;
	float _31, _32, _33, _34;
	float _41, _42, _43, _44;
};

struct EERIE_CYLINDER {
	Vec3f origin;
	float radius;
	float height;
};

struct EERIE_SPHERE {
	
	Vec3f origin;
	float radius;
	
	bool contains(const Vec3f & pos) const {
		return closerThan(pos, origin, radius);
	}
	
};

#endif // ARX_GRAPHICS_BASEGRAPHICSTYPES_H
