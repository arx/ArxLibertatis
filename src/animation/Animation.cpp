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
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved

#include "animation/Animation.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <sstream>

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/size.hpp>

#include "util/String.h"

#include "animation/AnimationFormat.h"

#include "audio/Audio.h"

#include "core/GameTime.h"

#include "game/EntityManager.h"
#include "game/NPC.h"

#include "graphics/BaseGraphicsTypes.h"
#include "graphics/Math.h"

#include "io/resource/ResourcePath.h"
#include "io/resource/PakReader.h"
#include "io/log/Logger.h"

#include "math/Angle.h"

#include "platform/Platform.h"

#include "scene/Object.h"
#include "scene/GameSound.h"

const size_t MAX_ANIMATIONS = 900;
std::vector<ANIM_HANDLE> animations(MAX_ANIMATIONS);

static const long anim_power[] = { 100, 20, 15, 12, 8, 6, 5, 4, 3, 2, 2, 1, 1, 1, 1 };

u16 ANIM_GetAltIdx(const ANIM_HANDLE & ah, u16 old) {
	
	if(ah.anims.size() == 1) {
		return 0;
	}
	
	long total = anim_power[0];
	for(size_t i = 1; i < ah.anims.size(); i++) {
		total += anim_power[std::min(i, size_t(boost::size(anim_power) - 1))];
	}
	
	while(true) {
		for(size_t i = 0; i < ah.anims.size(); i++) {
			if(i == old) {
				continue;
			}
			long r = Random::get(0l, total);
			if(r < anim_power[std::min(i, size_t(boost::size(anim_power) - 1))]) {
				return u16(i);
			}
		}
	}
	
}

void ANIM_Set(AnimLayer & layer, ANIM_HANDLE * anim) {
	
	if(!anim) {
		return;
	}
	
	layer.cur_anim = anim;
	layer.altidx_cur = ANIM_GetAltIdx(*anim, layer.altidx_cur);
	
	// TODO is this > correct ?
	if(layer.altidx_cur > layer.cur_anim->anims.size()) {
		layer.altidx_cur = 0;
	}
	
	layer.ctime = 0;
	layer.lastframe = -1;
	layer.flags &= ~EA_PAUSED;
	layer.flags &= ~EA_ANIMEND;
	layer.flags &= ~EA_LOOP;
	layer.flags &= ~EA_FORCEPLAY;
}

void stopAnimation(Entity * entity, size_t layer) {
	AnimLayer  & animlayer = entity->animlayer[layer];
	AcquireLastAnim(entity);
	FinishAnim(entity, animlayer.cur_anim);
	animlayer.cur_anim = NULL;
}

void changeAnimation(Entity * entity, size_t layer, ANIM_HANDLE * animation,
                          AnimUseType flags, bool startAtBeginning) {
	AnimLayer  & animlayer = entity->animlayer[layer];
	AcquireLastAnim(entity);
	FinishAnim(entity, animlayer.cur_anim);
	ANIM_Set(animlayer, animation);
	animlayer.flags |= flags;
	if(startAtBeginning) {
		animlayer.altidx_cur = 0;
	}
}

void changeAnimation(Entity * entity, ANIM_HANDLE * animation,
                     AnimUseType flags, bool startAtBeginning) {
	changeAnimation(entity, 0, animation, flags, startAtBeginning);
}

void setAnimation(Entity * entity, ANIM_HANDLE * animation,
                  AnimUseType flags, bool startAtBeginning) {
	if(entity->animlayer[0].cur_anim != animation) {
		changeAnimation(entity, animation, flags, startAtBeginning);
	}
}

static void ReleaseAnim(EERIE_ANIM * ea) {
	
	if(!ea) {
		return;
	}
	
	BOOST_FOREACH(EERIE_FRAME & frame, ea->frames) {
		ARX_SOUND_Free(frame.sample);
	}
	
	delete ea;
	
}

static void EERIE_ANIMMANAGER_Clear(ANIM_HANDLE & slot) {
	
	BOOST_FOREACH(EERIE_ANIM * anim, slot.anims) {
		ReleaseAnim(anim);
	}
	slot.anims.clear();
	slot.path.clear();
}

void EERIE_ANIMMANAGER_PurgeUnused() {
	
	BOOST_FOREACH(ANIM_HANDLE & slot, animations) {
		if(!slot.path.empty() && slot.locks == 0) {
			EERIE_ANIMMANAGER_Clear(slot);
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
	
	BOOST_FOREACH(ANIM_HANDLE & slot, animations) {
		if(slot.path == path) {
			return &slot;
		}
	}
	
	return NULL;
}

static float GetTimeBetweenKeyFrames(EERIE_ANIM * ea, long f1, long f2) {
	
	if(!ea || f1 < 0 || f1 > long(ea->frames.size()) - 1
	   || f2 < 0 || f2 > long(ea->frames.size()) - 1) {
		return 0;
	}
	
	AnimationDuration time = 0;
	
	for(long kk = f1 + 1; kk <= f2; kk++) {
		time += ea->frames[kk].time;
	}
	
	return toMsf(time);
}

static EERIE_ANIM * TheaToEerie(const char * adr, size_t size, const res::path & file) {

	(void)size; // TODO use size

	LogDebug("Loading animation file " << file);

	size_t pos = 0;
	
	const THEA_HEADER * th = reinterpret_cast<const THEA_HEADER *>(adr + pos);
	if(th->version < 2014) {
		LogError << "Invalid TEA Version " << th->version << " in " << file;
		return NULL;
	}
	pos += sizeof(THEA_HEADER);
	
	EERIE_ANIM * eerie = new EERIE_ANIM();
	
	LogDebug("TEA header size: " << sizeof(THEA_HEADER));
	LogDebug("Identity " << th->identity);
	LogDebug("Version - " << th->version << "  Frames " << th->nb_frames
	         << "  Groups " << th->nb_groups << "  KeyFrames " << th->nb_key_frames);
	
	size_t keyFrames = size_t(th->nb_key_frames);
	size_t groupCount = size_t(th->nb_groups);
	
	eerie->frames.resize(keyFrames);
	eerie->groups.resize(keyFrames * groupCount);
	eerie->voidgroups.resize(groupCount);
	std::fill(eerie->voidgroups.begin(), eerie->voidgroups.end(), false);

	eerie->anim_time = 0;

	// Go For Keyframes read
	for(size_t i = 0; i < keyFrames; i++) {
		LogDebug("Loading keyframe " << i);

		THEA_KEYFRAME_2015 kf2015;
		const THEA_KEYFRAME_2015 * tkf2015;
		if(th->version >= 2015) {
			LogDebug(" New keyframe version THEA_KEYFRAME_2015:" << sizeof(THEA_KEYFRAME_2015));
			tkf2015 = reinterpret_cast<const THEA_KEYFRAME_2015 *>(adr + pos);
			pos += sizeof(THEA_KEYFRAME_2015);
		} else {
			LogDebug(" Old keyframe version THEA_KEYFRAME_2014:" << sizeof(THEA_KEYFRAME_2014));
			const THEA_KEYFRAME_2014 * tkf = reinterpret_cast<const THEA_KEYFRAME_2014 *>(adr + pos);
			pos += sizeof(THEA_KEYFRAME_2014);
			memset(&kf2015, 0, sizeof(THEA_KEYFRAME_2015));
			kf2015.num_frame = tkf->num_frame;
			kf2015.flag_frame = tkf->flag_frame;
			kf2015.master_key_frame = tkf->master_key_frame;
			kf2015.key_frame = tkf->key_frame;
			kf2015.key_move = tkf->key_move;
			kf2015.key_orient = tkf->key_orient;
			kf2015.key_morph = tkf->key_morph;
			kf2015.time_frame = tkf->time_frame;
			tkf2015 = &kf2015;
		}

		eerie->frames[i].num_frame = tkf2015->num_frame;

		eerie->frames[i].f_rotate = (tkf2015->key_orient != 0);
		eerie->frames[i].f_translate = (tkf2015->key_move != 0);
		
		eerie->frames[i].time = AnimationDurationUs(s64(tkf2015->num_frame) * 1000 * 1000 / 24);
		
		arx_assert(tkf2015->flag_frame == -1 || tkf2015->flag_frame == 9);
		eerie->frames[i].stepSound = (tkf2015->flag_frame == 9);
		
		LogDebug(" pos " << pos << " - NumFr " << eerie->frames[i].num_frame
		         << " MKF " << tkf2015->master_key_frame << " THEA_KEYFRAME " << sizeof(THEA_KEYFRAME_2014)
		         << " TIME " << toS(eerie->frames[i].time) << "s -Move " << tkf2015->key_move
		         << " Orient " << tkf2015->key_orient << " Morph " << tkf2015->key_morph);
		
		// Is There a Global translation ?
		if(tkf2015->key_move != 0) {

			const THEA_KEYMOVE * tkm = reinterpret_cast<const THEA_KEYMOVE *>(adr + pos);
			pos += sizeof(THEA_KEYMOVE);
			
			LogDebug(" -> move x " << tkm->x << " y " << tkm->y << " z " << tkm->z
			         << " THEA_KEYMOVE:" << sizeof(THEA_KEYMOVE));
			
			eerie->frames[i].translate = tkm->toVec3();
		}

		// Is There a Global Rotation ?
		if(tkf2015->key_orient != 0) {
			pos += 8; // THEO_ANGLE

			const ArxQuat * quat = reinterpret_cast<const ArxQuat *>(adr + pos);
			pos += sizeof(ArxQuat);
			
			LogDebug(" -> rotate x " << quat->x << " y " << quat->y << " z " << quat->z
			         << " w " << quat->w << " ArxQuat:" << sizeof(ArxQuat));
			
			eerie->frames[i].quat = *quat;
		}

		// Is There a Global Morph ? (IGNORED!)
		if(tkf2015->key_morph != 0) {
			pos += 16; // THEA_MORPH
		}

		// Now go for Group Rotations/Translations/scaling for each GROUP
		for(size_t j = 0; j < groupCount; j++) {

			const THEO_GROUPANIM * tga = reinterpret_cast<const THEO_GROUPANIM *>(adr + pos);
			pos += sizeof(THEO_GROUPANIM);

			EERIE_GROUP * eg = &eerie->groups[j + i * groupCount];
			eg->key = tga->key_group;
			eg->quat = tga->Quaternion;
			eg->translate = tga->translate.toVec3();
			eg->zoom = tga->zoom.toVec3();
		}

		// Now Read Sound Data included in this frame
		s32 num_sample = *reinterpret_cast<const s32 *>(adr + pos);
		pos += sizeof(s32);
		LogDebug(" -> num_sample " << num_sample << " s32:" << sizeof(s32));

		eerie->frames[i].sample = audio::SampleHandle();
		if(num_sample != -1) {

			const THEA_SAMPLE * ts = reinterpret_cast<const THEA_SAMPLE *>(adr + pos);
			pos += sizeof(THEA_SAMPLE);
			pos += ts->sample_size;
			
			LogDebug(" -> sample " << ts->sample_name << " size " << ts->sample_size
			         << " THEA_SAMPLE:" << sizeof(THEA_SAMPLE));
			
			eerie->frames[i].sample = ARX_SOUND_Load(res::path::load(util::loadString(ts->sample_name)));
		}

		pos += 4; // num_sfx
	}
	
	for(size_t i = 0; i < keyFrames; i++) {
		
		if(!eerie->frames[i].f_translate) {
			
			long k = i;
			while(k >= 0 && !eerie->frames[k].f_translate) {
				k--;
			}
			
			size_t j = i;
			while(j < keyFrames && !eerie->frames[j].f_translate) {
				j++;
			}
			
			if(j < keyFrames && k >= 0) {
				float r1 = GetTimeBetweenKeyFrames(eerie, k, i);
				float r2 = GetTimeBetweenKeyFrames(eerie, i, j);
				float tot = 1.f / (r1 + r2);
				r1 *= tot;
				r2 *= tot;
				eerie->frames[i].translate = eerie->frames[j].translate * r1 + eerie->frames[k].translate * r2;
			}
			
		}
		
		if(!eerie->frames[i].f_rotate) {
			
			long k = i;
			while(k >= 0 && !eerie->frames[k].f_rotate) {
				k--;
			}
			
			size_t j = i;
			while(j < keyFrames && !eerie->frames[j].f_rotate) {
				j++;
			}
			
			if(j < keyFrames && k >= 0) {
				float r1 = GetTimeBetweenKeyFrames(eerie, k, i);
				float r2 = GetTimeBetweenKeyFrames(eerie, i, j);
				float tot = 1.f / (r1 + r2);
				r1 *= tot;
				r2 *= tot;
				// TODO use overloaded operators
				eerie->frames[i].quat.w = eerie->frames[j].quat.w * r1 + eerie->frames[k].quat.w * r2;
				eerie->frames[i].quat.x = eerie->frames[j].quat.x * r1 + eerie->frames[k].quat.x * r2;
				eerie->frames[i].quat.y = eerie->frames[j].quat.y * r1 + eerie->frames[k].quat.y * r2;
				eerie->frames[i].quat.z = eerie->frames[j].quat.z * r1 + eerie->frames[k].quat.z * r2;
			}
			
		}
		
	}

	for(size_t i = 0; i < keyFrames; i++) {
		eerie->frames[i].f_translate = true;
		eerie->frames[i].f_rotate = true;
	}
	
	// Sets Flag for voidgroups (unmodified groups for whole animation)
	for(size_t i = 0; i < eerie->nb_groups(); i++) {
		
		bool voidd = true;
		for(size_t j = 0; j < eerie->frames.size(); j++) {
			size_t group = i + (j * eerie->nb_groups());
			if(eerie->groups[group].quat != quat_identity()
			   || eerie->groups[group].translate != Vec3f(0.f)
			   || eerie->groups[group].zoom != Vec3f(0.f)) {
				voidd = false;
				break;
			}
		}
		
		if(voidd) {
			eerie->voidgroups[i] = true;
		}
		
	}
	
	eerie->anim_time = AnimationDurationUs(s64(th->nb_frames) * 1000 * 1000 / 24);
	if(eerie->anim_time < AnimationDurationMs(1)) {
		eerie->anim_time = AnimationDurationMs(1);
	}

	LogDebug("Finished Conversion TEA -> EERIE - " << toS(eerie->anim_time) << " seconds");

	return eerie;
}



static bool EERIE_ANIMMANAGER_AddAltAnim(ANIM_HANDLE & ah, const res::path & path) {
	
	if(ah.path.empty()) {
		return false;
	}
	
	std::string buffer = g_resources->read(path);
	if(buffer.empty()) {
		return false;
	}
	
	EERIE_ANIM * anim = TheaToEerie(buffer.data(), buffer.size(), path);
	if(!anim) {
		return false;
	}
	
	ah.anims.push_back(anim);
	
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
	
	ANIM_HANDLE * handle = EERIE_ANIMMANAGER_GetHandle(path);
	if(handle) {
		handle->locks++;
		return handle;
	}
	
	BOOST_FOREACH(ANIM_HANDLE & slot, animations) {
		
		if(!slot.path.empty()) {
			continue;
		}
		
		std::string buffer = g_resources->read(path);
		if(buffer.empty()) {
			return NULL;
		}
		
		EERIE_ANIM * anim = TheaToEerie(buffer.data(), buffer.size(), path);
		if(!anim) {
			return NULL;
		}
		
		slot.anims.push_back(anim);
		slot.path = path;
		slot.locks = 1;
		
		int pathcount = 2;
		res::path altpath;
		do {
			altpath = res::path(path);
			altpath.append_basename(boost::lexical_cast<std::string>(pathcount++));
		} while(EERIE_ANIMMANAGER_AddAltAnim(slot, altpath));
		
		return &slot;
	}
	
	return NULL;
}

/*!
 * \brief Fill "pos" with "eanim" total translation
 */
Vec3f GetAnimTotalTranslate(ANIM_HANDLE * eanim, size_t alt_idx) {
	
	if(!eanim || !eanim->anims[alt_idx] || eanim->anims[alt_idx]->frames.empty()) {
		return Vec3f(0.f);
	}
	
	size_t idx = eanim->anims[alt_idx]->frames.size() - 1;
	return eanim->anims[alt_idx]->frames[idx].translate;
}

/*!
 * \brief Main Procedure to draw an animated object
 *
 * \param time Time increment to current animation in Ms
 * \param io Referrence to Interactive Object (NULL if no IO)
 */
void PrepareAnim(AnimLayer & layer, AnimationDuration time, Entity * io) {
	
	arx_assert(layer.cur_anim);
	
	if(layer.flags & EA_PAUSED)
		time = 0;

	if(io && (io->ioflags & IO_FREEZESCRIPT))
		time = 0;
	
	if(layer.altidx_cur >= layer.cur_anim->anims.size()) {
		layer.altidx_cur = 0;
	}
	
	if(!(layer.flags & EA_EXCONTROL))
		layer.ctime += time;

	layer.flags &= ~EA_ANIMEND;

	AnimationDuration animTime = layer.currentAltAnim()->anim_time;
	
	if(layer.ctime > animTime) {
	
		if(layer.flags & EA_STOPEND) {
			layer.ctime = animTime;
		}
		
		if((layer.flags & EA_LOOP)
		   || (io && ((layer.cur_anim == io->anims[ANIM_WALK])
		              || (layer.cur_anim == io->anims[ANIM_WALK2])
		              || (layer.cur_anim == io->anims[ANIM_WALK3])
		              || (layer.cur_anim == io->anims[ANIM_RUN])
		              || (layer.cur_anim == io->anims[ANIM_RUN2])
		              || (layer.cur_anim == io->anims[ANIM_RUN3])))) {
			layer.ctime = AnimationDuration::ofRaw(layer.ctime.t % animTime.t);
			if(io)
				FinishAnim(io, layer.cur_anim);
		} else {
			layer.flags |= EA_ANIMEND;
			layer.ctime = layer.currentAltAnim()->anim_time;
		}
	
	}
	
	AnimationDuration tim;
	if(layer.flags & EA_REVERSE)
		tim = animTime - layer.ctime;
	else
		tim = layer.ctime;
	
	const EERIE_ANIM & anim = *layer.currentAltAnim();
	
	layer.currentFrame = long(anim.frames.size()) - 2;
	layer.currentInterpolation = 1.f;
	
	for(size_t i = 1; i < anim.frames.size(); i++) {
		AnimationDuration tcf = anim.frames[i - 1].time;
		AnimationDuration tnf = anim.frames[i].time;

		if(tcf == tnf)
			return;

		if((tim < tnf && tim >= tcf) || (i == anim.frames.size() - 1 && tim == tnf)) {
			const size_t fr = i - 1;
			tim -= tcf;
			float pour = toMsf(tim) / toMsf(tnf - tcf);
			
			if(!(layer.flags & EA_ANIMEND)
			   && time != 0
			   && layer.lastframe != long(fr)) {
			
				// Frame Sound Management
				if(anim.frames[fr].sample != audio::SampleHandle()) {
					Vec3f * position = io ? &io->pos : NULL;
					
					if(layer.lastframe < long(fr) && layer.lastframe != -1) {
						for(size_t n = size_t(layer.lastframe) + 1; n <= fr; n++) {
							ARX_SOUND_PlayAnim(anim.frames[n].sample, position);
						}
					} else {
						ARX_SOUND_PlayAnim(anim.frames[fr].sample, position);
					}
				}
				
				// Frame Flags Management
				if(anim.frames[fr].stepSound && io && io != entities.player()) {
					
					if(layer.lastframe < long(fr) && layer.lastframe != -1) {
						for(size_t n = size_t(layer.lastframe) + 1; n <= fr; n++) {
							if(anim.frames[n].stepSound) {
								ARX_NPC_NeedStepSound(io, io->pos);
							}
						}
					} else if(anim.frames[fr].stepSound) {
						ARX_NPC_NeedStepSound(io, io->pos);
					}
				}
			}
			
			// Memorize this frame as lastframe.
			layer.lastframe = long(fr);
			layer.currentFrame = long(fr);
			layer.currentInterpolation = pour;
			break;
		}
	}
}


void ResetAnim(AnimLayer & layer) {
	
	layer.ctime = 0;
	layer.lastframe = -1;
	layer.flags &= ~(EA_PAUSED | EA_ANIMEND | EA_LOOP | EA_FORCEPLAY);
	
}

void EERIE_ANIMMANAGER_ClearAll() {
	
	BOOST_FOREACH(ANIM_HANDLE & slot, animations) {
		if(!slot.path.empty()) {
			EERIE_ANIMMANAGER_Clear(slot);
		}
	}
}

/*!
 * \brief Memorizes information for animation to animation smoothing interpolation
 * \param io the animated Entity
 */
void AcquireLastAnim(Entity * io) {
	
	if(!io->animlayer[0].cur_anim && !io->animlayer[1].cur_anim
	   && !io->animlayer[2].cur_anim && !io->animlayer[3].cur_anim) {
		return;
	}
	
	// Stores Frametime and number of vertex for later interpolation
	io->animBlend.lastanimtime = g_gameTime.now();
	io->animBlend.m_active = true;
}

// Declares an Animation as finished.
// Usefull to update object true position with object virtual pos.
void FinishAnim(Entity * io, ANIM_HANDLE * eanim) {
	
	if(!io || !eanim) {
		return;
	}
	
	// Only layer 0 controls movement...
	if(eanim == io->animlayer[0].cur_anim && (io->ioflags & IO_NPC)) {
		io->move = io->lastmove = Vec3f(0.f);
	}
	
}

std::vector< std::pair<res::path, size_t> > ARX_SOUND_PushAnimSamples() {
	
	std::vector< std::pair<res::path, size_t> > samples;
	
	size_t number = 0;
	
	BOOST_FOREACH(const ANIM_HANDLE & slot, animations) {
		if(slot.path.empty()) {
			continue;
		}
		BOOST_FOREACH(const EERIE_ANIM * anim, slot.anims) {
			BOOST_FOREACH(const EERIE_FRAME & frame, anim->frames) {
				number++;
				if(frame.sample != audio::SampleHandle()) {
					res::path dest;
					audio::getSampleName(frame.sample, dest);
					if(!dest.empty()) {
						samples.push_back(std::make_pair(dest, number));
					}
				}
			}
		}
	}
	
	return samples;
}

void ARX_SOUND_PopAnimSamples(const std::vector< std::pair<res::path, size_t> > & samples) {
	
	if(samples.empty() || !ARX_SOUND_IsEnabled()) {
		return;
	}
	
	std::vector< std::pair<res::path, size_t> >::const_iterator p = samples.begin();
	
	size_t number = 0;
	
	BOOST_FOREACH(ANIM_HANDLE & slot, animations) {
		if(slot.path.empty()) {
			continue;
		}
		BOOST_FOREACH(EERIE_ANIM * anim, slot.anims) {
			BOOST_FOREACH(EERIE_FRAME & frame, anim->frames) {
				number++;
				if(p != samples.end() && p->second == number) {
					frame.sample = audio::createSample(p->first);
					++p;
				}
			}
		}
	}
	
}

void ReleaseAnimFromIO(Entity * io, long num) {

	for(size_t count = 0; count < MAX_ANIM_LAYERS; count++) {
		AnimLayer & layer = io->animlayer[count];
		
		if(layer.cur_anim == io->anims[num]) {
			layer = AnimLayer();
			layer.cur_anim = NULL;
		}
	}

	EERIE_ANIMMANAGER_ReleaseHandle(io->anims[num]);
	io->anims[num] = NULL;
}
