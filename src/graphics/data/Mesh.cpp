/*
 * Copyright 2011-2022 Arx Libertatis Team (see the AUTHORS file)
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
/* Based on:
===========================================================================
ARX FATALIS GPL Source Code
Copyright (C) 1999-2010 Arkane Studios SA, a ZeniMax Media company.

This file is part of the Arx Fatalis GPL Source Code ('Arx Fatalis Source Code').

Arx Fatalis Source Code is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Arx Fatalis Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Arx Fatalis Source Code.  If not, see
<http://www.gnu.org/licenses/>.

In addition, the Arx Fatalis Source Code is also subject to certain additional terms. You should have received a copy of these
additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Arx
Fatalis Source Code. If not, please request a copy in writing from Arkane Studios at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing Arkane Studios, c/o
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/
// Code: Cyril Meynier
//
// Copyright (c) 1999 ARKANE Studios SA. All rights reserved

#include "graphics/data/Mesh.h"

#include <cstdlib>
#include <cstdio>
#include <exception>
#include <limits>
#include <map>
#include <unordered_map>

#include <boost/scoped_array.hpp>

#include "ai/Anchors.h"
#include "ai/PathFinder.h"
#include "ai/PathFinderManager.h"

#include "animation/Animation.h"
#include "animation/AnimationRender.h"

#include "core/Config.h"
#include "core/Core.h"
#include "core/GameTime.h"

#include "game/EntityManager.h"
#include "game/NPC.h"
#include "game/Player.h"

#include "gui/LoadLevelScreen.h"

#include "graphics/Draw.h"
#include "graphics/Math.h"
#include "graphics/VertexBuffer.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/data/FastSceneFormat.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/texture/Texture.h"

#include "io/resource/ResourcePath.h"
#include "io/fs/FileStream.h"
#include "io/resource/PakReader.h"
#include "io/fs/Filesystem.h"
#include "io/Blast.h"
#include "io/log/Logger.h"

#include "platform/profiler/Profiler.h"

#include "scene/Interactive.h"
#include "scene/Light.h"
#include "scene/Rooms.h"
#include "scene/Scene.h"
#include "scene/Tiles.h"

#include "util/Range.h"
#include "util/String.h"

static bool IntersectLinePlane(const Vec3f & l1, const Vec3f & l2, const EERIEPOLY & ep, Vec3f * intersect) {
	
	Vec3f v = l2 - l1;
	
	float d = glm::dot(v, ep.norm);
	
	if(d != 0.0f) {
		Vec3f v1 = ep.center - l2;
		d = glm::dot(v1, ep.norm) / d;
		
		*intersect = (v * d) + l2;
		
		return true;
	}

	return false;
}

static bool RayIn3DPolyNoCull(const Vec3f & start, const Vec3f & end, const EERIEPOLY & epp) {
	
	Vec3f dir = end - start;
	
	Vec3f hit;
	if(arx::intersectLineTriangle(start, dir, epp.v[0].p, epp.v[1].p, epp.v[2].p, hit)) {
		if(hit.x >= 0.f && hit.x <= 1.f) {
			return true;
		}
	}
	
	if((epp.type & POLY_QUAD)) {
		if(arx::intersectLineTriangle(start, dir, epp.v[1].p, epp.v[3].p, epp.v[2].p, hit)) {
			if(hit.x >= 0.f && hit.x <= 1.f) {
				return true;
			}
		}
	}
	
	return false;
}

bool RayCollidingPoly(const Vec3f & orgn, const Vec3f & dest, const EERIEPOLY & ep, Vec3f * hit)
{
	if(IntersectLinePlane(orgn, dest, ep, hit)) {
		if(RayIn3DPolyNoCull(orgn, dest, ep))
			return true;
	}

	return false;
}

long MakeTopObjString(Entity * entity, std::string & dest) {
	
	if(!entity) {
		return -1;
	}
	
	EERIE_3D_BBOX box;
	box.reset();
	
	for(size_t i = 0; i < entity->obj->vertexlist.size(); i++) {
		box.add(entity->obj->vertexWorldPositions[i].v);
	}
	box.min.y -= 5.f;
	box.max.y -= 5.f;
	
	dest = "";
	
	for(const Entity & other : entities.inScene(IO_NPC | IO_ITEM)) {
		if(&other != entity) {
			if(other.pos.x > box.min.x && other.pos.x < box.max.x && other.pos.z > box.min.z && other.pos.z < box.max.z) {
				float offset = (&other == entities.player() ? 10.f : 0.f);
				if(glm::abs(other.pos.y - offset - box.min.y) < 40.f + offset) {
					if(dest.length() != 0) {
						dest += ' ';
					}
					dest += other.idString();
				}
			}
		}
	}
	
	if(dest.length() == 0) {
		dest = "none";
	}
	
	return -1;
}


EERIEPOLY * CheckInPoly(const Vec3f & poss, float * needY) {
	
	auto tile = g_tiles->getTile(poss);
	
	if(tile.y <= 0 || tile.y >= g_tiles->m_size.y - 1 || tile.x <= 0 || tile.x >= g_tiles->m_size.x - 1) {
		return nullptr;
	}
	
	float rx = poss.x - float(tile.x) * g_backgroundTileSize.x;
	float rz = poss.z - float(tile.y) * g_backgroundTileSize.y;
	
	Vec2s min = tile;
	Vec2s max = tile;
	
	(void)checked_range_cast<short>(tile.y - 1);
	(void)checked_range_cast<short>(tile.y + 1);
	
	if(rz < -40.f) {
		min.y = tile.y - 1;
		max.y = tile.y - 1;
	} else if(rz < 40.f) {
		min.y = tile.y - 1;
		max.y = tile.y;
	} else if(rz > 60.f) {
		min.y = tile.y;
		max.y = tile.y + 1;
	}
	
	(void)checked_range_cast<short>(tile.x + 1);
	(void)checked_range_cast<short>(tile.x - 1);
	
	if(rx < -40.f) {
		min.x = tile.x - 1;
		max.x = tile.x - 1;
	} else if(rx < 40.f) {
		min.x = tile.x - 1;
		max.x = tile.x;
	} else if(rx > 60.f) {
		min.x = tile.x;
		max.x = tile.x + 1;
	}
	
	EERIEPOLY * found = nullptr;
	float foundY = 0.f;
	
	// TODO why is tile.intersectingPolygons() not enough?
	for(auto neighbour : g_tiles->tilesIn(min, max + Vec2s(1))) {
		for(EERIEPOLY & polygon : neighbour.intersectingPolygons()) {
			if(poss.x >= polygon.min.x && poss.x <= polygon.max.x
			   && poss.z >= polygon.min.z && poss.z <= polygon.max.z
			   && !(polygon.type & (POLY_WATER | POLY_TRANS | POLY_NOCOL))
			   && polygon.max.y >= poss.y && &polygon != found
			   && PointIn2DPolyXZ(&polygon, poss.x, poss.z)
			   && GetTruePolyY(&polygon, poss, &rz)
			   && rz >= poss.y && (!found || rz <= foundY)) {
				found = &polygon;
				foundY = rz;
			}
		}
	}
	
	if(needY) {
		*needY = foundY;
	}
	
	return found;
}

EERIEPOLY * CheckTopPoly(const Vec3f & pos) {
	
	auto tile = g_tiles->getTile(pos);
	if(!tile) {
		return nullptr;
	}
	
	EERIEPOLY * found = nullptr;
	for(EERIEPOLY & polygon : tile.intersectingPolygons()) {
		
		if((!(polygon.type & (POLY_WATER | POLY_TRANS | POLY_NOCOL)))
		   && (polygon.min.y < pos.y)
		   && (pos.x >= polygon.min.x) && (pos.x <= polygon.max.x)
		   && (pos.z >= polygon.min.z) && (pos.z <= polygon.max.z)
		   && (PointIn2DPolyXZ(&polygon, pos.x, pos.z))) {
			
			if((glm::abs(polygon.max.y - polygon.min.y) > 50.f) && (pos.y - polygon.center.y < 60.f)) {
				continue;
			}
			
			if(polygon.tex != nullptr) {
				if(found == nullptr || polygon.min.y > found->min.y) {
					found = &polygon;
				}
			}
		}
		
	}
	
	return found;
}

bool IsAnyPolyThere(float x, float z) {
	
	auto tile = g_tiles->getTile(Vec3f(x, 0.f, z));
	if(!tile) {
		return false;
	}
	
	for(const EERIEPOLY & polygon : tile.intersectingPolygons()) {
		if(PointIn2DPolyXZ(&polygon, x, z)) {
			return true;
		}
	}
	
	return false;
}

EERIEPOLY * GetMinPoly(const Vec3f & pos) {
	
	auto tile = g_tiles->getTile(pos);
	if(!tile) {
		return nullptr;
	}
	
	EERIEPOLY * found = nullptr;
	float foundy = 0.0f;
	
	for(EERIEPOLY & polygon : tile.intersectingPolygons()) {
		
		if(polygon.type & (POLY_WATER | POLY_TRANS | POLY_NOCOL)) {
			continue;
		}
		
		if(PointIn2DPolyXZ(&polygon, pos.x, pos.z)) {
			float ret;
			if(GetTruePolyY(&polygon, pos, &ret)) {
				if(!found || ret > foundy) {
					found = &polygon;
					foundy = ret;
				}
			}
		}
		
	}
	
	return found;
}

EERIEPOLY * GetMaxPoly(const Vec3f & pos) {
	
	auto tile = g_tiles->getTile(pos);
	if(!tile) {
		return nullptr;
	}
	
	EERIEPOLY * found = nullptr;
	float foundy = 0.0f;
	
	for(EERIEPOLY & polygon : tile.intersectingPolygons()) {
		
		if(polygon.type & (POLY_WATER | POLY_TRANS | POLY_NOCOL)) {
			continue;
		}
		
		if(PointIn2DPolyXZ(&polygon, pos.x, pos.z)) {
			float ret;
			if(GetTruePolyY(&polygon, pos, &ret)) {
				if(!found || ret < foundy) {
					found = &polygon;
					foundy = ret;
				}
			}
		}
		
	}
	
	return found;
}

EERIEPOLY * EEIsUnderWater(const Vec3f & pos) {
	
	auto tile = g_tiles->getTile(pos);
	if(!tile) {
		return nullptr;
	}
	
	EERIEPOLY * found = nullptr;
	
	for(EERIEPOLY & polygon : tile.intersectingPolygons()) {
		if(polygon.type & POLY_WATER) {
			if(polygon.max.y < pos.y && PointIn2DPolyXZ(&polygon, pos.x, pos.z)) {
				if(!found || polygon.max.y < found->max.y) {
					found = &polygon;
				}
			}
		}
	}
	
	return found;
}

bool GetTruePolyY(const EERIEPOLY * ep, const Vec3f & pos, float * ret) {
	
	Vec3f n = glm::cross(ep->v[1].p - ep->v[0].p, ep->v[2].p - ep->v[0].p);
	if(n.y == 0.f) {
		return false;
	}
	
	float y = glm::dot(ep->v[0].p - toXZ(pos), n) / n.y;
	
	// Perhaps we can remove the clamp... (need to test)
	*ret = glm::clamp(y, ep->min.y, ep->max.y);
	
	return true;
}

// TODO copy-paste PortalPoly
bool GetTruePolyY(const RoomPortal & portal, const Vec3f & pos, float * ret) {
	
	Vec3f n = glm::cross(portal.p[1] - portal.p[0], portal.p[2] - portal.p[0]);
	if(n.y == 0.f) {
		return false;
	}
	
	float y = glm::dot(portal.p[0] - toXZ(pos), n) / n.y;
	
	// Perhaps we can remove the clamp... (need to test)
	*ret = glm::clamp(y, portal.minY, portal.maxY);
	
	return true;
}

Vec3f EE_RT(const Vec3f & in) {
	return Vec3f(g_preparedCamera.m_worldToView * Vec4f(in, 1.0f));
}

Vec4f worldToClipSpace(const Vec3f & in) {
	return g_preparedCamera.m_worldToScreen * Vec4f(in, 1.0f);
}

void worldToClipSpace(const Vec3f & in, TexturedVertex & out) {
	Vec4f p = worldToClipSpace(in);
	out.p = Vec3f(p);
	out.w = p.w;
}

long EERIEDrawnPolys = 0;

float PtIn2DPolyProj(const std::vector<Vec4f> & verts, const EERIE_FACE & ef, float x, float z) {
	
	Vec3f p[3];
	for(size_t i = 0; i < 3; i++) {
		if(verts[ef.vid[i]].w <= 0.f) {
			return 0.f;
		}
		p[i] = Vec3f(verts[ef.vid[i]]) / verts[ef.vid[i]].w;
	}
	
	bool c = false;
	
	for(size_t i = 0, j = 2; i < 3; j = i++) {
		if(((p[i].y <= z && z < p[j].y) || (p[j].y <= z && z < p[i].y))
		   && x < (p[j].x - p[i].x) * (z - p[i].y) / (p[j].y - p[i].y) + p[i].x) {
			c = !c;
		}
	}
	
	return c ? p[0].z : 0.f;
}

static int PointIn2DPolyXZ(const Vec3f (&verts)[4], bool isQuad, float x, float z) {
	
	int i, j, c = 0, d = 0;
	
	for(i = 0, j = 2; i < 3; j = i++) {
		if(((verts[i].z <= z && z < verts[j].z) || (verts[j].z <= z && z < verts[i].z))
		   && x < (verts[j].x - verts[i].x) * (z - verts[i].z) / (verts[j].z - verts[i].z) + verts[i].x) {
			c = !c;
		}
	}
	
	if(isQuad) {
		for(i = 1, j = 3; i < 4; j = i++) {
			if(((verts[i].z <= z && z < verts[j].z) || (verts[j].z <= z && z < verts[i].z))
			   && x < (verts[j].x - verts[i].x) * (z - verts[i].z) / (verts[j].z - verts[i].z) + verts[i].x) {
				d = !d;
			}
		}
	}
	
	return c + d;
}

int PointIn2DPolyXZ(const EERIEPOLY * ep, float x, float z) {
	Vec3f p[4] = { ep->v[0].p, ep->v[1].p, ep->v[2].p, ep->v[3].p };
	return PointIn2DPolyXZ(p, (ep->type & POLY_QUAD) != 0, x, z);
}

int PointIn2DPolyXZ(const RoomPortal & portal, float x, float z) {
	return PointIn2DPolyXZ(portal.p, true, x, z);
}

void UpdateIORoom(Entity * io)
{
	Vec3f pos = io->pos;
	pos.y -= 60.f;

	long roo = ARX_PORTALS_GetRoomNumForPosition(pos, 2);

	if(roo >= 0)
		io->room = checked_range_cast<short>(roo);

	io->requestRoomUpdate = false;
}

std::vector<ROOM_DIST_DATA> g_roomDistance;

void FreeRoomDistance() {
	g_roomDistance.clear();
}

static void SetRoomDistance(size_t i, size_t j, float val,
                            const Vec3f & p1, const Vec3f & p2) {
	
	size_t index = i + j * g_rooms->rooms.size();
	arx_assert(index < g_roomDistance.size());
	
	g_roomDistance[index].startpos = p1;
	g_roomDistance[index].endpos = p2;
	g_roomDistance[index].distance = val;
	
}

float SP_GetRoomDist(const Vec3f & pos, const Vec3f & c_pos, long io_room, long Cam_Room) {
	
	float dst = fdist(pos, c_pos);
	if(dst < 150.f) {
		return dst;
	}
	
	if(!g_rooms || g_roomDistance.empty()) {
		return dst;
	}
	
	if(io_room < 0 || size_t(io_room) >= g_rooms->rooms.size()) {
		return dst;
	}
	
	if(Cam_Room < 0 || size_t(Cam_Room) >= g_rooms->rooms.size()) {
		return dst;
	}
	
	const ROOM_DIST_DATA & dist = g_roomDistance[size_t(Cam_Room) + size_t(io_room) * g_rooms->rooms.size()];
	if(dist.distance <= 0.f) {
		return dst;
	}
	
	return fdist(c_pos, dist.startpos) + dist.distance + fdist(dist.endpos, pos);
}

void EERIE_PORTAL_Release() {
	
	delete g_rooms;
	g_rooms = nullptr;
	
}

void Draw3DObject(EERIE_3DOBJ * eobj, const Anglef & angle, const Vec3f & pos,
                  const Vec3f & scale, const Color4f & coll, RenderMaterial mat) {
	
	if(!eobj) {
		return;
	}
	
	TexturedVertex vert_list[3];
	
	glm::mat4 rotation = toRotationMatrix(angle);
	
	for(size_t i = 0; i < eobj->vertexlist.size(); i++) {
		Vec3f scaled = eobj->vertexlist[i].v * scale;
		
		Vec3f rotated = Vec3f(rotation * Vec4f(scaled, 1.f));
		
		eobj->vertexWorldPositions[i].v = (rotated += pos);

		eobj->vertexClipPositions[i] = worldToClipSpace(rotated);
	}

	for(size_t i = 0; i < eobj->facelist.size(); i++) {
		EERIE_FACE & face = eobj->facelist[i];
		
		for(size_t j = 0; j < 3; j++) {
			vert_list[j].p = Vec3f(eobj->vertexClipPositions[face.vid[j]]);
			vert_list[j].w = eobj->vertexClipPositions[face.vid[j]].w;
		}
		
		vert_list[0].uv.x = face.u[0];
		vert_list[0].uv.y = face.v[0];
		vert_list[1].uv.x = face.u[1];
		vert_list[1].uv.y = face.v[1];
		vert_list[2].uv.x = face.u[2];
		vert_list[2].uv.y = face.v[2];
		vert_list[0].color = vert_list[1].color = vert_list[2].color = coll.toRGBA();

		if(face.facetype == 0 || eobj->texturecontainer[face.texid] == nullptr)
			mat.resetTexture();
		else
			mat.setTexture(eobj->texturecontainer[face.texid]);
		
		mat.setCulling(!(face.facetype & POLY_DOUBLESIDED));
		
		g_renderBatcher.add(mat, vert_list);
	}
}

struct file_truncated_exception : public std::exception { };

template <typename T>
const T * fts_read(const char * & data, const char * end, size_t n = 1) {
	
	size_t toread = sizeof(T) * n;
	
	if(data + toread > end) {
		LogDebug(sizeof(T) << " * " << n << " > " << (end - data));
		throw file_truncated_exception();
	}
	
	const T * result = reinterpret_cast<const T *>(data);
	
	data += toread;
	
	return result;
}


static bool loadFastScene(const res::path & file, const char * data,
                          const char * end, Vec3f & trans);

bool FastSceneLoad(const res::path & partial_path, Vec3f & trans) {
	
	res::path file = "game" / partial_path / "fast.fts";
	
	
	// Load the whole file
	LogDebug("Loading " << file);
	std::string buffer = g_resources->read(file);
	if(buffer.empty()) {
		LogError << "FTS: could not read " << file;
		return false;
	}
	
	const char * data = buffer.data();
	const char * end = buffer.data() + buffer.size();
	
	size_t uncompressedSize = 0;
	{
		// Read the file header
		if(size_t(end - data) < sizeof(UNIQUE_HEADER)) {
			LogError << "Truncated FTS file " << file;
			return false;
		}
		const UNIQUE_HEADER & uh = *reinterpret_cast<const UNIQUE_HEADER *>(data);
		data += sizeof(UNIQUE_HEADER);
		if(uh.version != FTS_VERSION) {
			LogError << "FTS version mismatch: got " << uh.version << ", expected "
			         << FTS_VERSION << " in " << file;
			return false;
		}
		
		// Skip .scn file list and initialize the scene data
		if(size_t(end - data) < sizeof(UNIQUE_HEADER3) * uh.count) {
			LogError << "Truncated FTS file " << file;
			return false;
		}
		data += sizeof(UNIQUE_HEADER3) * uh.count;
		
		uncompressedSize = uh.uncompressedsize;
	}
	
	progressBarAdvance();
	LoadLevelScreen();
	
	if(uncompressedSize) {
		buffer = blast(std::string_view(data, end - data), uncompressedSize);
		if(buffer.empty()) {
			LogError << "Error decompressing scene data in " << file;
			return false;
		}
		if(buffer.size() != uncompressedSize) {
			LogWarning << "Unexpected decompressed FTS size: " << buffer.size()
			           << " != " << uncompressedSize << " in " << file;
		}
		data = buffer.data();
		end = buffer.data() + buffer.size();
	}
	
	progressBarAdvance(3.f);
	LoadLevelScreen();
	
	arx_assert(g_tiles);
	EERIE_PORTAL_Release();
	AnchorData_ClearAll();
	g_tiles->clear();
	FreeRoomDistance();
	
	progressBarAdvance();
	LoadLevelScreen();
	
	try {
		return loadFastScene(file, data, end, trans);
	} catch(const file_truncated_exception &) {
		LogError << "Truncated compressed data in FTS file " << file;
		return false;
	} catch(const std::exception & e) {
		LogError << "Error loading FTS file " << file << ": " << e.what();
		return false;
	}
	
}


static bool loadFastScene(const res::path & file, const char * data, const char * end, Vec3f & trans) {
	
	// Read the scene header
	const FAST_SCENE_HEADER * fsh = fts_read<FAST_SCENE_HEADER>(data, end);
	if(fsh->version != FTS_VERSION) {
		LogError << "FTS: version mismatch: got " << fsh->version << ", expected "
		         << FTS_VERSION << " in " << file;
		return false;
	}
	if(fsh->sizex != g_tiles->m_size.x || fsh->sizez != g_tiles->m_size.y) {
		LogError << "FTS: size mismatch in FAST_SCENE_HEADER";
		return false;
	}
	trans = fsh->Mscenepos.toVec3();
	
	// Load textures
	typedef std::map<s32, TextureContainer *> TextureContainerMap;
	TextureContainerMap textures;
	const FAST_TEXTURE_CONTAINER * ftc;
	ftc = fts_read<FAST_TEXTURE_CONTAINER>(data, end, fsh->nb_textures);
	for(long k = 0; k < fsh->nb_textures; k++) {
		res::path texture = res::path::load(util::loadString(ftc[k].fic)).remove_ext();
		TextureContainer * tmpTC = TextureContainer::Load(texture, TextureContainer::Level);
		if(tmpTC) {
			textures[ftc[k].tc] = tmpTC;
		}
	}
	progressBarAdvance(4.f);
	LoadLevelScreen();
	
	// TODO Most tiles are empty, adjust ACTIVEBKG->m_size accordingly
	
	g_tiles->resetActiveTiles();
	
	// Load cells with polygons and anchors
	LogDebug("FTS: loading " << fsh->sizex << " x " << fsh->sizez
	         << " cells ...");
	for(Vec2s tile : util::gridYX(Vec2s(0), Vec2s(fsh->sizex, fsh->sizez))) {
		
		const FAST_SCENE_INFO * fsi = fts_read<FAST_SCENE_INFO>(data, end);
		
		std::vector<EERIEPOLY> & polydata = g_tiles->get(tile).polygons();
		
		polydata.resize(fsi->nbpoly);
		for(EERIEPOLY & polygon : polydata) {
			
			const FAST_EERIEPOLY * ep = fts_read<FAST_EERIEPOLY>(data, end);
			
			polygon.room = ep->room;
			polygon.area = ep->area;
			polygon.norm = ep->norm.toVec3();
			polygon.norm2 = ep->norm2.toVec3();
			for(int l = 0; l < 4; l++) {
				polygon.nrml[l] = ep->nrml[l].toVec3();
			}
			
			if(ep->tex != 0) {
				TextureContainerMap::const_iterator cit = textures.find(ep->tex);
				polygon.tex = (cit != textures.end()) ? cit->second : nullptr;
			} else {
				polygon.tex = nullptr;
			}
			
			polygon.transval = ep->transval;
			polygon.type = PolyType::load(ep->type);
			
			for(size_t kk = 0; kk < 4; kk++) {
				polygon.v[kk].color = Color::white.toRGBA();
				polygon.v[kk].w = 1;
				polygon.v[kk].p = Vec3f(ep->v[kk].ssx, ep->v[kk].sy, ep->v[kk].ssz);
				polygon.v[kk].uv = Vec2f(ep->v[kk].stu, ep->v[kk].stv);
				polygon.color[kk] = Color::black.toRGBA();
			}
			
			size_t to = (ep->type & POLY_QUAD) ? 4 : 3;
			float div = 1.f / float(to);
			
			polygon.center = Vec3f(0.f);
			polygon.min = Vec3f(0.f);
			polygon.max = Vec3f(0.f);
			for(size_t h = 0; h < to; h++) {
				polygon.center += polygon.v[h].p;
				if(h != 0) {
					polygon.max = glm::max(polygon.max, polygon.v[h].p);
					polygon.min = glm::min(polygon.min, polygon.v[h].p);
				} else {
					polygon.min = polygon.max = polygon.v[0].p;
				}
			}
			polygon.center *= div;
			
			float dist = 0.f;
			for(size_t h = 0; h < to; h++) {
				float d = glm::distance(polygon.v[h].p, polygon.center);
				dist = std::max(dist, d);
			}
			polygon.v[0].w = dist;
			
			for(auto & index : polygon.uslInd) {
				index = 0;
			}
			
		}
		
		if(fsi->nbianchors > 0) {
			fts_read<s32>(data, end, fsi->nbianchors);
		}
		
	}
	progressBarAdvance(4.f);
	LoadLevelScreen();
	
	
	// Load anchor links
	LogDebug("FTS: loading " << fsh->nb_anchors << " anchors ...");
	g_anchors.resize(fsh->nb_anchors);
	for(ANCHOR_DATA & anchor : g_anchors) {
		
		const FAST_ANCHOR_DATA * fad = fts_read<FAST_ANCHOR_DATA>(data, end);
		
		anchor.blocked = (fad->flags & FastAnchorFlagBlocked) != 0;
		anchor.pos = fad->pos.toVec3();
		anchor.height = fad->height;
		anchor.radius = fad->radius;
		
		anchor.linked.resize(fad->nb_linked);
		if(fad->nb_linked > 0) {
			const s32 * links = fts_read<s32>(data, end, fad->nb_linked);
			std::copy(links, links + fad->nb_linked, anchor.linked.begin());
		}
		
	}
	progressBarAdvance();
	LoadLevelScreen();
	
	arx_assert(fsh->nb_rooms > 0);
	
	// Load rooms and portals
	EERIE_PORTAL_Release();
	
	g_rooms = new RoomData;
	g_rooms->rooms.resize(fsh->nb_rooms + 1);
	g_rooms->portals.resize(fsh->nb_portals);
	
	LogDebug("FTS: loading " << g_rooms->portals.size() << " portals ...");
	for(size_t portalidx = 0; portalidx < g_rooms->portals.size(); portalidx++) {
		RoomPortal & portal = g_rooms->portals[portalidx];
		
		const EERIE_SAVE_PORTALS * epo = fts_read<EERIE_SAVE_PORTALS>(data, end);
		
		if(epo->room_1 < 0 || size_t(epo->room_1) >= g_rooms->rooms.size() ||
		   epo->room_2 < 0 || size_t(epo->room_2) >= g_rooms->rooms.size()) {
			throw std::runtime_error("portal room index out of bounds");
		}
		if(epo->poly.type != 64 && epo->poly.type != 0) {
			throw std::runtime_error("invalid portal polygon type");
		}
		portal.room0 = RoomHandle(epo->room_1);
		portal.room1 = RoomHandle(epo->room_2);
		portal.useportal = epo->useportal;
		for(size_t j = 0; j < 4; j++) {
			portal.p[j] = epo->poly.v[j].pos.toVec3();
		}
		portal.bounds.radius = epo->poly.v[0].rhw;
		
		if(epo->poly.type == 0) {
			// Make sure all portal polys have 4 vertices
			// This is required to fix two polys in the original gamedata
			LogDebug("Adding position for non quad portal poly");
			portal.p[3] = glm::mix(portal.p[1], portal.p[2], 0.5f);
		}
		
		portal.plane = createNormalizedPlane(portal.p[0], portal.p[1], portal.p[2]);
		
		portal.bounds.origin = portal.p[0];
		portal.maxY = portal.minY = portal.p[0].y;
		for(long i = 1; i < 4; i++) {
			portal.bounds.origin += portal.p[i];
			portal.minY = std::min(portal.minY, portal.p[i].y);
			portal.maxY = std::max(portal.maxY, portal.p[i].y);
		}
		portal.bounds.origin /= 4.f;
		
		for(RoomHandle room : { portal.room0, portal.room1 }) {
			g_rooms->rooms[size_t(room)].portals.push_back(long(portalidx));
		}
		
	}
	
	LogDebug("FTS: loading " << g_rooms->rooms.size() << " rooms ...");
	for(Room & room : g_rooms->rooms) {
		
		const EERIE_SAVE_ROOM_DATA * erd = fts_read<EERIE_SAVE_ROOM_DATA>(data, end);
		
		LogDebug(" - room: " << erd->nb_polys << " polygons");
		
		// Portal room associations stored in the .fts files are incomplete so we have to fill in the
		// missing ones anyway and doing so without duplicates is easier when starting from scratch
		(void)fts_read<s32>(data, end, erd->nb_portals);
		
		room.epdata.resize(erd->nb_polys);
		const FAST_EP_DATA * ed = fts_read<FAST_EP_DATA>(data, end, erd->nb_polys);
		std::copy(ed, ed + erd->nb_polys, room.epdata.begin());
		
	}
	
	
	// Load distances between rooms
	FreeRoomDistance();
	if(g_rooms) {
		g_roomDistance.resize(g_rooms->rooms.size() * g_rooms->rooms.size());
		LogDebug("FTS: loading " << g_roomDistance.size() << " room distances ...");
		for(size_t n = 0; n < g_rooms->rooms.size(); n++) {
			for(size_t m = 0; m < g_rooms->rooms.size(); m++) {
				const ROOM_DIST_DATA_SAVE * rdds;
				rdds = fts_read<ROOM_DIST_DATA_SAVE>(data, end);
				SetRoomDistance(m, n, rdds->distance, rdds->startpos.toVec3(), rdds->endpos.toVec3());
			}
		}
	}
	progressBarAdvance();
	LoadLevelScreen();
	
	
	// Prepare the loaded data
	
	LogDebug("FTS: preparing scene data ...");
	
	g_tiles->computeIntersectingPolygons();
	progressBarAdvance(3.f);
	LoadLevelScreen();
	
	EERIE_PATHFINDER_Create();
	progressBarAdvance();
	LoadLevelScreen();
	
	ComputePortalVertexBuffer();
	progressBarAdvance();
	LoadLevelScreen();
	
	
	if(data != end) {
		LogWarning << "FTS: ignoring " << (end - data) << " bytes at the end of "
		           << file;
	}
	LogDebug("FTS: done loading");
	
	return true;
}

void EERIE_PORTAL_ReleaseOnlyVertexBuffer() {
	
	if(!g_rooms) {
		return;
	}
	
	if(g_rooms->rooms.empty()) {
		return;
	}
	
	LogDebug("Destroying scene VBOs");
	
	for(Room & room : g_rooms->rooms) {
		room.pVertexBuffer.reset();
		room.indexBuffer.clear();
		room.ppTextureContainer.clear();
	}
	
}

namespace {

struct SINFO_TEXTURE_VERTEX {
	
	int opaque;
	int multiplicative;
	int additive;
	int blended;
	int subtractive;
	
	SINFO_TEXTURE_VERTEX()
		: opaque(0)
		, multiplicative(0)
		, additive(0)
		, blended(0)
		, subtractive(0)
	{}
};

} // anonymous namespace

struct HasAlphaChannel {
	
	bool operator()(TextureContainer * texture) {
		return !texture || !texture->m_pTexture || !texture->m_pTexture->hasAlpha();
	}
	
};

void ComputePortalVertexBuffer() {
	
	if(!g_rooms) {
		return;
	}
	
	EERIE_PORTAL_ReleaseOnlyVertexBuffer();
	
	LogDebug("Creating scene VBOs");
	
	if(g_rooms->rooms.size() > 255) {
		LogError << "Too many rooms: " << g_rooms->rooms.size();
		return;
	}
	
	typedef std::unordered_map<TextureContainer *,  SINFO_TEXTURE_VERTEX> TextureMap;
	TextureMap infos;
	
	for(size_t i = 0; i < g_rooms->rooms.size(); i++) {
		Room * room = &g_rooms->rooms[i];
		
		// Skip empty rooms
		if(room->epdata.empty()) {
			continue;
		}
		
		infos.clear();
		
		// Count vertices / indices for each texture and blend types
		int vertexCount = 0, indexCount = 0, ignored = 0, hidden = 0, notex = 0;
		for(const EP_DATA & epd : room->epdata) {
			EERIEPOLY & poly = g_tiles->get(epd.tile).polygons()[epd.idx];
			
			if(poly.type & POLY_IGNORE) {
				ignored++;
				continue;
			}
			
			if(poly.type & POLY_HIDE) {
				hidden++;
				continue;
			}
			
			if(!poly.tex) {
				notex++;
				continue;
			}
			
			poly.tex->m_roomBatches.resize(g_rooms->rooms.size());
			
			SINFO_TEXTURE_VERTEX & info = infos[poly.tex];
			
			int nvertices = (poly.type & POLY_QUAD) ? 4 : 3;
			int nindices  = (poly.type & POLY_QUAD) ? 6 : 3;
			
			if(poly.type & POLY_TRANS) {
				
				float trans = poly.transval;
				
				if(poly.transval >= 2.f) { // multiplicative
					info.multiplicative += nindices;
					trans = trans * 0.5f + 0.5f;
				} else if(poly.transval >= 1.f) { // additive
					info.additive += nindices;
					trans -= 1.f;
				} else if(poly.transval > 0.f) { // normal trans
					info.blended += nindices;
					trans = 1.f - trans;
				} else { // subtractive
					info.subtractive += nindices;
					trans = 1.f - trans;
				}
				
				poly.v[3].color = poly.v[2].color = poly.v[1].color = poly.v[0].color = Color::gray(trans).toRGB();
				
			} else {
				info.opaque += nindices;
			}
			
			vertexCount += nvertices;
			indexCount += nindices;
		}
		
		
		if(!vertexCount) {
			LogWarning << "No visible vertices in room " << i << ": "
			           << ignored << " ignored, " << hidden << " hidden, "
			           << notex << " untextured";
			continue;
		}
		
		
		// Allocate the index buffer for this room
		room->indexBuffer.resize(indexCount);
		
		// Allocate the vertex buffer for this room
		// TODO should be static, but is updated for dynamic lighting
		room->pVertexBuffer = GRenderer->createVertexBuffer(vertexCount, Renderer::Dynamic);
		
		
		// Now fill the buffers
		
		SMY_VERTEX * vertex = room->pVertexBuffer->lock(NoOverwrite);
		
		int startIndex = 0;
		int startIndexCull = 0;
		
		size_t ntextures = infos.size();
		
		LogDebug(" - room " << i << ": " << ntextures << " textures, "
		         << vertexCount << " vertices, " << indexCount << " indices");
		
		// Allocate space to list all textures for this room
		room->ppTextureContainer.reserve(room->ppTextureContainer.size() + ntextures);
		
		for(const auto & entry : infos) {
			
			TextureContainer * texture = entry.first;
			const SINFO_TEXTURE_VERTEX & info = entry.second;
			
			unsigned short index = 0;
			
			// Upload all vertices for this texture and remember the indices
			for(const EP_DATA & epd : room->epdata) {
				EERIEPOLY & poly = g_tiles->get(epd.tile).polygons()[epd.idx];
				
				if((poly.type & POLY_IGNORE) || (poly.type & POLY_HIDE) || !poly.tex) {
					continue;
				}
				
				if(poly.tex != texture) {
					continue;
				}
				
				vertex->p.x = poly.v[0].p.x;
				vertex->p.y = poly.v[0].p.y;
				vertex->p.z = poly.v[0].p.z;
				vertex->color = poly.v[0].color;
				vertex->uv = poly.v[0].uv + texture->hd;
				vertex++;
				poly.uslInd[0] = index++;
				
				vertex->p.x = poly.v[1].p.x;
				vertex->p.y = poly.v[1].p.y;
				vertex->p.z = poly.v[1].p.z;
				vertex->color = poly.v[1].color;
				vertex->uv = poly.v[1].uv + texture->hd;
				vertex++;
				poly.uslInd[1] = index++;
				
				vertex->p.x = poly.v[2].p.x;
				vertex->p.y = poly.v[2].p.y;
				vertex->p.z = poly.v[2].p.z;
				vertex->color = poly.v[2].color;
				vertex->uv = poly.v[2].uv + texture->hd;
				vertex++;
				poly.uslInd[2] = index++;
				
				if(poly.type & POLY_QUAD) {
					vertex->p.x = poly.v[3].p.x;
					vertex->p.y = poly.v[3].p.y;
					vertex->p.z = poly.v[3].p.z;
					vertex->color = poly.v[3].color;
					vertex->uv = poly.v[3].uv + texture->hd;
					vertex++;
					poly.uslInd[3] = index++;
				}
			}
			
			// Record that the texture is used for this room
			room->ppTextureContainer.push_back(texture);
			
			// Save the
			
			SMY_ARXMAT & m = texture->m_roomBatches[i];
			
			m.uslStartVertex = startIndex;
			m.uslNbVertex = index;
			
			m.offset[BatchBucket_Opaque]         =  startIndexCull;
			m.offset[BatchBucket_Blended]        = (startIndexCull += info.opaque);
			m.offset[BatchBucket_Multiplicative] = (startIndexCull += info.blended);
			m.offset[BatchBucket_Additive]       = (startIndexCull += info.multiplicative);
			m.offset[BatchBucket_Subtractive]    = (startIndexCull += info.additive);
			                                       (startIndexCull += info.subtractive);
			
			m.count[BatchBucket_Opaque] = 0;
			m.count[BatchBucket_Blended] = 0;
			m.count[BatchBucket_Multiplicative] = 0;
			m.count[BatchBucket_Additive] = 0;
			m.count[BatchBucket_Subtractive] = 0;
			
			if(   info.opaque > 65535
			   || info.blended > 65535
			   || info.multiplicative > 65535
			   || info.additive > 65535
			   || info.subtractive > 65535
			) {
				LogWarning << "Too many indices for texture " << texture->m_texName
				           << " in room " << i;
			}
			
			startIndex += index;
		}
		
		room->pVertexBuffer->unlock();
		
		std::partition(room->ppTextureContainer.begin(), room->ppTextureContainer.end(), HasAlphaChannel());
		
	}
}
