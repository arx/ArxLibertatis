/*
 * Copyright 2013 Arx Libertatis Team (see the AUTHORS file)
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
#include "physics/Box.h"
#include "platform/profiler/Profiler.h"
#include "scene/Interactive.h"
#include "scene/GameSound.h"

#include "gui/Interface.h"
#include "gui/Text.h"
#include "gui/Menu.h"
#include "gui/hud/SecondaryInventory.h"


extern float STARTED_ANGLE;
long SPECIAL_DRAGINTER_RENDER=0;

EntityMoveCursor CANNOT_PUT_IT_HERE = EntityMoveCursor_Ok;

static TextureContainer * cursorTargetOn = NULL;
static TextureContainer * cursorTargetOff = NULL;
static TextureContainer * cursorInteractionOn = NULL;
static TextureContainer * cursorInteractionOff = NULL;
static TextureContainer * cursorMagic = NULL;
static TextureContainer * cursorThrowObject = NULL;
static TextureContainer * cursorRedist = NULL;
static TextureContainer * cursorCrossHair = NULL; // Animated Hand Cursor TC
TextureContainer * cursorMovable = NULL;   // TextureContainer for Movable Items (Red Cross)

TextureContainer *	scursor[8];			// Animated Hand Cursor TC

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


bool Manage3DCursor(Entity * io, bool simulate) {
	
	arx_assert(io);
	
	if(BLOCK_PLAYER_CONTROLS)
		return false;

	float ag = player.angle.getYaw();

	if(ag > 180)
		ag = ag - 360;

	float drop_miny = float(g_size.center().y) - float(g_size.center().y) * ag * (1.f/70);

	if(DANAEMouse.y < drop_miny)
		return false;
	
	Anglef temp = Anglef::ZERO;

	if(io->ioflags & IO_INVERTED) {
		temp.setYaw(180.f);
		temp.setPitch(-MAKEANGLE(270.f - io->angle.getPitch() - (player.angle.getPitch() - STARTED_ANGLE)));
	} else {
		temp.setPitch(MAKEANGLE(270.f - io->angle.getPitch() - (player.angle.getPitch() - STARTED_ANGLE)));
	}
	
	EERIE_3D_BBOX bbox;
	for(size_t i = 0; i < io->obj->vertexlist.size(); i++) {
		bbox.add(io->obj->vertexlist[i].v);
	}
	
	Vec3f mvectx = angleToVectorXZ(player.angle.getPitch() - 90.f);
	
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
	
	Vec3f collidpos = Vec3f_ZERO;
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
		
		for(int i = 1; i < io->obj->pbox->nb_physvert; i++) {
			const PHYSVERT & physVert = io->obj->pbox->vert[i];
			
			float dist = glm::distance(tmpVert, Vec2f(physVert.initpos.x, physVert.initpos.z)) + 14.f;
			maxdist = std::max(maxdist, dist);
		}
	}
	
	Cylinder cyl2 = Cylinder(Vec3f_ZERO, glm::clamp(maxdist, 20.f, 150.f), std::min(-30.f, height));
	
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
	
	objcenter = VRotateY(objcenter, temp.getPitch());
	
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
			
			glm::quat rotation = glm::toQuat(toRotationMatrix(temp));
			
			if(SPECIAL_DRAGINTER_RENDER) {
			if(glm::abs(lastanything) > glm::abs(height)) {
				TransformInfo t(collidpos, rotation, io->scale);

				static const float invisibility = 0.5f;

				DrawEERIEInter(io->obj, t, io, false, invisibility);
			} else {
				TransformInfo t(pos, rotation, io->scale);

				float invisibility = Cedric_GetInvisibility(io);

				DrawEERIEInter(io->obj, t, io, false, invisibility);
			}
			}
		} else {
			if(glm::abs(lastanything) > std::min(glm::abs(height), 12.0f)) {
				Entity * io = DRAGINTER;
				ARX_PLAYER_Remove_Invisibility();
				io->obj->pbox->active = 1;
				io->obj->pbox->stopcount = 0;
				io->pos = collidpos;
				io->velocity = Vec3f_ZERO;

				io->stopped = 1;

				movev.x *= 0.0001f;
				movev.y = 0.1f;
				movev.z *= 0.0001f;
				Vec3f viewvector = movev;

				Anglef angle = temp;
				io->soundtime = 0;
				io->soundcount = 0;
				EERIE_PHYSICS_BOX_Launch(io->obj, io->pos, angle, viewvector);
				ARX_SOUND_PlaySFX(SND_WHOOSH, &pos);
				io->show = SHOW_FLAG_IN_SCENE;
				Set_DragInter(NULL);
			} else {
				ARX_PLAYER_Remove_Invisibility();
				ARX_SOUND_PlayInterface(SND_INVSTD);
				ARX_INTERACTIVE_Teleport(io, pos, true);

				io->angle.setYaw(temp.getYaw());
				io->angle.setPitch(270.f - temp.getPitch());
				io->angle.setRoll(temp.getRoll());

				io->stopped = 0;
				io->show = SHOW_FLAG_IN_SCENE;
				io->obj->pbox->active = 0;
				Set_DragInter(NULL);
			}
		}

		GRenderer->SetCulling(CullNone);
		return true;
	} else {
		CANNOT_PUT_IT_HERE = EntityMoveCursor_Throw;
	}

	return false;
}

extern long LOOKING_FOR_SPELL_TARGET;
extern ArxInstant LOOKING_FOR_SPELL_TARGET_TIME;
extern bool PLAYER_INTERFACE_HIDE_COUNT;
extern long lCursorRedistValue;

int iHighLight=0;
float fHighLightAng=0.f;

static bool SelectSpellTargetCursorRender() {
	
	if(   !SPECIAL_DRAGINTER_RENDER
	   && LOOKING_FOR_SPELL_TARGET
	) {
		float elapsed = arxtime.now_f() - LOOKING_FOR_SPELL_TARGET_TIME;
		if(elapsed > 7000) {
			ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE, &player.pos);
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
				ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE, &player.pos);
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
	long m_time;
	long m_frame;
	long m_delay;
	
public:
	CursorAnimatedHand()
		: m_time(0)
		, m_frame(0)
		, m_delay(70)
	{}
	
	void reset() {
		m_frame = 0;
	}
	
	void update1() {
		m_time += checked_range_cast<long>(Original_framedelay);
		
		if(m_frame!=3) {
			while(m_time > m_delay) {
				m_time -= m_delay;
				m_frame++;
			}
		}

		if(m_frame > 7)
			m_frame = 0;
	}
	
	void update2() {
		if(m_frame) {
			m_time += checked_range_cast<long>(Original_framedelay);

			while(m_time > m_delay) {
				m_time -= m_delay;
				m_frame++;
			}
		}

		if(m_frame > 7)
			m_frame = 0;
	}
	
	TextureContainer * getCurrentTexture() {
		TextureContainer * tc = scursor[m_frame];
		arx_assert(tc);
		return tc;
	}
};
CursorAnimatedHand cursorAnimatedHand = CursorAnimatedHand();



static void ARX_INTERFACE_RenderCursorInternal(bool flag) {
	
	if(SelectSpellTargetCursorRender()) {
		return;
	}
	
	if(!(flag || (!BLOCK_PLAYER_CONTROLS && PLAYER_INTERFACE_HIDE_COUNT))) {
		return;
	}
		
	if(!SPECIAL_DRAGINTER_RENDER)
		GRenderer->SetCulling(CullNone);
	
	if(COMBINE || COMBINEGOLD) {
		if(SpecialCursor == CURSOR_INTERACTION_ON)
			SpecialCursor = CURSOR_COMBINEON;
		else
			SpecialCursor = CURSOR_COMBINEOFF;
	}
	
	if(!SPECIAL_DRAGINTER_RENDER) {
		if(FlyingOverIO || DRAGINTER) {
			fHighLightAng += g_framedelay * 0.5f;
			
			if(fHighLightAng > 90.f)
				fHighLightAng = 90.f;
			
			float fHLight = 100.f * glm::sin(glm::radians(fHighLightAng));
			
			iHighLight = checked_range_cast<int>(fHLight);
		} else {
			fHighLightAng = 0.f;
			iHighLight = 0;
		}
	}
	
	if(   SpecialCursor
	   || !PLAYER_MOUSELOOK_ON
	   || DRAGINTER
	   ||  (FlyingOverIO
		 && PLAYER_MOUSELOOK_ON
		 && !g_cursorOverBook
		 && (eMouseState != MOUSE_IN_NOTE)
		 && (FlyingOverIO->ioflags & IO_ITEM)
		 && (FlyingOverIO->gameFlags & GFLAG_INTERACTIVITY)
		 && (config.input.autoReadyWeapon == false))
	   || (MAGICMODE && PLAYER_MOUSELOOK_ON)
	) {
		CANNOT_PUT_IT_HERE = EntityMoveCursor_Ok;
		float ag=player.angle.getYaw();
		
		if(ag > 180)
			ag = ag - 360;
		
		float drop_miny = float(g_size.center().y) - float(g_size.center().y) * ag * (1.0f/70);
		
		if(   DANAEMouse.y > drop_miny
		   && DRAGINTER
		   && !InInventoryPos(DANAEMouse)
		   && !g_cursorOverBook
		) {
			if(!Manage3DCursor(DRAGINTER, true))
				CANNOT_PUT_IT_HERE = EntityMoveCursor_Throw;
			
			if(SPECIAL_DRAGINTER_RENDER) {
				CANNOT_PUT_IT_HERE = EntityMoveCursor_Ok;
				return;
			}
		} else {
			CANNOT_PUT_IT_HERE = EntityMoveCursor_Throw;
		}
		
		if(SPECIAL_DRAGINTER_RENDER)
			return;
		
		Vec2f mousePos = Vec2f(DANAEMouse);
		
		if(SpecialCursor && !DRAGINTER) {
			if((COMBINE && COMBINE->m_icon) || COMBINEGOLD) {
				if(TRUE_PLAYER_MOUSELOOK_ON && (config.input.autoReadyWeapon)) {
					mousePos = MemoMouse;
				}
				
				TextureContainer * tc;
				
				if(COMBINEGOLD)
					tc = GoldCoinsTC[5];
				else
					tc = COMBINE->m_icon;
				
				Vec2f size(tc->m_size.x, tc->m_size.y);
				
				if(SpecialCursor == CURSOR_COMBINEON) {
					EERIEDrawBitmap(Rectf(mousePos, size.x, size.y), .00001f, tc, Color::white);
					
					if(FlyingOverIO && (FlyingOverIO->ioflags & IO_BLACKSMITH)) {
						float v=ARX_DAMAGES_ComputeRepairPrice(COMBINE,FlyingOverIO);
						
						if(v > 0.f) {
							long t = long(v);
							Vec2f nuberOffset = Vec2f(-16, -10);
							ARX_INTERFACE_DrawNumber(mousePos + nuberOffset, t, 6, Color::cyan, 1.f);
						}
					}
				} else {
					EERIEDrawBitmap(Rectf(mousePos, size.x, size.y), 0.00001f, tc, Color(255, 170, 102, 255));
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
			case CURSOR_INTERACTION_ON:
				cursorAnimatedHand.update1();
				surf = cursorAnimatedHand.getCurrentTexture();
				break;
			default:
				cursorAnimatedHand.update2();
				surf = cursorAnimatedHand.getCurrentTexture();
				break;
			}
			
			arx_assert(surf);
			
			if(SpecialCursor == CURSOR_REDIST) {
				EERIEDrawBitmap(Rectf(mousePos, float(surf->m_size.x) * g_sizeRatio.x, float(surf->m_size.y) * g_sizeRatio.y),
								0.f, surf, Color::white);
				
				Vec2f textPos = Vec2f(DANAEMouse);
				textPos += Vec2f(17.5f, 12.5f) * g_sizeRatio;
				
				std::stringstream ss;
				ss << std::setw(3) << lCursorRedistValue;
				
				UNICODE_ARXDrawTextCenter(hFontInBook, textPos, ss.str(), Color::black);
			} else {
				
				EERIEDrawBitmap(Rectf(mousePos, float(surf->m_size.x), float(surf->m_size.y)), 0.f, surf, Color::white);
			}
			
			SpecialCursor = 0;
		} else {
			if(   !(player.m_currentMovement & PLAYER_CROUCH)
			   && !BLOCK_PLAYER_CONTROLS
			   && GInput->actionPressed(CONTROLS_CUST_MAGICMODE)
			   && ARXmenu.currentmode == AMCM_OFF
			) {
				if(!MAGICMODE) {
					if(player.Interface & INTER_MAP) {
						ARX_INTERFACE_BookClose(); // Forced Closing
					}
					MAGICMODE = true;
				}
				
				TextureContainer * surf = cursorMagic;
				
				Vec2f pos = Vec2f(DANAEMouse);
				
				if(TRUE_PLAYER_MOUSELOOK_ON) {
					pos = MemoMouse;
				}
				
				Vec2f size(surf->m_size.x, surf->m_size.y);
				
				pos += -size * 0.5f;
				
				EERIEDrawBitmap(Rectf(pos, size.x, size.y), 0.f, surf, Color::white);
			} else {
				if(MAGICMODE) {
					ARX_SOUND_Stop(SND_MAGIC_DRAW);
					MAGICMODE = false;
				}
				
				if(DRAGINTER && DRAGINTER->m_icon) {
					TextureContainer * tc = DRAGINTER->m_icon;
					TextureContainer * haloTc = NULL;
					
					if(NeedHalo(DRAGINTER))
						haloTc = DRAGINTER->m_icon->getHalo();//>_itemdata->halo_tc;
					
					Color color = (DRAGINTER->poisonous && DRAGINTER->poisonous_count != 0) ? Color::green : Color::white;
					
					Vec2f pos = mousePos;
					
					if(TRUE_PLAYER_MOUSELOOK_ON && config.input.autoReadyWeapon) {
						pos = MemoMouse;
					}
					
					Rectf rect(pos, float(tc->m_size.x), float(tc->m_size.y));
					
					if(!(DRAGINTER->ioflags & IO_MOVABLE)) {
						EERIEDrawBitmap(rect, .00001f, tc, color);
						
						if((DRAGINTER->ioflags & IO_ITEM) && DRAGINTER->_itemdata->count != 1) {
							Vec2f nuberOffset = Vec2f(2.f, 13.f);
							ARX_INTERFACE_DrawNumber(pos + nuberOffset, DRAGINTER->_itemdata->count, 3, Color::white, 1.f);
						}
					} else {
						if((InInventoryPos(DANAEMouse) || g_secondaryInventoryHud.containsPos(DANAEMouse)) || CANNOT_PUT_IT_HERE != EntityMoveCursor_Throw) {
							EERIEDrawBitmap(rect, .00001f, tc, color);
						}
					}
					
					//cross not over inventory icon
					if(   CANNOT_PUT_IT_HERE != EntityMoveCursor_Ok
					   && (eMouseState != MOUSE_IN_INVENTORY_ICON)
					   && !InInventoryPos(DANAEMouse)
					   && !g_secondaryInventoryHud.containsPos(DANAEMouse)
					   && !ARX_INTERFACE_MouseInBook()) {
						TextureContainer * tcc = cursorMovable;
						
						if(CANNOT_PUT_IT_HERE == EntityMoveCursor_Throw)
							tcc = cursorThrowObject;
						
						if(tcc && tcc != tc) // to avoid movable double red cross...
							EERIEDrawBitmap(Rectf(Vec2f(pos.x + 16, pos.y), float(tcc->m_size.x), float(tcc->m_size.y)), 0.00001f, tcc, Color::white);
					}
					
					if(haloTc) {
						ARX_INTERFACE_HALO_Draw(DRAGINTER, tc, haloTc, pos, Vec2f(1));
					}
				} else {
					cursorAnimatedHand.update2();
					TextureContainer * surf = cursorAnimatedHand.getCurrentTexture();
					
					if(surf) {
						EERIEDrawBitmap(Rectf(mousePos, float(surf->m_size.x), float(surf->m_size.y)), 0.f, surf, Color::white);
					}
				}
			}
		}
	} else { //mode system shock
		if(SPECIAL_DRAGINTER_RENDER)
			return;
		
		if(   TRUE_PLAYER_MOUSELOOK_ON
		   && config.interface.showCrosshair
		   && !(player.Interface & (INTER_COMBATMODE | INTER_NOTE | INTER_MAP))) {
			
			cursorAnimatedHand.reset();
			
			TextureContainer * surf = cursorCrossHair;
			arx_assert(surf);
			
			GRenderer->SetRenderState(Renderer::AlphaBlending, true);
			GRenderer->SetBlendFunc(BlendOne, BlendOne);
			
			Vec2f pos = Vec2f(g_size.center()) - Vec2f(surf->m_size) * .5f;
			
			EERIEDrawBitmap(Rectf(pos, float(surf->m_size.x), float(surf->m_size.y)), 0.f, surf, Color3f::gray(.5f).to<u8>());
			
			GRenderer->SetRenderState(Renderer::AlphaBlending, false);
		}
	}
}

void ARX_INTERFACE_RenderCursor(bool flag) {
	
	ARX_PROFILE_FUNC();
	
	if (!SPECIAL_DRAGINTER_RENDER)
	{
		GRenderer->GetTextureStage(0)->setMinFilter(TextureStage::FilterNearest);
		GRenderer->GetTextureStage(0)->setMagFilter(TextureStage::FilterNearest);
		GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapClamp);
	}

	ARX_INTERFACE_RenderCursorInternal(flag);

	// Ensure filtering settings are restored in all cases
	if (!SPECIAL_DRAGINTER_RENDER)
	{
		GRenderer->GetTextureStage(0)->setMinFilter(TextureStage::FilterLinear);
		GRenderer->GetTextureStage(0)->setMagFilter(TextureStage::FilterLinear);
		GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapRepeat);
	}
}

