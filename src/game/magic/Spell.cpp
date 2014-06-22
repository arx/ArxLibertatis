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

#include "game/magic/Spell.h"

#include "core/GameTime.h"

SpellBase::SpellBase() {
	m_longinfo_entity = InvalidEntityHandle;
	m_longinfo_damage = InvalidDamageHandle;
	m_longinfo_light = InvalidLightHandle;
	m_longinfo2_light = InvalidLightHandle;
	
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
