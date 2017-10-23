/*
 * Copyright 2011-2017 Arx Libertatis Team (see the AUTHORS file)
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

void EERIE_Object_Precompute_Fast_Access(EERIE_3DOBJ * eerie) {
	
	if(!eerie)
		return;

	// GetActionPointIdx(eerie, "v_right");
	// GetActionPointIdx(eerie, "u_right");
	// GetActionPointIdx(eerie, "carry_attach");
	// EERIE_OBJECT_GetGroup(eerie, "jaw");
	// EERIE_OBJECT_GetGroup(eerie, "mouth all");
	
	eerie->fastaccess.view_attach       = GetActionPointIdx(eerie, "view_attach");
	eerie->fastaccess.primary_attach    = GetActionPointIdx(eerie, "primary_attach");
	eerie->fastaccess.left_attach       = GetActionPointIdx(eerie, "left_attach");
	eerie->fastaccess.weapon_attach     = GetActionPointIdx(eerie, "weapon_attach");
	eerie->fastaccess.secondary_attach  = GetActionPointIdx(eerie, "secondary_attach");
	eerie->fastaccess.fire              = GetActionPointIdx(eerie, "fire");
	
	eerie->fastaccess.head_group = EERIE_OBJECT_GetGroup(eerie, "head");

	if(eerie->fastaccess.head_group != ObjVertGroup()) {
		ObjVertHandle lHeadOrigin = ObjVertHandle(eerie->grouplist[eerie->fastaccess.head_group.handleData()].origin);
		eerie->fastaccess.head_group_origin = lHeadOrigin;
	}
	
	eerie->fastaccess.sel_head     = EERIE_OBJECT_GetSelection(eerie, "head");
	eerie->fastaccess.sel_chest    = EERIE_OBJECT_GetSelection(eerie, "chest");
	eerie->fastaccess.sel_leggings = EERIE_OBJECT_GetSelection(eerie, "leggings");
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
	
		point0 = pos = Vec3f_ZERO;
		angle = Anglef::ZERO;

		origin = 0;

		vertexlocal = NULL;
		vertexlist.clear();
		vertexWorldPositions.clear();

		facelist.clear();
		grouplist.clear();
		texturecontainer.clear();

		originaltextures = NULL;
		
		quat = glm::quat();
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
	
	free(originaltextures);
	originaltextures = NULL;
	
	EERIE_RemoveCedricData(this);
	EERIE_PHYSICS_BOX_Release(this);
	
	grouplist.clear();
	linked.clear();
}

EERIE_3DOBJ * Eerie_Copy(const EERIE_3DOBJ * obj) {
	
	EERIE_3DOBJ * nouvo = new EERIE_3DOBJ();
	
	nouvo->vertexlist = obj->vertexlist;
	nouvo->vertexWorldPositions.resize(nouvo->vertexlist.size());
	nouvo->vertexClipPositions.resize(nouvo->vertexlist.size());
	nouvo->vertexColors.resize(nouvo->vertexlist.size());
	
	nouvo->pbox = NULL;
	nouvo->m_skeleton = NULL;
	nouvo->vertexlocal = NULL;
	
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
	nouvo->originaltextures = NULL;
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
	delete[] eobj->vertexlocal, eobj->vertexlocal = NULL;
}

void EERIE_CreateCedricData(EERIE_3DOBJ * eobj) {
	
	eobj->m_skeleton = new Skeleton();

	if(eobj->grouplist.size() <= 0) {
		// If no groups were specified

		// Make one bone
		eobj->m_skeleton->bones.resize(1);
		
		Bone & bone = eobj->m_skeleton->bones[0];

		// Add all vertices to the bone
		for(size_t i = 0; i < eobj->vertexlist.size(); i++) {
			bone.idxvertices.push_back(i);
		}

		// Initialize the bone
		bone.init.quat = glm::quat();
		bone.anim.quat = glm::quat();
		bone.init.scale = Vec3f_ZERO;
		bone.anim.scale = Vec3f_ZERO;
		bone.init.trans = Vec3f_ZERO;
		bone.transinit_global = bone.init.trans;
		bone.father = -1;
	} else {
		// Groups were specified

		// Alloc the bones
		eobj->m_skeleton->bones.resize(eobj->grouplist.size());
		
		bool * temp = new bool[eobj->vertexlist.size()];
		memset(temp, 0, eobj->vertexlist.size());

		for(long i = eobj->grouplist.size() - 1; i >= 0; i--) {
			VertexGroup & group = eobj->grouplist[i];
			Bone & bone = eobj->m_skeleton->bones[i];

			EERIE_VERTEX * v_origin = &eobj->vertexlist[group.origin];

			for(size_t j = 0; j < group.indexes.size(); j++) {
				if(!temp[group.indexes[j]]) {
					temp[group.indexes[j]] = true;
					bone.idxvertices.push_back(group.indexes[j]);
				}
			}

			bone.init.quat = glm::quat();
			bone.anim.quat = glm::quat();
			bone.init.scale = Vec3f_ZERO;
			bone.anim.scale = Vec3f_ZERO;
			bone.init.trans = Vec3f(v_origin->v.x, v_origin->v.y, v_origin->v.z);
			bone.transinit_global = bone.init.trans;
			bone.father = GetFather(eobj, group.origin, i - 1);
		}

		delete[] temp;

		// Try to correct lonely vertex
		for(size_t i = 0; i < eobj->vertexlist.size(); i++) {
			long ok = 0;

			for(size_t j = 0; j < eobj->grouplist.size(); j++) {
				for(size_t k = 0; k < eobj->grouplist[j].indexes.size(); k++) {
					if(eobj->grouplist[j].indexes[k] == i) {
						ok = 1;
						break;
					}
				}

				if(ok)
					break;
			}

			if(!ok) {
				eobj->m_skeleton->bones[0].idxvertices.push_back(i);
			}
		}
		
		for(long i = eobj->grouplist.size() - 1; i >= 0; i--) {
			Bone & bone = eobj->m_skeleton->bones[i];

			if(bone.father >= 0) {
				long father = bone.father;
				bone.init.trans -= eobj->m_skeleton->bones[father].init.trans;
			}
			bone.transinit_global = bone.init.trans;
		}

	}

	/* Build proper mesh */
	{
		Skeleton* obj = eobj->m_skeleton;

		for(size_t i = 0; i != obj->bones.size(); i++) {
			Bone & bone = obj->bones[i];

			if(bone.father >= 0) {
				size_t parentIndex = size_t(bone.father);
				Bone & parent = obj->bones[parentIndex];
				/* Rotation*/
				bone.anim.quat = parent.anim.quat * bone.init.quat;
				/* Translation */
				bone.anim.trans = parent.anim.quat * bone.init.trans;
				bone.anim.trans = parent.anim.trans + bone.anim.trans;
			} else {
				/* Rotation*/
				bone.anim.quat = bone.init.quat;
				/* Translation */
				bone.anim.trans = bone.init.trans;
			}
			bone.anim.scale = Vec3f_ONE;
		}

		eobj->vertexlocal = new Vec3f[eobj->vertexlist.size()];
		// TODO constructor is better than memset
		memset(eobj->vertexlocal, 0, sizeof(Vec3f)*eobj->vertexlist.size());

		for(size_t i = 0; i != obj->bones.size(); i++) {
			const Bone & bone = obj->bones[i];
			Vec3f vector = bone.anim.trans;
			
			for(size_t v = 0; v != bone.idxvertices.size(); v++) {
				
				size_t idx = bone.idxvertices[v];
				const EERIE_VERTEX & inVert = eobj->vertexlist[idx];
				Vec3f & outVert = eobj->vertexlocal[idx];
				
				Vec3f temp = inVert.v - vector;
				outVert = glm::inverse(bone.anim.quat) * temp;
			}
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
