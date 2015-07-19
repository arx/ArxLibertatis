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
#include "scene/GameSound.h"
#include "scene/Interactive.h"


float InventoryDir = 0; // 0 stable, 1 to right, -1 to left
TextureContainer * BasicInventorySkin = NULL;
float InventoryX = -60.f;

SecondaryInventoryHud g_secondaryInventoryHud;


void PickAllIconGui::init() {
	m_tex = TextureContainer::LoadUI("graph/interface/inventory/inv_pick");
	arx_assert(m_tex);
	
	m_size = Vec2f(16, 16);
}

void PickAllIconGui::update() {
	Rectf parent = Rectf(Vec2f(InventoryX, 0), BasicInventorySkin->m_dwWidth, BasicInventorySkin->m_dwHeight);
	
	Rectf spacer = createChild(parent, Anchor_BottomLeft, Vec2f(16, 16), Anchor_BottomLeft);
	
	m_rect = createChild(spacer, Anchor_BottomRight, m_size, Anchor_BottomLeft);
}

void PickAllIconGui::updateInput() {
	
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


void CloseSecondaryInventoryIconGui::init() {
	m_tex = TextureContainer::LoadUI("graph/interface/inventory/inv_close");
	arx_assert(m_tex);
	
	m_size = Vec2f(16, 16);
}

void CloseSecondaryInventoryIconGui::update() {
	Rectf parent = Rectf(Vec2f(InventoryX, 0), BasicInventorySkin->m_dwWidth, BasicInventorySkin->m_dwHeight);
	
	Rectf spacer = createChild(parent, Anchor_BottomRight, Vec2f(16, 16), Anchor_BottomRight);
	
	m_rect = createChild(spacer, Anchor_BottomLeft, m_size, Anchor_BottomRight);
}

void CloseSecondaryInventoryIconGui::updateInput() {
	
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
				ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
				InventoryDir=-1;
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
	
	pickAllIconGui.init();
	closeSecondaryInventoryIconGui.init();
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
			if(InventoryDir != -1) {
				ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
				
				InventoryDir=-1;
				SendIOScriptEvent(io,SM_INVENTORY2_CLOSE);
				TSecondaryInventory=SecondaryInventory;
				SecondaryInventory=NULL;
			} else {
				if(player.Interface & INTER_STEAL) {
					player.Interface &= ~INTER_STEAL;
				}
			}
		}
	} else if(InventoryDir != -1) {
		InventoryDir = -1;
	}
	
	
	if(!(player.Interface & INTER_COMBATMODE) && (player.Interface & INTER_MINIBACK)) {
		// Pick All/Close Secondary Inventory
		if(TSecondaryInventory) {
			//These have to be calculated on each frame (to make them move).
			pickAllIconGui.update();
			closeSecondaryInventoryIconGui.update();
		}
	}
}

void SecondaryInventoryHud::draw() {
	const INVENTORY_DATA * inventory = TSecondaryInventory;
	
	if(!inventory)
		return;
	
	bool _bSteal = (bool)((player.Interface & INTER_STEAL) != 0);
	
	arx_assert(BasicInventorySkin);
	ingame_inventory = BasicInventorySkin;
	if(inventory->io && !inventory->io->inventory_skin.empty()) {
		
		res::path file = "graph/interface/inventory" / inventory->io->inventory_skin;
		TextureContainer * tc = TextureContainer::LoadUI(file);
		if(tc)
			ingame_inventory = tc;
	}
	
	Rectf rect = Rectf(Vec2f(INTERFACE_RATIO(InventoryX), 0.f), m_size.x, m_size.y);
	EERIEDrawBitmap(rect, 0.001f, ingame_inventory, Color::white);
	
	for(long y = 0; y < inventory->m_size.y; y++) {
		for(long x = 0; x < inventory->m_size.x; x++) {
			Entity *io = inventory->slot[x][y].io;
			if(!io)
				continue;
			
			bool bItemSteal = false;
			TextureContainer *tc = io->inv;
			TextureContainer *tc2 = NULL;
			
			if(NeedHalo(io))
				tc2 = io->inv->getHalo();
			
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
				INTERFACE_RATIO(InventoryX) + (float)x*INTERFACE_RATIO(32) + INTERFACE_RATIO(2),
				(float)y*INTERFACE_RATIO(32) + INTERFACE_RATIO(13)
				);
				
				Vec2f size = Vec2f(tc->size());
				
				Color color = (io->poisonous && io->poisonous_count!=0) ? Color::green : Color::white;
				
				Rectf rect(p, size.x, size.y);
				// TODO use alpha blending so this will be anti-aliased even w/o alpha to coverage
				EERIEDrawBitmap(rect, 0.001f, tc, color);
				
				Color overlayColor = Color::black;
				
				if(!bItemSteal && (io==FlyingOverIO))
					overlayColor = Color::white;
				else if(!bItemSteal && (io->ioflags & IO_CAN_COMBINE)) {
					overlayColor = Color3f::gray(glm::abs(glm::cos(glm::radians(fDecPulse)))).to<u8>();
				}
				
				if(overlayColor != Color::black) {
					GRenderer->SetBlendFunc(Renderer::BlendSrcAlpha, Renderer::BlendOne);
					GRenderer->SetRenderState(Renderer::AlphaBlending, true);
					
					EERIEDrawBitmap(rect, 0.001f, tc, overlayColor);
					
					GRenderer->SetRenderState(Renderer::AlphaBlending, false);
				}
				
				if(tc2) {
					ARX_INTERFACE_HALO_Draw(io, tc, tc2, p);
				}
				
				if((io->ioflags & IO_ITEM) && io->_itemdata->count != 1)
					ARX_INTERFACE_DrawNumber(p, io->_itemdata->count, 3, Color::white, 1.f);
			}
		}
	}
	
	
	if(!(player.Interface & INTER_COMBATMODE) && (player.Interface & INTER_MINIBACK)) {
		if(TSecondaryInventory) {
			
			Entity *temp = TSecondaryInventory->io;
			if(temp && !(temp->ioflags & IO_SHOP) && !(temp == ioSteal)) {
				pickAllIconGui.draw();
			}
			closeSecondaryInventoryIconGui.draw();
		}
	}
}

void SecondaryInventoryHud::updateInputButtons() {
	
	if(TSecondaryInventory) {
		
		Entity * temp = TSecondaryInventory->io;
		
		if(temp && !(temp->ioflags & IO_SHOP) && !(temp == ioSteal)) {
			pickAllIconGui.updateInput();
		}
		
		closeSecondaryInventoryIconGui.updateInput();
	}
}
