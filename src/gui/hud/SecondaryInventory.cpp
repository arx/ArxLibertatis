/*
 * Copyright 2015 Arx Libertatis Team (see the AUTHORS file)
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
#include "game/Inventory.h"
#include "game/Item.h"
#include "game/Player.h"
#include "graphics/Color.h"
#include "graphics/Draw.h"
#include "graphics/data/TextureContainer.h"
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
		SpecialCursor=CURSOR_INTERACTION_ON;
		
		if(eeMouseDown1()) {
			// play un son que si un item est pris
			ARX_INVENTORY_TakeAllFromSecondaryInventory();
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
	
	if(m_isSelected) {
		SpecialCursor=CURSOR_INTERACTION_ON;
		
		if(eeMouseDown1()) {
			Entity * io = NULL;
			
			if(SecondaryInventory)
				io = SecondaryInventory->io;
			else if(player.Interface & INTER_STEAL)
				io = ioSteal;
			
			if(io) {
				ARX_SOUND_PlayInterface(SND_BACKPACK, Random::getf(0.9f, 1.1f));
				g_secondaryInventoryHud.m_fadeDirection = SecondaryInventoryHud::Fade_left;
				SendIOScriptEvent(io,SM_INVENTORY2_CLOSE);
				TSecondaryInventory=SecondaryInventory;
				SecondaryInventory=NULL;
			}
		}
		
		if(DRAGINTER == NULL)
			return;
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
	} else if(player.Interface & INTER_STEAL) {
		return ioSteal;
	}
	return NULL;
}

void SecondaryInventoryHud::update() {
	Entity * io = getSecondaryOrStealInvEntity();
	if(io) {
		float dist = fdist(io->pos, player.pos + (Vec3f_Y_AXIS * 80.f));
		
		float maxDist = player.m_telekinesis ? 900.f : 350.f;
		
		if(dist > maxDist) {
			if(m_fadeDirection != Fade_left) {
				ARX_SOUND_PlayInterface(SND_BACKPACK, Random::getf(0.9f, 1.1f));
				
				m_fadeDirection = Fade_left;
				SendIOScriptEvent(io,SM_INVENTORY2_CLOSE);
				TSecondaryInventory=SecondaryInventory;
				SecondaryInventory=NULL;
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
			//These have to be calculated on each frame (to make them move).
			Rectf parent = Rectf(Vec2f(m_fadePosition, 0), m_defaultBackground->m_size.x * m_scale, m_defaultBackground->m_size.y * m_scale);
			
			m_pickAllButton.setScale(m_scale);
			m_closeButton.setScale(m_scale);
			
			m_pickAllButton.update(parent);
			m_closeButton.update(parent);
		}
	}
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
	
	Rectf rect = Rectf(Vec2f(m_fadePosition * m_scale, 0.f), m_size.x * m_scale, m_size.y * m_scale);
	EERIEDrawBitmap(rect, 0.001f, ingame_inventory, Color::white);
	
	for(long y = 0; y < inventory->m_size.y; y++) {
		for(long x = 0; x < inventory->m_size.x; x++) {
			Entity *io = inventory->slot[x][y].io;
			if(!io)
				continue;
			
			bool bItemSteal = false;
			TextureContainer *tc = io->m_icon;
			TextureContainer *tc2 = NULL;
			
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
				
				Vec2f p = Vec2f(
				(m_fadePosition * m_scale) + (float)x*(32 * m_scale) + (2 * m_scale),
				(float)y*(32 * m_scale) + (13 * m_scale)
				);
				
				Vec2f size = Vec2f(tc->size());
				
				Color color = (io->poisonous && io->poisonous_count!=0) ? Color::green : Color::white;
				
				Rectf rect(p, size.x * m_scale, size.y * m_scale);
				// TODO use alpha blending so this will be anti-aliased even w/o alpha to coverage
				EERIEDrawBitmap(rect, 0.001f, tc, color);
				
				Color overlayColor = Color::black;
				
				if(!bItemSteal && (io==FlyingOverIO))
					overlayColor = Color::white;
				else if(!bItemSteal && (io->ioflags & IO_CAN_COMBINE)) {
					overlayColor = Color3f::gray(glm::abs(glm::cos(glm::radians(fDecPulse)))).to<u8>();
				}
				
				if(overlayColor != Color::black) {
					GRenderer->SetBlendFunc(BlendSrcAlpha, BlendOne);
					GRenderer->SetRenderState(Renderer::AlphaBlending, true);
					
					EERIEDrawBitmap(rect, 0.001f, tc, overlayColor);
					
					GRenderer->SetRenderState(Renderer::AlphaBlending, false);
				}
				
				if(tc2) {
					ARX_INTERFACE_HALO_Draw(io, tc, tc2, p, Vec2f(m_scale));
				}
				
				if((io->ioflags & IO_ITEM) && io->_itemdata->count != 1)
					ARX_INTERFACE_DrawNumber(p, io->_itemdata->count, 3, Color::white, m_scale);
			}
		}
	}
	
	
	if(!(player.Interface & INTER_COMBATMODE) && (player.Interface & INTER_MINIBACK)) {
		if(TSecondaryInventory) {
			
			Entity *temp = TSecondaryInventory->io;
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
		Vec2s t;
		t.x = pos.x + checked_range_cast<short>(m_fadePosition) - (2 * m_scale);
		t.y = pos.y - (13 * m_scale);
		t.x = t.x / (32 * m_scale);
		t.y = t.y / (32 * m_scale);
		
		if(t.x < 0 || t.x >= SecondaryInventory->m_size.x)
			return false;
		
		if(t.y < 0 || t.y >= SecondaryInventory->m_size.y)
			return false;
		
		return true;
	}
	
	return false;
}

extern long HERO_OR_SECONDARY;

Entity * SecondaryInventoryHud::getObj(const Vec2s & pos) {
	
	if(SecondaryInventory != NULL) {
		short tx = pos.x + checked_range_cast<short>(m_fadePosition) - (2 * m_scale);
		short ty = pos.y - (13 * m_scale);

		if(tx >= 0 && ty >= 0) {
			tx = tx / (32 * m_scale); 
			ty = ty / (32 * m_scale); 

			if(   tx >= 0
			   && tx <= SecondaryInventory->m_size.x
			   && ty >= 0
			   && ty <= SecondaryInventory->m_size.y
			) {
				if(SecondaryInventory->slot[tx][ty].io == NULL)
					return NULL;

				if(   (player.Interface & INTER_STEAL)
				   && !ARX_PLAYER_CanStealItem(SecondaryInventory->slot[tx][ty].io)
				) {
					return NULL;
				}

				Entity * io = SecondaryInventory->slot[tx][ty].io;

				if(!(io->gameFlags & GFLAG_INTERACTIVITY))
					return NULL;

				HERO_OR_SECONDARY = 2;
				return io;
			}
		}
	}
	
	return NULL;
}

void SecondaryInventoryHud::dropEntity() {
	
	// First Look for Identical Item...
	if(SecondaryInventory && g_secondaryInventoryHud.containsPos(DANAEMouse)) {
		Entity * io = SecondaryInventory->io;
		
		float fprice = ARX_INTERACTIVE_GetPrice(DRAGINTER, io) / 3.0f; //>>1;
		long price = checked_range_cast<long>(fprice);
		price *= DRAGINTER->_itemdata->count;
		fprice = price + price * player.m_skillFull.intuition * 0.005f;
		price = checked_range_cast<long>(fprice);
		
		// SHOP
		if(io->ioflags & IO_SHOP) {
			if(!io->shop_category.empty() && DRAGINTER->groups.find(io->shop_category) == DRAGINTER->groups.end())
				return;
			
			if(price <= 0)
				return;
			
			// Check shop group
			for(long j = 0; j < SecondaryInventory->m_size.y; j++) {
			for(long i = 0; i < SecondaryInventory->m_size.x; i++) {
				Entity * ioo = SecondaryInventory->slot[i][j].io;
				
				if(!ioo || !IsSameObject(DRAGINTER, ioo))
					continue;
				
				ioo->_itemdata->count += DRAGINTER->_itemdata->count;
				ioo->scale = 1.f;
				
				DRAGINTER->destroy();
				
				ARX_PLAYER_AddGold(price);
				ARX_SOUND_PlayInterface(SND_GOLD);
				ARX_SOUND_PlayInterface(SND_INVSTD);
				return;
			}
			}
		}
		
		Vec2s t = Vec2s_ZERO;
		t.x = DANAEMouse.x + static_cast<short>(m_fadePosition) - (2 * m_scale);
		t.y = DANAEMouse.y - (13 * m_scale);
		t.x = t.x / (32 * m_scale);
		t.y = t.y / (32 * m_scale);
		
		Vec2s s = DRAGINTER->m_inventorySize;
		
		if(t.x <= SecondaryInventory->m_size.x - s.x && t.y <= SecondaryInventory->m_size.y - s.y) {
			
			float fprice = ARX_INTERACTIVE_GetPrice(DRAGINTER, io) / 3.0f;
			long price = checked_range_cast<long>(fprice);
			price *= DRAGINTER->_itemdata->count;
			fprice = price + price * player.m_skillFull.intuition * 0.005f;
			price = checked_range_cast<long>(fprice);
			
			for(long j = 0; j < s.y; j++) {
			for(long i = 0; i < s.x; i++) {
				Entity * ioo = SecondaryInventory->slot[t.x+i][t.y+j].io;
				
				if(!ioo)
					continue;
				
				DRAGINTER->show = SHOW_FLAG_IN_INVENTORY;
				
				if(   ioo->_itemdata->playerstacksize > 1
				   && IsSameObject(DRAGINTER, ioo)
				   && ioo->_itemdata->count < ioo->_itemdata->playerstacksize
				) {
					ioo->_itemdata->count += DRAGINTER->_itemdata->count;
					
					if(ioo->_itemdata->count > ioo->_itemdata->playerstacksize) {
						DRAGINTER->_itemdata->count = ioo->_itemdata->count - ioo->_itemdata->playerstacksize;
						ioo->_itemdata->count = ioo->_itemdata->playerstacksize;
					} else {
						DRAGINTER->_itemdata->count = 0;
					}
				}
				
				if(DRAGINTER->_itemdata->count) {
					if(CanBePutInSecondaryInventory(SecondaryInventory, DRAGINTER)) {
						// SHOP
						if(io->ioflags & IO_SHOP) {
							ARX_PLAYER_AddGold(price);
							ARX_SOUND_PlayInterface(SND_GOLD);
						}
					} else {
						return;
					}
				}
				
				ARX_SOUND_PlayInterface(SND_INVSTD);
				Set_DragInter(NULL);
				return;
			}
			}
			
			if(DRAGINTER->ioflags & IO_GOLD) {
				ARX_PLAYER_AddGold(DRAGINTER);
				Set_DragInter(NULL);
				return;
			}

			for(long j = 0; j < s.y; j++) {
			for(long i = 0; i < s.x; i++) {
				SecondaryInventory->slot[t.x+i][t.y+j].io = DRAGINTER;
				SecondaryInventory->slot[t.x+i][t.y+j].show = false;
			}
			}
			
			// SHOP
			if(io->ioflags & IO_SHOP) {
				player.gold += price;
				ARX_SOUND_PlayInterface(SND_GOLD);
			}

			SecondaryInventory->slot[t.x][t.y].show = true;
			DRAGINTER->show = SHOW_FLAG_IN_INVENTORY;
			ARX_SOUND_PlayInterface(SND_INVSTD);
			Set_DragInter(NULL);
			return;
		}
	}
}

// TODO global sInventory
extern short sInventory;
extern Vec2s sInventoryPos;

bool SecondaryInventoryHud::dragEntity(Entity * io, const Vec2s & pos) {
	
	if(SecondaryInventory != NULL) {
		if(g_secondaryInventoryHud.containsPos(pos) && (io->ioflags & IO_ITEM)) {
			Entity * ioo = SecondaryInventory->io;
			
			if(ioo->ioflags & IO_SHOP) {
				long cos = ARX_INTERACTIVE_GetPrice(io, ioo);
				
				float fcos	= cos - cos * player.m_skillFull.intuition * 0.005f;
				cos = checked_range_cast<long>(fcos);
				
				if(player.gold < cos) {
					return false;
				}
				
				ARX_SOUND_PlayInterface(SND_GOLD);
				player.gold -= cos;
				
				if(io->_itemdata->count > 1) {
					Entity * ioo = CloneIOItem(io);
					ioo->show = SHOW_FLAG_NOT_DRAWN;
					ioo->scriptload = 1;
					ioo->_itemdata->count = 1;
					io->_itemdata->count--;
					ARX_SOUND_PlayInterface(SND_INVSTD);
					Set_DragInter(ioo);
					return true;
				}
			} else if(io->_itemdata->count > 1) {
				
				if(!GInput->actionPressed(CONTROLS_CUST_STEALTHMODE)) {
					Entity * ioo = CloneIOItem(io);
					ioo->show = SHOW_FLAG_NOT_DRAWN;
					ioo->scriptload = 1;
					ioo->_itemdata->count = 1;
					io->_itemdata->count--;
					ARX_SOUND_PlayInterface(SND_INVSTD);
					Set_DragInter(ioo);
					sInventory = 2;
					
					Vec2f calc;
					calc.x = (pos.x + m_fadePosition - (2 * m_scale)) / (32 * m_scale);
					calc.y = (pos.y - (13 * m_scale)) / (32 * m_scale);
					
					sInventoryPos.x = checked_range_cast<short>(calc.x);
					sInventoryPos.y = checked_range_cast<short>(calc.y);
					
					//ARX_INVENTORY_Object_Out(SecondaryInventory->io, ioo);
					
					ARX_INVENTORY_IdentifyIO(ioo);
					return true;
				}
			}
		}
		
		for(long j = 0; j < SecondaryInventory->m_size.y; j++)
		for(long i = 0; i < SecondaryInventory->m_size.x; i++) {
			INVENTORY_SLOT & slot = SecondaryInventory->slot[i][j];
			
			if(slot.io != io)
				continue;
			
			slot.io = NULL;
			slot.show = true;
			sInventory = 2;
			
			float fCalcX = (pos.x + m_fadePosition - (2 * m_scale)) / (32 * m_scale);
			float fCalcY = (pos.y - (13 * m_scale)) / (32 * m_scale);
			
			sInventoryPos.x = checked_range_cast<short>(fCalcX);
			sInventoryPos.y = checked_range_cast<short>(fCalcY);
		}
	}
	
	Set_DragInter(io);
	RemoveFromAllInventories(io);
	ARX_INVENTORY_IdentifyIO(io);
	return true;
}

void SecondaryInventoryHud::close() {
	
	Entity * io = NULL;
	
	if(SecondaryInventory)
		io = SecondaryInventory->io;
	else if(player.Interface & INTER_STEAL)
		io = ioSteal;
	
	if(io) {
		m_fadeDirection = Fade_left;
		SendIOScriptEvent(io, SM_INVENTORY2_CLOSE);
		TSecondaryInventory = SecondaryInventory;
		SecondaryInventory = NULL;
	}
}

void SecondaryInventoryHud::updateFader() {
	
	if(m_fadeDirection != Fade_stable) {
		if((player.Interface & INTER_COMBATMODE) || player.doingmagic >= 2 || m_fadeDirection == Fade_left) {
			if(m_fadePosition > -160)
				m_fadePosition -= (g_framedelay * ( 1.0f / 3 )) * m_scale;
		} else {
			if(m_fadePosition < 0)
				m_fadePosition += m_fadeDirection * (g_framedelay * ( 1.0f / 3 )) * m_scale;
		}
		
		if(m_fadePosition <= -160) {
			m_fadePosition = -160;
			m_fadeDirection = Fade_stable;
			
			if(player.Interface & INTER_STEAL || ioSteal) {
				SendIOScriptEvent(ioSteal, SM_STEAL, "off");
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
