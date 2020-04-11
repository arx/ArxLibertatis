/*
 * Copyright 2015-2019 Arx Libertatis Team (see the AUTHORS file)
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
#include "gui/Hud.h"
#include "gui/Interface.h"
#include "gui/hud/HudCommon.h"
#include "gui/hud/PlayerInventory.h"
#include "input/Input.h"
#include "scene/GameSound.h"
#include "scene/Interactive.h"


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
		
		if(DRAGINTER == NULL)
			return;
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
	
	if(eeMouseDown1()) {
		
		Entity * io = NULL;
		if(SecondaryInventory) {
			io = SecondaryInventory->io;
		} else if(player.Interface & INTER_STEAL) {
			io = ioSteal;
		}
		
		if(io) {
			ARX_SOUND_PlayInterface(g_snd.BACKPACK, Random::getf(0.9f, 1.1f));
			g_secondaryInventoryHud.m_fadeDirection = SecondaryInventoryHud::Fade_left;
			SendIOScriptEvent(entities.player(), io, SM_INVENTORY2_CLOSE);
			TSecondaryInventory = SecondaryInventory;
			SecondaryInventory = NULL;
		}
		
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

static Entity * getSecondaryOrStealInvEntity() {
	if(SecondaryInventory) {
		return SecondaryInventory->io;
	}
	if(player.Interface & INTER_STEAL) {
		return ioSteal;
	}
	return NULL;
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
				TSecondaryInventory = SecondaryInventory;
				SecondaryInventory = NULL;
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
		if(TSecondaryInventory) {
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
	const INVENTORY_DATA * inventory = TSecondaryInventory;
	
	if(!inventory)
		return;
	
	bool _bSteal = (player.Interface & INTER_STEAL) != 0;
	
	arx_assert(m_defaultBackground);
	ingame_inventory = m_defaultBackground;
	if(inventory->io && !inventory->io->inventory_skin.empty()) {
		
		res::path file = "graph/interface/inventory" / inventory->io->inventory_skin;
		TextureContainer * tc = TextureContainer::LoadUI(file);
		if(tc)
			ingame_inventory = tc;
	}
	
	EERIEDrawBitmap(m_rect, 0.001f, ingame_inventory, Color::white);
	
	for(long y = 0; y < inventory->m_size.y; y++) {
		for(long x = 0; x < inventory->m_size.x; x++) {
			
			Entity * io = inventory->slot[x][y].io;
			if(!io) {
				continue;
			}
			
			bool bItemSteal = false;
			TextureContainer * tc = io->m_icon;
			TextureContainer * tc2 = NULL;
			
			if(NeedHalo(io))
				tc2 = io->m_icon->getHalo();
			
			if(_bSteal) {
				if(!ARX_PLAYER_CanStealItem(io)) {
					bItemSteal = true;
					tc = m_canNotSteal;
					tc2 = NULL;
				}
			}
			
			if(tc && (inventory->slot[x][y].show || bItemSteal)) {
				UpdateGoldObject(io);
				
				Vec2f p = Vec2f(m_rect.left + float(x) * (32.f * m_scale) + (2.f * m_scale),
				                float(y) * (32.f * m_scale) + (13.f * m_scale));
				
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
	}
	
	
	if(!(player.Interface & INTER_COMBATMODE) && (player.Interface & INTER_MINIBACK)) {
		if(TSecondaryInventory) {
			Entity * temp = TSecondaryInventory->io;
			if(temp && !(temp->ioflags & IO_SHOP) && !(temp == ioSteal)) {
				m_pickAllButton.draw();
			}
			m_closeButton.draw();
		}
	}
}

void SecondaryInventoryHud::updateInputButtons() {
	
	if(TSecondaryInventory) {
		
		Entity * temp = TSecondaryInventory->io;
		
		if(temp && !(temp->ioflags & IO_SHOP) && !(temp == ioSteal)) {
			m_pickAllButton.updateInput();
		}
		
		m_closeButton.updateInput();
	}
}

bool SecondaryInventoryHud::containsPos(const Vec2s & pos) {
	
	if(SecondaryInventory != NULL) {
		Vec2s t = (pos + Vec2s(checked_range_cast<short>(m_fadePosition), 0) - Vec2s(Vec2f(2.f, 13.f) * m_scale))
		          / s16(32 * m_scale);
		if(t.x >= 0 && t.x < SecondaryInventory->m_size.x && t.y >= 0 && t.y < SecondaryInventory->m_size.y) {
			return true;
		}
	}
	
	return false;
}

Entity * SecondaryInventoryHud::getObj(const Vec2s & pos) {
	
	if(SecondaryInventory != NULL) {
		Vec2s t = (pos + Vec2s(checked_range_cast<short>(m_fadePosition), 0) - Vec2s(Vec2f(2.f, 13.f) * m_scale))
		          / s16(32 * m_scale);
		if(t.x >= 0 && t.x < SecondaryInventory->m_size.x && t.y >= 0 && t.y < SecondaryInventory->m_size.y) {
			Entity * io = SecondaryInventory->slot[t.x][t.y].io;
			if(!io || !(io->gameFlags & GFLAG_INTERACTIVITY)) {
				return NULL;
			}
			if((player.Interface & INTER_STEAL) && !ARX_PLAYER_CanStealItem(io)) {
				return NULL;
			}
			return io;
		}
	}
	
	return NULL;
}

void SecondaryInventoryHud::dropEntity() {
	
	if(!SecondaryInventory || !g_secondaryInventoryHud.containsPos(DANAEMouse)) {
		return;
	}
	
	Entity * io = SecondaryInventory->io;
	
	if(io->ioflags & IO_SHOP) {
		
		if(!io->shop_category.empty() && DRAGINTER->groups.find(io->shop_category) == DRAGINTER->groups.end()) {
			// Item not allowed by shop category
			return;
		}
		
		long price = ARX_INTERACTIVE_GetSellValue(DRAGINTER, io, DRAGINTER->_itemdata->count);
		if(price <= 0) {
			return;
		}
		
		if(insertIntoInventory(DRAGINTER, io)) {
			ARX_PLAYER_AddGold(price);
			ARX_SOUND_PlayInterface(g_snd.GOLD);
			ARX_SOUND_PlayInterface(g_snd.INVSTD);
		}
		
		return;
	}
	
	s16 itemPitch = s16(32.f * m_scale);
	Vec2f pos = Vec2f(DANAEMouse - Vec2s(2 * m_scale - m_fadePosition, 13 * m_scale)) / float(itemPitch);
	
	insertIntoInventoryAt(DRAGINTER, io, 0, pos, g_draggedItemPreviousPosition);
	
}

void SecondaryInventoryHud::dragEntity(Entity * io) {
	
	arx_assert(SecondaryInventory);
	arx_assert(io->ioflags & IO_ITEM);
	arx_assert(locateInInventories(io).io == SecondaryInventory->io->index());
	
	// For shops, check if the player can afford the item and deduct the cost
	Entity * ioo = SecondaryInventory->io;
	if(ioo->ioflags & IO_SHOP) {
		
		long price = ARX_INTERACTIVE_GetPrice(io, ioo);
		price = checked_range_cast<long>(float(price) - float(price) * player.m_skillFull.intuition * 0.005f);
		if(player.gold < price) {
			return;
		}
		
		ARX_SOUND_PlayInterface(g_snd.GOLD);
		player.gold -= price;
		
	}
	
	// Take only one item from stacks unless requested otherwise
	if(io->_itemdata->count > 1
	   && ((ioo->ioflags & IO_SHOP) || !GInput->actionPressed(CONTROLS_CUST_STEALTHMODE))) {
		Entity * unstackedEntity = CloneIOItem(io);
		unstackedEntity->scriptload = 1;
		unstackedEntity->_itemdata->count = 1;
		io->_itemdata->count--;
		ARX_SOUND_PlayInterface(g_snd.INVSTD);
		Set_DragInter(unstackedEntity);
		g_draggedItemPreviousPosition = locateInInventories(io);
		ARX_INVENTORY_IdentifyIO(unstackedEntity);
		return;
	}
	
	Set_DragInter(io);
	ARX_INVENTORY_IdentifyIO(io);
	
}

void SecondaryInventoryHud::open(Entity * container) {
	
	arx_assert(!container || container->inventory);
	
	if(!container || SecondaryInventory == container->inventory) {
		if(SecondaryInventory && SecondaryInventory->io)
			SendIOScriptEvent(entities.player(), SecondaryInventory->io, SM_INVENTORY2_CLOSE);

		g_secondaryInventoryHud.m_fadeDirection = SecondaryInventoryHud::Fade_left;
		TSecondaryInventory = SecondaryInventory;
		SecondaryInventory = NULL;
		DRAGGING = false;
	} else {
		if(TSecondaryInventory && TSecondaryInventory->io)
			SendIOScriptEvent(entities.player(), TSecondaryInventory->io, SM_INVENTORY2_CLOSE);

		g_secondaryInventoryHud.m_fadeDirection = SecondaryInventoryHud::Fade_right;
		TSecondaryInventory = SecondaryInventory = container->inventory;

		if(SecondaryInventory && SecondaryInventory->io != NULL) {
			if(SendIOScriptEvent(entities.player(), SecondaryInventory->io, SM_INVENTORY2_OPEN) == REFUSE) {
				g_secondaryInventoryHud.m_fadeDirection = SecondaryInventoryHud::Fade_left;
				TSecondaryInventory = SecondaryInventory = NULL;
				return;
			}
		}

		if(player.Interface & INTER_COMBATMODE) {
			ARX_INTERFACE_setCombatMode(COMBAT_MODE_OFF);
		}

		if(config.input.autoReadyWeapon != AlwaysAutoReadyWeapon) {
			TRUE_PLAYER_MOUSELOOK_ON = false;
		}

		if(SecondaryInventory && SecondaryInventory->io && (SecondaryInventory->io->ioflags & IO_SHOP)) {
			if(TSecondaryInventory) {
				optimizeInventory(TSecondaryInventory->io);
			}
		}
		
		DRAGGING = false;
	}
	
}

void SecondaryInventoryHud::close() {
	
	Entity * io = NULL;
	
	if(SecondaryInventory)
		io = SecondaryInventory->io;
	else if(player.Interface & INTER_STEAL)
		io = ioSteal;
	
	if(io) {
		m_fadeDirection = Fade_left;
		SendIOScriptEvent(entities.player(), io, SM_INVENTORY2_CLOSE);
		TSecondaryInventory = SecondaryInventory;
		SecondaryInventory = NULL;
	}
}

bool SecondaryInventoryHud::isVisible() {
	return TSecondaryInventory != NULL;
}

bool SecondaryInventoryHud::isOpen() {
	return SecondaryInventory != NULL;
}


bool SecondaryInventoryHud::isOpen(Entity * container) {
	return (isOpen() && SecondaryInventory == container->inventory);
}

void SecondaryInventoryHud::clear(Entity * container) {
	
	if(SecondaryInventory == container->inventory) {
		SecondaryInventory = NULL;
	}
	
	if(TSecondaryInventory == container->inventory) {
		TSecondaryInventory = NULL;
	}
	
}

void SecondaryInventoryHud::updateFader() {
	
	if(m_fadeDirection != Fade_stable) {
		float frameDelay = g_platformTime.lastFrameDuration() / PlatformDurationMs(3);
		
		if((player.Interface & INTER_COMBATMODE) || player.doingmagic >= 2 || m_fadeDirection == Fade_left) {
			if(m_fadePosition > -160)
				m_fadePosition -= frameDelay * m_scale;
		} else {
			if(m_fadePosition < 0)
				m_fadePosition += float(m_fadeDirection) * frameDelay * m_scale;
		}
		
		if(m_fadePosition <= -160) {
			m_fadePosition = -160;
			m_fadeDirection = Fade_stable;
			
			if(player.Interface & INTER_STEAL || ioSteal) {
				SendIOScriptEvent(entities.player(), ioSteal, SM_STEAL, "off");
				player.Interface &= ~INTER_STEAL;
				ioSteal = NULL;
			}
			
			SecondaryInventory = NULL;
			TSecondaryInventory = NULL;
			m_fadeDirection = Fade_stable;
		} else if(m_fadePosition >= 0) {
			m_fadePosition = 0;
			m_fadeDirection = Fade_stable;
		}
	}
}

void SecondaryInventoryHud::takeAllItems() {
	
	if(!TSecondaryInventory) {
		return;
	}
	
	bool success = false;
	for(long y = 0; y < TSecondaryInventory->m_size.y; y++) {
		for(long x = 0; x < TSecondaryInventory->m_size.x; x++) {
			INVENTORY_SLOT & slot = TSecondaryInventory->slot[x][y];
			if(slot.show && insertIntoInventory(slot.io, entities.player())) {
				success = true;
			}
		}
	}
	
	ARX_SOUND_PlayInterface(g_snd.INVSTD, success ? 1.f : 0.1f);
	
}
