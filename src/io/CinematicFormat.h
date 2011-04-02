
#ifndef ARX_IO_CINEMATICFORMAT_H
#define ARX_IO_CINEMATICFORMAT_H

#include "animation/Cinematic.h"
#include "core/Common.h"
#include "graphics/GraphicsFormat.h"


#pragma pack(push,1)


static const s32 CINEMATIC_FILE_VERSION = (1<<16) | 76;
static const s16 INTERP_NO_FADE = 2;
// Version 1.75 structures

struct C_KEY_1_75 {
	s32 frame;
	s32 numbitmap;
	s32 fx; // associated fx
	s16 typeinterp, force;
	SavedVec3 pos;
	f32 angz;
	s32 color;
	s32 colord;
	s32 colorf;
	s32 idsound;
	f32 speed;
	CinematicLight light;
	SavedVec3 posgrille;
	f32 angzgrille;
	f32 speedtrack;
};

struct SavedCinematicTrack {
	s32 startframe;
	s32 endframe;
	f32 currframe;
	f32 fps;
	s32 nbkey;
	s32 pause;
	s32 key;
};


#pragma pack(pop)


#endif // ARX_IO_CINEMATICFORMAT_H
