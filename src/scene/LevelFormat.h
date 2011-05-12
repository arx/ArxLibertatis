
#ifndef ARX_SCENE_LEVELFORMAT_H
#define ARX_SCENE_LEVELFORMAT_H

#include "graphics/GraphicsFormat.h"
#include "platform/Platform.h"

//Fileformat version
const f32 DLH_CURRENT_VERSION = 1.44f;


#pragma pack(push,1)


struct DANAE_LS_HEADER {
	
	f32 version;
	char ident[16];
	char lastuser[256];
	s32 time;
	SavedVec3 pos_edit;
	SavedAnglef angle_edit;
	s32 nb_scn;
	s32 nb_inter;
	s32 nb_nodes;
	s32 nb_nodeslinks;
	s32 nb_zones;
	s32 lighting;
	s32 Bpad[256];
	s32 nb_lights;
	s32 nb_fogs;
	
	s32 nb_bkgpolys;
	s32 nb_ignoredpolys;
	s32 nb_childpolys;
	s32 nb_paths;
	s32 pad[250];
	SavedVec3 offset;
	f32 fpad[253];
	char cpad[4096];
	s32 bpad[256];
	
};

struct DANAE_LS_SCENE {
	char name[512];
	s32 pad[16];
	f32 fpad[16];
};

struct DANAE_LS_LIGHTINGHEADER {
	s32 nb_values;
	s32 ViewMode;
	s32 ModeLight;
	s32 pad;
};

struct DANAE_LS_VLIGHTING {
	s32 r;
	s32 g;
	s32 b;
};

// version 1.003f
struct DANAE_LS_LIGHT {
	SavedVec3 pos;
	SavedColor rgb;
	f32 fallstart;
	f32 fallend;
	f32 intensity;
	f32 i;
	SavedColor ex_flicker;
	f32 ex_radius;
	f32 ex_frequency;
	f32 ex_size;
	f32 ex_speed;
	f32 ex_flaresize;
	f32 fpadd[24];
	s32 extras;
	s32 lpadd[31];
};

struct DANAE_LS_FOG {
	SavedVec3 pos;
	SavedColor rgb;
	f32 size;
	s32 special;
	f32 scale;
	SavedVec3 move;
	SavedAnglef angle;
	f32 speed;
	f32 rotatespeed;
	s32 tolive;
	s32 blend;
	f32 frequency;
	f32 fpadd[32];
	s32 lpadd[32];
	char cpadd[256];
};

struct DANAE_LS_NODE {
	char name[64];
	SavedVec3 pos;
	s32 pad[16];
	f32 fpad[16];
};

struct DANAE_LS_PATH {
	char name[64];
	s16 idx;
	s16 flags;
	SavedVec3 initpos;
	SavedVec3 pos;
	s32 nb_pathways;
	SavedColor rgb; 
	f32 farclip;
	f32 reverb;
	f32 amb_max_vol;
	f32 fpadd[26];
	s32 height;
	s32 lpadd[31];
	char ambiance[128]; 
	char cpadd[128];
};

struct DANAE_LS_PATHWAYS {
	SavedVec3 rpos;
	s32 flag;
	u32 time;
	f32 fpadd[2];
	s32 lpadd[2];
	char cpadd[32];
};

// Lighting File Header
struct DANAE_LLF_HEADER {
	f32 version;
	char ident[16];
	char lastuser[256];
	s32 time;
	s32 nb_lights;
	s32 nb_Shadow_Polys;
	s32 nb_IGNORED_Polys;
	s32 nb_bkgpolys;
	s32 pad[256];
	f32 fpad[256];
	char cpad[4096];
	s32 bpad[256];
};

struct DANAE_LS_INTER {
	char name[512];
	SavedVec3 pos;
	SavedAnglef angle;
	s32 ident;
	s32 flags;
	s32 pad[14];
	f32 fpad[16];
};

const size_t SAVED_MAX_LINKS = 12;


#pragma pack(pop)


#endif // ARX_SCENE_LEVELFORMAT_H
