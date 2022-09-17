/*
 * Copyright 2011-2022 Arx Libertatis Team (see the AUTHORS file)
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
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved

#ifndef ARX_GAME_SPELLS_H
#define ARX_GAME_SPELLS_H

#include <stddef.h>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "audio/AudioTypes.h"

#include "game/magic/Precast.h"
#include "game/magic/Rune.h"
#include "game/magic/RuneDraw.h"
#include "game/magic/SpellData.h"
#include "game/magic/SpellRecognition.h"

#include "math/Types.h"
#include "math/Angle.h"
#include "math/Random.h"
#include "math/Vector.h"

#include "scene/Light.h"

#include "util/Flags.h"
#include "util/Range.h"

class Entity;
class CSpellFx;
class TextureContainer;

class SpellManager {
	
public:
	
	void init();
	void clearAll();
	
	[[nodiscard]] Spell * operator[](SpellHandle handle) const noexcept;
	
	static void endSpell(Spell * spell) noexcept;
	
	void endByCaster(EntityHandle caster) noexcept;
	void endByCaster(EntityHandle caster, SpellType type) noexcept;
	
	void endByTarget(EntityHandle target, SpellType type) noexcept;
	void endByType(SpellType type) noexcept;
	
	[[nodiscard]] Spell * getSpellByCaster(EntityHandle caster, SpellType type) const noexcept;
	[[nodiscard]] Spell * getSpellOnTarget(EntityHandle target, SpellType type) const noexcept;
	[[nodiscard]] float getTotalSpellCasterLevelOnTarget(EntityHandle target, SpellType type) const noexcept;
	
	void replaceCaster(EntityHandle oldCaster, EntityHandle newCaster) noexcept;
	void removeTarget(Entity * io) noexcept;
	
	Spell & addSpell(std::unique_ptr<Spell> spell);
	void freeSlot(const Spell * spell);
	
	[[nodiscard]] auto begin() const noexcept { return util::entries(m_spells).begin(); }
	[[nodiscard]] auto end() const noexcept { return util::entries(m_spells).end(); }
	
	[[nodiscard]] auto byCaster(EntityHandle caster) const noexcept {
		return util::filter(*this, [caster](const Spell & spell) {
			return spell.m_caster == caster;
		});
	}
	
	[[nodiscard]] auto ofType(SpellType type) const noexcept {
		return util::filter(*this, [type](const Spell & spell) {
			return spell.m_type == type;
		});
	}
	
	Spell * getById(std::string_view idString) const noexcept;
	
private:
	
	std::vector<std::unique_ptr<Spell>> m_spells;
	size_t m_nextInstance;
	
};

extern SpellManager spells;

SpellType GetSpellId(std::string_view spell);
void TryToCastSpell(Entity * io, SpellType spellType, long level, EntityHandle target, SpellcastFlags flags, GameDuration duration);

bool ARX_SPELLS_Launch(SpellType typ, Entity & source, SpellcastFlags flags, long level,
                       Entity * target, GameDuration duration);
void ARX_SPELLS_Update();

void ARX_SPELLS_Fizzle(Spell * spell);

void ARX_SPELLS_ManageMagic();

void ARX_SPELLS_CancelSpellTarget();
void ARX_SPELLS_LaunchSpellTarget(Entity * io);
float ARX_SPELLS_GetManaCost(SpellType _lNumSpell, float casterLevel);
float ARX_SPELLS_ApplyFireProtection(Entity * io, float damages);
float ARX_SPELLS_ApplyColdProtection(Entity * io, float damages);

const char * getSpellName(SpellType num);

#endif // ARX_GAME_SPELLS_H
