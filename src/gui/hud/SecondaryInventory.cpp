/*
 * Copyright 2015-2022 Arx Libertatis Team (see the AUTHORS file)
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

#include "gui/hud/SecondaryInventory.h"

#include "core/Application.h"
#include "core/Config.h"
#include "core/Core.h"
#include "game/EntityManager.h"
#include "core/GameTime.h"
#include "game/Item.h"
#include "game/Player.h"
#include "graphics/Color.h"
#include "graphics/Draw.h"
#include "graphics/data/TextureContainer.h"
#include "gui/Cursor.h"
#include "gui/Dragging.h"
#include "gui/Hud.h"
#include "gui/Interface.h"
#include "gui/hud/HudCommon.h"
#include "gui/hud/PlayerInventory.h"
#include "input/Input.h"
#include "scene/GameSound.h"
#include "scene/Interactive.h"
#include "util/Cast.h"


SecondaryInventoryHud g_secondaryInventoryHud;


void SecondaryInventoryPickAllHudIcon::init() {
	m_tex = TextureContainer::LoadUI("graph/interface/inventory/inv_pick");
	arx_assert(m_tex);
	
	m_size = Vec2f(16, 16);
}

void SecondaryInventoryPickAllHudIcon::update(const Rectf & parent) {
	
	Rectf spacer = createChild(parent, Anchor_BottomLeft, Vec2f(16, 16) * m_scale, Anchor_BottomLeft);
	
	m_rect = createChild(spacer, Anchor_BottomRight, m_size * m_scale, Anchor_BottomLeft);
}

void SecondaryInventoryPickAllHudIcon::updateInput() {
	
	m_isSelected = m_rect.contains(Vec2f(DANAEMouse));
	
	if(m_isSelected) {
		cursorSetInteraction();
		
		if(eeMouseDown1()) {
			// play un son que si un item est pris
			g_secondaryInventoryHud.takeAllItems();
		}
		
	}
	
}


void SecondaryInventoryCloseHudIcon::init() {
	m_tex = TextureContainer::LoadUI("graph/interface/inventory/inv_close");
	arx_assert(m_tex);
	
	m_size = Vec2f(16, 16);
}

void SecondaryInventoryCloseHudIcon::update(const Rectf & parent) {
	
	Rectf spacer = createChild(parent, Anchor_BottomRight, Vec2f(16, 16) * m_scale, Anchor_BottomRight);
	
	m_rect = createChild(spacer, Anchor_BottomLeft, m_size * m_scale, Anchor_BottomRight);
}

void SecondaryInventoryCloseHudIcon::updateInput() {
	
	m_isSelected = m_rect.contains(Vec2f(DANAEMouse));
	if(!m_isSelected) {
		return;
	}
	
	cursorSetInteraction();
	
	if(eeMouseDown1() && g_secondaryInventoryHud.isOpen()) {
		ARX_SOUND_PlayInterface(g_snd.BACKPACK, Random::getf(0.9f, 1.1f));
		g_secondaryInventoryHud.close();
	}
	
}


void SecondaryInventoryHud::init() {
	m_size = Vec2f(115.f, 378.f);
	m_canNotSteal = TextureContainer::LoadUI("graph/interface/icons/cant_steal_item");
	arx_assert(m_canNotSteal);
	
	m_defaultBackground = TextureContainer::LoadUI("graph/interface/inventory/ingame_inventory");
	arx_assert(m_defaultBackground);
	
	m_pickAllButton.init();
	m_closeButton.init();
	
	m_fadeDirection = Fade_stable;
	m_fadePosition = -60.f;
}

Entity * SecondaryInventoryHud::getSecondaryOrStealInvEntity() {
	if(isOpen()) {
		return m_container;
	}
	if(player.Interface & INTER_STEAL) {
		return ioSteal;
	}
	return nullptr;
}

void SecondaryInventoryHud::update() {
	Entity * io = getSecondaryOrStealInvEntity();
	if(io) {
		float dist = fdist(io->pos, player.pos + Vec3f(0.f, 80.f, 0.f));
		
		float maxDist = player.m_telekinesis ? 900.f : 350.f;
		
		if(dist > maxDist) {
			if(m_fadeDirection != Fade_left) {
				ARX_SOUND_PlayInterface(g_snd.BACKPACK, Random::getf(0.9f, 1.1f));
				m_fadeDirection = Fade_left;
				SendIOScriptEvent(entities.player(), io, SM_INVENTORY2_CLOSE);
				m_open = false;
			} else {
				if(player.Interface & INTER_STEAL) {
					player.Interface &= ~INTER_STEAL;
				}
			}
		}
	} else if(m_fadeDirection != Fade_left) {
		m_fadeDirection = Fade_left;
	}
	
	if(!(player.Interface & INTER_COMBATMODE) && (player.Interface & INTER_MINIBACK)) {
		// Pick All/Close Secondary Inventory
		if(m_container) {
			// These have to be calculated on each frame (to make them move).
			m_pickAllButton.setScale(m_scale);
			m_closeButton.setScale(m_scale);
			m_pickAllButton.update(m_rect);
			m_closeButton.update(m_rect);
		}
	}
	
}

void SecondaryInventoryHud::updateRect() {
	
	m_rect = Rectf(Vec2f(m_fadePosition * m_scale, 0.f), m_size.x * m_scale, m_size.y * m_scale);
}

void SecondaryInventoryHud::draw() {
	
	if(!isVisible()) {
		return;
	}
	
	bool _bSteal = (player.Interface & INTER_STEAL) != 0;
	
	arx_assert(m_defaultBackground);
	ingame_inventory = m_defaultBackground;
	if(!m_container->inventory_skin.empty()) {
		res::path file = "graph/interface/inventory" / m_container->inventory_skin;
		TextureContainer * tc = TextureContainer::LoadUI(file);
		if(tc) {
			ingame_inventory = tc;
		}
	}
	
	EERIEDrawBitmap(m_rect, 0.001f, ingame_inventory, Color::white);
	
	for(auto slot : m_container->inventory->slotsInGrid()) {
		
		Entity * io = slot.entity;
		if(!io) {
			continue;
		}
		
		bool bItemSteal = false;
		TextureContainer * tc = io->m_icon;
		TextureContainer * tc2 = nullptr;
		
		if(NeedHalo(io)) {
			tc2 = io->m_icon->getHalo();
		}
		
		if(_bSteal) {
			if(!ARX_PLAYER_CanStealItem(io)) {
				bItemSteal = true;
				tc = m_canNotSteal;
				tc2 = nullptr;
			}
		}
		
		if(tc && (slot.show || bItemSteal)) {
			UpdateGoldObject(io);
			
			Vec2f p = Vec2f(m_rect.left + float(slot.x) * (32.f * m_scale) + (2.f * m_scale),
			                float(slot.y) * (32.f * m_scale) + (13.f * m_scale));
			
			Vec2f size = Vec2f(tc->size());
			
			Color color = (io->poisonous && io->poisonous_count != 0) ? Color::green : Color::white;
			
			if(tc2) {
				ARX_INTERFACE_HALO_Render(io->halo.color, io->halo.flags, tc2, p, Vec2f(m_scale));
			}
			
			Rectf rect(p, size.x * m_scale, size.y * m_scale);
			EERIEDrawBitmap(rect, 0.001f, tc, color);
			
			Color overlayColor = Color::black;
			
			if(!bItemSteal && io == FlyingOverIO) {
				overlayColor = Color::white;
			} else if(!bItemSteal && (io->ioflags & IO_CAN_COMBINE)) {
				overlayColor = Color::gray(glm::abs(glm::cos(glm::radians(fDecPulse))));
			}
			
			if(overlayColor != Color::black) {
				UseRenderState state(render2D().blendAdditive());
				EERIEDrawBitmap(rect, 0.001f, tc, overlayColor);
			}
			
			if((io->ioflags & IO_ITEM) && io->_itemdata->count != 1) {
				ARX_INTERFACE_DrawNumber(rect.topRight(), io->_itemdata->count, Color::white, m_scale);
			}
			
		}
		
	}
	
	if(!(player.Interface & INTER_COMBATMODE) && (player.Interface & INTER_MINIBACK)) {
		if(!(m_container->ioflags & IO_SHOP) && m_container != ioSteal) {
			m_pickAllButton.draw();
		}
		m_closeButton.draw();
	}
	
}

void SecondaryInventoryHud::updateCombineFlags(Entity * source) {
	
	if(!isOpen()) {
		return;
	}
	
	for(auto slot : m_container->inventory->slots()) {
		if(slot.show) {
			updateCombineFlagForEntity(source, slot.entity);
		}
	}
	
}

void SecondaryInventoryHud::updateInputButtons() {
	
	if(isVisible()) {
		
		if(!(m_container->ioflags & IO_SHOP) && m_container != ioSteal) {
			m_pickAllButton.updateInput();
		}
		
		m_closeButton.updateInput();
		
	}
	
}

bool SecondaryInventoryHud::containsPos(Vec2s pos) const noexcept {
	
	if(isOpen()) {
		Vec2s t = (pos + Vec2s(util::to<s16>(m_fadePosition), 0) - Vec2s(Vec2f(2.f, 13.f) * m_scale))
		          / s16(32 * m_scale);
		if(t.x >= 0 && t.x < m_container->inventory->size().x
		   && t.y >= 0 && t.y < m_container->inventory->size().y) {
			return true;
		}
	}
	
	return false;
}

Entity * SecondaryInventoryHud::getObj(Vec2s pos) const noexcept {
	
	if(isOpen()) {
		Vec2s t = (pos + Vec2s(util::to<s16>(m_fadePosition), 0) - Vec2s(Vec2f(2.f, 13.f) * m_scale))
		          / s16(32 * m_scale);
		if(t.x >= 0 && t.x < m_container->inventory->size().x
		   && t.y >= 0 && t.y < m_container->inventory->size().y) {
			Entity * io = m_container->inventory->get(Vec3s(t, 0)).entity;
			if(!io || !(io->gameFlags & GFLAG_INTERACTIVITY)) {
				return nullptr;
			}
			if((player.Interface & INTER_STEAL) && !ARX_PLAYER_CanStealItem(io)) {
				return nullptr;
			}
			return io;
		}
	}
	
	return nullptr;
}

void SecondaryInventoryHud::dropEntity() {
	
	Vec2s mouse = DANAEMouse + Vec2s(g_draggedIconOffset);
	
	if(!isOpen() || !g_secondaryInventoryHud.containsPos(mouse)) {
		return;
	}
	
	if(m_container->ioflags & IO_SHOP) {
		
		if(!m_container->shop_category.empty()
		   && g_draggedEntity->groups.find(m_container->shop_category) == g_draggedEntity->groups.end()) {
			// Item not allowed by shop category
			return;
		}
		
		long price = ARX_INTERACTIVE_GetSellValue(g_draggedEntity, m_container, g_draggedEntity->_itemdata->count);
		if(price <= 0) {
			return;
		}
		
		if(m_container->inventory->insert(g_draggedEntity)) {
			ARX_PLAYER_AddGold(price);
			ARX_SOUND_PlayInterface(g_snd.GOLD);
			ARX_SOUND_PlayInterface(g_snd.INVSTD);
		}
		
		return;
	}
	
	s16 itemPitch = s16(32.f * m_scale);
	Vec2f pos = Vec2f(mouse - Vec2s(2 * m_scale - m_fadePosition, 13 * m_scale)) / float(itemPitch);
	
	insertIntoInventoryAt(g_draggedEntity, m_container, 0, pos, g_draggedItemPreviousPosition);
	
}

void SecondaryInventoryHud::dragEntity(Entity * io) {
	
	arx_assert(isOpen());
	arx_assert(io->ioflags & IO_ITEM);
	
	InventoryPos pos = locateInInventories(io);
	arx_assert(pos.container == m_container);
	Vec2s anchor = Vec2s(2 * m_scale - m_fadePosition, 13 * m_scale);
	s16 itemPitch = s16(32.f * m_scale);
	Vec2f offset(anchor + Vec2s(pos.x, pos.y) * itemPitch - DANAEMouse);
	
	// For shops, check if the player can afford the item and deduct the cost
	Entity * container = m_container;
	if(container->ioflags & IO_SHOP) {
		
		long price = ARX_INTERACTIVE_GetPrice(io, container);
		price = util::to<long>(float(price) - float(price) * player.m_skillFull.intuition * 0.005f);
		if(player.gold < price) {
			return;
		}
		
		ARX_SOUND_PlayInterface(g_snd.GOLD);
		player.gold -= price;
		
	}
	
	ARX_SOUND_PlayInterface(g_snd.INVSTD);
	
	// Take only one item from stacks unless requested otherwise
	if(io->_itemdata->count > 1
	   && ((container->ioflags & IO_SHOP) || !GInput->actionPressed(CONTROLS_CUST_STEALTHMODE))) {
		Entity * unstackedEntity = CloneIOItem(io);
		unstackedEntity->scriptload = 1;
		unstackedEntity->_itemdata->count = 1;
		io->_itemdata->count--;
		setDraggedEntity(unstackedEntity);
		g_draggedItemPreviousPosition = locateInInventories(io);
		g_draggedIconOffset = offset;
		ARX_INVENTORY_IdentifyIO(unstackedEntity);
		return;
	}
	
	setDraggedEntity(io);
	g_draggedIconOffset = offset;
	ARX_INVENTORY_IdentifyIO(io);
	
}

void SecondaryInventoryHud::open(Entity * container) {
	
	if(container && !container->inventory) {
		container = nullptr;
	}
	
	if(!container || isOpen(container)) {
		
		if(isOpen()) {
			SendIOScriptEvent(entities.player(), m_container, SM_INVENTORY2_CLOSE);
		}
		g_secondaryInventoryHud.m_fadeDirection = SecondaryInventoryHud::Fade_left;
		m_open = false;
		DRAGGING = false;
		
	} else {
		
		if(m_container) {
			SendIOScriptEvent(entities.player(), m_container, SM_INVENTORY2_CLOSE);
		}
		g_secondaryInventoryHud.m_fadeDirection = SecondaryInventoryHud::Fade_right;
		m_container = container;
		m_open = true;
		
		if(SendIOScriptEvent(entities.player(), m_container, SM_INVENTORY2_OPEN) == REFUSE) {
			g_secondaryInventoryHud.m_fadeDirection = SecondaryInventoryHud::Fade_left;
			m_container = nullptr;
			m_open = false;
			return;
		}
		
		if(player.Interface & INTER_COMBATMODE) {
			ARX_INTERFACE_setCombatMode(COMBAT_MODE_OFF);
		}
		
		if(config.input.autoReadyWeapon != AlwaysAutoReadyWeapon) {
			TRUE_PLAYER_MOUSELOOK_ON = false;
		}
		
		if(isOpen() && (m_container->ioflags & IO_SHOP)) {
			optimizeInventory(m_container);
		}
		
		DRAGGING = false;
		
	}
	
}

void SecondaryInventoryHud::close() {
	
	Entity * io = getSecondaryOrStealInvEntity();
	
	if(io) {
		m_fadeDirection = Fade_left;
		SendIOScriptEvent(entities.player(), io, SM_INVENTORY2_CLOSE);
		m_open = false;
	}
	
}

bool SecondaryInventoryHud::isVisible() const noexcept {
	return m_container != nullptr;
}

bool SecondaryInventoryHud::isOpen() const noexcept {
	return m_open;
}

bool SecondaryInventoryHud::isOpen(const Entity * container) const noexcept {
	return (isOpen() && m_container == container);
}

void SecondaryInventoryHud::clear(const Entity * container) {
	
	if(m_container == container) {
		m_container = nullptr;
		m_open = false;
	}
	
}

void SecondaryInventoryHud::updateFader() {
	
	if(m_fadeDirection != Fade_stable) {
		
		float frameDelay = g_platformTime.lastFrameDuration() / 3ms;
		if((player.Interface & INTER_COMBATMODE) || player.doingmagic >= 2 || m_fadeDirection == Fade_left) {
			if(m_fadePosition > -160) {
				m_fadePosition -= frameDelay * m_scale;
			}
		} else {
			if(m_fadePosition < 0) {
				m_fadePosition += float(m_fadeDirection) * frameDelay * m_scale;
			}
		}
		
		if(m_fadePosition <= -160) {
			m_fadePosition = -160;
			m_fadeDirection = Fade_stable;
			if(player.Interface & INTER_STEAL || ioSteal) {
				SendIOScriptEvent(entities.player(), ioSteal, SM_STEAL, "off");
				player.Interface &= ~INTER_STEAL;
				ioSteal = nullptr;
			}
			m_container = nullptr;
			m_open = false;
			m_fadeDirection = Fade_stable;
		} else if(m_fadePosition >= 0) {
			m_fadePosition = 0;
			m_fadeDirection = Fade_stable;
		}
		
	}
	
}

void SecondaryInventoryHud::takeAllItems() {
	
	if(!isVisible()) {
		return;
	}
	
	bool success = false;
	for(auto slot : m_container->inventory->slotsInOrder()) {
		if(slot.show && entities.player()->inventory->insert(slot.entity)) {
			success = true;
		}
	}
	
	ARX_SOUND_PlayInterface(g_snd.INVSTD, success ? 1.f : 0.1f);
	
}

void SecondaryInventoryHud::drawItemPrice(float scale) {
	
	if(!isOpen()) {
		return;
	}
	
	if(m_container->ioflags & IO_SHOP) {
		
		Vec2f pos = Vec2f(DANAEMouse);
		pos += Vec2f(60, -10) * scale;
		
		if(g_secondaryInventoryHud.containsPos(DANAEMouse)) {
			
			long amount = ARX_INTERACTIVE_GetPrice(FlyingOverIO, m_container);
			// achat
			float famount = amount - amount * player.m_skillFull.intuition * 0.005f;
			// check should always be OK because amount is supposed positive
			amount = util::to<long>(famount);
			
			Color color = (amount <= player.gold) ? Color::green : Color::red;
			
			ARX_INTERFACE_DrawNumber(pos, amount, color, scale);
			
		} else if(g_playerInventoryHud.containsPos(DANAEMouse)) {
			
			long amount = ARX_INTERACTIVE_GetSellValue(FlyingOverIO, m_container);
			if(amount) {
				Color color = Color::red;
				
				if(m_container->shop_category.empty() ||
				   FlyingOverIO->groups.find(m_container->shop_category) != FlyingOverIO->groups.end()) {
					color = Color::green;
				}
				ARX_INTERFACE_DrawNumber(pos, amount, color, scale);
			}
			
		}
		
	}
	
}

bool SecondaryInventoryHud::isSlotVisible(InventoryPos pos) {
	return isVisible() && pos.container == m_container;
}
