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
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved

#include "animation/Animation.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <vector>

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

#include "animation/AnimationRender.h"

#include "core/Application.h"
#include "core/GameTime.h"
#include "core/Core.h"

#include "game/Damage.h"
#include "game/EntityManager.h"
#include "game/NPC.h"
#include "game/Player.h"
#include "game/Spells.h"

#include "graphics/BaseGraphicsTypes.h"
#include "graphics/Color.h"
#include "graphics/Draw.h"
#include "graphics/GraphicsTypes.h"
#include "graphics/Math.h"
#include "graphics/Renderer.h"
#include "graphics/data/Mesh.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/texture/TextureStage.h"

#include "io/resource/ResourcePath.h"
#include "io/resource/PakReader.h"
#include "io/log/Logger.h"

#include "math/Angle.h"
#include "math/Vector3.h"

#include "platform/Platform.h"

#include "scene/Object.h"
#include "scene/GameSound.h"
#include "scene/Scene.h"
#include "scene/Interactive.h"

#include "script/Script.h"

#include "physics/Collisions.h"

using std::min;
using std::max;
using std::string;
using std::ostringstream;
using std::vector;

long MAX_LLIGHTS = 18;
//-----------------------------------------------------------------------------
extern EERIE_CAMERA TCAM[32];
extern QUAKE_FX_STRUCT QuakeFx;
extern Color ulBKGColor;
extern long ZMAPMODE;

ANIM_HANDLE animations[MAX_ANIMATIONS];

TexturedVertex LATERDRAWHALO[HALOMAX * 4];
EERIE_LIGHT * llights[32];
float dists[32];
float values[32];
long TRAP_DETECT = -1;
long TRAP_SECRET = -1;
long HALOCUR = 0;

static const long anim_power[] = { 100, 20, 15, 12, 8, 6, 5, 4, 3, 2, 2, 1, 1, 1, 1 };

static TexturedVertex tTexturedVertexTab2[4000];

// ANIMATION HANDLES handling

short ANIM_GetAltIdx(ANIM_HANDLE * ah, long old) {

	if(ah->alt_nb == 1)
		return 0;

	long tot = anim_power[0];

	for(long i = 1; i < ah->alt_nb; i++) {
		tot += anim_power[min(i, 14L)];
	}

	while(1) {
		for(short i = 0; i < ah->alt_nb; i++) {
			float r = rnd() * tot;

			if(r < anim_power[min((int)i,14)] && i != old)
				return i;
		}
	}
}

void ANIM_Set(ANIM_USE *au, ANIM_HANDLE *anim)
{
	if(!au || !anim)
		return;

	au->cur_anim = anim;
	au->altidx_cur = ANIM_GetAltIdx(anim, au->altidx_cur);

	if(au->altidx_cur > au->cur_anim->alt_nb)
		au->altidx_cur = 0;

	au->ctime = 0;
	au->lastframe = -1;
	au->flags &= ~EA_PAUSED;
	au->flags &= ~EA_ANIMEND;
	au->flags &= ~EA_LOOP;
	au->flags &= ~EA_FORCEPLAY;
}

ANIM_HANDLE::ANIM_HANDLE() : path() {
	
	anims = NULL;
	alt_nb = 0;
	
	locks = 0;
}

void EERIE_ANIMMANAGER_PurgeUnused() {
	
	for(size_t i = 0; i < MAX_ANIMATIONS; i++) {
		if(!animations[i].path.empty() && animations[i].locks == 0) {
			for(long k = 0; k < animations[i].alt_nb; k++) {
				ReleaseAnim(animations[i].anims[k]);
				animations[i].anims[k] = NULL;
			}
			free(animations[i].anims), animations[i].anims = NULL;
			animations[i].path.clear();
		}
	}
}

void EERIE_ANIMMANAGER_ReleaseHandle(ANIM_HANDLE * anim) {

	if(!anim)
		return;

	anim->locks--;
	if(anim->locks < 0) {
		anim->locks = 0;
	}
}

static ANIM_HANDLE * EERIE_ANIMMANAGER_GetHandle(const res::path & path) {
	
	for(size_t i = 0; i < MAX_ANIMATIONS; i++) {
		if(animations[i].path == path) {
			return &animations[i];
		}
	}
	
	return NULL;
}

static bool EERIE_ANIMMANAGER_AddAltAnim(ANIM_HANDLE * ah, const res::path & path) {
	
	if(!ah || ah->path.empty()) {
		return false;
	}
	
	size_t FileSize;
	char * adr = resources->readAlloc(path, FileSize);
	if(!adr) {
		return false;
	}
	
	EERIE_ANIM * temp = TheaToEerie(adr, FileSize, path);
	free(adr);
	if(!temp) {
		return false;
	}
	
	ah->alt_nb++;
	ah->anims = (EERIE_ANIM **)realloc(ah->anims, sizeof(EERIE_ANIM *) * ah->alt_nb);
	ah->anims[ah->alt_nb - 1] = temp;
	
	return true;
}

ANIM_HANDLE * EERIE_ANIMMANAGER_Load(const res::path & path) {
	
	ANIM_HANDLE * anim = EERIE_ANIMMANAGER_Load_NoWarning(path);
	if(!anim) {
		LogWarning << "Animation not found: " << path;
	}
	
	return anim;
}

ANIM_HANDLE * EERIE_ANIMMANAGER_Load_NoWarning(const res::path & path) {
	
	ANIM_HANDLE * handl = EERIE_ANIMMANAGER_GetHandle(path);
	if(handl) {
		handl->locks++;
		return handl;
	}
	
	for(size_t i = 0; i < MAX_ANIMATIONS; i++) {
		
		if(!animations[i].path.empty()) {
			continue;
		}
		
		size_t FileSize;
		char * adr = resources->readAlloc(path, FileSize);
		if(!adr) {
			return NULL;
		}
		
		animations[i].anims = (EERIE_ANIM **)malloc(sizeof(EERIE_ANIM *));
		animations[i].anims[0] = TheaToEerie(adr, FileSize, path);
		animations[i].alt_nb = 1;
		
		free(adr);
		
		if(!animations[i].anims[0]) {
			return NULL;
		}
		
		animations[i].path = path;
		animations[i].locks = 1;
		
		int pathcount = 2;
		res::path altpath;
		do {
			altpath = res::path(path);
			altpath.append_basename(boost::lexical_cast<std::string>(pathcount++));
		} while(EERIE_ANIMMANAGER_AddAltAnim(&animations[i], altpath));
		
		return &animations[i];
	}
	
	return NULL;
}

/*!
 * \note tex Must be of sufficient size...
 */
long EERIE_ANIMMANAGER_Count( std::string& tex, long * memsize)
{
	char temp[512];
	long count=0;
	*memsize=0;

	for(size_t i = 0; i < MAX_ANIMATIONS; i++) {
		
		if(!animations[i].path.empty()) {
			count++;
			char txx[256];
			strcpy(txx,animations[i].path.string().c_str());
			long totsize=0;

			sprintf(temp, "%3ld[%3lu] %s size %ld Locks %ld Alt %d\r\n", count, (unsigned long)i,
			        txx, totsize, animations[i].locks, animations[i].alt_nb - 1);
			memsize+=totsize;
			tex += temp;
		}
	}

	return count;
}

//
/*!
 * \brief Fill "pos" with "eanim" total translation
 */
void GetAnimTotalTranslate( ANIM_HANDLE * eanim, long alt_idx, Vec3f * pos) {
	
	if(!pos)
		return;
	
	if(!eanim || !eanim->anims[alt_idx] || !eanim->anims[alt_idx]->frames
	   || eanim->anims[alt_idx]->nb_key_frames <= 0) {
		*pos = Vec3f::ZERO;
	} else {
		long idx = eanim->anims[alt_idx]->nb_key_frames - 1;
		*pos = eanim->anims[alt_idx]->frames[idx].translate;
	}
}

/*!
 * \brief Main Procedure to draw an animated object
 *
 * \param eobj main object data
 * \param eanim Animation data
 * \param time Time increment to current animation in Ms
 * \param io Referrence to Interactive Object (NULL if no IO)
 */
void PrepareAnim(EERIE_3DOBJ *eobj, ANIM_USE *eanim, unsigned long time, Entity *io) {
	
	if(!eobj || !eanim)
		return;

	if(eanim->flags & EA_PAUSED)
		time = 0;

	if(io && (io->ioflags & IO_FREEZESCRIPT))
		time = 0;

	if(eanim->altidx_cur >= eanim->cur_anim->alt_nb)
		eanim->altidx_cur = 0;

	if(!(eanim->flags & EA_EXCONTROL))
		eanim->ctime += time;

	eanim->flags &= ~EA_ANIMEND;

	if((eanim->flags & EA_STOPEND) && eanim->ctime > eanim->cur_anim->anims[eanim->altidx_cur]->anim_time)
	{
		eanim->ctime = eanim->cur_anim->anims[eanim->altidx_cur]->anim_time;
	}

	if((eanim->flags & EA_LOOP)
	   || (io && ((eanim->cur_anim == io->anims[ANIM_WALK])
	              || (eanim->cur_anim == io->anims[ANIM_WALK2])
	              || (eanim->cur_anim == io->anims[ANIM_WALK3])
				  || (eanim->cur_anim == io->anims[ANIM_RUN])
				  || (eanim->cur_anim == io->anims[ANIM_RUN2])
				  || (eanim->cur_anim == io->anims[ANIM_RUN3])))) {
		
		if(eanim->ctime > eanim->cur_anim->anims[eanim->altidx_cur]->anim_time) {
			
			long lost = eanim->ctime - long(eanim->cur_anim->anims[eanim->altidx_cur]->anim_time);

			if(!eanim->next_anim) {
				long t = eanim->cur_anim->anims[eanim->altidx_cur]->anim_time;
				eanim->ctime= eanim->ctime % t;

				if(io)
					FinishAnim(io,eanim->cur_anim);
			} else {
				if(io) {
					FinishAnim(io,eanim->cur_anim);

					if(io->lastanimtime != 0)
						AcquireLastAnim(io);
					else
						io->lastanimtime = 1;
				}

				eanim->cur_anim=eanim->next_anim;
				eanim->altidx_cur=ANIM_GetAltIdx(eanim->next_anim,eanim->altidx_cur);
				eanim->next_anim=NULL;
				ResetAnim(eanim);
				eanim->ctime = lost;
				eanim->flags=eanim->nextflags;
				eanim->flags&=~EA_ANIMEND;
				goto suite;
			}
		}
	} else if (eanim->ctime > eanim->cur_anim->anims[eanim->altidx_cur]->anim_time) {
		if(io) {
			long lost = eanim->ctime - eanim->cur_anim->anims[eanim->altidx_cur]->anim_time;

			if(eanim->next_anim) {
				FinishAnim(io,eanim->cur_anim);

				if (io->lastanimtime!=0)
					AcquireLastAnim(io);
				else
					io->lastanimtime=1;

				eanim->cur_anim=eanim->next_anim;
				eanim->altidx_cur=ANIM_GetAltIdx(eanim->next_anim,eanim->altidx_cur);
				eanim->next_anim=NULL;
				ResetAnim(eanim);
				eanim->ctime = lost;
				eanim->flags=eanim->nextflags;
				eanim->flags&=~EA_ANIMEND;
				goto suite;
			} else {
				eanim->ctime=eanim->cur_anim->anims[eanim->altidx_cur]->anim_time;
				eanim->flags&=~EA_ANIMEND;
			}
		}

		eanim->flags |= EA_ANIMEND;
		eanim->ctime = eanim->cur_anim->anims[eanim->altidx_cur]->anim_time;
	}

suite:

	if (!eanim->cur_anim)
		return;

	long tim;
	if(eanim->flags & EA_REVERSE)
		tim = eanim->cur_anim->anims[eanim->altidx_cur]->anim_time - eanim->ctime;
	else
		tim = eanim->ctime;

	eanim->fr=eanim->cur_anim->anims[eanim->altidx_cur]->nb_key_frames-2;
	eanim->pour=1.f;

	long fr;
	for(long i = 1; i < eanim->cur_anim->anims[eanim->altidx_cur]->nb_key_frames; i++) {
		long tcf = (long)eanim->cur_anim->anims[eanim->altidx_cur]->frames[i-1].time;
		long tnf = (long)eanim->cur_anim->anims[eanim->altidx_cur]->frames[i].time;

		if(tcf == tnf)
			return;

		if((tim < tnf && tim>=tcf) || (i == eanim->cur_anim->anims[eanim->altidx_cur]->nb_key_frames-1 && tim == tnf)) {
			fr = i - 1;
			tim -= tcf;
			float pour = (float)((float)tim/((float)tnf-(float)tcf));
			
			// Frame Sound Management
			if(!(eanim->flags & EA_ANIMEND) && time
			   && (eanim->cur_anim->anims[eanim->altidx_cur]->frames[fr].sample != -1)
			   && (eanim->lastframe != fr)) {

				Vec3f * position = io ? &io->pos : NULL;
				
				if(eanim->lastframe < fr && eanim->lastframe != -1) {
					for(long n = eanim->lastframe + 1; n <= fr; n++)
						ARX_SOUND_PlayAnim(eanim->cur_anim->anims[eanim->altidx_cur]->frames[n].sample, position);
				} else {
					ARX_SOUND_PlayAnim(eanim->cur_anim->anims[eanim->altidx_cur]->frames[fr].sample, position);
				}
			}

			// Frame Flags Management
			if(!(eanim->flags & EA_ANIMEND) && time
			   && (eanim->cur_anim->anims[eanim->altidx_cur]->frames[fr].flag > 0)
			   && (eanim->lastframe != fr)) {
				
				if(io != entities.player()) {
					if(eanim->lastframe < fr && eanim->lastframe != -1) {
						for(long n = eanim->lastframe+1; n<=fr; n++) {
							if(eanim->cur_anim->anims[eanim->altidx_cur]->frames[n].flag == 9)
								ARX_NPC_NeedStepSound(io, &io->pos);
						}
					}
					else if(eanim->cur_anim->anims[eanim->altidx_cur]->frames[fr].flag == 9)
						ARX_NPC_NeedStepSound(io, &io->pos);
				}
			}
			
			// Memorize this frame as lastframe.
			eanim->lastframe = fr;
			eanim->fr = fr;
			eanim->pour = pour;
			break;
		}
	}
}


static void CalcTranslation(ANIM_USE * animuse, Vec3f & ftr) {
	if(!animuse || !animuse->cur_anim)
		return;

	EERIE_ANIM	*eanim = animuse->cur_anim->anims[animuse->altidx_cur];

	if(!eanim)
		return;

	//Avoiding impossible cases
	if(animuse->fr < 0) {
		animuse->fr = 0;
		animuse->pour = 0.f;
	} else if(animuse->fr >= eanim->nb_key_frames - 1) {
		animuse->fr = eanim->nb_key_frames - 2;
		animuse->pour = 1.f;
	}
	animuse->pour = clamp(animuse->pour, 0.f, 1.f);


	// FRAME TRANSLATE : Gives the Virtual pos of Main Object
	if(eanim->frames[animuse->fr].f_translate && !(animuse->flags & EA_STATICANIM)) {
		EERIE_FRAME *sFrame = &eanim->frames[animuse->fr];
		EERIE_FRAME *eFrame = &eanim->frames[animuse->fr+1];

		// Linear interpolation of object translation (MOVE)
		ftr = sFrame->translate + (eFrame->translate - sFrame->translate) * animuse->pour;
	}
}


/* Evaluate main entity translation */
static void Cedric_AnimCalcTranslation(Entity * io, Vec3f & ftr) {

	// Fill frame translate values with multi-layer translate informations...
	for(int count = MAX_ANIM_LAYERS - 1; count >= 0; count--) {
		ANIM_USE *animuse = &io->animlayer[count];

		CalcTranslation(animuse, ftr);
	}
}

static void StoreEntityMovement(Entity * io, Vec3f & ftr, float scale, bool update_movement) {

	if(io && update_movement) {

		Vec3f ftr2 = Vec3f::ZERO;

		if(ftr != Vec3f::ZERO) {
			ftr *= scale;

			float temp;
			if (io == entities.player()) {
				temp = radians(MAKEANGLE(180.f - player.angle.b));
			} else {
				temp = radians(MAKEANGLE(180.f - io->angle.b));
			}

			YRotatePoint(&ftr, &ftr2, (float)EEcos(temp), (float)EEsin(temp));

			// stores Translations for a later use
			io->move = ftr2;
		}

		if(io->animlayer[0].cur_anim) {

			// Use calculated value to notify the Movement engine of the translation to do
			if(io->ioflags & IO_NPC) {
				ftr = Vec3f::ZERO;
				io->move -= io->lastmove;
			} else if (io->gameFlags & GFLAG_ELEVATOR) {
				// Must recover translations for NON-NPC IO
				PushIO_ON_Top(io, io->move.y - io->lastmove.y);
			}

			io->lastmove = ftr2;
		}
	}
}



extern bool EXTERNALVIEW;

void EERIEDrawAnimQuat(EERIE_3DOBJ *eobj, ANIM_USE *eanim, Anglef *angle, Vec3f *pos, unsigned long time, Entity *io, bool render, bool update_movement) {
	
	if(io) {
		float speedfactor = io->basespeed + io->speed_modif;

		if(speedfactor < 0)
			speedfactor = 0;

		time = time * speedfactor;
	}

	if(time > 0) {
		PrepareAnim(eobj,eanim,time,io);

		if(io) {
			for(long count=1; count<MAX_ANIM_LAYERS; count++) {
				ANIM_USE *animuse = &io->animlayer[count];
				if(animuse->cur_anim)
					PrepareAnim(eobj,animuse,time,io);
			}
		}
	}

	// Reset Frame Translate
	Vec3f ftr = Vec3f::ZERO;

	// Set scale and invisibility factors
	float scale = Cedric_GetScale(io);


	if(!io)
		CalcTranslation(eanim, ftr);
	else
		Cedric_AnimCalcTranslation(io, ftr);

	StoreEntityMovement(io, ftr, scale, update_movement);

	if(io && io != entities.player() && !Cedric_IO_Visible(&io->pos))
		return;


	Cedric_AnimateDrawEntity(eobj, eanim, angle, pos, io, ftr, scale);


	bool isFightingNpc = io &&
						 (io->ioflags & IO_NPC) &&
						 (io->_npcdata->behavior & BEHAVIOUR_FIGHT) &&
						 distSqr(io->pos, player.pos) < square(240.f);

	if(!isFightingNpc && ARX_SCENE_PORTAL_ClipIO(io, pos))
		return;

	if(render)
		Cedric_AnimateDrawEntityRender(eobj, pos, ftr, io);
}

// List of TO-TREAT vertex for MIPMESHING

// TODO: Convert to a RenderBatch & make TextureContainer constructor private
TextureContainer TexSpecialColor("specialcolor_list", TextureContainer::NoInsert);

//-----------------------------------------------------------------------------
TexturedVertex * PushVertexInTableCull(TextureContainer *pTex)
{
	if((pTex->ulNbVertexListCull+3)>pTex->ulMaxVertexListCull)
	{
		pTex->ulMaxVertexListCull+=10*3;
		pTex->pVertexListCull = (TexturedVertex *)realloc(pTex->pVertexListCull,
		                         pTex->ulMaxVertexListCull * sizeof(TexturedVertex));
	}

	pTex->ulNbVertexListCull+=3;
	return &pTex->pVertexListCull[pTex->ulNbVertexListCull-3];
}

//-----------------------------------------------------------------------------
TexturedVertex * PushVertexInTableCull_TNormalTrans(TextureContainer *pTex)
{
	if((pTex->ulNbVertexListCull_TNormalTrans+3)>pTex->ulMaxVertexListCull_TNormalTrans)
	{
		pTex->ulMaxVertexListCull_TNormalTrans+=20*3;
		pTex->pVertexListCull_TNormalTrans = (TexturedVertex *)realloc(
		                                      pTex->pVertexListCull_TNormalTrans,
		                                      pTex->ulMaxVertexListCull_TNormalTrans
		                                      * sizeof(TexturedVertex));

		if (!pTex->pVertexListCull_TNormalTrans)
		{
			pTex->ulMaxVertexListCull_TNormalTrans=0;
			pTex->ulNbVertexListCull_TNormalTrans=0;
			return NULL;
		}
	}

	pTex->ulNbVertexListCull_TNormalTrans+=3;
	return &pTex->pVertexListCull_TNormalTrans[pTex->ulNbVertexListCull_TNormalTrans-3];
}

//-----------------------------------------------------------------------------
TexturedVertex * PushVertexInTableCull_TAdditive(TextureContainer *pTex)
{
	if((pTex->ulNbVertexListCull_TAdditive+3)>pTex->ulMaxVertexListCull_TAdditive)
	{
		pTex->ulMaxVertexListCull_TAdditive+=20*3;
		pTex->pVertexListCull_TAdditive = (TexturedVertex * )realloc(
		                                   pTex->pVertexListCull_TAdditive,
		                                   pTex->ulMaxVertexListCull_TAdditive
		                                   * sizeof(TexturedVertex));

		if (!pTex->pVertexListCull_TAdditive)
		{
			pTex->ulMaxVertexListCull_TAdditive=0;
			pTex->ulNbVertexListCull_TAdditive=0;
			return NULL;
		}
	}

	pTex->ulNbVertexListCull_TAdditive+=3;
	return &pTex->pVertexListCull_TAdditive[pTex->ulNbVertexListCull_TAdditive-3];
}

//-----------------------------------------------------------------------------
TexturedVertex * PushVertexInTableCull_TSubstractive(TextureContainer *pTex)
{
	if((pTex->ulNbVertexListCull_TSubstractive+3)>pTex->ulMaxVertexListCull_TSubstractive)
	{
		pTex->ulMaxVertexListCull_TSubstractive+=20*3;
		pTex->pVertexListCull_TSubstractive = (TexturedVertex *)realloc(
		                                       pTex->pVertexListCull_TSubstractive,
		                                       pTex->ulMaxVertexListCull_TSubstractive
		                                       * sizeof(TexturedVertex));

		if (!pTex->pVertexListCull_TSubstractive)
		{
			pTex->ulMaxVertexListCull_TSubstractive=0;
			pTex->ulNbVertexListCull_TSubstractive=0;
			return NULL;
		}
	}

	pTex->ulNbVertexListCull_TSubstractive+=3;
	return &pTex->pVertexListCull_TSubstractive[pTex->ulNbVertexListCull_TSubstractive-3];
}

//-----------------------------------------------------------------------------
TexturedVertex * PushVertexInTableCull_TMultiplicative(TextureContainer *pTex)
{
	if((pTex->ulNbVertexListCull_TMultiplicative+3)>pTex->ulMaxVertexListCull_TMultiplicative)
	{
		pTex->ulMaxVertexListCull_TMultiplicative+=20*3;
		pTex->pVertexListCull_TMultiplicative = (TexturedVertex *)realloc(
		                                         pTex->pVertexListCull_TMultiplicative,
		                                         pTex->ulMaxVertexListCull_TMultiplicative
		                                         * sizeof(TexturedVertex));

		if (!pTex->pVertexListCull_TMultiplicative)
		{
			pTex->ulMaxVertexListCull_TMultiplicative=0;
			pTex->ulNbVertexListCull_TMultiplicative=0;
			return NULL;
		}
	}

	pTex->ulNbVertexListCull_TMultiplicative+=3;
	return &pTex->pVertexListCull_TMultiplicative[pTex->ulNbVertexListCull_TMultiplicative-3];
}

static void PopOneTriangleList(TextureContainer *_pTex) {
	
	if(!_pTex->ulNbVertexListCull) {
		return;
	}
	
	GRenderer->SetCulling(Renderer::CullNone);
	GRenderer->SetTexture(0, _pTex);
	
	if(_pTex->userflags & POLY_LATE_MIP) {
		const float GLOBAL_NPC_MIPMAP_BIAS = -2.2f;
		GRenderer->GetTextureStage(0)->SetMipMapLODBias(GLOBAL_NPC_MIPMAP_BIAS);
	}
	
	
	EERIEDRAWPRIM(Renderer::TriangleList, _pTex->pVertexListCull, _pTex->ulNbVertexListCull);
	
	_pTex->ulNbVertexListCull = 0;
	
	if(_pTex->userflags & POLY_LATE_MIP) {
		float biasResetVal = 0;
		GRenderer->GetTextureStage(0)->SetMipMapLODBias(biasResetVal);
	}
	
}

static void PopOneTriangleListTransparency(TextureContainer *_pTex) {
	
	if(!_pTex->ulNbVertexListCull_TNormalTrans
	   && !_pTex->ulNbVertexListCull_TAdditive
	   && !_pTex->ulNbVertexListCull_TSubstractive
	   && !_pTex->ulNbVertexListCull_TMultiplicative) {
		return;
	}

	GRenderer->SetCulling(Renderer::CullNone);
	GRenderer->SetTexture(0, _pTex);

	if(_pTex->ulNbVertexListCull_TNormalTrans) {
		GRenderer->SetBlendFunc(Renderer::BlendDstColor, Renderer::BlendSrcColor);
		if(_pTex->ulNbVertexListCull_TNormalTrans) {
			EERIEDRAWPRIM(Renderer::TriangleList, _pTex->pVertexListCull_TNormalTrans,
			              _pTex->ulNbVertexListCull_TNormalTrans);
			_pTex->ulNbVertexListCull_TNormalTrans=0;
		}
	}
	
	if(_pTex->ulNbVertexListCull_TAdditive) {
		GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
		if(_pTex->ulNbVertexListCull_TAdditive) {
			EERIEDRAWPRIM(Renderer::TriangleList, _pTex->pVertexListCull_TAdditive,
			              _pTex->ulNbVertexListCull_TAdditive);
			_pTex->ulNbVertexListCull_TAdditive=0;
		}
	}
	
	if(_pTex->ulNbVertexListCull_TSubstractive) {
		GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);
		if(_pTex->ulNbVertexListCull_TSubstractive) {
			EERIEDRAWPRIM(Renderer::TriangleList, _pTex->pVertexListCull_TSubstractive,
			              _pTex->ulNbVertexListCull_TSubstractive);
			_pTex->ulNbVertexListCull_TSubstractive=0;
		}
	}
	
	if(_pTex->ulNbVertexListCull_TMultiplicative) {
		GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
		if(_pTex->ulNbVertexListCull_TMultiplicative) {
			EERIEDRAWPRIM(Renderer::TriangleList, _pTex->pVertexListCull_TMultiplicative,
			              _pTex->ulNbVertexListCull_TMultiplicative);
			_pTex->ulNbVertexListCull_TMultiplicative = 0;
		}
	}
}

void PopAllTriangleList() {
	GRenderer->SetAlphaFunc(Renderer::CmpGreater, .5f);
	TextureContainer * pTex = GetTextureList();
	while(pTex) {
		PopOneTriangleList(pTex);
		pTex = pTex->m_pNext;
	}
	GRenderer->SetAlphaFunc(Renderer::CmpNotEqual, 0.f);
}

//-----------------------------------------------------------------------------
void PopOneInterZMapp(TextureContainer *_pTex)
{
	if(!_pTex->TextureRefinement) return;

	GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);

	if(_pTex->TextureRefinement->vPolyInterZMap.size())
	{
		GRenderer->SetTexture(0, _pTex->TextureRefinement);

		int iPos=0;
		
		vector<SMY_ZMAPPINFO>::iterator it;

		for (it = _pTex->TextureRefinement->vPolyInterZMap.begin();
			it != _pTex->TextureRefinement->vPolyInterZMap.end();
			++it)
		{
			SMY_ZMAPPINFO *pSMY = &(*it);
			
			tTexturedVertexTab2[iPos]        = pSMY->pVertex[0];
			tTexturedVertexTab2[iPos].color  = Color::gray(pSMY->color[0]).toBGR();
			tTexturedVertexTab2[iPos].uv.x   = pSMY->uv[0];
			tTexturedVertexTab2[iPos++].uv.y = pSMY->uv[1];
			tTexturedVertexTab2[iPos]        = pSMY->pVertex[1];
			tTexturedVertexTab2[iPos].color  = Color::gray(pSMY->color[1]).toBGR();
			tTexturedVertexTab2[iPos].uv.x   = pSMY->uv[2];
			tTexturedVertexTab2[iPos++].uv.y = pSMY->uv[3];
			tTexturedVertexTab2[iPos]        = pSMY->pVertex[2];
			tTexturedVertexTab2[iPos].color  = Color::gray(pSMY->color[2]).toBGR();
			tTexturedVertexTab2[iPos].uv.x   = pSMY->uv[4];
			tTexturedVertexTab2[iPos++].uv.y = pSMY->uv[5];
		}

		EERIEDRAWPRIM(Renderer::TriangleList, tTexturedVertexTab2, iPos);

		_pTex->TextureRefinement->vPolyInterZMap.clear();
	}
}

//-----------------------------------------------------------------------------
void PopAllTriangleListTransparency() {

	GRenderer->SetFogColor(Color::none);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetBlendFunc(Renderer::BlendDstColor, Renderer::BlendOne);
	GRenderer->SetAlphaFunc(Renderer::CmpGreater, .5f);

	PopOneTriangleList(&TexSpecialColor);

	TextureContainer * pTex = GetTextureList();

	while(pTex)
	{
		PopOneTriangleListTransparency(pTex);

		//ZMAP
		PopOneInterZMapp(pTex);

		pTex=pTex->m_pNext;
	}

	GRenderer->SetFogColor(ulBKGColor);
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	GRenderer->SetRenderState(Renderer::DepthWrite, true);
	GRenderer->SetAlphaFunc(Renderer::CmpNotEqual, 0.f);
}

float INVISIBILITY_OVERRIDE=0.f;

//-----------------------------------------------------------------------------
void CalculateInterZMapp(EERIE_3DOBJ * _pobj3dObj, long lIdList, long * _piInd,
                         TextureContainer * _pTex, TexturedVertex * _pVertex) {
	
	SMY_ZMAPPINFO sZMappInfo;

	if(!ZMAPMODE || !_pTex->TextureRefinement)
		return;

	bool bUp = false;

	if(fabs(_pobj3dObj->vertexlist[_piInd[0]].norm.y) >= .9f
	   || fabs(_pobj3dObj->vertexlist[_piInd[1]].norm.y) >= .9f
	   || fabs(_pobj3dObj->vertexlist[_piInd[2]].norm.y) >= .9f) {
		bUp = true;
	}

	for(int iI=0; iI<3; iI++) {
		if(bUp) {
			sZMappInfo.uv[iI<<1]=(_pobj3dObj->vertexlist3[_piInd[iI]].v.x*( 1.0f / 50 ));
			sZMappInfo.uv[(iI<<1)+1]=(_pobj3dObj->vertexlist3[_piInd[iI]].v.z*( 1.0f / 50 ));
		} else {
			sZMappInfo.uv[iI<<1]=(_pobj3dObj->facelist[lIdList].u[iI]*4.f);
			sZMappInfo.uv[(iI<<1)+1]=(_pobj3dObj->facelist[lIdList].v[iI]*4.f);
		}

		float fDist = fdist(ACTIVECAM->orgTrans.pos, _pobj3dObj->vertexlist3[_piInd[iI]].v) - 80.f;

		if(fDist < 10.f)
			fDist = 10.f;

		sZMappInfo.color[iI] = (150.f - fDist) * 0.006666666f;

		if(sZMappInfo.color[iI] < 0.f)
			sZMappInfo.color[iI] = 0.f;
	
		sZMappInfo.pVertex[iI]=_pVertex[iI];
	}

	//optim
	if(sZMappInfo.color[0] != 0.f || sZMappInfo.color[1] != 0.f || sZMappInfo.color[2] != 0.f) {
		_pTex->TextureRefinement->vPolyInterZMap.push_back(sZMappInfo);
	}
}





void EE_RT(TexturedVertex * in, Vec3f * out);
void EE_P(Vec3f * in, TexturedVertex * out);

void DrawEERIEInter(EERIE_3DOBJ *eobj, const EERIE_QUAT * rotation, Vec3f *poss, Entity *io, EERIE_MOD_INFO *modinfo, bool thrownEntity) {

	if(!eobj)
		return;
	
	// Resets 2D Bounding Box
	BBOXMIN.y = BBOXMIN.x = 32000;
	BBOXMAX.y = BBOXMAX.x = -32000;
	Vec3f pos = *poss;
	
	// Avoids To treat an object that isn't Visible
	if(io && io != entities.player() && !modinfo && !Cedric_IO_Visible(&pos))
		return;


	TexturedVertex vert_list_static[4];
	
	Color3f infra;

	float scale = Cedric_GetScale(io);
	float invisibility = Cedric_GetInvisibility(io);

	if(!io && INVISIBILITY_OVERRIDE != 0.f)
		invisibility = INVISIBILITY_OVERRIDE;
	

	// Test for Mipmeshing then pre-computes vertices
	ResetBBox3D( io );

	for(size_t i = 0 ; i < eobj->vertexlist.size(); i++) {

		Vec3f temp = eobj->vertexlist[i].v;

		if(modinfo) {
			temp -= modinfo->link_position;
		}

		temp *= scale;

		if(thrownEntity) {
			temp -= io->obj->pbox->vert[0].initpos * scale - io->obj->point0;
		}

		TransformVertexQuat(rotation, &temp, &vert_list_static[1].p);

		eobj->vertexlist3[i].v = vert_list_static[1].p += pos;

		EE_RT(&vert_list_static[1], &eobj->vertexlist[i].vworld);
		EE_P(&eobj->vertexlist[i].vworld, &eobj->vertexlist[i].vert);

		// Memorizes 2D Bounding Box using vertex min/max x,y pos
		if(eobj->vertexlist[i].vert.rhw > 0.f) {

			if ((eobj->vertexlist[i].vert.p.x >= -32000) &&
				(eobj->vertexlist[i].vert.p.x <= 32000) &&
				(eobj->vertexlist[i].vert.p.y >= -32000) &&
				(eobj->vertexlist[i].vert.p.y <= 32000))
			{
				BBOXMIN.x=min(BBOXMIN.x,eobj->vertexlist[i].vert.p.x);
				BBOXMAX.x=max(BBOXMAX.x,eobj->vertexlist[i].vert.p.x);
				BBOXMIN.y=min(BBOXMIN.y,eobj->vertexlist[i].vert.p.y);
				BBOXMAX.y=max(BBOXMAX.y,eobj->vertexlist[i].vert.p.y);
			}
		}

		AddToBBox3D(io,&eobj->vertexlist3[i].v);
	}

	if(io) {
		io->bbox1.x=(short)BBOXMIN.x;
		io->bbox2.x=(short)BBOXMAX.x;
		io->bbox1.y=(short)BBOXMIN.y;
		io->bbox2.y=(short)BBOXMAX.y;
	}

	if(!modinfo && ARX_SCENE_PORTAL_ClipIO(io, &pos))
		return;
	
	long special_color_flag = 0;
	Color3f special_color = Color3f::black;
	if(io) {
		special_color_flag = io->special_color_flag;
		special_color = io->special_color;
	}

	// Precalc local lights for this object then interpolate
	MakeCLight(io, &infra, rotation, &pos, eobj, special_color, special_color_flag);
	
	for(size_t i = 0; i < eobj->facelist.size(); i++) {
		long paf[3];
		paf[0]=eobj->facelist[i].vid[0];
		paf[1]=eobj->facelist[i].vid[1];
		paf[2]=eobj->facelist[i].vid[2];

		//CULL3D
		Vec3f nrm = eobj->vertexlist3[paf[0]].v - ACTIVECAM->orgTrans.pos;

		if(!(eobj->facelist[i].facetype&POLY_DOUBLESIDED)) {
			Vec3f normV10 = eobj->vertexlist3[paf[1]].v - eobj->vertexlist3[paf[0]].v;
			Vec3f normV20 = eobj->vertexlist3[paf[2]].v - eobj->vertexlist3[paf[0]].v;
			Vec3f normFace;
			normFace.x=(normV10.y*normV20.z)-(normV10.z*normV20.y);
			normFace.y=(normV10.z*normV20.x)-(normV10.x*normV20.z);
			normFace.z=(normV10.x*normV20.y)-(normV10.y*normV20.x);

			if(dot(normFace, nrm) > 0.f)
				continue;
		}

		TexturedVertex * vert_list;

		if(eobj->facelist[i].texid<0)
			continue;

		TextureContainer *pTex = eobj->texturecontainer[eobj->facelist[i].texid];
		if(!pTex)
			continue;

		if(io && (io->ioflags & IO_ANGULAR))
			MakeCLight2(io, &infra, rotation, &pos, eobj, i, special_color, special_color_flag);

		float fTransp = 0.f;

		if((eobj->facelist[i].facetype & POLY_TRANS) || invisibility > 0.f) {
			if(invisibility > 0.f)
				fTransp=2.f-invisibility;
			else
				fTransp=eobj->facelist[i].transval;
			
			if(fTransp >= 2.f) { //MULTIPLICATIVE
				fTransp *= (1.0f / 2);
				fTransp += 0.5f;
				vert_list = PushVertexInTableCull_TMultiplicative(pTex);
			} else if(fTransp >= 1.f) { //ADDITIVE
				fTransp -= 1.f;
				vert_list = PushVertexInTableCull_TAdditive(pTex);
			} else if(fTransp > 0.f) { //NORMAL TRANS
				fTransp = 1.f-fTransp;
				vert_list = PushVertexInTableCull_TNormalTrans(pTex);
			} else { //SUBTRACTIVE
				fTransp = 1.f-fTransp;
				vert_list = PushVertexInTableCull_TSubstractive(pTex);
			}
		} else {
			vert_list = PushVertexInTableCull(pTex);
		}

		vert_list[0]=eobj->vertexlist[paf[0]].vert;
		vert_list[1]=eobj->vertexlist[paf[1]].vert;
		vert_list[2]=eobj->vertexlist[paf[2]].vert;

		vert_list[0].uv.x=eobj->facelist[i].u[0];
		vert_list[0].uv.y=eobj->facelist[i].v[0];
		vert_list[1].uv.x=eobj->facelist[i].u[1];
		vert_list[1].uv.y=eobj->facelist[i].v[1];
		vert_list[2].uv.x=eobj->facelist[i].u[2];
		vert_list[2].uv.y=eobj->facelist[i].v[2];	

		// Treat WATER Polys (modify UVs)
		if(eobj->facelist[i].facetype & POLY_WATER) {
			for(long k = 0; k < 3; k++) {
				vert_list[k].uv.x=eobj->facelist[i].u[k];
				vert_list[k].uv.y=eobj->facelist[i].v[k];
				ApplyWaterFXToVertex(&eobj->vertexlist[eobj->facelist[i].vid[k]].v, &vert_list[k], 0.3f);
			}
		}

		if(eobj->facelist[i].facetype & POLY_GLOW) { // unaffected by light
			vert_list[0].color=vert_list[1].color=vert_list[2].color=0xffffffff;
		} else { // Normal Illuminations
			for(long j = 0; j < 3; j++) {
				vert_list[j].color=eobj->vertexlist3[paf[j]].vert.color;
			}
		}

		if(io && Project.improve) {
			int to=3;

			for(long k = 0; k < to; k++) {
				long lr=(vert_list[k].color>>16) & 255;
				float ffr=(float)(lr);

				float dd = vert_list[k].rhw;

				dd = clamp(dd, 0.f, 1.f);

				float fb=((1.f-dd)*6.f + (EEfabs(eobj->vertexlist[paf[k]].norm.x) + EEfabs(eobj->vertexlist[paf[k]].norm.y))) * 0.125f;
				float fr=((.6f-dd)*6.f + (EEfabs(eobj->vertexlist[paf[k]].norm.z) + EEfabs(eobj->vertexlist[paf[k]].norm.y))) * 0.125f;

				if(fr < 0.f)
					fr = 0.f;
				else
					fr = max(ffr, fr * 255.f);

				fr=min(fr,255.f);
				fb*=255.f;
				fb=min(fb,255.f);
				u8 lfr = fr;
				u8 lfb = fb;
				u8 lfg = 0x1E;
				vert_list[k].color = (0xff000000L | (lfr << 16) | (lfg << 8) | (lfb));
			}
		}
	
		for(long j = 0; j < 3; j++)
			eobj->facelist[i].color[j]=Color::fromBGRA(vert_list[j].color);

		// Transparent poly: storing info to draw later
		if((eobj->facelist[i].facetype & POLY_TRANS) || invisibility > 0.f) {
			vert_list[0].color = vert_list[1].color = vert_list[2].color = Color::gray(fTransp).toBGR();
		}

		if(io && (io->ioflags & IO_ZMAP)) {
			CalculateInterZMapp(eobj,i,paf,pTex,vert_list);
		}

		// HALO HANDLING START
		if(io && (io->halo.flags & HALO_ACTIVE)) {

			float mdist=ACTIVECAM->cdepth;
			float ddist = mdist-fdist(pos, ACTIVECAM->orgTrans.pos);
			ddist = ddist/mdist;
			ddist = std::pow(ddist, 6);

			ddist = clamp(ddist, 0.25f, 0.9f);

			float tot=0;
			float _ffr[3];

			TexturedVertex * workon=vert_list;

			for(long o = 0; o < 3; o++) {
				Vec3f temporary3D;
				TransformVertexQuat(rotation, &eobj->vertexlist[paf[o]].norm, &temporary3D);

				float power = 255.f-(float)EEfabs(255.f*(temporary3D.z)*( 1.0f / 2 ));

				power = clamp(power, 0.f, 255.f);

				tot += power;
				_ffr[o] = power;

				u8 lfr = io->halo.color.r * power;
				u8 lfg = io->halo.color.g * power;
				u8 lfb = io->halo.color.b * power;
				workon[o].color = ((0xFF << 24) | (lfr << 16) | (lfg << 8) | (lfb));
			}

			if(tot > 150.f) {
				long first;
				long second;
				long third;

				if(_ffr[0] >= _ffr[1] && _ffr[1] >= _ffr[2]) {
					first = 0;
					second = 1;
					third = 2;
				} else if(_ffr[0] >= _ffr[2] && _ffr[2] >= _ffr[1]) {
					first = 0;
					second = 2;
					third = 1;
				} else if(_ffr[1] >= _ffr[0] && _ffr[0] >= _ffr[2]) {
					first = 1;
					second = 0;
					third = 2;
				} else if(_ffr[1] >= _ffr[2] && _ffr[2] >= _ffr[0]) {
					first = 1;
					second = 2;
					third = 0;
				} else if(_ffr[2] >= _ffr[0] && _ffr[0] >= _ffr[1]) {
					first = 2;
					second = 0;
					third = 1;
				} else {
					first = 2;
					second = 1;
					third = 0;
				}

				if(_ffr[first] > 70.f && _ffr[second] > 60.f) {
					TexturedVertex *vert = &LATERDRAWHALO[(HALOCUR << 2)];

					if(HALOCUR < ((long)HALOMAX) - 1) {
						HALOCUR++;
					}

					memcpy(&vert[0], &workon[first], sizeof(TexturedVertex));
					memcpy(&vert[1], &workon[first], sizeof(TexturedVertex));
					memcpy(&vert[2], &workon[second], sizeof(TexturedVertex));
					memcpy(&vert[3], &workon[second], sizeof(TexturedVertex));

					float siz = ddist * (io->halo.radius * 1.5f * (EEsin((arxtime.get_frame_time() + i) * .01f) * .1f + .7f)) * .6f;

					Vec3f vect1;
					vect1.x = workon[first].p.x - workon[third].p.x;
					vect1.y = workon[first].p.y - workon[third].p.y;
					float len1 = 1.f / ffsqrt(vect1.x * vect1.x + vect1.y * vect1.y);

					if(vect1.x < 0.f)
						len1 *= 1.2f;

					vect1.x *= len1;
					vect1.y *= len1;

					Vec3f vect2;
					vect2.x = workon[second].p.x - workon[third].p.x;
					vect2.y = workon[second].p.y - workon[third].p.y;
					float len2 = 1.f / ffsqrt(vect2.x * vect2.x + vect2.y * vect2.y);

					if(vect2.x < 0.f)
						len2 *= 1.2f;

					vect2.x *= len2;
					vect2.y *= len2;

					vert[1].p.x += (vect1.x + 0.2f - rnd() * 0.1f) * siz;
					vert[1].p.y += (vect1.y + 0.2f - rnd() * 0.1f) * siz;
					vert[1].color = 0xFF000000;

					vert[0].p.z += 0.0001f;
					vert[3].p.z += 0.0001f;
					vert[1].rhw *= .8f;
					vert[2].rhw *= .8f;

					vert[2].p.x += (vect2.x + 0.2f - rnd() * 0.1f) * siz;
					vert[2].p.y += (vect2.y + 0.2f - rnd() * 0.1f) * siz;

					if(io->halo.flags & HALO_NEGATIVE)
						vert[2].color = 0x00000000;
					else
						vert[2].color = 0xFF000000;
				}
			}
		}
	}
}

void ResetAnim(ANIM_USE * eanim)
{
	if(!eanim)
		return;

	eanim->ctime=0;
	eanim->lastframe=-1;
	eanim->flags&=~EA_PAUSED;
	eanim->flags&=~EA_ANIMEND;
	eanim->flags&=~EA_LOOP;
	eanim->flags&=~EA_FORCEPLAY;
}

void llightsInit() {
	for(long i = 0; i < MAX_LLIGHTS; i++) {
		llights[i] = NULL;
		dists[i] = 999999999.f;
		values[i] = 999999999.f;
	}
}

// Inserts Light in the List of Nearest Lights
void Insertllight(EERIE_LIGHT * el,float dist)
{	
	if(!el)
		return;

	float threshold = el->fallend + 560.f;
	if(dist > threshold)
		return;

	float val = dist - el->fallend;

	if(val < 0)
		val=0;

	for(long i=0; i < MAX_LLIGHTS; i++) {
		if(!llights[i]) {
			llights[i]=el;
			dists[i]=dist;
			values[i]=val;
			return;
		} else if (val <= values[i]) { // Inserts light at the right place
			for(long j = MAX_LLIGHTS - 2; j >= i; j--) {
				if(llights[j]) {
					llights[j+1]=llights[j];
					dists[j+1]=dists[j];
					values[j+1]=values[j];
				}
			}

			llights[i]=el;
			dists[i]=dist;
			values[i]=val;
			return;
		}
	}
}

// Precalcs some misc things for lights
void Preparellights(Vec3f * pos) {
	for (long i = 0; i < MAX_LLIGHTS; i++) {
		EERIE_LIGHT * el = llights[i];
		if(el) {
			TCAM[i].orgTrans.pos = el->pos;
			TCAM[i].setTargetCamera(*pos);
			F_PrepareCamera(&TCAM[i]);
		}
	}
}

void EERIE_ANIMMANAGER_Clear(long i) {
	
	for(long k = 0; k < animations[i].alt_nb; k++) {
		ReleaseAnim(animations[i].anims[k]), animations[i].anims[k] = NULL;
	}
	
	free(animations[i].anims), animations[i].anims = NULL;
	
	animations[i].path.clear();
}

void EERIE_ANIMMANAGER_ClearAll() {
	
	for(size_t i = 0; i < MAX_ANIMATIONS; i++) {
		if(!animations[i].path.empty()) {
			EERIE_ANIMMANAGER_Clear(i);
		}
	}
}

void EERIE_ANIMMANAGER_ReloadAll() {
	
	BOOST_FOREACH(Entity * e, entities) {
		if(e) {
			
			for(size_t j = 0; j < MAX_ANIMS; j++) {
				EERIE_ANIMMANAGER_ReleaseHandle(e->anims[j]);
				e->anims[j] = NULL;
			}
			
			for(size_t count = 0; count < MAX_ANIM_LAYERS; count++) {
				memset(&e->animlayer[count], 0, sizeof(ANIM_USE));
				e->animlayer[count].cur_anim = NULL;
				e->animlayer[count].next_anim = NULL;
			}
		}
	}
	
	for(size_t i = 0; i < MAX_ANIMATIONS; i++) {
		if(!animations[i].path.empty()) {
			res::path path = animations[i].path;
			EERIE_ANIMMANAGER_Clear(i);
			EERIE_ANIMMANAGER_Load(path);
		}
	}
}
