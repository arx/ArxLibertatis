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

#include "game/magic/SpellData.h"

#include "core/Localisation.h"

SPELL_ICON spellicons[SPELL_TYPES_COUNT];

void spellDataInit() {
	
	for(size_t i = 0; i < SPELL_TYPES_COUNT; i++) {
		spellicons[i] = SPELL_ICON();
	}
	
	{ // Magic_Sight Level 1
		SPELL_ICON & s = spellicons[SPELL_MAGIC_SIGHT];
		s.name = getLocalised("system_spell_name_magic_sight");
		s.description = getLocalised("system_spell_description_magic_sight");
		s.level = 1;
		s.spellid = SPELL_MAGIC_SIGHT;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_magic_sight");
		s.symbols[0] = RUNE_MEGA;
		s.symbols[1] = RUNE_VISTA;
	}
	
	{ // Magic_Missile Level 1
		SPELL_ICON & s = spellicons[SPELL_MAGIC_MISSILE];
		s.name = getLocalised("system_spell_name_magic_projectile");
		s.description = getLocalised("system_spell_description_magic_projectile");
		s.level = 1;
		s.spellid = SPELL_MAGIC_MISSILE;
		s.m_hasDuration = false;
		s.bAudibleAtStart = true;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_magic_missile");
		s.symbols[0] = RUNE_AAM;
		s.symbols[1] = RUNE_TAAR;
	}
	
	{ // Ignit Level 1
		SPELL_ICON & s = spellicons[SPELL_IGNIT];
		s.name = getLocalised("system_spell_name_ignit");
		s.description = getLocalised("system_spell_description_ignit");
		s.level = 1;
		s.spellid = SPELL_IGNIT;
		s.m_hasDuration = false;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_ignite");
		s.symbols[0] = RUNE_AAM;
		s.symbols[1] = RUNE_YOK;
	}
	
	{ // Douse Level 1
		SPELL_ICON & s = spellicons[SPELL_DOUSE];
		s.name = getLocalised("system_spell_name_douse");
		s.description = getLocalised("system_spell_description_douse");
		s.level = 1;
		s.spellid = SPELL_DOUSE;
		s.m_hasDuration = false;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_douse");
		s.symbols[0] = RUNE_NHI;
		s.symbols[1] = RUNE_YOK;
	}
	
	{ // Activate_Portal Level 1
		SPELL_ICON & s = spellicons[SPELL_ACTIVATE_PORTAL];
		s.name = getLocalised("system_spell_name_activate_portal");
		s.description = getLocalised("system_spell_description_activate_portal");
		s.level = 1;
		s.spellid = SPELL_ACTIVATE_PORTAL;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_activate_portal");
		s.symbols[0] = RUNE_MEGA;
		s.symbols[1] = RUNE_SPACIUM;
		s.bSecret = true;
	}
	
	{ // Heal Level 2
		SPELL_ICON & s = spellicons[SPELL_HEAL];
		s.name = getLocalised("system_spell_name_heal");
		s.description = getLocalised("system_spell_description_heal");
		s.level = 2;
		s.spellid = SPELL_HEAL;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_heal");
		s.symbols[0] = RUNE_MEGA;
		s.symbols[1] = RUNE_VITAE;
	}
	
	{ // Detect_trap Level 2
		SPELL_ICON & s = spellicons[SPELL_DETECT_TRAP];
		s.name = getLocalised("system_spell_name_detect_trap");
		s.description = getLocalised("system_spell_description_detect_trap");
		s.level = 2;
		s.spellid = SPELL_DETECT_TRAP;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_detect_trap");
		s.symbols[0] = RUNE_MORTE;
		s.symbols[1] = RUNE_COSUM;
		s.symbols[2] = RUNE_VISTA;
	}
	
	{ // Armor Level 2
		SPELL_ICON & s = spellicons[SPELL_ARMOR];
		s.name = getLocalised("system_spell_name_armor");
		s.description = getLocalised("system_spell_description_armor");
		s.level = 2;
		s.spellid = SPELL_ARMOR;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_armor");
		s.symbols[0] = RUNE_MEGA;
		s.symbols[1] = RUNE_KAOM;
	}
	
	{ // Lower Armor Level 2
		SPELL_ICON & s = spellicons[SPELL_LOWER_ARMOR];
		s.name = getLocalised("system_spell_name_lower_armor");
		s.description = getLocalised("system_spell_description_lower_armor");
		s.level = 2;
		s.spellid = SPELL_LOWER_ARMOR;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_lower_armor");
		s.symbols[0] = RUNE_RHAA;
		s.symbols[1] = RUNE_KAOM;
	}
	
	{ // Harm Level 2
		SPELL_ICON & s = spellicons[SPELL_HARM];
		s.name = getLocalised("system_spell_name_harm");
		s.description = getLocalised("system_spell_description_harm");
		s.level = 2;
		s.spellid = SPELL_HARM;
		s.bAudibleAtStart = true;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_harm");
		s.symbols[0] = RUNE_RHAA;
		s.symbols[1] = RUNE_VITAE;
		s.bSecret = true;
	}
	
	{ // Speed Level 3
		SPELL_ICON & s = spellicons[SPELL_SPEED];
		s.name = getLocalised("system_spell_name_speed");
		s.description = getLocalised("system_spell_description_speed");
		s.level = 3;
		s.spellid = SPELL_SPEED;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_speed");
		s.symbols[0] = RUNE_MEGA;
		s.symbols[1] = RUNE_MOVIS;
	}
	
	{ // Reveal Level 3
		SPELL_ICON & s = spellicons[SPELL_DISPELL_ILLUSION];
		s.name = getLocalised("system_spell_name_reveal");
		s.description = getLocalised("system_spell_description_reveal");
		s.level = 3;
		s.spellid = SPELL_DISPELL_ILLUSION;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_reveal");
		s.symbols[0] = RUNE_NHI;
		s.symbols[1] = RUNE_STREGUM;
		s.symbols[2] = RUNE_VISTA;
	}
	
	{ // Fireball Level 3
		SPELL_ICON & s = spellicons[SPELL_FIREBALL];
		s.name = getLocalised("system_spell_name_fireball");
		s.description = getLocalised("system_spell_description_fireball");
		s.level = 3;
		s.spellid = SPELL_FIREBALL;
		s.m_hasDuration = false;
		s.bAudibleAtStart = true;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_fireball");
		s.symbols[0] = RUNE_AAM;
		s.symbols[1] = RUNE_YOK;
		s.symbols[2] = RUNE_TAAR;
	}
	
	{ // Create Food Level 3
		SPELL_ICON & s = spellicons[SPELL_CREATE_FOOD];
		s.name = getLocalised("system_spell_name_create_food");
		s.description = getLocalised("system_spell_description_create_food");
		s.level = 3;
		s.spellid = SPELL_CREATE_FOOD;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_create_food");
		s.symbols[0] = RUNE_AAM;
		s.symbols[1] = RUNE_VITAE;
		s.symbols[2] = RUNE_COSUM;
	}
	
	{ // Ice Projectile Level 3
		SPELL_ICON & s = spellicons[SPELL_ICE_PROJECTILE];
		s.name = getLocalised("system_spell_name_ice_projectile");
		s.description = getLocalised("system_spell_description_ice_projectile");
		s.level = 3;
		s.spellid = SPELL_ICE_PROJECTILE;
		s.m_hasDuration = false;
		s.bAudibleAtStart = true;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_iceball");
		s.symbols[0] = RUNE_AAM;
		s.symbols[1] = RUNE_FRIDD;
		s.symbols[2] = RUNE_TAAR;
		s.bSecret = true;
	}
	
	{ // Bless Level 4
		SPELL_ICON & s = spellicons[SPELL_BLESS];
		s.name = getLocalised("system_spell_name_sanctify");
		s.description = getLocalised("system_spell_description_sanctify");
		s.level = 4;
		s.spellid = SPELL_BLESS;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_bless");
		s.symbols[0] = RUNE_MEGA;
		s.symbols[1] = RUNE_STREGUM;
		s.symbols[2] = RUNE_VITAE;
	}
	
	{ // Dispel_Field Level 4
		SPELL_ICON & s = spellicons[SPELL_DISPELL_FIELD];
		s.name = getLocalised("system_spell_name_dispell_field");
		s.description = getLocalised("system_spell_description_dispell_field");
		s.level = 4;
		s.spellid = SPELL_DISPELL_FIELD;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_dispell_field");
		s.symbols[0] = RUNE_NHI;
		s.symbols[1] = RUNE_SPACIUM;
	}
	
	{ // Cold Protection Level 4
		SPELL_ICON & s = spellicons[SPELL_COLD_PROTECTION];
		s.name = getLocalised("system_spell_name_cold_protection");
		s.description = getLocalised("system_spell_description_cold_protection");
		s.level = 4;
		s.spellid = SPELL_COLD_PROTECTION;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_protection_cold");
		s.symbols[0] = RUNE_FRIDD;
		s.symbols[1] = RUNE_KAOM;
		s.bSecret = true;
	}
	
	{ // Fire Protection Level 4
		SPELL_ICON & s = spellicons[SPELL_FIRE_PROTECTION];
		s.name = getLocalised("system_spell_name_fire_protection");
		s.description = getLocalised("system_spell_description_fire_protection");
		s.level = 4;
		s.spellid = SPELL_FIRE_PROTECTION;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_protection_fire");
		s.symbols[0] = RUNE_YOK;
		s.symbols[1] = RUNE_KAOM;
	}
	
	{ // Telekinesis Level 4
		SPELL_ICON & s = spellicons[SPELL_TELEKINESIS];
		s.name = getLocalised("system_spell_name_telekinesis");
		s.description = getLocalised("system_spell_description_telekinesis");
		s.level = 4;
		s.spellid = SPELL_TELEKINESIS;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_telekinesis");
		s.symbols[0] = RUNE_SPACIUM;
		s.symbols[1] = RUNE_COMUNICATUM;
	}
	
	{ // Curse Level 4
		SPELL_ICON & s = spellicons[SPELL_CURSE];
		s.name = getLocalised("system_spell_name_curse");
		s.description = getLocalised("system_spell_description_curse");
		s.level = 4;
		s.spellid = SPELL_CURSE;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_curse");
		s.symbols[0] = RUNE_RHAA;
		s.symbols[1] = RUNE_STREGUM;
		s.symbols[2] = RUNE_VITAE;
		s.bSecret = true;
	}
	
	{ // Rune of Guarding Level 5
		SPELL_ICON & s = spellicons[SPELL_RUNE_OF_GUARDING];
		s.name = getLocalised("system_spell_name_rune_guarding");
		s.description = getLocalised("system_spell_description_rune_guarding");
		s.level = 5;
		s.spellid = SPELL_RUNE_OF_GUARDING;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_rune_guarding");
		s.symbols[0] = RUNE_AAM;
		s.symbols[1] = RUNE_MORTE;
		s.symbols[2] = RUNE_COSUM;
	}
	
	{ // Levitate Level 5
		SPELL_ICON & s = spellicons[SPELL_LEVITATE];
		s.name = getLocalised("system_spell_name_levitate");
		s.description = getLocalised("system_spell_description_levitate");
		s.level = 5;
		s.spellid = SPELL_LEVITATE;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_levitate");
		s.symbols[0] = RUNE_MEGA;
		s.symbols[1] = RUNE_SPACIUM;
		s.symbols[2] = RUNE_MOVIS;
	}
	
	{ // Cure Poison Level 5
		SPELL_ICON & s = spellicons[SPELL_CURE_POISON];
		s.name = getLocalised("system_spell_name_cure_poison");
		s.description = getLocalised("system_spell_description_cure_poison");
		s.level = 5;
		s.spellid = SPELL_CURE_POISON;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_cure_poison");
		s.symbols[0] = RUNE_NHI;
		s.symbols[1] = RUNE_CETRIUS;
	}
	
	{ // Repel Undead Level 5
		SPELL_ICON & s = spellicons[SPELL_REPEL_UNDEAD];
		s.name = getLocalised("system_spell_name_repel_undead");
		s.description = getLocalised("system_spell_description_repel_undead");
		s.level = 5;
		s.spellid = SPELL_REPEL_UNDEAD;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_repel_undead");
		s.symbols[0] = RUNE_MORTE;
		s.symbols[1] = RUNE_KAOM;
	}
	
	{ // Poison Projection Level 5
		SPELL_ICON & s = spellicons[SPELL_POISON_PROJECTILE];
		s.name = getLocalised("system_spell_name_poison_projection");
		s.description = getLocalised("system_spell_description_poison_projection");
		s.level = 5;
		s.spellid = SPELL_POISON_PROJECTILE;
		s.m_hasDuration = false;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_poison_projection");
		s.symbols[0] = RUNE_AAM;
		s.symbols[1] = RUNE_CETRIUS;
		s.symbols[2] = RUNE_TAAR;
		s.bSecret = true;
	}
	
	{ // Raise Dead Level 6
		SPELL_ICON & s = spellicons[SPELL_RISE_DEAD];
		s.name = getLocalised("system_spell_name_raise_dead");
		s.description = getLocalised("system_spell_description_raise_dead");
		s.level = 6;
		s.spellid = SPELL_RISE_DEAD;
		s.bAudibleAtStart = true;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_raise_dead");
		s.symbols[0] = RUNE_AAM;
		s.symbols[1] = RUNE_MORTE;
		s.symbols[2] = RUNE_VITAE;
	}
	
	{ // Paralyse Dead Level 6
		SPELL_ICON & s = spellicons[SPELL_PARALYSE];
		s.name = getLocalised("system_spell_name_paralyse");
		s.description = getLocalised("system_spell_description_paralyse");
		s.level = 6;
		s.spellid = SPELL_PARALYSE;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_paralyse");
		s.symbols[0] = RUNE_NHI;
		s.symbols[1] = RUNE_MOVIS;
	}
	
	{ // Create Field Dead Level 6
		SPELL_ICON & s = spellicons[SPELL_CREATE_FIELD];
		s.name = getLocalised("system_spell_name_create_field");
		s.description = getLocalised("system_spell_description_create_field");
		s.level = 6;
		s.spellid = SPELL_CREATE_FIELD;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_create_field");
		s.symbols[0] = RUNE_AAM;
		s.symbols[1] = RUNE_KAOM;
		s.symbols[2] = RUNE_SPACIUM;
	}
	
	{ // Disarm Trap Level 6
		SPELL_ICON & s = spellicons[SPELL_DISARM_TRAP];
		s.name = getLocalised("system_spell_name_disarm_trap");
		s.description = getLocalised("system_spell_description_disarm_trap");
		s.level = 6;
		s.spellid = SPELL_DISARM_TRAP;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_disarm_trap");
		s.symbols[0] = RUNE_NHI;
		s.symbols[1] = RUNE_MORTE;
		s.symbols[2] = RUNE_COSUM;
	}
	
	{ // Slow_Down Level 6 // SECRET SPELL
		SPELL_ICON & s = spellicons[SPELL_SLOW_DOWN];
		s.name = getLocalised("system_spell_name_slowdown");
		s.description = getLocalised("system_spell_description_slowdown");
		s.level = 6;
		s.spellid = SPELL_SLOW_DOWN;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_slow_down");
		s.symbols[0] = RUNE_RHAA;
		s.symbols[1] = RUNE_MOVIS;
		s.bSecret = true;
	}
	
	{ // Flying Eye Level 7
		SPELL_ICON & s = spellicons[SPELL_FLYING_EYE];
		s.name = getLocalised("system_spell_name_flying_eye");
		s.description = getLocalised("system_spell_description_flying_eye");
		s.level = 7;
		s.spellid = SPELL_FLYING_EYE;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_flying_eye");
		s.symbols[0] = RUNE_VISTA;
		s.symbols[1] = RUNE_MOVIS;
	}
	
	{ // Fire Field Eye Level 7
		SPELL_ICON & s = spellicons[SPELL_FIRE_FIELD];
		s.name = getLocalised("system_spell_name_fire_field");
		s.description = getLocalised("system_spell_description_fire_field");
		s.level = 7;
		s.spellid = SPELL_FIRE_FIELD;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_create_fire_field");
		s.symbols[0] = RUNE_AAM;
		s.symbols[1] = RUNE_YOK;
		s.symbols[2] = RUNE_SPACIUM;
	}
	
	{ // Ice Field Level 7
		SPELL_ICON & s = spellicons[SPELL_ICE_FIELD];
		s.name = getLocalised("system_spell_name_ice_field");
		s.description = getLocalised("system_spell_description_ice_field");
		s.level = 7;
		s.spellid = SPELL_ICE_FIELD;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_create_cold_field");
		s.symbols[0] = RUNE_AAM;
		s.symbols[1] = RUNE_FRIDD;
		s.symbols[2] = RUNE_SPACIUM;
		s.bSecret = true;
	}
	
	{ // Lightning Strike Level 7
		SPELL_ICON & s = spellicons[SPELL_LIGHTNING_STRIKE];
		s.name = getLocalised("system_spell_name_lightning_strike");
		s.description = getLocalised("system_spell_description_lightning_strike");
		s.level = 7;
		s.spellid = SPELL_LIGHTNING_STRIKE;
		s.m_hasDuration = false;
		s.bAudibleAtStart = true;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_lightning_strike");
		s.symbols[0] = RUNE_AAM;
		s.symbols[1] = RUNE_FOLGORA;
		s.symbols[2] = RUNE_TAAR;
	}
	
	{ // Confusion Level 7
		SPELL_ICON & s = spellicons[SPELL_CONFUSE];
		s.name = getLocalised("system_spell_name_confuse");
		s.description = getLocalised("system_spell_description_confuse");
		s.level = 7;
		s.spellid = SPELL_CONFUSE;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_confuse");
		s.symbols[0] = RUNE_RHAA;
		s.symbols[1] = RUNE_VISTA;
	}
	
	{ // Invisibility Level 8
		SPELL_ICON & s = spellicons[SPELL_INVISIBILITY];
		s.name = getLocalised("system_spell_name_invisibility");
		s.description = getLocalised("system_spell_description_invisibility");
		s.level = 8;
		s.spellid = SPELL_INVISIBILITY;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_invisibility");
		s.symbols[0] = RUNE_NHI;
		s.symbols[1] = RUNE_VISTA;
	}
	
	{ // Mana Drain Level 8
		SPELL_ICON & s = spellicons[SPELL_MANA_DRAIN];
		s.name = getLocalised("system_spell_name_mana_drain");
		s.description = getLocalised("system_spell_description_mana_drain");
		s.level = 8;
		s.spellid = SPELL_MANA_DRAIN;
		s.bAudibleAtStart = true;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_drain_mana");
		s.symbols[0] = RUNE_STREGUM;
		s.symbols[1] = RUNE_MOVIS;
	}
	
	{ // Explosion Level 8
		SPELL_ICON & s = spellicons[SPELL_EXPLOSION];
		s.name = getLocalised("system_spell_name_explosion");
		s.description = getLocalised("system_spell_description_explosion");
		s.level = 8;
		s.spellid = SPELL_EXPLOSION;
		s.m_hasDuration = false;
		s.bAudibleAtStart = true;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_explosion");
		s.symbols[0] = RUNE_AAM;
		s.symbols[1] = RUNE_MEGA;
		s.symbols[2] = RUNE_MORTE;
	}
	
	{ // Enchant Weapon Level 8
		SPELL_ICON & s = spellicons[SPELL_ENCHANT_WEAPON];
		s.name = getLocalised("system_spell_name_enchant_weapon");
		s.description = getLocalised("system_spell_description_enchant_weapon");
		s.level = 8;
		s.spellid = SPELL_ENCHANT_WEAPON;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_enchant_weapon");
		s.symbols[0] = RUNE_MEGA;
		s.symbols[1] = RUNE_STREGUM;
		s.symbols[2] = RUNE_COSUM;
	}
	
	{ // Life Drain Level 8 // SECRET SPELL
		SPELL_ICON & s = spellicons[SPELL_LIFE_DRAIN];
		s.name = getLocalised("system_spell_name_life_drain");
		s.description = getLocalised("system_spell_description_life_drain");
		s.level = 8;
		s.spellid = SPELL_LIFE_DRAIN;
		s.bAudibleAtStart = true;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_drain_life");
		s.symbols[0] = RUNE_VITAE;
		s.symbols[1] = RUNE_MOVIS;
		s.bSecret = true;
	}
	
	{ // Summon Creature Level 9
		SPELL_ICON & s = spellicons[SPELL_SUMMON_CREATURE];
		s.name = getLocalised("system_spell_name_summon_creature");
		s.description = getLocalised("system_spell_description_summon_creature");
		s.level = 9;
		s.spellid = SPELL_SUMMON_CREATURE;
		s.bAudibleAtStart = true;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_summon_creature");
		s.symbols[0] = RUNE_AAM;
		s.symbols[1] = RUNE_VITAE;
		s.symbols[2] = RUNE_TERA;
	}
	
	{ // FAKE Summon Creature Level 9
		SPELL_ICON & s = spellicons[SPELL_FAKE_SUMMON];
		s.name = getLocalised("system_spell_name_summon_creature");
		s.description = getLocalised("system_spell_description_summon_creature");
		s.level = 9;
		s.spellid = SPELL_FAKE_SUMMON;
		s.bAudibleAtStart = true;
		s.bSecret = true;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_summon_creature");
		s.symbols[0] = RUNE_AAM;
		s.symbols[1] = RUNE_VITAE;
		s.symbols[2] = RUNE_TERA;
	}
	
	{ // Negate Magic Level 9
		SPELL_ICON & s = spellicons[SPELL_NEGATE_MAGIC];
		s.name = getLocalised("system_spell_name_negate_magic");
		s.description = getLocalised("system_spell_description_negate_magic");
		s.level = 9;
		s.spellid = SPELL_NEGATE_MAGIC;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_negate_magic");
		s.symbols[0] = RUNE_NHI;
		s.symbols[1] = RUNE_STREGUM;
		s.symbols[2] = RUNE_SPACIUM;
	}
	
	{ // Incinerate Level 9
		SPELL_ICON & s = spellicons[SPELL_INCINERATE];
		s.name = getLocalised("system_spell_name_incinerate");
		s.description = getLocalised("system_spell_description_incinerate");
		s.level = 9;
		s.spellid = SPELL_INCINERATE;
		s.m_hasDuration = false;
		s.bAudibleAtStart = true;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_incinerate");
		s.symbols[0] = RUNE_AAM;
		s.symbols[1] = RUNE_MEGA;
		s.symbols[2] = RUNE_YOK;
	}
	
	{ // Mass paralyse Creature Level 9
		SPELL_ICON & s = spellicons[SPELL_MASS_PARALYSE];
		s.name = getLocalised("system_spell_name_mass_paralyse");
		s.description = getLocalised("system_spell_description_mass_paralyse");
		s.level = 9;
		s.spellid = SPELL_MASS_PARALYSE;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_mass_paralyse");
		s.symbols[0] = RUNE_MEGA;
		s.symbols[1] = RUNE_NHI;
		s.symbols[2] = RUNE_MOVIS;
	}
	
	{ // Mass Lightning Strike Level 10
		SPELL_ICON & s = spellicons[SPELL_MASS_LIGHTNING_STRIKE];
		s.name = getLocalised("system_spell_name_mass_lightning_strike");
		s.description = getLocalised("system_spell_description_mass_lightning_strike");
		s.level = 10;
		s.spellid = SPELL_MASS_LIGHTNING_STRIKE;
		s.m_hasDuration = false;
		s.bAudibleAtStart = true;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_mass_lighting_strike");
		s.symbols[0] = RUNE_AAM;
		s.symbols[1] = RUNE_FOLGORA;
		s.symbols[2] = RUNE_SPACIUM;
	}
	
	{ // Control Target Level 10
		SPELL_ICON & s = spellicons[SPELL_CONTROL_TARGET];
		s.name = getLocalised("system_spell_name_control_target");
		s.description = getLocalised("system_spell_description_control_target");
		s.level = 10;
		s.spellid = SPELL_CONTROL_TARGET;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_control_target");
		s.symbols[0] = RUNE_MOVIS;
		s.symbols[1] = RUNE_COMUNICATUM;
	}
	
	{ // Freeze time Level 10
		SPELL_ICON & s = spellicons[SPELL_FREEZE_TIME];
		s.name = getLocalised("system_spell_name_freeze_time");
		s.description = getLocalised("system_spell_description_freeze_time");
		s.level = 10;
		s.spellid = SPELL_FREEZE_TIME;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_freeze_time");
		s.symbols[0] = RUNE_RHAA;
		s.symbols[1] = RUNE_TEMPUS;
	}
	
	{ // Mass incinerate Level 10
		SPELL_ICON & s = spellicons[SPELL_MASS_INCINERATE];
		s.name = getLocalised("system_spell_name_mass_incinerate");
		s.description = getLocalised("system_spell_description_mass_incinerate");
		s.level = 10;
		s.spellid = SPELL_MASS_INCINERATE;
		s.m_hasDuration = false;
		s.bAudibleAtStart = true;
		s.tc = TextureContainer::LoadUI("graph/interface/icons/spell_mass_incinerate");
		s.symbols[0] = RUNE_MEGA;
		s.symbols[1] = RUNE_AAM;
		s.symbols[2] = RUNE_MEGA;
		s.symbols[3] = RUNE_YOK;
	}
	
}

void spellDataRelease() {
	for(size_t i = 0; i < SPELL_TYPES_COUNT; i++) {
		if(!spellicons[i].name.empty()) {
			spellicons[i].name.clear();
		}
		if(!spellicons[i].description.empty()) {
			spellicons[i].description.clear();
		}
	}
}
