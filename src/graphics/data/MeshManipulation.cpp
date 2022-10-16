/*
 * Copyright 2011-2021 Arx Libertatis Team (see the AUTHORS file)
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
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved

#include "graphics/data/MeshManipulation.h"

#include <stddef.h>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "game/Entity.h"

#include "graphics/BaseGraphicsTypes.h"
#include "graphics/GraphicsTypes.h"
#include "graphics/Vertex.h"
#include "graphics/data/Mesh.h"
#include "graphics/data/TextureContainer.h"

#include "io/resource/PakReader.h"
#include "io/log/Logger.h"

#include "math/Types.h"
#include "math/Vector.h"

#include "platform/Platform.h"

#include "scene/Object.h"

void EERIE_MESH_TWEAK_Skin(EERIE_3DOBJ * obj, const res::path & s1, const res::path & s2) {
	
	LogDebug("Tweak Skin " << s1 << " " << s2);
	
	if(obj == nullptr || s1.empty() || s2.empty()) {
		LogError << "Tweak Skin got NULL Pointer";
		return;
	}
	
	LogDebug("Tweak Skin " << s1 << " " << s2);
	
	res::path skintochange = "graph/obj3d/textures" / s1;
	
	res::path skinname = "graph/obj3d/textures" / s2;
	TextureContainer * tex = TextureContainer::Load(skinname);
	if(!tex) {
		return;
	}
	
	if(obj->originaltextures.empty()) {
		obj->originaltextures.reserve(obj->materials.size());
		for(TextureContainer * texture : obj->materials) {
			obj->originaltextures.emplace_back(texture ? texture->m_texName : std::string_view());
		}
	}
	
	arx_assert(obj->originaltextures.size() == obj->materials.size());
	
	bool found = false;
	
	for(MaterialId id : obj->materials.handles()) {
		if(obj->originaltextures[size_t(id)] == skintochange) {
			obj->materials[id] = tex;
			found = true;
		}
	}
	
	if(found) {
		return;
	}
	
	for(TextureContainer * & texture : obj->materials) {
		if(texture->m_texName == skintochange) {
			texture = tex;
		}
	}
	
}

bool IsInSelection(const EERIE_3DOBJ * obj, VertexId vert, VertexSelectionId tw) {
	
	if(!obj || !tw) {
		return false;
	}
	
	const EERIE_SELECTIONS & sel = obj->selections[tw];
	
	return std::find(sel.selected.begin(), sel.selected.end(), vert) != sel.selected.end();
}

static VertexId getEquivalentVertex(const EERIE_3DOBJ & obj, Vec3f vertex) {
	
	for(VertexId index : obj.vertexlist.handles()) {
		if(obj.vertexlist[index].v == vertex) {
			return index;
		}
	}
	
	return { };
}

static VertexId addVertex(EERIE_3DOBJ & obj, const EERIE_VERTEX & vertex) {
	
	if(VertexId index = getEquivalentVertex(obj, vertex.v)) {
		return index;
	}
	
	obj.vertexlist.push_back(vertex);
	
	return obj.vertexlist.last();
}

static long ObjectAddFace(EERIE_3DOBJ * obj, const EERIE_FACE * face, const EERIE_3DOBJ * srcobj) {
	
	// Check Already existing faces
	for(const EERIE_FACE & existing : obj->facelist) {
		if(obj->vertexlist[existing.vid[0]].v == srcobj->vertexlist[face->vid[0]].v &&
		   obj->vertexlist[existing.vid[1]].v == srcobj->vertexlist[face->vid[1]].v &&
		   obj->vertexlist[existing.vid[2]].v == srcobj->vertexlist[face->vid[2]].v) {
			return -1;
		}
	}
	
	EERIE_FACE & newface = obj->facelist.emplace_back(*face);
	newface.vid[0] = addVertex(*obj, srcobj->vertexlist[face->vid[0]]);
	newface.vid[1] = addVertex(*obj, srcobj->vertexlist[face->vid[1]]);
	newface.vid[2] = addVertex(*obj, srcobj->vertexlist[face->vid[2]]);
	newface.material = MaterialId(0);
	
	for(MaterialId material : obj->materials.handles()) {
		if(face->material && size_t(face->material) < srcobj->materials.size()
		   && obj->materials[material] == srcobj->materials[face->material]) {
			newface.material = material;
			break;
		}
	}
	
	return obj->facelist.size() - 1;
}

static void addNamedVertex(EERIE_3DOBJ & obj, std::string_view name, const EERIE_VERTEX & vertex) {
	
	VertexId newvert = addVertex(obj, vertex);
	
	for(const EERIE_ACTIONLIST & action : obj.actionlist) {
		if(action.name == name) {
			return;
		}
	}
	
	EERIE_ACTIONLIST & action = obj.actionlist.emplace_back();
	action.name = name;
	action.idx = newvert;
	
}

long ObjectAddMap(EERIE_3DOBJ * obj, TextureContainer * tc) {
	
	if(tc == nullptr) {
		return -1;
	}
	
	for(MaterialId id : obj->materials.handles()) {
		if(obj->materials[id] == tc) {
			return long(id);
		}
	}
	
	obj->materials.push_back(tc);
	
	return obj->materials.size() - 1;
}

static void addVertexToGroup(EERIE_3DOBJ & obj, VertexGroup & group, const EERIE_VERTEX & vertex) {
	
	for(VertexId index : obj.vertexlist.handles()) {
		if(obj.vertexlist[index].v == vertex.v) {
			if(std::find(group.indexes.begin(), group.indexes.end(), index) == group.indexes.end()) {
				group.indexes.push_back(index);
			}
		}
	}
	
}

static void copySelection(const EERIE_3DOBJ & source, VertexSelectionId sourceSelection,
                          EERIE_3DOBJ & dest, VertexSelectionId destSelection) {
	
	for(VertexId sourceVertex : source.selections[sourceSelection].selected) {
		if(VertexId destVertex = getEquivalentVertex(dest, source.vertexlist[sourceVertex].v)) {
			auto & selection = dest.selections[destSelection].selected;
			if(std::find(selection.begin(), selection.end(), destVertex) == selection.end()) {
				selection.push_back(destVertex);
			}
		}
	}
	
}

static std::unique_ptr<EERIE_3DOBJ> CreateIntermediaryMesh(const EERIE_3DOBJ * obj1, const EERIE_3DOBJ * obj2,
                                                           long tw) {
	
	VertexSelectionId sel_head1;
	VertexSelectionId sel_head2;
	VertexSelectionId sel_torso1;
	VertexSelectionId sel_torso2;
	VertexSelectionId sel_legs1;
	VertexSelectionId sel_legs2;
	
	// First we retreive selection groups indexes
	for(VertexSelectionId selection : obj1->selections.handles()) {
		if(obj1->selections[selection].name == "head") {
			sel_head1 = selection;
		} else if(obj1->selections[selection].name == "chest") {
			sel_torso1 = selection;
		} else if(obj1->selections[selection].name == "leggings") {
			sel_legs1 = selection;
		}
	}
	
	for(VertexSelectionId selection : obj2->selections.handles()) {
		if(obj2->selections[selection].name == "head") {
			sel_head2 = selection;
		} else if(obj2->selections[selection].name == "chest") {
			sel_torso2 = selection;
		} else if(obj2->selections[selection].name == "leggings") {
			sel_legs2 = selection;
		}
	}
	
	if(!sel_head1 || !sel_head2 || !sel_torso1 || !sel_torso2 || !sel_legs1 || !sel_legs2) {
		return nullptr;
	}
	
	VertexSelectionId tw1;
	VertexSelectionId tw2;
	VertexSelectionId iw1;
	VertexSelectionId jw1;
	
	if(tw == TWEAK_HEAD) {
		tw1 = sel_head1;
		tw2 = sel_head2;
		iw1 = sel_torso1;
		jw1 = sel_legs1;
	}
	
	if(tw == TWEAK_TORSO) {
		tw1 = sel_torso1;
		tw2 = sel_torso2;
		iw1 = sel_head1;
		jw1 = sel_legs1;
	}
	
	if(tw == TWEAK_LEGS) {
		tw1 = sel_legs1;
		tw2 = sel_legs2;
		iw1 = sel_torso1;
		jw1 = sel_head1;
	}
	
	if(!tw1 || !tw2) {
		return { };
	}
	
	if(!getNamedVertex(obj1, "head2chest") ||
	   !getNamedVertex(obj2, "head2chest") ||
	   !getNamedVertex(obj1, "chest2leggings") ||
	   !getNamedVertex(obj2, "chest2leggings")) {
		return { };
	}
	
	// Work will contain the Tweaked object
	std::unique_ptr<EERIE_3DOBJ> work = std::make_unique<EERIE_3DOBJ>();
	
	// Linked objects are linked to this object.
	if(obj1->linked.size() > obj2->linked.size()) {
		work->linked = obj1->linked;
	} else {
		work->linked = obj2->linked;
	}
	
	// Is the origin of object in obj1 or obj2 ? Retreives it for work object
	if(IsInSelection(obj1, obj1->origin, tw1)) {
		work->origin = addVertex(*work, obj2->vertexlist[obj2->origin]);
	} else {
		work->origin = addVertex(*work, obj1->vertexlist[obj1->origin]);
	}
	
	// Recreate Action Points included in work object.for Obj1
	for(size_t i = 0; i < obj1->actionlist.size(); i++) {
		const EERIE_ACTIONLIST & action = obj1->actionlist[i];
		if(IsInSelection(obj1, action.idx, iw1) ||
		   IsInSelection(obj1, action.idx, jw1) ||
		   action.name == "head2chest" ||
		   action.name == "chest2leggings") {
			addNamedVertex(*work, action.name, obj1->vertexlist[action.idx]);
		}
	}
	
	// Do the same for Obj2
	for(size_t i = 0; i < obj2->actionlist.size(); i++) {
		const EERIE_ACTIONLIST & action = obj2->actionlist[i];
		if(IsInSelection(obj2, action.idx, tw2) ||
		   action.name == "head2chest" ||
		   action.name == "chest2leggings") {
			addNamedVertex(*work, action.name, obj2->vertexlist[action.idx]);
		}
	}
	
	// Recreate Vertex using Obj1 Vertexes
	for(VertexId vertex : obj1->vertexlist.handles()) {
		if(IsInSelection(obj1, vertex, iw1) || IsInSelection(obj1, vertex, jw1)) {
			addVertex(*work, obj1->vertexlist[vertex]);
		}
	}
	
	// The same for Obj2
	for(VertexId vertex : obj2->vertexlist.handles()) {
		if(IsInSelection(obj2, vertex, tw2)) {
			addVertex(*work, obj2->vertexlist[vertex]);
		}
	}
	
	// Look in Faces for forgotten Vertexes... AND
	// Re-Create TextureContainers Infos
	// We look for texturecontainers included in the future tweaked object
	TextureContainer * tc = nullptr;
	for(const EERIE_FACE & face : obj1->facelist) {
		if((IsInSelection(obj1, face.vid[0], iw1) || IsInSelection(obj1, face.vid[0], jw1)) &&
		   (IsInSelection(obj1, face.vid[1], iw1) || IsInSelection(obj1, face.vid[1], jw1)) &&
		   (IsInSelection(obj1, face.vid[2], iw1) || IsInSelection(obj1, face.vid[2], jw1))) {
			if(face.material && tc != obj1->materials[face.material]) {
				tc = obj1->materials[face.material];
				ObjectAddMap(work.get(), tc);
			}
			ObjectAddFace(work.get(), &face, obj1);
		}
	}
	
	for(const EERIE_FACE & face : obj2->facelist) {
		if(IsInSelection(obj2, face.vid[0], tw2) ||
		   IsInSelection(obj2, face.vid[1], tw2) ||
		   IsInSelection(obj2, face.vid[2], tw2)) {
			if(face.material && tc != obj2->materials[face.material]) {
				tc = obj2->materials[face.material];
				ObjectAddMap(work.get(), tc);
			}
			ObjectAddFace(work.get(), &face, obj2);
		}
	}
	
	// Recreate Groups
	work->grouplist.resize(std::max(obj1->grouplist.size(), obj2->grouplist.size()));
	
	for(VertexGroupId group : obj1->grouplist.handles()) {
		work->grouplist[group].name = obj1->grouplist[group].name;
		if(VertexId vertex = getEquivalentVertex(*work, obj1->vertexlist[obj1->grouplist[group].origin].v)) {
			work->grouplist[group].m_blobShadowSize = obj1->grouplist[group].m_blobShadowSize;
			if(IsInSelection(obj1, obj1->grouplist[group].origin, iw1) ||
			   IsInSelection(obj1, obj1->grouplist[group].origin, jw1)) {
				work->grouplist[group].origin = vertex;
			}
		}
	}
	
	for(VertexGroupId group : obj2->grouplist.handles()) {
		if(size_t(group) >= obj1->grouplist.size()) {
			work->grouplist[group].name = obj2->grouplist[group].name;
		}
		if(VertexId vertex = getEquivalentVertex(*work, obj2->vertexlist[obj2->grouplist[group].origin].v)) {
			work->grouplist[group].m_blobShadowSize = obj2->grouplist[group].m_blobShadowSize;
			if(IsInSelection(obj2, obj2->grouplist[group].origin, tw2)) {
				work->grouplist[group].origin = vertex;
			}
		}
	}
	
	// Recreate Selection Groups (only the 3 selections needed to reiterate mesh tweaking)
	work->selections.resize(3);
	work->selections[VertexSelectionId(0)].name = "head";
	work->selections[VertexSelectionId(1)].name = "chest";
	work->selections[VertexSelectionId(2)].name = "leggings";
	if(tw == TWEAK_HEAD) {
		copySelection(*obj2, sel_head2, *work, VertexSelectionId(0));
	} else {
		copySelection(*obj1, sel_head1, *work, VertexSelectionId(0));
	}
	if(tw == TWEAK_TORSO) {
		copySelection(*obj2, sel_torso2, *work, VertexSelectionId(1));
	} else {
		copySelection(*obj1, sel_torso1, *work, VertexSelectionId(1));
	}
	if(tw == TWEAK_LEGS) {
		copySelection(*obj2, sel_legs2, *work, VertexSelectionId(2));
	} else {
		copySelection(*obj1, sel_legs1, *work, VertexSelectionId(2));
	}
	
	// Now recreates other selections...
	for(VertexSelectionId selection : obj1->selections.handles()) {
		
		if(EERIE_OBJECT_GetSelection(work.get(), obj1->selections[selection].name)) {
			continue;
		}
		
		work->selections.emplace_back().name = obj1->selections[selection].name;
		copySelection(*obj1, selection, *work,  work->selections.last());
		
		if(VertexSelectionId ii = EERIE_OBJECT_GetSelection(obj2, obj1->selections[selection].name)) {
			copySelection(*obj2, ii, *work,  work->selections.last());
		}
		
	}
	
	for(VertexSelectionId selection : obj2->selections.handles()) {
		
		if(EERIE_OBJECT_GetSelection(work.get(), obj2->selections[selection].name)) {
			continue;
		}
		
		work->selections.emplace_back().name = obj2->selections[selection].name;
		copySelection(*obj2, selection, *work,  work->selections.last());
		
	}
	
	// Recreate Animation-groups vertex
	for(VertexGroupId group : obj1->grouplist.handles()) {
		for(VertexId vertex : obj1->grouplist[group].indexes) {
			addVertexToGroup(*work, work->grouplist[group], obj1->vertexlist[vertex]);
		}
	}
	
	for(VertexGroupId group : obj2->grouplist.handles()) {
		for(VertexId vertex : obj2->grouplist[group].indexes) {
			addVertexToGroup(*work, work->grouplist[group], obj2->vertexlist[vertex]);
		}
	}
	
	work->vertexWorldPositions.resize(work->vertexlist.size());
	work->vertexClipPositions.resize(work->vertexlist.size());
	work->vertexColors.resize(work->vertexlist.size());
	
	return work;
}

void EERIE_MESH_TWEAK_Do(Entity * io, TweakType tw, const res::path & path) {
	
	if(!io || !io->obj) {
		return;
	}
	
	res::path ftl_file = ("game" / path).set_ext("ftl");
	if((!g_resources->getFile(ftl_file)) && (!g_resources->getFile(path))) {
		return;
	}
	
	if(path.empty() && tw == TWEAK_REMOVE) {
		if(io->tweaky) {
			delete io->obj;
			io->obj = io->tweaky;
			EERIE_Object_Precompute_Fast_Access(io->obj);
			io->tweaky = nullptr;
		}
		return;
	}
	
	if(!(tw & (TWEAK_HEAD | TWEAK_TORSO | TWEAK_LEGS))){
		return;
	}
	
	std::unique_ptr<EERIE_3DOBJ> tobj = loadObject(path);
	if(!tobj) {
		return;
	}
	
	std::unique_ptr<EERIE_3DOBJ> result;
	if(tw == (TWEAK_HEAD | TWEAK_TORSO | TWEAK_LEGS)) {
		
		result = std::move(tobj); // Replace the entire mesh
		
	} else {
		
		if(tw & TWEAK_HEAD) {
			result = CreateIntermediaryMesh(io->obj, tobj.get(), TWEAK_HEAD);
			if(!result) {
				return;
			}
		}
		if(tw & TWEAK_TORSO) {
			result = CreateIntermediaryMesh(result ? result.get() : io->obj, tobj.get(), TWEAK_TORSO);
			if(!result) {
				return;
			}
		}
		if(tw & TWEAK_LEGS) {
			result = CreateIntermediaryMesh(result ? result.get() : io->obj, tobj.get(), TWEAK_LEGS);
			if(!result) {
				return;
			}
		}
		
		EERIE_Object_Precompute_Fast_Access(result.get());
		
		EERIE_CreateCedricData(result.get());
		
		// TODO also do this for the other branch?
		io->animBlend.lastanimtime = 0;
		io->animBlend.m_active = false;
		
	}
	
	if(!io->tweaky) {
		io->tweaky = io->obj;
	} else if(io->tweaky != io->obj) {
		delete io->obj;
	}
	
	io->obj = result.release();
	
}
