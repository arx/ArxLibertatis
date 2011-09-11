
#ifndef ARX_GRAPHICS_VERTEX_H
#define ARX_GRAPHICS_VERTEX_H

#include "graphics/Color.h"
#include "platform/Platform.h"
#include "math/Vector2.h"
#include "math/Vector3.h"

struct TexturedVertex {
	
	Vec3f p;
	float rhw;
	
	ColorBGRA color;
	ColorBGRA specular;
	
	Vec2f uv;
	
	TexturedVertex(const Vec3f & _p, float _rhw, ColorBGRA _color, ColorBGRA _specular, Vec2f _uv) : p(_p), rhw(_rhw), color(_color), specular(_specular), uv(_uv) { }
	
	TexturedVertex(const TexturedVertex & o) : p(o.p), rhw(o.rhw), color(o.color), specular(o.specular), uv(o.uv) { }
	
	TexturedVertex() { }
	
};

template <class Vertex>
class VertexBuffer;

struct SMY_VERTEX {
	Vec3f p;
	ColorBGRA color;
	Vec2f uv;
};

struct SMY_VERTEX3 {
	Vec3f p;
	ColorBGRA color;
	Vec2f uv[3];
};

struct EERIE_VERTEX {
	TexturedVertex vert;
	Vec3f v;
	Vec3f norm;
	Vec3f vworld;
};

#endif // ARX_GRAPHICS_VERTEX_H
