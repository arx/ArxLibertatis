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
#include "gui/Notification.h"
#include "gui/Speech.h"
#include "gui/book/Book.h"
#include "gui/hud/HudCommon.h"
#include "gui/hud/PlayerInventory.h"
#include "gui/hud/SecondaryInventory.h"

#include "input/Input.h"

#include "math/RandomVector.h"

#include "scene/GameSound.h"
#include "scene/Interactive.h"

extern bool WILLRETURNTOFREELOOK;

static const int indicatorVertSpacing = 30;
static const int indicatorHorizSpacing = 20;

static void DrawItemPrice(float scale) {
	
	Entity * temp = SecondaryInventory->io;
	if(temp->ioflags & IO_SHOP) {
		Vec2f pos = Vec2f(DANAEMouse);
		pos += Vec2f(60, -10) * scale;
		
		if(g_secondaryInventoryHud.containsPos(DANAEMouse)) {
			
			long amount = ARX_INTERACTIVE_GetPrice(FlyingOverIO, temp);
			// achat
			float famount = amount - amount * player.m_skillFull.intuition * 0.005f;
			// check should always be OK because amount is supposed positive
			amount = checked_range_cast<long>(famount);

			Color color = (amount <= player.gold) ? Color::green : Color::red;
			
			ARX_INTERFACE_DrawNumber(pos, amount, color, scale);
		} else if(g_playerInventoryHud.containsPos(DANAEMouse)) {
			long amount = ARX_INTERACTIVE_GetSellValue(FlyingOverIO, temp);
			if(amount) {
				Color color = Color::red;
				
				if(temp->shop_category.empty() ||
				   FlyingOverIO->groups.find(temp->shop_category) != FlyingOverIO->groups.end()) {

					color = Color::green;
				}
				ARX_INTERFACE_DrawNumber(pos, amount, color, scale);
			}
		}
	}
}

HitStrengthGauge::HitStrengthGauge()
	: m_emptyTex(NULL)
	, m_fullTex(NULL)
	, m_hitTex(NULL)
	, m_size(122.f, 70.f)
	, m_hitSize(172.f, 130.f)
	, m_hitRect(Rectf::ZERO)
	, m_intensity(0.f)
	, m_flashActive(false)
	, m_flashTime(0)
	, m_flashIntensity(0.f)
{ }

void HitStrengthGauge::init() {
	m_emptyTex = TextureContainer::LoadUI("graph/interface/bars/aim_empty");
	m_fullTex = TextureContainer::LoadUI("graph/interface/bars/aim_maxi");
	m_hitTex = TextureContainer::LoadUI("graph/interface/bars/flash_gauge");
	arx_assert(m_emptyTex);
	arx_assert(m_fullTex);
	arx_assert(m_hitTex);
}

void HitStrengthGauge::requestFlash(float flashIntensity) {
	m_flashActive = true;
	m_flashTime = 0;
	m_flashIntensity = flashIntensity;
}

void HitStrengthGauge::updateRect(const Rectf & parent) {
	m_rect = createChild(parent, Anchor_BottomCenter, m_size * m_scale, Anchor_BottomCenter);
	m_rect.move(0.f, -2.f);
	
	m_hitRect = createChild(m_rect, Anchor_Center, m_hitSize * m_scale, Anchor_Center);
}

void HitStrengthGauge::update() {
	
	if(!player.isAiming()) {
		m_intensity = 0.2f;
	} else {
		float j;
		if(player.m_bowAimRatio > 0) {
			j = player.m_bowAimRatio;
		} else {
			j = player.m_aimTime / player.Full_AimTime;
		}
		m_intensity = glm::clamp(j, 0.2f, 1.f);
	}
	
	if(m_flashActive) {
		m_flashTime += g_platformTime.lastFrameDuration();
		if(m_flashTime >= PlatformDurationMs(500)) {
			m_flashActive = false;
			m_flashTime = 0;
		}
	}
}

void HitStrengthGauge::draw() {
	
	{
		UseRenderState state(render2D().blendAdditive());
		EERIEDrawBitmap(m_rect, 0.0001f, m_fullTex, Color::gray(m_intensity));
	}
	
	EERIEDrawBitmap(m_rect, 0.0001f, m_emptyTex, Color::white);
	
	if(m_flashActive && player.m_skillFull.etheralLink >= 40) {
		
		float j = 1.0f - m_flashIntensity;
		Color col = (j < 0.5f) ? Color::rgb(j * 2.f, 1.f, 0.f) : Color::rgb(1.f, m_flashIntensity, 0.f);
		
		UseRenderState state(render2D().blendAdditive());
		EERIEDrawBitmap(m_hitRect, 0.0001f, m_hitTex, col);
		
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
	: m_size(Vec2f(32, 32))
	, ulBookHaloTime(0)
{ }

void BookIconGui::init() {
	m_tex = TextureContainer::LoadUI("graph/interface/icons/book");
	arx_assert(m_tex);
	
	m_size = Vec2f(32, 32);
	
	m_haloColor = Color3f(0.2f, 0.4f, 0.8f);
	
	m_haloActive = false;
	ulBookHaloTime = 0;
}

void BookIconGui::requestHalo() {
	m_haloActive = true;
	ulBookHaloTime = 0;
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
		cursorSetInteraction();

		if(eeMouseDown1()) {
			g_playerBook.toggle();
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
	
	static PlatformInstant flDelay = 0;
	
	// Check for backpack Icon
	if(m_rect.contains(Vec2f(DANAEMouse))) {
		if(eeMouseUp1() && playerInventory.insert(DRAGINTER)) {
			ARX_SOUND_PlayInterface(g_snd.INVSTD);
			Set_DragInter(NULL);
		}
	}
	
	if(m_rect.contains(Vec2f(DANAEMouse)) || flDelay != 0) {
		eMouseState = MOUSE_IN_INVENTORY_ICON;
		cursorSetInteraction();
		
		
		if(eeMouseDoubleClick1()) {
			ARX_SOUND_PlayInterface(g_snd.BACKPACK, Random::getf(0.9f, 1.1f));
			
			playerInventory.optimize();
			
			flDelay = 0;
		} else if(eeMouseDown1() || flDelay != 0) {
			if(flDelay == 0) {
				flDelay = g_platformTime.frameStart();
				return;
			}
			if(g_platformTime.frameStart() - flDelay < PlatformDurationMs(300)) {
				return;
			}
			flDelay = 0;
			
			if(player.Interface & INTER_INVENTORYALL) {
				ARX_SOUND_PlayInterface(g_snd.BACKPACK, Random::getf(0.9f, 1.1f));
				g_playerInventoryHud.close();
			} else {
				bInverseInventory = !bInverseInventory;
				lOldTruePlayerMouseLook = TRUE_PLAYER_MOUSELOOK_ON;
			}
		} else if(eeMouseDown2()) {
			g_playerBook.close();
			ARX_INVENTORY_OpenClose(NULL);
			
			if(player.Interface & INTER_INVENTORYALL) {
				g_playerInventoryHud.close();
			} else {
				if(player.Interface & INTER_INVENTORY) {
					ARX_SOUND_PlayInterface(g_snd.BACKPACK, Random::getf(0.9f, 1.1f));
					g_playerInventoryHud.close();
					bInventorySwitch = true;
				} else {
					ARX_SOUND_PlayInterface(g_snd.BACKPACK, Random::getf(0.9f, 1.1f));
					player.Interface |= INTER_INVENTORYALL;
					
					g_playerInventoryHud.resetPos();
					
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
			
			eMouseState = MOUSE_IN_STEAL_ICON;
			cursorSetInteraction();
			
			if(eeMouseDown1()) {
				ARX_INVENTORY_OpenClose(ioSteal);
				
				if(player.Interface & (INTER_INVENTORY | INTER_INVENTORYALL)) {
					ARX_SOUND_PlayInterface(g_snd.BACKPACK, Random::getf(0.9f, 1.1f));
				}
				
				if(SecondaryInventory) {
					SendIOScriptEvent(entities.player(), ioSteal, SM_STEAL);
					bForceEscapeFreeLook = true;
					lOldTruePlayerMouseLook = !TRUE_PLAYER_MOUSELOOK_ON;
				}
			}
			
			if(DRAGINTER == NULL) {
				return;
			}
			
		}
	}
	
}

void StealIconGui::draw() {
	m_isSelected = eMouseState == MOUSE_IN_STEAL_ICON;
	HudIconBase::draw();
}


LevelUpIconGui::LevelUpIconGui()
	: m_size(32.f, 32.f)
	, m_visible(false)
{ }

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
		cursorSetInteraction();
		
		if(eeMouseDown1()) {
			g_playerBook.open();
		}
	}
}

void LevelUpIconGui::draw() {
	if(!m_visible)
		return;
	
	HudIconBase::draw();
}

PurseIconGui::PurseIconGui()
	: m_size(32.f, 32.f)
	, m_haloTime(0)
{ }

void PurseIconGui::init() {
	m_tex = TextureContainer::LoadUI("graph/interface/inventory/gold");
	arx_assert(m_tex);
	m_haloColor = Color3f(0.9f, 0.9f, 0.1f);
	m_haloActive = false;
}

void PurseIconGui::requestHalo() {
	m_haloActive = true;
	m_haloTime = 0;
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
			cursorSetInteraction();
			
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
		Vec2f numberPos = m_rect.topRight() + Vec2f(0.f, -15 * m_scale);
		ARX_INTERFACE_DrawNumber(numberPos, player.gold, Color::white, m_scale);
	}
	
}

CurrentTorchIconGui::CurrentTorchIconGui()
	: m_isActive(false)
	, m_tex(NULL)
	, m_size(32.f, 64.f)
{ }

void CurrentTorchIconGui::init() { }

bool CurrentTorchIconGui::isVisible() {
	return !(player.Interface & INTER_COMBATMODE) && player.torch;
}

void CurrentTorchIconGui::updateRect(const Rectf & parent) {
	
	m_rect = createChild(parent, Anchor_TopLeft, m_size * m_scale, Anchor_BottomLeft);
	
	if(g_secondaryInventoryHud.rect().overlaps(m_rect)) {
		m_rect.move(g_secondaryInventoryHud.rect().right, 0.f);
	}
}

void CurrentTorchIconGui::updateInput() {
	if(player.torch) {
		
		if(m_rect.contains(Vec2f(DANAEMouse))) {
			eMouseState = MOUSE_IN_TORCH_ICON;
			cursorSetInteraction();
			
			if(!DRAGINTER && !PLAYER_MOUSELOOK_ON && DRAGGING) {
				Entity * io = player.torch;
				player.torch->show = SHOW_FLAG_IN_SCENE;
				
				ARX_SOUND_PlaySFX(g_snd.TORCH_END);
				ARX_SOUND_Stop(player.torch_loop);
				player.torch_loop = audio::SourcedSample();
				
				player.torch = NULL;
				lightHandleGet(torchLightHandle)->m_exists = false;
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
	
	if(g_note.isOpen() && TSecondaryInventory != NULL
	   && (g_note.type() == Note::BigNote || g_note.type() == Note::Book)) {
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
	
	Vec2f pos = m_rect.topLeft() + arx::linearRand(Vec2f(9.f, 0.f), Vec2f(12.f, 6.f)) * m_scale;
	spawn2DFireParticle(pos, m_scale);
}

void CurrentTorchIconGui::draw() {
	
	if(!isVisible())
		return;
	
	EERIEDrawBitmap(m_rect, 0.001f, m_tex, Color::white);
}

ChangeLevelIconGui::ChangeLevelIconGui()
	: m_tex(NULL)
	, m_size(32.f, 32.f)
	, m_intensity(1.f)
{ }

void ChangeLevelIconGui::init() {
	m_tex = TextureContainer::LoadUI("graph/interface/icons/change_lvl");
	arx_assert(m_tex);
}

bool ChangeLevelIconGui::isVisible() {
	return CHANGE_LEVEL_ICON != NoChangeLevel;
}

void ChangeLevelIconGui::update(const Rectf & parent) {
	m_rect = createChild(parent, Anchor_TopRight, m_size * m_scale, Anchor_TopRight);
	
	float wave = timeWaveSin(g_gameTime.now(), GameDurationMsf(314.159f));
	m_intensity = 0.9f - wave * 0.5f + Random::getf(0.f, 0.1f);
	m_intensity = glm::clamp(m_intensity, 0.f, 1.f);
}

void ChangeLevelIconGui::draw() {
	
	if(!isVisible())
		return;
	
	EERIEDrawBitmap(m_rect, 0.0001f, m_tex, Color::gray(m_intensity));
	
	if(m_rect.contains(Vec2f(DANAEMouse))) {
		cursorSetInteraction();
		if(eeMouseUp1()) {
			CHANGE_LEVEL_ICON = ChangeLevelNow;
		}
	}
}


QuickSaveIconGui::QuickSaveIconGui()
	: m_duration(GameDurationMs(1000))
	, m_remainingTime(0)
{}

void QuickSaveIconGui::show() {
	m_remainingTime = m_duration;
}

void QuickSaveIconGui::hide() {
	m_remainingTime = 0;
}

void QuickSaveIconGui::update() {
	if(m_remainingTime != 0) {
		if(m_remainingTime > g_gameTime.lastFrameDuration()) {
			m_remainingTime -= g_gameTime.lastFrameDuration();
		} else {
			m_remainingTime = 0;
		}
	}
}

void QuickSaveIconGui::draw() {
	
	if(m_remainingTime == 0) {
		return;
	}
	
	UseRenderState state(render2D().blend(BlendSrcColor, BlendOne).alphaCutout());
	
	// Flash the icon twice, starting at about 0.7 opacity
	float step = 1.f - (m_remainingTime / m_duration);
	float alpha = std::min(1.f, 0.6f * (std::sin(step * (7.f / 2.f * glm::pi<float>())) + 1.f));
	
	TextureContainer * tex = TextureContainer::LoadUI("graph/interface/icons/menu_main_save");
	arx_assert(tex);
	
	Vec2f size = Vec2f(tex->size());
	EERIEDrawBitmap(Rectf(Vec2f(0, 0), size.x, size.y), 0.f, tex, Color::gray(alpha));
	
}

MemorizedRunesHud::MemorizedRunesHud()
	: m_size(0.f)
	, m_count(0)
{ }

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
				for(int j = i + 1; j < 6; j++) {
					player.SpellToMemorize.iSpellSymbols[j] = RUNE_NONE;
				}
			}
		}
		if(player.SpellToMemorize.iSpellSymbols[i] != RUNE_NONE) {
			
			Vec2f size = Vec2f(32.f, 32.f) * m_scale;
			Rectf rect = Rectf(pos, size.x, size.y);
			
			TextureContainer * tc = gui::necklace.pTexTab[player.SpellToMemorize.iSpellSymbols[i]];
			
			if(bHalo) {
				ARX_INTERFACE_HALO_Render(Color3f(0.2f, 0.4f, 0.8f), HALO_ACTIVE, tc->getHalo(), pos, Vec2f(m_scale));
			}
			
			EERIEDrawBitmap(rect, 0, tc, Color::white);
			
			if(!player.hasRune(player.SpellToMemorize.iSpellSymbols[i])) {
				UseRenderState state(render2D().blend(BlendInvDstColor, BlendOne).alphaCutout());
				EERIEDrawBitmap(rect, 0, cursorMovable, Color::gray(0.8f));
			}
			
			pos.x += 32 * m_scale;
		}
	}
	if(g_gameTime.now() - player.SpellToMemorize.lTimeCreation > GameDurationMs(30000)) {
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
	
	// Red gauge texures have a 2 pixel gap at the bottom,
	// tweak the position to hide it
	m_rect.move(0, 2.f * m_scale);
}

void HealthGauge::update() {
	
	m_amount = player.lifePool.current / player.Full_maxlife;
	
	if(player.poison > 0.f) {
		float val = std::min(player.poison, 0.2f) * 5.f;
		m_color = Color::rgb(1.f - val, val, 0.f);
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
				notification_add(ss.str());
			}
		}
	}
}

void HealthGauge::draw() {
	
	EERIEDrawBitmap2DecalY(m_rect, 0.f, m_filledTex, m_color, (1.f - m_amount));
	EERIEDrawBitmap(m_rect, 0.001f, m_emptyTex, Color::white);
}


ManaGauge::ManaGauge()
	: m_size(33.f, 80.f)
	, m_emptyTex(NULL)
	, m_filledTex(NULL)
	, m_amount(0.f)
{ }

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
				notification_add(ss.str());
			}
		}
	}
}

void ManaGauge::draw() {
	
	EERIEDrawBitmap2DecalY(m_rect, 0.f, m_filledTex, Color::white, (1.f - m_amount));
	EERIEDrawBitmap(m_rect, 0.001f, m_emptyTex, Color::white);
}


MecanismIcon::MecanismIcon()
	: m_iconSize(32.f, 32.f)
	, m_tex(NULL)
	, m_timeToDraw(0)
	, m_nbToDraw(0)
{ }

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
	if(m_timeToDraw > GameDurationMs(300)) {
		m_color = Color::black;
		if(m_timeToDraw > GameDurationMs(400)) {
			m_timeToDraw = 0;
			m_nbToDraw++;
		}
	}
	m_timeToDraw += g_gameTime.lastFrameDuration();
	
	m_rect = createChild(Rectf(g_size), Anchor_TopLeft, m_iconSize * m_scale, Anchor_TopLeft);
}

void MecanismIcon::draw() {
	
	if(m_nbToDraw >= 3) {
		return;
	}
	
	UseRenderState state(render2D().blendAdditive());
	
	EERIEDrawBitmap(m_rect, 0.01f, m_tex, m_color);
}


ScreenArrows::ScreenArrows()
	: m_horizontalArrowSize(8, 16)
	, m_verticalArrowSize(16, 8)
	, m_arrowLeftTex(NULL)
	, fArrowMove(0.f)
{ }

void ScreenArrows::init() {
	m_arrowLeftTex = TextureContainer::LoadUI("graph/interface/icons/arrow_left");
	arx_assert(m_arrowLeftTex);
}

void ScreenArrows::update() {
	
	if(!config.input.borderTurning) {
		return;
	}
	
	fArrowMove += .5f * toMs(g_platformTime.lastFrameDuration());
	if(fArrowMove > 180.f) {
		fArrowMove = 0.f;
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
	
	UseRenderState state(render2D().blendAdditive());
	
	Color lcolor = Color::gray(0.5f);
	
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
		cursorSetInteraction();
		
		if(eeMouseUp1()) {
			if(Precast[m_precastIndex.handleData()].typ >= 0) {
				notification_add(spellicons[Precast[m_precastIndex.handleData()].typ].name);
			}
		}
		
		if(eeMouseDoubleClick1()) {
			ARX_SPELLS_Precast_Launch(m_precastIndex);
		}
	}
}

void PrecastSpellsGui::PrecastSpellIconSlot::draw() {
	EERIEDrawBitmap(m_rect, 0.01f, m_tc, m_color);
}

PrecastSpellsGui::PrecastSpellsGui()
	: m_iconSize(24.f, 24.f)
{ }

bool PrecastSpellsGui::isVisible() {
	return !(player.Interface & INTER_PLAYERBOOK);
}

void PrecastSpellsGui::updateRect(const Rectf & parent) {
	
	Vec2f size = m_iconSize * Vec2f(Precast.size(), 1);
	
	m_rect = createChild(parent, Anchor_BottomRight, size * m_scale, Anchor_BottomLeft);
	
	if(g_playerInventoryHud.rect().overlaps(m_rect + Vec2f(0.0f, indicatorVertSpacing))) {
		m_rect.move(0.0f, g_playerInventoryHud.rect().top - parent.bottom - indicatorVertSpacing);
	}
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
		
		if(precastSlot.launch_time > 0 && g_gameTime.now() >= precastSlot.launch_time) {
			float tt = (g_gameTime.now() - precastSlot.launch_time) / GameDurationMs(1000);
			
			if(tt > 1.f)
				tt = 1.f;
			
			val *= (1.f - tt);
		}
		
		Color color = Color::rgb(0, val * 0.5f, val);
		
		Rectf childRect = createChild(m_rect, Anchor_BottomLeft, m_iconSize * m_scale, Anchor_BottomLeft);
		childRect.move(i * m_iconSize.x * m_scale, 0);
		
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
	
	UseRenderState state(render2D().blendAdditive());
	
	std::vector<PrecastSpellIconSlot>::iterator itr;
	for(itr = m_icons.begin(); itr != m_icons.end(); ++itr) {
		itr->draw();
	}
}


void ActiveSpellsGui::ActiveSpellIconSlot::updateInput(const Vec2f & mousePos) {
	
	if(!m_abortable)
		return;
	
	if(m_rect.contains(mousePos)) {
		cursorSetInteraction();
		
		if(eeMouseUp1()) {
			if(spells[spellIndex]->m_type >= 0) {
				notification_add(spellicons[spells[spellIndex]->m_type].name);
			}
		}
		
		if(eeMouseDoubleClick1()) {
			ARX_SOUND_PlaySFX(g_snd.MAGIC_FIZZLE);
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
	: m_texUnknown(NULL)
	, m_slotSize(24.f, 24.f)
	, m_spacerSize(60.f, 50.f)
	, m_slotSpacerSize(0.f, 9.f)
	, m_flickNow(false)
	, m_flickTime(0)
	, m_flickInterval(PlatformDurationMsf(1000.0f / 60.0f))
{ }

void ActiveSpellsGui::init() {
	m_texUnknown = TextureContainer::Load("graph/interface/icons/spell_unknown");
	arx_assert(m_texUnknown);
}

void ActiveSpellsGui::update(const Rectf & parent) {
	
	float intensity = 1.f - PULSATE * 0.5f;
	intensity = glm::clamp(intensity, 0.f, 1.f);
	
	m_flickTime += g_platformTime.lastFrameDuration();
	
	if(m_flickTime >= m_flickInterval) {
		m_flickNow = !m_flickNow;
		while(m_flickTime >= m_flickInterval) {
			m_flickTime -= m_flickInterval;
		}
	}
	
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
	
	UseRenderState state(render2D().blendAdditive());
	
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
		if(!spell) {
			continue;
		}
		
		if(std::find(spell->m_targets.begin(), spell->m_targets.end(), EntityHandle_Player) == spell->m_targets.end()) {
			continue;
		}
		
		if(spell->m_caster != EntityHandle_Player && spellicons[spell->m_type].m_hasDuration) {
			ManageSpellIcon(*spell, intensity, true);
		}
	}
}

void ActiveSpellsGui::ManageSpellIcon(SpellBase & spell, float intensity, bool flag) {
	
	Color color = (flag) ? Color::rgb(intensity, 0, 0) : Color::gray(intensity);
	
	bool flicker = true;
	
	if(spell.m_hasDuration) {
		if(player.manaPool.current < 20 || spell.m_duration - spell.m_elapsed < GameDurationMs(2000)) {
			flicker = m_flickNow;
		}
	} else {
		if(player.manaPool.current < 20) {
			flicker = m_flickNow;
		}
	}
	
	if(spell.m_type >= 0 && size_t(spell.m_type) < SPELL_TYPES_COUNT) {
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
	: m_size(64.f, 64.f)
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
	
	if(g_playerInventoryHud.rect().overlaps(m_rect + Vec2f(0.0f, indicatorVertSpacing))) {
		m_rect.move(0.0f, g_playerInventoryHud.rect().top - parent.bottom - indicatorVertSpacing);
	}
	
	if(g_secondaryInventoryHud.rect().overlaps(m_rect  + Vec2f(0.0f, -indicatorHorizSpacing))) {
		m_rect.move(g_secondaryInventoryHud.rect().right - m_rect.left + indicatorHorizSpacing, 0.0f);
	}
}

void DamagedEquipmentGui::update() {
	if(cinematicBorder.isActive() || BLOCK_PLAYER_CONTROLS) {
		return;
	}
		
	if(player.Interface & INTER_INVENTORYALL) {
		return;
	}
	
	for(long i = 0; i < 5; i++) {
		
		m_colors[i] = Color::black;
		
		long eq = -1;
		switch (i) {
			case 0: eq = EQUIP_SLOT_WEAPON; break;
			case 1: eq = EQUIP_SLOT_SHIELD; break;
			case 2: eq = EQUIP_SLOT_HELMET; break;
			case 3: eq = EQUIP_SLOT_ARMOR; break;
			case 4: eq = EQUIP_SLOT_LEGGINGS; break;
			default: arx_unreachable();
		}
		
		Entity * io = entities.get(player.equiped[eq]);
		if(io) {
			float ratio = io->durability / io->max_durability;
			if(ratio <= 0.5f) {
				m_colors[i] = Color::rgb(1.f - ratio, ratio, 0);
			}
		}
		
	}
}

void DamagedEquipmentGui::draw() {
	
	UseRenderState state(render2D().blendAdditive());
	
	for(long i = 0; i < 5; i++) {
		if(m_colors[i] == Color::black) {
			continue;
		}
		
		EERIEDrawBitmap(m_rect, 0.001f, iconequip[i], m_colors[i]);
	}
	
}

StealthGauge::StealthGauge()
	: m_texture(NULL)
	, m_visible(false)
	, m_color(Color::none)
	, m_size(32.f, 32.f)
{ }

void StealthGauge::init() {
	m_texture = TextureContainer::LoadUI("graph/interface/icons/stealth_gauge");
	arx_assert(m_texture);
}

void StealthGauge::updateRect(const Rectf & parent) {
	m_rect = createChild(parent, Anchor_TopRight, m_size * m_scale, Anchor_BottomLeft);
	
	if(g_playerInventoryHud.rect().overlaps(m_rect + Vec2f(0.0f, indicatorVertSpacing))) {
		m_rect.move(0.0f, g_playerInventoryHud.rect().top - parent.top - indicatorVertSpacing);
	}
	
	if(g_secondaryInventoryHud.rect().overlaps(m_rect  + Vec2f(0.0f, -indicatorHorizSpacing))) {
		m_rect.move(g_secondaryInventoryHud.rect().right - m_rect.left + indicatorHorizSpacing, 0.0f);
	}
}

void StealthGauge::update() {
	
	m_visible = false;
	
	if(!cinematicBorder.isActive()) {
		float v = GetPlayerStealth();
		
		if(CURRENT_PLAYER_COLOR < v) {
			float t = v - CURRENT_PLAYER_COLOR;
			
			if(t >= 15) {
				v = 1.f;
			} else {
				v = (t * (1.0f / 15)) * 0.9f + 0.1f;
			}
			
			m_color = Color::gray(v);
			
			m_visible = true;
		}
	}
}

void StealthGauge::draw() {
	
	if(!m_visible) {
		return;
	}
	
	UseRenderState state(render2D().blendAdditive());
	
	EERIEDrawBitmap(m_rect, 0.01f, m_texture, m_color);
}

bool PLAYER_INTERFACE_SHOW = true;

PlayerInterfaceFader::PlayerInterfaceFader()
	: m_direction(0)
	, m_current(0)
{}

void PlayerInterfaceFader::reset() {
	m_direction = 0;
	PLAYER_INTERFACE_SHOW = true;
}

void PlayerInterfaceFader::resetSlid() {
	m_current = 0;
}

void PlayerInterfaceFader::requestFade(FadeDirection showhide, long smooth) {
	if(showhide == FadeDirection_Out) {
		InventoryOpenClose(2);
		g_playerBook.close();
		ARX_INTERFACE_NoteClose();
	}
	
	PLAYER_INTERFACE_SHOW = (showhide == FadeDirection_In);
	
	if(smooth) {
		if(showhide == FadeDirection_In) {
			m_direction = -1;
		} else {
			m_direction = 1;
		}
	} else {
		if(showhide == FadeDirection_In) {
			m_current = 0;
		} else {
			m_current = PlatformDurationMs(1000);
		}
		
		lSLID_VALUE = m_current / PlatformDurationMs(10);
	}
}

PlatformInstant SLID_START = 0; // Charging Weapon

void PlayerInterfaceFader::update() {
	
	if(PLAYER_INTERFACE_SHOW && !m_direction) {
		bool bOk = true;
		
		if(TRUE_PLAYER_MOUSELOOK_ON) {
			if(!(player.Interface & INTER_COMBATMODE) && player.doingmagic != 2 && !InInventoryPos(DANAEMouse)) {
				bOk = false;
				
				PlatformInstant t = g_platformTime.frameStart();
				
				if(t - SLID_START > PlatformDurationMs(10000)) {
					m_current += g_platformTime.lastFrameDuration();
					
					if(m_current > PlatformDurationMs(1000))
						m_current = PlatformDurationMs(1000);
					
					lSLID_VALUE = m_current / PlatformDurationMs(10);
				} else {
					bOk = true;
				}
			}
		}
		
		if(bOk) {
			m_current -= g_platformTime.lastFrameDuration();
			
			if(m_current < 0) {
				m_current = 0;
			}
			
			lSLID_VALUE = m_current / PlatformDurationMs(10);
		}
	}
	
	if(m_direction == 1) {
		m_current += g_platformTime.lastFrameDuration();
		
		if(m_current > PlatformDurationMs(1000)) {
			m_current = PlatformDurationMs(1000);
			m_direction = 0;
		}
		lSLID_VALUE = m_current / PlatformDurationMs(10);
	} else if(m_direction == -1) {
		m_current -= g_platformTime.lastFrameDuration();
		
		if(m_current < 0) {
			m_current = 0;
			m_direction = 0;
		}
		lSLID_VALUE = m_current / PlatformDurationMs(10);
	}
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
	
	g_secondaryInventoryHud.updateFader();
	g_secondaryInventoryHud.updateRect();
	g_secondaryInventoryHud.update();
	g_playerInventoryHud.updateRect();
	g_playerInventoryHud.update();
	mecanismIcon.update();
	screenArrows.update();
	
	changeLevelIconGui.update(Rectf(g_size));
	memorizedRunesHud.update(changeLevelIconGui.rect());
	
	quickSaveIconGui.update();
	
	{
		Rectf spacer;
		spacer.left = healthGauge.rect().right;
		spacer.bottom = float(g_size.bottom);
		spacer.top = spacer.bottom - indicatorVertSpacing;
		spacer.right = spacer.left + indicatorHorizSpacing;
		stealthGauge.updateRect(spacer);
	}
	stealthGauge.update();
	
	damagedEquipmentGui.updateRect(stealthGauge.rect());
	if(player.torch && damagedEquipmentGui.rect().overlaps(currentTorchIconGui.rect())) {
		Vec2f offset = Vec2f(currentTorchIconGui.rect().right - stealthGauge.rect().right + indicatorHorizSpacing,
		                     0.0f);
		damagedEquipmentGui.updateRect(stealthGauge.rect() + offset);
	}
	damagedEquipmentGui.update();
	
	precastSpellsGui.updateRect(damagedEquipmentGui.rect());
	precastSpellsGui.update();
	
	UseRenderState state(render2D());
	UseTextureState textureState(getInterfaceTextureFilter(), TextureStage::WrapClamp);
	
	if(player.Interface & INTER_COMBATMODE) {
		hitStrengthGauge.draw();
	}
	
	g_secondaryInventoryHud.draw();
	g_playerInventoryHud.draw();
	
	if(FlyingOverIO  && !(player.Interface & INTER_COMBATMODE)
	   && !GInput->actionPressed(CONTROLS_CUST_MAGICMODE)
	   && (!PLAYER_MOUSELOOK_ON || config.input.autoReadyWeapon != AlwaysAutoReadyWeapon)) {
		if((FlyingOverIO->ioflags & IO_ITEM) && !DRAGINTER && SecondaryInventory) {
			DrawItemPrice(m_scale);
		}
		cursorSetInteraction();
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

	if((player.Interface & INTER_PLAYERBOOK) && !(player.Interface & INTER_COMBATMODE)) {
		g_playerBook.manage();
	}
	
	memorizedRunesHud.draw();
	
	if(player.Interface & INTER_LIFE_MANA) {
		manaGauge.update(hudSlider);
		manaGauge.updateInput(mousePos);
		manaGauge.draw();
		
		healthGauge.draw();
		
		if(bRenderInCursorMode) {
			if(!MAGICMODE) {
				mecanismIcon.draw();
			}
			screenArrows.draw();
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
		
		levelUpIconGui.draw();
		
		// Draw/Manage Gold Purse Icon
		if(player.gold > 0) {
			purseIconGui.draw();
		}
		
	}
	
	precastSpellsGui.draw();
	activeSpellsGui.update(hudSlider);
	activeSpellsGui.updateInput(mousePos);
	activeSpellsGui.draw();
	
}

void HudRoot::recalcScale() {
	setScale(getInterfaceScale(config.interface.hudScale, config.interface.hudScaleInteger));
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

float HudRoot::getScale() {
	return m_scale;
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
