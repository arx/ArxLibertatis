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

#include "gui/Hud.h"

#include <iomanip>
#include <sstream>

#include "core/Application.h"
#include "core/ArxGame.h"
#include "core/Config.h"
#include "core/Core.h"
#include "core/GameTime.h"
#include "core/Localisation.h"

#include "game/Entity.h"
#include "game/EntityManager.h"
#include "game/Equipment.h"
#include "game/Inventory.h"
#include "game/Item.h"
#include "game/Player.h"
#include "game/Spells.h"

#include "graphics/Draw.h"
#include "graphics/Renderer.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/data/TextureContainer.h"

#include "gui/Cursor.h"
#include "gui/Interface.h"
#include "gui/Speech.h"
#include "gui/book/Book.h"

#include "input/Input.h"

#include "scene/GameSound.h"
#include "scene/Interactive.h"

extern float GLOBAL_SLOWDOWN;
extern float InventoryDir;
extern bool WILLRETURNTOFREELOOK;

bool bIsAiming = false;
TextureContainer * BasicInventorySkin=NULL;
float InventoryX = -60.f;
static float fDecPulse;
bool bInventorySwitch = false;

enum Anchor {
	Anchor_TopLeft,
	Anchor_TopCenter,
	Anchor_TopRight,
	Anchor_LeftCenter,
	Anchor_Center,
	Anchor_RightCenter,
	Anchor_BottomLeft,
	Anchor_BottomCenter,
	Anchor_BottomRight,
};

static Vec2f getAnchorPos(const Rectf & rect, const Anchor anchor) {
	switch(anchor) {
		case Anchor_TopLeft:      return rect.topLeft();
		case Anchor_TopCenter:    return rect.topCenter();
		case Anchor_TopRight:     return rect.topRight();
		case Anchor_LeftCenter:   return Vec2f(rect.left, rect.top + rect.height() / 2);
		case Anchor_Center:       return rect.center();
		case Anchor_RightCenter:  return Vec2f(rect.right, rect.top + rect.height() / 2);
		case Anchor_BottomLeft:   return rect.bottomLeft();
		case Anchor_BottomCenter: return rect.bottomCenter();
		case Anchor_BottomRight:  return rect.bottomRight();
		default: return rect.topLeft();
	}
}

static Rectf createChild(const Rectf & parent, const Anchor parentAnchor,
                         const Vec2f & size, const Anchor childAnchor) {
	
	Vec2f parentPos = getAnchorPos(parent, parentAnchor);
	
	Rectf child(size.x, size.y);
	Vec2f childPos = getAnchorPos(child, childAnchor);
	child.move(parentPos.x, parentPos.y);
	child.move(-childPos.x, -childPos.y);
	return child;
}

namespace gui {

void InventoryFaderUpdate() {
	
	if(InventoryDir != 0) {
		if((player.Interface & INTER_COMBATMODE) || player.doingmagic >= 2 || InventoryDir == -1) {
			if(InventoryX > -160)
				InventoryX -= INTERFACE_RATIO(framedelay * ( 1.0f / 3 ));
		} else {
			if(InventoryX < 0)
				InventoryX += InventoryDir * INTERFACE_RATIO(framedelay * ( 1.0f / 3 ));
		}

		if(InventoryX <= -160) {
			InventoryX = -160;
			InventoryDir = 0;

			if(player.Interface & INTER_STEAL || ioSteal) {
				SendIOScriptEvent(ioSteal, SM_STEAL, "off");
				player.Interface &= ~INTER_STEAL;
				ioSteal = NULL;
			}

			SecondaryInventory = NULL;
			TSecondaryInventory = NULL;
			InventoryDir = 0;
		} else if(InventoryX >= 0) {
			InventoryX = 0;
			InventoryDir = 0;
		}
	}
}

void CloseSecondaryInventory() {
	
	Entity * io = NULL;
	
	if(SecondaryInventory)
		io = SecondaryInventory->io;
	else if(player.Interface & INTER_STEAL)
		io = ioSteal;
	
	if(io) {
		InventoryDir = -1;
		SendIOScriptEvent(io, SM_INVENTORY2_CLOSE);
		TSecondaryInventory = SecondaryInventory;
		SecondaryInventory = NULL;
	}
}

}



class HudItem {
public:
	HudItem()
		: m_scale(1.f)
	{}
	
	void setScale(float scale) {
		m_scale = scale;
	}

	Rectf rect() { return m_rect; }
	
protected:
	float m_scale;
	Rectf m_rect;
};

/*!
 * \brief the hit strength diamond shown at the bottom of the UI.
 */
class HitStrengthGauge : public HudItem{
private:
	TextureContainer * m_emptyTex;
	TextureContainer * m_fullTex;
	TextureContainer * m_hitTex;
	
	Vec2f m_size;
	Vec2f m_hitSize;
	
	Rectf m_hitRect;
	
	float m_intensity;
	bool bHitFlash;
	unsigned long ulHitFlash;
	float m_flashIntensity;
	
public:
	HitStrengthGauge()
		: m_emptyTex(NULL)
		, m_fullTex(NULL)
		, m_hitTex(NULL)
		, m_intensity(0.f)
		, bHitFlash(false)
		, ulHitFlash(0)
		, m_flashIntensity(0.f)
	{}
	
	void init() {
		m_emptyTex = TextureContainer::LoadUI("graph/interface/bars/aim_empty");
		m_fullTex = TextureContainer::LoadUI("graph/interface/bars/aim_maxi");
		m_hitTex = TextureContainer::LoadUI("graph/interface/bars/flash_gauge");
		arx_assert(m_emptyTex);
		arx_assert(m_fullTex);
		arx_assert(m_hitTex);
		
		m_size = Vec2f(122.f, 70.f);
		m_hitSize = Vec2f(172.f, 130.f);
	}
	
	void requestFlash(float flashIntensity) {
		bHitFlash = true;
		ulHitFlash = 0;
		m_flashIntensity = flashIntensity;
	}
	
	void updateRect(const Rectf & parent) {
		m_rect = createChild(parent, Anchor_BottomCenter, m_size * m_scale, Anchor_BottomCenter);
		m_rect.move(0.f, -2.f);
		
		m_hitRect = createChild(m_rect, Anchor_Center, m_hitSize * m_scale, Anchor_Center);
	}
	
	void update() {
		
		if(AimTime == 0) {
			m_intensity = 0.2f;
		} else {
			float j;
			if(BOW_FOCAL) {
				j=(float)(BOW_FOCAL)/710.f;
			} else {
				float at=float(arxtime)-(float)AimTime;
				
				//TODO global
				bIsAiming = at > 0.f;
				
				at=at*(1.f+(1.f-GLOBAL_SLOWDOWN));
				float aim = static_cast<float>(player.Full_AimTime);
				j=at/aim;
			}
			m_intensity = glm::clamp(j, 0.2f, 1.f);
		}
		
		if(bHitFlash) {
			float fCalc = ulHitFlash + Original_framedelay;
			ulHitFlash = checked_range_cast<unsigned long>(fCalc);
			if(ulHitFlash >= 500) {
				bHitFlash = false;
				ulHitFlash = 0;
			}
		}
	}
	
	void draw() {
		GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
		GRenderer->SetRenderState(Renderer::AlphaBlending, true);
		EERIEDrawBitmap(m_rect, 0.0001f, m_fullTex, Color3f::gray(m_intensity).to<u8>());
		
		GRenderer->SetRenderState(Renderer::AlphaBlending, false);
		EERIEDrawBitmap(m_rect, 0.0001f, m_emptyTex, Color::white);
		
		if(bHitFlash && player.m_skillFull.etheralLink >= 40) {
			
			float j = 1.0f - m_flashIntensity;
			Color col = (j < 0.5f) ? Color3f(j*2.0f, 1, 0).to<u8>() : Color3f(1, m_flashIntensity, 0).to<u8>();
			
			GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
			GRenderer->SetRenderState(Renderer::AlphaBlending, true);
			EERIEDrawBitmap(m_hitRect, 0.0001f, m_hitTex, col);
			GRenderer->SetRenderState(Renderer::AlphaBlending, false);
		}
	}
};
HitStrengthGauge hitStrengthGauge = HitStrengthGauge();

void hitStrengthGaugeRequestFlash(float flashIntensity) {
	hitStrengthGauge.requestFlash(flashIntensity);
}

class SecondaryInventoryGui {
private:
	TextureContainer * ingame_inventory;
	TextureContainer * m_canNotSteal;
	
public:
	void init() {
		m_canNotSteal = TextureContainer::LoadUI("graph/interface/icons/cant_steal_item");
		arx_assert(m_canNotSteal);
	}
	
	Entity* getSecondaryOrStealInvEntity() {
		if(SecondaryInventory) {
			return SecondaryInventory->io;
		} else if(player.Interface & INTER_STEAL) {
			return ioSteal;
		}
		return NULL;
	}
	
	void update() {
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
	}
	
	void draw() {
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
		
		Rectf rect = Rectf(Vec2f(INTERFACE_RATIO(InventoryX), 0.f), ingame_inventory->m_dwWidth, ingame_inventory->m_dwHeight);
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
				EERIEDrawBitmap(rect, 0.001f, tc, color);
				
				Color overlayColor = Color::black;
				
				if(!bItemSteal && (io==FlyingOverIO))
					overlayColor = Color::white;
				else if(!bItemSteal && (io->ioflags & IO_CAN_COMBINE)) {
					overlayColor = Color3f::gray(glm::abs(glm::cos(glm::radians(fDecPulse)))).to<u8>();
				}
				
				if(overlayColor != Color::black) {
					GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
					GRenderer->SetRenderState(Renderer::AlphaBlending, true);
					
					EERIEDrawBitmap(rect, 0.001f, tc, overlayColor);
					
					GRenderer->SetRenderState(Renderer::AlphaBlending, false);
				}
				
				if(tc2) {
					ARX_INTERFACE_HALO_Draw(io, tc, tc2, p);
				}
				
				if((io->ioflags & IO_ITEM) && io->_itemdata->count != 1)
					ARX_INTERFACE_DrawNumber(p, io->_itemdata->count, 3, Color::white);
			}
		}
		}
	}
};
SecondaryInventoryGui secondaryInventory;


extern PlayerInterfaceFlags lOldInterface;

class InventoryGui {
private:
	TextureContainer * m_heroInventory;
	TextureContainer * m_heroInventoryLink;
	TextureContainer * m_heroInventoryUp;
	TextureContainer * m_heroInventoryDown;
	
	Vec2f m_pos;
	
	Vec2f m_slotSize;
	Vec2f m_slotSpacing;
	
public:
	void init() {
		m_heroInventory = TextureContainer::LoadUI("graph/interface/inventory/hero_inventory");
		m_heroInventoryLink = TextureContainer::LoadUI("graph/interface/inventory/hero_inventory_link");
		m_heroInventoryUp = TextureContainer::LoadUI("graph/interface/inventory/scroll_up");
		m_heroInventoryDown = TextureContainer::LoadUI("graph/interface/inventory/scroll_down");
		arx_assert(m_heroInventory);
		arx_assert(m_heroInventoryLink);
		arx_assert(m_heroInventoryUp);
		arx_assert(m_heroInventoryDown);
	}
	
	bool updateInput() {
		Vec2f anchorPos = getInventoryGuiAnchorPosition();
		
		Vec2f pos = anchorPos + Vec2f(INTERFACE_RATIO_DWORD(m_heroInventory->m_dwWidth) - INTERFACE_RATIO(32 + 3), INTERFACE_RATIO(- 3 + 25));
		
		bool bQuitCombine = true;
		
		if(sActiveInventory > 0) {
			const Rect mouseTestRect(
			pos.x,
			pos.y,
			pos.x + INTERFACE_RATIO(32),
			pos.y + INTERFACE_RATIO(32)
			);
			
			if(mouseTestRect.contains(Vec2i(DANAEMouse)))
				bQuitCombine = false;
		}

		if(sActiveInventory < player.bag-1) {
			float fRatio = INTERFACE_RATIO(32 + 5);

			pos.y += checked_range_cast<int>(fRatio);
			
			const Rect mouseTestRect(
			pos.x,
			pos.y,
			pos.x + INTERFACE_RATIO(32),
			pos.y + INTERFACE_RATIO(32)
			);
			
			if(mouseTestRect.contains(Vec2i(DANAEMouse)))
				bQuitCombine = false;
		}
		
		return bQuitCombine;
	}
	
	void update() {
		if(player.Interface & INTER_INVENTORY) {
			if((player.Interface & INTER_COMBATMODE) || player.doingmagic >= 2) {
				long t = Original_framedelay * (1.f/5) + 2;
				InventoryY += static_cast<long>(INTERFACE_RATIO_LONG(t));
	
				if(InventoryY > INTERFACE_RATIO(110.f)) {
					InventoryY = static_cast<long>(INTERFACE_RATIO(110.f));
				}
			} else {
				if(bInventoryClosing) {
					long t = Original_framedelay * (1.f/5) + 2;
					InventoryY += static_cast<long>(INTERFACE_RATIO_LONG(t));
	
					if(InventoryY > INTERFACE_RATIO(110)) {
						InventoryY = static_cast<long>(INTERFACE_RATIO(110.f));
						bInventoryClosing = false;
	
						player.Interface &=~ INTER_INVENTORY;
	
						if(bInventorySwitch) {
							bInventorySwitch = false;
							ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
							player.Interface |= INTER_INVENTORYALL;
							ARX_INTERFACE_NoteClose();
							InventoryY = static_cast<long>(INTERFACE_RATIO(121.f) * player.bag);
							lOldInterface=INTER_INVENTORYALL;
						}
					}
				} else if(InventoryY > 0) {
					InventoryY -= static_cast<long>(INTERFACE_RATIO((Original_framedelay * (1.f/5)) + 2.f));
	
					if(InventoryY < 0) {
						InventoryY = 0;
					}
				}
			}
		} else if((player.Interface & INTER_INVENTORYALL) || bInventoryClosing) {
			float fSpeed = (1.f/3);
			if((player.Interface & INTER_COMBATMODE) || player.doingmagic >= 2) {
				if(InventoryY < INTERFACE_RATIO(121) * player.bag) {
					InventoryY += static_cast<long>(INTERFACE_RATIO((Original_framedelay * fSpeed) + 2.f));
				}
			} else {
				if(bInventoryClosing) {
					InventoryY += static_cast<long>(INTERFACE_RATIO((Original_framedelay * fSpeed) + 2.f));
					if(InventoryY > INTERFACE_RATIO(121) * player.bag) {
						bInventoryClosing = false;
						if(player.Interface & INTER_INVENTORYALL) {
							player.Interface &= ~INTER_INVENTORYALL;
						}
						lOldInterface=0;
					}
				} else if(InventoryY > 0) {
					InventoryY -= static_cast<long>(INTERFACE_RATIO((Original_framedelay * fSpeed) + 2.f));
					if(InventoryY < 0) {
						InventoryY = 0;
					}
				}
			}
		}
	}
	
	void CalculateInventoryCoordinates() {
		
		m_slotSize = Vec2f(32, 32);
		m_slotSpacing = Vec2f(7, 6);
		
		Vec2f anchorPos = getInventoryGuiAnchorPosition();
		
		m_pos.x = anchorPos.x + INTERFACE_RATIO_DWORD(m_heroInventory->m_dwWidth) - INTERFACE_RATIO(32 + 3) ;
		m_pos.y = anchorPos.y + INTERFACE_RATIO(- 3 + 25);
	}
	
	//-----------------------------------------------------------------------------
	void ARX_INTERFACE_DrawInventory(size_t bag, int _iX=0, int _iY=0)
	{
		fDecPulse += framedelay * 0.5f;
		
		Vec2f anchorPos = getInventoryGuiAnchorPosition();
		
		const Vec2f pos = anchorPos + Vec2f(_iX, _iY);
		
		Rectf rect = Rectf(pos + Vec2f(0.f, -INTERFACE_RATIO(5)), m_heroInventory->m_dwWidth, m_heroInventory->m_dwHeight);
		EERIEDrawBitmap(rect, 0.001f, m_heroInventory, Color::white);
		
		for(size_t y = 0; y < INVENTORY_Y; y++) {
		for(size_t x = 0; x < INVENTORY_X; x++) {
			Entity *io = inventory[bag][x][y].io;
			
			if(!io || !inventory[bag][x][y].show)
				continue;
			
			TextureContainer *tc = io->inv;
			TextureContainer *tc2 = NULL;
			
			if(NeedHalo(io))
				tc2 = io->inv->getHalo();
			
			if(!tc)
				continue;
			
			const Vec2f p = pos + Vec2f(x, y) * m_slotSize + m_slotSpacing;
			
			Color color = (io->poisonous && io->poisonous_count != 0) ? Color::green : Color::white;
			
			Rectf rect(p, tc->m_dwWidth, tc->m_dwHeight);
			EERIEDrawBitmap(rect, 0.001f, tc, color);
			
			Color overlayColor = Color::black;
			
			if(io == FlyingOverIO)
				overlayColor = Color::white;
			else if(io->ioflags & IO_CAN_COMBINE)
				overlayColor = Color3f::gray(glm::abs(glm::cos(glm::radians(fDecPulse)))).to<u8>();
			
			
			if(overlayColor != Color::black) {
				GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
				GRenderer->SetRenderState(Renderer::AlphaBlending, true);
				
				EERIEDrawBitmap(rect, 0.001f, tc, overlayColor);
				
				GRenderer->SetRenderState(Renderer::AlphaBlending, false);
			}
			
			if(tc2) {
				ARX_INTERFACE_HALO_Render(io->halo.color, io->halo.flags, tc2, p);
			}
			
			if((io->ioflags & IO_ITEM) && io->_itemdata->count != 1)
				ARX_INTERFACE_DrawNumber(p, io->_itemdata->count, 3, Color::white);
		}
		}
	}
	
	void draw() {
		if(player.Interface & INTER_INVENTORY) {		
			if(player.bag) {
				ARX_INTERFACE_DrawInventory(sActiveInventory);
				
				CalculateInventoryCoordinates();
				
				if(sActiveInventory > 0) {
					Rectf rect = Rectf(m_pos, 32.f, 32.f);
					
					EERIEDrawBitmap(rect, 0.001f, m_heroInventoryUp, Color::white);
					
					if(rect.contains(Vec2f(DANAEMouse))) {
						GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
						GRenderer->SetRenderState(Renderer::AlphaBlending, true);
						SpecialCursor=CURSOR_INTERACTION_ON;
						
						EERIEDrawBitmap(rect, 0.001f, m_heroInventoryUp, Color::white);
						
						GRenderer->SetRenderState(Renderer::AlphaBlending, false);
						SpecialCursor=CURSOR_INTERACTION_ON;
						
						if ((eeMouseDown1())
							|| (eeMouseUp1() && DRAGINTER))
						{
							if(sActiveInventory > 0) {
								ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
								sActiveInventory --;
							}
							
							EERIEMouseButton &= ~1;
						}
					}
				}
				
				if(sActiveInventory < player.bag-1) {
					Rectf rect = Rectf(m_pos + Vec2f(0.f, 32.f + 5.f), 32.f, 32.f);
					
					EERIEDrawBitmap(rect, 0.001f, m_heroInventoryDown, Color::white);
					
					if(rect.contains(Vec2f(DANAEMouse))) {
						GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
						GRenderer->SetRenderState(Renderer::AlphaBlending, true);
						
						EERIEDrawBitmap(rect, 0.001f, m_heroInventoryDown, Color::white);
						
						GRenderer->SetRenderState(Renderer::AlphaBlending, false);
						SpecialCursor=CURSOR_INTERACTION_ON;
						
						if ((eeMouseDown1())
							|| (eeMouseUp1() && DRAGINTER))
						{
							if(sActiveInventory < player.bag-1) {
								ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
								sActiveInventory ++;
							}
							
							EERIEMouseButton &= ~1;
						}
					}
				}
			}
		} else if((player.Interface & INTER_INVENTORYALL) || bInventoryClosing) {				
			
			Vec2f anchorPos = getInventoryGuiAnchorPosition();
			
			//TODO see about these coords, might be calculated once only
			const float fBag = (player.bag-1) * INTERFACE_RATIO(-121);
			const float fOffsetY = INTERFACE_RATIO(121);
			
			int iOffsetY = checked_range_cast<int>(fBag + fOffsetY);
			int posx = checked_range_cast<int>(anchorPos.x);
			int posy = checked_range_cast<int>(anchorPos.y + INTERFACE_RATIO(-3.f + 25 - 32));
			
			for(int i = 0; i < player.bag; i++) {
				Vec2f pos1 = Vec2f(posx + INTERFACE_RATIO(45), static_cast<float>(posy + iOffsetY));
				Vec2f pos2 = Vec2f(posx + INTERFACE_RATIO_DWORD(m_heroInventory->m_dwWidth)*0.5f + INTERFACE_RATIO(-16), posy+iOffsetY + INTERFACE_RATIO(-5));
				Vec2f pos3 = Vec2f(posx + INTERFACE_RATIO_DWORD(m_heroInventory->m_dwWidth) + INTERFACE_RATIO(-45-32), posy+iOffsetY + INTERFACE_RATIO(-15));
				
				TextureContainer * tex = m_heroInventoryLink;
				
				EERIEDrawBitmap(Rectf(pos1, tex->m_dwWidth, tex->m_dwHeight), 0.001f, tex, Color::white);
				EERIEDrawBitmap(Rectf(pos2, tex->m_dwWidth, tex->m_dwHeight), 0.001f, tex, Color::white);
				EERIEDrawBitmap(Rectf(pos3, tex->m_dwWidth, tex->m_dwHeight), 0.001f, tex, Color::white);
				
				iOffsetY += checked_range_cast<int>(fOffsetY);
			}
			
			iOffsetY = checked_range_cast<int>(fBag);
			
			for(short i = 0; i < player.bag; i++) {
				ARX_INTERFACE_DrawInventory(i, 0, iOffsetY);
				iOffsetY += checked_range_cast<int>(fOffsetY);
			}
		}
	}
};

static InventoryGui inventoryGui;

static void DrawItemPrice() {
	
	Entity *temp = SecondaryInventory->io;
	if(temp->ioflags & IO_SHOP) {
		Vec2f pos = Vec2f(DANAEMouse);
		pos += Vec2f(0, -10);
		
		if(InSecondaryInventoryPos(DANAEMouse)) {
			long amount=ARX_INTERACTIVE_GetPrice(FlyingOverIO,temp);
			// achat
			float famount = amount - amount * player.m_skillFull.intuition * 0.005f;
			// check should always be OK because amount is supposed positive
			amount = checked_range_cast<long>(famount);

			Color color = (amount <= player.gold) ? Color::green : Color::red;
			
			ARX_INTERFACE_DrawNumber(pos, amount, 6, color);
		} else if(InPlayerInventoryPos(DANAEMouse)) {
			long amount = static_cast<long>( ARX_INTERACTIVE_GetPrice( FlyingOverIO, temp ) / 3.0f );
			// achat
			float famount = amount + amount * player.m_skillFull.intuition * 0.005f;
			// check should always be OK because amount is supposed positive
			amount = checked_range_cast<long>( famount );

			if(amount) {
				Color color = Color::red;
				
				if(temp->shop_category.empty() ||
				   FlyingOverIO->groups.find(temp->shop_category) != FlyingOverIO->groups.end()) {

					color = Color::green;
				}
				ARX_INTERFACE_DrawNumber(pos, amount, 6, color);
			}
		}
	}
}


class HudIconBase : public HudItem {
protected:
	TextureContainer * m_tex;
	bool m_isSelected;
	
	bool m_haloActive;
	Color3f m_haloColor;
	
public:
	//Used for drawing icons like the book or backpack icon.
	void draw() {
		arx_assert(m_tex);
		
		EERIEDrawBitmap(m_rect, 0.001f, m_tex, Color::white);
		
		if(m_isSelected) {
			GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
			GRenderer->SetRenderState(Renderer::AlphaBlending, true);
			
			EERIEDrawBitmap(m_rect, 0.001f, m_tex, Color::white);
			
			GRenderer->SetRenderState(Renderer::AlphaBlending, false);
		}
		
		if(m_haloActive && m_tex->getHalo()) {
			ARX_INTERFACE_HALO_Render(m_haloColor, HALO_ACTIVE, m_tex->getHalo(), m_rect.topLeft());
		}
	}
};

extern TextureContainer * healing;

class BookIconGui : public HudIconBase {
private:
	
	void MakeBookFX(const Vec3f & pos) {
		
		for(long i = 0; i < 12; i++) {
			
			PARTICLE_DEF * pd = createParticle();
			if(!pd) {
				break;
			}
			
			pd->ov = pos + Vec3f(rnd() * 6.f - rnd() * 12.f, rnd() * 6.f - rnd() * 12.f, 0.f);
			pd->move = Vec3f(6.f - rnd() * 12.f, -8.f + rnd() * 16.f, 0.f);
			pd->scale = Vec3f(4.4f, 4.4f, 1.f);
			pd->tolive = Random::get(1500, 2400);
			pd->tc = healing;
			pd->rgb = Color3f::magenta;
			pd->siz = 56.f;
			pd->is2D = true;
		}
		
		for(int i = 0; i < 5; i++) {
			
			PARTICLE_DEF * pd = createParticle();
			if(!pd) {
				break;
			}
			
			pd->ov = pos - Vec3f(float(i * 2), float(i * 2), 0.f);
			pd->move = Vec3f(-float(i) * 0.5f, -float(i) * 0.5f, 0.f);
			pd->scale = Vec3f(float(i * 10), float(i * 10), 0.f);
			pd->tolive = Random::get(1200, 1600);
			pd->tc = m_tex;
			pd->rgb = Color3f(1.f - float(i) * 0.1f, float(i) * 0.1f, 0.5f - float(i) * 0.1f);
			pd->siz = 32.f + float(i * 4);
			pd->is2D = true;
		}
		
		NewSpell = 1;
	}
	
	Vec2f m_size;
	
	unsigned long ulBookHaloTime;
	
public:
	void init() {
		m_tex = TextureContainer::LoadUI("graph/interface/icons/book");
		arx_assert(m_tex);
		
		m_size = Vec2f(32, 32);
		
		m_haloColor = Color3f(0.2f, 0.4f, 0.8f);
		
		m_haloActive = false;
		ulBookHaloTime = 0;
	}
	
	void requestHalo() {
		m_haloActive = true;
		ulBookHaloTime = 0;
	}
	
	void requestFX() {
		MakeBookFX(Vec3f(Vec2f(g_size.bottomRight()) + Vec2f(-35, -148), 0.00001f));
	}
	
	void update(const Rectf & parent) {
		
		m_rect = createChild(parent, Anchor_TopRight, m_size * m_scale, Anchor_BottomRight);
		
		if(m_haloActive) {
			float fCalc = ulBookHaloTime + Original_framedelay;
			ulBookHaloTime = checked_range_cast<unsigned long>(fCalc);
			if(ulBookHaloTime >= 3000) { // ms
				m_haloActive = false;
			}
		}
	}
	
	void updateInput() {
		m_isSelected = m_rect.contains(Vec2f(DANAEMouse));
		
		if(m_isSelected) {
			SpecialCursor = CURSOR_INTERACTION_ON;

			if(eeMouseDown1()) {
				ARX_INTERFACE_BookToggle();
				EERIEMouseButton &=~1;
			}
			return;
		}
	}
};

static BookIconGui bookIconGui;

void bookIconGuiRequestHalo() {
	bookIconGui.requestHalo();
}

void bookIconGuiRequestFX() {
	bookIconGui.requestFX();
}



class BackpackIconGui : public HudIconBase {
private:
	
public:
	void init() {
		m_tex = TextureContainer::LoadUI("graph/interface/icons/backpack");
		arx_assert(m_tex);
	}

	void update(const Rectf & parent) {
		
		m_rect = createChild(parent, Anchor_TopRight, Vec2f(32, 32) * m_scale, Anchor_BottomRight);
	}
	
	void updateInput() {
		{
		static float flDelay=0;
		
		if(m_rect.contains(Vec2f(DANAEMouse)) || flDelay) {
			eMouseState = MOUSE_IN_INVENTORY_ICON;
			SpecialCursor = CURSOR_INTERACTION_ON;

			if(EERIEMouseButton & 4) {
				ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());

				playerInventory.optimize();

				flDelay=0;
				EERIEMouseButton&=~4;
			} else if((eeMouseDown1()) || flDelay) {
				if(!flDelay) {
					flDelay=arxtime.get_updated();
					return;
				} else {
					if((arxtime.get_updated() - flDelay) < 300) {
						return;
					} else {
						flDelay=0;
					}
				}

				if(player.Interface & INTER_INVENTORYALL) {
					ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
					bInventoryClosing = true;
				} else {
					bInverseInventory=!bInverseInventory;
					lOldTruePlayerMouseLook=TRUE_PLAYER_MOUSELOOK_ON;
				}

				EERIEMouseButton &=~1;
			} else if(eeMouseDown2()) {
				ARX_INTERFACE_BookClose();
				ARX_INVENTORY_OpenClose(NULL);

				if(player.Interface & INTER_INVENTORYALL) {
					bInventoryClosing = true;
				} else {
					if(player.Interface & INTER_INVENTORY) {
						ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
						bInventoryClosing = true;
						bInventorySwitch = true;
					} else {
						ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
						player.Interface |= INTER_INVENTORYALL;

						float fInventoryY = INTERFACE_RATIO(121.f) * (player.bag);
						InventoryY = checked_range_cast<long>(fInventoryY);

						ARX_INTERFACE_NoteClose();

						if(TRUE_PLAYER_MOUSELOOK_ON) {
							WILLRETURNTOFREELOOK = true;
						}
					}
				}

				EERIEMouseButton &= ~2;
				TRUE_PLAYER_MOUSELOOK_ON = false;
			}

			if(DRAGINTER == NULL)
				return;
		}
		}
	}
	
	void draw() {
		m_isSelected = eMouseState == MOUSE_IN_INVENTORY_ICON;
		HudIconBase::draw();
	}
};

static BackpackIconGui backpackIconGui;


class StealIconGui : public HudIconBase {
private:
	Vec2f m_size;
	Vec2f m_pos;
	
public:
	void init() {
		m_tex = TextureContainer::LoadUI("graph/interface/icons/steal");
		arx_assert(m_tex);
		
		m_size = Vec2f(32, 32);
	}
	
	void update() {
		Vec2f pos(static_cast<float>(-lSLID_VALUE), g_size.height() - (78 + 32));
		
		m_rect = Rectf(pos, m_size.x, m_size.y);
	}
	
	void updateInput() {
		
		// steal
		if(player.Interface & INTER_STEAL) {
			if(m_rect.contains(Vec2f(DANAEMouse))) {
				eMouseState=MOUSE_IN_STEAL_ICON;
				SpecialCursor=CURSOR_INTERACTION_ON;

				if(eeMouseDown1()) {
					ARX_INVENTORY_OpenClose(ioSteal);

					if(player.Interface&(INTER_INVENTORY | INTER_INVENTORYALL)) {
						ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
					}

					if(SecondaryInventory) {
						SendIOScriptEvent(ioSteal, SM_STEAL);

						bForceEscapeFreeLook=true;
					    lOldTruePlayerMouseLook=!TRUE_PLAYER_MOUSELOOK_ON;
					}

					EERIEMouseButton &=~1;
				}

				if(DRAGINTER == NULL)
					return;
			}
		}
	}
	
	void draw() {
		m_isSelected = eMouseState == MOUSE_IN_STEAL_ICON;
		HudIconBase::draw();
	}
};

static StealIconGui stealIconGui;

class PickAllIconGui : public HudIconBase {
private:
	Vec2f m_size;
	
public:
	void init() {
		m_tex = TextureContainer::LoadUI("graph/interface/inventory/inv_pick");
		arx_assert(m_tex);
		
		m_size = Vec2f(16, 16);
	}
	
	void update() {
		Rectf parent = Rectf(Vec2f(InventoryX, 0), BasicInventorySkin->m_dwWidth, BasicInventorySkin->m_dwHeight);
		
		Rectf spacer = createChild(parent, Anchor_BottomLeft, Vec2f(16, 16), Anchor_BottomLeft);
		
		m_rect = createChild(spacer, Anchor_BottomRight, m_size, Anchor_BottomLeft);
	}
	
	void updateInput() {
		
		m_isSelected = m_rect.contains(Vec2f(DANAEMouse));
		
		if(m_isSelected) {
			SpecialCursor=CURSOR_INTERACTION_ON;

			if(eeMouseDown1()) {
				if(TSecondaryInventory) {
					// play un son que si un item est pris
					ARX_INVENTORY_TakeAllFromSecondaryInventory();
				}

				EERIEMouseButton &=~1;
			}

			if(DRAGINTER == NULL)
				return;
		}
	}
};

static PickAllIconGui pickAllIconGui;

class CloseSecondaryInventoryIconGui : public HudIconBase {
private:
	Vec2f m_size;
	
public:
	void init() {
		m_tex = TextureContainer::LoadUI("graph/interface/inventory/inv_close");
		arx_assert(m_tex);
		
		m_size = Vec2f(16, 16);
	}
	
	void update() {
		Rectf parent = Rectf(Vec2f(InventoryX, 0), BasicInventorySkin->m_dwWidth, BasicInventorySkin->m_dwHeight);
		
		Rectf spacer = createChild(parent, Anchor_BottomRight, Vec2f(16, 16), Anchor_BottomRight);
		
		m_rect = createChild(spacer, Anchor_BottomLeft, m_size, Anchor_BottomRight);
	}
	
	void updateInput() {
		
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

				EERIEMouseButton &=~1;
			}

			if(DRAGINTER == NULL)
				return;
		}
	}
};

static CloseSecondaryInventoryIconGui closeSecondaryInventoryIconGui;

class LevelUpIconGui : public HudIconBase {
private:
	Vec2f m_pos;
	Vec2f m_size;
	bool m_visible;
	
public:
	void init() {
		m_tex = TextureContainer::LoadUI("graph/interface/icons/lvl_up");
		arx_assert(m_tex);
		m_size = Vec2f(32.f, 32.f);
	}
	
	void update(const Rectf & parent) {
		m_rect = createChild(parent, Anchor_TopRight, m_size * m_scale, Anchor_BottomRight);
		
		m_visible = (player.Skill_Redistribute) || (player.Attribute_Redistribute);
	}
	
	void updateInput() {
		if(!m_visible)
			return;
		
		m_isSelected = m_rect.contains(Vec2f(DANAEMouse));
		
		if(m_isSelected) {
			SpecialCursor = CURSOR_INTERACTION_ON;
			
			if(eeMouseDown1()) {
				ARX_INTERFACE_BookOpen();
				EERIEMouseButton &=~1;
			}
		}
	}
	

	void draw() {
		if(!m_visible)
			return;
		
		HudIconBase::draw();
	}
};

LevelUpIconGui levelUpIconGui;


class PurseIconGui : public HudIconBase {
private:
	Vec2f m_pos;
	Vec2f m_size;
	
	long ulGoldHaloTime;
	
public:
	void init() {
		m_tex = TextureContainer::LoadUI("graph/interface/inventory/gold");
		arx_assert(m_tex);
		m_size = Vec2f(32.f, 32.f);
		
		m_haloColor = Color3f(0.9f, 0.9f, 0.1f);
		
		m_haloActive = false;
		ulGoldHaloTime = 0;
	}
	
	void requestHalo() {
		m_haloActive = true;
		ulGoldHaloTime = 0;
	}
	
	void update(const Rectf & parent) {
		m_rect = createChild(parent, Anchor_TopRight, m_size * m_scale, Anchor_BottomRight);
		
		//A halo is drawn on the character's stats icon (book) when leveling up, for example.
		if(m_haloActive) {
			float fCalc = ulGoldHaloTime + Original_framedelay;
			ulGoldHaloTime = checked_range_cast<unsigned long>(fCalc);
			if(ulGoldHaloTime >= 1000) { // ms
				m_haloActive = false;
			}
		}
	}
	
	void updateInput() {
		m_isSelected = false;
		// gold
		if(player.gold > 0) {
			m_isSelected = m_rect.contains(Vec2f(DANAEMouse));
			
			if(m_isSelected) {
				SpecialCursor = CURSOR_INTERACTION_ON;

				if(   player.gold > 0
				   && !GInput->actionPressed(CONTROLS_CUST_MAGICMODE)
				   && !COMBINE
				   && !COMBINEGOLD
				   && (EERIEMouseButton & 4)
				) {
					COMBINEGOLD = true;
				}

				if(!DRAGINTER)
					return;
			}
		}
	}
	
	void draw() {
		HudIconBase::draw();
		
		if(m_isSelected) {
			Vec2f numberPos = m_rect.topLeft();
			numberPos += Vec2f(- INTERFACE_RATIO(30), + INTERFACE_RATIO(10 - 25));
			
			ARX_INTERFACE_DrawNumber(numberPos, player.gold, 6, Color::white);
		}
	}
};

static PurseIconGui purseIconGui;

void purseIconGuiRequestHalo() {
	purseIconGui.requestHalo();
}


class CurrentTorchIconGui {
private:
	bool m_isActive;
	Rectf m_rect;
	TextureContainer * m_tex;
	
public:
	void updateInput() {
		if(player.torch) {
			Vec2f pos(InventoryX + 110, g_size.height() - (158 + 32));
			
			if(pos.x < INTERFACE_RATIO(10))
				pos.x = INTERFACE_RATIO(10);
			
			const Rect mouseTestRect(
			pos.x,
			pos.y,
			pos.x + INTERFACE_RATIO(32),
			pos.y + INTERFACE_RATIO(64)
			);
			
			if(mouseTestRect.contains(Vec2i(DANAEMouse))) {
				eMouseState=MOUSE_IN_TORCH_ICON;
				SpecialCursor=CURSOR_INTERACTION_ON;

				if(eeMouseUp1()) {
					Entity * temp = player.torch;

					if(temp && !temp->locname.empty()) {
						if(((player.torch->ioflags & IO_ITEM) && player.torch->_itemdata->equipitem)
							&& (player.m_skillFull.objectKnowledge + player.m_attributeFull.mind
								>= player.torch->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_Identify_Value].value) )
						{
							SendIOScriptEvent(FlyingOverIO,SM_IDENTIFY);
						}

						WILLADDSPEECH = getLocalised(temp->locname);

						if(temp->ioflags & IO_GOLD) {
							std::stringstream ss;
							ss << temp->_itemdata->price << " " << WILLADDSPEECH;
							WILLADDSPEECH = ss.str();
						}

						if(temp->poisonous > 0 && temp->poisonous_count != 0) {
							std::string Text = getLocalised("description_poisoned", "error");
							std::stringstream ss;
							ss << WILLADDSPEECH << " (" << Text << " " << (int)temp->poisonous << ")";
							WILLADDSPEECH = ss.str();
						}

						if ((temp->ioflags & IO_ITEM) && temp->durability < 100.f) {
							std::string Text = getLocalised("description_durability", "error");
							std::stringstream ss;
							ss << WILLADDSPEECH << " " << Text << " " << std::fixed << std::setw(3) << std::setprecision(0) << temp->durability << std::setw(0) << "/" << std::setw(3) << temp->max_durability;
							WILLADDSPEECH = ss.str();
						}

						WILLADDSPEECHTIME = (unsigned long)(arxtime);
					}
				}

				if((EERIEMouseButton & 1) && (LastMouseClick & 1)) {
					if(abs(DANAEMouse.x-STARTDRAG.x) > 2 || abs(DANAEMouse.y-STARTDRAG.y) > 2)
						DRAGGING = true;
				}

				if(!DRAGINTER && !PLAYER_MOUSELOOK_ON && DRAGGING) {
					Entity * io=player.torch;
					player.torch->show=SHOW_FLAG_IN_SCENE;
					ARX_SOUND_PlaySFX(SND_TORCH_END);
					ARX_SOUND_Stop(SND_TORCH_LOOP);
					player.torch=NULL;
					lightHandleGet(torchLightHandle)->exist = 0;
					io->ignition=1;
					Set_DragInter(io);
				} else {
					if((EERIEMouseButton & 4) && !COMBINE) {
						COMBINE = player.torch;
					}

					if(eeMouseUp2()) {
						ARX_PLAYER_ClickedOnTorch(player.torch);
						EERIEMouseButton &= ~2;
						TRUE_PLAYER_MOUSELOOK_ON = false;
					}
				}
			}
		}
	}
	
	void update() {
		if((player.Interface & INTER_NOTE) && TSecondaryInventory != NULL
		   && (openNote.type() == gui::Note::BigNote || openNote.type() == gui::Note::Book)) {
			m_isActive = false;
			return;
		}
		m_isActive = true;
		
		m_tex = player.torch->inv;
		arx_assert(m_tex);
		
		
		float px = INTERFACE_RATIO(std::max(InventoryX + 110.f, 10.f));
		float py = g_size.height() - INTERFACE_RATIO(158.f + 32.f);
		
		m_rect = Rectf(Vec2f(px, py), m_tex->m_dwWidth, m_tex->m_dwHeight);
		
		if(rnd() <= 0.2f) {
			return;
		}
		
		createFireParticle(Vec2f(px, py));
	}
	
	void createFireParticle(Vec2f p) {
		
		PARTICLE_DEF * pd = createParticle();
		if(!pd) {
			return;
		}
		
		pd->special = FIRE_TO_SMOKE;
		pd->ov = Vec3f(p.x + INTERFACE_RATIO(12.f - rnd() * 3.f),
		               p.y + INTERFACE_RATIO(rnd() * 6.f), 0.0000001f);
		pd->move = Vec3f(INTERFACE_RATIO(1.5f - rnd() * 3.f),
		                 -INTERFACE_RATIO(5.f + rnd() * 1.f), 0.f);
		pd->scale = Vec3f(1.8f, 1.8f, 1.f);
		pd->tolive = Random::get(500, 900);
		pd->tc = fire2;
		pd->rgb = Color3f(1.f, .6f, .5f);
		pd->siz = INTERFACE_RATIO(14.f);
		pd->is2D = true;
	}
	
	void draw() {
		EERIEDrawBitmap(m_rect, 0.001f, m_tex, Color::white);
	}
};

CurrentTorchIconGui currentTorchIconGui;


class ChangeLevelIconGui : public HudItem {
private:
	TextureContainer * m_tex;
	Vec2f m_size;
	
	float m_intensity;
	
public:
	void init() {
		m_tex = TextureContainer::LoadUI("graph/interface/icons/change_lvl");
		arx_assert(m_tex);
		m_size = Vec2f(32.f, 32.f);
	}
	
	bool isVisible() {
		return CHANGE_LEVEL_ICON > -1;
	}
	
	void update(const Rectf & parent) {
		m_rect = createChild(parent, Anchor_TopRight, m_size * m_scale, Anchor_TopRight);
		
		m_intensity = 0.9f - std::sin(arxtime.get_frame_time()*( 1.0f / 50 ))*0.5f+rnd()*( 1.0f / 10 );
		m_intensity = glm::clamp(m_intensity, 0.f, 1.f);
	}
	
	void draw() {
		
		if(!isVisible())
			return;
		
		EERIEDrawBitmap(m_rect, 0.0001f, m_tex, Color3f::gray(m_intensity).to<u8>());
		
	    if(m_rect.contains(Vec2f(DANAEMouse))) {
			SpecialCursor=CURSOR_INTERACTION_ON;
			if(eeMouseUp1()) {
				CHANGE_LEVEL_ICON = 200;
			}
		}
	}
};
ChangeLevelIconGui changeLevelIconGui;

class QuickSaveIconGui {
private:
	//! Time in ms to show the icon
	u32 QUICK_SAVE_ICON_TIME;
	//! Remaining time for the quick sive icon
	unsigned g_quickSaveIconTime;
	
public:
	QuickSaveIconGui()
		: QUICK_SAVE_ICON_TIME(1000)
		, g_quickSaveIconTime(0)
	{}
	
	void show() {
		g_quickSaveIconTime = QUICK_SAVE_ICON_TIME;
	}
	
	void hide() {
		g_quickSaveIconTime = 0;
	}
	
	void update() {
		if(g_quickSaveIconTime) {
			if(g_quickSaveIconTime > unsigned(framedelay)) {
				g_quickSaveIconTime -= unsigned(framedelay);
			} else {
				g_quickSaveIconTime = 0;
			}
		}
	}
	
	void draw() {
		if(!g_quickSaveIconTime) {
			return;
		}
		
		// Flash the icon twice, starting at about 0.7 opacity
		float step = 1.f - float(g_quickSaveIconTime) * (1.f / QUICK_SAVE_ICON_TIME);
		float alpha = std::min(1.f, 0.6f * (std::sin(step * (7.f / 2.f * PI)) + 1.f));
		
		TextureContainer * tex = TextureContainer::LoadUI("graph/interface/icons/menu_main_save");
		arx_assert(tex);
		
		GRenderer->SetRenderState(Renderer::AlphaBlending, true);
		GRenderer->SetBlendFunc(Renderer::BlendSrcColor, Renderer::BlendOne);
		
		Vec2f size = Vec2f(tex->size());
		EERIEDrawBitmap2(Rectf(Vec2f(0, 0), size.x, size.y), 0.f, tex, Color::gray(alpha));
		
		GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	}
};

QuickSaveIconGui quickSaveIconGui = QuickSaveIconGui();


void showQuickSaveIcon() {
	quickSaveIconGui.show();
}

void hideQuickSaveIcon() {
	quickSaveIconGui.hide();
}


class MemorizedRunesHud : public HudIconBase {
private:
	Vec2f m_size;
	int m_count;
	
public:
	void update(const Rectf & parent) {
		int count = 0;
		int count2 = 0;
		for(long j = 0; j < 6; j++) {
			if(player.SpellToMemorize.iSpellSymbols[j] != RUNE_NONE) {
				count++;
			}
			if(SpellSymbol[j] != RUNE_NONE) {
				count2++;
			}
		}
		m_count = std::max(count, count2);
		
		m_size = Vec2f(m_count * 32, 32);
		
		m_rect = createChild(parent, Anchor_TopLeft, m_size * m_scale, Anchor_TopRight);
	}
	
	void draw() {
		Vec2f pos = m_rect.topLeft();
		
		for(int i = 0; i < 6; i++) {
			bool bHalo = false;
			if(SpellSymbol[i] != RUNE_NONE) {
				if(SpellSymbol[i] == player.SpellToMemorize.iSpellSymbols[i]) {
					bHalo = true;
				} else {
					player.SpellToMemorize.iSpellSymbols[i] = SpellSymbol[i];
	
					for(int j = i+1; j < 6; j++) {
						player.SpellToMemorize.iSpellSymbols[j] = RUNE_NONE;
					}
				}
			}
			if(player.SpellToMemorize.iSpellSymbols[i] != RUNE_NONE) {
				
				Vec2f size = Vec2f(32.f, 32.f) * m_scale;
				Rectf rect = Rectf(pos, size.x, size.y);
				
				TextureContainer *tc = gui::necklace.pTexTab[player.SpellToMemorize.iSpellSymbols[i]];
				
				EERIEDrawBitmap2(rect, 0, tc, Color::white);
				
				if(bHalo) {				
					ARX_INTERFACE_HALO_Render(Color3f(0.2f, 0.4f, 0.8f), HALO_ACTIVE, tc->getHalo(), pos, Vec2f(m_scale));
				}
				
				if(!player.hasRune(player.SpellToMemorize.iSpellSymbols[i])) {
					GRenderer->SetBlendFunc(Renderer::BlendInvDstColor, Renderer::BlendOne);
					GRenderer->SetRenderState(Renderer::AlphaBlending, true);
					EERIEDrawBitmap2(rect, 0, cursorMovable, Color3f::gray(.8f).to<u8>());
					GRenderer->SetRenderState(Renderer::AlphaBlending, false);
				}
				pos.x += 32 * m_scale;
			}
		}
		if(float(arxtime) - player.SpellToMemorize.lTimeCreation > 30000) {
			player.SpellToMemorize.bSpell = false;
		}
	}
};

MemorizedRunesHud memorizedRunesHud;


class HealthGauge : public HudItem {
private:
	Vec2f m_size;
	
	Color m_color;
	TextureContainer * m_emptyTex;
	TextureContainer * m_filledTex;
	float m_amount;
public:
	HealthGauge()
		: m_size(33.f, 80.f)
	{}
	
	void init() {
		m_emptyTex = TextureContainer::LoadUI("graph/interface/bars/empty_gauge_red");
		m_filledTex = TextureContainer::LoadUI("graph/interface/bars/filled_gauge_red");
		arx_assert(m_emptyTex);
		arx_assert(m_filledTex);
	}
	
	void updateRect(const Rectf & parent) {
		m_rect = createChild(parent, Anchor_BottomLeft, m_size * m_scale, Anchor_BottomLeft);
	}
	
	void update() {
		
		m_amount = (float)player.lifePool.current/(float)player.Full_maxlife;
		
		if(player.poison > 0.f) {
			float val = std::min(player.poison, 0.2f) * 255.f * 5.f;
			long g = val;
			m_color = Color(u8(255 - g), u8(g) , 0);
		} else {
			m_color = Color::red;
		}
	}
	
	void draw() {
		
		EERIEDrawBitmap2DecalY(m_rect, 0.f, m_filledTex, m_color, (1.f - m_amount));
		EERIEDrawBitmap(m_rect, 0.001f, m_emptyTex, Color::white);
		
		if(!(player.Interface & INTER_COMBATMODE)) {
			if(m_rect.contains(Vec2f(DANAEMouse))) {
				if(eeMouseDown1()) {
					std::stringstream ss;
					ss << checked_range_cast<int>(player.lifePool.current);
					ARX_SPEECH_Add(ss.str());
				}
			}
		}
	}
};
HealthGauge healthGauge;

class ManaGauge : public HudItem {
private:
	Vec2f m_size;
	
	TextureContainer * m_emptyTex;
	TextureContainer * m_filledTex;
	float m_amount;
	
public:
	ManaGauge()
		: HudItem()
		, m_size(33.f, 80.f)
		, m_emptyTex(NULL)
		, m_filledTex(NULL)
		, m_amount(0.f)
	{}
	
	void init() {
		m_emptyTex = TextureContainer::LoadUI("graph/interface/bars/empty_gauge_blue");
		m_filledTex = TextureContainer::LoadUI("graph/interface/bars/filled_gauge_blue");
		arx_assert(m_emptyTex);
		arx_assert(m_filledTex);
	}
	
	void update(const Rectf & parent) {
		
		m_rect = createChild(parent, Anchor_BottomRight, m_size * m_scale, Anchor_BottomRight);
		
		m_amount = player.manaPool.current / player.Full_maxmana;
	}
	
	void draw() {
		
		EERIEDrawBitmap2DecalY(m_rect, 0.f, m_filledTex, Color::white, (1.f - m_amount));
		EERIEDrawBitmap(m_rect, 0.001f, m_emptyTex, Color::white);
		
		if(!(player.Interface & INTER_COMBATMODE)) {
			if(m_rect.contains(Vec2f(DANAEMouse))) {
				if(eeMouseDown1()) {
					std::stringstream ss;
					ss << checked_range_cast<int>(player.manaPool.current);
					ARX_SPEECH_Add(ss.str());
				}
			}
		}
	}
};
ManaGauge manaGauge;

//The cogwheel icon that shows up when switching from mouseview to interaction mode.
class MecanismIcon : public HudItem {
private:
	Vec2f m_iconSize;
	TextureContainer * m_tex;
	Color m_color;
	long m_timeToDraw;
	long m_nbToDraw;
	
public:
	MecanismIcon()
		: HudItem()
		, m_iconSize(32.f, 32.f)
	{}
	
	void init() {
		m_tex = TextureContainer::LoadUI("graph/interface/cursors/mecanism");
		arx_assert(m_tex);
		
		reset();
	}
	
	void reset() {
		m_timeToDraw = 0;
		m_nbToDraw = 0;
	}
	
	void update() {
		m_color = Color::white;
		if(m_timeToDraw > 300) {
			m_color = Color::black;
			if(m_timeToDraw > 400) {
				m_timeToDraw=0;
				m_nbToDraw++;
			}
		}
		m_timeToDraw += static_cast<long>(framedelay);
		
		m_rect = createChild(Rectf(g_size), Anchor_TopLeft, m_iconSize * m_scale, Anchor_TopLeft);
	}
	
	void draw() {
		if(m_nbToDraw >= 3) {
			return;
		}
		
		EERIEDrawBitmap(m_rect, 0.01f, m_tex, m_color);
	}
};
MecanismIcon mecanismIcon;

void mecanismIconReset() {
	mecanismIcon.reset();
}

class ScreenArrows : public HudItem {
private:
	Vec2f m_horizontalArrowSize;
	Vec2f m_verticalArrowSize;
	
	Rectf m_left;
	Rectf m_right;
	Rectf m_top;
	Rectf m_bottom;
	
	TextureContainer * m_arrowLeftTex;
	
	float fArrowMove;
public:
	ScreenArrows()
		: HudItem()
		, m_horizontalArrowSize(8, 16)
		, m_verticalArrowSize(16, 8)
	{}
	
	void init() {
		m_arrowLeftTex = TextureContainer::LoadUI("graph/interface/icons/arrow_left");
		arx_assert(m_arrowLeftTex);
	}
	
	void update() {
		fArrowMove += .5f * framedelay;
		if(fArrowMove > 180.f) {
			fArrowMove=0.f;
		}
		
		float fMove = glm::abs(glm::sin(glm::radians(fArrowMove))) * m_horizontalArrowSize.x * m_scale * .5f;
		
		const Rectf parent = createChild(Rectf(g_size), Anchor_Center, Vec2f(g_size.size()) - Vec2f(fMove), Anchor_Center);
		m_left   = createChild(parent, Anchor_LeftCenter,   m_horizontalArrowSize * m_scale, Anchor_LeftCenter);
		m_right  = createChild(parent, Anchor_RightCenter,  m_horizontalArrowSize * m_scale, Anchor_RightCenter);
		m_top    = createChild(parent, Anchor_TopCenter,    m_verticalArrowSize * m_scale,   Anchor_TopCenter);
		m_bottom = createChild(parent, Anchor_BottomCenter, m_verticalArrowSize * m_scale,   Anchor_BottomCenter);
	}
	
	void draw() {
		Color lcolor = Color3f::gray(.5f).to<u8>();

		EERIEDrawBitmap(m_left, 0.01f, m_arrowLeftTex, lcolor);
		EERIEDrawBitmapUVs(m_right,  .01f, m_arrowLeftTex, lcolor, Vec2f(1.f, 0.f), Vec2f(0.f, 0.f), Vec2f(1.f, 1.f), Vec2f(0.f, 1.f));
		EERIEDrawBitmapUVs(m_top,    .01f, m_arrowLeftTex, lcolor, Vec2f(0.f, 1.f), Vec2f(0.f, 0.f), Vec2f(1.f, 1.f), Vec2f(1.f, 0.f));
		EERIEDrawBitmapUVs(m_bottom, .01f, m_arrowLeftTex, lcolor, Vec2f(1.f, 1.f), Vec2f(1.f, 0.f), Vec2f(0.f, 1.f), Vec2f(0.f, 0.f));
	}
};
ScreenArrows screenArrows;

class PrecastSpellsGui : public HudItem {
private:
	struct PrecastSpellIconSlot {
		Rectf m_rect;
		TextureContainer * m_tc;
		Color m_color;
		PrecastHandle m_precastIndex;
		
		void update(const Rectf & rect, TextureContainer * tc, Color color, PrecastHandle precastIndex) {
			m_rect = rect;
			m_tc = tc;
			m_color = color;
			m_precastIndex = precastIndex;
		}
		
		void updateInput() {
			if(m_rect.contains(Vec2f(DANAEMouse))) {
				SpecialCursor = CURSOR_INTERACTION_ON;
				
				if(eeMouseUp1()) {
					if(Precast[m_precastIndex].typ >= 0)
						WILLADDSPEECH = spellicons[Precast[m_precastIndex].typ].name;
					
					WILLADDSPEECHTIME = (unsigned long)(arxtime);
				}
				
				if(EERIEMouseButton & 4) {
					ARX_SPELLS_Precast_Launch(m_precastIndex);
					EERIEMouseButton &= ~4;
				}
			}
		}
		
		void draw() {
			EERIEDrawBitmap(m_rect, 0.01f, m_tc, m_color);
			
			GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendOne);
			
			Rectf rect2 = m_rect;
			rect2.move(-1, -1);
			EERIEDrawBitmap(rect2, 0.0001f, m_tc, m_color);
			
			Rectf rect3 = m_rect;
			rect3.move(1, 1);
			EERIEDrawBitmap(rect3, 0.0001f, m_tc, m_color);
			
			GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
		}
	};
	std::vector<PrecastSpellIconSlot> m_icons;
	
	
	Vec2f m_iconSize;
	
public:
	
	PrecastSpellsGui()
		: HudItem()
	{
		m_iconSize = Vec2f(48, 48) / Vec2f(2);
	}
	
	bool isVisible() {
		return !(player.Interface & INTER_INVENTORYALL) && !(player.Interface & INTER_MAP);
	}
	
	void updateRect(const Rectf & parent) {
		
		Vec2f size = m_iconSize * Vec2f(Precast.size(), 1);
		
		m_rect = createChild(parent, Anchor_BottomRight, size * m_scale, Anchor_BottomLeft);
	}
	
	void update() {
		m_icons.clear();
		
		if(!isVisible())
			return;
		
		float intensity = 1.f - PULSATE * 0.5f;
		intensity = glm::clamp(intensity, 0.f, 1.f);
		
		
		for(size_t i = 0; i < Precast.size(); i++) {
			
			PRECAST_STRUCT & precastSlot = Precast[i];
			
			float val = intensity;
			
			if(precastSlot.launch_time > 0 && (float(arxtime) >= precastSlot.launch_time)) {
				float tt = (float(arxtime) - precastSlot.launch_time) * (1.0f/1000);
				
				if(tt > 1.f)
					tt = 1.f;
				
				val *= (1.f - tt);
			}
	
			Color color = Color3f(0, val * (1.0f/2), val).to<u8>();
			
			Rectf childRect = createChild(m_rect, Anchor_BottomLeft, m_iconSize * m_scale, Anchor_BottomLeft);
			childRect.move(i * 33 * m_scale, 0);
			
			SpellType typ = precastSlot.typ;
			
			TextureContainer * tc = spellicons[typ].tc;
			arx_assert(tc);
			
			PrecastSpellIconSlot icon;
			icon.update(childRect, tc, color, PrecastHandle(i));
			
			if(!(player.Interface & INTER_COMBATMODE))
				icon.updateInput();
			
			m_icons.push_back(icon);
		}
	}
	
	void draw() {
		GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
		GRenderer->SetRenderState(Renderer::AlphaBlending, true);
		
		std::vector<PrecastSpellIconSlot>::iterator itr;
		for(itr = m_icons.begin(); itr != m_icons.end(); ++itr) {
			itr->draw();
		}
	}
};
PrecastSpellsGui precastSpellsGui;

//AFFICHAGE ICONE DE SPELLS DE DURATION
class ActiveSpellsGui {
private:
	TextureContainer * m_texUnknown;
	long currpos;
	
	struct ActiveSpellIconSlot {
		Rectf m_rect;
		TextureContainer * m_tc;
		Color m_color;
		SpellHandle spellIndex;
		
		void update(const Rectf & rect, TextureContainer * tc, Color color) {
			m_rect = rect;
			m_tc = tc;
			m_color = color;
		}
		
		void updateInput() {
			if(m_rect.contains(Vec2f(DANAEMouse))) {
				SpecialCursor = CURSOR_INTERACTION_ON;
				
				if(eeMouseUp1()) {
					if(spells[spellIndex]->m_type >= 0)
						WILLADDSPEECH = spellicons[spells[spellIndex]->m_type].name;
					
					WILLADDSPEECHTIME = (unsigned long)(arxtime);
				}
				
				if(EERIEMouseButton & 4) {
					ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE);
					EERIEMouseButton &= ~4;
					spells.endSpell(spells[spellIndex]);
				}
			}
		}
		
		void draw() {
			EERIEDrawBitmap(m_rect, 0.01f, m_tc, m_color);
		}
	};
	ActiveSpellIconSlot activeSpellIconSlot;
	
	
	void ManageSpellIcon(SpellBase & spell, float intensity, bool flag)
	{
		float POSX = g_size.width()-INTERFACE_RATIO(35);
		Color color;
		float posx = POSX+lSLID_VALUE;
		float posy = (float)currpos;
		
		if(flag) {
			color = Color3f(intensity, 0, 0).to<u8>();
		} else {
			color = Color3f::gray(intensity).to<u8>();
		}
		
		bool bOk=true;
		
		if(spell.m_hasDuration) {
			if(player.manaPool.current < 20 || spell.m_timcreation + spell.m_duration - float(arxtime) < 2000) {
				if(ucFlick&1)
					bOk=false;
			}
		} else {
			if(player.manaPool.current<20) {
				if(ucFlick&1)
					bOk=false;
			}
		}
		
		if(bOk && spell.m_type >= 0 && (size_t)spell.m_type < SPELL_TYPES_COUNT) {
			TextureContainer * tc = spellicons[spell.m_type].tc;
			arx_assert(tc);
			Rectf rect(Vec2f(posx, posy), tc->m_dwWidth * 0.5f, tc->m_dwHeight * 0.5f);
			
			activeSpellIconSlot.update(rect, tc, color);
			activeSpellIconSlot.spellIndex = spell.m_thisHandle;
			if(!flag && !(player.Interface & INTER_COMBATMODE)) {
				activeSpellIconSlot.updateInput();
			}
			activeSpellIconSlot.draw();
		}
		
		currpos += static_cast<long>(INTERFACE_RATIO(33.f));
	}
	
public:
	ActiveSpellsGui()
		: m_texUnknown(NULL)
		, currpos(0.f)
	{}
	
	void init() {
		m_texUnknown = TextureContainer::Load("graph/interface/icons/spell_unknown");
		arx_assert(m_texUnknown);
	}
	
	void spellsByPlayerUpdate(float intensity) {
		for(size_t i = 0; i < MAX_SPELLS; i++) {
			SpellBase * spell = spells[SpellHandle(i)];
			
			if(   spell
			   && spell->m_caster == PlayerEntityHandle
			   && spellicons[spell->m_type].m_hasDuration
			) {
				ManageSpellIcon(*spell, intensity, false);
			}
		}
	}
	
	void spellsOnPlayerUpdate(float intensity) {
		for(size_t i = 0; i < MAX_SPELLS; i++) {
			SpellBase * spell = spells[SpellHandle(i)];
			if(!spell)
				continue;
			
			if(std::find(spell->m_targets.begin(), spell->m_targets.end(), PlayerEntityHandle) == spell->m_targets.end()) {
				continue;
			}
			
			if(spell->m_caster != PlayerEntityHandle && spellicons[spell->m_type].m_hasDuration) {
				ManageSpellIcon(*spell, intensity, true);
			}
		}
	}
	
	void update() {
		currpos = static_cast<long>(INTERFACE_RATIO(50.f));
		
		float intensity = 1.f - PULSATE * 0.5f;
		intensity = glm::clamp(intensity, 0.f, 1.f);
		
		GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
		GRenderer->SetRenderState(Renderer::AlphaBlending, true);
		
		spellsByPlayerUpdate(intensity);
		spellsOnPlayerUpdate(intensity);
	}
	
	void draw() {
		update();
	}
};
ActiveSpellsGui activeSpellsGui = ActiveSpellsGui();

/*!
 * \brief Damaged Equipment Drawing
 */
class DamagedEquipmentGui : public HudItem {
private:
	Vec2f m_size;
	
	TextureContainer * iconequip[5];
	
	Color m_colors[5];
	
public:
	DamagedEquipmentGui()
		: HudItem()
		, m_size(64.f, 64.f)
	{}
	
	void init() {
		iconequip[0] = TextureContainer::LoadUI("graph/interface/icons/equipment_sword");
		iconequip[1] = TextureContainer::LoadUI("graph/interface/icons/equipment_shield");
		iconequip[2] = TextureContainer::LoadUI("graph/interface/icons/equipment_helm");
		iconequip[3] = TextureContainer::LoadUI("graph/interface/icons/equipment_chest");
		iconequip[4] = TextureContainer::LoadUI("graph/interface/icons/equipment_leggings");
		arx_assert(iconequip[0]);
		arx_assert(iconequip[1]);
		arx_assert(iconequip[2]);
		arx_assert(iconequip[3]);
		arx_assert(iconequip[4]);
	}
	
	void updateRect(const Rectf & parent) {
		m_rect = createChild(parent, Anchor_BottomRight, m_size * m_scale, Anchor_BottomLeft);
	}
	
	void update() {
		if(cinematicBorder.isActive() || BLOCK_PLAYER_CONTROLS)
			return;
	
		if(player.Interface & INTER_INVENTORYALL)
			return;
		
		for(long i = 0; i < 5; i++) {
			m_colors[i] = Color::black;
			
			long eq=-1;

			switch (i) {
				case 0: eq = EQUIP_SLOT_WEAPON; break;
				case 1: eq = EQUIP_SLOT_SHIELD; break;
				case 2: eq = EQUIP_SLOT_HELMET; break;
				case 3: eq = EQUIP_SLOT_ARMOR; break;
				case 4: eq = EQUIP_SLOT_LEGGINGS; break;
			}
			
			if(player.equiped[eq] > 0) {
				Entity *io = entities[player.equiped[eq]];
				float ratio = io->durability / io->max_durability;
				
				if(ratio <= 0.5f)
					m_colors[i] = Color3f(1.f-ratio, ratio, 0).to<u8>();
			}
		}
	}
	
	void draw() {
		
		GRenderer->SetRenderState(Renderer::AlphaBlending, true);
		GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
		
		GRenderer->SetCulling(Renderer::CullNone);
		GRenderer->SetRenderState(Renderer::DepthWrite, true);
		GRenderer->SetRenderState(Renderer::Fog, false);
		
		for(long i = 0; i < 5; i++) {
			if(m_colors[i] == Color::black)
				continue;
			
			EERIEDrawBitmap2(m_rect, 0.001f, iconequip[i], m_colors[i]);
		}
		
		GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	}
};
DamagedEquipmentGui damagedEquipmentGui;

extern float CURRENT_PLAYER_COLOR;

/*!
 * \brief Stealth Gauge Drawing
 */
class StealthGauge : public HudItem {
private:
	TextureContainer * stealth_gauge_tc;
	
	bool m_visible;
	Color m_color;
	Vec2f m_size;
public:
	void init() {
		stealth_gauge_tc = TextureContainer::LoadUI("graph/interface/icons/stealth_gauge");
		arx_assert(stealth_gauge_tc);
		m_size = Vec2f(32.f, 32.f);
	}
	
	void update(const Rectf & parent) {
		m_rect = createChild(parent, Anchor_TopRight, m_size * m_scale, Anchor_BottomLeft);
		
		m_visible = false;
		
		if(!cinematicBorder.isActive()) {
			float v=GetPlayerStealth();
	
			if(CURRENT_PLAYER_COLOR < v) {
				float t = v - CURRENT_PLAYER_COLOR;
	
				if(t >= 15)
					v = 1.f;
				else
					v = (t*( 1.0f / 15 ))* 0.9f + 0.1f;
				
				m_color = Color3f::gray(v).to<u8>();
				
				m_visible = true;
			}
		}
	}
	
	void draw() {
		if(!m_visible)
			return;
		
		GRenderer->SetRenderState(Renderer::AlphaBlending, true);
		GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
		EERIEDrawBitmap(m_rect, 0.01f, stealth_gauge_tc, m_color);
		GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	}
};
StealthGauge stealthGauge;


void hudElementsInit() {
	changeLevelIconGui.init();
	activeSpellsGui.init();
	damagedEquipmentGui.init();
	mecanismIcon.init();
	
	stealthGauge.init();
	screenArrows.init();
	
	healthGauge.init();
	manaGauge.init();
	bookIconGui.init();
	backpackIconGui.init();
	levelUpIconGui.init();
	stealIconGui.init();
	secondaryInventory.init();
	inventoryGui.init();
	
	BasicInventorySkin = TextureContainer::LoadUI("graph/interface/inventory/ingame_inventory");
	arx_assert(BasicInventorySkin);
	
	purseIconGui.init();
	pickAllIconGui.init();
	closeSecondaryInventoryIconGui.init();
	
	hitStrengthGauge.init();
	
	//setHudScale(2);
}

void setHudScale(float scale) {
	hitStrengthGauge.setScale(scale);
	healthGauge.setScale(scale);
	
	manaGauge.setScale(scale);
	backpackIconGui.setScale(scale);
	bookIconGui.setScale(scale);
	purseIconGui.setScale(scale);
	levelUpIconGui.setScale(scale);
	
	changeLevelIconGui.setScale(scale);
	memorizedRunesHud.setScale(scale);
	
	mecanismIcon.setScale(scale);
	screenArrows.setScale(scale);
	
	stealthGauge.setScale(scale);
	damagedEquipmentGui.setScale(scale);
	precastSpellsGui.setScale(scale);
}

void ArxGame::drawAllInterface() {
	
	Rectf hudSlider = Rectf(g_size);
	hudSlider.left  -= lSLID_VALUE;
	hudSlider.right += lSLID_VALUE;
	
	
	hitStrengthGauge.updateRect(Rectf(g_size));
	hitStrengthGauge.update();
	
	secondaryInventory.update();
	inventoryGui.update();
	mecanismIcon.update();
	screenArrows.update();
	
	changeLevelIconGui.update(Rectf(g_size));
	memorizedRunesHud.update(changeLevelIconGui.rect());
	
	quickSaveIconGui.update();
	
	
	Vec2f anchorPos = getInventoryGuiAnchorPosition();
	
	Rectf spacer;
	spacer.left = std::max(InventoryX + 160, healthGauge.rect().right);
	spacer.bottom = anchorPos.y;
	spacer.top = spacer.bottom - 30;
	spacer.right = spacer.left + 20;
	
	stealthGauge.update(spacer);
	
	damagedEquipmentGui.updateRect(stealthGauge.rect());
	damagedEquipmentGui.update();
	
	precastSpellsGui.updateRect(damagedEquipmentGui.rect());
	precastSpellsGui.update();
	
	
	GRenderer->GetTextureStage(0)->setMinFilter(TextureStage::FilterLinear);
	GRenderer->GetTextureStage(0)->setMagFilter(TextureStage::FilterNearest);
	GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapClamp);

	if(player.Interface & INTER_COMBATMODE) {
		hitStrengthGauge.draw();
	}	
	
	secondaryInventory.draw();
	inventoryGui.draw();
	
	if(FlyingOverIO 
		&& !(player.Interface & INTER_COMBATMODE)
		&& !GInput->actionPressed(CONTROLS_CUST_MAGICMODE)
		&& (!PLAYER_MOUSELOOK_ON || !config.input.autoReadyWeapon)
	  ) {
		if((FlyingOverIO->ioflags & IO_ITEM) && !DRAGINTER && SecondaryInventory) {
			DrawItemPrice();
		}
		SpecialCursor=CURSOR_INTERACTION_ON;
	}
	
	damagedEquipmentGui.draw();
	
	if(!(player.Interface & INTER_COMBATMODE) && (player.Interface & INTER_MINIBACK)) {
		
		if(player.Interface & INTER_STEAL) {
			stealIconGui.update();
			stealIconGui.draw();			
		}
		// Pick All/Close Secondary Inventory
		if(TSecondaryInventory) {
			//These have to be calculated on each frame (to make them move).
			pickAllIconGui.update();
			closeSecondaryInventoryIconGui.update();
			
			Entity *temp = TSecondaryInventory->io;
			if(temp && !(temp->ioflags & IO_SHOP) && !(temp == ioSteal)) {
				pickAllIconGui.draw();
			}
			closeSecondaryInventoryIconGui.draw();
		}
	}
	
	if(!(player.Interface & INTER_COMBATMODE) && player.torch) {
		currentTorchIconGui.update();
		currentTorchIconGui.draw();
	}
	
	changeLevelIconGui.draw();
	
	quickSaveIconGui.draw();
	stealthGauge.draw();

	if((player.Interface & INTER_MAP) && !(player.Interface & INTER_COMBATMODE)) {
		ARX_INTERFACE_ManageOpenedBook();
		
		GRenderer->SetRenderState(Renderer::DepthWrite, true);
		
		if((player.Interface & INTER_MAP) && !(player.Interface & INTER_COMBATMODE)) {
			if(Book_Mode == BOOKMODE_SPELLS) {
				gui::ARX_INTERFACE_ManageOpenedBook_Finish();
				ARX_INTERFACE_ManageOpenedBook_SpellsDraw();
			}
		}
	}
	
	if(CurrSpellSymbol || player.SpellToMemorize.bSpell) {
		memorizedRunesHud.draw();
	}
	
	healthGauge.updateRect(hudSlider);
	healthGauge.update();
	
	
	if(player.Interface & INTER_LIFE_MANA) {
		manaGauge.update(hudSlider);
		manaGauge.draw();
		
		healthGauge.draw();
		
		if(bRenderInCursorMode) {
			GRenderer->SetRenderState(Renderer::AlphaBlending, true);
			GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
			if(!MAGICMODE) {
				mecanismIcon.draw();
			}
			screenArrows.draw();
			GRenderer->SetRenderState(Renderer::AlphaBlending, false);
		}
	}
	
	if(!(player.Interface & INTER_COMBATMODE) && (player.Interface & INTER_MINIBACK)) {
		
		{
		Rectf spacer = createChild(manaGauge.rect(), Anchor_TopRight, Vec2f(0, 3), Anchor_BottomRight);
		backpackIconGui.update(spacer);
		}
		
		{
		Rectf spacer = createChild(backpackIconGui.rect(), Anchor_TopRight, Vec2f(0, 3), Anchor_BottomRight);
		bookIconGui.update(spacer);
		}
		
		{
		Rectf spacer = createChild(bookIconGui.rect(), Anchor_TopRight, Vec2f(0, 3), Anchor_BottomRight);
		purseIconGui.update(spacer);
		}
		
		{
		Rectf spacer = createChild(purseIconGui.rect(), Anchor_TopRight, Vec2f(0, 3), Anchor_BottomRight);
		levelUpIconGui.update(spacer);
		}
		
		backpackIconGui.draw();
		bookIconGui.draw();
		
		// Draw/Manage Gold Purse Icon
		if(player.gold > 0) {
			purseIconGui.draw();
		}
		
		levelUpIconGui.draw();
	}
	
	GRenderer->GetTextureStage(0)->setMinFilter(TextureStage::FilterLinear);
	GRenderer->GetTextureStage(0)->setMagFilter(TextureStage::FilterLinear);
	GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapRepeat);
	
	precastSpellsGui.draw();
	activeSpellsGui.draw();
}


void manageEditorControlsHUD() {
	if(!BLOCK_PLAYER_CONTROLS) {
		if(!(player.Interface & INTER_COMBATMODE)) {
			if(!TRUE_PLAYER_MOUSELOOK_ON) {
				
				currentTorchIconGui.updateInput();
				levelUpIconGui.updateInput();
				purseIconGui.updateInput();
				bookIconGui.updateInput();
				
				backpackIconGui.updateInput();
				
				
			}
			stealIconGui.updateInput();
		}
	}
}

void manageEditorControlsHUD2()
{
	if(TSecondaryInventory) {
		
		Entity * temp = TSecondaryInventory->io;

		if(temp && !(temp->ioflags & IO_SHOP) && !(temp == ioSteal)) {
			pickAllIconGui.updateInput();
		}
		
		closeSecondaryInventoryIconGui.updateInput();
	}
}


bool inventoryGuiupdateInputPROXY()
{
	return inventoryGui.updateInput();
}


Vec2f getInventoryGuiAnchorPosition() {
	
	return Vec2f(g_size.center().x - INTERFACE_RATIO(320) + INTERFACE_RATIO(35) ,
	g_size.height() - INTERFACE_RATIO(101) + INTERFACE_RATIO_LONG(InventoryY));
}
