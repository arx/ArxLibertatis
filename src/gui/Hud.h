/*
 * Copyright 2014-2019 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GUI_HUD_H
#define ARX_GUI_HUD_H

#include <vector>

#include "core/TimeTypes.h"
#include "game/GameTypes.h"
#include "gui/hud/HudCommon.h"
#include "math/Types.h"

/*!
 * \brief the hit strength diamond shown at the bottom of the UI.
 */
class HitStrengthGauge : public HudItem {
	
	TextureContainer * m_emptyTex;
	TextureContainer * m_fullTex;
	TextureContainer * m_hitTex;
	Vec2f m_size;
	Vec2f m_hitSize;
	Rectf m_hitRect;
	float m_intensity;
	bool m_flashActive;
	PlatformDuration m_flashTime;
	float m_flashIntensity;
	
public:
	HitStrengthGauge();
	
	void init();
	void updateRect(const Rectf & parent);
	void update();
	void draw();
	void requestFlash(float flashIntensity);
};

class BookIconGui : public HudIconBase {
	
	Vec2f m_size;
	PlatformDuration ulBookHaloTime;
	void MakeBookFX();
	
public:
	BookIconGui();
	
	void init();
	void update(const Rectf & parent);
	void updateInput();
	void requestHalo();
	void requestFX();
};

class BackpackIconGui : public HudIconBase {
	
	PlatformInstant flDelay;
	
public:
	
	BackpackIconGui()
		: flDelay(0)
	{ }
	
	void init();
	void update(const Rectf & parent);
	void updateInput();
	void draw();
	
};

class StealIconGui : public HudIconBase {
	
	Vec2f m_size;
	
public:
	StealIconGui()
		: m_size(0.f)
	{ }
	
	void init();
	void updateRect(const Rectf & parent);
	void updateInput();
	void draw();
};

class LevelUpIconGui : public HudIconBase {
	
	Vec2f m_size;
	bool m_visible;
	
public:
	LevelUpIconGui();
	
	void init();
	void update(const Rectf & parent);
	void updateInput();
	void draw();
};

class PurseIconGui : public HudIconBase {
	
	Vec2f m_size;
	PlatformDuration m_haloTime;
	
public:
	PurseIconGui();
	
	void init();
	void update(const Rectf & parent);
	void updateInput();
	void draw();
	void requestHalo();
};

class CurrentTorchIconGui : public HudItem {
	
	bool m_isActive;
	TextureContainer * m_tex;
	Vec2f m_size;
	bool isVisible();
	void createFireParticle();
	
public:
	CurrentTorchIconGui();
	
	void init();
	void updateRect(const Rectf & parent);
	void updateInput();
	void update();
	void draw();
};

class ChangeLevelIconGui : public HudItem {
	
	TextureContainer * m_tex;
	Vec2f m_size;
	float m_intensity;
	
public:
	ChangeLevelIconGui();
	
	void init();
	bool isVisible();
	void update(const Rectf & parent);
	void draw();
};

class QuickSaveIconGui {
	
	//! Time in ms to show the icon
	GameDuration m_duration;
	//! Remaining time for the quick save icon
	GameDuration m_remainingTime;
	
public:
	QuickSaveIconGui();
	
	void update();
	void draw();
	void show();
	void hide();
};

class MemorizedRunesHud : public HudIconBase {
	
	Vec2f m_size;
	int m_count;
	
public:
	MemorizedRunesHud();
	
	void update(const Rectf & parent);
	void draw();
};

class HealthGauge : public HudItem {
	
	Vec2f m_size;
	Color m_color;
	TextureContainer * m_emptyTex;
	TextureContainer * m_filledTex;
	float m_amount;
	
public:
	HealthGauge();
	
	void init();
	void updateRect(const Rectf & parent);
	void update();
	void updateInput(const Vec2f & mousePos);
	void draw();
};

class ManaGauge : public HudItem {
	
	Vec2f m_size;
	TextureContainer * m_emptyTex;
	TextureContainer * m_filledTex;
	float m_amount;
	
public:
	ManaGauge();
	
	void init();
	void update(const Rectf & parent);
	void updateInput(const Vec2f & mousePos);
	void draw();
};

// The cogwheel icon that shows up when switching from mouseview to interaction mode.
class MecanismIcon : public HudItem {
	
	Vec2f m_iconSize;
	TextureContainer * m_tex;
	Color m_color;
	GameDuration m_timeToDraw;
	long m_nbToDraw;
	
public:
	MecanismIcon();
	
	void init();
	void reset();
	void update();
	void draw();
};

class ScreenArrows : public HudItem {
	
	Vec2f m_horizontalArrowSize;
	Vec2f m_verticalArrowSize;
	Rectf m_left;
	Rectf m_right;
	Rectf m_top;
	Rectf m_bottom;
	TextureContainer * m_arrowLeftTex;
	float fArrowMove;
	
public:
	ScreenArrows();
	
	void init();
	void update();
	void draw();
};

class PrecastSpellsGui : public HudItem {
	
	struct PrecastSpellIconSlot {
		Rectf m_rect;
		TextureContainer * m_tc;
		Color m_color;
		PrecastHandle m_precastIndex;
		
		void update(const Rectf & rect, TextureContainer * tc, Color color, PrecastHandle precastIndex);
		void updateInput();
		void draw();
	};
	
	std::vector<PrecastSpellIconSlot> m_icons;
	Vec2f m_iconSize;
	
public:
	PrecastSpellsGui();
	
	bool isVisible();
	void updateRect(const Rectf & parent);
	void update();
	void draw();
};

#include "game/magic/Spell.h"

class ActiveSpellsGui : public HudItem {
	
	struct ActiveSpellIconSlot {
		Rectf m_rect;
		TextureContainer * m_tc;
		Color m_color;
		SpellHandle spellIndex;
		bool m_flicker;
		bool m_abortable;
		
		void updateInput(const Vec2f & mousePos);
		void draw();
	};
	
	TextureContainer * m_texUnknown;
	Vec2f m_slotSize;
	Vec2f m_spacerSize;
	Vec2f m_slotSpacerSize;
	bool m_flickNow;
	PlatformDuration m_flickTime;
	PlatformDuration m_flickInterval;
	
	std::vector<ActiveSpellIconSlot> m_slots;
	
	void spellsByPlayerUpdate(float intensity);
	void spellsOnPlayerUpdate(float intensity);
	void ManageSpellIcon(SpellBase & spell, float intensity, bool flag);
	
public:
	ActiveSpellsGui();
	
	void init();
	void update(const Rectf & parent);
	void updateInput(const Vec2f & mousePos);
	void draw();
};

/*!
 * \brief Damaged Equipment Drawing
 */
class DamagedEquipmentGui : public HudItem {
	
	Vec2f m_size;
	TextureContainer * iconequip[5];
	Color m_colors[5];
	
public:
	DamagedEquipmentGui();
	
	void init();
	void updateRect(const Rectf & parent);
	void update();
	void draw();
};

/*!
 * \brief Stealth Gauge Drawing
 */
class StealthGauge : public HudItem {
	
	TextureContainer * m_texture;
	bool m_visible;
	Color m_color;
	Vec2f m_size;
	
public:
	StealthGauge();
	
	void init();
	void updateRect(const Rectf & parent);
	void update();
	void draw();
};

enum FadeDirection {
	FadeDirection_Out,
	FadeDirection_In,
};

class PlayerInterfaceFader {
	
	long m_direction;
	PlatformDuration m_current;
	
public:
	PlayerInterfaceFader();
	
	void reset();
	void resetSlid();
	void requestFade(FadeDirection showhide, long smooth);
	void update();
};

class HudRoot : public HudItem {
	
	BackpackIconGui backpackIconGui;
	StealIconGui stealIconGui;
	LevelUpIconGui levelUpIconGui;
	CurrentTorchIconGui currentTorchIconGui;
	ChangeLevelIconGui changeLevelIconGui;
	MemorizedRunesHud memorizedRunesHud;
	HealthGauge healthGauge;
	ManaGauge manaGauge;
	ScreenArrows screenArrows;
	PrecastSpellsGui precastSpellsGui;
	ActiveSpellsGui activeSpellsGui;
	DamagedEquipmentGui damagedEquipmentGui;
	StealthGauge stealthGauge;
	
public:
	PlayerInterfaceFader playerInterfaceFader;
	HitStrengthGauge hitStrengthGauge;
	BookIconGui bookIconGui;
	PurseIconGui purseIconGui;
	QuickSaveIconGui quickSaveIconGui;
	MecanismIcon mecanismIcon;
	
	void setScale(float scale);
	float getScale();
	void init();
	void updateInput();
	void draw();
	void recalcScale();
};

extern HudRoot g_hudRoot;

#endif // ARX_GUI_HUD_H
