/*
 * Copyright 2015-2018 Arx Libertatis Team (see the AUTHORS file)
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

#include "gui/book/Book.h"

#include <iomanip>

#include "animation/AnimationRender.h"
#include "core/Application.h"
#include "core/Core.h"
#include "core/GameTime.h"
#include "core/Localisation.h"
#include "game/EntityManager.h"
#include "game/Equipment.h"
#include "game/Inventory.h"
#include "game/Levels.h"
#include "game/Player.h"
#include "game/magic/RuneDraw.h"
#include "graphics/Draw.h"
#include "graphics/Renderer.h"
#include "graphics/effects/Halo.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/texture/TextureStage.h"
#include "gui/Cursor.h"
#include "gui/Hud.h"
#include "gui/MiniMap.h"
#include "gui/Speech.h"
#include "gui/TextManager.h"
#include "gui/hud/PlayerInventory.h"
#include "input/Input.h"
#include "scene/GameSound.h"
#include "scene/Interactive.h"
#include "script/Script.h"

long IN_BOOK_DRAW = 0;

PlayerBook g_playerBook;

float g_bookScale = 1.0f;
Rectf g_bookRect = Rectf(Vec2f(97, 64), 513, 313);


void PlayerBook::open() {
	if((player.Interface & INTER_PLAYERBOOK))
		return;
	
	g_playerBook.toggle();
}

void PlayerBook::close() {
	if(!(player.Interface & INTER_PLAYERBOOK))
		return;
	
	g_playerBook.toggle();
}

static bool MouseInBookRect(const Vec2f pos, const Vec2f size) {
	
	Rectf rect = Rectf(pos, size.x, size.y);
	
	return rect.contains(Vec2f(DANAEMouse));
}

bool ARX_INTERFACE_MouseInBook() {
	if((player.Interface & INTER_PLAYERBOOK) && !(player.Interface & INTER_COMBATMODE)) {
		return MouseInBookRect(g_bookRect.topLeft() + Vec2f(2, 1) * g_bookScale, Vec2f(500, 307) * g_bookScale);
	}
	return false;
}


static void DrawBookInterfaceItem(TextureContainer * tc, Vec2f pos, Color color, float z) {
	arx_assert(tc);
	Rectf rect = Rectf(pos, float(tc->m_size.x) * g_bookScale, float(tc->m_size.y) * g_bookScale);
	EERIEDrawBitmap(rect, z, tc, color);
}

const Vec2f PlayerBookPage::m_activeTabOffsets[10] = {
	Vec2f(5.f, 18.f), Vec2f(3.f, 50.f), Vec2f(4.f, 77.f), Vec2f(3.f, 106.f), Vec2f(0.f, 135.f),
	Vec2f(6.f, 162.f), Vec2f(4.f, 191.f), Vec2f(2.f, 219.f), Vec2f(2.f, 243.f), Vec2f(7.f, 267.f)
};

const Vec2f PlayerBookPage::m_tabOffsets[10] = {
	Vec2f(3.f, 18.f), Vec2f(1.f, 48.f), Vec2f(0.f, 79.f), Vec2f(-2.f, 106.f), Vec2f(-2.f, 136.f),
	Vec2f(-3.f, 165.f), Vec2f(-3.f, 195.f), Vec2f(-5.f, 218.f), Vec2f(-7.f, 244.f), Vec2f(0.f, 267.f)
};

void PlayerBookPage::playReleaseSound() {
	ARX_SOUND_PlayInterface(g_snd.MENU_RELEASE);
}

void PlayerBookPage::playErrorSound() {
	ARX_SOUND_PlayInterface(g_snd.MENU_CLICK);
}

void PlayerBook::clearJournal() {
	questBook.clear();
}

void PlayerBookPage::drawTab(long tabNum) {
	Vec2f bookPos = g_bookRect.topLeft();
	float scale = g_bookScale;
	DrawBookInterfaceItem(g_bookResouces.accessibleTab[tabNum], bookPos + m_tabOffsets[tabNum] * scale, Color::white, 0.000001f);
}

void PlayerBookPage::drawActiveTab(long tabNum) {
	Vec2f bookPos = g_bookRect.topLeft();
	float scale = g_bookScale;
	DrawBookInterfaceItem(g_bookResouces.currentTab[tabNum], bookPos + m_activeTabOffsets[tabNum] * scale, Color::white, 0.000001f);
}

void PlayerBookPage::checkTabClick(long tabNum, long & activeTab) {
	
	Vec2f bookPos = g_bookRect.topLeft();
	float scale = g_bookScale;

	if(MouseInBookRect(bookPos + m_tabOffsets[tabNum] * scale, Vec2f(32, 32) * scale)) {
		UseRenderState state(render2D().blendAdditive());
		DrawBookInterfaceItem(g_bookResouces.accessibleTab[tabNum], bookPos + m_tabOffsets[tabNum] * scale,
		                      Color::gray(1.f / 3), 0.000001f);
		cursorSetInteraction();
		if(eeMouseDown1() || eeMouseDown2()) {
			ARX_SOUND_PlayInterface(g_snd.BOOK_PAGE_TURN, Random::getf(0.9f, 1.1f));
			activeTab =  tabNum;
		}
	}
}

void PlayerBookPage::manageLeftTabs(long tabNum, long & activeTab) {
	
	if(activeTab != tabNum) {
		drawTab(tabNum);
		checkTabClick(tabNum, activeTab);
	} else {
		drawActiveTab(tabNum);
	}
}

static void DrawBookTextCenter(Font * font, const Vec2f & pos, const std::string & text, Color col) {

	UNICODE_ARXDrawTextCenter(font, pos, text, col);
}

PlayerBook::PlayerBook()
	: m_currentPage(BOOKMODE_STATS)
	, lastRatio(0.f)
	, lastHudScale(0.f)
	, lastScaleSetting(-1.f)
	, lastMenuMode(MenuMode(-1))
{ }

bool PlayerBook::canOpenPage(ARX_INTERFACE_BOOK_MODE page) {
	switch (page) {
		case BOOKMODE_SPELLS:  return !!player.rune_flags;
		default:               return true;
	}
}

void PlayerBook::forcePage(ARX_INTERFACE_BOOK_MODE page) {
	m_currentPage = page;
}

void PlayerBook::onClosePage() {

	if(currentPage() == BOOKMODE_SPELLS) {
		// Closing spell page - clean up any rune flares
		ARX_SPELLS_ClearAllSymbolDraw();
	}
}

void PlayerBook::toggle() {
	
	if(player.Interface & INTER_PLAYERBOOK) {
		ARX_SOUND_PlayInterface(g_snd.BOOK_CLOSE, Random::getf(0.9f, 1.1f));
		SendIOScriptEvent(NULL, entities.player(), SM_BOOK_CLOSE);
		player.Interface &= ~INTER_PLAYERBOOK;
		g_miniMap.purgeTexContainer();
		onClosePage();
	} else {
		ARX_SOUND_PlayInterface(g_snd.BOOK_OPEN, Random::getf(0.9f, 1.1f));
		SendIOScriptEvent(NULL, entities.player(), SM_BOOK_OPEN);
		ARX_INTERFACE_NoteClose();
		player.Interface |= INTER_PLAYERBOOK;
		map.setMapLevel(glm::clamp(ARX_LEVELS_GetRealNum(CURRENTLEVEL), 0, 7));
	}
	
	if(player.Interface & INTER_COMBATMODE) {
		player.Interface &= ~INTER_COMBATMODE;
		ARX_EQUIPMENT_LaunchPlayerUnReadyWeapon();
	}
	
	if(player.Interface & INTER_INVENTORYALL) {
		ARX_SOUND_PlayInterface(g_snd.BACKPACK, Random::getf(0.9f, 1.1f));
		g_playerInventoryHud.close();
	}
	
	pTextManage->Clear();
	
	TRUE_PLAYER_MOUSELOOK_ON = false;
	
}

void PlayerBook::manage() {
	arx_assert(entities.player());
	
	UseRenderState state(render2D());
	UseTextureState textureState(getInterfaceTextureFilter(), TextureStage::WrapClamp);
	
	update();
	
	switch(currentPage()) {
		case BOOKMODE_STATS: {
			stats.manage();
			break;
		}
		case BOOKMODE_SPELLS: {
			spells.manage();
			break;
		}
		case BOOKMODE_MINIMAP: {
			map.manage();
			break;
		}
		case BOOKMODE_QUESTS: {
			questBook.manage();
			break;
		}
	}
	
	manageTopTabs();
	
}

void PlayerBook::openPage(ARX_INTERFACE_BOOK_MODE newPage, bool _toggle) {
	if((player.Interface & INTER_PLAYERBOOK) && currentPage() == newPage) {

		if(_toggle) {
			// Close the book
			g_playerBook.close();
		}

		return; // nothing to do
	}

	if(!canOpenPage(newPage)) {
		return;
	}

	if(player.Interface & INTER_PLAYERBOOK) {

		onClosePage();

		// If the book is already open, play the page turn sound
		ARX_SOUND_PlayInterface(g_snd.BOOK_PAGE_TURN, Random::getf(0.9f, 1.1f));

	} else {
		// Otherwise open the book
		toggle();
	}

	forcePage(newPage);
}

void PlayerBook::openNextPage() {
	openPage(nextPage());
}

void PlayerBook::openPrevPage() {
	openPage(prevPage());
}

ARX_INTERFACE_BOOK_MODE PlayerBook::nextPage() {

	ARX_INTERFACE_BOOK_MODE nextPage = currentPage(), oldPage;
	do {
		oldPage = nextPage;

		switch(oldPage) {
			case BOOKMODE_STATS:   nextPage = BOOKMODE_SPELLS;  break;
			case BOOKMODE_SPELLS:  nextPage = BOOKMODE_MINIMAP; break;
			case BOOKMODE_MINIMAP: nextPage = BOOKMODE_QUESTS;  break;
			case BOOKMODE_QUESTS:  nextPage = BOOKMODE_QUESTS;  break;
		}

		if(canOpenPage(nextPage)) {
			return nextPage;
		}

	} while(nextPage != oldPage);
	return currentPage();
}

ARX_INTERFACE_BOOK_MODE PlayerBook::prevPage() {

	ARX_INTERFACE_BOOK_MODE prevPage = currentPage(), oldPage;
	do {
		oldPage = prevPage;
		
		switch(oldPage) {
			case BOOKMODE_STATS:   prevPage = BOOKMODE_STATS;   break;
			case BOOKMODE_SPELLS:  prevPage = BOOKMODE_STATS;   break;
			case BOOKMODE_MINIMAP: prevPage = BOOKMODE_SPELLS;  break;
			case BOOKMODE_QUESTS:  prevPage = BOOKMODE_MINIMAP; break;
		}

		if(canOpenPage(prevPage)) {
			return prevPage;
		}

	} while(prevPage != oldPage);
	return currentPage();
}

float PlayerBook::getScale() {
	update();
	return g_bookScale;
}

const Rectf & PlayerBook::getArea() {
	update();
	return g_bookRect;
}

void PlayerBook::manageTopTabs() {
	
	float scale = g_bookScale;
	const Vec2f BOOKMARKS_POS = g_bookRect.topLeft() + Vec2f(119.f, -4.f) * scale;
	
	if(m_currentPage != BOOKMODE_STATS) {
		
		Vec2f pos = BOOKMARKS_POS;
		
		TextureContainer * tcBookmarkChar = g_bookResouces.bookmark_char;
		DrawBookInterfaceItem(tcBookmarkChar, pos, Color::white, 0.000001f);
		
		// Check for cursor on charcter sheet bookmark
		if(MouseInBookRect(pos, Vec2f(tcBookmarkChar->m_size) * scale)) {
			
			// Draw highlighted Character sheet icon
			UseRenderState state(render2D().blendAdditive());
			DrawBookInterfaceItem(tcBookmarkChar, pos, Color::gray(1.f / 3), 0.000001f);
			
			// Set cursor to interacting
			cursorSetInteraction();
			
			// Check for click
			if(eeMouseDown1() || eeMouseDown2()) {
				openPage(BOOKMODE_STATS);
				pTextManage->Clear();
			}
		}
		
	}
	
	if(m_currentPage != BOOKMODE_SPELLS) {
		if(player.rune_flags) {
			Vec2f pos = BOOKMARKS_POS + Vec2f(32, 0) * scale;
			
			DrawBookInterfaceItem(g_bookResouces.bookmark_magic, pos, Color::white, 0.000001f);

			if(NewSpell == 1) {
				NewSpell = 2;
				for(long nk = 0; nk < 2; nk++) {
					// TODO this effect is barely visible
					MagFX(Vec3f(pos, 0.000001f), 1.f);
				}
			}
			
			if(MouseInBookRect(pos, Vec2f(g_bookResouces.bookmark_magic->m_size) * scale)) {
				
				// Draw highlighted Magic sheet icon
				UseRenderState state(render2D().blendAdditive());
				DrawBookInterfaceItem(g_bookResouces.bookmark_magic, pos, Color::gray(1.f / 3), 0.000001f);
				
				// Set cursor to interacting
				cursorSetInteraction();
				
				// Check for click
				if(eeMouseDown1() || eeMouseDown2()) {
					openPage(BOOKMODE_SPELLS);
					pTextManage->Clear();
				}
			}
		}
	}
	
	if(m_currentPage != BOOKMODE_MINIMAP) {
		Vec2f pos = BOOKMARKS_POS + Vec2f(64, 0) * scale;
		
		DrawBookInterfaceItem(g_bookResouces.bookmark_map, pos, Color::white, 0.000001f);
		
		if(MouseInBookRect(pos, Vec2f(g_bookResouces.bookmark_map->m_size) * scale)) {
			
			UseRenderState state(render2D().blendAdditive());
			DrawBookInterfaceItem(g_bookResouces.bookmark_map, pos, Color::gray(1.f / 3), 0.000001f);
			
			// Set cursor to interacting
			cursorSetInteraction();
			
			// Check for click
			if(eeMouseDown1() || eeMouseDown2()) {
				openPage(BOOKMODE_MINIMAP);
				pTextManage->Clear();
			}
		}
	}
	
	if(m_currentPage != BOOKMODE_QUESTS) {
		Vec2f pos = BOOKMARKS_POS + Vec2f(96, 0) * scale;
		
		DrawBookInterfaceItem(g_bookResouces.bookmark_quest, pos, Color::white, 0.000001f);
		
		if(MouseInBookRect(pos, Vec2f(g_bookResouces.bookmark_quest->m_size) * scale)) {
			
			UseRenderState state(render2D().blendAdditive());
			DrawBookInterfaceItem(g_bookResouces.bookmark_quest, pos, Color::gray(1.f / 3), 0.000001f);
			
			// Set cursor to interacting
			cursorSetInteraction();
			
			// Check for click
			if(eeMouseDown1() || eeMouseDown2()) {
				openPage(BOOKMODE_QUESTS);
				pTextManage->Clear();
			}
		}
	}
}


bool PlayerBook::needsUpdate() {
	
	if(lastRatio == g_sizeRatio
	   && lastScaleSetting == config.interface.bookScale
	   && lastHudScale == g_hudRoot.getScale()
	   && lastMenuMode == ARXmenu.mode()) {
		return false;
	} else {
		lastRatio = g_sizeRatio;
		lastHudScale = g_hudRoot.getScale();
		lastScaleSetting = config.interface.bookScale;
		lastMenuMode = ARXmenu.mode();
		return true;
	}
}

void PlayerBook::updateScale() {
	
	float scale = getInterfaceScale(config.interface.bookScale, config.interface.bookScaleInteger);
	
	if(needsUpdate() || scale != g_bookScale) {
		g_bookScale = scale;
		ARX_Text_scaleBookFont(g_bookScale, config.interface.fontWeight);
		updateRect();
	}
	
}

void PlayerBook::updateRect() {
	
	float scale = g_bookScale;
	
	const Rectf bookRectOrig = Rectf(Vec2f(97.f, 64.f), 513.f, 313.f);
	
	g_bookRect = Rectf(Vec2f(g_size.center()) - bookRectOrig.size() * scale / 2.f,
	                   bookRectOrig.width() * scale,
	                   bookRectOrig.height() * scale);
	
	if(ARXmenu.mode() != Mode_CharacterCreation) {
		Rectf availableArea((bookRectOrig.topLeft() + Vec2f(10.f, 0.f)) * g_hudRoot.getScale(),
		                    Vec2f(g_size.bottomRight())
		                    - (Vec2f(640.f, 480.f) - bookRectOrig.bottomRight()) * g_hudRoot.getScale());
		// Move book horizontally to make place for other HUD elements if needed
		// Otherwise keep it centered in the whole screen
		if(g_bookRect.left < availableArea.left) {
			g_bookRect.move(availableArea.left - g_bookRect.left, 0.f);
		}
		if(g_bookRect.right > availableArea.right) {
			g_bookRect.move(availableArea.right - g_bookRect.right, 0.f);
		}
		// Center book vertically in the available area
		g_bookRect.move(0.f, availableArea.center().y - g_bookRect.center().y);
	}
	
}

void PlayerBook::update() {
	updateScale();
}

enum ARX_INTERFACE_BOOK_ITEM
{
	BOOK_NOTHING,
	BOOK_STRENGTH,
	BOOK_MIND,
	BOOK_DEXTERITY,
	BOOK_CONSTITUTION,
	BOOK_STEALTH,
	BOOK_MECANISM,
	BOOK_INTUITION,
	BOOK_ETHERAL_LINK,
	BOOK_OBJECT_KNOWLEDGE,
	BOOK_CASTING,
	BOOK_CLOSE_COMBAT,
	BOOK_PROJECTILE,
	BOOK_DEFENSE,
	WND_ATTRIBUTES,
	WND_SKILLS,
	WND_STATUS,
	WND_LEVEL,
	WND_XP,
	WND_HP,
	WND_MANA,
	WND_AC,
	WND_RESIST_MAGIC,
	WND_RESIST_POISON,
	WND_DAMAGE,
	WND_NEXT_LEVEL
};

void StatsPage::loadStrings() {
	
	flyover[BOOK_STRENGTH] = getLocalised("system_charsheet_strength");
	flyover[BOOK_MIND] = getLocalised("system_charsheet_intel");
	flyover[BOOK_DEXTERITY] = getLocalised("system_charsheet_dex");
	flyover[BOOK_CONSTITUTION] = getLocalised("system_charsheet_consti");
	flyover[BOOK_STEALTH] = getLocalised("system_charsheet_stealth");
	flyover[BOOK_MECANISM] = getLocalised("system_charsheet_mecanism");
	flyover[BOOK_INTUITION] = getLocalised("system_charsheet_intuition");
	flyover[BOOK_ETHERAL_LINK] = getLocalised("system_charsheet_etheral_link");
	flyover[BOOK_OBJECT_KNOWLEDGE] = getLocalised("system_charsheet_objknoledge");
	flyover[BOOK_CASTING] = getLocalised("system_charsheet_casting");
	flyover[BOOK_PROJECTILE] = getLocalised("system_charsheet_projectile");
	flyover[BOOK_CLOSE_COMBAT] = getLocalised("system_charsheet_closecombat");
	flyover[BOOK_DEFENSE] = getLocalised("system_charsheet_defense");
	flyover[WND_ATTRIBUTES] = getLocalised("system_charsheet_atributes");
	flyover[WND_SKILLS] = getLocalised("system_charsheet_skills");
	flyover[WND_STATUS] = getLocalised("system_charsheet_status");
	flyover[WND_LEVEL] = getLocalised("system_charsheet_level");
	flyover[WND_XP] = getLocalised("system_charsheet_xpoints");
	flyover[WND_HP] = getLocalised("system_charsheet_hp");
	flyover[WND_MANA] = getLocalised("system_charsheet_mana");
	flyover[WND_AC] = getLocalised("system_charsheet_ac");
	flyover[WND_RESIST_MAGIC] = getLocalised("system_charsheet_res_magic");
	flyover[WND_RESIST_POISON] = getLocalised("system_charsheet_res_poison");
	flyover[WND_DAMAGE] = getLocalised("system_charsheet_damage");
}

void StatsPage::manage() {
	
	DrawBookInterfaceItem(g_bookResouces.playerbook, g_bookRect.topLeft(), Color::white, 0.9999f);
	
	manageStats();
}

void StatsPage::manageNewQuest() {
	
	g_playerBook.update();

	Vec2f bookPos = g_bookRect.topLeft();
	DrawBookInterfaceItem(g_bookResouces.playerbook, bookPos, Color::white, 0.000001f);
	
	manageStats();
}

void StatsPage::manageStats()
{
	long FLYING_OVER = 0;
	
	ARX_PLAYER_ComputePlayerFullStats();

	Vec2f bookPos = g_bookRect.topLeft();
	float scale = g_bookScale;
	
	{
		std::stringstream ss;
		ss << g_bookResouces.Level << " " << std::setw(3) << player.level;
		DrawBookTextCenter(hFontInBook, bookPos + Vec2f(301, 10) * scale, ss.str(), Color::black);
	}
	
	{
		std::stringstream ss;
		ss << g_bookResouces.Xp << " " << std::setw(8) << player.xp;
		DrawBookTextCenter(hFontInBook, bookPos + Vec2f(413, 10) * scale, ss.str(), Color::black);
	}
	
	if (MouseInBookRect(bookPos + Vec2f(366, 10) * scale, Vec2f(87, 20) * scale))
		FLYING_OVER = WND_XP;
	
	{
		Vec2f attribAreaSize = Vec2f(32, 45) * scale;
		if(MouseInBookRect(bookPos + Vec2f(41, 62) * scale, attribAreaSize)) {
			FLYING_OVER = WND_AC;
		} else if(MouseInBookRect(bookPos + Vec2f(41, 120) * scale, attribAreaSize)) {
			FLYING_OVER = WND_RESIST_MAGIC;
		} else if(MouseInBookRect(bookPos + Vec2f(41, 178) * scale, attribAreaSize)) {
			FLYING_OVER = WND_RESIST_POISON;
		} else if(MouseInBookRect(bookPos + Vec2f(211, 62) * scale, attribAreaSize)) {
			FLYING_OVER = WND_HP;
		} else if(MouseInBookRect(bookPos + Vec2f(211, 120) * scale, attribAreaSize)) {
			FLYING_OVER = WND_MANA;
		} else if(MouseInBookRect(bookPos + Vec2f(211, 178) * scale, attribAreaSize)) {
			FLYING_OVER = WND_DAMAGE;
		}
	}
	
	if(!((player.Attribute_Redistribute == 0) && (ARXmenu.mode() != Mode_CharacterCreation))) {
		// Main Player Attributes
		if(CheckAttributeClick(bookPos + Vec2f(282, 31) * scale, &player.m_attribute.strength, g_bookResouces.ic_strength)) {
			FLYING_OVER = BOOK_STRENGTH;
			cursorSetRedistribute(player.Attribute_Redistribute);
		}

		if(CheckAttributeClick(bookPos + Vec2f(331, 31) * scale, &player.m_attribute.mind, g_bookResouces.ic_mind)) {
			FLYING_OVER = BOOK_MIND;
			cursorSetRedistribute(player.Attribute_Redistribute);
		}

		if(CheckAttributeClick(bookPos + Vec2f(380, 31) * scale, &player.m_attribute.dexterity, g_bookResouces.ic_dexterity)) {
			FLYING_OVER = BOOK_DEXTERITY;
			cursorSetRedistribute(player.Attribute_Redistribute);
		}

		if(CheckAttributeClick(bookPos + Vec2f(429, 31) * scale, &player.m_attribute.constitution, g_bookResouces.ic_constitution)) {
			FLYING_OVER = BOOK_CONSTITUTION;
			cursorSetRedistribute(player.Attribute_Redistribute);
		}
	}

	if(!((player.Skill_Redistribute == 0) && (ARXmenu.mode() != Mode_CharacterCreation))) {
		if (CheckSkillClick(bookPos + Vec2f(293, 113) * scale, &player.m_skill.stealth, g_bookResouces.ic_stealth, player.m_skillOld.stealth)) {
			FLYING_OVER = BOOK_STEALTH;
			cursorSetRedistribute(player.Skill_Redistribute);
		}

		if(CheckSkillClick(bookPos + Vec2f(356, 113) * scale, &player.m_skill.mecanism, g_bookResouces.ic_mecanism, player.m_skillOld.mecanism)) {
			FLYING_OVER = BOOK_MECANISM;
			cursorSetRedistribute(player.Skill_Redistribute);
		}

		if(CheckSkillClick(bookPos + Vec2f(419, 113) * scale, &player.m_skill.intuition, g_bookResouces.ic_intuition, player.m_skillOld.intuition)) {
			FLYING_OVER = BOOK_INTUITION;
			cursorSetRedistribute(player.Skill_Redistribute);
		}

		if(CheckSkillClick(bookPos + Vec2f(293, 166) * scale, &player.m_skill.etheralLink, g_bookResouces.ic_etheral_link, player.m_skillOld.etheralLink)) {
			FLYING_OVER = BOOK_ETHERAL_LINK;
			cursorSetRedistribute(player.Skill_Redistribute);
		}

		if(CheckSkillClick(bookPos + Vec2f(356, 166) * scale, &player.m_skill.objectKnowledge, g_bookResouces.ic_object_knowledge, player.m_skillOld.objectKnowledge)) {
			FLYING_OVER = BOOK_OBJECT_KNOWLEDGE;
			cursorSetRedistribute(player.Skill_Redistribute);

			if(eeMouseDown1()) {
				ARX_INVENTORY_IdentifyAll();
				ARX_EQUIPMENT_IdentifyAll();
			}

			ARX_PLAYER_ComputePlayerFullStats();
		}

		if(CheckSkillClick(bookPos + Vec2f(419, 166) * scale, &player.m_skill.casting, g_bookResouces.ic_casting, player.m_skillOld.casting)) {
			FLYING_OVER = BOOK_CASTING;
			cursorSetRedistribute(player.Skill_Redistribute);
		}

		if(CheckSkillClick(bookPos + Vec2f(293, 220) * scale, &player.m_skill.closeCombat, g_bookResouces.ic_close_combat, player.m_skillOld.closeCombat)) {
			FLYING_OVER = BOOK_CLOSE_COMBAT;
			cursorSetRedistribute(player.Skill_Redistribute);
		}

		if(CheckSkillClick(bookPos + Vec2f(356, 220) * scale, &player.m_skill.projectile, g_bookResouces.ic_projectile, player.m_skillOld.projectile)) {
			FLYING_OVER = BOOK_PROJECTILE;
			cursorSetRedistribute(player.Skill_Redistribute);
		}

		if(CheckSkillClick(bookPos + Vec2f(419, 220) * scale, &player.m_skill.defense, g_bookResouces.ic_defense, player.m_skillOld.defense)) {
			FLYING_OVER = BOOK_DEFENSE;
			cursorSetRedistribute(player.Skill_Redistribute);
		}
	} else {
		if(MouseInBookRect(bookPos + Vec2f(282, 31) * scale, Vec2f(32, 32) * scale)) {
			FLYING_OVER = BOOK_STRENGTH;
		} else if(MouseInBookRect(bookPos + Vec2f(331, 31) * scale, Vec2f(32, 32) * scale)) {
			FLYING_OVER = BOOK_MIND;
		} else if(MouseInBookRect(bookPos + Vec2f(380, 31) * scale, Vec2f(32, 32) * scale)) {
			FLYING_OVER = BOOK_DEXTERITY;
		} else if(MouseInBookRect(bookPos + Vec2f(429, 31) * scale, Vec2f(32, 32) * scale)) {
			FLYING_OVER = BOOK_CONSTITUTION;
		}
		if(MouseInBookRect(bookPos + Vec2f(292, 113) * scale, Vec2f(32, 32) * scale)) {
			FLYING_OVER = BOOK_STEALTH;
		} else if(MouseInBookRect(bookPos + Vec2f(356, 113) * scale, Vec2f(32, 32) * scale)) {
			FLYING_OVER = BOOK_MECANISM;
		} else if(MouseInBookRect(bookPos + Vec2f(419, 113) * scale, Vec2f(32, 32) * scale)) {
			FLYING_OVER = BOOK_INTUITION;
		} else if(MouseInBookRect(bookPos + Vec2f(292, 166) * scale, Vec2f(32, 32) * scale)) {
			FLYING_OVER = BOOK_ETHERAL_LINK;
		} else if(MouseInBookRect(bookPos + Vec2f(356, 166) * scale, Vec2f(32, 32) * scale)) {
			FLYING_OVER = BOOK_OBJECT_KNOWLEDGE;
		} else if(MouseInBookRect(bookPos + Vec2f(419, 166) * scale, Vec2f(32, 32) * scale)) {
			FLYING_OVER = BOOK_CASTING;
		} else if(MouseInBookRect(bookPos + Vec2f(292, 220) * scale, Vec2f(32, 32) * scale)) {
			FLYING_OVER = BOOK_CLOSE_COMBAT;
		} else if(MouseInBookRect(bookPos + Vec2f(356, 220) * scale, Vec2f(32, 32) * scale)) {
			FLYING_OVER = BOOK_PROJECTILE;
		} else if(MouseInBookRect(bookPos + Vec2f(419, 220) * scale, Vec2f(32, 32) * scale)) {
			FLYING_OVER = BOOK_DEFENSE;
		}
	}
	
	
	if(!flyover[FLYING_OVER].empty()) {
		
		int t = Random::get(0, 2);
		
		pTextManage->Clear();
		
		std::string toDisplay;
		if(FLYING_OVER == WND_XP) {
			std::stringstream ss;
			ss << flyover[WND_XP] << " " << std::setw(8) << GetXPforLevel(player.level + 1) - player.xp;
			toDisplay = ss.str();
		} else {
			toDisplay = flyover[FLYING_OVER];
		}
		
		UNICODE_ARXDrawTextCenteredScroll(hFontInGame,
			float(g_size.width()) * 0.5f,
			4,
			float(g_size.center().x) * 0.82f,
			toDisplay,
			Color(232 + t, 204 + t, 143 + t),
			PlatformDurationMs(1000),
			0.01f,
			3,
			0);
	}
	
	//------------------------------
	
	{
	Vec2f pos = bookPos + Vec2f(294, 63) * scale;
	
	std::stringstream ss3;
	ss3 << std::setw(3) << std::setprecision(0) << std::fixed << player.m_attributeFull.strength;
	
	Color color = attributeModToColor(player.m_attributeMod.strength);
	
	if(ARXmenu.mode() == Mode_CharacterCreation) {
		if(player.m_attributeFull.strength == 6)
			color = Color::red;
	}
	
	DrawBookTextCenter(hFontInBook, pos, ss3.str(), color);
	}
	
	{
	Vec2f pos = bookPos + Vec2f(343, 63) * scale;
	
	std::stringstream ss3;
	ss3 << std::setw(3) << std::setprecision(0) << std::fixed << player.m_attributeFull.mind;
	
	Color color = attributeModToColor(player.m_attributeMod.mind);
	
	if(ARXmenu.mode() == Mode_CharacterCreation) {
		if(player.m_attributeFull.mind == 6)
			color = Color::red;
	}
	
	DrawBookTextCenter(hFontInBook, pos, ss3.str(), color);
	}
	
	{
	Vec2f pos = bookPos + Vec2f(393, 63) * scale;
	
	std::stringstream ss3;
	ss3 << std::setw(3) << std::setprecision(0) << std::fixed << player.m_attributeFull.dexterity;
	
	Color color = attributeModToColor(player.m_attributeMod.dexterity);
	
	if(ARXmenu.mode() == Mode_CharacterCreation) {
		if(player.m_attributeFull.dexterity == 6)
			color = Color::red;
	}
	
	DrawBookTextCenter(hFontInBook, pos, ss3.str(), color);
	}
	
	{
	Vec2f pos = bookPos + Vec2f(441, 63) * scale;
	
	std::stringstream ss3;
	ss3 << std::setw(3) << std::setprecision(0) << std::fixed << player.m_attributeFull.constitution;
	
	Color color = attributeModToColor(player.m_attributeMod.constitution);
	
	if(ARXmenu.mode() == Mode_CharacterCreation) {
		if(player.m_attributeFull.constitution == 6)
			color = Color::red;
	}
	
	DrawBookTextCenter(hFontInBook, pos, ss3.str(), color);
	}
	
	// Player Skills
	{
	Vec2f pos = bookPos + Vec2f(305, 144) * scale;
	
	std::stringstream ss3;
	ss3 << std::setw(3) << std::setprecision(0) << std::fixed << player.m_skillFull.stealth;
	
	Color color = attributeModToColor(player.m_skillMod.stealth);
	
	if(ARXmenu.mode() == Mode_CharacterCreation) {
		if(player.m_skill.stealth == 0)
			color = Color::red;
	}
	
	DrawBookTextCenter(hFontInBook, pos, ss3.str(), color);
	}
	
	{
		Vec2f pos = bookPos + Vec2f(369, 144) * scale;
	
	std::stringstream ss3;
	ss3 << std::setw(3) << std::setprecision(0) << std::fixed << player.m_skillFull.mecanism;
	
	Color color = attributeModToColor(player.m_skillMod.mecanism);
	
	if(ARXmenu.mode() == Mode_CharacterCreation) {
		if(player.m_skill.mecanism == 0)
			color = Color::red;
	}
	
	DrawBookTextCenter(hFontInBook, pos, ss3.str(), color);
	}
	
	{
	Vec2f pos = bookPos + Vec2f(433, 144) * scale;
	
	std::stringstream ss3;
	ss3 << std::setw(3) << std::setprecision(0) << std::fixed << player.m_skillFull.intuition;
	
	Color color = attributeModToColor(player.m_skillMod.intuition);
	
	if(ARXmenu.mode() == Mode_CharacterCreation) {
		if(player.m_skill.intuition == 0)
			color = Color::red;
	}

	DrawBookTextCenter(hFontInBook, pos, ss3.str(), color);
	}
	
	{
	Vec2f pos = bookPos + Vec2f(305, 198) * scale;
	
	std::stringstream ss3;
	ss3 << std::setw(3) << std::setprecision(0) << std::fixed << player.m_skillFull.etheralLink;
	
	Color color = attributeModToColor(player.m_skillMod.etheralLink);
	
	if(ARXmenu.mode() == Mode_CharacterCreation) {
		if(player.m_skill.etheralLink == 0)
			color = Color::red;
	}

	DrawBookTextCenter(hFontInBook, pos, ss3.str(), color);
	}
	
	{
	Vec2f pos = bookPos + Vec2f(369, 198) * scale;
	
	std::stringstream ss3;
	ss3 << std::setw(3) << std::setprecision(0) << std::fixed << player.m_skillFull.objectKnowledge;
	
	Color color = attributeModToColor(player.m_skillMod.objectKnowledge);
	
	if(ARXmenu.mode() == Mode_CharacterCreation) {
		if(player.m_skill.objectKnowledge == 0)
			color = Color::red;
	}
	
	DrawBookTextCenter(hFontInBook, pos, ss3.str(), color);
	}
	
	{
	Vec2f pos = bookPos + Vec2f(433, 198) * scale;
	
	std::stringstream ss3;
	ss3 << std::setw(3) << std::setprecision(0) << std::fixed << player.m_skillFull.casting;
	
	Color color = attributeModToColor(player.m_skillMod.casting);
	
	if(ARXmenu.mode() == Mode_CharacterCreation) {
		if(player.m_skill.casting == 0)
			color = Color::red;
	}

	DrawBookTextCenter(hFontInBook, pos, ss3.str(), color);
	}
	
	{
	Vec2f pos = bookPos + Vec2f(305, 253) * scale;
	
	std::stringstream ss3;
	ss3 << std::setw(3) << std::setprecision(0) << std::fixed << player.m_skillFull.closeCombat;
	
	Color color = attributeModToColor(player.m_skillMod.closeCombat);
	
	if(ARXmenu.mode() == Mode_CharacterCreation) {
		if(player.m_skill.closeCombat == 0)
			color = Color::red;
	}

	DrawBookTextCenter(hFontInBook, pos, ss3.str(), color);
	}

	{
	Vec2f pos = bookPos + Vec2f(369, 253) * scale;
	
	std::stringstream ss3;
	ss3 << std::setw(3) << std::setprecision(0) << std::fixed << player.m_skillFull.projectile;
	
	Color color = attributeModToColor(player.m_skillMod.projectile);
	
	if(ARXmenu.mode() == Mode_CharacterCreation) {
		if(player.m_skill.projectile == 0)
			color = Color::red;
	}

	DrawBookTextCenter(hFontInBook, pos, ss3.str(), color);
	}
	
	{
	Vec2f pos = bookPos + Vec2f(433, 253) * scale;
	
	std::stringstream ss3;
	ss3 << std::setw(3) << std::setprecision(0) << std::fixed << player.m_skillFull.defense;
	
	Color color = attributeModToColor(player.m_skillMod.defense);
	
	if(ARXmenu.mode() == Mode_CharacterCreation) {
		if(player.m_skill.defense == 0)
			color = Color::red;
	}
	
	DrawBookTextCenter(hFontInBook, pos, ss3.str(), color);
	}
	
	// Secondary Attributes
	{
		Vec2f pos = bookPos + Vec2f(227, 90) * scale;
		std::string value = boost::lexical_cast<std::string>(std::ceil(player.Full_maxlife));
		Color color = attributeModToColor(player.Full_maxlife, player.lifePool.max);
		DrawBookTextCenter(hFontInBook, pos, value, color);
	}
	
	{
		Vec2f pos = bookPos + Vec2f(227, 150) * scale;
		std::string value = boost::lexical_cast<std::string>(std::ceil(player.Full_maxmana));
		Color color = attributeModToColor(player.Full_maxmana, player.manaPool.max);
		DrawBookTextCenter(hFontInBook, pos, value, color);
	}
	
	{
		Vec2f pos = bookPos + Vec2f(227, 210) * scale;
		std::string value = boost::lexical_cast<std::string>(std::ceil(player.m_miscFull.damages));
		Color color = attributeModToColor(player.m_miscMod.damages);
		DrawBookTextCenter(hFontInBook, pos, value, color);
	}
	
	{
		Vec2f pos = bookPos + Vec2f(54, 90) * scale;
		std::string value = boost::lexical_cast<std::string>(std::ceil(player.m_miscFull.armorClass));
		Color color = attributeModToColor(player.m_miscMod.armorClass);
		DrawBookTextCenter(hFontInBook, pos, value, color);
	}
	
	{
		Vec2f pos = bookPos + Vec2f(54, 150) * scale;
		std::string value = boost::lexical_cast<std::string>(std::ceil(player.m_miscFull.resistMagic));
		Color color = attributeModToColor(player.m_miscMod.resistMagic);
		DrawBookTextCenter(hFontInBook, pos, value, color);
	}
	
	{
		Vec2f pos = bookPos + Vec2f(54, 210) * scale;
		std::string value = boost::lexical_cast<std::string>(std::ceil(player.m_miscFull.resistPoison));
		Color color = attributeModToColor(player.m_miscMod.resistPoison);
		DrawBookTextCenter(hFontInBook, pos, value, color);
	}
	
	RenderBookPlayerCharacter();
	
}

void StatsPage::RenderBookPlayerCharacter() {
	
	// TODO use assert ?
	if(!entities.player()->obj)
		return;
	
	float scale = g_bookScale;
	Vec2f bookPos = g_bookRect.topLeft();
	
	Rect rec;
	if (ARXmenu.mode() == Mode_CharacterCreation) {
		
		rec = Rect(Vec2i(bookPos + Vec2f(23.f, 5.f) * scale),
		           Vec2i(bookPos + Vec2f(233.f, 236.f) * scale));
		GRenderer->Clear(Renderer::DepthBuffer, Color::none, 1.f, 1, &rec);
		
		Rect vp = Rect(Vec2i(bookPos + Vec2f(75.f, 5.f) * scale),
		               Vec2i(bookPos + Vec2f(212.f, 219.f) * scale));
		GRenderer->SetScissor(vp);
		
	} else {
		
		rec = Rect(Vec2i(bookPos + Vec2f(21.f, 5.f) * scale),
		           Vec2i(bookPos + Vec2f(253.f, 274.f) * scale));
		GRenderer->Clear(Renderer::DepthBuffer, Color::none, 1.f, 1, &rec);
		rec.right -= s32(50.f * scale);
		
	}
	
	EERIE_LIGHT eLight1;
	eLight1.pos = Vec3f(50.f, 50.f, 200.f);
	eLight1.m_exists = true;
	eLight1.rgb = Color3f(0.15f, 0.06f, 0.003f);
	eLight1.intensity = 8.8f;
	eLight1.fallstart = 2020;
	eLight1.fallend = eLight1.fallstart + 60;
	RecalcLight(&eLight1);
	
	EERIE_LIGHT eLight2;
	eLight2.m_exists = true;
	eLight2.pos = Vec3f(-50.f, -50.f, -200.f);
	eLight2.rgb = Color3f::gray(0.6f);
	eLight2.intensity = 3.8f;
	eLight2.fallstart = 0;
	eLight2.fallend = eLight2.fallstart + 3460.f;
	RecalcLight(&eLight2);
	
	EERIE_LIGHT * SavePDL[2];
	SavePDL[0] = g_culledDynamicLights[0];
	SavePDL[1] = g_culledDynamicLights[1];
	size_t iSavePDL = g_culledDynamicLightsCount;
	
	g_culledDynamicLights[0] = &eLight1;
	g_culledDynamicLights[1] = &eLight2;
	g_culledDynamicLightsCount = 2;
	
	Camera bookcam;
	bookcam.angle = Anglef();
	bookcam.m_pos = Vec3f(0.f);
	bookcam.focal = 520.f;
	bookcam.cdepth = 2200.f;
	
	Camera * oldcam = g_camera;
	PrepareCamera(&bookcam, Rect(g_bookRect), rec.center());
	
	GRenderer->SetAntialiasing(true);
	
	Anglef ePlayerAngle;
	Vec3f pos;
	if(ARXmenu.mode() == Mode_CharacterCreation) {
		
		switch(player.skin) {
			case 0:
				ePlayerAngle.setYaw(-25.f);
				break;
			case 1:
				ePlayerAngle.setYaw(-10.f);
				break;
			case 2:
				ePlayerAngle.setYaw(20.f);
				break;
			case 3:
				ePlayerAngle.setYaw(35.f);
				break;
		}
		
		pos = Vec3f(8, 162, 75);
		eLight1.pos.z = -90.f;
		
	} else {
		
		ePlayerAngle.setYaw(-20.f);
		pos = Vec3f(20.f, 96.f, 260.f);
		
		ARX_EQUIPMENT_AttachPlayerWeaponToHand();
	}
	
	bool ti = player.m_improve;
	player.m_improve = false;
	
	
	float invisibility = entities.player()->invisibility;
	
	if(invisibility > 0.5f)
		invisibility = 0.5f;
	
	IN_BOOK_DRAW = 1;
	std::vector<EERIE_VERTEX> vertexlist = entities.player()->obj->vertexWorldPositions;
	
	arx_assert(player.bookAnimation[0].cur_anim);
	
	{
		EERIE_3DOBJ * eobj = entities.player()->obj;
		AnimationDuration time = toAnimationDuration(g_platformTime.lastFrameDuration());
		
		EERIEDrawAnimQuatUpdate(eobj, player.bookAnimation, ePlayerAngle, pos, time, NULL, true);
		EERIEDrawAnimQuatRender(eobj, pos, NULL, invisibility);
	}
	
	IN_BOOK_DRAW = 0;
	
	Halo_Render();
	
	PopAllTriangleListOpaque(render3D().fog(false));
	PopAllTriangleListTransparency();
	
	g_culledDynamicLights[0] = SavePDL[0];
	g_culledDynamicLights[1] = SavePDL[1];
	g_culledDynamicLightsCount = iSavePDL;
	
	entities.player()->obj->vertexWorldPositions = vertexlist;
	vertexlist.clear();
	
	player.m_improve = ti;
	
	if(ARXmenu.mode() == Mode_CharacterCreation) {
		GRenderer->SetScissor(Rect::ZERO);
	}
	
	GRenderer->SetAntialiasing(false);
	
	PrepareCamera(oldcam, g_size);
	
	player.bookAnimation[0].cur_anim = herowaitbook;
	
	if(Entity * weapon = entities.get(player.equiped[EQUIP_SLOT_WEAPON])) {
		if(weapon->type_flags & OBJECT_TYPE_2H) {
			player.bookAnimation[0].cur_anim = herowait_2h;
		}
		// TODO workaround for bbox2D being relative to viewport
		weapon->bbox2D.min += bookPos;
		weapon->bbox2D.max += bookPos;
	}
	
	if(Entity * shield = entities.get(player.equiped[EQUIP_SLOT_SHIELD])) {
		// TODO workaround for bbox2D being relative to viewport
		shield->bbox2D.min += bookPos;
		shield->bbox2D.max += bookPos;
	}
	
	if(Entity * tod = entities.get(player.equiped[EQUIP_SLOT_ARMOR])) {
		tod->bbox2D.min = bookPos + Vec2f(90.f, 52.f) * scale;
		tod->bbox2D.max = bookPos + Vec2f(170.f, 118.f) * scale;
		tod->ioflags |= IO_ICONIC;
	}
	
	if(Entity * tod = entities.get(player.equiped[EQUIP_SLOT_LEGGINGS])) {
		tod->bbox2D.min = bookPos + Vec2f(110.f, 119.f) * scale;
		tod->bbox2D.max = bookPos + Vec2f(170.f, 265.f) * scale;
		tod->ioflags |= IO_ICONIC;
	}
	
	if(Entity * tod = entities.get(player.equiped[EQUIP_SLOT_HELMET])) {
		tod->bbox2D.min = bookPos + Vec2f(115.f, 20.f) * scale;
		tod->bbox2D.max = bookPos + Vec2f(153.f, 51.f) * scale;
		tod->ioflags |= IO_ICONIC;
	}
	
	if(Entity * todraw = entities.get(player.equiped[EQUIP_SLOT_RING_LEFT])) {
		
		TextureContainer * tc = todraw->m_icon;
		TextureContainer * tc2 = NULL;
		
		if(NeedHalo(todraw))
			tc2 = todraw->m_icon->getHalo();
		
		if(tc) {
			todraw->bbox2D.min = bookPos + Vec2f(50.f, 246.f) * scale;
			
			if(tc2) {
				ARX_INTERFACE_HALO_Render(todraw->halo.color, todraw->halo.flags, tc2, todraw->bbox2D.min, Vec2f(scale));
			}
			
			Color color = (todraw->poisonous && todraw->poisonous_count != 0) ? Color::green : Color::white;
			DrawBookInterfaceItem(tc, todraw->bbox2D.min, color, 0);
			
			todraw->bbox2D.max = todraw->bbox2D.min + Vec2f(tc->size()) * g_bookScale;
			
			todraw->ioflags |= IO_ICONIC;
		}
	}
	
	if(Entity * todraw = entities.get(player.equiped[EQUIP_SLOT_RING_RIGHT])) {
		
		TextureContainer * tc = todraw->m_icon;
		TextureContainer * tc2 = NULL;
		
		if(NeedHalo(todraw))
			tc2 = todraw->m_icon->getHalo();
		
		if(tc) {
			todraw->bbox2D.min = bookPos + Vec2f(200.f, 246.f) * scale;
			
			if(tc2) {
				ARX_INTERFACE_HALO_Render(todraw->halo.color, todraw->halo.flags, tc2, todraw->bbox2D.min, Vec2f(scale));
			}
			
			Color color = (todraw->poisonous && todraw->poisonous_count != 0) ? Color::green : Color::white;
			DrawBookInterfaceItem(tc, todraw->bbox2D.min, color, 0);
			
			todraw->bbox2D.max = todraw->bbox2D.min + Vec2f(tc->size()) * g_bookScale;
			
			todraw->ioflags |= IO_ICONIC;
		}
	}
	
	if(!(ARXmenu.mode() == Mode_CharacterCreation))
		ARX_EQUIPMENT_AttachPlayerWeaponToBack();
	
}

bool StatsPage::CheckAttributeClick(Vec2f pos, float * val, TextureContainer * tc) {
	
	bool rval = false;
	float t = *val;
	
	float scale = g_bookScale;
	
	if(MouseInBookRect(pos, Vec2f(32, 32) * scale)) {
		
		rval = true;
		
		if((eeMousePressed1() || eeMousePressed2()) && tc) {
			DrawBookInterfaceItem(tc, pos, Color::white, 0.000001f);
		}
		
		if(eeMouseUp1()) {
			if(player.Attribute_Redistribute > 0) {
				player.Attribute_Redistribute--;
				t++;
				*val = t;
				playReleaseSound();
			} else {
				playErrorSound();
			}
		}
		
		if(eeMouseUp2()) {
			if(ARXmenu.mode() == Mode_CharacterCreation) {
				if(t > 6 && player.level == 0) {
					player.Attribute_Redistribute++;
					t--;
					*val = t;
					playReleaseSound();
				} else {
					playErrorSound();
				}
			} else {
				playErrorSound();
			}
		}
		
	}
	
	return rval;
}

bool StatsPage::CheckSkillClick(Vec2f pos, float * val, TextureContainer * tc, float oldval) {
	
	bool rval = false;
	
	float t = *val;
	float ot = oldval;

	float scale = g_bookScale;
	
	if(MouseInBookRect(pos, Vec2f(32, 32) * scale)) {
		
		rval = true;
		
		if((eeMousePressed1() || eeMousePressed2()) && tc)
			DrawBookInterfaceItem(tc, pos, Color::white, 0.000001f);
		
		if(eeMouseUp1()) {
			if(player.Skill_Redistribute > 0) {
				player.Skill_Redistribute--;
				t++;
				*val = t;
				playReleaseSound();
			} else {
				playErrorSound();
			}
		}
		
		if(eeMouseUp2()) {
			if(ARXmenu.mode() == Mode_CharacterCreation) {
				if(t > ot && player.level == 0) {
					player.Skill_Redistribute++;
					t--;
					*val = t;
					playReleaseSound();
				} else {
					playErrorSound();
				}
			} else {
				playErrorSound();
			}
		}
		
	}
	
	return rval;
}

Color StatsPage::attributeModToColor(float modValue, float baseValue) {
	if(modValue < baseValue) {
		return Color::red;
	}
	if(modValue > baseValue) {
		return Color::blue;
	}
	return Color::black;
}

SpellsPage::SpellsPage()
	: m_currentTab(0)
{
}

void SpellsPage::manage() {
	
	DrawBookInterfaceItem(g_bookResouces.ptexspellbook, g_bookRect.topLeft(), Color::white, 0.9999f);
	drawLeftTabs();
	
	float scale = g_bookScale;
	
	Rect runeDrawRect = Rect(Vec2i((g_bookRect.topLeft() + Vec2f(29, 210) * scale)),
	                         s32(g_bookRect.width() * 0.43f), s32(g_bookRect.height() * 0.25f));
	
	ARX_SPELLS_UpdateBookSymbolDraw(runeDrawRect);
	gui::ARX_INTERFACE_ManageOpenedBook_Finish(Vec2f(DANAEMouse), g_bookRect, g_bookScale);
	drawSpells();
	
}

void SpellsPage::drawLeftTabs() {
	
	for(size_t i = 0; i < SPELL_TYPES_COUNT; ++i) {
		if(!spellicons[i].bSecret) {
			bool bOk = true;

			for(long j = 0; j < 4 && spellicons[i].symbols[j] != RUNE_NONE; ++j) {
				if(!player.hasRune(spellicons[i].symbols[j]))
					bOk = false;
			}

			if(bOk)
				PlayerBookPage::manageLeftTabs(spellicons[i].level - 1, m_currentTab);
		}
	}
}

void SpellsPage::drawSpells() {
	
	// Now Draws Spells for this level...
	ARX_PLAYER_ComputePlayerFullStats();
	
	Vec2f tmpPos(0.f);
	
	for(size_t i = 0; i < SPELL_TYPES_COUNT; i++) {
		const SPELL_ICON & spellInfo = spellicons[i];
		
		if(spellInfo.level != (m_currentTab + 1) || spellInfo.bSecret)
			continue;
		
		// check if player can cast it
		bool bOk = true;
		for(long j = 0; j < 4 && spellInfo.symbols[j] != RUNE_NONE; j++) {
			if(!player.hasRune(spellInfo.symbols[j])) {
				bOk = false;
			}
		}
		
		if(!bOk)
			continue;
			
		float scale = g_bookScale;
		Vec2f bookPos = g_bookRect.topLeft();
		Vec2f fPos = bookPos + Vec2f(73.f, 71.f) * scale + tmpPos * Vec2f(85.f, 70.f) * scale;
		long flyingover = 0;
		
		if(MouseInBookRect(fPos, Vec2f(48, 48) * scale)) {
			flyingover = 1;
			
			cursorSetInteraction();
			DrawBookTextCenter(hFontInBook, bookPos + Vec2f(111, 26) * scale, spellInfo.name, Color::none);
			
			pTextManage->Clear();
			UNICODE_ARXDrawTextCenteredScroll(hFontInGame,
				float(g_size.center().x),
				12,
				float(g_size.center().x) * 0.82f,
				spellInfo.description,
				Color(232, 204, 143),
				PlatformDurationMs(1000),
				0.01f,
				2,
				0);
			
			size_t count = 0;
			
			for(size_t j = 0; j < 6; ++j) {
				if(spellInfo.symbols[j] != RUNE_NONE) {
					++count;
				}
			}
			
			for(size_t j = 0; j < 6; ++j) {
				if(spellInfo.symbols[j] != RUNE_NONE) {
					Vec2f pos = bookPos + Vec2f(143.f - float(count) * 16.f + float(j) * 32.f, 242.f) * scale;
					DrawBookInterfaceItem(gui::necklace.pTexTab[spellInfo.symbols[j]], pos, Color::white, 0.000001f);
				}
			}
			
		}
		
		if(spellInfo.tc) {
			
			UseRenderState state(render2D().blend(BlendZero, BlendInvSrcColor).alphaCutout());
			
			Color color;
			if(flyingover) {
				color = Color::white;
				
				if(eeMouseDown1()) {
					player.SpellToMemorize.bSpell = true;
					
					for(long j = 0; j < 6; j++) {
						player.SpellToMemorize.iSpellSymbols[j] = spellInfo.symbols[j];
					}
					
					player.SpellToMemorize.lTimeCreation = g_gameTime.now();
				}
			} else {
				color = Color(168, 208, 223);
			}
			
			DrawBookInterfaceItem(spellInfo.tc, fPos, color, 0.000001f);
			
		}
		
		tmpPos.x++;
		
		if(tmpPos.x >= 2) {
			tmpPos.x = 0;
			tmpPos.y++;
		}
		
	}
}

MapPage::MapPage()
	: m_currentLevel(0)
{
}

void MapPage::manage() {
	DrawBookInterfaceItem(g_bookResouces.questbook, g_bookRect.topLeft(), Color::white, 0.9999f);
	drawLeftTabs();
	drawMaps();
}

void MapPage::setMapLevel(long level) {
	m_currentLevel = level;
}

void MapPage::drawMaps()
{
	long SHOWLEVEL = m_currentLevel;
	Vec2f bookPos = g_bookRect.topLeft();
	float scale = g_bookScale;

	if(SHOWLEVEL >= 0 && SHOWLEVEL < 32)
		g_miniMap.showBookEntireMap(SHOWLEVEL, Rect(Vec2i(bookPos + Vec2f(43, 56) * scale),
		                                            Vec2i(bookPos + Vec2f(248, 226) * scale)), scale);

	SHOWLEVEL = ARX_LEVELS_GetRealNum(CURRENTLEVEL);

	if(SHOWLEVEL >= 0 && SHOWLEVEL < 32)
		g_miniMap.showBookMiniMap(SHOWLEVEL, Rect(Vec2i(bookPos + Vec2f(263, 21) * scale),
		                                          Vec2i(bookPos + Vec2f(480, 291) * scale)), scale);
}

void MapPage::drawLeftTabs() {
	
	size_t max_onglet = 7;
	
	for(size_t i = 0; i <= max_onglet; i++) {
		PlayerBookPage::manageLeftTabs(long(i), m_currentLevel);
	}
	
}

void QuestBookPage::manage() {
	
	// Cache the questbook data
	if(m_questBook.type() == Note::Undefined) {
		std::string text;
		for(size_t i = 0; i < g_playerQuestLogEntries.size(); ++i) {
			std::string quest = getLocalised(g_playerQuestLogEntries[i]);
			if(!quest.empty()) {
				text += quest;
				text += "\n\n";
			}
		}
		m_questBook.setData(Note::QuestBook, text);
		m_questBook.setPage(m_questBook.pageCount() - 1);
	}
	
	m_questBook.manageActions();
	
	m_questBook.render();
	
}

void QuestBookPage::clear() {
	// Clear the quest book cache - it will be re-created when needed
	m_questBook.clear();
}

