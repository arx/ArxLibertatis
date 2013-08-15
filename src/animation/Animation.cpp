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
#include "graphics/effects/Halo.h"

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

ANIM_HANDLE animations[MAX_ANIMATIONS];

static const long anim_power[] = { 100, 20, 15, 12, 8, 6, 5, 4, 3, 2, 2, 1, 1, 1, 1 };

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
void PrepareAnim(ANIM_USE *eanim, unsigned long time, Entity *io) {
	
	if(!eanim)
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
