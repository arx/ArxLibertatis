
#ifndef ARX_GRAPHICS_VERTEX_H
#define ARX_GRAPHICS_VERTEX_H

#include "platform/Platform.h"
#include "math/Vector3.h"

struct TexturedVertex {
	
	float sx;
	float sy;
	float sz;
	float rhw;
	
	u32 color;
	u32 specular;
	
	float tu;
	float tv;
	
	TexturedVertex(const Vec3f & p, float _rhw, u32 _color, u32 _specular, float _tu, float _tv) : sx(p.x), sy(p.y), sz(p.z), rhw(_rhw), color(_color), specular(_specular), tu(_tu), tv(_tv) { }
	
	TexturedVertex(const TexturedVertex & o) : sx(o.sx), sy(o.sy), sz(o.sz), rhw(o.rhw), color(o.color), specular(o.specular), tu(o.tu), tv(o.tv) { }
	
	TexturedVertex() { }
	
};

template <class Vertex>
class VertexBuffer;

struct SMY_D3DVERTEX {
	float x, y, z;
	u32 color;
	float tu, tv;
};

struct SMY_D3DVERTEX3 {
	float x, y, z;
	u32 color;
	float tu, tv;
	float tu2, tv2;
	float tu3, tv3;
};

struct EERIE_VERTEX {
	TexturedVertex vert;
	Vec3f v;
	Vec3f norm;
	Vec3f vworld;
};

#endif // ARX_GRAPHICS_VERTEX_H
