/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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

#include "scene/Object.h"

#include <cstdio>
#include <vector>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/foreach.hpp>

#include "core/Config.h"
#include "core/Core.h"

#include "game/Entity.h"
#include "game/EntityManager.h"

#include "graphics/GraphicsTypes.h"
#include "graphics/Math.h"
#include "graphics/data/FTL.h"
#include "graphics/data/TextureContainer.h"

#include "io/fs/FilePath.h"
#include "io/fs/SystemPaths.h"
#include "io/resource/ResourcePath.h"
#include "io/resource/PakReader.h"
#include "io/log/Logger.h"

#include "physics/CollisionShapes.h"
#include "physics/Physics.h"

#include "scene/LinkedObject.h"
#include "scene/GameSound.h"
#include "scene/Interactive.h"
#include "scene/Light.h"

#include "util/String.h"


void EERIE_RemoveCedricData(EERIE_3DOBJ * eobj);

ObjVertHandle GetGroupOriginByName(const EERIE_3DOBJ * eobj, const std::string & text) {
	
	if(!eobj)
		return ObjVertHandle();
	
	BOOST_FOREACH(const VertexGroup & group, eobj->grouplist) {
		if(group.name == text) {
			return ObjVertHandle(group.origin);
		}
	}
	
	return ObjVertHandle();
}

ActionPoint GetActionPointIdx(const EERIE_3DOBJ * eobj, const std::string & text) {
	
	if(!eobj)
		return ActionPoint();
	
	BOOST_FOREACH(const EERIE_ACTIONLIST & action, eobj->actionlist) {
		if(action.name == text) {
			return action.idx;
		}
	}
	
	return ActionPoint();
}

ObjVertGroup GetActionPointGroup(const EERIE_3DOBJ * eobj, ActionPoint idx) {
	
	if(!eobj)
		return ObjVertGroup();
	
	for(long i = eobj->grouplist.size() - 1; i >= 0; i--) {
		const std::vector<u32> & indices = eobj->grouplist[i].indexes;
		for(size_t j = 0; j < indices.size(); j++){
			if(long(indices[j]) == idx.handleData()) {
				return ObjVertGroup(i);
			}
		}
	}
	
	return ObjVertGroup();
}

void EERIE_Object_Precompute_Fast_Access(EERIE_3DOBJ * object) {
	
	if(!object) {
		return;
	}
	
	object->fastaccess.view_attach       = GetActionPointIdx(object, "view_attach");
	object->fastaccess.primary_attach    = GetActionPointIdx(object, "primary_attach");
	object->fastaccess.left_attach       = GetActionPointIdx(object, "left_attach");
	object->fastaccess.weapon_attach     = GetActionPointIdx(object, "weapon_attach");
	object->fastaccess.secondary_attach  = GetActionPointIdx(object, "secondary_attach");
	object->fastaccess.fire              = GetActionPointIdx(object, "fire");
	
	object->fastaccess.head_group = EERIE_OBJECT_GetGroup(object, "head");
	
	if(object->fastaccess.head_group != ObjVertGroup()) {
		ObjVertHandle lHeadOrigin = ObjVertHandle(object->grouplist[object->fastaccess.head_group.handleData()].origin);
		object->fastaccess.head_group_origin = lHeadOrigin;
	}
	
	object->fastaccess.sel_head     = EERIE_OBJECT_GetSelection(object, "head");
	object->fastaccess.sel_chest    = EERIE_OBJECT_GetSelection(object, "chest");
	object->fastaccess.sel_leggings = EERIE_OBJECT_GetSelection(object, "leggings");
}

void MakeUserFlag(TextureContainer * tc) {
	
	if(!tc)
		return;
	
	const std::string & tex = tc->m_texName.string();
	
	if(boost::contains(tex, "npc_")) {
		tc->userflags |= POLY_LATE_MIP;
	}
	
	if(boost::contains(tex, "nocol")) {
		tc->userflags |= POLY_NOCOL;
	}
	
	if(boost::contains(tex, "climb")) {
		tc->userflags |= POLY_CLIMB;
	}
	
	if(boost::contains(tex, "fall")) {
		tc->userflags |= POLY_FALL;
	}
	
	if(boost::contains(tex, "lava")) {
		tc->userflags |= POLY_LAVA;
	}
	
	if(boost::contains(tex, "water") || boost::contains(tex, "spider_web")) {
		tc->userflags |= POLY_WATER;
		tc->userflags |= POLY_TRANS;
	} else if(boost::contains(tex, "[metal]")) {
		tc->userflags |= POLY_METAL;
	}
	
}

//-----------------------------------------------------------------------------------------------------
// Warning Clear3DObj don't release Any pointer Just Clears Structures
void EERIE_3DOBJ::clear() {
	
		point0 = pos = Vec3f(0.f);
		angle = Anglef();

		origin = 0;

		vertexlocal.clear();
		vertexlist.clear();
		vertexWorldPositions.clear();

		facelist.clear();
		grouplist.clear();
		texturecontainer.clear();

		originaltextures.clear();
		
		quat = quat_identity();
		linked.clear();

		pbox = 0;
		sdata = false;
		
		fastaccess = EERIE_FASTACCESS();
		
		m_skeleton = 0;
		
	cub.xmin = cub.ymin = cub.zmin = std::numeric_limits<float>::max();
	cub.xmax = cub.ymax = cub.zmax = std::numeric_limits<float>::min();
}

// TODO move to destructor?
EERIE_3DOBJ::~EERIE_3DOBJ() {
	EERIE_RemoveCedricData(this);
	EERIE_PHYSICS_BOX_Release(this);
}

EERIE_3DOBJ * Eerie_Copy(const EERIE_3DOBJ * obj) {
	
	EERIE_3DOBJ * nouvo = new EERIE_3DOBJ();
	
	nouvo->vertexlist = obj->vertexlist;
	nouvo->vertexWorldPositions.resize(nouvo->vertexlist.size());
	nouvo->vertexClipPositions.resize(nouvo->vertexlist.size());
	nouvo->vertexColors.resize(nouvo->vertexlist.size());
	
	nouvo->pbox = NULL;
	nouvo->m_skeleton = NULL;
	
	nouvo->angle = obj->angle;
	nouvo->pos = obj->pos;
	nouvo->cub.xmax = obj->cub.xmax;
	nouvo->cub.xmin = obj->cub.xmin;
	nouvo->cub.ymax = obj->cub.ymax;
	nouvo->cub.ymin = obj->cub.ymin;
	nouvo->cub.zmax = obj->cub.zmax;
	nouvo->cub.zmin = obj->cub.zmin;
	
	if(!obj->file.empty())
		nouvo->file = obj->file;
	
	if(!obj->name.empty())
		nouvo->name = obj->name;

	nouvo->origin = obj->origin;
	nouvo->point0 = obj->point0;
	nouvo->quat = obj->quat;
	
	nouvo->facelist = obj->facelist;
	nouvo->grouplist = obj->grouplist;
	nouvo->actionlist = obj->actionlist;
	nouvo->selections = obj->selections;
	nouvo->texturecontainer = obj->texturecontainer;
	nouvo->fastaccess = obj->fastaccess;
	
	EERIE_CreateCedricData(nouvo);

	if(obj->pbox) {
		nouvo->pbox = new PHYSICS_BOX_DATA();
		nouvo->pbox->stopcount = 0;
		nouvo->pbox->radius = obj->pbox->radius;
		
		nouvo->pbox->vert = obj->pbox->vert;
	}
	
	nouvo->linked.clear();
	nouvo->originaltextures.clear();
	
	return nouvo;
}

ObjSelection EERIE_OBJECT_GetSelection(const EERIE_3DOBJ * obj, const std::string & selname) {
	
	if(!obj)
		return ObjSelection();
	
	for(size_t i = 0; i < obj->selections.size(); i++) {
		if(obj->selections[i].name == selname) {
			return ObjSelection(i);
		}
	}
	
	return ObjSelection();
}

ObjVertGroup EERIE_OBJECT_GetGroup(const EERIE_3DOBJ * obj, const std::string & groupname) {
	
	if(!obj)
		return ObjVertGroup();
	
	for(size_t i = 0; i < obj->grouplist.size(); i++) {
		if(obj->grouplist[i].name == groupname) {
			return ObjVertGroup(i);
		}
	}
	
	return ObjVertGroup();
}

static long GetFather(EERIE_3DOBJ * eobj, size_t origin, long startgroup) {
	
	for(long i = startgroup; i >= 0; i--) {
		for(size_t j = 0; j < eobj->grouplist[i].indexes.size(); j++) {
			if(eobj->grouplist[i].indexes[j] == origin) {
				return i;
			}
		}
	}

	return -1;
}

void EERIE_RemoveCedricData(EERIE_3DOBJ * eobj) {
	
	if(!eobj || !eobj->m_skeleton)
		return;
	
	delete eobj->m_skeleton, eobj->m_skeleton = NULL;
	eobj->vertexlocal.clear();
	
}

void EERIE_CreateCedricData(EERIE_3DOBJ * eobj) {
	
	eobj->m_skeleton = new Skeleton();
	
	if(eobj->grouplist.empty()) {
		// If no groups were specified
		
		eobj->m_skeleton->bones.resize(1);
		
		Bone & bone = eobj->m_skeleton->bones[0];
		
		// Add all vertices to the bone
		for(size_t i = 0; i < eobj->vertexlist.size(); i++) {
			bone.idxvertices.push_back(i);
		}
		
		bone.father = -1;
		bone.anim.scale = Vec3f(1.f);
		
	} else {
		// Groups were specified
		
		eobj->m_skeleton->bones.resize(eobj->grouplist.size());
		
		// Create one bone for each vertex group and assign vertices to the inner-most group
		std::vector<bool> vertexAssigned(eobj->vertexlist.size(), false);
		for(long i = eobj->grouplist.size() - 1; i >= 0; i--) {
			VertexGroup & group = eobj->grouplist[i];
			Bone & bone = eobj->m_skeleton->bones[i];
			
			for(size_t j = 0; j < group.indexes.size(); j++) {
				if(!vertexAssigned[group.indexes[j]]) {
					vertexAssigned[group.indexes[j]] = true;
					bone.idxvertices.push_back(group.indexes[j]);
				}
			}
			
			bone.anim.trans = eobj->vertexlist[group.origin].v;
			bone.father = GetFather(eobj, group.origin, i - 1);
			bone.anim.scale = Vec3f(1.f);
			
		}
		
		// Assign vertices that are not in any group to the root bone
		for(size_t i = 0; i < eobj->vertexlist.size(); i++) {
			bool found = false;
			BOOST_FOREACH(const VertexGroup & group, eobj->grouplist) {
				BOOST_FOREACH(u32 index, group.indexes) {
					if(index == i) {
						found = true;
						break;
					}
				}
				if(found) {
					break;
				}
			}
			if(!found) {
				eobj->m_skeleton->bones[0].idxvertices.push_back(i);
			}
		}
		
		// Calculate relative bone positions
		BOOST_FOREACH(Bone & bone, eobj->m_skeleton->bones) {
			if(bone.father >= 0) {
				Bone & parent = eobj->m_skeleton->bones[size_t(bone.father)];
				bone.transinit_global = bone.init.trans = bone.anim.trans - parent.anim.trans;
			} else {
				bone.transinit_global = bone.init.trans = bone.anim.trans;
			}
		}
		
	}
	
	// Calculate relative vertex positions
	eobj->vertexlocal.resize(eobj->vertexlist.size());
	BOOST_FOREACH(const Bone & bone, eobj->m_skeleton->bones) {
		BOOST_FOREACH(u32 index, bone.idxvertices) {
			eobj->vertexlocal[index] = eobj->vertexlist[index].v - bone.anim.trans;
		}
	}

}

EERIE_3DOBJ * loadObject(const res::path & file, bool pbox) {
	
	EERIE_3DOBJ * ret = ARX_FTL_Load(file);
	if(ret && pbox) {
		EERIE_PHYSICS_BOX_Create(ret);
	}
	
	return ret;
}

void EERIE_OBJECT_CenterObjectCoordinates(EERIE_3DOBJ * ret)
{
	if (!ret) return;

	Vec3f offset = ret->vertexlist[ret->origin].v;

	if ((offset.x == 0) && (offset.y == 0) && (offset.z == 0))
		return;

	LogWarning << "NOT CENTERED " << ret->file;
	
	for(size_t i = 0; i < ret->vertexlist.size(); i++) {
		ret->vertexlist[i].v -= offset;
	}
	
	ret->point0 -= offset;
}
