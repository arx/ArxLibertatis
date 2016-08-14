/*
 * Copyright 2011-2013 Arx Libertatis Team (see the AUTHORS file)
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
/* Based on:
===========================================================================
ARX FATALIS GPL Source Code
Copyright (C) 1999-2010 Arkane Studios SA, a ZeniMax Media company.

This file is part of the Arx Fatalis GPL Source Code ('Arx Fatalis Source Code').

Arx Fatalis Source Code is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Arx Fatalis Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Arx Fatalis Source Code.  If not, see
<http://www.gnu.org/licenses/>.

In addition, the Arx Fatalis Source Code is also subject to certain additional terms. You should have received a copy of these
additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Arx
Fatalis Source Code. If not, please request a copy in writing from Arkane Studios at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing Arkane Studios, c/o
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved

#include "gui/Interface.h"

#include <stddef.h>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <set>
#include <utility>
#include <vector>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/foreach.hpp>

#include "animation/Animation.h"
#include "animation/AnimationRender.h"

#include "core/Application.h"
#include "core/ArxGame.h"
#include "core/Config.h"
#include "core/GameTime.h"
#include "core/Localisation.h"
#include "core/Core.h"

#include "game/Damage.h"
#include "game/EntityManager.h"
#include "game/Equipment.h"
#include "game/Inventory.h"
#include "game/Item.h"
#include "game/Levels.h"
#include "game/NPC.h"
#include "game/Player.h"
#include "game/spell/FlyingEye.h"

#include "graphics/BaseGraphicsTypes.h"
#include "graphics/Color.h"
#include "graphics/GraphicsTypes.h"
#include "graphics/Draw.h"
#include "graphics/Math.h"
#include "graphics/Renderer.h"
#include "graphics/Vertex.h"
#include "graphics/data/Mesh.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/effects/PolyBoom.h"
#include "graphics/effects/Halo.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/texture/TextureStage.h"
#include "graphics/texture/Texture.h"

#include "gui/Cursor.h"
#include "gui/Hud.h"
#include "gui/Menu.h"
#include "gui/Speech.h"
#include "gui/MiniMap.h"
#include "gui/TextManager.h"
#include "gui/Text.h"
#include "gui/hud/PlayerInventory.h"
#include "gui/hud/SecondaryInventory.h"

#include "input/Input.h"
#include "input/Keyboard.h"

#include "io/resource/ResourcePath.h"

#include "math/Angle.h"
#include "math/Random.h"
#include "math/Rectangle.h"
#include "math/Vector.h"

#include "physics/Box.h"
#include "physics/Collisions.h"
#include "platform/profiler/Profiler.h"

#include "scene/LinkedObject.h"
#include "scene/GameSound.h"
#include "scene/Interactive.h"
#include "scene/Light.h"

#include "script/Script.h"

#include "window/RenderWindow.h"


extern float Original_framedelay;
extern EERIE_3DOBJ *arrowobj;

struct ARX_INTERFACE_HALO_STRUCT
{
	Entity  * io;
	TextureContainer * tc;
	TextureContainer * tc2;
	Vec2f m_pos;
	Vec2f ratio;
};

extern TextureContainer * inventory_font;

extern float PLAYER_ROTATION;
extern float SLID_VALUE;

long lSLID_VALUE = 0;

extern bool BLOCK_PLAYER_CONTROLS;
extern long DeadTime;

extern bool WILLRETURNTOFREELOOK;

extern bool SHOW_INGAME_MINIMAP;

//-----------------------------------------------------------------------------

std::vector<ARX_INTERFACE_HALO_STRUCT> deferredUiHalos;

E_ARX_STATE_MOUSE	eMouseState;
Vec2s MemoMouse;

INVENTORY_DATA *	TSecondaryInventory;
Entity * FlyingOverIO=NULL;
Entity *	STARTED_ACTION_ON_IO=NULL;

INTERFACE_TC g_bookResouces = INTERFACE_TC();

gui::Note openNote;

bool				bInventoryClosing = false;

extern PlatformInstant SLID_START;

Vec2f				BOOKDEC = Vec2f(0.f, 0.f);

bool				PLAYER_MOUSELOOK_ON = false;
bool				TRUE_PLAYER_MOUSELOOK_ON = false;
static bool MEMO_PLAYER_MOUSELOOK_ON = false;

bool				COMBINEGOLD = false;

bool				DRAGGING = false;
bool				MAGICMODE = false;
long				SpecialCursor=0;

ArxInstant COMBAT_MODE_ON_START_TIME = ArxInstant_ZERO;
static long SPECIAL_DRAW_WEAPON = 0;
bool bGCroucheToggle=false;


bool bInverseInventory = false;
bool lOldTruePlayerMouseLook = TRUE_PLAYER_MOUSELOOK_ON;
bool bForceEscapeFreeLook = false;
bool bRenderInCursorMode = true;

long lChangeWeapon=0;
Entity *pIOChangeWeapon=NULL;

PlayerInterfaceFlags lOldInterface;


void ARX_INTERFACE_DrawNumber(const Vec2f & pos, const long num, const int _iNb, const Color color, float scale) {
	
	ColorRGBA col = color.toRGBA();
	
	TexturedVertex v[4];
	v[0] = TexturedVertex(Vec3f_ZERO, 1.f, ColorRGBA(1), Vec2f_ZERO);
	v[1] = TexturedVertex(Vec3f_ZERO, 1.f, ColorRGBA(1), Vec2f_X_AXIS);
	v[2] = TexturedVertex(Vec3f_ZERO, 1.f, ColorRGBA(1), Vec2f(1.f, 1.f));
	v[3] = TexturedVertex(Vec3f_ZERO, 1.f, ColorRGBA(1), Vec2f_Y_AXIS);
	
	v[0].p.z = v[1].p.z = v[2].p.z = v[3].p.z = 0.0000001f;

	if(inventory_font) {
		
		char tx[7];
		float ttx;
		float divideX = 1.f/((float) inventory_font->m_size.x);
		float divideY = 1.f/((float) inventory_font->m_size.y);
		
		sprintf(tx, "%*ld", _iNb, num); // TODO use a safe string function.
		long removezero = 1;

		for(long i = 0; i < 6 && tx[i] != '\0'; i++) {

			long tt = tx[i] - '0';

			if(tt == 0 && removezero)
				continue;

			if(tt >= 0) {
				removezero = 0;
				v[0].p.x = v[3].p.x = pos.x + i * (10 * scale);
				v[1].p.x = v[2].p.x = v[0].p.x + (10 * scale);
				v[0].p.y = v[1].p.y = pos.y;
				v[2].p.y = v[3].p.y = pos.y + (10 * scale);
				v[0].color = v[1].color = v[2].color = v[3].color = col;

				ttx = ((float)tt * (float)11.f) + 1.5f;
				v[3].uv.x = v[0].uv.x = ttx * divideX;
				v[1].uv.x = v[2].uv.x = (ttx + 10.f) * divideX;

				ttx = 0.5f * divideY;
				v[1].uv.y = v[0].uv.y = divideY + ttx;
				v[2].uv.y = v[3].uv.y = divideY * 12;
				GRenderer->SetTexture(0, inventory_font);

				EERIEDRAWPRIM(Renderer::TriangleFan, v, 4);
			}
		}
	}
}

void INTERFACE_TC::init() {
	
	playerbook            = TextureContainer::LoadUI("graph/interface/book/character_sheet/char_sheet_book");
	ic_casting            = TextureContainer::LoadUI("graph/interface/book/character_sheet/buttons_carac/icone_casting");
	ic_close_combat       = TextureContainer::LoadUI("graph/interface/book/character_sheet/buttons_carac/icone_close_combat");
	ic_constitution       = TextureContainer::LoadUI("graph/interface/book/character_sheet/buttons_carac/icone_constit");
	ic_defense            = TextureContainer::LoadUI("graph/interface/book/character_sheet/buttons_carac/icone_defense");
	ic_dexterity          = TextureContainer::LoadUI("graph/interface/book/character_sheet/buttons_carac/icone_dext");
	ic_etheral_link       = TextureContainer::LoadUI("graph/interface/book/character_sheet/buttons_carac/icone_etheral_link");
	ic_mind               = TextureContainer::LoadUI("graph/interface/book/character_sheet/buttons_carac/icone_intel");
	ic_intuition          = TextureContainer::LoadUI("graph/interface/book/character_sheet/buttons_carac/icone_intuition");
	ic_mecanism           = TextureContainer::LoadUI("graph/interface/book/character_sheet/buttons_carac/icone_mecanism");
	ic_object_knowledge   = TextureContainer::LoadUI("graph/interface/book/character_sheet/buttons_carac/icone_obj_knowledge");
	ic_projectile         = TextureContainer::LoadUI("graph/interface/book/character_sheet/buttons_carac/icone_projectile");
	ic_stealth            = TextureContainer::LoadUI("graph/interface/book/character_sheet/buttons_carac/icone_stealth");
	ic_strength           = TextureContainer::LoadUI("graph/interface/book/character_sheet/buttons_carac/icone_strenght");
	
	questbook        = TextureContainer::LoadUI("graph/interface/book/questbook");
	ptexspellbook    = TextureContainer::LoadUI("graph/interface/book/spellbook");
	bookmark_char    = TextureContainer::LoadUI("graph/interface/book/bookmark_char");
	bookmark_magic   = TextureContainer::LoadUI("graph/interface/book/bookmark_magic");
	bookmark_map     = TextureContainer::LoadUI("graph/interface/book/bookmark_map");
	bookmark_quest   = TextureContainer::LoadUI("graph/interface/book/bookmark_quest");
	
	accessibleTab[0]   = TextureContainer::LoadUI("graph/interface/book/accessible/accessible_1");
	accessibleTab[1]   = TextureContainer::LoadUI("graph/interface/book/accessible/accessible_2");
	accessibleTab[2]   = TextureContainer::LoadUI("graph/interface/book/accessible/accessible_3");
	accessibleTab[3]   = TextureContainer::LoadUI("graph/interface/book/accessible/accessible_4");
	accessibleTab[4]   = TextureContainer::LoadUI("graph/interface/book/accessible/accessible_5");
	accessibleTab[5]   = TextureContainer::LoadUI("graph/interface/book/accessible/accessible_6");
	accessibleTab[6]   = TextureContainer::LoadUI("graph/interface/book/accessible/accessible_7");
	accessibleTab[7]   = TextureContainer::LoadUI("graph/interface/book/accessible/accessible_8");
	accessibleTab[8]   = TextureContainer::LoadUI("graph/interface/book/accessible/accessible_9");
	accessibleTab[9]   = TextureContainer::LoadUI("graph/interface/book/accessible/accessible_10");
	currentTab[0]   = TextureContainer::LoadUI("graph/interface/book/current_page/current_1");
	currentTab[1]   = TextureContainer::LoadUI("graph/interface/book/current_page/current_2");
	currentTab[2]   = TextureContainer::LoadUI("graph/interface/book/current_page/current_3");
	currentTab[3]   = TextureContainer::LoadUI("graph/interface/book/current_page/current_4");
	currentTab[4]   = TextureContainer::LoadUI("graph/interface/book/current_page/current_5");
	currentTab[5]   = TextureContainer::LoadUI("graph/interface/book/current_page/current_6");
	currentTab[6]   = TextureContainer::LoadUI("graph/interface/book/current_page/current_7");
	currentTab[7]   = TextureContainer::LoadUI("graph/interface/book/current_page/current_8");
	currentTab[8]   = TextureContainer::LoadUI("graph/interface/book/current_page/current_9");
	currentTab[9]   = TextureContainer::LoadUI("graph/interface/book/current_page/current_10");
	
	
	arx_assert(playerbook);
	arx_assert(ic_casting);
	arx_assert(ic_close_combat);
	arx_assert(ic_constitution);
	arx_assert(ic_defense);
	arx_assert(ic_dexterity);
	arx_assert(ic_etheral_link);
	arx_assert(ic_mind);
	arx_assert(ic_intuition);
	arx_assert(ic_mecanism);
	arx_assert(ic_object_knowledge);
	arx_assert(ic_projectile);
	arx_assert(ic_stealth);
	arx_assert(ic_strength);
	
	arx_assert(questbook);
	arx_assert(ptexspellbook);
	arx_assert(bookmark_char);
	arx_assert(bookmark_magic);
	arx_assert(bookmark_map);
	arx_assert(bookmark_quest);
	
	arx_assert(accessibleTab[1]);
	arx_assert(accessibleTab[1]);
	arx_assert(accessibleTab[2]);
	arx_assert(accessibleTab[3]);
	arx_assert(accessibleTab[4]);
	arx_assert(accessibleTab[5]);
	arx_assert(accessibleTab[6]);
	arx_assert(accessibleTab[7]);
	arx_assert(accessibleTab[8]);
	arx_assert(accessibleTab[9]);
	arx_assert(currentTab[0]);
	arx_assert(currentTab[1]);
	arx_assert(currentTab[2]);
	arx_assert(currentTab[3]);
	arx_assert(currentTab[4]);
	arx_assert(currentTab[5]);
	arx_assert(currentTab[6]);
	arx_assert(currentTab[7]);
	arx_assert(currentTab[8]);
	arx_assert(currentTab[9]);
	
	g_bookResouces.Level = getLocalised("system_charsheet_player_lvl");
	g_bookResouces.Xp = getLocalised("system_charsheet_player_xp");
}


//-----------------------------------------------------------------------------
void ARX_INTERFACE_HALO_Render(Color3f color,
							   long _lHaloType,
							   TextureContainer * haloTexture,
							   Vec2f pos, Vec2f ratio)
{
	float power = 0.9f;
	power -= std::sin(arxtime.get_frame_time()*0.01f) * 0.3f;

	color.r = glm::clamp(color.r * power, 0.f, 1.f);
	color.g = glm::clamp(color.g * power, 0.f, 1.f);
	color.b = glm::clamp(color.b * power, 0.f, 1.f);
	Color col = Color4f(color).to<u8>();

	if(_lHaloType & HALO_NEGATIVE) {
		GRenderer->SetBlendFunc(BlendZero, BlendInvSrcColor);
	} else {
		GRenderer->SetBlendFunc(BlendSrcAlpha, BlendOne);
	}
	
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	
	float x = pos.x - TextureContainer::HALO_RADIUS * ratio.x;
	float y = pos.y - TextureContainer::HALO_RADIUS * ratio.y;
	float width = haloTexture->m_size.x * ratio.x;
	float height = haloTexture->m_size.y * ratio.y;
	
	EERIEDrawBitmap(Rectf(Vec2f(x, y), width, height), 0.00001f, haloTexture, col);

	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
}

void ARX_INTERFACE_HALO_Draw(Entity * io, TextureContainer * tc, TextureContainer * tc2, Vec2f pos, Vec2f ratio) {
	
	ARX_INTERFACE_HALO_STRUCT halo;
	halo.io = io;
	halo.tc = tc;
	halo.tc2 = tc2;
	halo.m_pos = pos;
	halo.ratio = ratio;
	
	deferredUiHalos.push_back(halo);
}

void ReleaseHalo() {
	
	deferredUiHalos.clear();
}

void ARX_INTERFACE_HALO_Flush() {
	
	BOOST_FOREACH(ARX_INTERFACE_HALO_STRUCT & halo, deferredUiHalos) {
		ARX_INTERFACE_HALO_Render(
		halo.io->halo.color,
		halo.io->halo.flags,
		halo.tc2, halo.m_pos, halo.ratio);
	}
	
	deferredUiHalos.clear();
}

//-----------------------------------------------------------------------------
bool NeedHalo(Entity * io)
{
	if(!io || !(io->ioflags & IO_ITEM))
		return false;

	if(io->halo.flags & HALO_ACTIVE) {
		if(io->m_icon)
			io->m_icon->getHalo();

		return true;
	}

	return false;
}


void InventoryOpenClose(unsigned long t) {
	
	if(t == 1 && (player.Interface & INTER_INVENTORY))
		return;

	if(t == 2 && !(player.Interface & INTER_INVENTORY))
		return;

	ARX_SOUND_PlayInterface(SND_BACKPACK, Random::getf(0.9f, 1.1f));

	if((player.Interface & INTER_INVENTORY) || (player.Interface & INTER_INVENTORYALL)) {
		bInventoryClosing = true;

		if(WILLRETURNTOFREELOOK) {
			TRUE_PLAYER_MOUSELOOK_ON = true;
			SLID_START = g_platformTime.frameStart();
			WILLRETURNTOFREELOOK = false;
		}
	} else {
		player.Interface |= INTER_INVENTORY;
		InventoryY = 100;

		if(TRUE_PLAYER_MOUSELOOK_ON)
			WILLRETURNTOFREELOOK = true;
	}

	if(((player.Interface & INTER_INVENTORYALL) || TRUE_PLAYER_MOUSELOOK_ON) && (player.Interface & INTER_NOTE))
		ARX_INTERFACE_NoteClose();

	if(!bInventoryClosing && config.input.autoReadyWeapon == false)
		TRUE_PLAYER_MOUSELOOK_ON = false;
}

void ARX_INTERFACE_NoteClear() {
	player.Interface &= ~INTER_NOTE;
	openNote.clear();
}

void ARX_INTERFACE_NoteOpen(gui::Note::Type type, const std::string & text) {
	
	if(player.Interface & INTER_NOTE) {
		ARX_INTERFACE_NoteClose();
	}
	
	ARX_INTERFACE_BookClose();
	
	openNote.setData(type, getLocalised(text));
	openNote.setPage(0);
	
	player.Interface |= INTER_NOTE;
	
	switch(openNote.type()) {
		case gui::Note::Notice:
			ARX_SOUND_PlayInterface(SND_MENU_CLICK, Random::getf(0.9f, 1.1f));
			break;
		case gui::Note::Book:
			ARX_SOUND_PlayInterface(SND_BOOK_OPEN, Random::getf(0.9f, 1.1f));
			break;
		case gui::Note::SmallNote:
		case gui::Note::BigNote:
			ARX_SOUND_PlayInterface(SND_SCROLL_OPEN, Random::getf(0.9f, 1.1f));
			break;
		default: break;
	}
	
	if(TRUE_PLAYER_MOUSELOOK_ON && type == gui::Note::Book) {
		TRUE_PLAYER_MOUSELOOK_ON = false;
	}
	
	if(player.Interface & INTER_INVENTORYALL) {
		bInventoryClosing = true;
		ARX_SOUND_PlayInterface(SND_BACKPACK, Random::getf(0.9f, 1.1f));
	}
}

void ARX_INTERFACE_NoteClose() {
	
	if(!(player.Interface & INTER_NOTE)) {
		return;
	}
	
	switch(openNote.type()) {
		case gui::Note::Notice: {
			ARX_SOUND_PlayInterface(SND_MENU_CLICK, Random::getf(0.9f, 1.1f));
			break;
		}
		case gui::Note::Book:
			ARX_SOUND_PlayInterface(SND_BOOK_CLOSE, Random::getf(0.9f, 1.1f));
			break;
		case gui::Note::SmallNote:
		case gui::Note::BigNote:
			ARX_SOUND_PlayInterface(SND_SCROLL_CLOSE, Random::getf(0.9f, 1.1f));
			break;
		default: break;
	}
	
	ARX_INTERFACE_NoteClear();
}

void ARX_INTERFACE_NoteManage() {
	
	if(!(player.Interface & INTER_NOTE)) {
		return;
	}
	
	if(gui::manageNoteActions(openNote)) {
		ARX_INTERFACE_NoteClose();
	}
	
	openNote.render();
}


//-------------------------------------------------------------------------------------
// Keyboard/Mouse Management
//-------------------------------------------------------------------------------------

void ResetPlayerInterface() {
	player.Interface |= INTER_LIFE_MANA;
	g_hudRoot.playerInterfaceFader.resetSlid();
	lSLID_VALUE = 0;
	SLID_START = g_platformTime.frameStart();
}

static void ReleaseInfosCombine() {
	
	arx_assert(player.bag >= 0);
	arx_assert(player.bag <= 3);
	
	for(size_t bag = 0; bag < size_t(player.bag); bag++)
	for(size_t y = 0; y < INVENTORY_Y; y++)
	for(size_t x = 0; x < INVENTORY_X; x++) {
		Entity * io = inventory[bag][x][y].io;

		if(io)
			io->ioflags &= ~IO_CAN_COMBINE;
	}

	if(SecondaryInventory) {
		for(long y = 0; y < SecondaryInventory->m_size.y; y++)
		for(long x = 0; x < SecondaryInventory->m_size.x; x++) {
			Entity * io = SecondaryInventory->slot[x][y].io;

			if(io)
				io->ioflags &= ~IO_CAN_COMBINE;
		}
	}
}

//-----------------------------------------------------------------------------
static char * findParam(char * pcToken, const char * param) {
	
	char* pStartString = 0;

	if(strstr(pcToken,"^$param1"))
		pStartString = strstr(pcToken, param);

	return pStartString;
}

static void GetInfosCombineWithIO(Entity * combine, Entity * _pWithIO) {
	
	if(!combine)
		return;
	
	std::string tcIndent = combine->idString();
	
	if(_pWithIO && _pWithIO != combine && _pWithIO->script.data) {
		char* pCopyScript = new char[_pWithIO->script.size + 1];
		pCopyScript[_pWithIO->script.size] = '\0';
		memcpy(pCopyScript, _pWithIO->script.data, _pWithIO->script.size);
		
		char* pCopyOverScript = NULL;
		
		if(_pWithIO->over_script.data) {
			pCopyOverScript = new char[_pWithIO->over_script.size + 1];
			pCopyOverScript[_pWithIO->over_script.size] = '\0';
			memcpy(pCopyOverScript, _pWithIO->over_script.data, _pWithIO->over_script.size);
		}
		
		char *pcFound = NULL;
		
		if(pCopyOverScript) {
			pcFound = strstr((char*)pCopyOverScript, "on combine");
			
			if(pcFound) {
				unsigned int uiNbOpen = 0;
				
				char *pcToken = strtok(pcFound, "\r\n");
				
				if(strstr(pcToken, "{")) {
					uiNbOpen++;
				}
				
				while(pcToken) {
					pcToken = strtok(NULL, "\r\n");
					
					bool bCanCombine = false;
					char* pStartString;
					char* pEndString;
					
					pStartString = findParam(pcToken, "isclass");
					if(pStartString) {
						pStartString = strstr(pStartString, "\"");
						
						if(pStartString) {
							pStartString++;
							pEndString = strstr(pStartString, "\"");
							
							if(pEndString) {
								char tTxtCombineDest[256];
								memcpy(tTxtCombineDest, pStartString, pEndString - pStartString);
								tTxtCombineDest[pEndString - pStartString] = 0;
								
								if(tTxtCombineDest == combine->className()) {
									//same class
									bCanCombine = true;
								}
							}
						}
					} else {
						pStartString = findParam(pcToken, "==");
						if(pStartString) {
							pStartString = strstr(pStartString, "\"");
							
							if(pStartString) {
								pStartString++;
								pEndString = strstr(pStartString, "\"");
								
								if(pEndString) {
									char tTxtCombineDest[256];
									memcpy(tTxtCombineDest, pStartString, pEndString - pStartString);
									tTxtCombineDest[pEndString - pStartString] = 0;
									
									if(tTxtCombineDest == tcIndent) {
										//same class
										bCanCombine=true;
									}
								}
							}
						} else {
							pStartString = findParam(pcToken, "isgroup");
							if(pStartString) {
								pStartString = strstr(pStartString, " ");
								
								if(pStartString) {
									pStartString++;
									pEndString = strstr(pStartString, " ");
									char* pEndString2 = strstr(pStartString, ")");
									
									if(pEndString2 < pEndString) {
										pEndString = pEndString2;
									}
									
									if(pEndString) {
										char tTxtCombineDest[256];
										memcpy(tTxtCombineDest, pStartString, pEndString - pStartString);
										tTxtCombineDest[pEndString - pStartString] = 0;
										if(combine->groups.find(tTxtCombineDest) != combine->groups.end()) {
											//same class
											bCanCombine = true;
										}
									}
								}
							}
						}
					}
					
					if(strstr(pcToken, "{")) {
						uiNbOpen++;
					}
					
					if(strstr(pcToken, "}")) {
						uiNbOpen--;
					}
					
					if(bCanCombine) {
						uiNbOpen = 0;
						_pWithIO->ioflags |= IO_CAN_COMBINE;
					} else {
						_pWithIO->ioflags &= ~IO_CAN_COMBINE;
					}
					
					if(!uiNbOpen) {
						break;
					}
				}
			}
		}
		
		if(_pWithIO->ioflags & IO_CAN_COMBINE) {
			delete[] pCopyScript;
			delete[] pCopyOverScript;
			return;
		}
		
		pcFound = strstr((char*)pCopyScript, "on combine");
		
		if(pcFound) {
			unsigned int uiNbOpen=0;
			
			char *pcToken = strtok(pcFound, "\r\n");
			
			if(strstr(pcToken,"{")) {
				uiNbOpen++;
			}
			
			while(pcToken) {
				pcToken = strtok(NULL, "\r\n");
				
				bool bCanCombine = false;
				char* pStartString;
				char* pEndString;
				
				pStartString = findParam(pcToken, "isclass");
				if(pStartString) {
					pStartString = strstr(pStartString, "\"");
					
					if(pStartString) {
						pStartString++;
						pEndString = strstr(pStartString, "\"");
						
						if(pEndString) {
							char tTxtCombineDest[256];
							memcpy(tTxtCombineDest, pStartString, pEndString - pStartString);
							tTxtCombineDest[pEndString - pStartString] = 0;
							
							if(tTxtCombineDest == combine->className()) {
								//same class
								bCanCombine = true;
							}
						}
					}
				} else {
					pStartString = findParam(pcToken, "==");
					if(pStartString) {
						pStartString = strstr(pStartString, "\"");
						
						if(pStartString) {
							pStartString++;
							pEndString = strstr(pStartString, "\"");
							
							if(pEndString) {
								char tTxtCombineDest[256];
								memcpy(tTxtCombineDest, pStartString, pEndString - pStartString);
								tTxtCombineDest[pEndString - pStartString] = 0;
								
								if(tTxtCombineDest == tcIndent) {
									//same class
									bCanCombine = true;
								}
							}
						}
					} else {
						pStartString = findParam(pcToken, "isgroup");
						if(pStartString) {
							pStartString = strstr(pStartString, " ");
							
							if(pStartString) {
								pStartString++;
								pEndString = strstr(pStartString, " ");
								char* pEndString2 = strstr(pStartString, ")");
								
								if(pEndString2 < pEndString) {
									pEndString = pEndString2;
								}
								
								if(pEndString) {
									char tTxtCombineDest[256];
									memcpy(tTxtCombineDest, pStartString, pEndString - pStartString);
									tTxtCombineDest[pEndString - pStartString] = 0;
									
									if(combine->groups.find(tTxtCombineDest) != combine->groups.end()) {
										// same class
										bCanCombine = true;
									}
								}
							}
						}
					}
				}
				
				if(strstr(pcToken, "{")) {
					uiNbOpen++;
				}
				
				if(strstr(pcToken, "}")) {
					uiNbOpen--;
				}
				
				if(bCanCombine) {
					uiNbOpen = 0;
					_pWithIO->ioflags |= IO_CAN_COMBINE;
				} else {
					_pWithIO->ioflags &= ~IO_CAN_COMBINE;
				}
				
				if(!uiNbOpen) {
					break;
				}
			}
		}
		
		delete[] pCopyScript;
		delete[] pCopyOverScript;
	}
}

static void GetInfosCombine() {
	
	arx_assert(player.bag >= 0);
	arx_assert(player.bag <= 3);
	
	for(size_t bag = 0; bag < size_t(player.bag); bag++)
	for(size_t y = 0; y < INVENTORY_Y; y++)
	for(size_t x = 0; x < INVENTORY_X; x++) {
		Entity * io = inventory[bag][x][y].io;
		GetInfosCombineWithIO(COMBINE, io);
	}

	if(SecondaryInventory) {
		for(long y = 0; y < SecondaryInventory->m_size.y; y++)
		for(long x = 0; x < SecondaryInventory->m_size.x; x++) {
			Entity * io = SecondaryInventory->slot[x][y].io;
			GetInfosCombineWithIO(COMBINE, io);
		}
	}
}

//-----------------------------------------------------------------------------
// Switches from/to combat Mode i=-1 for switching from actual configuration
// 2 to force Draw Weapon
// 1 to force Combat Mode On
// 0 to force Combat Mode Off
//-----------------------------------------------------------------------------
void ARX_INTERFACE_Combat_Mode(long i) {
	arx_assert(entities.player());
	arx_assert(arrowobj);
	
	if(i >= 1 && (player.Interface & INTER_COMBATMODE))
		return;

	if(i == 0 && !(player.Interface & INTER_COMBATMODE))
		return;

	if((player.Interface & INTER_COMBATMODE)) {
		player.Interface&=~INTER_COMBATMODE;
		player.Interface&=~INTER_NO_STRIKE;

		ARX_EQUIPMENT_LaunchPlayerUnReadyWeapon();
		WeaponType weapontype = ARX_EQUIPMENT_GetPlayerWeaponType();

		if(weapontype == WEAPON_BOW) {
			EERIE_LINKEDOBJ_UnLinkObjectFromObject(entities.player()->obj, arrowobj);
		}

		player.doingmagic=0;
	} else if(   !entities.player()->animlayer[1].cur_anim
	           || entities.player()->animlayer[1].cur_anim == entities.player()->anims[ANIM_WAIT]
	) {
		ARX_INTERFACE_BookClose();

		player.Interface|=INTER_COMBATMODE;

		if(i==2) {
			player.Interface|=INTER_NO_STRIKE;

			if(config.input.mouseLookToggle) {
				TRUE_PLAYER_MOUSELOOK_ON = true;
				SLID_START = g_platformTime.frameStart();
			}
		}

		ARX_EQUIPMENT_LaunchPlayerReadyWeapon();
		player.doingmagic=0;
	}
}
long CSEND=0;
long MOVE_PRECEDENCE=0;


extern ArxInstant REQUEST_JUMP;
//-----------------------------------------------------------------------------
void ArxGame::managePlayerControls() {
	
	ARX_PROFILE_FUNC();
	
	if(   eeMouseDoubleClick1()
	   && !(player.Interface & INTER_COMBATMODE)
	   && !player.doingmagic
	   && !g_cursorOverBook
	   && eMouseState != MOUSE_IN_NOTE
	) {
		Entity *t = InterClick(DANAEMouse);

		if(t) {
			if(t->ioflags & IO_NPC) {
				if(t->script.data) {
					if(t->_npcdata->lifePool.current>0.f) {
						SendIOScriptEvent(t,SM_CHAT);
						DRAGGING = false;
					} else {
						if(t->inventory) {
							if(player.Interface & INTER_STEAL)
								if(ioSteal && t != ioSteal) {
									SendIOScriptEvent(ioSteal, SM_STEAL,"off");
									player.Interface &= ~INTER_STEAL;
								}
							
							ARX_INVENTORY_OpenClose(t);
							
							if(player.Interface & (INTER_INVENTORY | INTER_INVENTORYALL)) {
								ARX_SOUND_PlayInterface(SND_BACKPACK, Random::getf(0.9f, 1.1f));
							}
							
							if(SecondaryInventory) {
								bForceEscapeFreeLook=true;
								lOldTruePlayerMouseLook=!TRUE_PLAYER_MOUSELOOK_ON;
							}
						}
					}
				}
			} else {
				if(t->inventory) {
					if(player.Interface & INTER_STEAL) {
						if(ioSteal && t != ioSteal) {
							SendIOScriptEvent(ioSteal, SM_STEAL,"off");
							player.Interface &= ~INTER_STEAL;
						}
					}

					ARX_INVENTORY_OpenClose(t);

					if(SecondaryInventory) {
						bForceEscapeFreeLook=true;
						lOldTruePlayerMouseLook=!TRUE_PLAYER_MOUSELOOK_ON;
					}
				} else if (t->script.data) {
					SendIOScriptEvent(t,SM_ACTION);
				}
				DRAGGING = false;
			}
		}
	}

	float MoveDiv;
	
	// Checks STEALTH Key Status.
	if(GInput->actionPressed(CONTROLS_CUST_STEALTHMODE)) {
		MoveDiv=0.02f;
		player.m_currentMovement |= PLAYER_MOVE_STEALTH;
	} else {
		MoveDiv=0.0333333f;
	}

	{
		long NOMOREMOVES=0;
		float FD = 1.f;

		if(eyeball.exist == 2) {
			FD = 18.f;
			Vec3f old = eyeball.pos;

			// Checks WALK_FORWARD Key Status.
			if(GInput->actionPressed(CONTROLS_CUST_WALKFORWARD)) {
				eyeball.pos += angleToVectorXZ(eyeball.angle.getYaw()) * 20.f * FD * 0.033f;
				NOMOREMOVES=1;
			}

			// Checks WALK_BACKWARD Key Status.
			if(GInput->actionPressed(CONTROLS_CUST_WALKBACKWARD)) {
				eyeball.pos += angleToVectorXZ_180offset(eyeball.angle.getYaw()) * 20.f * FD * 0.033f;
				NOMOREMOVES=1;
			}

			// Checks STRAFE_LEFT Key Status.
			if( (GInput->actionPressed(CONTROLS_CUST_STRAFELEFT)||
				(GInput->actionPressed(CONTROLS_CUST_STRAFE)&&GInput->actionPressed(CONTROLS_CUST_TURNLEFT)))
				&& !NOMOREMOVES)
			{
				eyeball.pos += angleToVectorXZ(eyeball.angle.getYaw() + 90.f) * 10.f * FD * 0.033f;
				NOMOREMOVES=1;			
			}

			// Checks STRAFE_RIGHT Key Status.
			if( (GInput->actionPressed(CONTROLS_CUST_STRAFERIGHT)||
				(GInput->actionPressed(CONTROLS_CUST_STRAFE)&&GInput->actionPressed(CONTROLS_CUST_TURNRIGHT)))
				&& !NOMOREMOVES)
			{
				eyeball.pos += angleToVectorXZ(eyeball.angle.getYaw() - 90.f) * 10.f * FD * 0.033f;
				//eyeball.pos.y+=FD*0.33f;
				NOMOREMOVES=1;
			}

			IO_PHYSICS phys;
			phys.cyl.height = -110.f;
			phys.cyl.origin = eyeball.pos + Vec3f(0.f, 70.f, 0.f);
			phys.cyl.radius = 45.f;

			Cylinder test = phys.cyl;
			
			bool npc = AttemptValidCylinderPos(test, NULL, CFLAG_JUST_TEST | CFLAG_NPC);
			float val=CheckAnythingInCylinder(phys.cyl,entities.player(),CFLAG_NO_NPC_COLLIDE | CFLAG_JUST_TEST);

			if(val > -40.f) {
				if(val <= 70.f) {
					eyeball.pos.y += val-70.f;
				}

				if(!npc) {
					MagicSightFader+=g_framedelay*( 1.0f / 200 );

					if(MagicSightFader > 1.f)
						MagicSightFader = 1.f;
				}
			} else {
				eyeball.pos = old;
			}
		}

		if(arxtime.is_paused())
			FD = 40.f;

		bool left=GInput->actionPressed(CONTROLS_CUST_STRAFELEFT);

		if(!left) {
			if(GInput->actionPressed(CONTROLS_CUST_STRAFE)&&GInput->actionPressed(CONTROLS_CUST_TURNLEFT)) {
				left = true;
			}
		}

		bool right=GInput->actionPressed(CONTROLS_CUST_STRAFERIGHT);

		if(!right) {
			if(GInput->actionPressed(CONTROLS_CUST_STRAFE)&&GInput->actionPressed(CONTROLS_CUST_TURNRIGHT)) {
				right = true;
			}
		}
		
		Vec3f tm = Vec3f_ZERO;
		
		// Checks WALK_BACKWARD Key Status.
		if(GInput->actionPressed(CONTROLS_CUST_WALKBACKWARD) && !NOMOREMOVES) {
			player.m_strikeDirection=3;
			float multi = 1;

			if(left || right) {
				multi = 0.8f;
			}
			
			multi = 5.f * FD * MoveDiv * multi;
			tm += angleToVectorXZ_180offset(player.angle.getYaw()) * multi;

			if(!USE_PLAYERCOLLISIONS) {
				float t = glm::radians(player.angle.getPitch());
				tm.y -= std::sin(t) * multi;
			}

			player.m_currentMovement |= PLAYER_MOVE_WALK_BACKWARD;

			if(GInput->actionNowPressed(CONTROLS_CUST_WALKBACKWARD)) {
				MOVE_PRECEDENCE=PLAYER_MOVE_WALK_BACKWARD;
			}
		} else if(MOVE_PRECEDENCE == PLAYER_MOVE_WALK_BACKWARD) {
			MOVE_PRECEDENCE = 0;
		}

		// Checks WALK_FORWARD Key Status.
		if(GInput->actionPressed(CONTROLS_CUST_WALKFORWARD) && !NOMOREMOVES) {
			player.m_strikeDirection=2;
			float multi = 1;

			if(left || right) {
				multi=0.8f;
			}
			
			multi = 10.f * FD * MoveDiv * multi;
			tm += angleToVectorXZ(player.angle.getYaw()) * multi;

			if(!USE_PLAYERCOLLISIONS) {
				float t = glm::radians(player.angle.getPitch());
				tm.y += std::sin(t) * multi;
			}

			player.m_currentMovement |= PLAYER_MOVE_WALK_FORWARD;

			if(GInput->actionNowPressed(CONTROLS_CUST_WALKFORWARD)) {
				MOVE_PRECEDENCE=PLAYER_MOVE_WALK_FORWARD;
			}
		} else if(MOVE_PRECEDENCE == PLAYER_MOVE_WALK_FORWARD) {
			MOVE_PRECEDENCE = 0;
		}

		// Checks STRAFE_LEFT Key Status.
		if(left && !NOMOREMOVES) {
			player.m_strikeDirection=0;
			float multi = 6.f * FD * MoveDiv;
			tm += angleToVectorXZ(player.angle.getYaw() + 90.f) * multi;
			
			player.m_currentMovement |= PLAYER_MOVE_STRAFE_LEFT;

			if(GInput->actionNowPressed(CONTROLS_CUST_STRAFELEFT)) {
				MOVE_PRECEDENCE=PLAYER_MOVE_STRAFE_LEFT;
			}
		} else if(MOVE_PRECEDENCE == PLAYER_MOVE_STRAFE_LEFT) {
			MOVE_PRECEDENCE = 0;
		}

		// Checks STRAFE_RIGHT Key Status.
		if(right && !NOMOREMOVES) {
			player.m_strikeDirection=1;
			float multi = 6.f * FD * MoveDiv;
			tm += angleToVectorXZ(player.angle.getYaw() - 90.f) * multi;
			
			player.m_currentMovement |= PLAYER_MOVE_STRAFE_RIGHT;

			if(GInput->actionNowPressed(CONTROLS_CUST_STRAFERIGHT)) {
				MOVE_PRECEDENCE=PLAYER_MOVE_STRAFE_RIGHT;
			}
		} else if(MOVE_PRECEDENCE == PLAYER_MOVE_STRAFE_RIGHT) {
			MOVE_PRECEDENCE = 0;
		}

		g_moveto = player.pos + tm;
	}

	// Checks CROUCH Key Status.
	if(GInput->actionNowPressed(CONTROLS_CUST_CROUCHTOGGLE)) {
		bGCroucheToggle = !bGCroucheToggle;
	}

	if(GInput->actionPressed(CONTROLS_CUST_CROUCH) || bGCroucheToggle) {
		player.m_currentMovement |= PLAYER_CROUCH;
	}

	if(GInput->actionNowPressed(CONTROLS_CUST_UNEQUIPWEAPON)) {
		ARX_EQUIPMENT_UnEquipPlayerWeapon();
	}

	// Can only lean outside of combat mode
	if(!(player.Interface & INTER_COMBATMODE)) {
		// Checks LEAN_LEFT Key Status.
		if(GInput->actionPressed(CONTROLS_CUST_LEANLEFT))
			player.m_currentMovement |= PLAYER_LEAN_LEFT;

		// Checks LEAN_RIGHT Key Status.
		if(GInput->actionPressed(CONTROLS_CUST_LEANRIGHT))
			player.m_currentMovement |= PLAYER_LEAN_RIGHT;
	}
	
	// Checks JUMP Key Status.
	if(player.jumpphase == NotJumping && GInput->actionNowPressed(CONTROLS_CUST_JUMP)) {
		REQUEST_JUMP = arxtime.now();
	}
	
	// MAGIC
	if(GInput->actionPressed(CONTROLS_CUST_MAGICMODE)) {
		if(!(player.m_currentMovement & PLAYER_CROUCH) && !BLOCK_PLAYER_CONTROLS && ARXmenu.currentmode == AMCM_OFF) {
			if(!ARX_SOUND_IsPlaying(SND_MAGIC_AMBIENT))
				ARX_SOUND_PlaySFX(SND_MAGIC_AMBIENT, NULL, 1.0F, ARX_SOUND_PLAY_LOOPED);
		}
	} else {
		ARX_SOUND_Stop(SND_MAGIC_AMBIENT);
		ARX_SOUND_Stop(SND_MAGIC_DRAW);
	}

	if(GInput->actionNowPressed(CONTROLS_CUST_DRINKPOTIONLIFE)) {
		SendInventoryObjectCommand("graph/obj3d/textures/item_potion_life", SM_INVENTORYUSE);
	}

	if(GInput->actionNowPressed(CONTROLS_CUST_DRINKPOTIONMANA)) {
		SendInventoryObjectCommand("graph/obj3d/textures/item_potion_mana", SM_INVENTORYUSE);
	}

	if(GInput->actionNowPressed(CONTROLS_CUST_TORCH)) {
		if(player.torch) {
			ARX_PLAYER_KillTorch();
		} else {
			Entity * io = ARX_INVENTORY_GetTorchLowestDurability();

			if(io) {
				Entity * ioo = io;
				
				if(io->_itemdata->count > 1) {
					ioo = CloneIOItem(io);
					ioo->show = SHOW_FLAG_NOT_DRAWN;
					ioo->scriptload = 1;
					ioo->_itemdata->count = 1;
					io->_itemdata->count--;
				}
				
				ARX_PLAYER_ClickedOnTorch(ioo);
			}
		}
	}

	if(GInput->actionNowPressed(CONTROLS_CUST_MINIMAP)) {
		SHOW_INGAME_MINIMAP = !SHOW_INGAME_MINIMAP;
	}

	if(GInput->actionNowPressed(CONTROLS_CUST_PREVIOUS)) {
		if(eMouseState == MOUSE_IN_BOOK) {
			if(player.Interface & INTER_MAP) {
				openBookPage(prevBookPage());
			}
		} else if(g_playerInventoryHud.containsPos(DANAEMouse)) {
			g_playerInventoryHud.previousBag();
		} else if(player.Interface & INTER_MAP) {
			openBookPage(prevBookPage());
		} else {
			g_playerInventoryHud.previousBag();
		}
	}

	if(GInput->actionNowPressed(CONTROLS_CUST_NEXT)) {
		if(eMouseState == MOUSE_IN_BOOK) {
			if(player.Interface & INTER_MAP) {
				openBookPage(nextBookPage());
			}
		} else if(g_playerInventoryHud.containsPos(DANAEMouse)) {
			g_playerInventoryHud.nextBag();
		} else if(player.Interface & INTER_MAP) {
			openBookPage(nextBookPage());
		} else {
			g_playerInventoryHud.nextBag();
		}
	}
	
	if(GInput->actionNowPressed(CONTROLS_CUST_BOOKCHARSHEET)) {
		openBookPage(BOOKMODE_STATS, true);
	}
	
	if(GInput->actionNowPressed(CONTROLS_CUST_BOOKSPELL) && player.rune_flags) {
		openBookPage(BOOKMODE_SPELLS, true);
	}
	
	if(GInput->actionNowPressed(CONTROLS_CUST_BOOKMAP)) {
		openBookPage(BOOKMODE_MINIMAP, true);
	}
	
	if(GInput->actionNowPressed(CONTROLS_CUST_BOOKQUEST)) {
		openBookPage(BOOKMODE_QUESTS, true);
	}
	
	if(GInput->actionNowPressed(CONTROLS_CUST_CANCELCURSPELL)) {
		for(long i = MAX_SPELLS - 1; i >= 0; i--) {
			SpellBase * spell = spells[SpellHandle(i)];
			
			if(spell && spell->m_caster == EntityHandle_Player)
				if(spellicons[spell->m_type].m_hasDuration) {
					ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE);
					spells.endSpell(spell);
					break;
				}
		}
	}

	if(((player.Interface & INTER_COMBATMODE) && !bIsAiming) || !player.doingmagic) {
		if(GInput->actionNowPressed(CONTROLS_CUST_PRECAST1)) {
			ARX_SPELLS_Precast_Launch(PrecastHandle(0));
		}
	
		if(GInput->actionNowPressed(CONTROLS_CUST_PRECAST2)) {
			ARX_SPELLS_Precast_Launch(PrecastHandle(1));
		}
	
		if(GInput->actionNowPressed(CONTROLS_CUST_PRECAST3)) {
			ARX_SPELLS_Precast_Launch(PrecastHandle(2));
		}
	}

	if(GInput->actionNowPressed(CONTROLS_CUST_WEAPON) || lChangeWeapon) {
		bool bGo = true; 

		if(lChangeWeapon > 0) {
			if(lChangeWeapon == 2) {
				lChangeWeapon--;
			} else {
				if(!entities.player()->animlayer[1].cur_anim ||
					entities.player()->animlayer[1].cur_anim == entities.player()->anims[ANIM_WAIT])
				{
					lChangeWeapon--;

					if(pIOChangeWeapon) {
						SendIOScriptEvent(pIOChangeWeapon,SM_INVENTORYUSE,"");
						pIOChangeWeapon=NULL;
					}
				} else {
					bGo=false;
				}
			}
		}

		if(bGo) {
			if(player.Interface & INTER_COMBATMODE) {
				ARX_INTERFACE_Combat_Mode(0);
				SPECIAL_DRAW_WEAPON=0;

				if(config.input.mouseLookToggle)
					TRUE_PLAYER_MOUSELOOK_ON=MEMO_PLAYER_MOUSELOOK_ON;
			} else {
				MEMO_PLAYER_MOUSELOOK_ON=TRUE_PLAYER_MOUSELOOK_ON;
				SPECIAL_DRAW_WEAPON=1;
				TRUE_PLAYER_MOUSELOOK_ON = true;
				SLID_START = g_platformTime.frameStart();
				ARX_INTERFACE_Combat_Mode(2);
			}
		}
	}
	
	if(bForceEscapeFreeLook) {
		TRUE_PLAYER_MOUSELOOK_ON = false;

		if(!GInput->actionPressed(CONTROLS_CUST_FREELOOK)) {
			bForceEscapeFreeLook=false;
		}
	} else {
		if(eMouseState!=MOUSE_IN_INVENTORY_ICON) {
			if(!config.input.mouseLookToggle) {
				if(GInput->actionPressed(CONTROLS_CUST_FREELOOK)) {
					if(!TRUE_PLAYER_MOUSELOOK_ON) {
						TRUE_PLAYER_MOUSELOOK_ON = true;
						SLID_START = g_platformTime.frameStart();
					}
				} else {
					TRUE_PLAYER_MOUSELOOK_ON = false;
				}
			} else {
				if(GInput->actionNowPressed(CONTROLS_CUST_FREELOOK)) {
					if(!TRUE_PLAYER_MOUSELOOK_ON) {
						ARX_INTERFACE_BookClose();
						TRUE_PLAYER_MOUSELOOK_ON = true;
						SLID_START = g_platformTime.frameStart();
					} else {
						TRUE_PLAYER_MOUSELOOK_ON = false;

						if(player.Interface & INTER_COMBATMODE)
							ARX_INTERFACE_Combat_Mode(0);			
					}
				}
			}
		}
	}

	if((player.Interface & INTER_COMBATMODE) && GInput->actionNowReleased(CONTROLS_CUST_FREELOOK)) {
		ARX_INTERFACE_Combat_Mode(0);
	}

	// Checks INVENTORY Key Status.
	if(GInput->actionNowPressed(CONTROLS_CUST_INVENTORY)) {
		if(player.Interface & INTER_COMBATMODE) {
			ARX_INTERFACE_Combat_Mode(0);
		}

		bInverseInventory=!bInverseInventory;
		lOldTruePlayerMouseLook=TRUE_PLAYER_MOUSELOOK_ON;

		if(!config.input.mouseLookToggle) {
			bForceEscapeFreeLook=true;
		}
	}

	// Checks BOOK Key Status.
	if(GInput->actionNowPressed(CONTROLS_CUST_BOOK))
		ARX_INTERFACE_BookToggle();

	// Check For Combat Mode ON/OFF
	if(   eeMousePressed1()
	   && !(player.Interface & INTER_COMBATMODE)
	   && !g_cursorOverBook
	   && !SpecialCursor
	   && PLAYER_MOUSELOOK_ON
	   && !DRAGINTER
	   && !InInventoryPos(DANAEMouse)
	   && config.input.autoReadyWeapon
	) {
		if(eeMouseDown1()) {
			// TODO use os time
			COMBAT_MODE_ON_START_TIME = arxtime.now();
		} else {
			if(arxtime.now() - COMBAT_MODE_ON_START_TIME > ArxDurationMs(10)) {
				ARX_INTERFACE_Combat_Mode(1);
			}
		}
	}

	if(lOldTruePlayerMouseLook != TRUE_PLAYER_MOUSELOOK_ON) {
		bInverseInventory=false;

		if(TRUE_PLAYER_MOUSELOOK_ON) {
			if(!CSEND) {
				CSEND=1;
				SendIOScriptEvent(entities.player(),SM_EXPLORATIONMODE);
			}
		}
	} else {
		if(CSEND) {
			CSEND=0;
			SendIOScriptEvent(entities.player(),SM_CURSORMODE);
		}
	}

	static PlayerInterfaceFlags lOldInterfaceTemp=0;

	if(TRUE_PLAYER_MOUSELOOK_ON) {
		if(bInverseInventory) {
			bRenderInCursorMode=true;

			if(!MAGICMODE) {
				InventoryOpenClose(1);
			}
		} else {
			if(!bInventoryClosing) {
				g_hudRoot.mecanismIcon.reset();
				
				if(player.Interface & INTER_INVENTORYALL) {
					ARX_SOUND_PlayInterface(SND_BACKPACK, Random::getf(0.9f, 1.1f));
					bInventoryClosing = true;
					lOldInterfaceTemp=INTER_INVENTORYALL;
				}

				bRenderInCursorMode=false;
				InventoryOpenClose(2);

				if(player.Interface &INTER_INVENTORY) {
					g_secondaryInventoryHud.close();
				}

				if(config.input.mouseLookToggle) {
					TRUE_PLAYER_MOUSELOOK_ON = true;
					SLID_START = g_platformTime.frameStart();
				}
			}
		}
	} else {
		if(bInverseInventory) {
			if(!bInventoryClosing) {
				bRenderInCursorMode=false;

				InventoryOpenClose(2);

				if(player.Interface &INTER_INVENTORY) {
					g_secondaryInventoryHud.close();
				}

				if(config.input.mouseLookToggle) {
					TRUE_PLAYER_MOUSELOOK_ON = true;
					SLID_START = g_platformTime.frameStart();
				}
			}
		} else {
			bRenderInCursorMode=true;

			if(!MAGICMODE) {
				if(lOldInterfaceTemp) {
					lOldInterface=lOldInterfaceTemp;
					lOldInterfaceTemp=0;
					ARX_SOUND_PlayInterface(SND_BACKPACK, Random::getf(0.9f, 1.1f));
				}

				if(lOldInterface) {
					player.Interface|=lOldInterface;
					player.Interface&=~INTER_INVENTORY;
				}
				else
					InventoryOpenClose(1);
			}
		}

		if(bRenderInCursorMode) {
			if(eyeball.exist != 0) {
				spells.endByCaster(EntityHandle_Player, SPELL_FLYING_EYE);
			}
		}
	}
}

void ARX_INTERFACE_Reset()
{
	g_hudRoot.playerInterfaceFader.reset();
	g_hudRoot.playerInterfaceFader.resetSlid();
	BLOCK_PLAYER_CONTROLS = false;
	cinematicBorder.reset();
	cinematicBorder.CINEMA_DECAL = 0;
	g_hudRoot.quickSaveIconGui.hide();
}


void ArxGame::manageKeyMouse() {
	arx_assert(entities.player());
	
	if(ARXmenu.currentmode == AMCM_OFF) {
		Entity * pIO = NULL;

		if(!BLOCK_PLAYER_CONTROLS) {
			if(TRUE_PLAYER_MOUSELOOK_ON
			   && !(player.Interface & INTER_COMBATMODE)
			   && eMouseState != MOUSE_IN_NOTE
			) {
				Vec2s poss = MemoMouse;

				// mode systemshock
				if(config.input.mouseLookToggle && config.input.autoReadyWeapon == false) {
					
					DANAEMouse = g_size.center();
					
					pIO = FlyingOverObject(DANAEMouse);
					if(pIO) {
						FlyingOverIO = pIO;
						MemoMouse = DANAEMouse;
					}
				} else {
					pIO = FlyingOverObject(poss);
				}
			} else {
				pIO = FlyingOverObject(DANAEMouse);
			}
		}

		if(pIO && g_cursorOverBook) {
			for(size_t i = 0; i < MAX_EQUIPED; i++) {
				if(ValidIONum(player.equiped[i]) && entities[player.equiped[i]] == pIO) {
					FlyingOverIO = pIO;
				}
			}
		}

		if(pIO
		   && (pIO->gameFlags & GFLAG_INTERACTIVITY)
		   && !g_cursorOverBook
		   && eMouseState != MOUSE_IN_NOTE
		) {
			if(   eeMouseUp2()
			   && STARTED_ACTION_ON_IO == pIO
			) {
				if(pIO->ioflags & IO_ITEM) {
					FlyingOverIO = pIO;
					COMBINE = NULL;

					if(DRAGINTER == NULL) {
						bool bOk = true;

						if(SecondaryInventory != NULL) {
							Entity * temp = SecondaryInventory->io;

							if(IsInSecondaryInventory(FlyingOverIO))
								if(temp->ioflags & IO_SHOP)
									bOk = false;
						}
						
						Entity * io = entities.player();
						const AnimLayer & layer1 = io->animlayer[1];
						WeaponType type = ARX_EQUIPMENT_GetPlayerWeaponType();
						
						switch(type) {
						case WEAPON_DAGGER:
							if(layer1.cur_anim == io->anims[ANIM_DAGGER_UNREADY_PART_1])
								bOk = false;
						
							break;
						case WEAPON_1H:
							if(layer1.cur_anim == io->anims[ANIM_1H_UNREADY_PART_1])
								bOk = false;
						
							break;
						case WEAPON_2H:
							if(layer1.cur_anim == io->anims[ANIM_2H_UNREADY_PART_1])
								bOk = false;
						
							break;
						case WEAPON_BOW:
							if(layer1.cur_anim == io->anims[ANIM_MISSILE_UNREADY_PART_1])
								bOk = false;
						
							break;
						default:
							break;
						}
						
						if(bOk) {
							if(!(FlyingOverIO->_itemdata->playerstacksize <= 1 && FlyingOverIO->_itemdata->count > 1)) {
								SendIOScriptEvent(FlyingOverIO, SM_INVENTORYUSE);

								if (!(config.input.autoReadyWeapon == false && config.input.mouseLookToggle)) {
									TRUE_PLAYER_MOUSELOOK_ON = false;
								}
							}
						}
					}

					if(config.input.autoReadyWeapon == false && config.input.mouseLookToggle) {
						EERIEMouseButton &= ~2;
					}
				}
			} else { //!TRUE_PLAYER_MOUSELOOK_ON
				if(eeMouseDown2()) {
					STARTED_ACTION_ON_IO = FlyingOverIO;
				}
			}
		}

		if((eMouseState == MOUSE_IN_WORLD) ||
			((eMouseState == MOUSE_IN_BOOK) && !(g_cursorOverBook && (g_guiBookCurrentTopTab != BOOKMODE_MINIMAP)))
		) {
			if(!config.input.mouseLookToggle) {
				if(TRUE_PLAYER_MOUSELOOK_ON && !(EERIEMouseButton & 2) && !SPECIAL_DRAW_WEAPON) {
					if(!GInput->actionPressed(CONTROLS_CUST_FREELOOK))
						TRUE_PLAYER_MOUSELOOK_ON = false;
				}
			}
		}

		PLAYER_MOUSELOOK_ON = TRUE_PLAYER_MOUSELOOK_ON;

		if(player.doingmagic == 2 && config.input.mouseLookToggle)
			PLAYER_MOUSELOOK_ON = false;
	}

	if(ARXmenu.currentmode != AMCM_OFF) {
		PLAYER_MOUSELOOK_ON = false;
	}

	// Checks For MouseGrabbing/Restoration after Grab
	bool bRestoreCoordMouse=true;

	static bool LAST_PLAYER_MOUSELOOK_ON = false;
	bool mouselook = PLAYER_MOUSELOOK_ON && !BLOCK_PLAYER_CONTROLS;
	if(mouselook && !LAST_PLAYER_MOUSELOOK_ON) {
		
		MemoMouse = DANAEMouse;
		GInput->setMouseMode(Mouse::Relative);
		
	} else if(!mouselook && LAST_PLAYER_MOUSELOOK_ON) {
		
		GInput->setMouseMode(Mouse::Absolute);
		DANAEMouse = MemoMouse;
		
		if(mainApp->getWindow()->isFullScreen()) {
			GInput->setMousePosAbs(DANAEMouse);
		}
		
		bRestoreCoordMouse=false;
	}
	
	LAST_PLAYER_MOUSELOOK_ON = mouselook;
	PLAYER_ROTATION=0;

	Vec2f mouseDiff = Vec2f(GInput->getMousePosRel());
	
	if(config.input.mouseAcceleration > 0) {
		Vec2f speed = mouseDiff / g_framedelay;
		Vec2f sign(speed.x < 0 ? -1.f : 1.f, speed.y < 0 ? -1.f : 1.f);
		float exponent = 1.f + config.input.mouseAcceleration * 0.05f;
		speed.x = (std::pow(speed.x * sign.x + 1.f, exponent) - 1.f) * sign.x;
		speed.y = (std::pow(speed.y * sign.y + 1.f, exponent) - 1.f) * sign.y;
		mouseDiff = speed * g_framedelay;
	}
	
	ARX_Menu_Manage();
	
	if(bRestoreCoordMouse) {
		DANAEMouse=GInput->getMousePosAbs();
	}
	
	// Player/Eyeball Freelook Management
	if(!BLOCK_PLAYER_CONTROLS) {
		GetInventoryObj_INVENTORYUSE(DANAEMouse);

		bool bKeySpecialMove=false;
		
		struct PushTime {
			unsigned long turnLeft;
			unsigned long turnRight;
			unsigned long lookUp;
			unsigned long lookDown;
		};
		
		static PushTime pushTime = {0, 0, 0, 0};
		
		if(!GInput->actionPressed(CONTROLS_CUST_STRAFE)) {
			arxtime.update();
			const ArxInstant now = arxtime.now();

			if(GInput->actionPressed(CONTROLS_CUST_TURNLEFT)) {
				if(!pushTime.turnLeft)
					pushTime.turnLeft = now;

				bKeySpecialMove = true;
			}
			else
				pushTime.turnLeft = 0;

			if(GInput->actionPressed(CONTROLS_CUST_TURNRIGHT)) {
				if(!pushTime.turnRight)
					pushTime.turnRight = now;

				bKeySpecialMove = true;
			}
			else
				pushTime.turnRight = 0;
		}

		if(USE_PLAYERCOLLISIONS) {
			arxtime.update();
			const ArxInstant now = arxtime.now();

			if(GInput->actionPressed(CONTROLS_CUST_LOOKUP)) {
				if(!pushTime.lookUp)
					pushTime.lookUp = now;

				bKeySpecialMove = true;
			}
			else
				pushTime.lookUp = 0;

			if(GInput->actionPressed(CONTROLS_CUST_LOOKDOWN)) {
				if(!pushTime.lookDown)
					pushTime.lookDown = now;

				bKeySpecialMove = true;
			}
			else
				pushTime.lookDown = 0;
		}

		if(bKeySpecialMove) {
			if(pushTime.turnLeft || pushTime.turnRight) {
				if(pushTime.turnLeft < pushTime.turnRight)
					mouseDiff.x = 10.f;
				else
					mouseDiff.x = -10.f;
			} else {
				mouseDiff.x = 0.f;
			}

			if(pushTime.lookUp || pushTime.lookDown) {
				if(pushTime.lookUp < pushTime.lookDown)
					mouseDiff.y = 10.f;
				else
					mouseDiff.y = -10.f;
			} else {
				mouseDiff.y = 0.f;
			}
		} else if(config.input.borderTurning) {
			
			// Turn the player if the curser is close to the edges
			
			bool dragging = GInput->getMouseButtonRepeat(Mouse::Button_0);
			
			static bool mouseInBorder = false;
			static ArxInstant mouseInBorderTime = ArxInstant_ZERO;
			
			if(!bRenderInCursorMode || (!dragging && !GInput->isMouseInWindow())) {
				mouseInBorder = false;
			} else {
				
				int borderSize = 10;
				ArxDuration borderDelay = ArxDurationMs(100);
				if(!dragging && !mainApp->getWindow()->isFullScreen()) {
					borderSize = 50;
					borderDelay = ArxDurationMs(200);
				}
				
				int distLeft = DANAEMouse.x - g_size.left;
				int distRight = g_size.right - DANAEMouse.x;
				int distTop = DANAEMouse.y - g_size.top;
				int distBottom = g_size.bottom - DANAEMouse.y;
				
				if(g_secondaryInventoryHud.containsPos(DANAEMouse)
				   || g_playerInventoryHud.containsPos(DANAEMouse)
				   || (!dragging && distBottom < g_size.height() / 4 && distRight <= borderSize)
				   || (!dragging && distTop <= 4 * borderSize && distRight <= 4 * borderSize)) {
					borderSize = 2;
					borderDelay = ArxDurationMs(600);
				}
				
				mouseDiff = Vec2f_ZERO;
				
				if(distLeft < borderSize) {
					float speed = 1.f - float(distLeft) / float(borderSize);
					mouseDiff.x -= speed * g_framedelay;
				}
				
				if(distRight < borderSize) {
					float speed = 1.f - float(distRight) / float(borderSize);
					mouseDiff.x += speed * g_framedelay;
				}
				
				if(distTop < borderSize) {
					float speed = 1.f - float(distTop) / float(borderSize);
					mouseDiff.y -= speed * g_framedelay;
				}
				
				if(distBottom < borderSize) {
					float speed = 1.f - float(distBottom) / float(borderSize);
					mouseDiff.y += speed * g_framedelay;
				}
				
				if(distLeft >= 3 * borderSize && distRight >= 3 * borderSize
				   && distTop >= 3 * borderSize && distBottom >= 3 * borderSize) {
					mouseInBorder = false;
				} else if(!mouseInBorder) {
					mouseInBorderTime = arxtime.now();
					mouseInBorder = true;
				}
				
				if(borderDelay > 0 && arxtime.now() - mouseInBorderTime < borderDelay) {
					mouseDiff = Vec2f_ZERO;
				} else {
					bKeySpecialMove = true;
				}
				
			}
		}

		if(GInput->actionPressed(CONTROLS_CUST_CENTERVIEW)) {
			eyeball.angle.setPitch(0);
			eyeball.angle.setRoll(0);
			player.desiredangle.setPitch(0);
			player.angle.setPitch(0);
			player.desiredangle.setRoll(0);
			player.angle.setRoll(0);
		}

		float mouseSensitivity = (((float)GInput->getMouseSensitivity()) + 1.f) * 0.1f * ((640.f / (float)g_size.width()));
		if (mouseSensitivity > 200) {
			mouseSensitivity = 200;
		}

		mouseSensitivity *= (float)g_size.width() * ( 1.0f / 640 );
		mouseSensitivity *= (1.0f / 5);
		
		Vec2f rotation = mouseDiff * mouseSensitivity;
		
		if(config.input.invertMouse)
			rotation.y *= -1.f;
		
		if(PLAYER_MOUSELOOK_ON || bKeySpecialMove) {

			if(eyeball.exist == 2) {
				if(eyeball.angle.getPitch() < 70.f) {
					if(eyeball.angle.getPitch() + rotation.y < 70.f)
						eyeball.angle.setPitch(eyeball.angle.getPitch() + rotation.y);
				} else if(eyeball.angle.getPitch() > 300.f) {
					if(eyeball.angle.getPitch() + rotation.y > 300.f)
						eyeball.angle.setPitch(eyeball.angle.getPitch() + rotation.y);
				}

				eyeball.angle.setPitch(MAKEANGLE(eyeball.angle.getPitch()));
				eyeball.angle.setYaw(MAKEANGLE(eyeball.angle.getYaw() - rotation.x));
			} else if(ARXmenu.currentmode != AMCM_NEWQUEST) {

				float iangle = player.angle.getPitch();

				player.desiredangle.setPitch(player.angle.getPitch());
				player.desiredangle.setPitch(player.desiredangle.getPitch() + rotation.y);
				player.desiredangle.setPitch(MAKEANGLE(player.desiredangle.getPitch()));

				if(player.desiredangle.getPitch() >= 74.9f && player.desiredangle.getPitch() <= 301.f) {
					if(iangle < 75.f)
						player.desiredangle.setPitch(74.9f); //69
					else
						player.desiredangle.setPitch(301.f);
				}

				if(glm::abs(rotation.y) > 2.f)
					entities.player()->animBlend.lastanimtime = ArxInstant_ZERO;

				if(rotation.x != 0.f)
					player.m_currentMovement |= PLAYER_ROTATE;

				PLAYER_ROTATION = rotation.x;

				player.desiredangle.setYaw(player.angle.getYaw());
				player.desiredangle.setYaw(MAKEANGLE(player.desiredangle.getYaw() - rotation.x));
			}
		}
	}
	
	manageEntityDescription();
}

void ArxGame::manageEntityDescription() {
	
	if(   !BLOCK_PLAYER_CONTROLS
	   && !(player.Interface & INTER_COMBATMODE)
	   && !DRAGINTER
	   && (config.input.autoDescription || (eeMouseUp1() && !eeMouseDoubleClick1() && !(LastMouseClick & 4)))
	   && FlyingOverIO
	   && !FlyingOverIO->locname.empty()
	) {
		Entity * temp = FlyingOverIO;
		
		ARX_INVENTORY_IdentifyIO(temp);
		
		std::string description = getLocalised(temp->locname);
		
		if(temp->ioflags & IO_GOLD) {
			std::stringstream ss;
			ss << temp->_itemdata->price << " " << description;
			description = ss.str();
		}
		
		if(temp->poisonous > 0 && temp->poisonous_count != 0) {
			std::string Text = getLocalised("description_poisoned", "error");
			std::stringstream ss;
			ss << " (" << Text << " " << (int)temp->poisonous << ")";
			description += ss.str();
		}
		
		if((temp->ioflags & IO_ITEM) && temp->durability < 100.f) {
			std::string Text = getLocalised("description_durability", "error");
			std::stringstream ss;
			ss << " " << Text << " " << std::fixed << std::setw(3) << std::setprecision(0) << temp->durability << "/" << temp->max_durability;
			description += ss.str();
		}
		
		bool bAddText = true;
		if(temp->obj && temp->obj->pbox && temp->obj->pbox->active == 1) {
			bAddText=false;
		}
		
		if(bAddText) {
			Rect::Num x = checked_range_cast<Rect::Num>(120 * g_sizeRatio.x);
			Rect::Num y = checked_range_cast<Rect::Num>(14 * g_sizeRatio.y);
			Rect::Num w = checked_range_cast<Rect::Num>((120 + 500) * g_sizeRatio.x);
			Rect::Num h = checked_range_cast<Rect::Num>((14 + 200) * g_sizeRatio.y);
			Rect rDraw(x, y, w, h);
			pTextManage->Clear();
			if(!config.input.autoDescription) {
				pTextManage->AddText(hFontInBook, description, rDraw, Color(232, 204, 143), 2000 + description.length()*60);
			} else {
				pTextManage->AddText(hFontInBook, description, rDraw, Color(232, 204, 143));
			}
		}
	}
}

EntityHandle LastSelectedIONum = EntityHandle();

void ArxGame::manageEditorControls() {
	
	ARX_PROFILE_FUNC();
	
	eMouseState = MOUSE_IN_WORLD;

	if(   TRUE_PLAYER_MOUSELOOK_ON
	   && config.input.autoReadyWeapon == false
	   && config.input.mouseLookToggle
	) {
		DANAEMouse = g_size.center();
	}
	
	if(eeMousePressed1()) {
		static Vec2s dragThreshold = Vec2s_ZERO;
		
		if(eeMouseDown1()) {
			
			STARTDRAG = DANAEMouse;
			DRAGGING = false;
			dragThreshold = Vec2s_ZERO;
		} else {
			dragThreshold += GInput->getMousePosRel();
			if((std::abs(DANAEMouse.x - STARTDRAG.x) > 2 && std::abs(DANAEMouse.y - STARTDRAG.y) > 2)
			   || (std::abs(dragThreshold.x) > 2 || std::abs(dragThreshold.y) > 2)) {
				DRAGGING = true;
			}
		}
	} else {
		DRAGGING = false;
	}
	
	// on ferme
	if((player.Interface & INTER_COMBATMODE) || player.doingmagic >= 2) {
		g_secondaryInventoryHud.close();
	}
	
	g_hudRoot.playerInterfaceFader.update();
	cinematicBorder.update();
	
	g_hudRoot.updateInput();
	
	// gros player book
	if(player.Interface & INTER_MAP) {
		Vec2f pos(97 * g_sizeRatio.x, 64 * g_sizeRatio.y);
		
		TextureContainer* playerbook = g_bookResouces.playerbook;
		arx_assert(g_bookResouces.playerbook);
		
		const Rect mouseTestRect(
		pos.x,
		pos.y,
		pos.x + playerbook->m_size.x * g_sizeRatio.x,
		pos.y + playerbook->m_size.y * g_sizeRatio.y
		);
		
		if(mouseTestRect.contains(Vec2i(DANAEMouse))) {
			eMouseState = MOUSE_IN_BOOK;
		}
	}
	
	// gros book/note
	if(player.Interface & INTER_NOTE) {
		if(openNote.area().contains(Vec2f(DANAEMouse))) {
			eMouseState = MOUSE_IN_NOTE;
			return;
		}
	}
	
	g_secondaryInventoryHud.updateInputButtons();
	
	// Single Click On Object
	if(   eeMouseUp1()
	   && FlyingOverIO
	   && !DRAGINTER
	) {
		SendIOScriptEvent(FlyingOverIO, SM_CLICKED);
		bool bOk = true;
		
		if(SecondaryInventory) {
			Entity *temp = SecondaryInventory->io;
			
			if(IsInSecondaryInventory(FlyingOverIO) && (temp->ioflags & IO_SHOP))
				bOk = false;
		}
		
		if(   !(FlyingOverIO->ioflags & IO_MOVABLE)
		   && (FlyingOverIO->ioflags & IO_ITEM)
		   && bOk
		   && GInput->actionPressed(CONTROLS_CUST_STEALTHMODE)
		   && !g_playerInventoryHud.containsPos(DANAEMouse)
		   && !ARX_INTERFACE_MouseInBook()
		) {
			Vec2s s = Vec2s_ZERO;
			bool bSecondary = false;
			
			if(TSecondaryInventory && IsInSecondaryInventory(FlyingOverIO)) {
				if(SecondaryInventory) {
					bool bfound = true;
					
					for(long y = 0; y < SecondaryInventory->m_size.y && bfound; y++)
					for(long x = 0; x < SecondaryInventory->m_size.x && bfound; x++) {
						const INVENTORY_SLOT & slot = SecondaryInventory->slot[x][y];
						
						if(slot.io == FlyingOverIO) {
							s = Vec2s(x, y);
							bfound = false;
						}
					}
					
					if(bfound)
						ARX_DEAD_CODE();
				}
				
				bSecondary = true;
			}
			
			RemoveFromAllInventories( FlyingOverIO );
			FlyingOverIO->show = SHOW_FLAG_IN_INVENTORY;
			
			if(FlyingOverIO->ioflags & IO_GOLD)
				ARX_SOUND_PlayInterface(SND_GOLD);
			
			ARX_SOUND_PlayInterface(SND_INVSTD);
			
			if(!playerInventory.insert(FlyingOverIO)) {
				if(TSecondaryInventory && bSecondary) {
					// TODO global sInventory
					extern short sInventory;
					extern Vec2s sInventoryPos;
					sInventory = 2;
					sInventoryPos = s;
					
					CanBePutInSecondaryInventory(TSecondaryInventory, FlyingOverIO);
				}
				
				if(!bSecondary)
					FlyingOverIO->show = SHOW_FLAG_IN_SCENE;
			}
			
			if(DRAGINTER == FlyingOverIO)
				DRAGINTER = NULL;
			
			FlyingOverIO = NULL;
		}
	}
	
	if(!(player.Interface & INTER_COMBATMODE)) {
		// Dropping an Interactive Object that has been dragged
		if(eeMouseUp1() && DRAGINTER) {
			//if (ARX_EQUIPMENT_PutOnPlayer(DRAGINTER))
			if(InInventoryPos(DANAEMouse)) {// Attempts to put it in inventory
				PutInInventory();
			} else if(ARX_INTERFACE_MouseInBook()) {
				if(g_guiBookCurrentTopTab == BOOKMODE_STATS) {
					SendIOScriptEvent(DRAGINTER,SM_INVENTORYUSE);
					COMBINE=NULL;
				}
			} else if(DRAGINTER->ioflags & IO_GOLD) {
				ARX_PLAYER_AddGold(DRAGINTER);
				Set_DragInter(NULL);
			} else if(DRAGINTER) {
				
				if(   !((DRAGINTER->ioflags & IO_ITEM) && DRAGINTER->_itemdata->count > 1)
				   && DRAGINTER->obj
				   && DRAGINTER->obj->pbox
				   && !InInventoryPos(DANAEMouse)
				   && !g_cursorOverBook
				) {
					//Put object in fromt of player
					
					bool res = Manage3DCursor(DRAGINTER, false);
					// Throw Object
					if(!res) {
						Entity * io=DRAGINTER;
						ARX_PLAYER_Remove_Invisibility();
						io->obj->pbox->active=1;
						io->obj->pbox->stopcount=0;
						io->pos = player.pos + Vec3f(0.f, 80.f, 0.f);
						io->velocity = Vec3f_ZERO;
						io->stopped = 1;
						
						Vec2f centerOffset = Vec2f(DANAEMouse) - Vec2f(g_size.center());
						Vec2f ratio;
						ratio.y = centerOffset.y / g_size.height() * 2;
						ratio.x = -centerOffset.x / g_size.center().x;
						
						Vec3f viewvector = angleToVector(player.angle + Anglef(0.f, ratio.x * 30.f, 0.f));
						viewvector.y += ratio.y;
						
						io->soundtime = ArxInstant_ZERO;
						io->soundcount=0;
						
						EERIE_PHYSICS_BOX_Launch(io->obj, io->pos, io->angle, viewvector);
						ARX_SOUND_PlaySFX(SND_WHOOSH, &io->pos);
						
						io->show=SHOW_FLAG_IN_SCENE;
						Set_DragInter(NULL);
					}
				}
			}
		}
		
		if(COMBINE && COMBINE != player.torch) {
			Vec3f pos = GetItemWorldPosition(COMBINE);
			
			if(fartherThan(pos, player.pos, 300.f))
				COMBINE = NULL;
		}
		
		if(eeMouseDown1() && (COMBINE || COMBINEGOLD)) {
			ReleaseInfosCombine();
			
			Entity * io;
			
			if((io=FlyingOverIO)!=NULL) {
				if(COMBINEGOLD) {
					SendIOScriptEvent(io, SM_COMBINE, "gold_coin");
				} else {
					if(io != COMBINE) {
						std::string temp = COMBINE->idString();
						EVENT_SENDER=COMBINE;
	
						if(boost::starts_with(COMBINE->className(), "keyring")) {
							ARX_KEYRING_Combine(io);
						} else {
							SendIOScriptEvent(io, SM_COMBINE, temp);
						}
					}
				}
			} else { // GLights
				float fMaxdist = player.m_telekinesis ? 850 : 300;
				
				for(size_t i = 0; i < g_staticLightsMax; i++) {
					EERIE_LIGHT * light = g_staticLights[i];
					
					if(   light
					   && light->exist
					   && !fartherThan(light->pos, player.pos, fMaxdist)
					   && !(light->extras & EXTRAS_NO_IGNIT)
					   && light->m_screenRect.contains(Vec2f(DANAEMouse))
					   && (COMBINE->ioflags & IO_ITEM)
					) {
						if((COMBINE == player.torch) || (COMBINE->_itemdata->LightValue == 1)) {
							if(!light->m_ignitionStatus) {
								light->m_ignitionStatus = true;
								ARX_SOUND_PlaySFX(SND_TORCH_START, &light->pos);
							}
						}
						
						if(COMBINE->_itemdata->LightValue == 0) {
							if(light->m_ignitionStatus) {
								light->m_ignitionStatus = false;
								ARX_SOUND_PlaySFX(SND_TORCH_END, &light->pos);
								SendIOScriptEvent(COMBINE, SM_CUSTOM, "douse");
							}
						}
					}
				}
			}
	
			COMBINEGOLD = false;
			bool bQuitCombine = true;
	
			if((player.Interface & INTER_INVENTORY)) {
				if(player.bag) {
					bQuitCombine = g_playerInventoryHud.updateInput();
				}
			}
	
			if(bQuitCombine) {
				COMBINE=NULL;
			}
		}
		
		//lights
		if(COMBINE) {
			float fMaxdist = player.m_telekinesis ? 850 : 300;
			
			for(size_t i = 0; i < g_staticLightsMax; i++) {
				EERIE_LIGHT * light = g_staticLights[i];
				
				if(   light
				   && light->exist
				   && !fartherThan(light->pos, player.pos, fMaxdist)
				   && !(light->extras & EXTRAS_NO_IGNIT)
				   && light->m_screenRect.contains(Vec2f(DANAEMouse))
				) {
					SpecialCursor = CURSOR_INTERACTION_ON;
				}
			}
		}

		// Double Clicked and not already combining.
		if(eeMouseDoubleClick1() && !COMBINE) {
			bool accept_combine = true;
			
			if(SecondaryInventory && g_secondaryInventoryHud.containsPos(DANAEMouse)) {
				Entity * io = SecondaryInventory->io;
				
				if(io->ioflags & IO_SHOP)
					accept_combine = false;
			}
			
			if(accept_combine) {
				if(FlyingOverIO && ((FlyingOverIO->ioflags & IO_ITEM) && !(FlyingOverIO->ioflags & IO_MOVABLE))) {
					COMBINE=FlyingOverIO;
					GetInfosCombine();
				}
			}
		}
		
		// Checks for Object Dragging
		if(   DRAGGING
		   && (!PLAYER_MOUSELOOK_ON || !config.input.autoReadyWeapon)
		   && !GInput->actionPressed(CONTROLS_CUST_MAGICMODE)
		   && !DRAGINTER
		) {
			if(!TakeFromInventory(STARTDRAG)) {
				bool bOk = false;
				
				Entity *io = InterClick(STARTDRAG);
				
				if(io && !BLOCK_PLAYER_CONTROLS) {
					if(g_cursorOverBook) {
						if(io->show == SHOW_FLAG_ON_PLAYER)
							bOk = true;
					} else {
						bOk = true;
					}
				}
				
				if(bOk) {
					Set_DragInter(io);
					
					if(io) {
						ARX_PLAYER_Remove_Invisibility();
						
						if(DRAGINTER->show == SHOW_FLAG_ON_PLAYER) {
							ARX_EQUIPMENT_UnEquip(entities.player(),DRAGINTER);
							RemoveFromAllInventories(DRAGINTER);
							DRAGINTER->bbox2D.max.x = -1;
						}
						
						if((io->ioflags & IO_NPC) || (io->ioflags & IO_FIX)) {
							Set_DragInter(NULL);
						} else {
							
							if(io->ioflags & IO_UNDERWATER) {
								io->ioflags &= ~IO_UNDERWATER;
								ARX_SOUND_PlayInterface(SND_PLOUF, Random::getf(0.8f, 1.2f));
							}
							
							DRAGINTER->show = SHOW_FLAG_NOT_DRAWN;
							ARX_SOUND_PlayInterface(SND_INVSTD);
						}
					}
				} else {
					Set_DragInter(NULL);
				}
			} else {
				ARX_PLAYER_Remove_Invisibility();
			}
		}
	
		// Debug Selection
		if(eeMouseUp1()) {
			Entity * io = GetFirstInterAtPos(DANAEMouse);

			if(io) {
				LastSelectedIONum = io->index();
			} else {
				if(LastSelectedIONum == EntityHandle())
					LastSelectedIONum = EntityHandle_Player;
				else
					LastSelectedIONum = EntityHandle();
			}
		}
	}
}
