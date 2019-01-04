/*
 * Copyright 2014-2018 Arx Libertatis Team (see the AUTHORS file)
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

static const Vec2s symbolVecScale(16, 12);

SYMBOL_DRAW g_bookSymbolDraw;

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
		ReCenterSequence(info.sequence, iMin, iMax);
		
		Vec2s iSize = iMax - iMin;
		lMaxSymbolDrawSize = glm::max(iSize, lMaxSymbolDrawSize);
	}
}

static void updateIOLight(Entity * io) {
	
	if(io->flarecount) {
		EERIE_LIGHT * light = dynLightCreate(io->dynlight);
		if(light) {
			light->pos = io->pos;
			light->pos += angleToVectorXZ(io->angle.getYaw() - 45.f) * 60.f;
			light->pos += Vec3f(0.f, -120.f, 0.f);
			float rr = Random::getf();
			light->fallstart = 140.f + io->flarecount * 0.333333f + rr * 5.f;
			light->fallend = 220.f + io->flarecount * 0.5f + rr * 5.f;
			light->intensity = 1.6f;
			light->rgb = Color3f(0.02f, 0.018f, 0.016f) * io->flarecount;
		}
	} else {
		lightHandleDestroy(io->dynlight);
	}
	
}

void ARX_SPELLS_UpdateBookSymbolDraw(const Rect & rect) {

	if(g_bookSymbolDraw.sequence.empty()) {
		return;
	}
	
	GameInstant now = g_gameTime.now();

	SYMBOL_DRAW * sd = &g_bookSymbolDraw;
	AnimationDuration elapsed = toAnimationDuration(now - sd->starttime);
	sd->elapsed = elapsed;

	const size_t nbcomponents = sd->sequence.length();

	if(elapsed > sd->duration || nbcomponents == 0) {
		sd->sequence.clear();
		return;
	}
		
	AnimationDuration timePerComponent = sd->duration * (1.0f / float(nbcomponents));

	if(timePerComponent <= 0)
		timePerComponent = AnimationDurationMs(1);

	AnimationDuration timeRemaining = elapsed;
	
	if(timeRemaining > sd->duration)
		timeRemaining = sd->duration;
	
	// Keep size ratios among runes
	Vec2f rectToSymbolsRatio = Vec2f(rect.size()) / (Vec2f(lMaxSymbolDrawSize));
	Vec2f scale = Vec2f(glm::min(rectToSymbolsRatio.x, rectToSymbolsRatio.y));

	Vec2s iMin;
	Vec2s iMax;

	ReCenterSequence(sd->sequence, iMin, iMax);
	Vec2f size = Vec2f(iMax - iMin) * scale;

	Vec2f scaledMin = Vec2f(iMin) * scale;

	Vec2f pos = Vec2f(rect.center()) - size / 2.0f - scaledMin;

	for(size_t j = 0; j < nbcomponents; j++) {

		Vec2f vect = Vec2f(GetSymbVector(sd->sequence[j]));
		vect *= symbolVecScale;
		vect *= scale;

		if(timeRemaining < timePerComponent) {
			float ratio = timeRemaining / timePerComponent;
			pos += vect * ratio * 0.5f;
			AddFlare(pos, glm::min(scale.x, scale.y) * 0.2f, 1, entities.player(), true);

			break;
		}

		pos += vect;

		timeRemaining -= timePerComponent;
	}
}

void ARX_SPELLS_UpdateSymbolDraw() {
	
	GameInstant now = g_gameTime.now();
	
	for(size_t i = 0; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		Entity * io = entities[handle];
		if(!io || io == entities.player())
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

				if(tst) {
					if(io->spellcast_data.symb[0] != RUNE_NONE) {
						Rune symb = io->spellcast_data.symb[0];

						for(long j = 0; j < 3; j++) {
							io->spellcast_data.symb[j] = io->spellcast_data.symb[j + 1];
						}

						io->spellcast_data.symb[3] = RUNE_NONE;
						float speedFactor = std::max(io->speed_modif + io->basespeed, 0.01f);
						float duration = (1000 - (io->spellcast_data.spell_level * 60)) * speedFactor;
						ARX_SPELLS_RequestSymbolDraw2(io, symb, GameDurationMsf(duration));
						io->gameFlags &= ~GFLAG_INVISIBILITY;
					} else { // cast spell !!!
						io->gameFlags &= ~GFLAG_INVISIBILITY;
						ARX_SPELLS_Launch(io->spellcast_data.castingspell, handle, io->spellcast_data.spell_flags,
						                  io->spellcast_data.spell_level, io->spellcast_data.target,
						                  io->spellcast_data.duration);
						if(!(io->spellcast_data.spell_flags & SPELLCAST_FLAG_NOANIM) && (io->ioflags & IO_NPC)) {
							changeAnimation(io, 1, io->anims[ANIM_CAST]);
						}
						io->spellcast_data.castingspell = SPELL_NONE;
					}
				}
			}
		}

		updateIOLight(io);

		if(io->symboldraw) {
			SYMBOL_DRAW * sd = io->symboldraw;
			AnimationDuration elapsed = toAnimationDuration(now - sd->starttime);

			if(elapsed > sd->duration) {
				endLightDelayed(io->dynlight, GameDurationMs(600));
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
			
			float ti = toMsf(sd->duration) / float(nbcomponents);
			if(ti <= 0) {
				ti = 1;
			}
			
			AnimationDuration newtime = elapsed;
			AnimationDuration oldtime = sd->elapsed;

			if(oldtime > sd->duration)
				oldtime = sd->duration;

			if(newtime > sd->duration)
				newtime = sd->duration;

			sd->elapsed = elapsed;
			
			float div_ti = 1.f / ti;

			Vec2s pos1 = Vec2s(g_size.center()) - symbolVecScale * short(2) + sd->cPosStart * symbolVecScale;

			Vec2s old_pos = pos1;

			for(size_t j = 0; j < nbcomponents; j++) {
				Vec2s vect = GetSymbVector(sd->sequence[j]);
				vect *= symbolVecScale;
				vect += vect / Vec2s(2);
				if(oldtime <= AnimationDurationMsf(ti)) {
					float ratio = toMsf(oldtime) * div_ti;
					old_pos += Vec2s(Vec2f(vect) * ratio);
					break;
				}
				old_pos += vect;
				oldtime -= AnimationDurationMsf(ti);
			}

			for(size_t j = 0; j < nbcomponents; j++) {
				Vec2s vect = GetSymbVector(sd->sequence[j]);
				vect *= symbolVecScale;
				vect += vect / Vec2s(2);

				if(newtime <= AnimationDurationMsf(ti)) {
					float ratio = toMsf(newtime) * div_ti;
					pos1 += Vec2s(Vec2f(vect) * ratio);
					AddFlare(Vec2f(pos1), 0.1f, 1, io);
					FlareLine(Vec2f(old_pos), Vec2f(pos1), io);
					break;
				}

				pos1 += vect;
				newtime -= AnimationDurationMsf(ti);
				
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

static void ARX_SPELLS_RequestSymbolDrawCommon(Entity * io, GameDuration duration,
                                               RuneInfo & info) {
	
	SYMBOL_DRAW * sd;
	if(io != entities.player()) {
		if(!io->symboldraw) {
			io->symboldraw = new SYMBOL_DRAW;
		}
		sd = io->symboldraw;
	} else {
		sd = &g_bookSymbolDraw;
	}
	
	sd->duration = toAnimationDuration(std::max(GameDurationMs(1), duration));
	sd->sequence = info.sequence;
	
	sd->starttime = g_gameTime.now();
	sd->elapsed = 0;
	
	sd->cPosStart = info.startOffset;
	
	io->gameFlags &= ~GFLAG_INVISIBILITY;
	
}

void ARX_SPELLS_RequestSymbolDraw(Entity * io, const std::string & name, GameDuration duration) {
	
	BOOST_FOREACH(RuneInfo & info, runeInfos) {
		if(info.name == name) {
			ARX_SPELLS_RequestSymbolDrawCommon(io, duration, info);
			break;
		}
	}
	
}

void ARX_SPELLS_RequestSymbolDraw2(Entity * io, Rune symb, GameDuration duration) {
	
	BOOST_FOREACH(RuneInfo & info, runeInfos) {
		if(info.rune == symb) {
			ARX_SPELLS_RequestSymbolDrawCommon(io, duration, info);
			break;
		}
	}
	
}

