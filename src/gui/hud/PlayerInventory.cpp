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

#include "gui/hud/PlayerInventory.h"

#include "core/Application.h"
#include "core/Config.h"
#include "core/Core.h"
#include "core/GameTime.h"
#include "game/Inventory.h"
#include "game/Item.h"
#include "game/Player.h"
#include "graphics/Draw.h"
#include "graphics/data/TextureContainer.h"
#include "gui/Cursor.h"
#include "gui/Hud.h"
#include "gui/Interface.h"
#include "input/Input.h"
#include "scene/GameSound.h"
#include "scene/Interactive.h"

bool bInventorySwitch = false;
float fDecPulse;

extern PlayerInterfaceFlags lOldInterface;

void PlayerInventoryHud::init() {
	m_heroInventory = TextureContainer::LoadUI("graph/interface/inventory/hero_inventory");
	m_heroInventoryLink = TextureContainer::LoadUI("graph/interface/inventory/hero_inventory_link");
	m_heroInventoryUp = TextureContainer::LoadUI("graph/interface/inventory/scroll_up");
	m_heroInventoryDown = TextureContainer::LoadUI("graph/interface/inventory/scroll_down");
	arx_assert(m_heroInventory);
	arx_assert(m_heroInventoryLink);
	arx_assert(m_heroInventoryUp);
	arx_assert(m_heroInventoryDown);
	
	m_slotSize = Vec2f(32, 32);
	m_slotSpacing = Vec2f(7, 6);
	
	m_bagSize = Vec2f(562, 121);
	
	m_isClosing = false;
	m_inventoryY = 110;
}


Vec2f PlayerInventoryHud::anchorPosition() {
	
	return Vec2f(g_size.center().x - (320 * m_scale) + (35 * m_scale) ,
	g_size.height() - (101 * m_scale) + (m_inventoryY * m_scale));
}

void PlayerInventoryHud::updateRect(){
	
	Vec2f anchorPos = anchorPosition();
	
	if(player.Interface & INTER_INVENTORYALL) {
		m_rect = Rectf(anchorPos - Vec2f(0, (player.m_bags - 1) * m_bagSize.y * m_scale),
		                                 m_bagSize.x * m_scale, player.m_bags * m_bagSize.y * m_scale);
	} else {
		m_rect = Rectf(anchorPos, m_bagSize.x * m_scale, m_bagSize.y * m_scale);
	}
	
}

bool PlayerInventoryHud::updateInput() {
	
	Vec2f anchorPos = anchorPosition();
	Vec2f pos = anchorPos + Vec2f((m_bagSize.x * m_scale) - ((32 + 3) * m_scale), ((-3 + 25) * m_scale));
	
	bool bQuitCombine = true;
	
	if(m_currentBag > 0) {
		
		const Rect mouseTestRect(Vec2i(pos), int(32 * m_scale), int(32 * m_scale));
		if(mouseTestRect.contains(Vec2i(DANAEMouse))) {
			bQuitCombine = false;
		}
		
	}

	if(m_currentBag < player.m_bags - 1) {
		
		float fRatio = (32 + 5) * m_scale;
		pos.y += checked_range_cast<int>(fRatio);
		
		const Rect mouseTestRect(Vec2i(pos), int(32 * m_scale), int(32 * m_scale));
		if(mouseTestRect.contains(Vec2i(DANAEMouse))) {
			bQuitCombine = false;
		}
		
	}
	
	return bQuitCombine;
}

void PlayerInventoryHud::update() {
	
	const float framedelay = toMs(g_platformTime.lastFrameDuration());
	
	if(player.Interface & INTER_INVENTORY) {
		
		long t = long(framedelay * 0.2f + 2.0f);
		if((player.Interface & INTER_COMBATMODE) || player.doingmagic >= 2) {
			
			m_inventoryY += t;
			
			if(m_inventoryY > 110) {
				m_inventoryY = 110;
			}
		} else {
			if(m_isClosing) {
				m_inventoryY += t;
				
				if(m_inventoryY > 110) {
					m_inventoryY = 110;
					m_isClosing = false;
					
					player.Interface &= ~INTER_INVENTORY;
					
					if(bInventorySwitch) {
						bInventorySwitch = false;
						ARX_SOUND_PlayInterface(g_snd.BACKPACK, Random::getf(0.9f, 1.1f));
						player.Interface |= INTER_INVENTORYALL;
						ARX_INTERFACE_NoteClose();
						m_inventoryY = 121 * player.m_bags;
						lOldInterface = INTER_INVENTORYALL;
					}
				}
			} else if(m_inventoryY > 0) {
				m_inventoryY -= t;
				
				if(m_inventoryY < 0) {
					m_inventoryY = 0;
				}
			}
		}
		
	} else if((player.Interface & INTER_INVENTORYALL) || isClosing()) {
		
		float fSpeed = 1.f / 3;
		long t = long((framedelay * fSpeed) + 2.f);
		if((player.Interface & INTER_COMBATMODE) || player.doingmagic >= 2) {
			if(m_inventoryY < 121 * player.m_bags) {
				m_inventoryY += t;
			}
		} else {
			if(m_isClosing) {
				m_inventoryY += t;
				if(m_inventoryY > 121 * player.m_bags) {
					m_isClosing = false;
					if(player.Interface & INTER_INVENTORYALL) {
						player.Interface &= ~INTER_INVENTORYALL;
					}
					lOldInterface = 0;
				}
			} else if(m_inventoryY > 0) {
				m_inventoryY -= t;
				if(m_inventoryY < 0) {
					m_inventoryY = 0;
				}
			}
		}
		
	}
}

void PlayerInventoryHud::CalculateInventoryCoordinates() {
	
	
	Vec2f anchorPos = anchorPosition();
	
	m_arrowsAnchor.x = anchorPos.x + (m_bagSize.x * m_scale) - ((32 + 3)  * m_scale);
	m_arrowsAnchor.y = anchorPos.y + ((-3 + 25) * m_scale);
}

//-----------------------------------------------------------------------------
void PlayerInventoryHud::drawBag(size_t bag, Vec2i i)
{
	fDecPulse += toMs(g_platformTime.lastFrameDuration()) * 0.5f;
	
	Vec2f anchorPos = anchorPosition();
	
	const Vec2f pos = anchorPos + Vec2f(i.x, i.y);
	
	{
		Rectf rect = Rectf(pos + Vec2f(0.f, -(5 * m_scale)), m_bagSize.x * m_scale, m_bagSize.y * m_scale);
		EERIEDrawBitmap(rect, 0.001f, m_heroInventory, Color::white);
	}
	
	for(size_t y = 0; y < INVENTORY_Y; y++) {
	for(size_t x = 0; x < INVENTORY_X; x++) {
		
		Entity * io = g_inventory[bag][x][y].io;
		if(!io || !g_inventory[bag][x][y].show) {
			continue;
		}
		
		TextureContainer * tc = io->m_icon;
		TextureContainer * tc2 = NULL;
		
		if(NeedHalo(io)) {
			tc2 = io->m_icon->getHalo();
		}
		
		if(!tc) {
			continue;
		}
		
		const Vec2f p = pos + Vec2f(x, y) * (m_slotSize * m_scale) + (m_slotSpacing * m_scale);
		
		Color color = (io->poisonous && io->poisonous_count != 0) ? Color::green : Color::white;
		
		Rectf rect(p, tc->m_size.x * m_scale, tc->m_size.y * m_scale);
		
		if(tc2) {
			ARX_INTERFACE_HALO_Render(io->halo.color, io->halo.flags, tc2, p, Vec2f(m_scale));
		}
		
		EERIEDrawBitmap(rect, 0.001f, tc, color);
		
		Color overlayColor = Color::black;
		
		if(io == FlyingOverIO)
			overlayColor = Color::white;
		else if(io->ioflags & IO_CAN_COMBINE)
			overlayColor = Color::gray(glm::abs(glm::cos(glm::radians(fDecPulse))));
		
		
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

void PlayerInventoryHud::draw() {
	if(player.Interface & INTER_INVENTORY) {
		if(player.m_bags) {
			drawBag(m_currentBag, Vec2i(0));
			
			CalculateInventoryCoordinates();
			
			if(m_currentBag > 0) {
				Rectf rect = Rectf(m_arrowsAnchor, 32.f * m_scale, 32.f * m_scale);
				
				EERIEDrawBitmap(rect, 0.001f, m_heroInventoryUp, Color::white);
				
				if(rect.contains(Vec2f(DANAEMouse))) {
					
					cursorSetInteraction();
					
					UseRenderState state(render2D().blendAdditive());
					EERIEDrawBitmap(rect, 0.001f, m_heroInventoryUp, Color::white);
					
					cursorSetInteraction();
					
					if(eeMouseUp1()) {
						previousBag();
					}
				}
			}
			
			if(m_currentBag < player.m_bags - 1) {
				
				Rectf rect = Rectf(m_arrowsAnchor + Vec2f(0.f, 32.f + 5.f) * m_scale, 32.f * m_scale, 32.f * m_scale);
				
				EERIEDrawBitmap(rect, 0.001f, m_heroInventoryDown, Color::white);
				
				if(rect.contains(Vec2f(DANAEMouse))) {
					
					UseRenderState state(render2D().blendAdditive());
					EERIEDrawBitmap(rect, 0.001f, m_heroInventoryDown, Color::white);
					
					cursorSetInteraction();
					
					if(eeMouseUp1()) {
						nextBag();
					}
				}
				
			}
		}
	} else if((player.Interface & INTER_INVENTORYALL) || m_isClosing) {
		
		Vec2f anchorPos = anchorPosition();
		
		// TODO see about these coords, might be calculated once only
		const float fBag = (player.m_bags - 1) * (-121.f * m_scale);
		const float fOffsetY = (121 * m_scale);
		
		int iOffsetY = checked_range_cast<int>(fBag + fOffsetY);
		int posx = checked_range_cast<int>(anchorPos.x);
		int posy = checked_range_cast<int>(anchorPos.y + ((-3.f + 25 - 32) * m_scale));
		
		for(int i = 0; i < player.m_bags; i++) {
			
			float linkPosY = float(posy + iOffsetY);
			Vec2f pos1 = Vec2f(posx + (45 * m_scale), linkPosY);
			Vec2f pos2 = Vec2f(posx + (m_bagSize.x * m_scale) * 0.5f + (-16.f * m_scale), linkPosY);
			Vec2f pos3 = Vec2f(posx + (m_bagSize.x * m_scale) + (-77.f * m_scale), linkPosY);
			
			TextureContainer * tex = m_heroInventoryLink;
			Vec2f texSize = Vec2f(tex->m_size) * m_scale;
			
			EERIEDrawBitmap(Rectf(pos1, texSize.x, texSize.y), 0.001f, tex, Color::white);
			EERIEDrawBitmap(Rectf(pos2, texSize.x, texSize.y), 0.001f, tex, Color::white);
			EERIEDrawBitmap(Rectf(pos3, texSize.x, texSize.y), 0.001f, tex, Color::white);
			
			iOffsetY += checked_range_cast<int>(fOffsetY);
		}
		
		iOffsetY = checked_range_cast<int>(fBag);
		
		for(short i = 0; i < player.m_bags; i++) {
			drawBag(i, Vec2i(0, iOffsetY));
			iOffsetY += checked_range_cast<int>(fOffsetY);
		}
	}
}

void PlayerInventoryHud::nextBag() {
	
	if((player.Interface & INTER_INVENTORY) && player.m_bags != 0 && m_currentBag < player.m_bags - 1) {
		ARX_SOUND_PlayInterface(g_snd.BACKPACK, Random::getf(0.9f, 1.1f));
		m_currentBag++;
	}
	
}

void PlayerInventoryHud::previousBag() {
	
	if((player.Interface & INTER_INVENTORY) && player.m_bags != 0 && m_currentBag > 0) {
		ARX_SOUND_PlayInterface(g_snd.BACKPACK, Random::getf(0.9f, 1.1f));
		m_currentBag--;
	}
	
}

PlayerInventoryHud g_playerInventoryHud;

bool PlayerInventoryHud::InPlayerInventoryBag(const Vec2s & pos) {
	
	if(pos.x >= 0 && pos.y >= 0) {
		Vec2s t(s16(pos.x / (32 * m_scale)), s16((pos.y + 5 * m_scale) / (32 * m_scale)));
		if(t.x >= 0 && size_t(t.x) <= INVENTORY_X && t.y >= 0 && size_t(t.y) <= INVENTORY_Y) {
			return true;
		}
	}
	
	return false;
}

/*!
 * \brief Returns true if xx,yy is a position in player inventory
 */
bool PlayerInventoryHud::containsPos(const Vec2s & pos) {
	Vec2f anchorPos = anchorPosition();
	
	Vec2s iPos = Vec2s(anchorPos);

	if(player.Interface & INTER_INVENTORY) {
		Vec2s t = pos - iPos;
		
		return InPlayerInventoryBag(t);
	}
	
	if(player.Interface & INTER_INVENTORYALL) {
		float fBag = (player.m_bags - 1) * (-121 * m_scale);

		short iY = checked_range_cast<short>(fBag);

		if(   pos.x >= iPos.x
		   && pos.x <= iPos.x + INVENTORY_X * (32 * m_scale)
		   && pos.y >= iPos.y + iY
		   && pos.y <= g_size.height()
		) {
			return true;
		}

		for(int i = 0; i < player.m_bags; i++) {
			Vec2s t = pos - iPos;
			t.y -= iY;
			
			if(InPlayerInventoryBag(t))
				return true;
			
			float fRatio = (121 * m_scale);
			
			iY = checked_range_cast<short>(iY + fRatio);
		}
	}

	return false;
}

Entity * PlayerInventoryHud::getObj(const Vec2s & pos) {
	
	Vec2f anchorPos = anchorPosition();
	
	Vec2i iPos = Vec2i(anchorPos);
	
	if(player.Interface & INTER_INVENTORY) {
		
		long tx = pos.x - iPos.x;
		long ty = pos.y - iPos.y;
		if(tx >= 0 && ty >= 0) {
			tx = checked_range_cast<long>((tx - 6 * m_scale) / (32 * m_scale));
			ty = checked_range_cast<long>((ty - 5 * m_scale) / (32 * m_scale));
			if(tx >= 0 && size_t(tx) < INVENTORY_X && ty >= 0 && size_t(ty) < INVENTORY_Y) {
				Entity * result = g_inventory[m_currentBag][tx][ty].io;
				if(result && (result->gameFlags & GFLAG_INTERACTIVITY)) {
					return result;
				}
			}
			return NULL;
		}
		
	} else if(player.Interface & INTER_INVENTORYALL) {
		
		float fBag = (player.m_bags - 1) * (-121 * m_scale);
		
		int iY = checked_range_cast<int>(fBag);

		for(size_t bag = 0; bag < size_t(player.m_bags); bag++) {
			
			long tx = pos.x - iPos.x;
			long ty = pos.y - iPos.y - iY;
			tx = checked_range_cast<long>((tx - 6 * m_scale) / (32 * m_scale));
			ty = checked_range_cast<long>((ty - 5 * m_scale) / (32 * m_scale));
			
			if(tx >= 0 && size_t(tx) < INVENTORY_X && ty >= 0 && size_t(ty) < INVENTORY_Y) {
				Entity * result = g_inventory[bag][tx][ty].io;
				if(result && (result->gameFlags & GFLAG_INTERACTIVITY)) {
					return result;
				}
				return NULL;
			}
			
			iY += checked_range_cast<int>((121 * m_scale));
			
		}
	}

	return NULL;
}


void PlayerInventoryHud::dropEntity() {
	if(!(player.Interface & INTER_INVENTORY) && !(player.Interface & INTER_INVENTORYALL))
		return;
	
	if(m_inventoryY != 0)
		return;
	
	if(!g_playerInventoryHud.containsPos(DANAEMouse))
		return;
	
	// If inventories overlap entity might have been dropped alreadry
	if(!DRAGINTER) {
		return;
	}
	
	Vec2s itemSize = DRAGINTER->m_inventorySize;
	
	int bag = 0;
	
	Vec2f anchorPos = g_playerInventoryHud.anchorPosition();
	Vec2s iPos = Vec2s(anchorPos);
	
	Vec2s t(0);
	
	if(player.Interface & INTER_INVENTORY) {
		t.x = DANAEMouse.x - iPos.x;
		t.y = DANAEMouse.y - iPos.y;
		t.x = s16(t.x / (32 * m_scale));
		t.y = s16(t.y / (32 * m_scale));
		
		if((t.x >= 0) && (t.x <= 16 - itemSize.x) && (t.y >= 0) && (t.y <= 3 - itemSize.y)) {
			bag = m_currentBag;
		} else {
			return;
		}
	} else {
		bool bOk = false;
		
		float fBag = (player.m_bags - 1) * (-121 * m_scale);
		
		short iY = checked_range_cast<short>(fBag);
		
		// We must enter the for-loop to initialyze tx/ty
		arx_assert(0 < player.m_bags);
		
		for(int i = 0; i < player.m_bags; i++) {
			t.x = DANAEMouse.x - iPos.x;
			t.y = DANAEMouse.y - iPos.y - iY;
			
			if((t.x >= 0) && (t.y >= 0)) {
				t.x = s16(t.x / (32 * m_scale));
				t.y = s16(t.y / (32 * m_scale));
				
				if((t.x >= 0) && (t.x <= 16 - itemSize.x) && (t.y >= 0) && (t.y <= 3 - itemSize.y)) {
					bOk = true;
					bag = i;
					break;
				}
			}
			
			float fRatio = (121 * m_scale);
			
			iY += checked_range_cast<short>(fRatio);
		}
		
		if(!bOk)
			return;
	}
	
	if(DRAGINTER->ioflags & IO_GOLD) {
		ARX_PLAYER_AddGold(DRAGINTER);
		Set_DragInter(NULL);
		return;
	}
	
	for(long y = 0; y < itemSize.y; y++)
	for(long x = 0; x < itemSize.x; x++) {
		Entity * ioo = g_inventory[bag][t.x + x][t.y + y].io;
		
		if(!ioo)
			continue;
		
		ARX_INVENTORY_IdentifyIO(ioo);
		
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
			
			ioo->scale = 1.f;
			ARX_INVENTORY_Declare_InventoryIn(DRAGINTER);
			
			if(!DRAGINTER->_itemdata->count) {
				DRAGINTER->destroy();
			}
			
			ARX_SOUND_PlayInterface(g_snd.INVSTD);
			return;
		}
		
		return;
	}
	
	for(long y = 0; y < itemSize.y; y++) {
	for(long x = 0; x < itemSize.x; x++) {
		g_inventory[bag][t.x + x][t.y + y].io = DRAGINTER;
		g_inventory[bag][t.x + x][t.y + y].show = false;
	}
	}
	
	g_inventory[bag][t.x][t.y].show = true;
	
	ARX_INVENTORY_Declare_InventoryIn(DRAGINTER);
	ARX_SOUND_PlayInterface(g_snd.INVSTD);
	DRAGINTER->show = SHOW_FLAG_IN_INVENTORY;
	Set_DragInter(NULL);
}

// TODO global sInventory
extern short sInventory;
extern Vec2s sInventoryPos;

void PlayerInventoryHud::dragEntity(Entity * io, const Vec2s & pos) {
	
	Vec2s iPos = Vec2s(g_playerInventoryHud.anchorPosition());
	
	if(g_playerInventoryHud.containsPos(pos)) {
		if(!GInput->actionPressed(CONTROLS_CUST_STEALTHMODE)) {
			if((io->ioflags & IO_ITEM) && io->_itemdata->count > 1) {
				if(io->_itemdata->count - 1 > 0) {
					
					Entity * ioo = AddItem(io->classPath());
					ioo->show = SHOW_FLAG_NOT_DRAWN;
					ioo->_itemdata->count = 1;
					io->_itemdata->count--;
					ioo->scriptload = 1;
					ARX_SOUND_PlayInterface(g_snd.INVSTD);
					Set_DragInter(ioo);
					RemoveFromAllInventories(ioo);
					sInventory = 1;
					sInventoryPos = (pos - iPos) / s16(32.f * m_scale);
					
					SendInitScriptEvent(ioo);
					ARX_INVENTORY_IdentifyIO(ioo);
					return;
				}
			}
		}
	}
	
	arx_assert(player.m_bags >= 0);
	arx_assert(player.m_bags <= 3);
	
	for(size_t bag = 0; bag < size_t(player.m_bags); bag++)
	for(size_t y = 0; y < INVENTORY_Y; y++)
	for(size_t x = 0; x < INVENTORY_X; x++) {
		INVENTORY_SLOT & slot = g_inventory[bag][x][y];
		
		if(slot.io == io) {
			slot.io = NULL;
			slot.show = true;
			sInventory = 1;
			sInventoryPos = (pos - iPos) / s16(32.f * m_scale);
		}
	}
	
	Set_DragInter(io);
	
	RemoveFromAllInventories(io);
	ARX_INVENTORY_IdentifyIO(io);
}

void PlayerInventoryHud::close() {
	m_isClosing = true;
}

bool PlayerInventoryHud::isClosing() {
	return m_isClosing;
}

void PlayerInventoryHud::resetPos() {
	if(player.Interface & INTER_INVENTORY) {
		m_inventoryY = 110;
	} else if(player.Interface & INTER_INVENTORYALL) {
		m_inventoryY = 121 * player.m_bags;
	}
}

void PlayerInventoryHud::setCurrentBag(short bag) {
	if(bag < player.m_bags) {
		m_currentBag = bag;
	}
}
