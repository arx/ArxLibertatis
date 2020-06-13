/*
 * Copyright 2013-2020 Arx Libertatis Team (see the AUTHORS file)
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

#include "gui/Cursor.h"

#include <iomanip>
#include <sstream>

#include "core/Core.h"
#include "core/Application.h"
#include "core/Config.h"
#include "core/GameTime.h"

#include "input/Input.h"

#include "game/Player.h"
#include "game/Inventory.h"
#include "game/Item.h"
#include "game/NPC.h"

#include "math/Angle.h"
#include "math/Rectangle.h"

#include "graphics/BaseGraphicsTypes.h"
#include "graphics/Math.h"
#include "graphics/Draw.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/texture/TextureStage.h"
#include "graphics/Renderer.h"

#include "scene/Interactive.h"
#include "scene/GameSound.h"

#include "gui/Dragging.h"
#include "gui/Hud.h"
#include "gui/Interface.h"
#include "gui/Text.h"
#include "gui/Menu.h"

enum ARX_INTERFACE_CURSOR_MODE
{
	CURSOR_UNDEFINED,
	CURSOR_INTERACTION_ON,
	CURSOR_REDIST,
	CURSOR_COMBINEON,
	CURSOR_COMBINEOFF,
	CURSOR_READY_WEAPON
};

ARX_INTERFACE_CURSOR_MODE SpecialCursor = CURSOR_UNDEFINED;
// Used to redist points - attributes and skill
static long lCursorRedistValue = 0;


bool cursorIsSpecial() {
	return SpecialCursor != CURSOR_UNDEFINED;
}

void cursorSetInteraction() {
	SpecialCursor = CURSOR_INTERACTION_ON;
}

void cursorSetRedistribute(long value) {
	SpecialCursor = CURSOR_REDIST;
	lCursorRedistValue = value;
}

static TextureContainer * cursorTargetOn = NULL;
static TextureContainer * cursorTargetOff = NULL;
static TextureContainer * cursorInteractionOn = NULL;
static TextureContainer * cursorInteractionOff = NULL;
static TextureContainer * cursorMagic = NULL;
static TextureContainer * cursorThrowObject = NULL;
static TextureContainer * cursorRedist = NULL;
static TextureContainer * cursorCrossHair = NULL; // Animated Hand Cursor TC
static TextureContainer * cursorReadyWeapon = NULL;
TextureContainer * cursorMovable = NULL;   // TextureContainer for Movable Items (Red Cross)

TextureContainer * scursor[8]; // Animated Hand Cursor

void cursorTexturesInit() {
	
	cursorTargetOn       = TextureContainer::LoadUI("graph/interface/cursors/target_on");
	cursorTargetOff      = TextureContainer::LoadUI("graph/interface/cursors/target_off");
	cursorInteractionOn  = TextureContainer::LoadUI("graph/interface/cursors/interaction_on");
	cursorInteractionOff = TextureContainer::LoadUI("graph/interface/cursors/interaction_off");
	cursorMagic          = TextureContainer::LoadUI("graph/interface/cursors/magic");
	cursorThrowObject    = TextureContainer::LoadUI("graph/interface/cursors/throw");
	cursorRedist         = TextureContainer::LoadUI("graph/interface/cursors/add_points");
	cursorCrossHair      = TextureContainer::LoadUI("graph/interface/cursors/cruz");
	cursorMovable        = TextureContainer::LoadUI("graph/interface/cursors/wrong");
	cursorReadyWeapon    = TextureContainer::LoadUI("graph/interface/icons/equipment_sword");
	
	arx_assert(cursorTargetOn);
	arx_assert(cursorTargetOff);
	arx_assert(cursorInteractionOn);
	arx_assert(cursorInteractionOff);
	arx_assert(cursorMagic);
	arx_assert(cursorThrowObject);
	arx_assert(cursorRedist);
	arx_assert(cursorCrossHair);
	
	std::ostringstream oss;
	for(size_t i = 0; i < 8; i++) {
		oss.str(std::string());
		oss << "graph/interface/cursors/cursor" << std::setfill('0') << std::setw(2) << i;
		scursor[i] = TextureContainer::LoadUI(oss.str());
		arx_assert(scursor[i]);
	}
	
	// TODO currently unused
	TextureContainer::LoadUI("graph/interface/cursors/cursor");
	TextureContainer::LoadUI("graph/interface/cursors/drop");
}

extern long LOOKING_FOR_SPELL_TARGET;
extern GameInstant LOOKING_FOR_SPELL_TARGET_TIME;

int iHighLight = 0;
float fHighLightAng = 0.f;

static bool SelectSpellTargetCursorRender() {
	
	if(LOOKING_FOR_SPELL_TARGET) {
		
		GameDuration elapsed = g_gameTime.now() - LOOKING_FOR_SPELL_TARGET_TIME;
		if(elapsed > GameDurationMs(7000)) {
			ARX_SOUND_PlaySFX(g_snd.MAGIC_FIZZLE, &player.pos);
			ARX_SPELLS_CancelSpellTarget();
		}
		
		TextureContainer * surf;
		
		if(FlyingOverIO
			&& (((LOOKING_FOR_SPELL_TARGET & 1) && (FlyingOverIO->ioflags & IO_NPC))
			||  ((LOOKING_FOR_SPELL_TARGET & 2) && (FlyingOverIO->ioflags & IO_ITEM)))
		){
			surf = cursorTargetOn;
			
			if(eeMouseUp1()) {
				ARX_SPELLS_LaunchSpellTarget(FlyingOverIO);
			}
		} else {
			surf = cursorTargetOff;
			
			if(GInput->actionPressed(CONTROLS_CUST_MAGICMODE)) {
				ARX_SOUND_PlaySFX(g_snd.MAGIC_FIZZLE, &player.pos);
				ARX_SPELLS_CancelSpellTarget();
			}
		}
		
		Vec2f pos = Vec2f(DANAEMouse);
		
		if(TRUE_PLAYER_MOUSELOOK_ON) {
			pos = MemoMouse;
		}
		
		Vec2f texSize = Vec2f(surf->size());
		pos += -texSize * 0.5f;
		
		EERIEDrawBitmap(Rectf(pos, texSize.x, texSize.y), 0.f, surf, Color::white);
		
		return true;
	}
	
	return false;
}


class CursorAnimatedHand {
private:
	PlatformDuration m_time;
	long m_frame;
	PlatformDuration m_delay;
	
public:
	CursorAnimatedHand()
		: m_time(0)
		, m_frame(0)
		, m_delay(PlatformDurationMs(70))
	{}
	
	void reset() {
		m_frame = 0;
	}
	
	void update1() {
		
		m_time += g_platformTime.lastFrameDuration();
		
		if(m_frame != 3) {
			while(m_time > m_delay) {
				m_time -= m_delay;
				m_frame++;
			}
		}
		
		if(m_frame > 7) {
			m_frame = 0;
		}
		
	}
	
	void update2() {
		
		if(m_frame) {
			m_time += g_platformTime.lastFrameDuration();
			while(m_time > m_delay) {
				m_time -= m_delay;
				m_frame++;
			}
		}
		
		if(m_frame > 7) {
			m_frame = 0;
		}
		
	}
	
	TextureContainer * getCurrentTexture() const {
		TextureContainer * tc = scursor[m_frame];
		arx_assert(tc);
		return tc;
	}
	
};

CursorAnimatedHand cursorAnimatedHand = CursorAnimatedHand();

void ARX_INTERFACE_RenderCursor(bool flag) {
	
	UseRenderState state(render2D());
	UseTextureState textureState(getInterfaceTextureFilter(), TextureStage::WrapClamp);
	
	if(SelectSpellTargetCursorRender()) {
		return;
	}
	
	if(!(flag || (!BLOCK_PLAYER_CONTROLS && PLAYER_INTERFACE_SHOW))) {
		return;
	}
		
	if(COMBINE || COMBINEGOLD) {
		if(SpecialCursor == CURSOR_INTERACTION_ON)
			SpecialCursor = CURSOR_COMBINEON;
		else
			SpecialCursor = CURSOR_COMBINEOFF;
	}
	
	if(FlyingOverIO && config.input.autoReadyWeapon == AutoReadyWeaponNearEnemies && isEnemy(FlyingOverIO)) {
		SpecialCursor = CURSOR_READY_WEAPON;
	}
	
	if(FlyingOverIO || g_draggedEntity) {
		fHighLightAng += toMs(g_platformTime.lastFrameDuration()) * 0.5f;
		
		if(fHighLightAng > 90.f)
			fHighLightAng = 90.f;
		
		float fHLight = 100.f * glm::sin(glm::radians(fHighLightAng));
		
		iHighLight = checked_range_cast<int>(fHLight);
	} else {
		fHighLightAng = 0.f;
		iHighLight = 0;
	}
	
	float iconScale = g_hudRoot.getScale();
	float cursorScale = getInterfaceScale(config.interface.cursorScale, config.interface.cursorScaleInteger);
	
	if(SpecialCursor || !PLAYER_MOUSELOOK_ON || g_draggedEntity
	   || (FlyingOverIO && PLAYER_MOUSELOOK_ON && !g_cursorOverBook && eMouseState != MOUSE_IN_NOTE
	       && (FlyingOverIO->ioflags & IO_ITEM) && (FlyingOverIO->gameFlags & GFLAG_INTERACTIVITY)
	       && config.input.autoReadyWeapon != AlwaysAutoReadyWeapon)
	   || (MAGICMODE && PLAYER_MOUSELOOK_ON)) {
		
		Vec2f mousePos = Vec2f(DANAEMouse);
		
		if(SpecialCursor && !g_draggedEntity) {
			if((COMBINE && COMBINE->m_icon) || COMBINEGOLD) {
				if(TRUE_PLAYER_MOUSELOOK_ON && config.input.autoReadyWeapon == AlwaysAutoReadyWeapon) {
					mousePos = MemoMouse;
				}
				
				TextureContainer * tc;
				
				if(COMBINEGOLD)
					tc = GoldCoinsTC[5];
				else
					tc = COMBINE->m_icon;
				
				Vec2f size(tc->m_size.x, tc->m_size.y);
				size *= iconScale;
				
				TextureContainer * haloTc = NULL;
				
				if(COMBINE && NeedHalo(COMBINE))
					haloTc = tc->getHalo();
				
				if(haloTc) {
					Color3f haloColor = COMBINE->halo.color;
					if(SpecialCursor != CURSOR_COMBINEON) {
						haloColor *= Color3f::rgb(1.f, 2.f / 3, 0.4f);
					}
					ARX_INTERFACE_HALO_Render(haloColor, COMBINE->halo.flags, haloTc, mousePos, Vec2f(iconScale));
				}
				
				if(SpecialCursor == CURSOR_COMBINEON) {
					EERIEDrawBitmap(Rectf(mousePos, size.x, size.y), .00001f, tc, Color::white);
					
					if(FlyingOverIO && (FlyingOverIO->ioflags & IO_BLACKSMITH)) {
						float v = ARX_DAMAGES_ComputeRepairPrice(COMBINE, FlyingOverIO);
						if(v > 0.f) {
							long t = long(v);
							Vec2f nuberOffset = Vec2f(-76, -10) * iconScale;
							ARX_INTERFACE_DrawNumber(mousePos + nuberOffset, t, Color::cyan, iconScale);
						}
					}
				} else {
					EERIEDrawBitmap(Rectf(mousePos, size.x, size.y), 0.00001f, tc, Color(255, 170, 102));
				}
			}
			
			TextureContainer * surf;
			
			switch(SpecialCursor) {
			case CURSOR_REDIST:
				surf = cursorRedist;
				break;
			case CURSOR_COMBINEOFF:
				surf = cursorTargetOff;
				mousePos.x -= 16.f;
				mousePos.y -= 16.f;
				break;
			case CURSOR_COMBINEON:
				surf = cursorTargetOn;
				arx_assert(surf);
				
				mousePos.x -= 16.f;
				mousePos.y -= 16.f;
				break;
			case CURSOR_INTERACTION_ON: {
				cursorAnimatedHand.update1();
				surf = cursorAnimatedHand.getCurrentTexture();
				break;
			}
			case CURSOR_READY_WEAPON: {
				surf = cursorReadyWeapon;
				arx_assert(surf);
				mousePos -= surf->size() / s32(2);
				break;
			}
			default:
				cursorAnimatedHand.update2();
				surf = cursorAnimatedHand.getCurrentTexture();
				break;
			}
			
			arx_assert(surf);
			
			if(SpecialCursor == CURSOR_REDIST) {
				EERIEDrawBitmap(Rectf(mousePos, float(surf->m_size.x) * g_sizeRatio.x,
				                float(surf->m_size.y) * g_sizeRatio.y), 0.f, surf, Color::white);
				
				Vec2f textPos = Vec2f(DANAEMouse);
				textPos += Vec2f(16.5f, 11.5f) * g_sizeRatio;
				
				std::stringstream ss;
				ss << std::setw(3) << lCursorRedistValue;
				
				UNICODE_ARXDrawTextCenter(hFontInBook, textPos, ss.str(), Color::black);
			} else {
				Vec2f size = Vec2f(surf->m_size) * cursorScale;
				EERIEDrawBitmap(Rectf(mousePos, size.x, size.y), 0.f, surf, Color::white);
			}
			
			SpecialCursor = CURSOR_UNDEFINED;
		} else {
			if(   !(player.m_currentMovement & PLAYER_CROUCH)
			   && !BLOCK_PLAYER_CONTROLS
			   && GInput->actionPressed(CONTROLS_CUST_MAGICMODE)
			   && ARXmenu.mode() == Mode_InGame
			) {
				if(!MAGICMODE) {
					if(player.Interface & INTER_PLAYERBOOK) {
						g_playerBook.close(); // Forced Closing
					}
					MAGICMODE = true;
				}
				
				TextureContainer * surf = cursorMagic;
				
				Vec2f pos = Vec2f(DANAEMouse);
				
				if(TRUE_PLAYER_MOUSELOOK_ON) {
					pos = MemoMouse;
				}
				
				Vec2f size(surf->m_size.x, surf->m_size.y);
				size *= cursorScale;
				
				pos += -size * 0.5f;
				
				EERIEDrawBitmap(Rectf(pos, size.x, size.y), 0.f, surf, Color::white);
			} else {
				if(MAGICMODE) {
					ARX_SOUND_Stop(player.magic_draw);
					player.magic_draw = audio::SourcedSample();
					MAGICMODE = false;
				}
				
				if(g_draggedEntity && g_draggedEntity->m_icon) {
					
					TextureContainer * tc = g_draggedEntity->m_icon;
					TextureContainer * haloTc = NULL;
					if(NeedHalo(g_draggedEntity)) {
						haloTc = g_draggedEntity->m_icon->getHalo();
					}
					
					Color color = (g_draggedEntity->poisonous && g_draggedEntity->poisonous_count != 0) ? Color::green : Color::white;
					
					Vec2f pos = mousePos;
					
					if(TRUE_PLAYER_MOUSELOOK_ON && config.input.autoReadyWeapon == AlwaysAutoReadyWeapon) {
						pos = MemoMouse;
					}
					
					{
						Vec2f size = Vec2f(tc->m_size) * iconScale;
						Rectf rect(pos, size.x, size.y);
						
						if(haloTc) {
							ARX_INTERFACE_HALO_Render(g_draggedEntity->halo.color, g_draggedEntity->halo.flags, haloTc,
							                          pos, Vec2f(iconScale));
						}
						
						if(!(g_draggedEntity->ioflags & IO_MOVABLE)) {
							EERIEDrawBitmap(rect, .00001f, tc, color);
							
							if((g_draggedEntity->ioflags & IO_ITEM) && g_draggedEntity->_itemdata->count != 1) {
								ARX_INTERFACE_DrawNumber(rect.topRight(), g_draggedEntity->_itemdata->count, Color::white,
								                         iconScale);
							}
						} else {
							if(g_dragStatus != EntityDragStatus_Throw) {
								EERIEDrawBitmap(rect, .00001f, tc, color);
							}
						}
						
					}
					
					// Cross not over inventory icon
					if(g_dragStatus == EntityDragStatus_Throw || g_dragStatus == EntityDragStatus_Invalid) {
						TextureContainer * tcc =  cursorMovable;
						
						if(g_dragStatus == EntityDragStatus_Throw)
							tcc = cursorThrowObject;
						
						if(tcc && tcc != tc) { // to avoid movable double red cross...
							Vec2f size = Vec2f(tcc->m_size) * cursorScale;
							EERIEDrawBitmap(Rectf(Vec2f(pos.x + 16, pos.y), size.x, size.y), 0.00001f, tcc, Color::white);
						}
					}
					
				} else {
					cursorAnimatedHand.update2();
					TextureContainer * surf = cursorAnimatedHand.getCurrentTexture();
					
					if(surf) {
						Vec2f size = Vec2f(surf->m_size) * cursorScale;
						EERIEDrawBitmap(Rectf(mousePos, size.x, size.y), 0.f, surf, Color::white);
					}
				}
			}
		}
	} else {
		
		// System shock mode
		
		float alpha = 0.5f;
		if(player.Interface & INTER_COMBATMODE) {
			alpha *= std::max(player.m_bowAimRatio - 0.75f, 0.f) / 0.25f;
		}
		
		if(TRUE_PLAYER_MOUSELOOK_ON && config.interface.showCrosshair
		   && !(player.Interface & (INTER_PLAYERBOOK)) && !g_note.isOpen() && alpha > 0.f) {
			
			cursorAnimatedHand.reset();
			
			TextureContainer * surf = cursorCrossHair;
			arx_assert(surf);
			
			UseRenderState additeState(render2D().blendAdditive());
			
			Vec2f pos = Vec2f(g_size.center());
			Vec2f size = Vec2f(surf->m_size) * cursorScale;
			EERIEDrawBitmap(Rectf(pos - size * 0.5f, size.x, size.y), 0.f, surf, Color::gray(alpha));
			
		}
		
	}
	
}
