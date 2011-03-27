
#ifndef ARX_GRAPHICS_BASEGRAPHICSTYPES_H
#define ARX_GRAPHICS_BASEGRAPHICSTYPES_H

struct EERIE_RGB {
	float r;
	float g;
	float b;
};

struct EERIE_RGBA : EERIE_RGB
{
	float a;
};

struct EERIE_QUAT {
	float x;
	float y;
	float z;
	float w;
};

struct EERIE_2D {
	union {
		float x;
		float a;
		float u;
	};
	union {
		float y;
		float b;
		float v;
	};
};

struct EERIE_3D {
	
	union {
		float x;
		float a;
		float yaw;
	};
	union {
		float y;
		float b;
		float pitch;
	};
	union {
		float z;
		float g;
		float roll;
	};

	void clear() {
		x = 0;
		y = 0;
		z = 0;
	}
	
};

struct EERIE_4D : EERIE_3D
{
	float w;
};

struct EERIEMATRIX {
	float _11, _12, _13, _14;
	float _21, _22, _23, _24;
	float _31, _32, _33, _34;
	float _41, _42, _43, _44;
};

struct EERIE_CYLINDER {
	EERIE_3D origin;
	float radius;
	float height;
};

struct EERIE_SPHERE {
	EERIE_3D origin;
	float radius;
}; // Aligned 1 2 4

#endif // ARX_GRAPHICS_BASEGRAPHICSTYPES_H
