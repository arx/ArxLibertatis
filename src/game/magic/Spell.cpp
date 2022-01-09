/*
 * Copyright 2014-2021 Arx Libertatis Team (see the AUTHORS file)
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

#include "game/magic/Spell.h"

#include "game/EntityId.h"
#include "game/EntityManager.h"
#include "game/Player.h"
#include "game/Spells.h"

#include "core/GameTime.h"

Spell::Spell()
	: m_instance(0)
	, m_level(1.f)
	, m_hand_pos(0.f)
	, m_caster_pos(0.f)
	, m_type(SPELL_NONE)
	, m_timcreation(0)
	, m_hasDuration(false)
	, m_duration(0)
	, m_elapsed(0)
	, m_fManaCostPerSecond(0.f)
	, m_launchDuration(GameDuration::ofRaw(-1))
{
	
	m_targets.clear();
}

Vec3f Spell::getPosition() const {
	return getCasterPosition();
}

Vec3f Spell::getCasterPosition() const {
	if(Entity * caster = entities.get(m_caster)) {
		return caster->pos;
	} else {
		// should not happen
		return Vec3f(0.f);
	}
}

Vec3f Spell::getTargetPosition() const {
	if(Entity * target = entities.get(m_target)) {
		return target->pos;
	} else {
		// should not happen
		return Vec3f(0.f);
	}
}

void Spell::updateCasterHand() {
	
	// Create hand position if a hand is defined
	Entity * caster = entities.get(m_caster);
	if(caster == entities.player()) {
		m_hand_group = caster->obj->fastaccess.primary_attach;
	} else if(caster) {
		m_hand_group = caster->obj->fastaccess.left_attach;
	}
	
	if(m_hand_group != ActionPoint() && caster) {
		m_hand_pos = actionPointPosition(caster->obj, m_hand_group);
	}
	
}

void Spell::updateCasterPosition() {
	
	Entity * caster = entities.get(m_caster);
	if(caster == entities.player()) {
		m_caster_pos = player.pos;
	} else if(caster) {
		m_caster_pos = caster->pos;
	}
	
}

std::string_view Spell::className() const noexcept {
	const char * spellName = getSpellName(m_type);
	return spellName ? spellName : "spell";
}

std::string Spell::idString() const noexcept {
	return EntityId(className(), m_instance).string();
}

Vec3f Spell::getTargetPos(EntityHandle source, EntityHandle target) {
	
	if(target == EntityHandle_Player) {
		// player target
		return player.pos;
	} else if(target != EntityHandle()) {
		// IO target
		return entities[target]->pos;
	} else if(source == EntityHandle_Player) {
		// no target... player spell targeted by sight
		Vec3f targetPos = player.pos + angleToVectorXZ(player.angle.getYaw()) * 60.f;
		targetPos.y += std::sin(glm::radians(player.angle.getPitch())) * 60.f;
		return targetPos;
	} else {
		// no target... targeted by sight
		Vec3f targetPos = entities[source]->pos + angleToVectorXZ(entities[source]->angle.getYaw()) * 60.f;
		targetPos += Vec3f(0.f, -120.f, 0.f);
		return targetPos;
	}
	
}
