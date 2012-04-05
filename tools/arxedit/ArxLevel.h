/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ARX_TOOLS_ARXEDIT_ARXLEVEL_H
#define ARX_TOOLS_ARXEDIT_ARXLEVEL_H

#include <stdint.h>

// Ogre
#include <OGRE/OgreSceneManager.h>

#include "io/fs/FilePath.h"
#include "io/resource/ResourcePath.h"

namespace Arx {
	
	typedef float       f32;
	typedef int16_t     s16;
	typedef uint16_t    u16;
	typedef int32_t     s32;
	typedef uint32_t    u32;
	
	struct SavedVec3 {
		f32 x;
		f32 y;
		f32 z;
	};
	
	struct SavedAnglef {
		f32 a;
		f32 b;
		f32 g;
	};

	struct SavedVertex {
		f32	sy;
		f32	ssx;
		f32	ssz;
		f32	stu;
		f32	stv;
	};
	
	struct SavedColor {
		f32 r;
		f32 g;
		f32 b;
	};
	
	struct LevelInfo {
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
	
	struct SceneInfo {
		char name[512];
		s32 pad[16];
		f32 fpad[16];
	};
	
	struct SceneHeader {
		char path[256];
		s32 count;
		f32 version;
		s32 uncompressedsize;
		s32 pad[3];
	};
	
	struct FastSceneHeader {
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
	
	struct FastTextureInfo {
		s32 tc;
		s32 temp;
		char fic[256];
	};
	
	struct FastSceneInfo {
		s32 nbpoly;
		s32 nbianchors;
	};
	
	struct FastPoly {
		SavedVertex v[4];
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
	
	struct RoomData{
		std::map< s32, std::vector<u32> > materialPolygons;
	};
	
	struct LevelLightsInfo {
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
	
	struct Light {
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
	
	class Level {
		
	public:
		
		Level(Ogre::SceneManager* pSceneManager);
		
		bool load(const fs::path& levelFile);
		
	private:
		
		bool loadGeometry();
		bool loadLights();
		
	private:
		
		Ogre::SceneManager* mSceneManager;
		res::path mLevelFile;
	};
}

#endif // ARX_TOOLS_ARXEDIT_ARXLEVEL_H
