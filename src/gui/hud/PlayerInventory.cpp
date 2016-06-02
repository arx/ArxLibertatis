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

#include "gui/hud/PlayerInventory.h"

#include "core/Application.h"
#include "core/Config.h"
#include "core/Core.h"
#include "game/Inventory.h"
#include "game/Item.h"
#include "game/Player.h"
#include "graphics/Draw.h"
#include "graphics/data/TextureContainer.h"
#include "gui/Hud.h"
#include "gui/Interface.h"
#include "input/Input.h"
#include "scene/GameSound.h"
#include "scene/Interactive.h"

bool bInventorySwitch = false;
float fDecPulse;
short g_currentInventoryBag = 0;

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
	
	m_bagBackgroundSize = Vec2f(562, 121);
}


Vec2f PlayerInventoryHud::anchorPosition() {
	
	return Vec2f(g_size.center().x - (320 * m_scale) + (35 * m_scale) ,
	g_size.height() - (101 * m_scale) + (InventoryY * m_scale));
}

bool PlayerInventoryHud::updateInput() {
	Vec2f anchorPos = anchorPosition();
	
	Vec2f pos = anchorPos + Vec2f((m_bagBackgroundSize.x * m_scale) - ((32 + 3) * m_scale), ((-3 + 25) * m_scale));
	
	bool bQuitCombine = true;
	
	if(g_currentInventoryBag > 0) {
		const Rect mouseTestRect(Vec2i(pos), int(32 * m_scale), int(32 * m_scale));
		
		if(mouseTestRect.contains(Vec2i(DANAEMouse)))
			bQuitCombine = false;
	}

	if(g_currentInventoryBag < player.bag-1) {
		float fRatio = (32 + 5) * m_scale;

		pos.y += checked_range_cast<int>(fRatio);
		
		const Rect mouseTestRect(Vec2i(pos), int(32 * m_scale), int(32 * m_scale));
		
		if(mouseTestRect.contains(Vec2i(DANAEMouse)))
			bQuitCombine = false;
	}
	
	return bQuitCombine;
}

void PlayerInventoryHud::update() {
	if(player.Interface & INTER_INVENTORY) {
		if((player.Interface & INTER_COMBATMODE) || player.doingmagic >= 2) {
			long t = Original_framedelay * (1.f/5) + 2;
			InventoryY += t;

			if(InventoryY > 110) {
				InventoryY = 110;
			}
		} else {
			if(bInventoryClosing) {
				long t = Original_framedelay * (1.f/5) + 2;
				InventoryY += t;

				if(InventoryY > 110) {
					InventoryY = 110;
					bInventoryClosing = false;

					player.Interface &=~ INTER_INVENTORY;

					if(bInventorySwitch) {
						bInventorySwitch = false;
						ARX_SOUND_PlayInterface(SND_BACKPACK, Random::getf(0.9f, 1.1f));
						player.Interface |= INTER_INVENTORYALL;
						ARX_INTERFACE_NoteClose();
						InventoryY = 121 * player.bag;
						lOldInterface=INTER_INVENTORYALL;
					}
				}
			} else if(InventoryY > 0) {
				InventoryY -= long((Original_framedelay * (1.f/5)) + 2.f);

				if(InventoryY < 0) {
					InventoryY = 0;
				}
			}
		}
	} else if((player.Interface & INTER_INVENTORYALL) || bInventoryClosing) {
		float fSpeed = (1.f/3);
		if((player.Interface & INTER_COMBATMODE) || player.doingmagic >= 2) {
			if(InventoryY < 121 * player.bag) {
				InventoryY += long((Original_framedelay * fSpeed) + 2.f);
			}
		} else {
			if(bInventoryClosing) {
				InventoryY += long((Original_framedelay * fSpeed) + 2.f);
				if(InventoryY > 121 * player.bag) {
					bInventoryClosing = false;
					if(player.Interface & INTER_INVENTORYALL) {
						player.Interface &= ~INTER_INVENTORYALL;
					}
					lOldInterface=0;
				}
			} else if(InventoryY > 0) {
				InventoryY -= long((Original_framedelay * fSpeed) + 2.f);
				if(InventoryY < 0) {
					InventoryY = 0;
				}
			}
		}
	}
}

void PlayerInventoryHud::CalculateInventoryCoordinates() {
	
	
	Vec2f anchorPos = anchorPosition();
	
	m_arrowsAnchor.x = anchorPos.x + (m_bagBackgroundSize.x * m_scale) - ((32 + 3)  * m_scale) ;
	m_arrowsAnchor.y = anchorPos.y + ((-3 + 25) * m_scale);
}

//-----------------------------------------------------------------------------
void PlayerInventoryHud::ARX_INTERFACE_DrawInventory(size_t bag, Vec2i i)
{
	fDecPulse += g_framedelay * 0.5f;
	
	Vec2f anchorPos = anchorPosition();
	
	const Vec2f pos = anchorPos + Vec2f(i.x, i.y);
	
	Rectf rect = Rectf(pos + Vec2f(0.f, -(5 * m_scale)), m_bagBackgroundSize.x * m_scale, m_bagBackgroundSize.y * m_scale);
	EERIEDrawBitmap(rect, 0.001f, m_heroInventory, Color::white);
	
	for(size_t y = 0; y < INVENTORY_Y; y++) {
	for(size_t x = 0; x < INVENTORY_X; x++) {
		Entity *io = inventory[bag][x][y].io;
		
		if(!io || !inventory[bag][x][y].show)
			continue;
		
		TextureContainer *tc = io->m_icon;
		TextureContainer *tc2 = NULL;
		
		if(NeedHalo(io))
			tc2 = io->m_icon->getHalo();
		
		if(!tc)
			continue;
		
		const Vec2f p = pos + Vec2f(x, y) * (m_slotSize * m_scale) + (m_slotSpacing * m_scale);
		
		Color color = (io->poisonous && io->poisonous_count != 0) ? Color::green : Color::white;
		
		Rectf rect(p, tc->m_size.x * m_scale, tc->m_size.y * m_scale);
		// TODO use alpha blending so this will be anti-aliased even w/o alpha to coverage
		EERIEDrawBitmap(rect, 0.001f, tc, color);
		
		Color overlayColor = Color::black;
		
		if(io == FlyingOverIO)
			overlayColor = Color::white;
		else if(io->ioflags & IO_CAN_COMBINE)
			overlayColor = Color3f::gray(glm::abs(glm::cos(glm::radians(fDecPulse)))).to<u8>();
		
		
		if(overlayColor != Color::black) {
			GRenderer->SetBlendFunc(BlendSrcAlpha, BlendOne);
			GRenderer->SetRenderState(Renderer::AlphaBlending, true);
			
			EERIEDrawBitmap(rect, 0.001f, tc, overlayColor);
			
			GRenderer->SetRenderState(Renderer::AlphaBlending, false);
		}
		
		if(tc2) {
			ARX_INTERFACE_HALO_Render(io->halo.color, io->halo.flags, tc2, p, Vec2f(m_scale));
		}
		
		if((io->ioflags & IO_ITEM) && io->_itemdata->count != 1)
			ARX_INTERFACE_DrawNumber(p, io->_itemdata->count, 3, Color::white, m_scale);
	}
	}
}

void PlayerInventoryHud::draw() {
	if(player.Interface & INTER_INVENTORY) {		
		if(player.bag) {
			ARX_INTERFACE_DrawInventory(g_currentInventoryBag, Vec2i_ZERO);
			
			CalculateInventoryCoordinates();
			
			if(g_currentInventoryBag > 0) {
				Rectf rect = Rectf(m_arrowsAnchor, 32.f * m_scale, 32.f * m_scale);
				
				EERIEDrawBitmap(rect, 0.001f, m_heroInventoryUp, Color::white);
				
				if(rect.contains(Vec2f(DANAEMouse))) {
					GRenderer->SetBlendFunc(BlendOne, BlendOne);
					GRenderer->SetRenderState(Renderer::AlphaBlending, true);
					SpecialCursor=CURSOR_INTERACTION_ON;
					
					EERIEDrawBitmap(rect, 0.001f, m_heroInventoryUp, Color::white);
					
					GRenderer->SetRenderState(Renderer::AlphaBlending, false);
					SpecialCursor=CURSOR_INTERACTION_ON;
					
					if(eeMouseUp1()) {
						previousBag();
					}
				}
			}
			
			if(g_currentInventoryBag < player.bag-1) {
				Rectf rect = Rectf(m_arrowsAnchor + Vec2f(0.f, 32.f + 5.f) * m_scale, 32.f * m_scale, 32.f * m_scale);
				
				EERIEDrawBitmap(rect, 0.001f, m_heroInventoryDown, Color::white);
				
				if(rect.contains(Vec2f(DANAEMouse))) {
					GRenderer->SetBlendFunc(BlendOne, BlendOne);
					GRenderer->SetRenderState(Renderer::AlphaBlending, true);
					
					EERIEDrawBitmap(rect, 0.001f, m_heroInventoryDown, Color::white);
					
					GRenderer->SetRenderState(Renderer::AlphaBlending, false);
					SpecialCursor=CURSOR_INTERACTION_ON;
					
					if(eeMouseUp1()) {
						nextBag();
					}
				}
			}
		}
	} else if((player.Interface & INTER_INVENTORYALL) || bInventoryClosing) {				
		
		Vec2f anchorPos = anchorPosition();
		
		//TODO see about these coords, might be calculated once only
		const float fBag = (player.bag-1) * (-121 * m_scale);
		const float fOffsetY = (121 * m_scale);
		
		int iOffsetY = checked_range_cast<int>(fBag + fOffsetY);
		int posx = checked_range_cast<int>(anchorPos.x);
		int posy = checked_range_cast<int>(anchorPos.y + ((-3.f + 25 - 32) * m_scale));
		
		for(int i = 0; i < player.bag; i++) {
			Vec2f pos1 = Vec2f(posx + (45 * m_scale), static_cast<float>(posy + iOffsetY));
			Vec2f pos2 = Vec2f(posx + (m_bagBackgroundSize.x * m_scale)*0.5f + (-16 * m_scale), posy+iOffsetY + (-5 * m_scale));
			Vec2f pos3 = Vec2f(posx + (m_bagBackgroundSize.x * m_scale) + ((-45-32) * m_scale), posy+iOffsetY + (-15 * m_scale));
			
			TextureContainer * tex = m_heroInventoryLink;
			
			EERIEDrawBitmap(Rectf(pos1, tex->m_size.x, tex->m_size.y), 0.001f, tex, Color::white);
			EERIEDrawBitmap(Rectf(pos2, tex->m_size.x, tex->m_size.y), 0.001f, tex, Color::white);
			EERIEDrawBitmap(Rectf(pos3, tex->m_size.x, tex->m_size.y), 0.001f, tex, Color::white);
			
			iOffsetY += checked_range_cast<int>(fOffsetY);
		}
		
		iOffsetY = checked_range_cast<int>(fBag);
		
		for(short i = 0; i < player.bag; i++) {
			ARX_INTERFACE_DrawInventory(i, Vec2i(0, iOffsetY));
			iOffsetY += checked_range_cast<int>(fOffsetY);
		}
	}
}

void PlayerInventoryHud::nextBag() {
	
	if((player.Interface & INTER_INVENTORY)) {
		if(player.bag) {
			if(g_currentInventoryBag < player.bag - 1) {
				ARX_SOUND_PlayInterface(SND_BACKPACK, Random::getf(0.9f, 1.1f));
				g_currentInventoryBag ++;
			}
		}
	}
}

void PlayerInventoryHud::previousBag() {
	
	if((player.Interface & INTER_INVENTORY)) {
		if(player.bag) {
			if(g_currentInventoryBag > 0) {
				ARX_SOUND_PlayInterface(SND_BACKPACK, Random::getf(0.9f, 1.1f));
				g_currentInventoryBag --;
			}
		}
	}
}

PlayerInventoryHud g_playerInventoryHud;

bool PlayerInventoryHud::InPlayerInventoryBag(const Vec2s & pos) {
	if(pos.x >= 0 && pos.y >= 0) {
		Vec2s t;
		t.x = pos.x / (32 * m_scale);
		t.y = (pos.y + 5 * m_scale) / (32 * m_scale);

		if(   t.x >= 0
		   && (size_t)t.x <= INVENTORY_X
		   && t.y >= 0
		   && (size_t)t.y <= INVENTORY_Y
		) {
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
	} else if(player.Interface & INTER_INVENTORYALL) {
		float fBag = (player.bag - 1) * (-121 * m_scale);

		short iY = checked_range_cast<short>(fBag);

		if(   pos.x >= iPos.x
		   && pos.x <= iPos.x + INVENTORY_X * (32 * m_scale)
		   && pos.y >= iPos.y + iY
		   && pos.y <= g_size.height()
		) {
			return true;
		}

		for(int i = 0; i < player.bag; i++) {
			Vec2s t = pos - iPos;
			t.y -= iY;
			
			if(InPlayerInventoryBag(t))
				return true;
			
			float fRatio	= (121 * m_scale);

			iY = checked_range_cast<short>(iY + fRatio);
		}
	}

	return false;
}

extern long HERO_OR_SECONDARY;

Entity * PlayerInventoryHud::getObj(const Vec2s & pos) {
	
	Vec2f anchorPos = anchorPosition();
	
	Vec2i iPos = Vec2i(anchorPos);
	
	if(player.Interface & INTER_INVENTORY) {
		long tx = pos.x - iPos.x; //-4
		long ty = pos.y - iPos.y; //-2

		if(tx >= 0 && ty >= 0) {
			tx = checked_range_cast<long>((tx - 6 * m_scale) / (32 * m_scale));
			ty = checked_range_cast<long>((ty - 5 * m_scale) / (32 * m_scale));

			if((tx >= 0) && ((size_t)tx < INVENTORY_X) && (ty >= 0) && ((size_t)ty < INVENTORY_Y)) {
				Entity *result = inventory[g_currentInventoryBag][tx][ty].io;

				if(result && (result->gameFlags & GFLAG_INTERACTIVITY)) {
					HERO_OR_SECONDARY = 1;
					return result;
				}
			}

			return NULL;
		}
	} else if(player.Interface & INTER_INVENTORYALL) {

		float fBag	= (player.bag - 1) * (-121 * m_scale);

		int iY = checked_range_cast<int>(fBag);

		for(size_t bag = 0; bag < size_t(player.bag); bag++) {
			long tx = pos.x - iPos.x;
			long ty = pos.y - iPos.y - iY;

			tx = checked_range_cast<long>((tx - 6 * m_scale) / (32 * m_scale));
			ty = checked_range_cast<long>((ty - 5 * m_scale) / (32 * m_scale));

			if(tx >= 0 && (size_t)tx < INVENTORY_X && ty >= 0 && (size_t)ty < INVENTORY_Y) {
				Entity *result = inventory[bag][tx][ty].io;

				if(result && (result->gameFlags & GFLAG_INTERACTIVITY)) {
					HERO_OR_SECONDARY = 1;
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
	
	if(InventoryY != 0)
		return;
	
	if(!g_playerInventoryHud.containsPos(DANAEMouse))
		return;
	
	Vec2s s = DRAGINTER->m_inventorySize;
	
	int bag = 0;
	
	Vec2f anchorPos = g_playerInventoryHud.anchorPosition();
	Vec2s iPos = Vec2s(anchorPos);
	
	Vec2s t = Vec2s_ZERO;
	
	if(player.Interface & INTER_INVENTORY) {
		t.x = DANAEMouse.x - iPos.x;
		t.y = DANAEMouse.y - iPos.y;
		t.x = t.x / (32 * m_scale); 
		t.y = t.y / (32 * m_scale); 
		
		if((t.x >= 0) && (t.x <= 16 - s.x) && (t.y >= 0) && (t.y <= 3 - s.y)) {
			bag = g_currentInventoryBag;
		} else {
			return;
		}
	} else {
		bool bOk = false;
		
		float fBag	= (player.bag - 1) * (-121 * m_scale);
		
		short iY = checked_range_cast<short>(fBag);
		
		//We must enter the for-loop to initialyze tx/ty
		arx_assert(0 < player.bag);
		
		for(int i = 0; i < player.bag; i++) {
			t.x = DANAEMouse.x - iPos.x;
			t.y = DANAEMouse.y - iPos.y - iY;
			
			if((t.x >= 0) && (t.y >= 0)) {
				t.x = t.x / (32 * m_scale); 
				t.y = t.y / (32 * m_scale); 
				
				if((t.x >= 0) && (t.x <= 16 - s.x) && (t.y >= 0) && (t.y <= 3 - s.y)) {
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
	
	for(long y = 0; y < s.y; y++)
	for(long x = 0; x < s.x; x++) {
		Entity * ioo = inventory[bag][t.x + x][t.y + y].io;
		
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
			
			ARX_SOUND_PlayInterface(SND_INVSTD);
			return;
		}
		
		return;
	}
	
	for(long y = 0; y < s.y; y++) {
	for(long x = 0; x < s.x; x++) {
		inventory[bag][t.x + x][t.y + y].io = DRAGINTER;
		inventory[bag][t.x + x][t.y + y].show = false;
	}
	}
	
	inventory[bag][t.x][t.y].show = true;
	
	ARX_INVENTORY_Declare_InventoryIn(DRAGINTER);
	ARX_SOUND_PlayInterface(SND_INVSTD);
	DRAGINTER->show = SHOW_FLAG_IN_INVENTORY;
	Set_DragInter(NULL);
}

// TODO global sInventory
extern short sInventory;
extern Vec2s sInventoryPos;

void PlayerInventoryHud::dragEntity(Entity * io, const Vec2s &pos) {
	Vec2f anchorPos = g_playerInventoryHud.anchorPosition();
	
	Vec2i iPos = Vec2i(anchorPos);
	
	if(g_playerInventoryHud.containsPos(pos)) {
		if(!GInput->actionPressed(CONTROLS_CUST_STEALTHMODE)) {
			if((io->ioflags & IO_ITEM) && io->_itemdata->count > 1) {
				if(io->_itemdata->count - 1 > 0) {
					
					Entity * ioo = AddItem(io->classPath());
					ioo->show = SHOW_FLAG_NOT_DRAWN;
					ioo->_itemdata->count = 1;
					io->_itemdata->count--;
					ioo->scriptload = 1;
					ARX_SOUND_PlayInterface(SND_INVSTD);
					Set_DragInter(ioo);
					RemoveFromAllInventories(ioo);
					sInventory = 1;
					
					Vec2f f;
					f.x = (pos.x - iPos.x) / (32 * m_scale);
					f.y = (pos.y - iPos.y) / (32 * m_scale);
					
					sInventoryPos.x = checked_range_cast<short>(f.x);
					sInventoryPos.y = checked_range_cast<short>(f.y);
					
					SendInitScriptEvent(ioo);
					ARX_INVENTORY_IdentifyIO(ioo);
					return;
				}
			}
		}
	}
	
	arx_assert(player.bag >= 0);
	arx_assert(player.bag <= 3);
	
	for(size_t bag = 0; bag < size_t(player.bag); bag++)
	for(size_t y = 0; y < INVENTORY_Y; y++)
	for(size_t x = 0; x < INVENTORY_X; x++) {
		INVENTORY_SLOT & slot = inventory[bag][x][y];
		
		if(slot.io == io) {
			slot.io = NULL;
			slot.show = true;
			sInventory = 1;
			
			Vec2f f;
			f.x = (pos.x - iPos.x) / (32 * m_scale);
			f.y = (pos.y - iPos.y) / (32 * m_scale);
			
			sInventoryPos.x = checked_range_cast<short>(f.x);
			sInventoryPos.y = checked_range_cast<short>(f.y);
		}
	}
	
	Set_DragInter(io);
	
	RemoveFromAllInventories(io);
	ARX_INVENTORY_IdentifyIO(io);
}
