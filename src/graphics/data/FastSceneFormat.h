
#ifndef ARX_GRAPHICS_DATA_FASTSCENEFORMAT_H
#define ARX_GRAPHICS_DATA_FASTSCENEFORMAT_H

#include "core/Common.h"
#include "graphics/GraphicsFormat.h"


#pragma pack(push,1)


const float FTS_VERSION = 0.141f;

struct UNIQUE_HEADER {
	char path[256];
	s32 count;
	f32 version;
	s32 uncompressedsize;
	s32 pad[3];
};

struct UNIQUE_HEADER2 {
	char path[256];
};

struct FAST_VERTEX {
	f32	sy;
	f32	ssx;
	f32	ssz;
	f32	stu;
	f32	stv;
};

struct FAST_EERIEPOLY {
	FAST_VERTEX v[4];
	s32 tex;
	SavedVec3 norm;
	SavedVec3 norm2;
	SavedVec3 nrml[4];
	f32 transval;
	f32 area;
	s32 type;
	s16 room;
	s16 paddy;
};

struct FAST_SCENE_HEADER {
	f32 version;
	s32 sizex;
	s32 sizez;
	s32 nb_textures;
	s32 nb_polys;
	s32 nb_anchors;
	SavedVec3 playerpos;
	SavedVec3 Mscenepos;
	s32 nb_portals;
	s32 nb_rooms;
};

struct FAST_TEXTURE_CONTAINER {
	s32 tc;
	s32 temp;
	char fic[256];
};

struct FAST_ANCHOR_DATA {
	SavedVec3 pos;
	f32 radius;
	f32 height;
	s16 nb_linked;
	s16 flags;
};

struct FAST_SCENE_INFO {
	s32 nbpoly;
	s32 nbianchors;
};

struct ROOM_DIST_DATA_SAVE {
	f32 distance; // -1 means use truedist
	SavedVec3 startpos;
	SavedVec3 endpos;
};

struct SAVE_EERIEPOLY {
	s32 type; // at least 16 bits
	SavedVec3 min;
	SavedVec3 max;
	SavedVec3 norm;
	SavedVec3 norm2;
	SavedD3DTLVertex v[4];
	SavedD3DTLVertex tv[4];
	SavedVec3 nrml[4];
	s32 tex;
	SavedVec3 center;
	f32 transval;
	f32 area;
	s16 room;
	s16 misc;
};

struct EERIE_SAVE_PORTALS {
	SAVE_EERIEPOLY poly;
	s32 room_1; // facing normal
	s32 room_2;
	s16 useportal;
	s16 paddy;
};

struct FAST_EP_DATA {
	
	s16 px;
	s16 py;
	s16 idx;
	s16 padd;
	
	operator EP_DATA() {
		EP_DATA b;
		b.px = px;
		b.py = py;
		b.idx = idx;
		b.padd = padd;
		return b;
	}
	
	FAST_EP_DATA & operator=(const EP_DATA & a) {
		px = a.px;
		py = a.py;
		idx = a.idx;
		padd = a.padd;
		return *this;
	}
	
};

struct EERIE_SAVE_ROOM_DATA {
	s32 nb_portals;
	s32 nb_polys;
	s32 padd[6];
};


#pragma pack(pop)


#endif // ARX_GRAPHICS_DATA_FASTSCENEFORMAT_H
