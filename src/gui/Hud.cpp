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

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

#include <boost/foreach.hpp>

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
#include "gui/hud/HudCommon.h"
#include "gui/hud/PlayerInventory.h"
#include "gui/hud/SecondaryInventory.h"

#include "input/Input.h"

#include "scene/GameSound.h"
#include "scene/Interactive.h"

extern bool WILLRETURNTOFREELOOK;

bool bIsAiming = false;


static void DrawItemPrice() {
	
	Entity *temp = SecondaryInventory->io;
	if(temp->ioflags & IO_SHOP) {
		Vec2f pos = Vec2f(DANAEMouse);
		pos += Vec2f(0, -10);
		
		if(g_secondaryInventoryHud.containsPos(DANAEMouse)) {
			long amount=ARX_INTERACTIVE_GetPrice(FlyingOverIO,temp);
			// achat
			float famount = amount - amount * player.m_skillFull.intuition * 0.005f;
			// check should always be OK because amount is supposed positive
			amount = checked_range_cast<long>(famount);

			Color color = (amount <= player.gold) ? Color::green : Color::red;
			
			ARX_INTERFACE_DrawNumber(pos, amount, 6, color, 1.f);
		} else if(g_playerInventoryHud.containsPos(DANAEMouse)) {
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
				ARX_INTERFACE_DrawNumber(pos, amount, 6, color, 1.f);
			}
		}
	}
}


HitStrengthGauge::HitStrengthGauge()
	: m_emptyTex(NULL)
	, m_fullTex(NULL)
	, m_hitTex(NULL)
	, m_intensity(0.f)
	, m_flashActive(false)
	, m_flashTime(0)
	, m_flashIntensity(0.f)
{}

void HitStrengthGauge::init() {
	m_emptyTex = TextureContainer::LoadUI("graph/interface/bars/aim_empty");
	m_fullTex = TextureContainer::LoadUI("graph/interface/bars/aim_maxi");
	m_hitTex = TextureContainer::LoadUI("graph/interface/bars/flash_gauge");
	arx_assert(m_emptyTex);
	arx_assert(m_fullTex);
	arx_assert(m_hitTex);
	
	m_size = Vec2f(122.f, 70.f);
	m_hitSize = Vec2f(172.f, 130.f);
}

void HitStrengthGauge::requestFlash(float flashIntensity) {
	m_flashActive = true;
	m_flashTime = PlatformDuration_ZERO;
	m_flashIntensity = flashIntensity;
}

void HitStrengthGauge::updateRect(const Rectf & parent) {
	m_rect = createChild(parent, Anchor_BottomCenter, m_size * m_scale, Anchor_BottomCenter);
	m_rect.move(0.f, -2.f);
	
	m_hitRect = createChild(m_rect, Anchor_Center, m_hitSize * m_scale, Anchor_Center);
}

void HitStrengthGauge::update() {
	
	if(player.m_aimTime == 0) {
		m_intensity = 0.2f;
	} else {
		float j;
		if(player.m_bowAimRatio > 0) {
			j = player.m_bowAimRatio;
		} else {
			const ArxDuration delta = arxtime.now() - player.m_aimTime;
			
			//TODO global
			bIsAiming = delta > 0;
			
			float at = delta * (1.f+(1.f-GLOBAL_SLOWDOWN));
			float aim = static_cast<float>(player.Full_AimTime);
			j=at/aim;
		}
		m_intensity = glm::clamp(j, 0.2f, 1.f);
	}
	
	if(m_flashActive) {
		m_flashTime += g_platformTime.lastFrameDuration();
		if(m_flashTime >= PlatformDurationMs(500)) {
			m_flashActive = false;
			m_flashTime = PlatformDuration_ZERO;
		}
	}
}

void HitStrengthGauge::draw() {
	GRenderer->SetBlendFunc(BlendOne, BlendOne);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	EERIEDrawBitmap(m_rect, 0.0001f, m_fullTex, Color3f::gray(m_intensity).to<u8>());
	
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	EERIEDrawBitmap(m_rect, 0.0001f, m_emptyTex, Color::white);
	
	if(m_flashActive && player.m_skillFull.etheralLink >= 40) {
		
		float j = 1.0f - m_flashIntensity;
		Color col = (j < 0.5f) ? Color3f(j*2.0f, 1, 0).to<u8>() : Color3f(1, m_flashIntensity, 0).to<u8>();
		
		GRenderer->SetBlendFunc(BlendOne, BlendOne);
		GRenderer->SetRenderState(Renderer::AlphaBlending, true);
		EERIEDrawBitmap(m_hitRect, 0.0001f, m_hitTex, col);
		GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	}
}


void BookIconGui::MakeBookFX() {
	
	static const float z = 0.00001f;
	
	for(long i = 0; i < 12; i++) {
		
		MagFX(Vec3f(m_rect.topLeft(), z), m_scale);
	}
	
	for(int i = 0; i < 5; i++) {
		
		PARTICLE_DEF * pd = createParticle();
		if(!pd) {
			break;
		}
		
		float s = i * m_scale;
		
		pd->ov = Vec3f(m_rect.topLeft() - Vec2f(s * 2, s * 2), z);
		pd->move = Vec3f(s * -0.5f, s * -0.5f, 0.f);
		pd->scale = Vec3f(s * 10, s * 10, 0.f);
		pd->tolive = Random::getu(1200, 1600);
		pd->tc = m_tex;
		pd->rgb = Color3f(1.f - i * 0.1f, i * 0.1f, 0.5f - i * 0.1f);
		pd->siz = m_rect.width() + s * 4.f;
		pd->is2D = true;
	}
	
	NewSpell = 1;
}

BookIconGui::BookIconGui()
	: HudIconBase()
	, m_size(Vec2f(32, 32))
	, ulBookHaloTime(0)
{}

void BookIconGui::init() {
	m_tex = TextureContainer::LoadUI("graph/interface/icons/book");
	arx_assert(m_tex);
	
	m_size = Vec2f(32, 32);
	
	m_haloColor = Color3f(0.2f, 0.4f, 0.8f);
	
	m_haloActive = false;
	ulBookHaloTime = PlatformDuration_ZERO;
}

void BookIconGui::requestHalo() {
	m_haloActive = true;
	ulBookHaloTime = PlatformDuration_ZERO;
}

void BookIconGui::requestFX() {
	MakeBookFX();
}

void BookIconGui::update(const Rectf & parent) {
	
	m_rect = createChild(parent, Anchor_TopRight, m_size * m_scale, Anchor_BottomRight);
	
	if(m_haloActive) {
		ulBookHaloTime += g_platformTime.lastFrameDuration();
		if(ulBookHaloTime >= PlatformDurationMs(3000)) { // ms
			m_haloActive = false;
		}
	}
}

void BookIconGui::updateInput() {
	m_isSelected = m_rect.contains(Vec2f(DANAEMouse));
	
	if(m_isSelected) {
		SpecialCursor = CURSOR_INTERACTION_ON;

		if(eeMouseDown1()) {
			ARX_INTERFACE_BookToggle();
		}
		return;
	}
}


void BackpackIconGui::init() {
	m_tex = TextureContainer::LoadUI("graph/interface/icons/backpack");
	arx_assert(m_tex);
}

void BackpackIconGui::update(const Rectf & parent) {
	
	m_rect = createChild(parent, Anchor_TopRight, Vec2f(32, 32) * m_scale, Anchor_BottomRight);
}

void BackpackIconGui::updateInput() {
	
	static PlatformInstant flDelay = PlatformInstant_ZERO;
	
	// Check for backpack Icon
	if(m_rect.contains(Vec2f(DANAEMouse))) {
		if(eeMouseUp1() && playerInventory.insert(DRAGINTER)) {
			ARX_SOUND_PlayInterface(SND_INVSTD);
			Set_DragInter(NULL);
		}
	}
	
	if(m_rect.contains(Vec2f(DANAEMouse)) || flDelay != PlatformInstant_ZERO) {
		eMouseState = MOUSE_IN_INVENTORY_ICON;
		SpecialCursor = CURSOR_INTERACTION_ON;
		
		
		if(eeMouseDoubleClick1()) {
			ARX_SOUND_PlayInterface(SND_BACKPACK, Random::getf(0.9f, 1.1f));
			
			playerInventory.optimize();
			
			flDelay = PlatformInstant_ZERO;
		} else if(eeMouseDown1() || flDelay != PlatformInstant_ZERO) {
			if(flDelay == PlatformInstant_ZERO) {
				flDelay = g_platformTime.frameStart();
				return;
			} else {
				if(g_platformTime.frameStart() - flDelay < PlatformDurationMs(300)) {
					return;
				} else {
					flDelay = PlatformInstant_ZERO;
				}
			}
			
			if(player.Interface & INTER_INVENTORYALL) {
				ARX_SOUND_PlayInterface(SND_BACKPACK, Random::getf(0.9f, 1.1f));
				bInventoryClosing = true;
			} else {
				bInverseInventory=!bInverseInventory;
				lOldTruePlayerMouseLook=TRUE_PLAYER_MOUSELOOK_ON;
			}
		} else if(eeMouseDown2()) {
			ARX_INTERFACE_BookClose();
			ARX_INVENTORY_OpenClose(NULL);
			
			if(player.Interface & INTER_INVENTORYALL) {
				bInventoryClosing = true;
			} else {
				if(player.Interface & INTER_INVENTORY) {
					ARX_SOUND_PlayInterface(SND_BACKPACK, Random::getf(0.9f, 1.1f));
					bInventoryClosing = true;
					bInventorySwitch = true;
				} else {
					ARX_SOUND_PlayInterface(SND_BACKPACK, Random::getf(0.9f, 1.1f));
					player.Interface |= INTER_INVENTORYALL;
					
					InventoryY = 121 * player.bag;
					
					ARX_INTERFACE_NoteClose();
					
					if(TRUE_PLAYER_MOUSELOOK_ON) {
						WILLRETURNTOFREELOOK = true;
					}
				}
			}
			
			TRUE_PLAYER_MOUSELOOK_ON = false;
		}
		
		if(DRAGINTER == NULL)
			return;
	}
}

void BackpackIconGui::draw() {
	m_isSelected = eMouseState == MOUSE_IN_INVENTORY_ICON;
	HudIconBase::draw();
}


void StealIconGui::init() {
	m_tex = TextureContainer::LoadUI("graph/interface/icons/steal");
	arx_assert(m_tex);
	
	m_size = Vec2f(32, 32);
}

void StealIconGui::updateRect(const Rectf & parent) {
	
	m_rect = createChild(parent, Anchor_TopLeft, m_size * m_scale, Anchor_BottomLeft);
}

void StealIconGui::updateInput() {
	
	// steal
	if(player.Interface & INTER_STEAL) {
		if(m_rect.contains(Vec2f(DANAEMouse))) {
			eMouseState=MOUSE_IN_STEAL_ICON;
			SpecialCursor=CURSOR_INTERACTION_ON;
			
			if(eeMouseDown1()) {
				ARX_INVENTORY_OpenClose(ioSteal);
				
				if(player.Interface&(INTER_INVENTORY | INTER_INVENTORYALL)) {
					ARX_SOUND_PlayInterface(SND_BACKPACK, Random::getf(0.9f, 1.1f));
				}
				
				if(SecondaryInventory) {
					SendIOScriptEvent(ioSteal, SM_STEAL);
					
					bForceEscapeFreeLook=true;
					lOldTruePlayerMouseLook=!TRUE_PLAYER_MOUSELOOK_ON;
				}
			}
			
			if(DRAGINTER == NULL)
				return;
		}
	}
}

void StealIconGui::draw() {
	m_isSelected = eMouseState == MOUSE_IN_STEAL_ICON;
	HudIconBase::draw();
}


LevelUpIconGui::LevelUpIconGui()
	: HudIconBase()
	, m_pos(0.f, 0.f)
	, m_size(32.f, 32.f)
	, m_visible(false)
{}

void LevelUpIconGui::init() {
	m_tex = TextureContainer::LoadUI("graph/interface/icons/lvl_up");
	arx_assert(m_tex);
	m_size = Vec2f(32.f, 32.f);
}

void LevelUpIconGui::update(const Rectf & parent) {
	m_rect = createChild(parent, Anchor_TopRight, m_size * m_scale, Anchor_BottomRight);
	
	m_visible = (player.Skill_Redistribute) || (player.Attribute_Redistribute);
}

void LevelUpIconGui::updateInput() {
	if(!m_visible)
		return;
	
	m_isSelected = m_rect.contains(Vec2f(DANAEMouse));
	
	if(m_isSelected) {
		SpecialCursor = CURSOR_INTERACTION_ON;
		
		if(eeMouseDown1()) {
			ARX_INTERFACE_BookOpen();
		}
	}
}


void LevelUpIconGui::draw() {
	if(!m_visible)
		return;
	
	HudIconBase::draw();
}


PurseIconGui::PurseIconGui()
	: HudIconBase()
	, m_pos()
	, m_size()
	, m_haloTime(0)
{}

void PurseIconGui::init() {
	m_tex = TextureContainer::LoadUI("graph/interface/inventory/gold");
	arx_assert(m_tex);
	m_size = Vec2f(32.f, 32.f);
	
	m_haloColor = Color3f(0.9f, 0.9f, 0.1f);
	
	m_haloActive = false;
	m_haloTime = PlatformDuration_ZERO;
}

void PurseIconGui::requestHalo() {
	m_haloActive = true;
	m_haloTime = PlatformDuration_ZERO;
}

void PurseIconGui::update(const Rectf & parent) {
	m_rect = createChild(parent, Anchor_TopRight, m_size * m_scale, Anchor_BottomRight);
	
	if(m_haloActive) {
		m_haloTime += g_platformTime.lastFrameDuration();
		if(m_haloTime >= PlatformDurationMs(1000)) {
			m_haloActive = false;
		}
	}
}

void PurseIconGui::updateInput() {
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
			   && eeMouseDoubleClick1()
			) {
				COMBINEGOLD = true;
			}
			
			if(!DRAGINTER)
				return;
		}
	}
}

void PurseIconGui::draw() {
	HudIconBase::draw();
	
	if(m_isSelected) {
		Vec2f numberPos = m_rect.topLeft();
		numberPos += Vec2f(-30 * m_scale, -15 * m_scale);
		
		ARX_INTERFACE_DrawNumber(numberPos, player.gold, 6, Color::white, m_scale);
	}
}


CurrentTorchIconGui::CurrentTorchIconGui()
	: HudItem()
	, m_isActive(false)
	, m_rect()
	, m_tex(NULL)
	, m_size()
{}

void CurrentTorchIconGui::init() {
	m_size = Vec2f(32.f, 64.f);
}

bool CurrentTorchIconGui::isVisible() {
	return !(player.Interface & INTER_COMBATMODE) && player.torch;
}

void CurrentTorchIconGui::updateRect(const Rectf & parent) {
	
	float secondaryInventoryX = g_secondaryInventoryHud.m_fadePosition + 110.f;
	
	m_rect = createChild(parent, Anchor_TopLeft, m_size * m_scale, Anchor_BottomLeft);
	
	if(m_rect.left < secondaryInventoryX) {
		m_rect.move(secondaryInventoryX, 0.f);
	}
}

void CurrentTorchIconGui::updateInput() {
	if(player.torch) {
		
		if(m_rect.contains(Vec2f(DANAEMouse))) {
			eMouseState = MOUSE_IN_TORCH_ICON;
			SpecialCursor = CURSOR_INTERACTION_ON;
			
			if(!DRAGINTER && !PLAYER_MOUSELOOK_ON && DRAGGING) {
				Entity * io = player.torch;
				player.torch->show = SHOW_FLAG_IN_SCENE;
				ARX_SOUND_PlaySFX(SND_TORCH_END);
				ARX_SOUND_Stop(SND_TORCH_LOOP);
				player.torch = NULL;
				lightHandleGet(torchLightHandle)->exist = 0;
				io->ignition = 1;
				Set_DragInter(io);
			} else {
				if(eeMouseDoubleClick1() && !COMBINE) {
					COMBINE = player.torch;
				}
				
				if(eeMouseUp2()) {
					ARX_PLAYER_ClickedOnTorch(player.torch);
					TRUE_PLAYER_MOUSELOOK_ON = false;
				}
			}
		}
	}
}

void CurrentTorchIconGui::update() {
	
	if(!isVisible())
		return;
	
	if((player.Interface & INTER_NOTE) && TSecondaryInventory != NULL
	   && (openNote.type() == gui::Note::BigNote || openNote.type() == gui::Note::Book)) {
		m_isActive = false;
		return;
	}
	m_isActive = true;
	
	m_tex = player.torch->m_icon;
	arx_assert(m_tex);
	
	if(Random::getf() <= 0.2f) {
		return;
	}
	
	createFireParticle();
}

void CurrentTorchIconGui::createFireParticle() {
	
	Vec2f pos = m_rect.topLeft() + Vec2f(Random::getf(9.f, 12.f), Random::getf(0.f, 6.f)) * m_scale;
	spawn2DFireParticle(pos, m_scale);
}

void CurrentTorchIconGui::draw() {
	
	if(!isVisible())
		return;
	
	EERIEDrawBitmap(m_rect, 0.001f, m_tex, Color::white);
}


ChangeLevelIconGui::ChangeLevelIconGui()
	: m_tex(NULL)
	, m_intensity(1.f)
{}

void ChangeLevelIconGui::init() {
	m_tex = TextureContainer::LoadUI("graph/interface/icons/change_lvl");
	arx_assert(m_tex);
	m_size = Vec2f(32.f, 32.f);
}

bool ChangeLevelIconGui::isVisible() {
	return CHANGE_LEVEL_ICON > -1;
}

void ChangeLevelIconGui::update(const Rectf & parent) {
	m_rect = createChild(parent, Anchor_TopRight, m_size * m_scale, Anchor_TopRight);
	
	m_intensity = 0.9f - std::sin(arxtime.get_frame_time() * 0.02f) * 0.5f + Random::getf(0.f, 0.1f);
	m_intensity = glm::clamp(m_intensity, 0.f, 1.f);
}

void ChangeLevelIconGui::draw() {
	
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


QuickSaveIconGui::QuickSaveIconGui()
	: m_duration(1000)
	, m_remainingTime(0)
{}

void QuickSaveIconGui::show() {
	m_remainingTime = m_duration;
}

void QuickSaveIconGui::hide() {
	m_remainingTime = 0;
}

void QuickSaveIconGui::update() {
	if(m_remainingTime) {
		if(m_remainingTime > unsigned(g_framedelay)) {
			m_remainingTime -= unsigned(g_framedelay);
		} else {
			m_remainingTime = 0;
		}
	}
}

void QuickSaveIconGui::draw() {
	if(!m_remainingTime) {
		return;
	}
	
	// Flash the icon twice, starting at about 0.7 opacity
	float step = 1.f - float(m_remainingTime) * (1.f / m_duration);
	float alpha = std::min(1.f, 0.6f * (std::sin(step * (7.f / 2.f * glm::pi<float>())) + 1.f));
	
	TextureContainer * tex = TextureContainer::LoadUI("graph/interface/icons/menu_main_save");
	arx_assert(tex);
	
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetBlendFunc(BlendSrcColor, BlendOne);
	
	Vec2f size = Vec2f(tex->size());
	EERIEDrawBitmap2(Rectf(Vec2f(0, 0), size.x, size.y), 0.f, tex, Color::gray(alpha));
	
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
}


MemorizedRunesHud::MemorizedRunesHud()
	: HudIconBase()
	, m_size()
	, m_count(0)
{}

void MemorizedRunesHud::update(const Rectf & parent) {
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

void MemorizedRunesHud::draw() {
	
	if(!(CurrSpellSymbol || player.SpellToMemorize.bSpell)) {
		return;
	}
	
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
				GRenderer->SetBlendFunc(BlendInvDstColor, BlendOne);
				GRenderer->SetRenderState(Renderer::AlphaBlending, true);
				EERIEDrawBitmap2(rect, 0, cursorMovable, Color3f::gray(.8f).to<u8>());
				GRenderer->SetRenderState(Renderer::AlphaBlending, false);
			}
			pos.x += 32 * m_scale;
		}
	}
	if(arxtime.now() - player.SpellToMemorize.lTimeCreation > ArxDurationMs(30000)) {
		player.SpellToMemorize.bSpell = false;
	}
}


HealthGauge::HealthGauge()
	: m_size(33.f, 80.f)
	, m_emptyTex(NULL)
	, m_filledTex(NULL)
	, m_amount(0.f)
{}

void HealthGauge::init() {
	m_emptyTex = TextureContainer::LoadUI("graph/interface/bars/empty_gauge_red");
	m_filledTex = TextureContainer::LoadUI("graph/interface/bars/filled_gauge_red");
	arx_assert(m_emptyTex);
	arx_assert(m_filledTex);
}

void HealthGauge::updateRect(const Rectf & parent) {
	m_rect = createChild(parent, Anchor_BottomLeft, m_size * m_scale, Anchor_BottomLeft);
}

void HealthGauge::update() {
	
	m_amount = player.lifePool.current / player.Full_maxlife;
	
	if(player.poison > 0.f) {
		float val = std::min(player.poison, 0.2f) * 255.f * 5.f;
		long g = val;
		m_color = Color(u8(255 - g), u8(g) , 0);
	} else {
		m_color = Color::red;
	}
}

void HealthGauge::updateInput(const Vec2f & mousePos) {
	if(!(player.Interface & INTER_COMBATMODE)) {
		if(m_rect.contains(mousePos)) {
			if(eeMouseDown1()) {
				std::stringstream ss;
				ss << checked_range_cast<int>(player.lifePool.current);
				ARX_SPEECH_Add(ss.str());
			}
		}
	}
}

void HealthGauge::draw() {
	
	EERIEDrawBitmap2DecalY(m_rect, 0.f, m_filledTex, m_color, (1.f - m_amount));
	EERIEDrawBitmap(m_rect, 0.001f, m_emptyTex, Color::white);
}


ManaGauge::ManaGauge()
	: HudItem()
	, m_size(33.f, 80.f)
	, m_emptyTex(NULL)
	, m_filledTex(NULL)
	, m_amount(0.f)
{}

void ManaGauge::init() {
	m_emptyTex = TextureContainer::LoadUI("graph/interface/bars/empty_gauge_blue");
	m_filledTex = TextureContainer::LoadUI("graph/interface/bars/filled_gauge_blue");
	arx_assert(m_emptyTex);
	arx_assert(m_filledTex);
}

void ManaGauge::update(const Rectf & parent) {
	
	m_rect = createChild(parent, Anchor_BottomRight, m_size * m_scale, Anchor_BottomRight);
	
	m_amount = player.manaPool.current / player.Full_maxmana;
}

void ManaGauge::updateInput(const Vec2f & mousePos) {
	if(!(player.Interface & INTER_COMBATMODE)) {
		if(m_rect.contains(mousePos)) {
			if(eeMouseDown1()) {
				std::stringstream ss;
				ss << checked_range_cast<int>(player.manaPool.current);
				ARX_SPEECH_Add(ss.str());
			}
		}
	}
}

void ManaGauge::draw() {
	
	EERIEDrawBitmap2DecalY(m_rect, 0.f, m_filledTex, Color::white, (1.f - m_amount));
	EERIEDrawBitmap(m_rect, 0.001f, m_emptyTex, Color::white);
}


MecanismIcon::MecanismIcon()
	: HudItem()
	, m_iconSize(32.f, 32.f)
	, m_tex(NULL)
	, m_timeToDraw(0)
	, m_nbToDraw(0)
{}

void MecanismIcon::init() {
	m_tex = TextureContainer::LoadUI("graph/interface/cursors/mecanism");
	arx_assert(m_tex);
	
	reset();
}

void MecanismIcon::reset() {
	m_timeToDraw = 0;
	m_nbToDraw = 0;
}

void MecanismIcon::update() {
	m_color = Color::white;
	if(m_timeToDraw > 300) {
		m_color = Color::black;
		if(m_timeToDraw > 400) {
			m_timeToDraw=0;
			m_nbToDraw++;
		}
	}
	m_timeToDraw += static_cast<long>(g_framedelay);
	
	m_rect = createChild(Rectf(g_size), Anchor_TopLeft, m_iconSize * m_scale, Anchor_TopLeft);
}

void MecanismIcon::draw() {
	if(m_nbToDraw >= 3) {
		return;
	}
	
	EERIEDrawBitmap(m_rect, 0.01f, m_tex, m_color);
}


ScreenArrows::ScreenArrows()
	: HudItem()
	, m_horizontalArrowSize(8, 16)
	, m_verticalArrowSize(16, 8)
	, m_arrowLeftTex(NULL)
	, fArrowMove(0.f)
{}

void ScreenArrows::init() {
	m_arrowLeftTex = TextureContainer::LoadUI("graph/interface/icons/arrow_left");
	arx_assert(m_arrowLeftTex);
}

void ScreenArrows::update() {
	
	if(!config.input.borderTurning) {
		return;
	}
	
	fArrowMove += .5f * g_framedelay;
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

void ScreenArrows::draw() {
	
	if(!config.input.borderTurning) {
		return;
	}
	
	Color lcolor = Color3f::gray(.5f).to<u8>();
	
	EERIEDrawBitmap(m_left, 0.01f, m_arrowLeftTex, lcolor);
	EERIEDrawBitmapUVs(m_right,  .01f, m_arrowLeftTex, lcolor, Vec2f(1.f, 0.f), Vec2f(0.f, 0.f), Vec2f(1.f, 1.f), Vec2f(0.f, 1.f));
	EERIEDrawBitmapUVs(m_top,    .01f, m_arrowLeftTex, lcolor, Vec2f(0.f, 1.f), Vec2f(0.f, 0.f), Vec2f(1.f, 1.f), Vec2f(1.f, 0.f));
	EERIEDrawBitmapUVs(m_bottom, .01f, m_arrowLeftTex, lcolor, Vec2f(1.f, 1.f), Vec2f(1.f, 0.f), Vec2f(0.f, 1.f), Vec2f(0.f, 0.f));
}


void PrecastSpellsGui::PrecastSpellIconSlot::update(const Rectf & rect, TextureContainer * tc, Color color, PrecastHandle precastIndex) {
	m_rect = rect;
	m_tc = tc;
	m_color = color;
	m_precastIndex = precastIndex;
}

void PrecastSpellsGui::PrecastSpellIconSlot::updateInput() {
	if(m_rect.contains(Vec2f(DANAEMouse))) {
		SpecialCursor = CURSOR_INTERACTION_ON;
		
		if(eeMouseUp1()) {
			if(Precast[m_precastIndex.handleData()].typ >= 0) {
				ARX_SPEECH_Add(spellicons[Precast[m_precastIndex.handleData()].typ].name);
			}
		}
		
		if(eeMouseDoubleClick1()) {
			ARX_SPELLS_Precast_Launch(m_precastIndex);
		}
	}
}

void PrecastSpellsGui::PrecastSpellIconSlot::draw() {
	EERIEDrawBitmap(m_rect, 0.01f, m_tc, m_color);
	
	GRenderer->SetBlendFunc(BlendZero, BlendOne);
	
	Rectf rect2 = m_rect;
	rect2.move(-1, -1);
	EERIEDrawBitmap(rect2, 0.0001f, m_tc, m_color);
	
	Rectf rect3 = m_rect;
	rect3.move(1, 1);
	EERIEDrawBitmap(rect3, 0.0001f, m_tc, m_color);
	
	GRenderer->SetBlendFunc(BlendOne, BlendOne);
}

PrecastSpellsGui::PrecastSpellsGui()
	: HudItem()
{
	m_iconSize = Vec2f(48, 48) / Vec2f(2);
}

bool PrecastSpellsGui::isVisible() {
	return !(player.Interface & INTER_INVENTORYALL) && !(player.Interface & INTER_MAP);
}

void PrecastSpellsGui::updateRect(const Rectf & parent) {
	
	Vec2f size = m_iconSize * Vec2f(Precast.size(), 1);
	
	m_rect = createChild(parent, Anchor_BottomRight, size * m_scale, Anchor_BottomLeft);
}

void PrecastSpellsGui::update() {
	m_icons.clear();
	
	if(!isVisible())
		return;
	
	float intensity = 1.f - PULSATE * 0.5f;
	intensity = glm::clamp(intensity, 0.f, 1.f);
	
	
	for(size_t i = 0; i < Precast.size(); i++) {
		
		PRECAST_STRUCT & precastSlot = Precast[i];
		
		float val = intensity;
		
		if(precastSlot.launch_time > ArxInstant_ZERO && arxtime.now() >= precastSlot.launch_time) {
			float tt = (arxtime.now() - precastSlot.launch_time) * (1.0f/1000);
			
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

void PrecastSpellsGui::draw() {
	GRenderer->SetBlendFunc(BlendOne, BlendOne);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	
	std::vector<PrecastSpellIconSlot>::iterator itr;
	for(itr = m_icons.begin(); itr != m_icons.end(); ++itr) {
		itr->draw();
	}
}


void ActiveSpellsGui::ActiveSpellIconSlot::updateInput(const Vec2f & mousePos) {
	
	if(!m_abortable)
		return;
	
	if(m_rect.contains(mousePos)) {
		SpecialCursor = CURSOR_INTERACTION_ON;
		
		if(eeMouseUp1()) {
			if(spells[spellIndex]->m_type >= 0) {
				ARX_SPEECH_Add(spellicons[spells[spellIndex]->m_type].name);
			}
		}
		
		if(eeMouseDoubleClick1()) {
			ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE);
			spells.endSpell(spells[spellIndex]);
		}
	}
}

void ActiveSpellsGui::ActiveSpellIconSlot::draw() {
	
	if(!m_flicker)
		return;
	
	EERIEDrawBitmap(m_rect, 0.01f, m_tc, m_color);
}

ActiveSpellsGui::ActiveSpellsGui()
	: HudItem()
	, m_texUnknown(NULL)
{}

void ActiveSpellsGui::init() {
	m_texUnknown = TextureContainer::Load("graph/interface/icons/spell_unknown");
	arx_assert(m_texUnknown);
	
	m_slotSize = Vec2f(24.f, 24.f);
	m_spacerSize = Vec2f(60.f, 50.f);
	m_slotSpacerSize = Vec2f(0.f, 9.f);
}

void ActiveSpellsGui::update(Rectf parent) {
	
	float intensity = 1.f - PULSATE * 0.5f;
	intensity = glm::clamp(intensity, 0.f, 1.f);
	
	m_slots.clear();
	
	spellsByPlayerUpdate(intensity);
	spellsOnPlayerUpdate(intensity);
	
	Rectf spacer = createChild(parent, Anchor_TopRight, m_spacerSize * m_scale, Anchor_TopRight);
	Rectf siblingRect = spacer;
	
	BOOST_FOREACH(ActiveSpellIconSlot & slot, m_slots) {
		
		Rectf slotRect = createChild(siblingRect, Anchor_BottomLeft, m_slotSize * m_scale, Anchor_TopLeft);
		Rectf slotSpacer = createChild(slotRect, Anchor_BottomLeft, m_slotSpacerSize * m_scale, Anchor_TopLeft);
		siblingRect = slotSpacer;
		
		slot.m_rect = slotRect;
	}
}

void ActiveSpellsGui::updateInput(const Vec2f & mousePos) {
	
	BOOST_FOREACH(ActiveSpellIconSlot & slot, m_slots) {
		slot.updateInput(mousePos);
	}
}

void ActiveSpellsGui::draw() {
	
	GRenderer->SetBlendFunc(BlendOne, BlendOne);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	
	BOOST_FOREACH(ActiveSpellIconSlot & slot, m_slots) {
		slot.draw();
	}
}

void ActiveSpellsGui::spellsByPlayerUpdate(float intensity) {
	for(size_t i = 0; i < MAX_SPELLS; i++) {
		SpellBase * spell = spells[SpellHandle(i)];
		
		if(   spell
		   && spell->m_caster == EntityHandle_Player
		   && spellicons[spell->m_type].m_hasDuration
		) {
			ManageSpellIcon(*spell, intensity, false);
		}
	}
}

void ActiveSpellsGui::spellsOnPlayerUpdate(float intensity) {
	for(size_t i = 0; i < MAX_SPELLS; i++) {
		SpellBase * spell = spells[SpellHandle(i)];
		if(!spell)
			continue;
		
		if(std::find(spell->m_targets.begin(), spell->m_targets.end(), EntityHandle_Player) == spell->m_targets.end()) {
			continue;
		}
		
		if(spell->m_caster != EntityHandle_Player && spellicons[spell->m_type].m_hasDuration) {
			ManageSpellIcon(*spell, intensity, true);
		}
	}
}

void ActiveSpellsGui::ManageSpellIcon(SpellBase & spell, float intensity, bool flag) {
	
	
	Color color = (flag) ? Color3f(intensity, 0, 0).to<u8>() : Color3f::gray(intensity).to<u8>();
	
	bool flicker = true;
	
	if(spell.m_hasDuration) {
		if(player.manaPool.current < 20 || spell.m_timcreation + spell.m_duration - arxtime.now() < ArxDurationMs(2000)) {
			if(ucFlick&1)
				flicker = false;
		}
	} else {
		if(player.manaPool.current<20) {
			if(ucFlick&1)
				flicker = false;
		}
	}
	
	if(spell.m_type >= 0 && (size_t)spell.m_type < SPELL_TYPES_COUNT) {
	
		ActiveSpellIconSlot slot;
		slot.m_tc = spellicons[spell.m_type].tc;
		slot.m_color = color;
		slot.spellIndex = spell.m_thisHandle;
		slot.m_flicker = flicker;
		slot.m_abortable = (!flag && !(player.Interface & INTER_COMBATMODE));
		
		m_slots.push_back(slot);
	}
}


DamagedEquipmentGui::DamagedEquipmentGui()
	: HudItem()
	, m_size(64.f, 64.f)
{
	iconequip[0] = NULL;
	iconequip[1] = NULL;
	iconequip[2] = NULL;
	iconequip[3] = NULL;
	iconequip[4] = NULL;
}

void DamagedEquipmentGui::init() {
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

void DamagedEquipmentGui::updateRect(const Rectf & parent) {
	m_rect = createChild(parent, Anchor_BottomRight, m_size * m_scale, Anchor_BottomLeft);
}

void DamagedEquipmentGui::update() {
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
		
		if(ValidIONum(player.equiped[eq])) {
			Entity *io = entities[player.equiped[eq]];
			float ratio = io->durability / io->max_durability;
			
			if(ratio <= 0.5f)
				m_colors[i] = Color3f(1.f-ratio, ratio, 0).to<u8>();
		}
	}
}

void DamagedEquipmentGui::draw() {
	
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetBlendFunc(BlendOne, BlendOne);
	
	GRenderer->SetCulling(CullNone);
	GRenderer->SetRenderState(Renderer::DepthWrite, true);
	GRenderer->SetRenderState(Renderer::Fog, false);
	
	for(long i = 0; i < 5; i++) {
		if(m_colors[i] == Color::black)
			continue;
		
		EERIEDrawBitmap2(m_rect, 0.001f, iconequip[i], m_colors[i]);
	}
	
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
}


StealthGauge::StealthGauge()
	: HudItem()
	, m_texture(NULL)
	, m_visible(false)
{}

void StealthGauge::init() {
	m_texture = TextureContainer::LoadUI("graph/interface/icons/stealth_gauge");
	arx_assert(m_texture);
	m_size = Vec2f(32.f, 32.f);
}

void StealthGauge::update(const Rectf & parent) {
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

void StealthGauge::draw() {
	if(!m_visible)
		return;
	
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetBlendFunc(BlendOne, BlendOne);
	EERIEDrawBitmap(m_rect, 0.01f, m_texture, m_color);
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
}

bool PLAYER_INTERFACE_HIDE_COUNT = true;

PlayerInterfaceFader::PlayerInterfaceFader()
	: m_direction(0)
	, m_current(PlatformDuration_ZERO)
{}

void PlayerInterfaceFader::reset() {
	m_direction = 0;
	PLAYER_INTERFACE_HIDE_COUNT = true;
}

void PlayerInterfaceFader::resetSlid() {
	m_current = PlatformDuration_ZERO;
}

void PlayerInterfaceFader::requestFade(FadeDirection showhide, long smooth) {
	if(showhide == FadeDirection_Out) {
		InventoryOpenClose(2);
		ARX_INTERFACE_BookClose();
		ARX_INTERFACE_NoteClose();
	}
	
	if(showhide == FadeDirection_In)
		PLAYER_INTERFACE_HIDE_COUNT = true;
	else
		PLAYER_INTERFACE_HIDE_COUNT = false;
	
	if(smooth) {
		if(showhide == FadeDirection_In)
			m_direction = -1;
		else
			m_direction = 1;
	} else {
		if(showhide == FadeDirection_In)
			m_current = PlatformDuration_ZERO;
		else
			m_current = PlatformDurationMs(1000);
		
		lSLID_VALUE = float(toMs(m_current)) / 10.f;
	}
}

PlatformInstant SLID_START = PlatformInstant_ZERO; // Charging Weapon

void PlayerInterfaceFader::update() {
	
	if(PLAYER_INTERFACE_HIDE_COUNT && !m_direction) {
		bool bOk = true;
		
		if(TRUE_PLAYER_MOUSELOOK_ON) {
			if(!(player.Interface & INTER_COMBATMODE) && player.doingmagic != 2 && !InInventoryPos(DANAEMouse)) {
				bOk = false;
				
				PlatformInstant t = g_platformTime.frameStart();
				
				if(t - SLID_START > PlatformDurationMs(10000)) {
					m_current += g_platformTime.lastFrameDuration();
					
					if(m_current > PlatformDurationMs(1000))
						m_current = PlatformDurationMs(1000);
					
					lSLID_VALUE = float(toMs(m_current)) / 10.f;
				} else {
					bOk = true;
				}
			}
		}
		
		if(bOk) {
			m_current -= g_platformTime.lastFrameDuration();
			
			if(m_current < PlatformDuration_ZERO)
				m_current = PlatformDuration_ZERO;
			
			lSLID_VALUE = float(toMs(m_current)) / 10.f;
		}
	}
	
	if(m_direction == 1) {
		m_current += g_platformTime.lastFrameDuration();
		
		if(m_current > PlatformDurationMs(1000)) {
			m_current = PlatformDurationMs(1000);
			m_direction = 0;
		}
		lSLID_VALUE = float(toMs(m_current)) / 10.f;
	} else if(m_direction == -1) {
		m_current -= g_platformTime.lastFrameDuration();
		
		if(m_current < PlatformDuration_ZERO) {
			m_current = PlatformDuration_ZERO;
			m_direction = 0;
		}
		lSLID_VALUE = float(toMs(m_current)) / 10.f;
	}
}

static void setHudTextureState() {
	TextureStage::FilterMode filter = TextureStage::FilterLinear;
	if(config.interface.hudScaleFilter == UIFilterNearest) {
		filter = TextureStage::FilterNearest;
	}
	GRenderer->GetTextureStage(0)->setMinFilter(filter);
	GRenderer->GetTextureStage(0)->setMagFilter(filter);
	GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapClamp);
}

void HudRoot::draw() {
	
	if(player.lifePool.current <= 0) {
		return;
	}
	
	const Vec2f mousePos = Vec2f(DANAEMouse);
	
	Rectf hudSlider = Rectf(g_size);
	hudSlider.left  -= lSLID_VALUE;
	hudSlider.right += lSLID_VALUE;
	
	
	hitStrengthGauge.updateRect(Rectf(g_size));
	hitStrengthGauge.update();
	
	g_secondaryInventoryHud.update();
	g_playerInventoryHud.update();
	mecanismIcon.update();
	screenArrows.update();
	
	changeLevelIconGui.update(Rectf(g_size));
	memorizedRunesHud.update(changeLevelIconGui.rect());
	
	quickSaveIconGui.update();
	
	
	Vec2f anchorPos = g_playerInventoryHud.anchorPosition();
	
	Rectf spacer;
	spacer.left = std::max(g_secondaryInventoryHud.m_fadePosition + 160, healthGauge.rect().right);
	spacer.bottom = anchorPos.y;
	spacer.top = spacer.bottom - 30;
	spacer.right = spacer.left + 20;
	
	stealthGauge.update(spacer);
	
	damagedEquipmentGui.updateRect(stealthGauge.rect());
	damagedEquipmentGui.update();
	
	precastSpellsGui.updateRect(damagedEquipmentGui.rect());
	precastSpellsGui.update();
	
	setHudTextureState();
	
	if(player.Interface & INTER_COMBATMODE) {
		hitStrengthGauge.draw();
	}	
	
	g_secondaryInventoryHud.draw();
	g_playerInventoryHud.draw();
	
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
	
	healthGauge.updateRect(hudSlider);
	healthGauge.updateInput(mousePos);
	healthGauge.update();
	
	stealIconGui.updateRect(healthGauge.rect());
	
	damagedEquipmentGui.draw();
	
	if(!(player.Interface & INTER_COMBATMODE) && (player.Interface & INTER_MINIBACK)) {
		
		if(player.Interface & INTER_STEAL) {
			stealIconGui.draw();			
		}
	}
	
	currentTorchIconGui.updateRect(stealIconGui.rect());
	currentTorchIconGui.update();
	currentTorchIconGui.draw();
	
	changeLevelIconGui.draw();
	
	quickSaveIconGui.draw();
	stealthGauge.draw();

	if((player.Interface & INTER_MAP) && !(player.Interface & INTER_COMBATMODE)) {
		ARX_INTERFACE_ManageOpenedBook();
		
		GRenderer->SetRenderState(Renderer::DepthWrite, true);
		
		if((player.Interface & INTER_MAP) && !(player.Interface & INTER_COMBATMODE)) {
			if(g_guiBookCurrentTopTab == BOOKMODE_SPELLS) {
				gui::ARX_INTERFACE_ManageOpenedBook_Finish(mousePos);
				ARX_INTERFACE_ManageOpenedBook_SpellsDraw();
			}
		}
		
		setHudTextureState();
	}
	
	memorizedRunesHud.draw();
	
	if(player.Interface & INTER_LIFE_MANA) {
		manaGauge.update(hudSlider);
		manaGauge.updateInput(mousePos);
		manaGauge.draw();
		
		healthGauge.draw();
		
		if(bRenderInCursorMode) {
			GRenderer->SetRenderState(Renderer::AlphaBlending, true);
			GRenderer->SetBlendFunc(BlendOne, BlendOne);
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
	
	precastSpellsGui.draw();
	activeSpellsGui.update(hudSlider);
	activeSpellsGui.updateInput(mousePos);
	activeSpellsGui.draw();
	
	GRenderer->GetTextureStage(0)->setMinFilter(TextureStage::FilterLinear);
	GRenderer->GetTextureStage(0)->setMagFilter(TextureStage::FilterLinear);
	GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapRepeat);
}

void HudRoot::recalcScale() {
	
	float maxScale = minSizeRatio();
	float scale = glm::clamp(1.f, maxScale * config.interface.hudScale, maxScale);
	
	if(config.interface.hudScaleInteger && maxScale > 1.f) {
		if(scale < 1.3f || maxScale < 1.5f) {
			scale = 1.f;
		} else if(scale < 1.75f || maxScale < 2.f) {
			scale = 1.5f;
		} else {
			scale = std::floor(std::min(scale + 0.5f, maxScale));
		}
	}
	
	setScale(scale);
}


void HudRoot::setScale(float scale) {
	HudItem::setScale(scale);
	
	hitStrengthGauge.setScale(scale);
	healthGauge.setScale(scale);
	stealIconGui.setScale(scale);
	currentTorchIconGui.setScale(scale);
	
	manaGauge.setScale(scale);
	backpackIconGui.setScale(scale);
	bookIconGui.setScale(scale);
	purseIconGui.setScale(scale);
	levelUpIconGui.setScale(scale);
	
	changeLevelIconGui.setScale(scale);
	memorizedRunesHud.setScale(scale);
	activeSpellsGui.setScale(scale);
	
	mecanismIcon.setScale(scale);
	screenArrows.setScale(scale);
	
	stealthGauge.setScale(scale);
	damagedEquipmentGui.setScale(scale);
	precastSpellsGui.setScale(scale);
	
	g_playerInventoryHud.setScale(scale);
	g_secondaryInventoryHud.setScale(scale);
}

void HudRoot::init() {
	changeLevelIconGui.init();
	currentTorchIconGui.init();
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
	g_secondaryInventoryHud.init();
	g_playerInventoryHud.init();
	
	purseIconGui.init();
	
	hitStrengthGauge.init();
	
	//setHudScale(2);
}

void HudRoot::updateInput() {
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

HudRoot g_hudRoot;
