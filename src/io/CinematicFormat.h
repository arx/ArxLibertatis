
#ifndef ARX_IO_CINEMATICFORMAT_H
#define ARX_IO_CINEMATICFORMAT_H

#include "animation/Cinematic.h"
#include "core/Common.h"
#include "graphics/GraphicsFormat.h"


#pragma pack(push,1)


static const s32 CINEMATIC_FILE_VERSION = (1<<16) | 76;
static const s16 INTERP_NO_FADE = 2;

// Version 1.59 structures

struct C_KEY_1_59 {
	s32 frame;
	s32 numbitmap;
	s32 fx; // associated fx
	s32 typeinterp;
	SavedVec3 pos;
	f32 angz;
	s32 color;
	s32 colord;
	s32 colorf;
	f32 speed;
};

// Version 1.65 structures

struct C_KEY_1_65 {
	s32 frame;
	s32 numbitmap;
	s32 fx; // associated fx
	s32 typeinterp;
	SavedVec3 pos;
	f32 angz;
	s32 color;
	s32 colord;
	s32 colorf;
	s32 idsound;
	f32 speed;
};

// Version 1.70 structures

struct C_KEY_1_70 {
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
};

// Version 1.71 structures

struct CinematicLight_1_71 {
	
	SavedVec3 pos;
	f32 fallin;
	f32 fallout;
	f32 r, g, b;
	f32 intensity;
	f32 intensiternd;
	s32 prev;
	s32 next;
	
	inline operator CinematicLight() {
		CinematicLight l;
		l.pos = pos;
		l.fallin = fallin;
		l.fallout = fallout;
		l.r = r;
		l.g = g;
		l.b = b;
		l.intensity = intensity;
		l.intensiternd = intensiternd;
		return l;
	}
	
};

struct C_KEY_1_71 {
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
	CinematicLight_1_71 light;
};

// Version 1.72 structures

struct CinematicLight_1_72 {
	
	SavedVec3 pos;
	f32 fallin;
	f32 fallout;
	f32 r, g, b;
	f32 intensity;
	f32 intensiternd;
	
	inline operator CinematicLight() {
		CinematicLight l;
		l.fallin = fallin;
		l.fallout = fallout;
		l.r = r;
		l.g = g;
		l.b = b;
		l.intensity = intensity;
		l.intensiternd = intensiternd;
		return l;
	}
	
};

struct C_KEY_1_72 {
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
	CinematicLight_1_72 light;
	SavedVec3 posgrille;
	f32 angzgrille;
};

// Version 1.74 structures

struct C_KEY_1_74 {
	s32 frame;
	s32 numbitmap;
	s32 fx; //associated fx
	s16 typeinterp, force;
	SavedVec3 pos;
	f32 angz;
	s32 color;
	s32 colord;
	s32 colorf;
	s32 idsound;
	f32 speed;
	CinematicLight_1_71 light;
	SavedVec3 posgrille;
	f32 angzgrille;
};

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
	CinematicLight_1_71 light;
	SavedVec3 posgrille;
	f32 angzgrille;
	f32 speedtrack;
};

// Version 1.75 structures

struct C_KEY_1_76 {
	s32 frame;
	s32 numbitmap;
	s32 fx; // associated fx
	s16 typeinterp, force;
	SavedVec3 pos;
	f32 angz;
	s32 color;
	s32 colord;
	s32 colorf;
	f32 speed;
	CinematicLight_1_71 light;
	SavedVec3 posgrille;
	f32 angzgrille;
	f32 speedtrack;
	s32 idsound[16];
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
