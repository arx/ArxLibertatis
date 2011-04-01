
#ifndef ARX_GRAPHICS_BASEGRAPHICSTYPES_H
#define ARX_GRAPHICS_BASEGRAPHICSTYPES_H

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

struct EERIE_2D {
	union {
		float x;
		float a;
	};
	union {
		float y;
		float b;
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
	
	inline EERIE_3D() {
		// TODO initialize?
	}
	
	inline EERIE_3D(const EERIE_3D & o) {
		x = o.x, y = o.y, z = o.z;
	}
	
	inline EERIE_3D(float _x, float _y, float _z) {
		x = _x, y = _y, z = _z;
	}
	
	inline void clear() {
		x = y = z = 0;
	}
	
	inline EERIE_3D operator-() const {
		return EERIE_3D(-x, -y, -z);
	}
	
	inline EERIE_3D & operator+=(const EERIE_3D & o) {
		x += o.x, y += o.y, z += o.z;
		return *this;
	}
	
	inline EERIE_3D & operator-=(const EERIE_3D & o) {
		x -= o.x, y -= o.y, z -= o.z;
		return *this;
	}
	
	inline EERIE_3D & operator*=(const EERIE_3D & o) {
		x *= o.x, y *= o.y, z *= o.z;
		return *this;
	}
	
	inline EERIE_3D & operator+=(const float v) {
		x += v, y += v, z += v;
		return *this;
	}
	
	inline EERIE_3D & operator-=(const float v) {
		x -= v, y -= v, z -= v;
		return *this;
	}
	
	inline EERIE_3D & operator*=(const float v) {
		x *= v, y *= v, z *= v;
		return *this;
	}
	
	inline EERIE_3D & operator/=(const float v) {
		x /= v, y /= v, z /= v;
		return *this;
	}
	
	inline EERIE_3D operator+(const EERIE_3D & o) const {
		return EERIE_3D(x + o.x, y + o.y, z + o.z);
	}
	
	inline EERIE_3D operator-(const EERIE_3D & o) const {
		return EERIE_3D(x - o.x, y - o.y, z - o.z);
	}
	
	inline EERIE_3D operator*(const EERIE_3D & o) const {
		return EERIE_3D(x * o.x, y * o.y, z * o.z);
	}
	
	inline EERIE_3D operator+(const float v) const {
		return EERIE_3D(x + v, y + v, z + v);
	}
	
	inline EERIE_3D operator-(const float v) const {
		return EERIE_3D(x - v, y - v, z - v);
	}
	
	inline EERIE_3D operator*(const float v) const {
		return EERIE_3D(x * v, y * v, z * v);
	}
	
	inline EERIE_3D operator/(const float v) const {
		return EERIE_3D(x / v, y / v, z / v);
	}
	
	inline EERIE_3D & operator=(const EERIE_3D & o) {
		x = o.x, y = o.y, z = o.z;
		return *this;
	}
	
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
};

#endif // ARX_GRAPHICS_BASEGRAPHICSTYPES_H
