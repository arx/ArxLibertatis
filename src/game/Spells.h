/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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
#include <string>

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

class Entity;
class CSpellFx;
class TextureContainer;

const size_t MAX_SPELLS = 20;

class SpellManager {
	
public:
	
	void init();
	void clearAll();
	
	SpellBase * operator[](SpellHandle handle);
	
	void endSpell(SpellBase * spell);
	
	void endByCaster(EntityHandle caster);
	void endByCaster(EntityHandle caster, SpellType type);
	
	void endByTarget(EntityHandle target, SpellType type);
	void endByType(SpellType type);
	
	bool ExistAnyInstanceForThisCaster(SpellType typ, EntityHandle caster);
	
	SpellBase * getSpellByCaster(EntityHandle caster, SpellType type);
	SpellBase * getSpellOnTarget(EntityHandle target, SpellType type);
	
	void replaceCaster(EntityHandle oldCaster, EntityHandle newCaster);
	void removeTarget(Entity * io);
	
	bool hasFreeSlot();
	void addSpell(SpellBase * spell);
	void freeSlot(SpellBase * spell);
	
	SpellHandle create();
	
private:
	
	SpellBase * m_spells[MAX_SPELLS];
	
};

extern SpellManager spells;

SpellType GetSpellId(const std::string & spell);
void TryToCastSpell(Entity * io, SpellType spellType, long level, EntityHandle target, SpellcastFlags flags, GameDuration duration);

bool ARX_SPELLS_Launch(SpellType typ, EntityHandle source, SpellcastFlags flags, long level, EntityHandle target, GameDuration duration);
void ARX_SPELLS_Update();

void ARX_SPELLS_Fizzle(SpellBase * spell);

void ARX_SPELLS_ManageMagic();

void ARX_SPELLS_CancelSpellTarget();
void ARX_SPELLS_LaunchSpellTarget(Entity * io);
float ARX_SPELLS_GetManaCost(SpellType _lNumSpell, float casterLevel);
float ARX_SPELLS_ApplyFireProtection(Entity * io, float damages);
float ARX_SPELLS_ApplyColdProtection(Entity * io, float damages);

#endif // ARX_GAME_SPELLS_H
