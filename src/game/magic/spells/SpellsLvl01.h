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

#ifndef ARX_GAME_MAGIC_SPELLS_SPELLSLVL01_H
#define ARX_GAME_MAGIC_SPELLS_SPELLSLVL01_H

#include "game/magic/Spell.h"

#include "graphics/effects/MagicMissile.h"

class MagicSightSpell : public SpellBase {
public:
	bool CanLaunch();
	void Launch();
	void End();
	void Update();
};

class MagicMissileSpell : public SpellBase {
public:
	MagicMissileSpell();
	~MagicMissileSpell();
	
	void Launch();
	void End();
	void Update();
	
private:
	bool m_mrCheat;
	std::vector<CMagicMissile *> pTab;
};

class IgnitSpell : public SpellBase {
public:
	IgnitSpell();
	
	void Launch();
	void End();
	void Update();
	
private:
	Vec3f m_srcPos;
	ArxDuration m_elapsed;
	
	struct T_LINKLIGHTTOFX {
		LightHandle m_effectLight;
		int m_targetLight;
	};
	std::vector<T_LINKLIGHTTOFX> m_lights;
};

class DouseSpell : public SpellBase {
public:
	void Launch();
	void End();
	void Update();
	
private:
	std::vector<size_t> m_lights;
};

class ActivatePortalSpell : public SpellBase {
public:
	void Launch();
};

#endif // ARX_GAME_MAGIC_SPELLS_SPELLSLVL01_H
