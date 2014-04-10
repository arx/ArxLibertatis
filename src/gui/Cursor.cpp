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

#include "graphics/Math.h"
#include "graphics/Draw.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/texture/TextureStage.h"
#include "graphics/Renderer.h"

#include "animation/AnimationRender.h"
#include "physics/Collisions.h"
#include "physics/Box.h"
#include "scene/Interactive.h"
#include "scene/GameSound.h"

#include "gui/Interface.h"
#include "gui/Text.h"
#include "gui/Menu.h"

extern Rect g_size;
extern Vec2s DANAEMouse;

extern bool TRUE_PLAYER_MOUSELOOK_ON;

extern float STARTED_ANGLE;
long SPECIAL_DRAGINTER_RENDER=0;
long CANNOT_PUT_IT_HERE=0;

static TextureContainer * cursorTargetOn = NULL;
static TextureContainer * cursorTargetOff = NULL;

TextureContainer * ThrowObject = NULL;

void cursorTexturesInit() {
	ITC.Reset();
	
	cursorTargetOn = TextureContainer::LoadUI("graph/interface/cursors/target_on");
	cursorTargetOff = TextureContainer::LoadUI("graph/interface/cursors/target_off");
	arx_assert(cursorTargetOn);
	arx_assert(cursorTargetOff);
	
	ITC.Set("interaction_on", "graph/interface/cursors/interaction_on");
	ITC.Set("interaction_off", "graph/interface/cursors/interaction_off");
	ITC.Set("magic", "graph/interface/cursors/magic");
	
	ThrowObject = TextureContainer::LoadUI("graph/interface/cursors/throw");
}


long Manage3DCursor(bool simulate) {

	if(BLOCK_PLAYER_CONTROLS)
		return 0;

	float ag = player.angle.getYaw();

	if(ag > 180)
		ag = ag - 360;

	float drop_miny = (float)(g_size.center().y) - g_size.center().y * (ag * (1.f/70));

	if(DANAEMouse.y < drop_miny)
		return 0;

	Entity * io = DRAGINTER;
	if(!io)
		return 0;

	Anglef temp = Anglef::ZERO;

	if(io->ioflags & IO_INVERTED) {
		temp.setYaw(180.f);
		temp.setPitch(-MAKEANGLE(270.f - io->angle.getPitch() - (player.angle.getPitch() - STARTED_ANGLE)));
	} else {
		temp.setPitch(MAKEANGLE(270.f - io->angle.getPitch() - (player.angle.getPitch() - STARTED_ANGLE)));
	}

	float angle = radians(MAKEANGLE(player.angle.getPitch()));
	float angle2 = radians(MAKEANGLE(player.angle.getPitch() - 90.f));

	//between 0 (bottom) and 1 (top)
	float zrange = (g_size.height() - DANAEMouse.y) / (g_size.height() - drop_miny);

	float va = player.angle.getYaw();

	if(va > 180)
		va = 0;

	float vd=(100.f-va)*( 1.0f / 90 );
	float mod=va-50.f;

	if(mod < 0)
		mod = 0;

	mod *= (1.f / 20);
	va = vd * (1.3f + 0.3f * mod);

	vd = ((1.f-zrange)*0.6f-vd)*150.f;

	if(va<0)
		va = 0;

	if(vd < 0)
		vd = 0;

	float mx = DANAEMouse.x;

	if(TRUE_PLAYER_MOUSELOOK_ON && config.input.autoReadyWeapon) {
		mx = MemoMouse.x;
	}

	Vec3f pos;
	pos.x = player.pos.x + std::sin(angle2) * (g_size.center().x - mx)*0.7f*va - std::sin(angle)*(va*zrange*400.f+vd);
	pos.z = player.pos.z - std::cos(angle2) * (g_size.center().x - mx)*0.7f*va + std::cos(angle)*(va*zrange*400.f+vd);
	pos.y = player.pos.y;

	Vec3f objcenter = Vec3f_ZERO;
	float maxdist = 0.f;
	float miny = 99999999.f;
	float maxy = -99999999.f;
	Vec3f minoff;
	Vec3f maxoff;
	maxoff = minoff = io->obj->vertexlist[0].v;
	for(size_t i = 0; i < io->obj->vertexlist.size(); i++) {
		maxoff = glm::max(maxoff, io->obj->vertexlist[i].v);
		minoff = glm::min(minoff, io->obj->vertexlist[i].v);
		miny = std::min(miny, io->obj->vertexlist[i].v.y);
		maxy = std::max(maxy, io->obj->vertexlist[i].v.y);
	}
	
	Vec3f orgn,dest,mvectx;
	mvectx.x = -std::sin(radians(player.angle.getPitch() - 90.f));
	mvectx.y = 0;
	mvectx.z = +std::cos(radians(player.angle.getPitch() - 90.f));
	mvectx = glm::normalize(mvectx);

	float xmod = (float)(DANAEMouse.x-g_size.center().x) / (float)g_size.center().x*160.f;
	float ymod = (float)(DANAEMouse.y-g_size.center().y) / (float)g_size.center().y*220.f;
	mvectx *= xmod;
	Vec3f mvecty(0, ymod, 0);

	orgn.x=player.pos.x- std::sin(radians(player.angle.getPitch())) * std::cos(radians(player.angle.getYaw()))*50.f + mvectx.x;
	orgn.y=player.pos.y+ std::sin(radians(player.angle.getYaw()))*50.f + mvectx.y + mvecty.y;
	orgn.z=player.pos.z+ std::cos(radians(player.angle.getPitch())) * std::cos(radians(player.angle.getYaw()))*50.f + mvectx.z;

	dest.x=player.pos.x- std::sin(radians(player.angle.getPitch())) * std::cos(radians(player.angle.getYaw()))*10000.f + mvectx.x;
	dest.y=player.pos.y+ std::sin(radians(player.angle.getYaw()))*10000.f + mvectx.y + mvecty.y * 5.f;
	dest.z=player.pos.z+ std::cos(radians(player.angle.getPitch())) * std::cos(radians(player.angle.getYaw()))*10000.f + mvectx.z;
	pos = orgn;

	Vec3f movev = glm::normalize(dest - orgn);

	float lastanything = 0.f;
	float height = -(maxy - miny);

	if(height > -30.f)
		height = -30.f;

	objcenter.x = minoff.x + (maxoff.x - minoff.x) * 0.5f;
	objcenter.y = 0;
	objcenter.z = minoff.z + (maxoff.z - minoff.z) * 0.5f;

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

	objcenter = VRotateY(objcenter, temp.getPitch());

	maxdist = clamp(maxdist, 15.f, 150.f);

	bool bCollidposNoInit = true;
	Vec3f collidpos = Vec3f_ZERO;
	EERIE_CYLINDER cyl2;
	float inc = 10.f;
	long iterating = 40;

	cyl2.height = std::min(-30.f, height);
	cyl2.radius = std::max(20.f, maxdist);


	while(iterating > 0) {
		cyl2.origin.x = pos.x + movev.x * inc;
		cyl2.origin.y = pos.y + movev.y * inc + maxy;
		cyl2.origin.z = pos.z + movev.z * inc;

		float anything = CheckAnythingInCylinder(&cyl2, io, CFLAG_JUST_TEST | CFLAG_COLLIDE_NOCOL | CFLAG_NO_NPC_COLLIDE);

		if(anything < 0.f) {
			if(iterating == 40) {
				CANNOT_PUT_IT_HERE = 1;
				return -1;
			}

			iterating = 0;

			collidpos = cyl2.origin;
			bCollidposNoInit = false;

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

	collidpos.x -= objcenter.x;
	collidpos.z -= objcenter.z;

	pos.x -= objcenter.x;
	pos.z -= objcenter.z;

	if(iterating != -1) {
		CANNOT_PUT_IT_HERE = 1;
		return 0;
	}

	if(iterating == -1 && closerThan(player.pos, pos, 300.f)) {
		if(simulate) {
			ARX_INTERACTIVE_Teleport(io, pos, true);

			io->gameFlags &= ~GFLAG_NOCOMPUTATION;

			if(bCollidposNoInit) {
				ARX_DEAD_CODE();
			}

			glm::quat rotation = glm::toQuat(toRotationMatrix(temp));
			
			if(SPECIAL_DRAGINTER_RENDER) {
			if(EEfabs(lastanything) > EEfabs(height)) {
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
			if(EEfabs(lastanything) > std::min(EEfabs(height), 12.0f)) {
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

		GRenderer->SetCulling(Renderer::CullNone);
		return 1;
	} else {
		CANNOT_PUT_IT_HERE=-1;
	}

	return 0;
}

extern long LOOKING_FOR_SPELL_TARGET;
extern unsigned long LOOKING_FOR_SPELL_TARGET_TIME;
extern bool PLAYER_INTERFACE_HIDE_COUNT;
extern bool COMBINEGOLD;
extern bool PLAYER_MOUSELOOK_ON;
extern E_ARX_STATE_MOUSE eMouseState;
extern long MAGICMODE;
extern TextureContainer * scursor[];
extern long lCursorRedistValue;
extern TextureContainer * Movable;
extern TextureContainer * pTCCrossHair;

int iHighLight=0;
float fHighLightAng=0.f;
long CURCURTIME=0;
long CURCURPOS=0;
long CURCURDELAY=70;


bool SelectSpellTargetCursorRender() {
	
	if(   !SPECIAL_DRAGINTER_RENDER
	   && LOOKING_FOR_SPELL_TARGET
	) {
		if(float(arxtime) > LOOKING_FOR_SPELL_TARGET_TIME + 7000) {
			ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE, &player.pos);
			ARX_SPELLS_CancelSpellTarget();
		}
		
		TextureContainer * surf;
		
		if(FlyingOverIO
			&& (((LOOKING_FOR_SPELL_TARGET & 1) && (FlyingOverIO->ioflags & IO_NPC))
			||  ((LOOKING_FOR_SPELL_TARGET & 2) && (FlyingOverIO->ioflags & IO_ITEM)))
		){
			surf = cursorTargetOn;
			
			if(!(EERIEMouseButton & 1) && (LastMouseClick & 1)) {
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


void ARX_INTERFACE_RenderCursorInternal(bool flag) {
	
	if(SelectSpellTargetCursorRender()) {
		return;
	}
	
	if(flag || (!BLOCK_PLAYER_CONTROLS && !PLAYER_INTERFACE_HIDE_COUNT)) {
		if(!SPECIAL_DRAGINTER_RENDER)
			GRenderer->SetCulling(Renderer::CullNone);

		if(COMBINE || COMBINEGOLD) {
			if(SpecialCursor == CURSOR_INTERACTION_ON)
				SpecialCursor = CURSOR_COMBINEON;
			else
				SpecialCursor = CURSOR_COMBINEOFF;
		}

		if(!SPECIAL_DRAGINTER_RENDER) {
			if(FlyingOverIO || DRAGINTER) {
				fHighLightAng += (float)(framedelay*0.5);

				if(fHighLightAng > 90.f)
					fHighLightAng = 90.f;

				float fHLight = 100.f * sin(radians(fHighLightAng));

				iHighLight = checked_range_cast<int>(fHLight);
			} else {
				fHighLightAng = 0.f;
				iHighLight = 0;
			}
		}

		if ((SpecialCursor) || !PLAYER_MOUSELOOK_ON || (DRAGINTER!=NULL)
				|| ((FlyingOverIO) && PLAYER_MOUSELOOK_ON && !(ARX_MOUSE_OVER & ARX_MOUSE_OVER_BOOK)
			&& (eMouseState != MOUSE_IN_NOTE)
			&& (FlyingOverIO->ioflags & IO_ITEM)
			&& (FlyingOverIO->gameFlags & GFLAG_INTERACTIVITY)
			&& (config.input.autoReadyWeapon == false))
			|| ((MAGICMODE==1) && PLAYER_MOUSELOOK_ON))
		{

			CANNOT_PUT_IT_HERE=0;
			float ag=player.angle.getYaw();

			if(ag > 180)
				ag = ag - 360;

			float drop_miny=(float)(g_size.center().y)-g_size.center().y*(ag*( 1.0f / 70 ));

			if(   DANAEMouse.y > drop_miny
			   && DRAGINTER
			   && !InInventoryPos(DANAEMouse)
			   && !(ARX_MOUSE_OVER & ARX_MOUSE_OVER_BOOK)
			) {
				if(Manage3DCursor(true) == 0)
					CANNOT_PUT_IT_HERE = -1;

				if(SPECIAL_DRAGINTER_RENDER) {
					CANNOT_PUT_IT_HERE=0;
					return;
				}
			} else {
				CANNOT_PUT_IT_HERE = -1;
			}

			if(SPECIAL_DRAGINTER_RENDER)
				return;

			Vec2f mousePos = Vec2f(DANAEMouse);

			if(SpecialCursor && !DRAGINTER) {
				if((COMBINE && COMBINE->inv) || COMBINEGOLD) {
					if(TRUE_PLAYER_MOUSELOOK_ON && (config.input.autoReadyWeapon)) {
						mousePos = MemoMouse;
					}

					TextureContainer * tc;

					if(COMBINEGOLD)
						tc = GoldCoinsTC[5];
					else
						tc = COMBINE->inv;
					
					Vec2f size(tc->m_dwWidth, tc->m_dwHeight);

					if(SpecialCursor == CURSOR_COMBINEON) {
						EERIEDrawBitmap(Rectf(mousePos, size.x, size.y), .00001f, tc, Color::white);

						if(FlyingOverIO && (FlyingOverIO->ioflags & IO_BLACKSMITH)) {
							float v=ARX_DAMAGES_ComputeRepairPrice(COMBINE,FlyingOverIO);

							if(v > 0.f) {
								long t = v;
								Vec2f nuberOffset = Vec2f(-16, -10);
								ARX_INTERFACE_DrawNumber(mousePos + nuberOffset, t, 6, Color::cyan);
							}
						}
					} else {
						EERIEDrawBitmap(Rectf(mousePos, size.x, size.y), 0.00001f, tc, Color::fromBGRA(0xFFFFAA66));
					}
				}
				
				TextureContainer * surf;
				
				switch(SpecialCursor) {
				case CURSOR_REDIST:
					surf = ITC.Get("ptexcursorredist");
					break;
				case CURSOR_COMBINEOFF:
					surf = cursorTargetOff;
					mousePos.x -= 16.f;
					mousePos.y -= 16.f;
					break;
				case CURSOR_COMBINEON:
					surf = cursorTargetOn;

					if(surf)
						mousePos.x -= 16.f;

					mousePos.y -= 16.f;
					break;
				case CURSOR_FIREBALLAIM: {
					surf = cursorTargetOn;

					Vec2i size;
					if(surf) {
						size = Vec2i(surf->m_dwWidth, surf->m_dwHeight);
					} else {
						ARX_DEAD_CODE();
						size = Vec2i_ZERO;
					}

					mousePos.x = 320.f - size.x / 2.f;
					mousePos.y = 280.f - size.y / 2.f;
					break;
				}
				case CURSOR_INTERACTION_ON:
					CURCURTIME += checked_range_cast<long>(Original_framedelay);

					if(CURCURPOS!=3) {
						while(CURCURTIME > CURCURDELAY) {
							CURCURTIME -= CURCURDELAY;
							CURCURPOS++;
						}
					}

					if(CURCURPOS > 7)
						CURCURPOS = 0;

					surf = scursor[CURCURPOS];
					break;
				default:
					if(CURCURPOS) {
						CURCURTIME += checked_range_cast<long>(Original_framedelay);

						while(CURCURTIME > CURCURDELAY) {
							CURCURTIME -= CURCURDELAY;
							CURCURPOS++;
						}
					}

					if(CURCURPOS > 7)
						CURCURPOS = 0;

					surf = scursor[CURCURPOS];
					break;
				}

				if(surf) {
					if(SpecialCursor == CURSOR_REDIST) {
						EERIEDrawBitmap(Rectf(mousePos, surf->m_dwWidth * g_sizeRatio.x, surf->m_dwHeight * g_sizeRatio.y),
										0.f, surf, Color::white);

						Vec2f textPos = Vec2f(DANAEMouse);
						textPos += Vec2f(6, 11) * g_sizeRatio;
						
						std::stringstream ss;
						ss << std::setw(3) << lCursorRedistValue;
						ARX_TEXT_Draw(hFontInBook, textPos, ss.str(), Color::black);
					} else {

						EERIEDrawBitmap(Rectf(mousePos, surf->m_dwWidth, surf->m_dwHeight), 0.f, surf, Color::white);
					}
				}

				SpecialCursor = 0;
			} else {
				if(   !(player.Current_Movement & PLAYER_CROUCH)
				   && !BLOCK_PLAYER_CONTROLS
				   && GInput->actionPressed(CONTROLS_CUST_MAGICMODE)
				   && ARXmenu.currentmode == AMCM_OFF
				) {
					if(MAGICMODE < 0) {
						if(player.Interface & INTER_MAP) {
							ARX_INTERFACE_BookOpenClose(2); // Forced Closing
						}
						MAGICMODE = 1;
					}

					TextureContainer * surf = ITC.Get("magic");

					Vec2f pos = Vec2f(DANAEMouse);
					
					if(TRUE_PLAYER_MOUSELOOK_ON) {
						pos = MemoMouse;
					}
					
					Vec2f size(surf->m_dwWidth, surf->m_dwHeight);
					
					pos += -size * 0.5f;
					
					EERIEDrawBitmap(Rectf(pos, size.x, size.y), 0.f, surf, Color::white);
				} else {
					if(MAGICMODE > -1) {
						ARX_SOUND_Stop(SND_MAGIC_DRAW);
						MAGICMODE = -1;
					}

					if(DRAGINTER && DRAGINTER->inv) {
						TextureContainer * tc = DRAGINTER->inv;
						TextureContainer * tc2 = NULL;
						
						if(NeedHalo(DRAGINTER))
							tc2 = DRAGINTER->inv->getHalo();//>_itemdata->halo_tc;

						Color color = (DRAGINTER->poisonous && DRAGINTER->poisonous_count != 0) ? Color::green : Color::white;

						Vec2f pos = mousePos;
						
						if(TRUE_PLAYER_MOUSELOOK_ON && config.input.autoReadyWeapon) {
							pos = MemoMouse;
						}
						
						Rectf rect(pos, tc->m_dwWidth, tc->m_dwHeight);

						if(!(DRAGINTER->ioflags & IO_MOVABLE)) {
							EERIEDrawBitmap(rect, .00001f, tc, color);

							if((DRAGINTER->ioflags & IO_ITEM) && DRAGINTER->_itemdata->count != 1) {
								Vec2f nuberOffset = Vec2f(2.f, 13.f);
								ARX_INTERFACE_DrawNumber(pos + nuberOffset, DRAGINTER->_itemdata->count, 3, Color::white);
							}
						} else {
							if((InInventoryPos(DANAEMouse) || InSecondaryInventoryPos(DANAEMouse)) || CANNOT_PUT_IT_HERE != -1) {
								EERIEDrawBitmap(rect, .00001f, tc, color);
							}
						}

						//cross not over inventory icon
						if(CANNOT_PUT_IT_HERE && (eMouseState != MOUSE_IN_INVENTORY_ICON)) {
							if(!InInventoryPos(DANAEMouse) && !InSecondaryInventoryPos(DANAEMouse) && !ARX_INTERFACE_MouseInBook()) {
								TextureContainer * tcc = Movable;

								if(CANNOT_PUT_IT_HERE == -1)
									tcc = ThrowObject;

								if(tcc && tcc != tc) // to avoid movable double red cross...
									EERIEDrawBitmap(Rectf(Vec2f(pos.x + 16, pos.y), tcc->m_dwWidth, tcc->m_dwHeight), 0.00001f, tcc, Color::white);
							}
						}

						if(tc2) {
							ARX_INTERFACE_HALO_Draw(DRAGINTER, tc, tc2, pos.x, pos.y, 1.f, 1.f);
						}
					} else {
						if(CURCURPOS != 0) {
							CURCURTIME += checked_range_cast<long>(Original_framedelay);
							while(CURCURTIME > CURCURDELAY) {
								CURCURTIME -= CURCURDELAY;
								CURCURPOS++;
							}
						}

						if(CURCURPOS > 7)
							CURCURPOS = 0;

						TextureContainer * surf = scursor[CURCURPOS];

						if(surf) {
							EERIEDrawBitmap(Rectf(mousePos, surf->m_dwWidth, surf->m_dwHeight), 0.f, surf, Color::white);
						}
					}
				}
			}
		} else { //mode system shock
			if(SPECIAL_DRAGINTER_RENDER)
				return;

			if(TRUE_PLAYER_MOUSELOOK_ON && config.video.showCrosshair) {
				if(!(player.Interface & (INTER_COMBATMODE | INTER_NOTE | INTER_MAP))) {

					CURCURPOS = 0;

					TextureContainer * surf = pTCCrossHair ? pTCCrossHair : cursorTargetOff;

					if(surf) {
						GRenderer->SetRenderState(Renderer::AlphaBlending, true);
						GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);

						float POSX = g_size.center().x - surf->m_dwWidth * .5f;
						float POSY = g_size.center().y - surf->m_dwHeight * .5f;

						EERIEDrawBitmap(Rectf(Vec2f(POSX, POSY), surf->m_dwWidth, surf->m_dwHeight), 0.f, surf, Color3f::gray(.5f).to<u8>());

						GRenderer->SetRenderState(Renderer::AlphaBlending, false);
					}
				}
			}
		}
	}
}

void ARX_INTERFACE_RenderCursor(bool flag)
{
	if (!SPECIAL_DRAGINTER_RENDER)
	{
		ManageIgnition_2(DRAGINTER);
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

