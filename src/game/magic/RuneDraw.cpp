/*
 * Copyright 2014 Arx Libertatis Team (see the AUTHORS file)
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

#include "game/magic/RuneDraw.h"

#include <boost/foreach.hpp>

#include "core/Core.h"
#include "core/GameTime.h"
#include "game/Entity.h"
#include "game/EntityManager.h"
#include "graphics/particle/MagicFlare.h"
#include "math/Types.h"

extern EERIE_CAMERA subj;

static const Vec2s symbolVecScale(8*2, 6*2);



static Vec2s GetSymbVector(char c) {
	switch(c) {
		case '1': return Vec2s(-1,  1);
		case '2': return Vec2s( 0,  1);
		case '3': return Vec2s( 1,  1);
		case '4': return Vec2s(-1,  0);
		case '6': return Vec2s( 1,  0);
		case '7': return Vec2s(-1, -1);
		case '8': return Vec2s( 0, -1);
		case '9': return Vec2s( 1, -1);
		default : return Vec2s( 0,  0);
	}
}

static void ReCenterSequence(const std::string & _pcSequence, Vec2s & iMin, Vec2s & iMax) {
	
	Vec2s iSize = Vec2s(0, 0);
	iMin = Vec2s(0, 0);
	iMax = Vec2s(0, 0);
	
	for(size_t iI = 0; iI < _pcSequence.length(); iI++) {
		Vec2s es2dVector = GetSymbVector(_pcSequence[iI]);
		es2dVector *= symbolVecScale;
		iSize += es2dVector;
		iMin = glm::min(iMin, iSize);
		iMax = glm::max(iMax, iSize);
	}
}

static Vec2s lMaxSymbolDrawSize;

//-----------------------------------------------------------------------------
// Initializes Spell engine (Called once at DANAE startup)
void ARX_SPELLS_Init_Rects() {
	lMaxSymbolDrawSize.x = std::numeric_limits<s16>::min();
	lMaxSymbolDrawSize.y = std::numeric_limits<s16>::min();

	BOOST_FOREACH(RuneInfo & info, runeInfos) {
		
		Vec2s iMin;
		Vec2s iMax;
		ReCenterSequence(info.sequence.c_str(), iMin, iMax);
		
		Vec2s iSize = iMax - iMin;
		lMaxSymbolDrawSize = glm::max(iSize, lMaxSymbolDrawSize);
	}
}

void ARX_SPELLS_UpdateSymbolDraw() {
	
	ArxInstant now = arxtime.now_ul();
	
	for(size_t i = 0; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		Entity * io = entities[handle];
		if(!io)
			continue;

		if(io->spellcast_data.castingspell != SPELL_NONE) {
			if(!io->symboldraw) {
				bool tst = false;

				if(!(io->spellcast_data.spell_flags & SPELLCAST_FLAG_NOANIM) && (io->ioflags & IO_NPC)) {
					AnimLayer & layer1 = io->animlayer[1];

					if(layer1.cur_anim == io->anims[ANIM_CAST_START]  && (layer1.flags & EA_ANIMEND)) {
						// TODO why no AcquireLastAnim() like everywhere else?
						FinishAnim(io, layer1.cur_anim);
						ANIM_Set(layer1, io->anims[ANIM_CAST_CYCLE]);
						tst = true;
					} else if(layer1.cur_anim == io->anims[ANIM_CAST_CYCLE]) {
						tst = true;
					} else if(layer1.cur_anim != io->anims[ANIM_CAST_START]) {
						io->spellcast_data.castingspell = SPELL_NONE;
					}
				} else {
					tst = true;
				}

				if(io->spellcast_data.symb[0] != RUNE_NONE && tst) {
					Rune symb = io->spellcast_data.symb[0];

					for(long j = 0; j < 3; j++) {
						io->spellcast_data.symb[j] = io->spellcast_data.symb[j+1];
					}

					io->spellcast_data.symb[3] = RUNE_NONE;
					float speedFactor = std::max(io->speed_modif + io->basespeed, 0.01f);
					float duration = (1000 - (io->spellcast_data.spell_level * 60)) * speedFactor;
					ARX_SPELLS_RequestSymbolDraw2(io, symb, duration);
					io->gameFlags &= ~GFLAG_INVISIBILITY;
				} else if(tst) { // cast spell !!!
					io->gameFlags &= ~GFLAG_INVISIBILITY;
					
					ARX_SPELLS_Launch(io->spellcast_data.castingspell,
					                  handle,
					                  io->spellcast_data.spell_flags,
					                  io->spellcast_data.spell_level,
					                  io->spellcast_data.target,
					                  io->spellcast_data.duration);

					if(!(io->spellcast_data.spell_flags & SPELLCAST_FLAG_NOANIM) && (io->ioflags & IO_NPC)) {
						changeAnimation(io, 1, io->anims[ANIM_CAST]);
					}
					io->spellcast_data.castingspell = SPELL_NONE;
				}
			}
		}

		if(io->flarecount) {
			if(!lightHandleIsValid(io->dynlight))
				io->dynlight = GetFreeDynLight();

			if(lightHandleIsValid(io->dynlight)) {
				EERIE_LIGHT * light = lightHandleGet(io->dynlight);
				
				light->pos = io->pos;
				light->pos += angleToVectorXZ(io->angle.getPitch() - 45.f) * 60.f;
				light->pos += Vec3f(0.f, -120.f, 0.f);
				
				float rr = Random::getf();
				light->fallstart = 140.f + io->flarecount * 0.333333f + rr * 5.f;
				light->fallend = 220.f + io->flarecount * 0.5f + rr * 5.f;
				light->intensity=1.6f;
				light->rgb.r=0.01f*io->flarecount*2;
				light->rgb.g=0.009f*io->flarecount*2;
				light->rgb.b=0.008f*io->flarecount*2;
			}
		} else if(lightHandleIsValid(io->dynlight)) {
			lightHandleGet(io->dynlight)->exist = 0;
			io->dynlight = LightHandle();
		}

		if(io->symboldraw) {
			SYMBOL_DRAW * sd = entities[handle]->symboldraw;
			long elapsed = now - sd->starttime;

			if(elapsed > sd->duration) {
				endLightDelayed(io->dynlight, 600);
				io->dynlight = LightHandle();
				
				delete io->symboldraw;
				io->symboldraw = NULL;
				continue;
			}

			const size_t nbcomponents = sd->sequence.length();

			if(nbcomponents == 0) {
				delete io->symboldraw;
				io->symboldraw = NULL;
				continue;
			}

			float ti = ((float)sd->duration/(float)nbcomponents);

			if(ti <= 0)
				ti = 1;
			
			long newtime=elapsed;
			long oldtime = sd->lastElapsed;

			if(oldtime>sd->duration)
				oldtime=sd->duration;

			if(newtime>sd->duration)
				newtime=sd->duration;

			sd->lastElapsed = (short)elapsed;
			
			float div_ti=1.f/ti;

			if(io != entities.player()) {
				
				Vec2s pos1 = Vec2s(subj.center) - symbolVecScale * short(2) + sd->cPosStart * symbolVecScale;
				
				Vec2s old_pos = pos1;

				for(size_t j = 0; j < nbcomponents; j++) {
					Vec2s vect = GetSymbVector(sd->sequence[j]);
					vect *= symbolVecScale;
					vect += vect / Vec2s(2);

					if(oldtime <= ti) {
						float ratio = float(oldtime)*div_ti;
						old_pos += Vec2s(Vec2f(vect) * ratio);
						break;
					}

					old_pos += vect;
					oldtime -= (long)ti;
				}

				for(size_t j = 0; j < nbcomponents; j++) {
					Vec2s vect = GetSymbVector(sd->sequence[j]);
					vect *= symbolVecScale;
					vect += vect / Vec2s(2);

					if(newtime <= ti) {
						float ratio = float(newtime) * div_ti;
						pos1 += Vec2s(Vec2f(vect) * ratio);
						AddFlare(pos1, 0.1f, 1, entities[handle]);
						FlareLine(old_pos, pos1, entities[handle]);
						break;
					}

					pos1 += vect;
					newtime -= (long)ti;
				}
			} else {
				Vec2s iMin;
				Vec2s iMax;
				
				ReCenterSequence(sd->sequence, iMin, iMax);
				Vec2s iSize = iMax - iMin;
				Vec2s pos1 = Vec2s(97, 64);
				
				Vec2s lPos;
				lPos.x = (((513>>1)-lMaxSymbolDrawSize.x)>>1);
				lPos.y = (313-(((313*3/4)-lMaxSymbolDrawSize.y)>>1));

				pos1 += lPos;
				pos1 += (lMaxSymbolDrawSize - iSize) / Vec2s(2);
				pos1 -= iMin;

				for(size_t j = 0; j < nbcomponents; j++) {

					Vec2s vect = GetSymbVector(sd->sequence[j]);
					vect *= symbolVecScale;

					if(newtime < ti) {
						float ratio = (float)(newtime) * div_ti;
						
						pos1 += Vec2s(Vec2f(ratio) * Vec2f(vect) * 0.5f);
						
						Vec2s pos = Vec2s(Vec2f(pos1) * g_sizeRatio);
						AddFlare(pos, 0.1f, 1, entities[handle], true);

						break;
					}

					pos1 += vect;

					newtime -= (long)ti;
				}
			}
		}
	}
}

void ARX_SPELLS_ClearAllSymbolDraw() {
	BOOST_FOREACH(Entity * e, entities) {
		if(e && e->symboldraw) {
			delete e->symboldraw;
			e->symboldraw = NULL;
		}
	}
}

static void ARX_SPELLS_RequestSymbolDrawCommon(Entity * io, float duration,
                                               RuneInfo & info) {
	
	if(!io->symboldraw)
		io->symboldraw = new SYMBOL_DRAW;
	
	
	SYMBOL_DRAW *sd = io->symboldraw;

	sd->duration = (short)std::max(1l, long(duration));
	sd->sequence = info.sequence;

	sd->starttime = arxtime.now_ul();
	sd->lastElapsed = 0;
	
	float tmpAngle = io->angle.getPitch() - 45.0F + info.startOffset.x * 2;
	
	sd->lastpos = io->pos;
	sd->lastpos += angleToVectorXZ(tmpAngle) * 60.f;
	sd->lastpos += Vec3f(0.f, -120.0f, 0.f);
	sd->lastpos += Vec3f(0.f, -info.startOffset.y * 5, 0.f);
	
	sd->cPosStart = info.startOffset;

	io->gameFlags &= ~GFLAG_INVISIBILITY;
}
	

void ARX_SPELLS_RequestSymbolDraw(Entity *io, const std::string & name, float duration) {

	BOOST_FOREACH(RuneInfo & info, runeInfos) {
		if(info.name == name) {
			ARX_SPELLS_RequestSymbolDrawCommon(io, duration, info);
			break;
		}
	}
}

void ARX_SPELLS_RequestSymbolDraw2(Entity *io, Rune symb, float duration)
{
	BOOST_FOREACH(RuneInfo & info, runeInfos) {
		if(info.rune == symb) {
			ARX_SPELLS_RequestSymbolDrawCommon(io, duration, info);
			break;
		}
	}
}

