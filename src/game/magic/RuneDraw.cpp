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

struct SYMBOL_DRAW {
	unsigned long	starttime;
	Vec3f		lastpos;
	short			lasttim;
	short			duration;
	char			sequence[32];
	Vec2s cPosStart;
};

Vec2s GetSymbVector(char c) {

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

void ReCenterSequence(const char *_pcSequence, Vec2s & iMin, Vec2s & iMax) {
	
	Vec2s iSize = Vec2s(0, 0);
	iMin = Vec2s(0, 0);
	iMax = Vec2s(0, 0);
	
	int iLenght=strlen(_pcSequence);

	for(int iI = 0; iI < iLenght; iI++) {
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
	
	unsigned long curtime = (unsigned long)(arxtime);
	
	for(size_t i = 0; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		Entity * io = entities[handle];
		if(!io)
			continue;

		if(io->spellcast_data.castingspell != SPELL_NONE) {
			if(!io->symboldraw) {
				long tst = 0;

				if(!(io->spellcast_data.spell_flags & SPELLCAST_FLAG_NOANIM) && (io->ioflags & IO_NPC)) {
					ANIM_USE * ause1 = &io->animlayer[1];

					if(ause1->cur_anim == io->anims[ANIM_CAST_START]  && (ause1->flags & EA_ANIMEND)) {
						// TODO why no AcquireLastAnim() like everywhere else?
						FinishAnim(io, ause1->cur_anim);
						ANIM_Set(ause1, io->anims[ANIM_CAST_CYCLE]);
						tst = 1;
					} else if(ause1->cur_anim == io->anims[ANIM_CAST_CYCLE]) {
						tst = 1;
					} else if(ause1->cur_anim != io->anims[ANIM_CAST_START]) {
						io->spellcast_data.castingspell = SPELL_NONE;
					}
				} else {
					tst = 1;
				}

				if(io->spellcast_data.symb[0] != RUNE_NONE && tst) {
					Rune symb = io->spellcast_data.symb[0];

					for(long j = 0; j < 3; j++) {
						io->spellcast_data.symb[j] = io->spellcast_data.symb[j+1];
					}

					io->spellcast_data.symb[3] = RUNE_NONE;
					ARX_SPELLS_RequestSymbolDraw2(io, symb, (1000-(io->spellcast_data.spell_level*60))*std::max(io->speed_modif+io->basespeed,0.01f));
					io->gameFlags &= ~GFLAG_INVISIBILITY;
				} else if(tst) { // cast spell !!!
					io->gameFlags &= ~GFLAG_INVISIBILITY;
					ARX_SPELLS_Launch(io->spellcast_data.castingspell, EntityHandle(i), io->spellcast_data.spell_flags,io->spellcast_data.spell_level,io->spellcast_data.target,io->spellcast_data.duration);

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
				
				float rr = rnd();
				light->pos.x = io->pos.x - std::sin(glm::radians(MAKEANGLE(io->angle.getPitch() - 45.f)))*60.f;
				light->pos.y = io->pos.y - 120.f;
				light->pos.z = io->pos.z + std::cos(glm::radians(MAKEANGLE(io->angle.getPitch() - 45.f)))*60.f;
				light->fallstart=140.f+(float)io->flarecount*0.333333f+rr*5.f;
				light->fallend=220.f+(float)io->flarecount*0.5f+rr*5.f;
				light->intensity=1.6f;
				light->rgb.r=0.01f*io->flarecount*2;
				light->rgb.g=0.009f*io->flarecount*2;
				light->rgb.b=0.008f*io->flarecount*2;
			}
		} else if(lightHandleIsValid(io->dynlight)) {
			lightHandleGet(io->dynlight)->exist = 0;
			io->dynlight = LightHandle::Invalid;
		}

		if(io->symboldraw) {
			SYMBOL_DRAW * sd = entities[handle]->symboldraw;
			long tim = curtime - sd->starttime;

			if(tim > sd->duration) {
				if(lightHandleIsValid(io->dynlight)) {
					EERIE_LIGHT * light = lightHandleGet(io->dynlight);
					
					light->time_creation = (unsigned long)(arxtime);
					light->duration = 600;
					io->dynlight = LightHandle::Invalid;
				}

				free(io->symboldraw);
				io->symboldraw = NULL;
				continue;
			}

			long nbcomponents=strlen(sd->sequence);

			if(nbcomponents <= 0) {
				free(io->symboldraw);
				io->symboldraw = NULL;
				continue;
			}

			float ti = ((float)sd->duration/(float)nbcomponents);

			if(ti <= 0)
				ti = 1;

			Vec2s pos1, old_pos;
			long newtime=tim;
			long oldtime=sd->lasttim;

			if(oldtime>sd->duration)
				oldtime=sd->duration;

			if(newtime>sd->duration)
				newtime=sd->duration;

			sd->lasttim=(short)tim;

			pos1.x = (short)subj.center.x - symbolVecScale.x * 2 + sd->cPosStart.x * symbolVecScale.x;
			pos1.y = (short)subj.center.y - symbolVecScale.y * 2 + sd->cPosStart.y * symbolVecScale.y;

			float div_ti=1.f/ti;

			if(io != entities.player()) {
				old_pos = pos1;

				for(long j = 0; j < nbcomponents; j++) {
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

				for(int j = 0; j < nbcomponents; j++) {
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
				pos1 = Vec2s(97, 64);
				
				Vec2s lPos;
				lPos.x = (((513>>1)-lMaxSymbolDrawSize.x)>>1);
				lPos.y = (313-(((313*3/4)-lMaxSymbolDrawSize.y)>>1));

				pos1 += lPos;
				pos1 += (lMaxSymbolDrawSize - iSize) / Vec2s(2);
				pos1 -= iMin;

				for(long j = 0; j < nbcomponents; j++) {

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
			free(e->symboldraw);
			e->symboldraw = NULL;
		}
	}
}






void ARX_SPELLS_RequestSymbolDrawCommon(Entity *io, float duration, RuneInfo & info) {
	SYMBOL_DRAW * ptr;
	ptr = (SYMBOL_DRAW *)realloc(io->symboldraw, sizeof(SYMBOL_DRAW));

	if(!ptr)
		return;

	io->symboldraw = ptr;

	SYMBOL_DRAW *sd = io->symboldraw;

	sd->duration = (short)std::max(1l, long(duration));
	strcpy(sd->sequence, info.sequence.c_str());

	sd->starttime = (unsigned long)(arxtime);
	sd->lasttim = 0;
	sd->lastpos.x = io->pos.x - std::sin(glm::radians(MAKEANGLE(io->angle.getPitch() - 45.0F + info.startOffset.x*2))) * 60.0F;
	sd->lastpos.y = io->pos.y - 120.0F - info.startOffset.y*5;
	sd->lastpos.z = io->pos.z + std::cos(glm::radians(MAKEANGLE(io->angle.getPitch() - 45.0F + info.startOffset.x * 2))) * 60.0F;

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

