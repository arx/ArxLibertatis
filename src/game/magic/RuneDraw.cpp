/*
 * Copyright 2014-2022 Arx Libertatis Team (see the AUTHORS file)
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

static void ReCenterSequence(std::string_view sequence, Vec2s & iMin, Vec2s & iMax) {
	
	Vec2s iSize = Vec2s(0, 0);
	iMin = Vec2s(0, 0);
	iMax = Vec2s(0, 0);
	
	for(char symbol : sequence) {
		Vec2s es2dVector = GetSymbVector(symbol);
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
	
	for(const RuneInfo & info : runeInfos) {
		
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
		timePerComponent = 1ms;

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
			AddFlare(pos, glm::min(scale.x, scale.y) * 0.2f, false, entities.player(), true);

			break;
		}

		pos += vect;

		timeRemaining -= timePerComponent;
	}
}

void ARX_SPELLS_UpdateSymbolDraw() {
	
	GameInstant now = g_gameTime.now();
	
	for(Entity & entity : entities) {
		
		if(entity == *entities.player()) {
			continue;
		}
		
		IO_SPELLCAST_DATA & spellcast = entity.spellcast_data;
		if(spellcast.castingspell != SPELL_NONE && !entity.symboldraw) {
			
			bool tst = false;
			if(!(spellcast.spell_flags & SPELLCAST_FLAG_NOANIM) && (entity.ioflags & IO_NPC)) {
				AnimLayer & layer1 = entity.animlayer[1];
				if(layer1.cur_anim == entity.anims[ANIM_CAST_START]  && (layer1.flags & EA_ANIMEND)) {
					// TODO why no AcquireLastAnim() like everywhere else?
					FinishAnim(&entity, layer1.cur_anim);
					ANIM_Set(layer1, entity.anims[ANIM_CAST_CYCLE]);
					tst = true;
				} else if(layer1.cur_anim == entity.anims[ANIM_CAST_CYCLE]) {
					tst = true;
				} else if(layer1.cur_anim != entity.anims[ANIM_CAST_START]) {
					spellcast.castingspell = SPELL_NONE;
				}
			} else {
				tst = true;
			}
			
			if(tst) {
				if(spellcast.symb[0] != RUNE_NONE) {
					Rune symb = spellcast.symb[0];
					for(long j = 0; j < 3; j++) {
						spellcast.symb[j] = spellcast.symb[j + 1];
					}
					spellcast.symb[3] = RUNE_NONE;
					float speedFactor = std::max(entity.speed_modif + entity.basespeed, 0.01f);
					float duration = (1000 - (spellcast.spell_level * 60)) * speedFactor;
					ARX_SPELLS_RequestSymbolDraw2(&entity, symb, std::chrono::duration<float, std::milli>(duration));
					entity.gameFlags &= ~GFLAG_INVISIBILITY;
				} else {
					entity.gameFlags &= ~GFLAG_INVISIBILITY;
					ARX_SPELLS_Launch(spellcast.castingspell, entity, spellcast.spell_flags,
					                  spellcast.spell_level, entities.get(spellcast.target), spellcast.duration);
					if(!(spellcast.spell_flags & SPELLCAST_FLAG_NOANIM) && (entity.ioflags & IO_NPC)) {
						changeAnimation(&entity, 1, entity.anims[ANIM_CAST]);
					}
					spellcast.castingspell = SPELL_NONE;
				}
			}
			
		}
		
		updateIOLight(&entity);
		
		if(!entity.symboldraw) {
			continue;
		}
		
		AnimationDuration elapsed = toAnimationDuration(now - entity.symboldraw->starttime);
		if(elapsed > entity.symboldraw->duration) {
			endLightDelayed(entity.dynlight, 600ms);
			entity.dynlight = { };
			delete entity.symboldraw;
			entity.symboldraw = nullptr;
			continue;
		}
		
		const size_t nbcomponents = entity.symboldraw->sequence.length();
		if(nbcomponents == 0) {
			delete entity.symboldraw;
			entity.symboldraw = nullptr;
			continue;
		}
		
		SYMBOL_DRAW * sd = entity.symboldraw;
		AnimationDuration ti = sd->duration / nbcomponents;
		if(ti <= 0) {
			ti = 1ms;
		}
		
		AnimationDuration newtime = std::min(elapsed, sd->duration);
		AnimationDuration oldtime = std::min(sd->elapsed, sd->duration);
		sd->elapsed = elapsed;
		
		Vec2s pos1 = Vec2s(g_size.center()) - symbolVecScale * short(2) + sd->cPosStart * symbolVecScale;
		
		Vec2s old_pos = pos1;
		for(size_t j = 0; j < nbcomponents; j++) {
			Vec2s vect = GetSymbVector(sd->sequence[j]);
			vect *= symbolVecScale;
			vect += vect / Vec2s(2);
			if(oldtime <= ti) {
				float ratio = oldtime / ti;
				old_pos += Vec2s(Vec2f(vect) * ratio);
				break;
			}
			old_pos += vect;
			oldtime -= ti;
		}
		
		for(size_t j = 0; j < nbcomponents; j++) {
			Vec2s vect = GetSymbVector(sd->sequence[j]);
			vect *= symbolVecScale;
			vect += vect / Vec2s(2);
			if(newtime <= ti) {
				float ratio = newtime / ti;
				pos1 += Vec2s(Vec2f(vect) * ratio);
				AddFlare(Vec2f(pos1), 0.1f, false, &entity);
				FlareLine(Vec2f(old_pos), Vec2f(pos1), &entity);
				break;
			}
			pos1 += vect;
			newtime -= ti;
		}
		
	}
	
}

void ARX_SPELLS_ClearAllSymbolDraw() {
	for(Entity & entity : entities) {
		delete entity.symboldraw;
		entity.symboldraw = nullptr;
	}
}

static void ARX_SPELLS_RequestSymbolDrawCommon(Entity * io, GameDuration duration,
                                               const RuneInfo & info) {
	
	SYMBOL_DRAW * sd;
	if(io != entities.player()) {
		if(!io->symboldraw) {
			io->symboldraw = new SYMBOL_DRAW;
		}
		sd = io->symboldraw;
	} else {
		sd = &g_bookSymbolDraw;
	}
	
	sd->duration = toAnimationDuration(std::max(GameDuration(1ms), duration));
	sd->sequence = info.sequence;
	
	sd->starttime = g_gameTime.now();
	sd->elapsed = 0;
	
	sd->cPosStart = info.startOffset;
	
	io->gameFlags &= ~GFLAG_INVISIBILITY;
	
}

void ARX_SPELLS_RequestSymbolDraw(Entity * io, std::string_view name, GameDuration duration) {
	
	for(const RuneInfo & info : runeInfos) {
		if(info.name == name) {
			ARX_SPELLS_RequestSymbolDrawCommon(io, duration, info);
			break;
		}
	}
	
}

void ARX_SPELLS_RequestSymbolDraw2(Entity * io, Rune symb, GameDuration duration) {
	
	for(const RuneInfo & info : runeInfos) {
		if(info.rune == symb) {
			ARX_SPELLS_RequestSymbolDrawCommon(io, duration, info);
			break;
		}
	}
	
}

