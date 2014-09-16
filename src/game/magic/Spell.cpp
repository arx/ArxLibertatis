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

#include "game/EntityManager.h"
#include "game/Player.h"
#include "game/magic/Spell.h"

#include "core/GameTime.h"

SpellBase::SpellBase()
	: m_level(1.f)
	, m_hand_group(-1)
	, m_type(SPELL_NONE)
	, m_timcreation(0)
	, m_hasDuration(false)
	, m_duration(0)
	, m_fManaCostPerSecond(0.f)
	, m_snd_loop(audio::INVALID_ID)
	, m_pSpellFx(NULL)
	, m_launchDuration(-1)
{
	
	m_targets.clear();
}

void SpellBase::BaseEnd() {
	
	// All Levels - Kill Light
	if(m_pSpellFx && lightHandleIsValid(m_pSpellFx->lLightId)) {
		EERIE_LIGHT * light = lightHandleGet(m_pSpellFx->lLightId);
		
		light->duration = 500; 
		light->time_creation = (unsigned long)(arxtime);
	}
	
	delete m_pSpellFx;
	m_pSpellFx = NULL;
}

Vec3f SpellBase::getTargetPos(EntityHandle source, EntityHandle target)
{
	Vec3f targetPos;
	if(target == EntityHandle::Invalid) {
		// no target... targeted by sight
		if(source == PlayerEntityHandle) {
			// no target... player spell targeted by sight
			targetPos.x = player.pos.x - std::sin(glm::radians(player.angle.getPitch()))*60.f;
			targetPos.y = player.pos.y + std::sin(glm::radians(player.angle.getYaw()))*60.f;
			targetPos.z = player.pos.z + std::cos(glm::radians(player.angle.getPitch()))*60.f;
		} else {
			// TODO entities[target] with target < 0 ??? - uh oh!
			targetPos.x = entities[target]->pos.x - std::sin(glm::radians(entities[target]->angle.getPitch()))*60.f;
			targetPos.y = entities[target]->pos.y - 120.f;
			targetPos.z = entities[target]->pos.z + std::cos(glm::radians(entities[target]->angle.getPitch()))*60.f;
		}
	} else if(target == PlayerEntityHandle) {
		// player target
		targetPos = player.pos;
	} else {
		// IO target
		targetPos = entities[target]->pos;
	}
	
	return targetPos;
}
