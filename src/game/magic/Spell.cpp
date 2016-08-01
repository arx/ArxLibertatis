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
#include "scene/Interactive.h"

#include "core/GameTime.h"

SpellBase::SpellBase()
	: m_level(1.f)
	, m_hand_group()
	, m_type(SPELL_NONE)
	, m_timcreation(0)
	, m_hasDuration(false)
	, m_duration(0)
	, m_fManaCostPerSecond(0.f)
	, m_snd_loop(audio::INVALID_ID)
	, m_launchDuration(-1)
{
	
	m_targets.clear();
}

Vec3f SpellBase::getPosition() {
	return getCasterPosition();
}

Vec3f SpellBase::getCasterPosition() {
	if(ValidIONum(m_caster)) {
		return entities[m_caster]->pos;
	} else {
		// should not happen
		return Vec3f_ZERO;
	}
}

Vec3f SpellBase::getTargetPosition() {
	if(ValidIONum(m_target)) {
		return entities[m_target]->pos;
	} else {
		// should not happen
		return Vec3f_ZERO;
	}
}

void SpellBase::updateCasterHand() {
	
	// Create hand position if a hand is defined
	if(m_caster == EntityHandle_Player) {
		m_hand_group = entities[m_caster]->obj->fastaccess.primary_attach;
	} else {
		m_hand_group = entities[m_caster]->obj->fastaccess.left_attach;
	}
	
	if(m_hand_group != ActionPoint()) {
		m_hand_pos = actionPointPosition(entities[m_caster]->obj, m_hand_group);
	}
}

void SpellBase::updateCasterPosition() {
	
	if(m_caster == EntityHandle_Player) {
		m_caster_pos = player.pos;
	} else {
		m_caster_pos = entities[m_caster]->pos;
	}
}

Vec3f SpellBase::getTargetPos(EntityHandle source, EntityHandle target)
{
	Vec3f targetPos;
	if(target == EntityHandle()) {
		// no target... targeted by sight
		if(source == EntityHandle_Player) {
			// no target... player spell targeted by sight
			targetPos = player.pos;
			targetPos += angleToVectorXZ(player.angle.getYaw()) * 60.f;
			targetPos.y += std::sin(glm::radians(player.angle.getPitch())) * 60.f;
		} else {
			// TODO entities[target] with target < 0 ??? - uh oh!
			targetPos = entities[target]->pos;
			targetPos += angleToVectorXZ(entities[target]->angle.getYaw()) * 60.f;
			targetPos += Vec3f(0.f, -120.f, 0.f);
		}
	} else if(target == EntityHandle_Player) {
		// player target
		targetPos = player.pos;
	} else {
		// IO target
		targetPos = entities[target]->pos;
	}
	
	return targetPos;
}
