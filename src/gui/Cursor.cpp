/*
 * Copyright 2013-2018 Arx Libertatis Team (see the AUTHORS file)
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

#include "animation/AnimationRender.h"

#include "physics/Collisions.h"
#include "physics/Physics.h"

#include "platform/profiler/Profiler.h"
#include "scene/Interactive.h"
#include "scene/GameSound.h"

#include "gui/Hud.h"
#include "gui/Interface.h"
#include "gui/Text.h"
#include "gui/Menu.h"
#include "gui/hud/SecondaryInventory.h"

enum ARX_INTERFACE_CURSOR_MODE
{
	CURSOR_UNDEFINED,
	CURSOR_FIREBALLAIM,
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


extern float STARTED_ANGLE;

EntityMoveCursor CANNOT_PUT_IT_HERE = EntityMoveCursor_Ok;

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


bool Manage3DCursor(Entity * io, bool simulate, bool draginter) {
	
	arx_assert(io);
	
	if(simulate && draginter) {
		io->show = SHOW_FLAG_ON_PLAYER;
	}
	
	if(BLOCK_PLAYER_CONTROLS) {
		return false;
	}
	
	float ag = player.angle.getPitch();
	if(ag > 180.f) {
		ag = ag - 360.f;
	}
	
	float drop_miny = float(g_size.center().y) - float(g_size.center().y) * ag * (1.f / 70);
	if(DANAEMouse.y < drop_miny) {
		return false;
	}
	
	Anglef angle = io->angle;
	
	float deltaYaw = player.angle.getYaw() - STARTED_ANGLE;
	angle.setPitch(MAKEANGLE(angle.getPitch() + std::sin(glm::radians(angle.getRoll())) * deltaYaw));
	angle.setYaw(MAKEANGLE(angle.getYaw() + std::cos(glm::radians(angle.getRoll())) * deltaYaw));
	
	io->angle = angle;
	STARTED_ANGLE = player.angle.getYaw();
	
	EERIE_3D_BBOX bbox;
	for(size_t i = 0; i < io->obj->vertexlist.size(); i++) {
		bbox.add(io->obj->vertexlist[i].v);
	}
	
	Vec3f mvectx = angleToVectorXZ(player.angle.getYaw() - 90.f);
	
	Vec2f mod = Vec2f(Vec2i(DANAEMouse) - g_size.center()) / Vec2f(g_size.center()) * Vec2f(160.f, 220.f);
	mvectx *= mod.x;
	Vec3f mvecty(0, mod.y, 0);

	Vec3f orgn = player.pos;
	orgn += angleToVector(player.angle) * 50.f;
	orgn += mvectx;
	orgn.y += mvecty.y;

	Vec3f dest = player.pos;
	dest += angleToVector(player.angle) * 10000.f;
	dest += mvectx;
	dest.y += mvecty.y * 5.f;
	
	Vec3f pos = orgn;

	Vec3f movev = glm::normalize(dest - orgn);

	float lastanything = 0.f;
	float height = -(bbox.max.y - bbox.min.y);

	if(height > -30.f)
		height = -30.f;
	
	Vec3f objcenter = bbox.min + (bbox.max - bbox.min) * Vec3f(0.5f);
	
	Vec3f collidpos(0.f);
	bool collidpos_ok = false;
	
	{
	float maxdist = 0.f;
	
	for(size_t i = 0; i < io->obj->vertexlist.size(); i++) {
		const EERIE_VERTEX & vert = io->obj->vertexlist[i];
		
		float dist = glm::distance(Vec2f(objcenter.x, objcenter.z), Vec2f(vert.v.x, vert.v.z)) - 4.f;
		maxdist = std::max(maxdist, dist);
	}

	if(io->obj->pbox) {
		Vec2f tmpVert(io->obj->pbox->vert[0].initpos.x, io->obj->pbox->vert[0].initpos.z);
		
		for(size_t i = 1; i < io->obj->pbox->vert.size(); i++) {
			const PhysicsParticle & physVert = io->obj->pbox->vert[i];
			
			float dist = glm::distance(tmpVert, Vec2f(physVert.initpos.x, physVert.initpos.z)) + 14.f;
			maxdist = std::max(maxdist, dist);
		}
	}
	
	Cylinder cyl2 = Cylinder(Vec3f(0.f), glm::clamp(maxdist, 20.f, 150.f), std::min(-30.f, height));
	
	const float inc = 10.f;
	long iterating = 40;
	
	while(iterating > 0) {
		cyl2.origin = pos + movev * inc + Vec3f(0.f, bbox.max.y, 0.f);

		float anything = CheckAnythingInCylinder(cyl2, io, CFLAG_JUST_TEST | CFLAG_COLLIDE_NOCOL | CFLAG_NO_NPC_COLLIDE);

		if(anything < 0.f) {
			if(iterating == 40) {
				CANNOT_PUT_IT_HERE = EntityMoveCursor_Invalid;
				// TODO is this correct ?
				return true;
			}

			iterating = 0;

			collidpos = cyl2.origin;

			if(lastanything < 0.f) {
				pos.y += lastanything;
				collidpos.y += lastanything;
			}
		} else {
			pos = cyl2.origin;
			lastanything = anything;
		}

		iterating--;
	}
	collidpos_ok = iterating == -1;
	
	}
	
	angle.setYaw(MAKEANGLE(270.f - angle.getYaw()));
	objcenter = VRotateY(objcenter, angle.getYaw());
	objcenter = VRotateX(objcenter, -angle.getPitch());
	objcenter = VRotateZ(objcenter, angle.getRoll());
	
	collidpos.x -= objcenter.x;
	collidpos.z -= objcenter.z;

	pos.x -= objcenter.x;
	pos.z -= objcenter.z;

	if(!collidpos_ok) {
		CANNOT_PUT_IT_HERE = EntityMoveCursor_Invalid;
		return false;
	}

	if(collidpos_ok && closerThan(player.pos, pos, 300.f)) {
		if(simulate) {
			ARX_INTERACTIVE_Teleport(io, pos, true);
			io->gameFlags &= ~GFLAG_NOCOMPUTATION;
			glm::quat rotation = glm::quat_cast(toRotationMatrix(angle));
			if(draginter) {
				if(glm::abs(lastanything) > glm::abs(height)) {
					io->show = SHOW_FLAG_TELEPORTING;
					TransformInfo t(collidpos, rotation, io->scale);
					static const float invisibility = 0.5f;
					DrawEERIEInter(io->obj, t, io, false, invisibility);
				} else {
					io->show = SHOW_FLAG_IN_SCENE;
					// Entity will be rendered with the rest of the in-scene entities
				}
			}
		} else {
			if(glm::abs(lastanything) > std::min(glm::abs(height), 12.0f)) {
				ARX_PLAYER_Remove_Invisibility();
				io->obj->pbox->active = 1;
				io->obj->pbox->stopcount = 0;
				io->pos = collidpos;

				movev.x *= 0.0001f;
				movev.y = 0.1f;
				movev.z *= 0.0001f;
				Vec3f viewvector = movev;

				io->soundtime = 0;
				io->soundcount = 0;
				EERIE_PHYSICS_BOX_Launch(io->obj, io->pos, io->angle, viewvector);
				ARX_SOUND_PlaySFX(g_snd.WHOOSH, &pos);
				io->show = SHOW_FLAG_IN_SCENE;
				Set_DragInter(NULL);
			} else {
				ARX_PLAYER_Remove_Invisibility();
				ARX_SOUND_PlayInterface(g_snd.INVSTD);
				ARX_INTERACTIVE_Teleport(io, pos, true);
				
				io->show = SHOW_FLAG_IN_SCENE;
				io->obj->pbox->active = 0;
				Set_DragInter(NULL);
			}
		}

		return true;
	} else {
		CANNOT_PUT_IT_HERE = EntityMoveCursor_Throw;
	}

	return false;
}

extern long LOOKING_FOR_SPELL_TARGET;
extern GameInstant LOOKING_FOR_SPELL_TARGET_TIME;
extern bool PLAYER_INTERFACE_SHOW;

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
	
	TextureContainer * getCurrentTexture() {
		TextureContainer * tc = scursor[m_frame];
		arx_assert(tc);
		return tc;
	}
};

CursorAnimatedHand cursorAnimatedHand = CursorAnimatedHand();

void ARX_INTERFACE_RenderCursor(bool flag, bool draginter) {
	
	ARX_PROFILE_FUNC();
	
	UseRenderState state(render2D());
	UseTextureState textureState(getInterfaceTextureFilter(), TextureStage::WrapClamp);
	
	if(!draginter && SelectSpellTargetCursorRender()) {
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
	
	if(!draginter) {
		if(FlyingOverIO || DRAGINTER) {
			fHighLightAng += toMs(g_platformTime.lastFrameDuration()) * 0.5f;
			
			if(fHighLightAng > 90.f)
				fHighLightAng = 90.f;
			
			float fHLight = 100.f * glm::sin(glm::radians(fHighLightAng));
			
			iHighLight = checked_range_cast<int>(fHLight);
		} else {
			fHighLightAng = 0.f;
			iHighLight = 0;
		}
	}
	
	float iconScale = g_hudRoot.getScale();
	float cursorScale = getInterfaceScale(config.interface.cursorScale, config.interface.cursorScaleInteger);
	
	if(SpecialCursor || !PLAYER_MOUSELOOK_ON || DRAGINTER
	   || (FlyingOverIO && PLAYER_MOUSELOOK_ON && !g_cursorOverBook && eMouseState != MOUSE_IN_NOTE
	       && (FlyingOverIO->ioflags & IO_ITEM) && (FlyingOverIO->gameFlags & GFLAG_INTERACTIVITY)
	       && config.input.autoReadyWeapon != AlwaysAutoReadyWeapon)
	   || (MAGICMODE && PLAYER_MOUSELOOK_ON)) {
		
		CANNOT_PUT_IT_HERE = EntityMoveCursor_Ok;
		
		float ag = player.angle.getPitch();
		if(ag > 180)
			ag = ag - 360;
		
		float drop_miny = float(g_size.center().y) - float(g_size.center().y) * ag * (1.f / 70);
		if(DANAEMouse.y > drop_miny && DRAGINTER && !InInventoryPos(DANAEMouse) && !g_cursorOverBook) {
			
			if(!Manage3DCursor(DRAGINTER, true, draginter)) {
				CANNOT_PUT_IT_HERE = EntityMoveCursor_Throw;
			}
			
			if(draginter) {
				CANNOT_PUT_IT_HERE = EntityMoveCursor_Ok;
				return;
			}
			
		} else {
			CANNOT_PUT_IT_HERE = EntityMoveCursor_Throw;
		}
		
		if(draginter) {
			return;
		}
		
		Vec2f mousePos = Vec2f(DANAEMouse);
		
		if(SpecialCursor && !DRAGINTER) {
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
			case CURSOR_FIREBALLAIM: {
				surf = cursorTargetOn;
				arx_assert(surf);
				
				mousePos = Vec2f(320.f, 280.f) - Vec2f(surf->m_size) * 0.5f;
				break;
			}
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
				
				if(DRAGINTER && DRAGINTER->m_icon) {
					TextureContainer * tc = DRAGINTER->m_icon;
					TextureContainer * haloTc = NULL;
					
					if(NeedHalo(DRAGINTER)) {
						haloTc = DRAGINTER->m_icon->getHalo();
					}
					
					Color color = (DRAGINTER->poisonous && DRAGINTER->poisonous_count != 0) ? Color::green : Color::white;
					
					Vec2f pos = mousePos;
					
					if(TRUE_PLAYER_MOUSELOOK_ON && config.input.autoReadyWeapon == AlwaysAutoReadyWeapon) {
						pos = MemoMouse;
					}
					
					{
						Vec2f size = Vec2f(tc->m_size) * iconScale;
						Rectf rect(pos, size.x, size.y);
						
						if(haloTc) {
							ARX_INTERFACE_HALO_Render(DRAGINTER->halo.color, DRAGINTER->halo.flags, haloTc, pos, Vec2f(iconScale));
						}
						
						if(!(DRAGINTER->ioflags & IO_MOVABLE)) {
							EERIEDrawBitmap(rect, .00001f, tc, color);
							
							if((DRAGINTER->ioflags & IO_ITEM) && DRAGINTER->_itemdata->count != 1) {
								ARX_INTERFACE_DrawNumber(rect.topRight(), DRAGINTER->_itemdata->count, Color::white, iconScale);
							}
						} else {
							if((InInventoryPos(DANAEMouse) || g_secondaryInventoryHud.containsPos(DANAEMouse)) || CANNOT_PUT_IT_HERE != EntityMoveCursor_Throw) {
								EERIEDrawBitmap(rect, .00001f, tc, color);
							}
						}
						
					}
					
					// Cross not over inventory icon
					if(   CANNOT_PUT_IT_HERE != EntityMoveCursor_Ok
					   && (eMouseState != MOUSE_IN_INVENTORY_ICON)
					   && !InInventoryPos(DANAEMouse)
					   && !g_secondaryInventoryHud.containsPos(DANAEMouse)
					   && !ARX_INTERFACE_MouseInBook()) {
						TextureContainer * tcc = cursorMovable;
						
						if(CANNOT_PUT_IT_HERE == EntityMoveCursor_Throw)
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
		
		if(draginter) {
			return;
		}
		
		if(TRUE_PLAYER_MOUSELOOK_ON && config.interface.showCrosshair
		   && !(player.Interface & (INTER_COMBATMODE | INTER_PLAYERBOOK)) && !g_note.isOpen()) {
			
			cursorAnimatedHand.reset();
			
			TextureContainer * surf = cursorCrossHair;
			arx_assert(surf);
			
			
			UseRenderState additeState(render2D().blendAdditive());
			
			Vec2f pos = Vec2f(g_size.center()) - Vec2f(surf->m_size) * .5f;
			
			Vec2f size = Vec2f(surf->m_size) * cursorScale;
			EERIEDrawBitmap(Rectf(pos, size.x, size.y), 0.f, surf, Color::gray(0.5f));
			
		}
		
	}
}
