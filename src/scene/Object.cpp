/*
 * Copyright 2011-2013 Arx Libertatis Team (see the AUTHORS file)
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

#include "core/Config.h"
#include "core/Core.h"

#include "game/Entity.h"
#include "game/EntityManager.h"

#include "graphics/GraphicsTypes.h"
#include "graphics/Math.h"
#include "graphics/data/Progressive.h"
#include "graphics/data/FTL.h"
#include "graphics/data/TextureContainer.h"

#include "io/fs/FilePath.h"
#include "io/fs/SystemPaths.h"
#include "io/resource/ResourcePath.h"
#include "io/resource/PakReader.h"
#include "io/log/Logger.h"

#include "physics/Clothes.h"
#include "physics/Box.h"
#include "physics/CollisionShapes.h"

#include "scene/LinkedObject.h"
#include "scene/GameSound.h"
#include "scene/ObjectFormat.h"
#include "scene/Interactive.h"
#include "scene/Light.h"

#include "util/String.h"

using std::sprintf;
using std::min;
using std::max;
using std::string;
using std::vector;

void EERIE_RemoveCedricData(EERIE_3DOBJ * eobj);

void Clear3DScene(EERIE_3DSCENE	* eerie);

long GetGroupOriginByName(const EERIE_3DOBJ * eobj, const string & text) {
	
	if(!eobj)
		return -1;
	
	for(long i = 0; i < eobj->nbgroups; i++) {
		if(eobj->grouplist[i].name == text) {
			return eobj->grouplist[i].origin;
		}
	}
	
	return -1;
}

long GetActionPointIdx(const EERIE_3DOBJ * eobj, const string & text) {
	
	if(!eobj)
		return -1;
	
	for(vector<EERIE_ACTIONLIST>::const_iterator i = eobj->actionlist.begin();
	    i != eobj->actionlist.end(); ++i) {
		if(i->name == text) {
			return i->idx;
		}
	}
	
	return -1;
}

long GetActionPointGroup(const EERIE_3DOBJ * eobj, long idx) {
	
	if(!eobj)
		return -1;
	
	for(long i = eobj->nbgroups - 1; i >= 0; i--) {
		const vector<long> & indices = eobj->grouplist[i].indexes;
		for(size_t j = 0; j < indices.size(); j++){
			if(indices[j] == idx) {
				return i;
			}
		}
	}
	
	return -1;
}

void EERIE_Object_Precompute_Fast_Access(EERIE_3DOBJ * eerie) {
	
	if(!eerie)
		return;

	long lVRight		=	GetActionPointIdx(eerie, "v_right");
	long lURight		=	GetActionPointIdx(eerie, "u_right");
	long lViewAttach	=	GetActionPointIdx(eerie, "view_attach") ;
	long lPrimAttach	=	GetActionPointIdx(eerie, "primary_attach");
	long lLeftAttach	=	GetActionPointIdx(eerie, "left_attach");

	eerie->fastaccess.V_right = checked_range_cast<short>(lVRight);
	eerie->fastaccess.U_right = checked_range_cast<short>(lURight);
	eerie->fastaccess.view_attach = checked_range_cast<short>(lViewAttach);
	eerie->fastaccess.primary_attach = checked_range_cast<short>(lPrimAttach);
	eerie->fastaccess.left_attach = checked_range_cast<short>(lLeftAttach);


	long lWeapAttach				=	GetActionPointIdx(eerie, "weapon_attach");
	long lSecAttach					=	GetActionPointIdx(eerie, "secondary_attach");
	long lJaw						=	EERIE_OBJECT_GetGroup(eerie, "jaw");
	long lMouthAll					=	EERIE_OBJECT_GetGroup(eerie, "mouth all");

	eerie->fastaccess.weapon_attach		=	checked_range_cast<short>(lWeapAttach);
	eerie->fastaccess.secondary_attach	=	checked_range_cast<short>(lSecAttach);
	eerie->fastaccess.jaw_group			=	checked_range_cast<short>(lJaw);
	eerie->fastaccess.mouth_group		=	checked_range_cast<short>(lMouthAll);


	if(eerie->fastaccess.mouth_group == -1)
		eerie->fastaccess.mouth_group_origin = -1;
	else
	{
		long lMouthOrigin = eerie->grouplist[eerie->fastaccess.mouth_group].origin;
		eerie->fastaccess.mouth_group_origin = checked_range_cast<short>(lMouthOrigin);
	}

	long lHeadGroup					=	EERIE_OBJECT_GetGroup(eerie, "head");
	eerie->fastaccess.head_group	=	checked_range_cast<short>(lHeadGroup);

	if(eerie->fastaccess.head_group == -1)
		eerie->fastaccess.head_group_origin = -1;
	else
	{
		long lHeadOrigin  = eerie->grouplist[eerie->fastaccess.head_group].origin;
		eerie->fastaccess.head_group_origin = checked_range_cast<short>(lHeadOrigin);
	}


	long lFire = GetActionPointIdx(eerie, "fire");
	long lCarryAttach = GetActionPointIdx(eerie, "carry_attach");
	long lHead = EERIE_OBJECT_GetSelection(eerie, "head");
	long lChest = EERIE_OBJECT_GetSelection(eerie, "chest");
	long lLeggings = EERIE_OBJECT_GetSelection(eerie, "leggings") ;
	
	eerie->fastaccess.fire = checked_range_cast<short>(lFire);
	eerie->fastaccess.carry_attach = checked_range_cast<short>(lCarryAttach);
	eerie->fastaccess.sel_head = checked_range_cast<short>(lHead);
	eerie->fastaccess.sel_chest = checked_range_cast<short>(lChest);
	eerie->fastaccess.sel_leggings = checked_range_cast<short>(lLeggings);
}

void MakeUserFlag(TextureContainer * tc) {
	
	if(!tc)
		return;
	
	const string & tex = tc->m_texName.string();
	
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

#if BUILD_EDIT_LOADSAVE

static void ReCreateUVs(EERIE_3DOBJ * eerie) {
	
	if(eerie->texturecontainer.empty())
		return;
	
	for(size_t i = 0; i < eerie->facelist.size(); i++) {
		
		if(eerie->facelist[i].texid == -1)
			continue;
		
		TextureContainer * tex = eerie->texturecontainer[eerie->facelist[i].texid];
		Vec2f scale = (tex) ? Vec2f(1.f / tex->m_dwWidth, 1.f / tex->m_dwHeight) : (Vec2f_ONE / Vec2f(256));
		
		eerie->facelist[i].u[0] = (float)eerie->facelist[i].ou[0] * scale.x; 
		eerie->facelist[i].u[1] = (float)eerie->facelist[i].ou[1] * scale.x; 
		eerie->facelist[i].u[2] = (float)eerie->facelist[i].ou[2] * scale.x; 
		eerie->facelist[i].v[0] = (float)eerie->facelist[i].ov[0] * scale.y; 
		eerie->facelist[i].v[1] = (float)eerie->facelist[i].ov[1] * scale.y; 
		eerie->facelist[i].v[2] = (float)eerie->facelist[i].ov[2] * scale.y; 
	}
}

static void loadObjectData(EERIE_3DOBJ * eerie, const char * adr, size_t * poss, long version) {
	
	LogWarning << "loadObjectData";
	
	size_t pos = *poss;
	
	const THEO_OFFSETS * to = reinterpret_cast<const THEO_OFFSETS *>(adr + pos);
	pos += sizeof(THEO_OFFSETS);
	
	const THEO_NB * tn = reinterpret_cast<const THEO_NB *>(adr + pos);
	
	LogDebug("Nb Vertex " << tn->nb_vertex << " Nb Action Points " << tn->nb_action_point
	         << " Nb Lines " << tn->nb_lines);
	LogDebug("Nb Faces " << tn->nb_faces << " Nb Groups " << tn->nb_groups);
	
	eerie->vertexlist.resize(tn->nb_vertex);
	eerie->facelist.resize(tn->nb_faces);
	eerie->nbgroups = tn->nb_groups;
	eerie->actionlist.resize(tn->nb_action_point);
	
	eerie->ndata = NULL;
	eerie->pdata = NULL;
	eerie->cdata = NULL;
	eerie->sdata = NULL;
	
	eerie->vertexlist3.resize(tn->nb_vertex);
	
	if(tn->nb_groups == 0) {
		eerie->grouplist = NULL;
	} else {
		eerie->grouplist = new EERIE_GROUPLIST[tn->nb_groups]; 
	}
	
	// read vertices
	
	pos = to->vertex_seek;
	
	if(tn->nb_vertex > 65535) {
		LogError << ("Warning Vertex Number Too High...");
	}
	
	for(long i = 0; i < tn->nb_vertex; i++) {
		const THEO_VERTEX * ptv = reinterpret_cast<const THEO_VERTEX *>(adr + pos);
		pos += sizeof(THEO_VERTEX);
		eerie->vertexlist[i].v = ptv->pos.toVec3();
		eerie->cub.xmin = min(eerie->cub.xmin, ptv->pos.x);
		eerie->cub.xmax = max(eerie->cub.xmax, ptv->pos.x);
		eerie->cub.ymin = min(eerie->cub.ymin, ptv->pos.y);
		eerie->cub.ymax = max(eerie->cub.ymax, ptv->pos.y);
		eerie->cub.zmin = min(eerie->cub.zmin, ptv->pos.z);
		eerie->cub.zmax = max(eerie->cub.zmax, ptv->pos.z);
	}

	// Lecture des FACES THEO
	pos = to->faces_seek;

	for(long i = 0; i < tn->nb_faces; i++) {
		
		THEO_FACES_3006 tf3006;
		const THEO_FACES_3006 * ptf3006;
		if(version >= 3006) {
			ptf3006 = reinterpret_cast<const THEO_FACES_3006 *>(adr + pos);
			pos += sizeof(THEO_FACES_3006);
		} else {
			memset(&tf3006, 0, sizeof(THEO_FACES_3006));
			const THEO_FACES * ptf = reinterpret_cast<const THEO_FACES *>(adr + pos);
			pos += sizeof(THEO_FACES);
			tf3006.color = ptf->color;
			tf3006.index1 = ptf->index1;
			tf3006.index2 = ptf->index2;
			tf3006.index3 = ptf->index3;
			tf3006.ismap = ptf->ismap;
			tf3006.liste_uv = ptf->liste_uv;
			tf3006.element_uv = ptf->element_uv;
			tf3006.num_map = ptf->num_map;
			tf3006.tile_x = ptf->tile_x;
			tf3006.tile_y = ptf->tile_y;
			tf3006.user_tile_x = ptf->user_tile_x;
			tf3006.user_tile_y = ptf->user_tile_y;
			tf3006.flag = ptf->flag;
			tf3006.collision_type = ptf->collision_type;
			tf3006.rgb = ptf->rgb;
			tf3006.rgb1 = ptf->rgb1;
			tf3006.rgb2 = ptf->rgb2;
			tf3006.rgb3 = ptf->rgb3;
			tf3006.double_side = ptf->double_side;
			tf3006.transparency = ptf->transparency;
			tf3006.trans = ptf->trans;
			ptf3006 = &tf3006;
		}
		
		eerie->facelist[i].vid[0] = (unsigned short)ptf3006->index1;
		eerie->facelist[i].vid[1] = (unsigned short)ptf3006->index2;
		eerie->facelist[i].vid[2] = (unsigned short)ptf3006->index3;
		
		s32 num_map = ((size_t)ptf3006->num_map >= eerie->texturecontainer.size()) ? -1 : ptf3006->num_map;
		
		if(ptf3006->ismap) {
			eerie->facelist[i].texid = (short)num_map;
			eerie->facelist[i].facetype = POLY_NO_SHADOW;
			
			if(num_map >= 0 && eerie->texturecontainer[num_map] && (eerie->texturecontainer[num_map]->userflags & POLY_NOCOL)) {
				eerie->facelist[i].facetype |= POLY_NOCOL;
			}
		} else if(ptf3006->rgb) {
			eerie->facelist[i].texid = -1;
		} else {
			eerie->facelist[i].texid = -1;
		}
		
		switch(ptf3006->flag) {
			case 0:
				eerie->facelist[i].facetype |= POLY_GLOW;
				break;
			case 1:
				eerie->facelist[i].facetype |= POLY_NO_SHADOW;
				break;
			case 4:
				eerie->facelist[i].facetype |= POLY_METAL;
				break;
			case 10:
				eerie->facelist[i].facetype |= POLY_NOPATH;
				break;
			case 11:
				eerie->facelist[i].facetype |= POLY_CLIMB;
				break;
			case 12:
				eerie->facelist[i].facetype |= POLY_NOCOL;
				break;
			case 13:
				eerie->facelist[i].facetype |= POLY_NODRAW;
				break;
			case 14:
				eerie->facelist[i].facetype |= POLY_PRECISE_PATH;
				break;
			case 16:
				eerie->facelist[i].facetype |= POLY_NO_CLIMB;
				break;
		}
		
		eerie->facelist[i].ou[0] = (short)ptf3006->liste_uv.u1;
		eerie->facelist[i].ov[0] = (short)ptf3006->liste_uv.v1;
		eerie->facelist[i].ou[1] = (short)ptf3006->liste_uv.u2;
		eerie->facelist[i].ov[1] = (short)ptf3006->liste_uv.v2;
		eerie->facelist[i].ou[2] = (short)ptf3006->liste_uv.u3;
		eerie->facelist[i].ov[2] = (short)ptf3006->liste_uv.v3;
		
		if(ptf3006->double_side) {
			eerie->facelist[i].facetype |= POLY_DOUBLESIDED;
		}
		
		if(ptf3006->transparency > 0) {
			if(ptf3006->transparency == 2) {
				// NORMAL TRANS 0.00001 to 0.999999
				if(ptf3006->trans < 1.f) {
					eerie->facelist[i].facetype |= POLY_TRANS;
					eerie->facelist[i].transval = ptf3006->trans;
				}
			}
			else if (ptf3006->transparency == 1) {
				if(ptf3006->trans < 0.f) {
					// SUBTRACTIVE -0.000001 to -0.999999
					eerie->facelist[i].facetype |= POLY_TRANS;
					eerie->facelist[i].transval = ptf3006->trans;
				} else {
					// ADDITIVE 1.000001 to 1.9999999
					eerie->facelist[i].facetype |= POLY_TRANS;
					eerie->facelist[i].transval = ptf3006->trans + 1.f;
				}
			} else {
				// MULTIPLICATIVE 2.000001 to 2.9999999
				eerie->facelist[i].facetype |= POLY_TRANS;
				eerie->facelist[i].transval = ptf3006->trans + 2.f;
			}
		}
		
		if(eerie->facelist[i].texid != -1 && !eerie->texturecontainer.empty() && eerie->texturecontainer[eerie->facelist[i].texid] != NULL) {
			
			if(eerie->texturecontainer[eerie->facelist[i].texid]->userflags & POLY_TRANS) {
				if(!(eerie->facelist[i].facetype & POLY_TRANS)) {
					eerie->facelist[i].facetype |= POLY_TRANS;
					eerie->facelist[i].transval = ptf3006->trans;
				}
			}
			
			if(eerie->texturecontainer[eerie->facelist[i].texid]->userflags & POLY_WATER) {
				eerie->facelist[i].facetype |= POLY_WATER;
			}
			
			if(eerie->texturecontainer[eerie->facelist[i].texid]->userflags & POLY_LAVA) {
				eerie->facelist[i].facetype |= POLY_LAVA;
			}
			
			if(eerie->texturecontainer[eerie->facelist[i].texid]->userflags & POLY_FALL) {
				eerie->facelist[i].facetype |= POLY_FALL;
			}

			if(eerie->texturecontainer[eerie->facelist[i].texid]->userflags & POLY_CLIMB) {
				eerie->facelist[i].facetype |= POLY_CLIMB;
			}
		}
		
	}
	
	// Groups Data
	pos = to->groups_seek;
	
	for(long i = 0; i < tn->nb_groups; i++) {
		
		THEO_GROUPS_3011 tg3011;
		const THEO_GROUPS_3011 * ptg3011;
		if(version >= 3011) {
			ptg3011 = reinterpret_cast<const THEO_GROUPS_3011 *>(adr + pos);
			pos += sizeof(THEO_GROUPS_3011);
		} else {
			const THEO_GROUPS * ltg = reinterpret_cast<const THEO_GROUPS *>(adr + pos);
			pos += sizeof(THEO_GROUPS);
			memset(&tg3011, 0, sizeof(THEO_GROUPS_3011));
			tg3011.origin = ltg->origin;
			tg3011.nb_index = ltg->nb_index;
			ptg3011 = &tg3011;
		}
		
		eerie->grouplist[i].origin = ptg3011->origin;
		eerie->grouplist[i].indexes.resize(ptg3011->nb_index);
		
		std::copy((const long*)(adr + pos), (const long*)(adr + pos) + ptg3011->nb_index, eerie->grouplist[i].indexes.begin());
		pos += ptg3011->nb_index * sizeof(long);
		
		eerie->grouplist[i].name = boost::to_lower_copy(util::loadString(adr + pos, 256));
		pos += 256;
		eerie->grouplist[i].siz = 0.f;
		
		for(long o = 0; o < ptg3011->nb_index; o++) {
			eerie->grouplist[i].siz = max(eerie->grouplist[i].siz,
			                              fdist(eerie->vertexlist[eerie->grouplist[i].origin].v,
			                                           eerie->vertexlist[eerie->grouplist[i].indexes[o]].v));
		}
		
		eerie->grouplist[i].siz = ffsqrt(eerie->grouplist[i].siz) * (1.f/16);
		
	}

	// SELECTIONS
	s32 THEO_nb_selected = *reinterpret_cast<const s32 *>(adr + pos);
	pos += sizeof(s32);
	
	eerie->selections.resize(THEO_nb_selected);
	for(long i = 0; i < THEO_nb_selected; i++) {
		
		const THEO_SELECTED * pts = reinterpret_cast<const THEO_SELECTED *>(adr + pos);
		pos += sizeof(THEO_SELECTED);
		
		eerie->selections[i].name = boost::to_lower_copy(util::loadString(pts->name));
		eerie->selections[i].selected.resize(pts->nb_index);
		
		if(pts->nb_index > 0) {
			std::copy((const long*)(adr + pos), (const long*)(adr + pos) + pts->nb_index, eerie->selections[i].selected.begin());
			pos += sizeof(long) * pts->nb_index;
		}
	}
	
	// Theo Action Points Read
	pos = to->action_point_seek;

	for(long i = 0; i < tn->nb_action_point; i++) {
		
		const THEO_ACTION_POINT * ptap = reinterpret_cast<const THEO_ACTION_POINT *>(adr + pos);
		pos += sizeof(THEO_ACTION_POINT);
		
		eerie->actionlist[i].act = ptap->action;
		eerie->actionlist[i].sfx = ptap->num_sfx;
		eerie->actionlist[i].idx = ptap->vert_index;
		eerie->actionlist[i].name = boost::to_lower_copy(util::loadString(ptap->name));
	}
	
	eerie->angle = Anglef::ZERO;
	eerie->pos = Vec3f_ZERO;
	
	// Now Interpret Extra Data chunk
	pos = to->extras_seek + 4;
	
	if(version >= 3005) {
		
		const THEO_EXTRA_DATA_3005 * pted3005 = reinterpret_cast<const THEO_EXTRA_DATA_3005 *>(adr + pos);
		pos += sizeof(THEO_EXTRA_DATA_3005);
		
		eerie->pos = pted3005->pos.toVec3();
		
		eerie->angle.setYaw((float)(pted3005->angle.alpha & 0xfff) * THEO_ROTCONVERT);
		eerie->angle.setPitch((float)(pted3005->angle.beta & 0xfff) * THEO_ROTCONVERT);
		eerie->angle.setRoll((float)(pted3005->angle.gamma & 0xfff) * THEO_ROTCONVERT);
		
		eerie->point0 = eerie->vertexlist[pted3005->origin_index].v;
		eerie->origin = pted3005->origin_index;
		
		eerie->quat = pted3005->quat;
	
	} else {
		
		const THEO_EXTRA_DATA * pted = reinterpret_cast<const THEO_EXTRA_DATA *>(adr + pos);
		pos += sizeof(THEO_EXTRA_DATA);
		
		eerie->pos = pted->pos.toVec3();
		
		eerie->angle.setYaw((float)(pted->angle.alpha & 0xfff) * THEO_ROTCONVERT);
		eerie->angle.setPitch((float)(pted->angle.beta & 0xfff) * THEO_ROTCONVERT);
		eerie->angle.setRoll((float)(pted->angle.gamma & 0xfff) * THEO_ROTCONVERT);
		
		eerie->point0 = eerie->vertexlist[pted->origin_index].v;
		eerie->origin = pted->origin_index;
	}
	
	*poss = pos;
	
	eerie->vertexlist3 = eerie->vertexlist;
	ReCreateUVs(eerie);
	EERIE_Object_Precompute_Fast_Access(eerie);
}

static EERIE_3DSCENE * ScnToEerie(const char * adr, size_t size, const res::path & fic) {
	
	(void)size; // TODO use size
	
	LogDebug("Loading Scene " << fic);
	
	size_t pos = 0;
	
	EERIE_3DSCENE * seerie = allocStructZero<EERIE_3DSCENE>();
	Clear3DScene(seerie);
	
	const TSCN_HEADER * psth = reinterpret_cast<const TSCN_HEADER *>(adr + pos);
	pos += sizeof(TSCN_HEADER);
	
	LogDebug("SCNtoEERIE " << fic << " Version " << psth->version << " Nb Textures " << psth->nb_maps);
	
	if(psth->version < 3008 || psth->version > 3024) {
		LogError << "ScnToEerie: invalid version in " << fic << ": found " << psth->version
		         << " expected 3008 to 3024";
		free(seerie);
		return NULL;
	}
	
	seerie->nbtex = psth->nb_maps;
	
	const res::path temp = "graph/obj3d/textures";
	
	if(psth->type_write == 0) {
		
		seerie->texturecontainer = allocStructZero<TextureContainer *>(psth->nb_maps); 
		
		for(long i = 0; i < psth->nb_maps; i++) {
			
			const THEO_TEXTURE * tt = reinterpret_cast<const THEO_TEXTURE *>(adr + pos);
			pos += sizeof(THEO_TEXTURE);
			
			res::path mapsname = temp / res::path::load(util::loadString(tt->texture_name)).remove_ext();
			seerie->texturecontainer[i] = TextureContainer::Load(mapsname, TextureContainer::Level);
		}
		
	} else {
		
		if((psth->type_write & SAVE_MAP_BMP) || (psth->type_write & SAVE_MAP_TGA)) {
			
			seerie->texturecontainer = allocStructZero<TextureContainer *>(psth->nb_maps);
			
			for(long i = 0; i < psth->nb_maps; i++) {
				
				res::path name;
				if(psth->version >= 3019) {
					const THEO_SAVE_MAPS_IN_3019 * tsmi3019 = reinterpret_cast<const THEO_SAVE_MAPS_IN_3019 *>(adr + pos);
					pos += sizeof(THEO_SAVE_MAPS_IN_3019);
					name = res::path::load(util::loadString(tsmi3019->texture_name)).remove_ext();
				} else {
					const THEO_SAVE_MAPS_IN * tsmi = reinterpret_cast<const THEO_SAVE_MAPS_IN *>(adr + pos);
					pos += sizeof(THEO_SAVE_MAPS_IN);
					name = res::path::load(util::loadString(tsmi->texture_name)).remove_ext();
				}
				
				if(!name.empty()) {
					seerie->texturecontainer[i] = TextureContainer::Load(temp / name, TextureContainer::Level);
				}
			}
		}
	}
	
	// read objects
	pos = psth->object_seek;
	
	s32 nbo = *reinterpret_cast<const s32 *>(adr + pos);
	pos += sizeof(s32);
	
	seerie->nbobj = nbo;
	seerie->objs = allocStructZero<EERIE_3DOBJ *>(nbo);
	
	seerie->point0 = Vec3f(-999999999999.f);
	
	long id = 0;
	
	for(long i = 0; i < nbo; i++) {
		
		const TSCN_OBJHEADER * ptoh = reinterpret_cast<const TSCN_OBJHEADER *>(adr + pos);
		pos += sizeof(TSCN_OBJHEADER);
		
		seerie->objs[id] = new EERIE_3DOBJ(); 
		// TODO most is done in the constructor already
		seerie->objs[id]->clear();
		
		seerie->objs[id]->texturecontainer.resize(seerie->nbtex);
		std::copy(seerie->texturecontainer, seerie->texturecontainer + seerie->nbtex, seerie->objs[id]->texturecontainer.begin());
		
		long objVersion;
		if(psth->version < 3013) {
			objVersion = 3004;
		} else if(psth->version < 3015) {
			objVersion = 3005;
		} else if(psth->version < 3019) {
			objVersion = 3006;
		} else if(psth->version < 3023) {
			objVersion = 3008;
		} else {
			objVersion = 3011;
		}
		loadObjectData(seerie->objs[id], adr, &pos, objVersion);
		
		seerie->cub.xmin = min(seerie->cub.xmin, seerie->objs[id]->cub.xmin + seerie->objs[id]->pos.x);
		seerie->cub.xmax = max(seerie->cub.xmax, seerie->objs[id]->cub.xmax + seerie->objs[id]->pos.x);
		seerie->cub.ymin = min(seerie->cub.ymin, seerie->objs[id]->cub.ymin + seerie->objs[id]->pos.y);
		seerie->cub.ymax = max(seerie->cub.ymax, seerie->objs[id]->cub.ymax + seerie->objs[id]->pos.y);
		seerie->cub.zmin = min(seerie->cub.zmin, seerie->objs[id]->cub.zmin + seerie->objs[id]->pos.z);
		seerie->cub.zmax = max(seerie->cub.zmax, seerie->objs[id]->cub.zmax + seerie->objs[id]->pos.z);
		
		string name = boost::to_lower_copy(util::loadString(ptoh->object_name));
		if(name == "map_origin") {
			seerie->point0 = seerie->objs[id]->point0 + seerie->objs[id]->pos;
			delete seerie->objs[id];
			seerie->nbobj--;
			id--;
		} else {
			seerie->objs[id]->name = name;
		}
		
		id++;
		
		pos = ptoh->next_obj;
	}
	
	pos = psth->light_seek; // ambient
	
	pos += sizeof(SavedColor); // ignore ambient color
	
	s32 nbl = *reinterpret_cast<const s32 *>(adr + pos);
	pos += sizeof(s32);
	
	seerie->light = NULL; 
	seerie->nblight = nbl;
	
	for(long i = 0; i < nbl; i++) {
		
		TSCN_LIGHT_3024 sl3024;
		const TSCN_LIGHT_3024 * tsl3024;
		
		if(psth->version >= 3024) {
			tsl3024 = reinterpret_cast<const TSCN_LIGHT_3024 *>(adr + pos);
			pos += sizeof(TSCN_LIGHT_3024);
		} else if(psth->version >= 3019) {
			const TSCN_LIGHT_3019 * tsl3019 = reinterpret_cast<const TSCN_LIGHT_3019 *>(adr + pos);
			pos += sizeof(TSCN_LIGHT_3019);
			memset(&sl3024, 0, sizeof(TSCN_LIGHT_3024));
			sl3024.red = tsl3019->red;
			sl3024.green = tsl3019->green;
			sl3024.blue = tsl3019->blue;
			sl3024.pos = tsl3019->pos;
			sl3024.hotspot = tsl3019->hotspot;
			sl3024.falloff = tsl3019->falloff;
			sl3024.intensity = tsl3019->intensity;
			tsl3024 = &sl3024;
		} else {
			const TSCN_LIGHT * tsl = reinterpret_cast<const TSCN_LIGHT *>(adr + pos);
			pos += sizeof(TSCN_LIGHT);
			memset(&sl3024, 0, sizeof(TSCN_LIGHT_3024));
			sl3024.red = tsl->red;
			sl3024.green = tsl->green;
			sl3024.blue = tsl->blue;
			sl3024.pos = tsl->pos;
			sl3024.hotspot = tsl->hotspot;
			sl3024.falloff = tsl->falloff;
			sl3024.intensity = tsl->intensity;
			tsl3024 = &sl3024;
		}
		
		EERIE_LIGHT light;
		
		light.rgb.r = (float)tsl3024->red * ( 1.0f / 255 );
		light.rgb.g = (float)tsl3024->green * ( 1.0f / 255 );
		light.rgb.b = (float)tsl3024->blue * ( 1.0f / 255 );
		light.pos = tsl3024->pos.toVec3();
		light.fallstart = (float)tsl3024->hotspot;
		light.fallend = (float)tsl3024->falloff;
		
		float t = light.fallend - light.fallstart;
		if(t < 150.f) {
			light.fallend += 150.f - t;
		}
		
		light.intensity = (float)tsl3024->intensity;
		light.exist = 1;
		light.treat = 1;
		light.selected = 0;
		light.type = 0;
		EERIE_LIGHT_GlobalAdd(&light);
	}
	
	return seerie;
}

static void ReleaseScene(EERIE_3DSCENE * scene) {
	
	free(scene->texturecontainer), scene->texturecontainer = NULL;
	
	for(long i = 0; i < scene->nbobj; i++) {
		delete scene->objs[i];
	}
	
	free(scene->objs), scene->objs = NULL;
	free(scene->texturecontainer), scene->texturecontainer = NULL;
	
	if(scene->light) {
		for(long i = 0; i < scene->nblight; i++) {
			free(scene->light[i]), scene->light[i] = NULL;
		}
		free(scene->light), scene->light = NULL;
	}
	
	free(scene);
}

void ReleaseMultiScene(EERIE_MULTI3DSCENE * ms) {
	
	if(ms) {
		for(long i = 0; i < ms->nb_scenes; i++) {
			ReleaseScene(ms->scenes[i]);
			ms->scenes[i] = NULL;
		}
	}
	
	free(ms);
}

static EERIE_MULTI3DSCENE * PAK_MultiSceneToEerie_Impl(const res::path & dirr) {
	
	EERIE_MULTI3DSCENE * es = allocStructZero<EERIE_MULTI3DSCENE>();
	
	LastLoadedScene = dirr;
	
	PakDirectory * dir = resources->getDirectory(dirr);
	if(dir) {
		bool loaded = false;
		for(PakDirectory::files_iterator i = dir->files_begin(); i != dir->files_end(); i++) {
			if(!res::path(i->first).has_ext("scn")) {
				continue;
			}
			
			char * adr = i->second->readAlloc();
			if(adr) {
				es->scenes[es->nb_scenes] = ScnToEerie(adr, i->second->size(), dirr);
				es->nb_scenes++;
				free(adr);
			} else {
				LogError << "Could not read scene " << dirr << '/' << i->first;
			}
			
			loaded = true;
		}
		if(!loaded) {
			LogWarning << "Empty multiscene: " << dirr;
		}
	} else {
		LogWarning << "Multiscene not found: " << dirr;
	}
	
	es->cub.xmax = -9999999999.f;
	es->cub.xmin = 9999999999.f;
	es->cub.ymax = -9999999999.f;
	es->cub.ymin = 9999999999.f;
	es->cub.zmax = -9999999999.f;
	es->cub.zmin = 9999999999.f;
	
	for(long i = 0; i < es->nb_scenes; i++) {
		es->cub.xmax = max(es->cub.xmax, es->scenes[i]->cub.xmax);
		es->cub.xmin = min(es->cub.xmin, es->scenes[i]->cub.xmin);
		es->cub.ymax = max(es->cub.ymax, es->scenes[i]->cub.ymax);
		es->cub.ymin = min(es->cub.ymin, es->scenes[i]->cub.ymin);
		es->cub.zmax = max(es->cub.zmax, es->scenes[i]->cub.zmax);
		es->cub.zmin = min(es->cub.zmin, es->scenes[i]->cub.zmin);
		es->pos = es->scenes[i]->pos;
		
		if((es->scenes[i]->point0.x != -999999999999.f) &&
		   (es->scenes[i]->point0.y != -999999999999.f) &&
		   (es->scenes[i]->point0.z != -999999999999.f)) {
			es->point0 = es->scenes[i]->point0;
		}
	}
	
	if(es->nb_scenes == 0) {
		free(es);
		return NULL;
	}
	
	return es;
}

EERIE_MULTI3DSCENE * PAK_MultiSceneToEerie(const res::path & dirr) {
	
	LogDebug("Loading Multiscene " << dirr);
	
	EERIE_MULTI3DSCENE * em = NULL;
	
	em = PAK_MultiSceneToEerie_Impl(dirr);

	EERIEPOLY_Compute_PolyIn();
	return em;
}

#endif // BUILD_EDIT_LOADSAVE

//-----------------------------------------------------------------------------------------------------
// Warning Clear3DObj/Clear3DScene don't release Any pointer Just Clears Structures
void EERIE_3DOBJ::clear() {
	
		point0 = pos = Vec3f_ZERO;
		angle = Anglef::ZERO;

		origin = 0;
		ident = 0;
		nbgroups = 0;

		vertexlocal = NULL;
		vertexlist.clear();
		vertexlist3.clear();

		facelist.clear();
		grouplist = NULL;
		texturecontainer.clear();

		originaltextures = NULL;
		linked = NULL;

		// TODO Default constructor
		quat.x = quat.y = quat.z = quat.w = 0;
		nblinked = 0;

		pbox = 0;
		pdata = 0;
		ndata = 0;
		cdata = 0;
		sdata = 0;

		fastaccess.view_attach = 0;
		fastaccess.primary_attach = 0;
		fastaccess.left_attach = 0;
		fastaccess.weapon_attach = 0;
		fastaccess.secondary_attach = 0;
		fastaccess.mouth_group = 0;
		fastaccess.jaw_group = 0;
		fastaccess.head_group_origin = 0;
		fastaccess.head_group = 0;
		fastaccess.mouth_group_origin = 0;
		fastaccess.V_right = 0;
		fastaccess.U_right = 0;
		fastaccess.fire = 0;
		fastaccess.sel_head = 0;
		fastaccess.sel_chest = 0;
		fastaccess.sel_leggings = 0;
		fastaccess.carry_attach = 0;
		fastaccess.padding_ = 0;

		c_data = 0;
		
	cub.xmin = cub.ymin = cub.zmin = std::numeric_limits<float>::max();
	cub.xmax = cub.ymax = cub.zmax = std::numeric_limits<float>::min();
}

void Clear3DScene(EERIE_3DSCENE * eerie) {
	
	if(!eerie)
		return;
	
	memset(eerie, 0, sizeof(EERIE_3DSCENE));
	eerie->cub.xmin = eerie->cub.ymin = eerie->cub.zmin = std::numeric_limits<float>::max();
	eerie->cub.xmax = eerie->cub.ymax = eerie->cub.zmax = std::numeric_limits<float>::min();
}

// TODO move to destructor?
EERIE_3DOBJ::~EERIE_3DOBJ() {
	
	free(originaltextures), originaltextures = NULL;
	
	if(ndata) {
		KillNeighbours(this);
	}
	
	if(pdata) {
		KillProgressiveData(this);
	}
	
	if(cdata) {
		KillClothesData(this);
	}
	
	EERIE_RemoveCedricData(this);
	EERIE_PHYSICS_BOX_Release(this);
	EERIE_COLLISION_SPHERES_Release(this);
	
	delete[] grouplist;
	free(linked);
}

EERIE_3DOBJ * Eerie_Copy(const EERIE_3DOBJ * obj) {
	
	EERIE_3DOBJ * nouvo = new EERIE_3DOBJ(); 
	
	nouvo->vertexlist = obj->vertexlist;
	
	nouvo->vertexlist3 = obj->vertexlist3;
	
	nouvo->linked = NULL;
	nouvo->ndata = NULL;
	nouvo->pbox = NULL;
	nouvo->pdata = NULL;
	nouvo->cdata = NULL;
	nouvo->sdata = NULL;
	nouvo->c_data = NULL;
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
	
	nouvo->ident = obj->ident;

	if(!obj->name.empty())
		nouvo->name = obj->name;

	nouvo->origin = obj->origin;
	nouvo->point0 = obj->point0;
	nouvo->quat = obj->quat;


	if(obj->ndata)
		nouvo->ndata = copyStruct(obj->ndata, obj->vertexlist.size());
	else
		nouvo->ndata = NULL;

	nouvo->facelist = obj->facelist;

	if(obj->nbgroups) {
		nouvo->nbgroups = obj->nbgroups;
		nouvo->grouplist = new EERIE_GROUPLIST[obj->nbgroups];
		std::copy(obj->grouplist, obj->grouplist + obj->nbgroups, nouvo->grouplist);
	}

	nouvo->actionlist = obj->actionlist;

	nouvo->selections = obj->selections;

	nouvo->texturecontainer = obj->texturecontainer;
	
	memcpy(&nouvo->fastaccess, &obj->fastaccess, sizeof(EERIE_FASTACCESS));
	EERIE_CreateCedricData(nouvo);

	if(obj->pbox) {
		nouvo->pbox = allocStructZero<PHYSICS_BOX_DATA>();
		nouvo->pbox->nb_physvert = obj->pbox->nb_physvert;
		nouvo->pbox->stopcount = 0;
		nouvo->pbox->radius = obj->pbox->radius;
		
		nouvo->pbox->vert = copyStruct(obj->pbox->vert, obj->pbox->nb_physvert);
	}

	nouvo->linked = NULL;
	nouvo->nblinked = 0;
	nouvo->originaltextures = NULL;
	return nouvo;
}

long EERIE_OBJECT_GetSelection(const EERIE_3DOBJ * obj, const string & selname) {
	
	if(!obj)
		return -1;
	
	for(size_t i = 0; i < obj->selections.size(); i++) {
		if(obj->selections[i].name == selname) {
			return i;
		}
	}
	
	return -1;
}

long EERIE_OBJECT_GetGroup(const EERIE_3DOBJ * obj, const string & groupname) {
	
	if(!obj)
		return -1;
	
	for(long i = 0; i < obj->nbgroups; i++) {
		if(obj->grouplist[i].name == groupname) {
			return i;
		}
	}
	
	return -1;
}

void AddIdxToBone(EERIE_BONE * bone, long idx)
{
	bone->idxvertices = (long *)realloc(bone->idxvertices, sizeof(long) * (bone->nb_idxvertices + 1));

	if(bone->idxvertices) {
		bone->idxvertices[bone->nb_idxvertices] = idx;
		bone->nb_idxvertices++;
	}
}

long GetFather(EERIE_3DOBJ * eobj, long origin, long startgroup)
{
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
	
	if(!eobj || !eobj->c_data)
		return;
	
	for(long i = 0; i < eobj->c_data->nb_bones; i++) {
		free(eobj->c_data->bones[i].idxvertices);
		eobj->c_data->bones[i].idxvertices = NULL;
	}
	
	delete[] eobj->c_data->bones, eobj->c_data->bones = NULL;
	delete eobj->c_data, eobj->c_data = NULL;
	delete[] eobj->vertexlocal, eobj->vertexlocal = NULL;
}

void EERIE_CreateCedricData(EERIE_3DOBJ * eobj) {
	
	eobj->c_data = new EERIE_C_DATA();
	memset(eobj->c_data, 0, sizeof(EERIE_C_DATA));

	if(eobj->nbgroups <= 0) {
		// If no groups were specified

		// Make one bone
		eobj->c_data->nb_bones = 1;
		eobj->c_data->bones = new EERIE_BONE[eobj->c_data->nb_bones];
		memset(eobj->c_data->bones, 0, sizeof(EERIE_BONE)*eobj->c_data->nb_bones);

		EERIE_BONE & bone = eobj->c_data->bones[0];

		// Add all vertices to the bone
		for(size_t i = 0; i < eobj->vertexlist.size(); i++)
			AddIdxToBone(&bone, i);

		// Initialize the bone
		Quat_Init(&bone.init.quat);
		Quat_Init(&bone.anim.quat);
		bone.init.scale = Vec3f_ZERO;
		bone.anim.scale = Vec3f_ZERO;
		bone.init.trans = Vec3f_ZERO;
		bone.transinit_global = bone.init.trans;
		bone.original_group = NULL;
		bone.father = -1;
	} else {
		// Groups were specified

		// Alloc the bones
		eobj->c_data->nb_bones = eobj->nbgroups;
		eobj->c_data->bones = new EERIE_BONE[eobj->c_data->nb_bones];
		// TODO memset -> use constructor instead
		memset(eobj->c_data->bones, 0, sizeof(EERIE_BONE)*eobj->c_data->nb_bones);

		bool * temp = new bool[eobj->vertexlist.size()];
		memset(temp, 0, eobj->vertexlist.size());

		for(long i = eobj->nbgroups - 1; i >= 0; i--) {
			EERIE_GROUPLIST & group = eobj->grouplist[i];
			EERIE_BONE & bone = eobj->c_data->bones[i];

			EERIE_VERTEX * v_origin = &eobj->vertexlist[group.origin];

			for(size_t j = 0; j < group.indexes.size(); j++) {
				if(!temp[group.indexes[j]]) {
					temp[group.indexes[j]] = true;
					AddIdxToBone(&bone, group.indexes[j]);
				}
			}

			Quat_Init(&bone.init.quat);
			Quat_Init(&bone.anim.quat);
			bone.init.scale = Vec3f_ZERO;
			bone.anim.scale = Vec3f_ZERO;
			bone.init.trans = Vec3f(v_origin->v.x, v_origin->v.y, v_origin->v.z);
			bone.transinit_global = bone.init.trans;
			bone.original_group = &group;
			bone.father = GetFather(eobj, group.origin, i - 1);
		}

		delete[] temp;

		// Try to correct lonely vertex
		for(size_t i = 0; i < eobj->vertexlist.size(); i++) {
			long ok = 0;

			for(long j = 0; j < eobj->nbgroups; j++) {
				for(size_t k = 0; k < eobj->grouplist[j].indexes.size(); k++) {
					if((size_t)eobj->grouplist[j].indexes[k] == i) {
						ok = 1;
						break;
					}
				}

				if(ok)
					break;
			}

			if(!ok) {
				AddIdxToBone(&eobj->c_data->bones[0], i);
			}
		}
		
		for(long i = eobj->nbgroups - 1; i >= 0; i--) {
			EERIE_BONE & bone = eobj->c_data->bones[i];

			if(bone.father >= 0) {
				long father = bone.father;
				bone.init.trans -= eobj->c_data->bones[father].init.trans;
			}
			bone.transinit_global = bone.init.trans;
		}

	}

	/* Build proper mesh */
	{
		EERIE_C_DATA* obj = eobj->c_data;

		for(long i = 0; i != obj->nb_bones; i++) {
			EERIE_BONE & bone = obj->bones[i];

			if(bone.father >= 0) {
				/* Rotation*/
				bone.anim.quat = Quat_Multiply(obj->bones[bone.father].anim.quat, bone.init.quat);
				/* Translation */
				bone.anim.trans = TransformVertexQuat(obj->bones[bone.father].anim.quat, bone.init.trans);
				bone.anim.trans = obj->bones[bone.father].anim.trans + bone.anim.trans;
			} else {
				/* Rotation*/
				bone.anim.quat = bone.init.quat;
				/* Translation */
				bone.anim.trans = bone.init.trans;
			}
			bone.anim.scale = Vec3f_ONE;
		}

		eobj->vertexlocal = new EERIE_3DPAD[eobj->vertexlist.size()];
		// TODO constructor is better than memset
		memset(eobj->vertexlocal, 0, sizeof(EERIE_3DPAD)*eobj->vertexlist.size());

		for(long i = 0; i != obj->nb_bones; i++) {
			Vec3f vector = obj->bones[i].anim.trans;
			
			for(int v = 0; v != obj->bones[i].nb_idxvertices; v++) {
				
				long idx = obj->bones[i].idxvertices[v];
				const EERIE_VERTEX & inVert = eobj->vertexlist[idx];
				EERIE_3DPAD & outVert = eobj->vertexlocal[idx];
				
				Vec3f temp = inVert.v - vector;
				TransformInverseVertexQuat(&obj->bones[i].anim.quat, &temp, &temp);
				outVert.x = temp.x;
				outVert.y = temp.y;
				outVert.z = temp.z;
			}
		}
	}
}

#if BUILD_EDIT_LOADSAVE

// Converts a Theo Object to an EERIE object
static EERIE_3DOBJ * TheoToEerie(const char * adr, long size, const res::path & texpath, const res::path & fic) {
	
	LogWarning << "TheoToEerie " << fic;
	
	if(!adr)
		return NULL;
	
	res::path txpath = texpath.empty() ? "graph/obj3d/textures" : texpath;
	
	if(size < 10) {
		return NULL;
	}
	
	size_t pos = 0;
	
	const THEO_HEADER * pth = reinterpret_cast<const THEO_HEADER *>(adr + pos);
	pos += sizeof(THEO_HEADER);
	
	if(pth->version < 3003 || pth->version > 3011) {
		LogError << "TheoToEerie: invalid version in " << fic << ": found " << pth->version
		         << " expected 3004 to 3011";
		return NULL;
	}
	
	EERIE_3DOBJ * eerie = new EERIE_3DOBJ;
	eerie->clear();
	
	eerie->file = fic;
	
	if(pth->type_write == 0) {
		// read the texture
		
		LogError <<  "WARNING object " << fic << " SAVE MAP IN OBJECT = INVALID... Using Dummy Textures...";
		
		eerie->texturecontainer.resize(pth->nb_maps);
		for(long i = 0; i < pth->nb_maps; i++) {
			pos += sizeof(THEO_TEXTURE);
			eerie->texturecontainer[i] = GetAnyTexture();
		}
		
	} else {
		
		if((pth->type_write & SAVE_MAP_BMP) || (pth->type_write & SAVE_MAP_TGA)) {
			
			eerie->texturecontainer.resize(pth->nb_maps);
			for(long i = 0; i < pth->nb_maps; i++) {
				
				res::path name;
				if(pth->version >= 3008) {
					const THEO_SAVE_MAPS_IN_3019 * tsmi3019 = reinterpret_cast<const THEO_SAVE_MAPS_IN_3019 *>(adr + pos);
					pos += sizeof(THEO_SAVE_MAPS_IN_3019);
					name = res::path::load(util::loadString(tsmi3019->texture_name)).remove_ext();
				} else {
					const THEO_SAVE_MAPS_IN * tsmi = reinterpret_cast<const THEO_SAVE_MAPS_IN *>(adr + pos);
					pos += sizeof(THEO_SAVE_MAPS_IN);
					name = res::path::load(util::loadString(tsmi->texture_name)).remove_ext();
				}
				
				if(!name.empty()) {
					eerie->texturecontainer[i] = TextureContainer::Load(txpath / name, TextureContainer::Level);
				}
			}
		}
	}
	
	pos = pth->object_seek;
	loadObjectData(eerie, adr, &pos, pth->version);
	eerie->angle = Anglef::ZERO;
	eerie->pos = Vec3f_ZERO;

	// NORMALS CALCULATIONS

	//Compute Faces Areas
	for(size_t i = 0; i < eerie->facelist.size(); i++) {
		const Vec3f & p0 = eerie->vertexlist[eerie->facelist[i].vid[0]].v;
		const Vec3f & p1 = eerie->vertexlist[eerie->facelist[i].vid[1]].v;
		const Vec3f & p2 = eerie->vertexlist[eerie->facelist[i].vid[2]].v;
		eerie->facelist[i].temp = glm::distance((p0 + p1) * .5f, p2) * glm::distance(p0, p1) * .5f;
	}

	for(size_t i = 0; i < eerie->facelist.size(); i++) {
		CalcObjFaceNormal(
		    &eerie->vertexlist[eerie->facelist[i].vid[0]].v,
		    &eerie->vertexlist[eerie->facelist[i].vid[1]].v,
		    &eerie->vertexlist[eerie->facelist[i].vid[2]].v,
		    &eerie->facelist[i]
		);
		float area = eerie->facelist[i].temp;

		for(long j = 0; j < 3; j++) {
			float mod = area * area;
			Vec3f nrml = eerie->facelist[i].norm * mod;
			float count = mod;

			for(size_t i2 = 0; i2 < eerie->facelist.size(); i2++) {
				if(i != i2) {
					float area2 = eerie->facelist[i].temp;

					for(long j2 = 0; j2 < 3; j2++) {
						if(closerThan(eerie->vertexlist[eerie->facelist[i2].vid[j2]].v, eerie->vertexlist[eerie->facelist[i].vid[j]].v, .1f)) {
							mod = (area2 * area2);
							nrml += eerie->facelist[i2].norm * mod;
							count += mod; 
						}
					}
				}
			}

			count = 1.f / count;
			eerie->vertexlist[eerie->facelist[i].vid[j]].vert.p = nrml * count;
		}
	}

	for(size_t i = 0; i < eerie->facelist.size(); i++) {
		for(long j = 0; j < 3; j++) {
			eerie->vertexlist[eerie->facelist[i].vid[j]].norm = eerie->vertexlist[eerie->facelist[i].vid[j]].vert.p;
		}
	}

	// Apply Normals Spherical correction for NPC head
	long neck_orgn = GetGroupOriginByName(eerie, "neck");
	long head_idx = EERIE_OBJECT_GetGroup(eerie, "head");

	if(head_idx >= 0 && neck_orgn >= 0) {
		Vec3f center = Vec3f_ZERO;
		Vec3f origin = eerie->vertexlist[neck_orgn].v;
		float count = (float)eerie->grouplist[head_idx].indexes.size();

		if(count > 0.f) {
			for(size_t idx = 0 ; idx < eerie->grouplist[head_idx].indexes.size(); idx++) {
				center += eerie->vertexlist[ eerie->grouplist[head_idx].indexes[idx] ].v;
			}
			
			center = (center * (1.f / count) + origin + origin) * (1.0f / 3);
			float max_threshold = glm::distance(origin, center);
			
			for(size_t i = 0; i < eerie->grouplist[head_idx].indexes.size(); i++) {
				EERIE_VERTEX * ev = &eerie->vertexlist[eerie->grouplist[head_idx].indexes[i]];
				float d = glm::distance(ev->v, origin);
				float factor = 1.f;

				if(d < max_threshold) {
					factor = d / max_threshold;
				}

				float ifactor = 1.f - factor;
				Vec3f fakenorm;
				fakenorm = ev->v - center;
				fakenorm = glm::normalize(fakenorm);
				ev->norm = ev->norm * ifactor + fakenorm * factor;
				ev->norm = glm::normalize(ev->norm);
			}
		}
	}

	// NORMALS CALCULATIONS END
	//***********************************************************

	EERIE_LINKEDOBJ_InitData(eerie);
	eerie->c_data = NULL;
	EERIE_CreateCedricData(eerie);
	return eerie;
}

static EERIE_3DOBJ * GetExistingEerie(const res::path & file) {
	
	for(size_t i = 1; i < entities.size(); i++) {
		if(entities[i] != NULL && !entities[i]->tweaky && entities[i]->obj) {
			EERIE_3DOBJ * obj = entities[i]->obj;
			if(!obj->originaltextures && entities[i]->obj->file == file) {
				return entities[i]->obj;
			}
		}
	}
	
	return NULL;
}

#endif

static EERIE_3DOBJ * TheoToEerie_Fast(const res::path & texpath, const res::path & file, bool pbox) {
	
	EERIE_3DOBJ * ret = ARX_FTL_Load(file);
	if(ret) {
		if(pbox) {
			EERIE_PHYSICS_BOX_Create(ret);
		}
		return ret;
	}
	
#if !BUILD_EDIT_LOADSAVE
	ARX_UNUSED(texpath);
#else
	
	ret = GetExistingEerie(file);
	if(ret) {
		ret = Eerie_Copy(ret);
	}
	
	if(!ret) {
		
		size_t size = 0;
		char * adr = resources->readAlloc(file, size);
		if(!adr) {
			LogWarning << "Object not found: " << file;
			return NULL;
		}
		
		ret = TheoToEerie(adr, size, texpath, file);
		if(!ret) {
			free(adr);
			return NULL;
		}
		
		EERIE_OBJECT_CenterObjectCoordinates(ret);
		free(adr);
	}
	
	CreateNeighbours(ret);
	EERIEOBJECT_AddClothesData(ret);
	KillNeighbours(ret);
	
	if(ret->cdata) {
		EERIE_COLLISION_SPHERES_Create(ret); // Must be out of the Neighbours zone
	}
	
	if(pbox) {
		EERIE_PHYSICS_BOX_Create(ret);
	}
	
	ARX_FTL_Save(fs::paths.user / file.string(), ret);
	
#endif // BUILD_EDIT_LOADSAVE
	
	return ret;
}

EERIE_3DOBJ * loadObject(const res::path & file, bool pbox) {
	return TheoToEerie_Fast("graph/obj3d/textures", file, pbox);
}

EERIE_3DOBJ * LoadTheObj(const res::path & file, const res::path & texpath) {
	return TheoToEerie_Fast(file.parent() / texpath, file, true);
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
		ret->vertexlist[i].vert.p -= offset;
		ret->vertexlist3[i].v -= offset;
		ret->vertexlist3[i].vert.p -= offset;
		ret->vertexlist3[i].v -= offset;
		ret->vertexlist3[i].vert.p -= offset;
	}
	
	ret->point0 -= offset;
}
