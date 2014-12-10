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
#include "graphics/effects/DrawEffects.h"
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

extern float MagicSightFader;
extern float Original_framedelay;
extern EERIE_3DOBJ *arrowobj;

long IN_BOOK_DRAW=0;

extern EntityHandle LastSelectedIONum;

//-----------------------------------------------------------------------------
struct ARX_INTERFACE_HALO_STRUCT
{
	Entity  * io;
	TextureContainer * tc;
	TextureContainer * tc2;
	Vec2f m_pos;
	Vec2f ratio;
};
//-----------------------------------------------------------------------------
static const float BOOKMARKS_POS_X = 216.f;
static const float BOOKMARKS_POS_Y = 60.f;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
extern TextureContainer * inventory_font;
extern Notification speech[];

extern float PLAYER_ROTATION;
extern float SLID_VALUE;

extern long BOOKBUTTON;
extern long LASTBOOKBUTTON;
long lSLID_VALUE = 0;

extern bool BLOCK_PLAYER_CONTROLS;
extern long DeadTime;

extern bool WILLRETURNTOFREELOOK;

extern Vec2s DANAEMouse;

extern bool SHOW_INGAME_MINIMAP;

//-----------------------------------------------------------------------------

ARX_INTERFACE_HALO_STRUCT * aiHalo=NULL;
E_ARX_STATE_MOUSE	eMouseState;
bool bookclick;
Vec2s MemoMouse;

INVENTORY_DATA *	TSecondaryInventory;
Entity * FlyingOverIO=NULL;
Entity *	STARTED_ACTION_ON_IO=NULL;

INTERFACE_TC ITC = INTERFACE_TC();

gui::Note openNote;
static gui::Note questBook;

bool				bBookHalo = false;

bool				bInventoryClosing = false;

unsigned long		ulBookHaloTime = 0;

float				InventoryDir=0; // 0 stable, 1 to right, -1 to left

float				SLID_START=0.f; // Charging Weapon

Vec2f				BOOKDEC = Vec2f(0.f, 0.f);

bool				PLAYER_MOUSELOOK_ON = false;
bool				TRUE_PLAYER_MOUSELOOK_ON = false;
bool				LAST_PLAYER_MOUSELOOK_ON = false;
static bool MEMO_PLAYER_MOUSELOOK_ON = false;

bool				COMBINEGOLD = false;
ARX_INTERFACE_BOOK_MODE Book_Mode = BOOKMODE_STATS;
long				Book_MapPage=1;
long				Book_SpellPage=1;

long				SMOOTHSLID=0;

bool				DRAGGING = false;
bool				PLAYER_INTERFACE_HIDE_COUNT = true;
bool				MAGICMODE = false;
long				SpecialCursor=0;
long				FLYING_OVER		= 0;
long				OLD_FLYING_OVER	= 0;
long				LastRune=-1;
long				BOOKZOOM=0;
static long INTERFACE_HALO_NB = 0;
static long INTERFACE_HALO_MAX_NB = 0;
long				LastMouseClick=0;

//used to redist points - attributes and skill
long lCursorRedistValue = 0;

unsigned long		COMBAT_MODE_ON_START_TIME = 0;
static long SPECIAL_DRAW_WEAPON = 0;
bool bGCroucheToggle=false;

float INTERFACE_RATIO(float a)
{
	return a;
}
float INTERFACE_RATIO_LONG(const long a)
{
	return INTERFACE_RATIO(static_cast<float>(a));
}
float INTERFACE_RATIO_DWORD(const u32 a)
{
	return INTERFACE_RATIO(static_cast<float>(a));
}


short SHORT_INTERFACE_RATIO(const float _a) {
	float fRes = INTERFACE_RATIO(_a);
	return checked_range_cast<short>(fRes);
}


bool bInverseInventory = false;
bool lOldTruePlayerMouseLook = TRUE_PLAYER_MOUSELOOK_ON;
bool bForceEscapeFreeLook = false;
bool bRenderInCursorMode = true;

long lChangeWeapon=0;
Entity *pIOChangeWeapon=NULL;

PlayerInterfaceFlags lOldInterface;

namespace gui {

namespace {

/*!
 * Manage forward and backward buttons on notes and the quest book.
 * \return true if the note was clicked
 */
bool manageNoteActions(Note & note) {
	
	if(note.prevPageButton().contains(Vec2f(DANAEMouse))) {
		SpecialCursor = CURSOR_INTERACTION_ON;
		if(!(EERIEMouseButton & 1) && (LastMouseClick & 1)) {
			ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9f + 0.2f * rnd());
			arx_assert(note.page() >= 2);
			note.setPage(note.page() - 2);
		}
		
	} else if(note.nextPageButton().contains(Vec2f(DANAEMouse))) {
		SpecialCursor = CURSOR_INTERACTION_ON;
		if(!(EERIEMouseButton & 1) && (LastMouseClick & 1)) {
			ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9f + 0.2f * rnd());
			note.setPage(note.page() + 2);
		}
		
	} else if(note.area().contains(Vec2f(DANAEMouse))) {
		if(((EERIEMouseButton & 1) && !(LastMouseClick & 1) && TRUE_PLAYER_MOUSELOOK_ON)
		   || ((EERIEMouseButton & 2) && !(LastMouseClick & 2))) {
			EERIEMouseButton &= ~2;
			return true;
		}
	}
	
	return false;
}


//! Update and render the quest book.
void manageQuestBook() {
	
	// Cache the questbook data
	if(questBook.text().empty() && !PlayerQuest.empty()) {
		std::string text;
		for(size_t i = 0; i < PlayerQuest.size(); ++i) {
			std::string quest = getLocalised(PlayerQuest[i].ident);
			if(!quest.empty()) {
				text += quest;
				text += "\n\n";
			}
		}
		questBook.setData(Note::QuestBook, text);
		questBook.setPage(questBook.pageCount() - 1);
	}
	
	manageNoteActions(questBook);
	
	questBook.render();
}

} // anonymous namespace

void updateQuestBook() {
	// Clear the quest book cache - it will be re-created when needed
	questBook.clear();
}

} // namespace gui

//-----------------------------------------------------------------------------

static bool MouseInBookRect(const float x, const float y, const float cx, const float cy) {
	return DANAEMouse.x >= (x + BOOKDEC.x) * g_sizeRatio.x
		&& DANAEMouse.x <= (cx + BOOKDEC.x) * g_sizeRatio.x
		&& DANAEMouse.y >= (y + BOOKDEC.y) * g_sizeRatio.y
		&& DANAEMouse.y <= (cy + BOOKDEC.y) * g_sizeRatio.y;
}

bool ARX_INTERFACE_MouseInBook() {
	if((player.Interface & INTER_MAP) && !(player.Interface & INTER_COMBATMODE)) {
		return MouseInBookRect(99, 65, 599, 372);
	} else {
		return false;
	}
}

void ARX_INTERFACE_DrawNumber(const Vec2f & pos, const long num, const int _iNb, const Color color) {
	
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
		float divideX = 1.f/((float) inventory_font->m_dwWidth);
		float divideY = 1.f/((float) inventory_font->m_dwHeight);
		
		sprintf(tx, "%*ld", _iNb, num); // TODO use a safe string function.
		long removezero = 1;

		for(long i = 0; i < 6 && tx[i] != '\0'; i++) {

			long tt = tx[i] - '0';

			if(tt == 0 && removezero)
				continue;

			if(tt >= 0) {
				removezero = 0;
				v[0].p.x = v[3].p.x = pos.x + i * INTERFACE_RATIO(10);
				v[1].p.x = v[2].p.x = v[0].p.x + INTERFACE_RATIO(10);
				v[0].p.y = v[1].p.y = pos.y;
				v[2].p.y = v[3].p.y = pos.y + INTERFACE_RATIO(10);
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

void KillInterfaceTextureContainers() {
	ITC.Reset();
}

void INTERFACE_TC::Reset()
{
	ITC.Level.clear();
	ITC.Xp.clear();
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
	
	accessible_1   = TextureContainer::LoadUI("graph/interface/book/accessible/accessible_1");
	accessible_2   = TextureContainer::LoadUI("graph/interface/book/accessible/accessible_2");
	accessible_3   = TextureContainer::LoadUI("graph/interface/book/accessible/accessible_3");
	accessible_4   = TextureContainer::LoadUI("graph/interface/book/accessible/accessible_4");
	accessible_5   = TextureContainer::LoadUI("graph/interface/book/accessible/accessible_5");
	accessible_6   = TextureContainer::LoadUI("graph/interface/book/accessible/accessible_6");
	accessible_7   = TextureContainer::LoadUI("graph/interface/book/accessible/accessible_7");
	accessible_8   = TextureContainer::LoadUI("graph/interface/book/accessible/accessible_8");
	accessible_9   = TextureContainer::LoadUI("graph/interface/book/accessible/accessible_9");
	accessible_10  = TextureContainer::LoadUI("graph/interface/book/accessible/accessible_10");
	current_1   = TextureContainer::LoadUI("graph/interface/book/current_page/current_1");
	current_2   = TextureContainer::LoadUI("graph/interface/book/current_page/current_2");
	current_3   = TextureContainer::LoadUI("graph/interface/book/current_page/current_3");
	current_4   = TextureContainer::LoadUI("graph/interface/book/current_page/current_4");
	current_5   = TextureContainer::LoadUI("graph/interface/book/current_page/current_5");
	current_6   = TextureContainer::LoadUI("graph/interface/book/current_page/current_6");
	current_7   = TextureContainer::LoadUI("graph/interface/book/current_page/current_7");
	current_8   = TextureContainer::LoadUI("graph/interface/book/current_page/current_8");
	current_9   = TextureContainer::LoadUI("graph/interface/book/current_page/current_9");
	current_10  = TextureContainer::LoadUI("graph/interface/book/current_page/current_10");
	
	
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
	
	arx_assert(accessible_1);
	arx_assert(accessible_2);
	arx_assert(accessible_3);
	arx_assert(accessible_4);
	arx_assert(accessible_5);
	arx_assert(accessible_6);
	arx_assert(accessible_7);
	arx_assert(accessible_8);
	arx_assert(accessible_9);
	arx_assert(accessible_10);
	arx_assert(current_1);
	arx_assert(current_2);
	arx_assert(current_3);
	arx_assert(current_4);
	arx_assert(current_5);
	arx_assert(current_6);
	arx_assert(current_7);
	arx_assert(current_8);
	arx_assert(current_9);
	arx_assert(current_10);
	
	ITC.Level = getLocalised("system_charsheet_player_lvl");
	ITC.Xp = getLocalised("system_charsheet_player_xp");
}

static void DrawBookInterfaceItem(TextureContainer * tc, Vec2f pos, Color color = Color::white, float z = 0.000001f) {
	if(tc) {
		EERIEDrawBitmap2(Rectf(
			(pos + BOOKDEC) * g_sizeRatio,
			tc->m_dwWidth * g_sizeRatio.x,
			tc->m_dwHeight * g_sizeRatio.y),
			z,
			tc,
			color
		);
	}
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
		GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);
	} else {
		GRenderer->SetBlendFunc(Renderer::BlendSrcAlpha, Renderer::BlendOne);
	}
	
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	
	float x = pos.x - TextureContainer::HALO_RADIUS * ratio.x;
	float y = pos.y - TextureContainer::HALO_RADIUS * ratio.y;
	float width = haloTexture->m_dwWidth * ratio.x;
	float height = haloTexture->m_dwHeight * ratio.y;
	
	EERIEDrawBitmap(Rectf(Vec2f(x, y), width, height), 0.00001f, haloTexture, col);

	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
}

void ARX_INTERFACE_HALO_Draw(Entity * io, TextureContainer * tc, TextureContainer * tc2, Vec2f pos, Vec2f ratio) {
	INTERFACE_HALO_NB++;
	
	if(INTERFACE_HALO_NB > INTERFACE_HALO_MAX_NB) {
		INTERFACE_HALO_MAX_NB = INTERFACE_HALO_NB;
		aiHalo = (ARX_INTERFACE_HALO_STRUCT *)realloc(aiHalo,sizeof(ARX_INTERFACE_HALO_STRUCT)*INTERFACE_HALO_NB);
	}
	
	aiHalo[INTERFACE_HALO_NB-1].io=io;
	aiHalo[INTERFACE_HALO_NB-1].tc=tc;
	aiHalo[INTERFACE_HALO_NB-1].tc2=tc2;
	aiHalo[INTERFACE_HALO_NB-1].m_pos.x=pos.x;
	aiHalo[INTERFACE_HALO_NB-1].m_pos.y=pos.y;
	aiHalo[INTERFACE_HALO_NB-1].ratio.x = ratio.x;
	aiHalo[INTERFACE_HALO_NB-1].ratio.y = ratio.y;
}

void ReleaseHalo() {
	free(aiHalo);
	aiHalo=NULL;
}

void ARX_INTERFACE_HALO_Flush() {
	
	for (long i=0;i<INTERFACE_HALO_NB;i++)
		ARX_INTERFACE_HALO_Render(
		aiHalo[i].io->halo.color,
		aiHalo[i].io->halo.flags,
		aiHalo[i].tc2, aiHalo[i].m_pos, aiHalo[i].ratio);

	INTERFACE_HALO_NB=0;
}

//-----------------------------------------------------------------------------
bool NeedHalo(Entity * io)
{
	if(!io || !(io->ioflags & IO_ITEM))
		return false;

	if(io->halo.flags & HALO_ACTIVE) {
		if(io->inv)
			io->inv->getHalo();

		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// 0 switch 1 forceopen 2 forceclose
static void InventoryOpenClose(unsigned long t) {
	
	if(t == 1 && (player.Interface & INTER_INVENTORY))
		return;

	if(t == 2 && !(player.Interface & INTER_INVENTORY))
		return;

	ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());

	if((player.Interface & INTER_INVENTORY) || (player.Interface & INTER_INVENTORYALL)) {
		bInventoryClosing = true;

		if(WILLRETURNTOFREELOOK) {
			TRUE_PLAYER_MOUSELOOK_ON = true;
			SLID_START=float(arxtime);
			WILLRETURNTOFREELOOK = false;
		}
	} else {
		player.Interface |= INTER_INVENTORY;
		InventoryY = static_cast<long>(INTERFACE_RATIO(100.f));

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
	
	ARX_INTERFACE_BookOpenClose(2);
	
	openNote.setData(type, getLocalised(text));
	openNote.setPage(0);
	
	player.Interface |= INTER_NOTE;
	
	switch(openNote.type()) {
		case gui::Note::Notice:
			ARX_SOUND_PlayInterface(SND_MENU_CLICK, 0.9F + 0.2F * rnd());
			break;
		case gui::Note::Book:
			ARX_SOUND_PlayInterface(SND_BOOK_OPEN, 0.9F + 0.2F * rnd());
			break;
		case gui::Note::SmallNote:
		case gui::Note::BigNote:
			ARX_SOUND_PlayInterface(SND_SCROLL_OPEN, 0.9F + 0.2F * rnd());
			break;
		default: break;
	}
	
	if(TRUE_PLAYER_MOUSELOOK_ON && type == gui::Note::Book) {
		TRUE_PLAYER_MOUSELOOK_ON = false;
	}
	
	if(player.Interface & INTER_INVENTORYALL) {
		bInventoryClosing = true;
		ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
	}
}

void ARX_INTERFACE_NoteClose() {
	
	if(!(player.Interface & INTER_NOTE)) {
		return;
	}
	
	switch(openNote.type()) {
		case gui::Note::Notice: {
			ARX_SOUND_PlayInterface(SND_MENU_CLICK, 0.9F + 0.2F * rnd());
			break;
		}
		case gui::Note::Book:
			ARX_SOUND_PlayInterface(SND_BOOK_CLOSE, 0.9F + 0.2F * rnd());
			break;
		case gui::Note::SmallNote:
		case gui::Note::BigNote:
			ARX_SOUND_PlayInterface(SND_SCROLL_CLOSE, 0.9F + 0.2F * rnd());
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

static void onBookClosePage() {
	
	if(Book_Mode == BOOKMODE_SPELLS) {
		// Closing spell page - clean up any rune flares
		ARX_SPELLS_ClearAllSymbolDraw();
	}
	
}

void ARX_INTERFACE_BookOpenClose(unsigned long t) // 0 switch 1 forceopen 2 forceclose
{
	if(t == 1 && (player.Interface & INTER_MAP))
		return;

	if(t == 2 && !(player.Interface & INTER_MAP))
		return;


	if(player.Interface & INTER_MAP) {
		ARX_SOUND_PlayInterface(SND_BOOK_CLOSE, 0.9F + 0.2F * rnd());
		SendIOScriptEvent(entities.player(),SM_BOOK_CLOSE);
		player.Interface &=~ INTER_MAP;
		g_miniMap.purgeTexContainer();

		if(ARXmenu.mda) {
			for(long i = 0; i < MAX_FLYOVER; i++) {
				ARXmenu.mda->flyover[i].clear();
			}
			free(ARXmenu.mda);
			ARXmenu.mda=NULL;
		}
		
		onBookClosePage();
	} else {
		SendIOScriptEvent(entities.player(),SM_NULL,"","book_open");

		ARX_SOUND_PlayInterface(SND_BOOK_OPEN, 0.9F + 0.2F * rnd());
		SendIOScriptEvent(entities.player(),SM_BOOK_OPEN);
		ARX_INTERFACE_NoteClose();
		player.Interface |= INTER_MAP;
		Book_MapPage=ARX_LEVELS_GetRealNum(CURRENTLEVEL)+1;

		if(Book_MapPage > 8)
			Book_MapPage = 8;

		if(Book_MapPage < 1)
			Book_MapPage = 1;

		if(!ARXmenu.mda) {
//			ARXmenu.mda = (MENU_DYNAMIC_DATA *)malloc(sizeof(MENU_DYNAMIC_DATA));
//			memset(ARXmenu.mda,0,sizeof(MENU_DYNAMIC_DATA));
			ARXmenu.mda = new MENU_DYNAMIC_DATA();
			
			ARXmenu.mda->flyover[BOOK_STRENGTH] = getLocalised("system_charsheet_strength");
			ARXmenu.mda->flyover[BOOK_MIND] = getLocalised("system_charsheet_intel");
			ARXmenu.mda->flyover[BOOK_DEXTERITY] = getLocalised("system_charsheet_dex");
			ARXmenu.mda->flyover[BOOK_CONSTITUTION] = getLocalised("system_charsheet_consti");
			ARXmenu.mda->flyover[BOOK_STEALTH] = getLocalised("system_charsheet_stealth");
			ARXmenu.mda->flyover[BOOK_MECANISM] = getLocalised("system_charsheet_mecanism");
			ARXmenu.mda->flyover[BOOK_INTUITION] = getLocalised("system_charsheet_intuition");
			ARXmenu.mda->flyover[BOOK_ETHERAL_LINK] = getLocalised("system_charsheet_etheral_link");
			ARXmenu.mda->flyover[BOOK_OBJECT_KNOWLEDGE] = getLocalised("system_charsheet_objknoledge");
			ARXmenu.mda->flyover[BOOK_CASTING] = getLocalised("system_charsheet_casting");
			ARXmenu.mda->flyover[BOOK_PROJECTILE] = getLocalised("system_charsheet_projectile");
			ARXmenu.mda->flyover[BOOK_CLOSE_COMBAT] = getLocalised("system_charsheet_closecombat");
			ARXmenu.mda->flyover[BOOK_DEFENSE] = getLocalised("system_charsheet_defense");
			ARXmenu.mda->flyover[BUTTON_QUICK_GENERATION] = getLocalised("system_charsheet_quickgenerate");
			ARXmenu.mda->flyover[BUTTON_DONE] = getLocalised("system_charsheet_done");
			ARXmenu.mda->flyover[BUTTON_SKIN] = getLocalised("system_charsheet_skin");
			ARXmenu.mda->flyover[WND_ATTRIBUTES] = getLocalised("system_charsheet_atributes");
			ARXmenu.mda->flyover[WND_SKILLS] = getLocalised("system_charsheet_skills");
			ARXmenu.mda->flyover[WND_STATUS] = getLocalised("system_charsheet_status");
			ARXmenu.mda->flyover[WND_LEVEL] = getLocalised("system_charsheet_level");
			ARXmenu.mda->flyover[WND_XP] = getLocalised("system_charsheet_xpoints");
			ARXmenu.mda->flyover[WND_HP] = getLocalised("system_charsheet_hp");
			ARXmenu.mda->flyover[WND_MANA] = getLocalised("system_charsheet_mana");
			ARXmenu.mda->flyover[WND_AC] = getLocalised("system_charsheet_ac");
			ARXmenu.mda->flyover[WND_RESIST_MAGIC] = getLocalised("system_charsheet_res_magic");
			ARXmenu.mda->flyover[WND_RESIST_POISON] = getLocalised("system_charsheet_res_poison");
			ARXmenu.mda->flyover[WND_DAMAGE] = getLocalised("system_charsheet_damage");
		}
	}

	if(player.Interface & INTER_COMBATMODE) {
		player.Interface&=~INTER_COMBATMODE;
		ARX_EQUIPMENT_LaunchPlayerUnReadyWeapon();
	}

	if(player.Interface & INTER_INVENTORYALL) {
		ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
		bInventoryClosing = true;
	}

	BOOKZOOM = 0;
	bBookHalo = false;
	ulBookHaloTime = 0;
	pTextManage->Clear();

	TRUE_PLAYER_MOUSELOOK_ON = false;
}

//-------------------------------------------------------------------------------------
// Keyboard/Mouse Management
//-------------------------------------------------------------------------------------

void ResetPlayerInterface() {
	player.Interface |= INTER_LIFE_MANA;
	SLID_VALUE = 0;
	lSLID_VALUE = 0;
	SLID_START=float(arxtime);
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

static void GetInfosCombineWithIO(Entity * _pWithIO) {
	
	if(!COMBINE)
		return;
	
	std::string tcIndent = COMBINE->idString();
	
	char tTxtCombineDest[256];
	
	if(_pWithIO && _pWithIO != COMBINE && _pWithIO->script.data) {
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
								memcpy(tTxtCombineDest, pStartString, pEndString - pStartString);
								tTxtCombineDest[pEndString - pStartString] = 0;
								
								if(tTxtCombineDest == COMBINE->className()) {
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
										memcpy(tTxtCombineDest, pStartString, pEndString - pStartString);
										tTxtCombineDest[pEndString - pStartString] = 0;
										if(COMBINE->groups.find(tTxtCombineDest) != COMBINE->groups.end()) {
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
							memcpy(tTxtCombineDest, pStartString, pEndString - pStartString);
							tTxtCombineDest[pEndString - pStartString] = 0;
							
							if(tTxtCombineDest == COMBINE->className()) {
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
									memcpy(tTxtCombineDest, pStartString, pEndString - pStartString);
									tTxtCombineDest[pEndString - pStartString] = 0;
									
									if(COMBINE->groups.find(tTxtCombineDest) != COMBINE->groups.end()) {
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
		GetInfosCombineWithIO(io);
	}

	if(SecondaryInventory) {
		for(long y = 0; y < SecondaryInventory->m_size.y; y++)
		for(long x = 0; x < SecondaryInventory->m_size.x; x++) {
			Entity * io = SecondaryInventory->slot[x][y].io;
			GetInfosCombineWithIO(io);
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
		ARX_INTERFACE_BookOpenClose(2);

		player.Interface|=INTER_COMBATMODE;

		if(i==2) {
			player.Interface|=INTER_NO_STRIKE;

			if(config.input.mouseLookToggle) {
				TRUE_PLAYER_MOUSELOOK_ON = true;
				SLID_START=float(arxtime);
			}
		}

		ARX_EQUIPMENT_LaunchPlayerReadyWeapon();
		player.doingmagic=0;
	}
}
long CSEND=0;
long MOVE_PRECEDENCE=0;

static bool canOpenBookPage(ARX_INTERFACE_BOOK_MODE page) {
	switch(page) {
		case BOOKMODE_SPELLS:  return !!player.rune_flags;
		default:               return true;
	}
}

static void openBookPage(ARX_INTERFACE_BOOK_MODE newPage, bool toggle = false) {
	
	if((player.Interface & INTER_MAP) && Book_Mode == newPage) {
		
		if(toggle) {
			// Close the book
			ARX_INTERFACE_BookOpenClose(2);
		}
		
		return; // nothing to do
	}
	
	if(!canOpenBookPage(newPage)) {
		return;
	}
	
	if(player.Interface & INTER_MAP) {
		
		onBookClosePage();
		
		// If the book is already open, play the page turn sound
		ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
		
	} else {
		// Otherwise open the book
		ARX_INTERFACE_BookOpenClose(0);
	}
	
	Book_Mode = newPage;
}

static ARX_INTERFACE_BOOK_MODE nextBookPage() {
	ARX_INTERFACE_BOOK_MODE nextPage = Book_Mode, oldPage;
	do {
		oldPage = nextPage;
		
		switch(oldPage) {
			case BOOKMODE_STATS:   nextPage = BOOKMODE_SPELLS;  break;
			case BOOKMODE_SPELLS:  nextPage = BOOKMODE_MINIMAP; break;
			case BOOKMODE_MINIMAP: nextPage = BOOKMODE_QUESTS;  break;
			case BOOKMODE_QUESTS:  nextPage = BOOKMODE_QUESTS;  break;
		}
		
		if(canOpenBookPage(nextPage)) {
			return nextPage;
		}
		
	} while(nextPage != oldPage);
	return Book_Mode;
}

static ARX_INTERFACE_BOOK_MODE prevBookPage() {
	ARX_INTERFACE_BOOK_MODE prevPage = Book_Mode, oldPage;
	do {
		oldPage = prevPage;
		
		switch(oldPage) {
			case BOOKMODE_STATS:   prevPage = BOOKMODE_STATS;   break;
			case BOOKMODE_SPELLS:  prevPage = BOOKMODE_STATS;   break;
			case BOOKMODE_MINIMAP: prevPage = BOOKMODE_SPELLS;  break;
			case BOOKMODE_QUESTS:  prevPage = BOOKMODE_MINIMAP; break;
		}
		
		if(canOpenBookPage(prevPage)) {
			return prevPage;
		}
		
	} while(prevPage != oldPage);
	return Book_Mode;
}

extern unsigned long REQUEST_JUMP;
//-----------------------------------------------------------------------------
void ArxGame::managePlayerControls() {
	
	ARX_PROFILE_FUNC();
	
	if(   (EERIEMouseButton & 4)
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
						EERIEMouseButton&=~4;
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
									ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
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
				EERIEMouseButton&=~4;
				DRAGGING = false;
				EERIEMouseButton = 0;
			}

			EERIEMouseButton&=~4;
			EERIEMouseButton = 0;
		}
	}

	float MoveDiv;
	
	Vec3f tm = Vec3f_ZERO;
	
	// Checks STEALTH Key Status.
	if(GInput->actionPressed(CONTROLS_CUST_STEALTHMODE)) {
		MoveDiv=0.02f;
		player.Current_Movement|=PLAYER_MOVE_STEALTH;
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
				float tr = glm::radians(eyeball.angle.getPitch());
				eyeball.pos.x += -std::sin(tr) * 20.f * FD * 0.033f;
				eyeball.pos.z += +std::cos(tr) * 20.f * FD * 0.033f;
				NOMOREMOVES=1;
			}

			// Checks WALK_BACKWARD Key Status.
			if(GInput->actionPressed(CONTROLS_CUST_WALKBACKWARD)) {
				float tr = glm::radians(eyeball.angle.getPitch());
				eyeball.pos.x +=  std::sin(tr) * 20.f * FD * 0.033f;
				eyeball.pos.z += -std::cos(tr) * 20.f * FD * 0.033f;
				NOMOREMOVES=1;
			}

			// Checks STRAFE_LEFT Key Status.
			if( (GInput->actionPressed(CONTROLS_CUST_STRAFELEFT)||
				(GInput->actionPressed(CONTROLS_CUST_STRAFE)&&GInput->actionPressed(CONTROLS_CUST_TURNLEFT)))
				&& !NOMOREMOVES)
			{
				float tr = glm::radians(MAKEANGLE(eyeball.angle.getPitch()+90.f));
				eyeball.pos.x += -std::sin(tr) * 10.f * FD * 0.033f;
				eyeball.pos.z += +std::cos(tr) * 10.f * FD * 0.033f;
				NOMOREMOVES=1;			
			}

			// Checks STRAFE_RIGHT Key Status.
			if( (GInput->actionPressed(CONTROLS_CUST_STRAFERIGHT)||
				(GInput->actionPressed(CONTROLS_CUST_STRAFE)&&GInput->actionPressed(CONTROLS_CUST_TURNRIGHT)))
				&& !NOMOREMOVES)
			{
				float tr = glm::radians(MAKEANGLE(eyeball.angle.getPitch()-90.f));
				eyeball.pos.x += -std::sin(tr) * 10.f * FD * 0.033f;
				//eyeball.pos.y+=FD*0.33f;
				eyeball.pos.z +=  std::cos(tr) * 10.f * FD * 0.033f;
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
					MagicSightFader+=framedelay*( 1.0f / 200 );

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

		// Checks WALK_BACKWARD Key Status.
		if(GInput->actionPressed(CONTROLS_CUST_WALKBACKWARD) && !NOMOREMOVES) {
			CurrFightPos=3;
			float multi = 1;

			if(left || right) {
				multi = 0.8f;
			}

			float t = glm::radians(player.angle.getPitch());
			multi = 5.f * FD * MoveDiv * multi;
			tm.x += std::sin(t) * multi;
			tm.z -= std::cos(t) * multi;

			if(!USE_PLAYERCOLLISIONS) {
				t = glm::radians(player.angle.getYaw());
				tm.y-=std::sin(t)*multi;
			}

			player.Current_Movement|=PLAYER_MOVE_WALK_BACKWARD;

			if (GInput->actionNowPressed(CONTROLS_CUST_WALKBACKWARD) )
				MOVE_PRECEDENCE=PLAYER_MOVE_WALK_BACKWARD;
		}
		else if(MOVE_PRECEDENCE == PLAYER_MOVE_WALK_BACKWARD)
			MOVE_PRECEDENCE = 0;

		// Checks WALK_FORWARD Key Status.
		if(GInput->actionPressed(CONTROLS_CUST_WALKFORWARD) && !NOMOREMOVES) {
			CurrFightPos=2;
			float multi = 1;

			if(left || right) {
				multi=0.8f;
			}

			float t = glm::radians(player.angle.getPitch());
			multi = 10.f * FD * MoveDiv * multi;
			tm.x -= std::sin(t) * multi;
			tm.z += std::cos(t) * multi;

			if(!USE_PLAYERCOLLISIONS) {
				t = glm::radians(player.angle.getYaw());
				tm.y+=std::sin(t)*multi;
			}

			player.Current_Movement|=PLAYER_MOVE_WALK_FORWARD;

			if(GInput->actionNowPressed(CONTROLS_CUST_WALKFORWARD))
				MOVE_PRECEDENCE=PLAYER_MOVE_WALK_FORWARD;
		}
		else if(MOVE_PRECEDENCE == PLAYER_MOVE_WALK_FORWARD)
			MOVE_PRECEDENCE = 0;

		// Checks STRAFE_LEFT Key Status.
		if(left && !NOMOREMOVES) {
			CurrFightPos=0;
			float t = glm::radians(MAKEANGLE(player.angle.getPitch()+90.f));
			float multi = 6.f * FD * MoveDiv;
			tm.x -= std::sin(t) * multi;
			tm.z += std::cos(t) * multi;

			player.Current_Movement|=PLAYER_MOVE_STRAFE_LEFT;

			if (GInput->actionNowPressed(CONTROLS_CUST_STRAFELEFT) )
				MOVE_PRECEDENCE=PLAYER_MOVE_STRAFE_LEFT;
		}
		else if(MOVE_PRECEDENCE == PLAYER_MOVE_STRAFE_LEFT)
			MOVE_PRECEDENCE = 0;

		// Checks STRAFE_RIGHT Key Status.
		if(right && !NOMOREMOVES) {
			CurrFightPos=1;
			float t = glm::radians(MAKEANGLE(player.angle.getPitch()-90.f));
			float multi = 6.f * FD * MoveDiv;
			tm.x -= std::sin(t) * multi;
			tm.z += std::cos(t) * multi;

			player.Current_Movement|=PLAYER_MOVE_STRAFE_RIGHT;

			if(GInput->actionNowPressed(CONTROLS_CUST_STRAFERIGHT))
				MOVE_PRECEDENCE=PLAYER_MOVE_STRAFE_RIGHT;
		}
		else if(MOVE_PRECEDENCE == PLAYER_MOVE_STRAFE_RIGHT)
			MOVE_PRECEDENCE = 0;

		moveto = player.pos + tm;
	}

	// Checks CROUCH Key Status.
	if(GInput->actionNowPressed(CONTROLS_CUST_CROUCHTOGGLE)) {
		bGCroucheToggle = !bGCroucheToggle;
	}

	if(GInput->actionPressed(CONTROLS_CUST_CROUCH) || bGCroucheToggle) {
		player.Current_Movement |= PLAYER_CROUCH;
	}

	if(GInput->actionNowPressed(CONTROLS_CUST_UNEQUIPWEAPON)) {
		ARX_EQUIPMENT_UnEquipPlayerWeapon();
	}

	// Can only lean outside of combat mode
	if(!(player.Interface & INTER_COMBATMODE)) {
		// Checks LEAN_LEFT Key Status.
		if(GInput->actionPressed(CONTROLS_CUST_LEANLEFT))
			player.Current_Movement |= PLAYER_LEAN_LEFT;

		// Checks LEAN_RIGHT Key Status.
		if(GInput->actionPressed(CONTROLS_CUST_LEANRIGHT))
			player.Current_Movement |= PLAYER_LEAN_RIGHT;
	}
	
	// Checks JUMP Key Status.
	if(player.jumpphase == NotJumping && GInput->actionNowPressed(CONTROLS_CUST_JUMP)) {
		REQUEST_JUMP = (unsigned long)(arxtime);
	}
	
	// MAGIC
	if(GInput->actionPressed(CONTROLS_CUST_MAGICMODE)) {
		if(!(player.Current_Movement & PLAYER_CROUCH) && !BLOCK_PLAYER_CONTROLS && ARXmenu.currentmode == AMCM_OFF) {
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
		} else if(InPlayerInventoryPos(DANAEMouse)) {
				if((player.Interface & INTER_INVENTORY)) {
					if(player.bag) {
						if(sActiveInventory > 0) {
							ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
							sActiveInventory --;
						}
					}
				}
		} else if(player.Interface & INTER_MAP) {
			openBookPage(prevBookPage());
		} else {
				if((player.Interface & INTER_INVENTORY)) {
					if(player.bag) {
						if(sActiveInventory > 0) {
							ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
							sActiveInventory --;
						}
					}
				}
		}
	}

	if(GInput->actionNowPressed(CONTROLS_CUST_NEXT)) {
		if(eMouseState == MOUSE_IN_BOOK) {
			if(player.Interface & INTER_MAP) {
				openBookPage(nextBookPage());
			}
		} else if(InPlayerInventoryPos(DANAEMouse)) {
				if((player.Interface & INTER_INVENTORY)) {
					if(player.bag) {
						if(sActiveInventory < player.bag - 1) {
							ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
							sActiveInventory ++;
						}
					}
				}
		} else if(player.Interface & INTER_MAP) {
			openBookPage(nextBookPage());
		} else {
				if((player.Interface & INTER_INVENTORY)) {
					if(player.bag) {
						if(sActiveInventory < player.bag - 1) {
							ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
							sActiveInventory ++;
						}
					}
				}
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
			
			if(spell && spell->m_caster == PlayerEntityHandle)
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
				SLID_START=float(arxtime);
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
						SLID_START=float(arxtime);
					}
				} else {
					TRUE_PLAYER_MOUSELOOK_ON = false;
				}
			} else {
				if(GInput->actionNowPressed(CONTROLS_CUST_FREELOOK)) {
					if(!TRUE_PLAYER_MOUSELOOK_ON) {
						TRUE_PLAYER_MOUSELOOK_ON = true;
						SLID_START=float(arxtime);
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
		ARX_INTERFACE_BookOpenClose(0);

	// Check For Combat Mode ON/OFF
	if(   (EERIEMouseButton & 1)
	   && !(player.Interface & INTER_COMBATMODE)
	   && !g_cursorOverBook
	   && !SpecialCursor
	   && PLAYER_MOUSELOOK_ON
	   && !DRAGINTER
	   && !InInventoryPos(DANAEMouse)
	   && config.input.autoReadyWeapon
	) {
		if(!(LastMouseClick & 1)) {
			COMBAT_MODE_ON_START_TIME = (unsigned long)(arxtime);
		} else {
			if(float(arxtime) - COMBAT_MODE_ON_START_TIME > 10) {
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
				mecanismIconReset();
				
				if(player.Interface & INTER_INVENTORYALL) {
					ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
					bInventoryClosing = true;
					lOldInterfaceTemp=INTER_INVENTORYALL;
				}

				bRenderInCursorMode=false;
				InventoryOpenClose(2);

				if(player.Interface &INTER_INVENTORY) {
					Entity * io = NULL;

					if(SecondaryInventory) {
						io = SecondaryInventory->io;
					} else if (player.Interface & INTER_STEAL) {
						io = ioSteal;
					}

					if(io) {
						InventoryDir=-1;
						SendIOScriptEvent(io,SM_INVENTORY2_CLOSE);
						TSecondaryInventory=SecondaryInventory;
						SecondaryInventory=NULL;
					}
				}

				if(config.input.mouseLookToggle) {
					TRUE_PLAYER_MOUSELOOK_ON = true;
					SLID_START=float(arxtime);
				}
			}
		}
	} else {
		if(bInverseInventory) {
			if(!bInventoryClosing) {
				bRenderInCursorMode=false;

				InventoryOpenClose(2);

				if(player.Interface &INTER_INVENTORY) {
					Entity * io = NULL;

					if (SecondaryInventory) {
						io = SecondaryInventory->io;
					} else if (player.Interface & INTER_STEAL) {
						io = ioSteal;
					}

					if(io){
						InventoryDir=-1;
						SendIOScriptEvent(io,SM_INVENTORY2_CLOSE);
						TSecondaryInventory=SecondaryInventory;
						SecondaryInventory=NULL;
					}
				}

				if(config.input.mouseLookToggle) {
					TRUE_PLAYER_MOUSELOOK_ON = true;
					SLID_START=float(arxtime);
				}
			}
		} else {
			bRenderInCursorMode=true;

			if(!MAGICMODE) {
				if(lOldInterfaceTemp) {
					lOldInterface=lOldInterfaceTemp;
					lOldInterfaceTemp=0;
					ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
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
				spells.endByCaster(PlayerEntityHandle, SPELL_FLYING_EYE);
			}
		}
	}
}

class PlayerInterfaceFader {
public:
	void reset() {
		SMOOTHSLID=0;
		PLAYER_INTERFACE_HIDE_COUNT = true;
	}
	
	void requestFade(FadeDirection showhide, long smooth) {
		if(showhide == FadeDirection_Out) {
			InventoryOpenClose(2);
			ARX_INTERFACE_BookOpenClose(2);
			ARX_INTERFACE_NoteClose();
		}
		
		if(showhide == FadeDirection_In)
			PLAYER_INTERFACE_HIDE_COUNT = true;
		else
			PLAYER_INTERFACE_HIDE_COUNT = false;
		
		if(smooth) {
			if(showhide == FadeDirection_In)
				SMOOTHSLID = -1;
			else
				SMOOTHSLID = 1;
		} else {
			if(showhide == FadeDirection_In)
				SLID_VALUE = 0.f;
			else
				SLID_VALUE = 100.f;
			
			lSLID_VALUE = SLID_VALUE;
		}
	}
	
	void updateFirst() {
		/////////////////////////////////////////////////////
		// begining to count time for sliding interface
		if(PLAYER_INTERFACE_HIDE_COUNT && !SMOOTHSLID) {
			bool bOk = true;
	
			if(TRUE_PLAYER_MOUSELOOK_ON) {
				if(!(player.Interface & INTER_COMBATMODE) && player.doingmagic != 2 && !InInventoryPos(DANAEMouse)) {
					bOk = false;
	
					float t=float(arxtime);
	
					if(t-SLID_START > 10000.f) {
						SLID_VALUE += (float)Original_framedelay*( 1.0f / 10 );
	
						if(SLID_VALUE > 100.f)
							SLID_VALUE = 100.f;
	
						lSLID_VALUE = SLID_VALUE;
					} else {
						bOk = true;
					}
				}
			}
	
			if(bOk) {
				SLID_VALUE -= (float)Original_framedelay*( 1.0f / 10 );
	
				if(SLID_VALUE < 0.f)
					SLID_VALUE = 0.f;
	
				lSLID_VALUE = SLID_VALUE;
			}
		}
	}
	
	void update() {
		if(SMOOTHSLID == 1) {
			SLID_VALUE += (float)Original_framedelay*( 1.0f / 10 );
			
			if(SLID_VALUE > 100.f) {
				SLID_VALUE = 100.f;
				SMOOTHSLID = 0;
			}
			lSLID_VALUE = SLID_VALUE;
		} else if(SMOOTHSLID == -1) {
			SLID_VALUE -= (float)Original_framedelay*( 1.0f / 10 );
			
			if (SLID_VALUE < 0.f) {
				SLID_VALUE = 0.f;
				SMOOTHSLID = 0;
			}
			lSLID_VALUE = SLID_VALUE;
		}
	}
};
PlayerInterfaceFader playerInterfaceFader;

void playerInterfaceFaderRequestFade(FadeDirection showhide, long smooth) {
	playerInterfaceFader.requestFade(showhide, smooth);
}


void ARX_INTERFACE_Reset()
{
	playerInterfaceFader.reset();
	BLOCK_PLAYER_CONTROLS = false;
	SLID_VALUE=0;
	cinematicBorder.reset2();
	cinematicBorder.reset();
	CINEMA_DECAL=0;
	hideQuickSaveIcon();
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
			for(long i = 0; i < MAX_EQUIPED; i++) {
				if(player.equiped[i] != PlayerEntityHandle
				   && ValidIONum(player.equiped[i])
				   && entities[player.equiped[i]] == pIO
				) {
					FlyingOverIO = pIO;
				}
			}
		}

		if(pIO
		   && (pIO->gameFlags & GFLAG_INTERACTIVITY)
		   && !g_cursorOverBook
		   && eMouseState != MOUSE_IN_NOTE
		) {
			if(!(EERIEMouseButton & 2)
			   && (LastMouseClick & 2)
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
						ANIM_USE * useanim = &io->animlayer[1];
						WeaponType type = ARX_EQUIPMENT_GetPlayerWeaponType();
						
						switch(type) {
						case WEAPON_DAGGER:
							if(useanim->cur_anim == io->anims[ANIM_DAGGER_UNREADY_PART_1])
								bOk = false;
						
							break;
						case WEAPON_1H:
							if(useanim->cur_anim == io->anims[ANIM_1H_UNREADY_PART_1])
								bOk = false;
						
							break;
						case WEAPON_2H:
							if(useanim->cur_anim == io->anims[ANIM_2H_UNREADY_PART_1])
								bOk = false;
						
							break;
						case WEAPON_BOW:
							if(useanim->cur_anim == io->anims[ANIM_MISSILE_UNREADY_PART_1])
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
				if((EERIEMouseButton & 2) && !(LastMouseClick & 2)) {
					STARTED_ACTION_ON_IO = FlyingOverIO;
				}
			}
		}

		if((eMouseState == MOUSE_IN_WORLD) ||
			((eMouseState == MOUSE_IN_BOOK) && !(g_cursorOverBook && (Book_Mode != BOOKMODE_MINIMAP)))
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

	if(PLAYER_MOUSELOOK_ON && !LAST_PLAYER_MOUSELOOK_ON) {
		
		MemoMouse = DANAEMouse;
		EERIEMouseGrab = true;
		
	} else if(!PLAYER_MOUSELOOK_ON && LAST_PLAYER_MOUSELOOK_ON) {
		
		EERIEMouseGrab = false;
		DANAEMouse = MemoMouse;
		
		if(mainApp->getWindow()->isFullScreen()) {
			GInput->setMousePosAbs(DANAEMouse);
		}
		
		bRestoreCoordMouse=false;
	}
	
	LAST_PLAYER_MOUSELOOK_ON=PLAYER_MOUSELOOK_ON;
	PLAYER_ROTATION=0;

	Vec2s mouseDiff = GInput->getMousePosRel();
	
	ARX_Menu_Manage();
	
	if(bRestoreCoordMouse) {
		DANAEMouse=GInput->getMousePosAbs();
	}
	
	// Player/Eyeball Freelook Management
	if(!BLOCK_PLAYER_CONTROLS) {
		GetInventoryObj_INVENTORYUSE(DANAEMouse);

		bool bKeySpecialMove=false;

		static int flPushTimeX[2]={0,0};
		static int flPushTimeY[2]={0,0};


		if(!GInput->actionPressed(CONTROLS_CUST_STRAFE)) {
			float fTime = arxtime.get_updated();
			int	iTime = checked_range_cast<int>(fTime);

			if(GInput->actionPressed(CONTROLS_CUST_TURNLEFT)) {
				if(!flPushTimeX[0])
					flPushTimeX[0] = iTime;

				bKeySpecialMove = true;
			}
			else
				flPushTimeX[0]=0;

			if(GInput->actionPressed(CONTROLS_CUST_TURNRIGHT)) {
				if(!flPushTimeX[1])
					flPushTimeX[1] = iTime;

				bKeySpecialMove = true;
			}
			else
				flPushTimeX[1]=0;
		}

		if(USE_PLAYERCOLLISIONS) {
			float fTime = arxtime.get_updated();
			int iTime = checked_range_cast<int>(fTime);

			if(GInput->actionPressed(CONTROLS_CUST_LOOKUP)) {
				if(!flPushTimeY[0])
					flPushTimeY[0] = iTime;

				bKeySpecialMove = true;
			}
			else
				flPushTimeY[0]=0;

			if(GInput->actionPressed(CONTROLS_CUST_LOOKDOWN)) {
				if(!flPushTimeY[1])
					flPushTimeY[1] = iTime;

				bKeySpecialMove = true;
			}
			else
				flPushTimeY[1]=0;
		}

		if(bKeySpecialMove) {

			int iAction=0;

			if(flPushTimeX[0] || flPushTimeX[1]) {
				if(flPushTimeX[0] < flPushTimeX[1])
					mouseDiff.x = 10;
				else
					mouseDiff.x = -10;

				iAction |= 1;
			}

			if(flPushTimeY[0] || flPushTimeY[1]) {
				if(flPushTimeY[0]<flPushTimeY[1])
					mouseDiff.y = 10;
				else
					mouseDiff.y = -10;

				iAction |= 2;
			}

			if(!(iAction & 1))
				mouseDiff.x = 0;

			if(!(iAction & 2))
				mouseDiff.y = 0;
		} else {
			if(bRenderInCursorMode) {
				Vec2s mousePosRel = GInput->getMousePosRel();
				if(DANAEMouse.x == g_size.width() - 1 && mousePosRel.x > 8) {
					mouseDiff.y = 0;
					mouseDiff.x = mousePosRel.x;
					bKeySpecialMove = true;
				} else if(DANAEMouse.x == 0 && mousePosRel.x < -8) {
					mouseDiff.y = 0;
					mouseDiff.x = mousePosRel.x;
					bKeySpecialMove = true;
				}
				if(DANAEMouse.y == g_size.height() - 1 && mousePosRel.y > 8) {
					mouseDiff.y = mousePosRel.y;
					mouseDiff.x = 0;
					bKeySpecialMove = true;
				} else if(DANAEMouse.y == 0 && mousePosRel.y < -8) {
					mouseDiff.y = mousePosRel.y;
					mouseDiff.x = 0;
					bKeySpecialMove = true;
				}
			}
		}

		if(GInput->actionPressed(CONTROLS_CUST_CENTERVIEW)) {
			eyeball.angle.setYaw(0);
			eyeball.angle.setRoll(0);
			player.desiredangle.setYaw(0);
			player.angle.setYaw(0);
			player.desiredangle.setRoll(0);
			player.angle.setRoll(0);
		}

		float mouseSensitivity = (((float)GInput->getMouseSensitivity()) + 1.f) * 0.1f * ((640.f / (float)g_size.width()));
		if (mouseSensitivity > 200) {
			mouseSensitivity = 200;
		}

		mouseSensitivity *= (float)g_size.width() * ( 1.0f / 640 );
		mouseSensitivity *= (1.0f / 5);
		
		Vec2f rotation = Vec2f(mouseDiff) * mouseSensitivity;
		
		if(config.input.invertMouse)
			rotation.y *= -1.f;
		
		if(PLAYER_MOUSELOOK_ON || bKeySpecialMove) {

			if(eyeball.exist == 2) {
				if(eyeball.angle.getYaw() < 70.f) {
					if(eyeball.angle.getYaw() + rotation.y < 70.f)
						eyeball.angle.setYaw(eyeball.angle.getYaw() + rotation.y);
				} else if(eyeball.angle.getYaw() > 300.f) {
					if(eyeball.angle.getYaw() + rotation.y > 300.f)
						eyeball.angle.setYaw(eyeball.angle.getYaw() + rotation.y);
				}

				eyeball.angle.setYaw(MAKEANGLE(eyeball.angle.getYaw()));
				eyeball.angle.setPitch(MAKEANGLE(eyeball.angle.getPitch() - rotation.x));
			} else if(ARXmenu.currentmode != AMCM_NEWQUEST) {

				float iangle = player.angle.getYaw();

				player.desiredangle.setYaw(player.angle.getYaw());
				player.desiredangle.setYaw(player.desiredangle.getYaw() + rotation.y);
				player.desiredangle.setYaw(MAKEANGLE(player.desiredangle.getYaw()));

				if(player.desiredangle.getYaw() >= 74.9f && player.desiredangle.getYaw() <= 301.f) {
					if(iangle < 75.f)
						player.desiredangle.setYaw(74.9f); //69
					else
						player.desiredangle.setYaw(301.f);
				}

				if(glm::abs(rotation.y) > 2.f)
					entities.player()->animBlend.lastanimtime = 0;

				if(rotation.x != 0.f)
					player.Current_Movement|=PLAYER_ROTATE;

				PLAYER_ROTATION = rotation.x;

				player.desiredangle.setPitch(player.angle.getPitch());
				player.desiredangle.setPitch(MAKEANGLE(player.desiredangle.getPitch() - rotation.x));
			}
		}
	}

	if((!BLOCK_PLAYER_CONTROLS) && !(player.Interface & INTER_COMBATMODE)) {
		if(!DRAGINTER) {
			if(config.input.autoDescription || ((LastMouseClick & 1) && !(EERIEMouseButton & 1) && !(EERIEMouseButton & 4) && !(LastMouseClick & 4)))
			{
				Entity * temp;
				temp = FlyingOverIO;

				if(temp && !temp->locname.empty()) {

					if (((FlyingOverIO->ioflags & IO_ITEM) && FlyingOverIO->_itemdata->equipitem)
						&& (player.m_skillFull.objectKnowledge + player.m_attributeFull.mind
						>= FlyingOverIO->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_Identify_Value].value) )
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
						ss << " (" << Text << " " << (int)temp->poisonous << ")";
						WILLADDSPEECH += ss.str();
					}

					if((temp->ioflags & IO_ITEM) && temp->durability < 100.f) {
						std::string Text = getLocalised("description_durability", "error");
						std::stringstream ss;
						ss << " " << Text << " " << std::fixed << std::setw(3) << std::setprecision(0) << temp->durability << "/" << temp->max_durability;
						WILLADDSPEECH += ss.str();
					}

					WILLADDSPEECHTIME = (unsigned long)(arxtime);//treat warning C4244 conversion from 'float' to 'unsigned long'

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
							pTextManage->AddText(hFontInBook, WILLADDSPEECH, rDraw, Color(232, 204, 143), 2000 + WILLADDSPEECH.length()*60);
						} else {
							pTextManage->AddText(hFontInBook, WILLADDSPEECH, rDraw, Color(232, 204, 143));
						}
					}

					WILLADDSPEECH.clear();
				}
			}
		}
	}

	if ((EERIEMouseButton & 4) || (LastMouseClick & 4))
		WILLADDSPEECH.clear();

	if (!WILLADDSPEECH.empty())
		if (WILLADDSPEECHTIME+300<arxtime.get_frame_time())
		{
			ARX_SPEECH_Add(WILLADDSPEECH);
			WILLADDSPEECH.clear();
		}
}

static void ARX_INTERFACE_RELEASESOUND() {
	ARX_SOUND_PlayInterface(SND_MENU_RELEASE);
}

static void ARX_INTERFACE_ERRORSOUND() {
	ARX_SOUND_PlayInterface(SND_MENU_CLICK);
}

static bool CheckAttributeClick(float x, float y, float * val, TextureContainer * tc) {
	
	bool rval=false;
	float t = *val;

	if(MouseInBookRect(x, y, x + 32, y + 32)) {
		rval = true;

		if(((BOOKBUTTON & 1) || (BOOKBUTTON & 2)) && tc)
			DrawBookInterfaceItem(tc, Vec2f(x, y));

		if(!(BOOKBUTTON & 1) && (LASTBOOKBUTTON & 1)) {
			if(player.Attribute_Redistribute > 0) {
				player.Attribute_Redistribute--;
				t++;
				*val=t;
				ARX_INTERFACE_RELEASESOUND();
			}
			else
				ARX_INTERFACE_ERRORSOUND();
		}

		if(!(BOOKBUTTON & 2) && (LASTBOOKBUTTON & 2)) {
			if(ARXmenu.currentmode == AMCM_NEWQUEST) {
				if(t > 6 && player.level == 0) {
					player.Attribute_Redistribute++;
					t --;
					*val=t;
					ARX_INTERFACE_RELEASESOUND();
				}
				else
					ARX_INTERFACE_ERRORSOUND();
			}
			else
				ARX_INTERFACE_ERRORSOUND();
		}
	}

	return rval;
}

static bool CheckSkillClick(float x, float y, float * val, TextureContainer * tc,
                            float * oldval) {
	
	bool rval=false;

	float t = *val;
	float ot = *oldval;

	if(MouseInBookRect( x, y, x + 32, y + 32)) {
		rval=true;

		if(((BOOKBUTTON & 1) || (BOOKBUTTON & 2)) && tc)
			DrawBookInterfaceItem(tc, Vec2f(x, y));

		if(!(BOOKBUTTON & 1) && (LASTBOOKBUTTON & 1)) {
			if(player.Skill_Redistribute > 0) {
				player.Skill_Redistribute--;
				t++;
				*val=t;
				ARX_INTERFACE_RELEASESOUND();
			}
			else
				ARX_INTERFACE_ERRORSOUND();
		}

		if(!(BOOKBUTTON & 2) && (LASTBOOKBUTTON & 2)) {
			if(ARXmenu.currentmode == AMCM_NEWQUEST) {
				if(t > ot && player.level == 0) {
					player.Skill_Redistribute++;
					t --;
					*val=t;
					ARX_INTERFACE_RELEASESOUND();
				}
				else
					ARX_INTERFACE_ERRORSOUND();
			}
			else
				ARX_INTERFACE_ERRORSOUND();
		}
	}

	return rval;
}

void ARX_INTERFACE_ManageOpenedBook_Finish()
{
	GRenderer->SetRenderState(Renderer::DepthWrite, true);

	if((player.Interface & INTER_MAP) && !(player.Interface & INTER_COMBATMODE)) {
		if(Book_Mode == BOOKMODE_SPELLS) {

			Vec3f pos = Vec3f(0.f, 0.f, 2100.f);
			Anglef angle = Anglef::ZERO;

			EERIE_LIGHT * light = lightHandleGet(torchLightHandle);
			
			EERIE_LIGHT tl = *light;

			light->pos = Vec3f(500.f, -1960.f, 1590.f);
			light->exist = 1;
			light->rgb = Color3f(0.6f, 0.7f, 0.9f);
			light->intensity  = 1.8f;
			light->fallstart=4520.f;
			light->fallend = light->fallstart + 600.f;
			RecalcLight(light);
			
			EERIE_CAMERA * oldcam = ACTIVECAM;
			
			PDL[0] = light;
			TOTPDL=1;

			long found2=0;
			float n;
			long xpos=0;
			long ypos=0;

			for(size_t i = 0; i < RUNE_COUNT; i++) {
				if(gui::necklace.runes[i]) {
					bookcam.center.x = (382 + xpos * 45 + BOOKDEC.x) * g_sizeRatio.x;
					bookcam.center.y = (100 + ypos * 64 + BOOKDEC.y) * g_sizeRatio.y;

					SetActiveCamera(&bookcam);
					PrepareCamera(&bookcam, g_size);

					// First draw the lace
					angle.setPitch(0.f);

					if(player.rune_flags & (RuneFlag)(1<<i)) {
						
						TransformInfo t1(pos, glm::toQuat(toRotationMatrix(angle)));
						DrawEERIEInter(gui::necklace.lacet, t1, NULL);

						if(gui::necklace.runes[i]->angle.getPitch() != 0.f) {
							if(gui::necklace.runes[i]->angle.getPitch() > 300.f)
								gui::necklace.runes[i]->angle.setPitch(300.f);

							angle.setPitch(std::sin(arxtime.get_updated() * (1.0f / 200)) * gui::necklace.runes[i]->angle.getPitch() * (1.0f / 40));
						}

						gui::necklace.runes[i]->angle.setPitch(gui::necklace.runes[i]->angle.getPitch() - framedelay * 0.2f);

						if(gui::necklace.runes[i]->angle.getPitch() < 0.f)
							gui::necklace.runes[i]->angle.setPitch(0.f);

						GRenderer->SetRenderState(Renderer::DepthWrite, true);
						GRenderer->SetRenderState(Renderer::AlphaBlending, false);
						
						// Now draw the rune
						TransformInfo t2(pos, glm::toQuat(toRotationMatrix(angle)));
						DrawEERIEInter(gui::necklace.runes[i], t2, NULL);

						EERIE_2D_BBOX runeBox;
						UpdateBbox2d(*gui::necklace.runes[i], runeBox);

						PopAllTriangleList();

						xpos++;

						if(xpos > 4) {
							xpos = 0;
							ypos++;
						}
						
						const Rect runeMouseTestRect(
						runeBox.min.x,
						runeBox.min.y,
						runeBox.max.x,
						runeBox.max.y
						);

						// Checks for Mouse floating over a rune...
						if(!found2 && runeMouseTestRect.contains(Vec2i(DANAEMouse))) {
							long r=0;

							for(size_t j = 0; j < gui::necklace.runes[i]->facelist.size(); j++) {
								n=PtIn2DPolyProj(gui::necklace.runes[i], &gui::necklace.runes[i]->facelist[j], (float)DANAEMouse.x, (float)DANAEMouse.y);

								if(n!=0.f) {
									r=1;
									break;
								}
							}

							if(r) {
								GRenderer->SetRenderState(Renderer::AlphaBlending, true);
								GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
								
								TransformInfo t(pos, glm::toQuat(toRotationMatrix(angle)));
								DrawEERIEInter(gui::necklace.runes[i], t, NULL);

								gui::necklace.runes[i]->angle.setPitch(gui::necklace.runes[i]->angle.getPitch() + framedelay*2.f);

								PopAllTriangleList();
								
								GRenderer->SetRenderState(Renderer::AlphaBlending, false);

								SpecialCursor=CURSOR_INTERACTION_ON;

								if((EERIEMouseButton & 1) && !(LastMouseClick & 1))
									if((size_t)LastRune != i) {
										switch(i)
										{
										case RUNE_AAM:
											ARX_SPELLS_RequestSymbolDraw(entities.player(), "aam", ARX_SOUND_GetDuration(SND_SYMB_AAM));
											ARX_SOUND_PlayInterface(SND_SYMB_AAM);
											break;
										case RUNE_CETRIUS:
											ARX_SPELLS_RequestSymbolDraw(entities.player(), "cetrius", ARX_SOUND_GetDuration(SND_SYMB_CETRIUS));
											ARX_SOUND_PlayInterface(SND_SYMB_CETRIUS);
											break;
										case RUNE_COMUNICATUM:
											ARX_SPELLS_RequestSymbolDraw(entities.player(), "comunicatum", ARX_SOUND_GetDuration(SND_SYMB_COMUNICATUM));
											ARX_SOUND_PlayInterface(SND_SYMB_COMUNICATUM);
											break;
										case RUNE_COSUM:
											ARX_SPELLS_RequestSymbolDraw(entities.player(), "cosum", ARX_SOUND_GetDuration(SND_SYMB_COSUM));
											ARX_SOUND_PlayInterface(SND_SYMB_COSUM);
											break;
										case RUNE_FOLGORA:
											ARX_SPELLS_RequestSymbolDraw(entities.player(), "folgora", ARX_SOUND_GetDuration(SND_SYMB_FOLGORA));
											ARX_SOUND_PlayInterface(SND_SYMB_FOLGORA);
											break;
										case RUNE_FRIDD:
											ARX_SPELLS_RequestSymbolDraw(entities.player(), "fridd", ARX_SOUND_GetDuration(SND_SYMB_FRIDD));
											ARX_SOUND_PlayInterface(SND_SYMB_FRIDD);
											break;
										case RUNE_KAOM:
											ARX_SPELLS_RequestSymbolDraw(entities.player(), "kaom", ARX_SOUND_GetDuration(SND_SYMB_KAOM));
											ARX_SOUND_PlayInterface(SND_SYMB_KAOM);
											break;
										case RUNE_MEGA:
											ARX_SPELLS_RequestSymbolDraw(entities.player(), "mega", ARX_SOUND_GetDuration(SND_SYMB_MEGA));
											ARX_SOUND_PlayInterface(SND_SYMB_MEGA);
											break;
										case RUNE_MORTE:
											ARX_SPELLS_RequestSymbolDraw(entities.player(), "morte", ARX_SOUND_GetDuration(SND_SYMB_MORTE));
											ARX_SOUND_PlayInterface(SND_SYMB_MORTE);
											break;
										case RUNE_MOVIS:
											ARX_SPELLS_RequestSymbolDraw(entities.player(), "movis", ARX_SOUND_GetDuration(SND_SYMB_MOVIS));
											ARX_SOUND_PlayInterface(SND_SYMB_MOVIS);
											break;
										case RUNE_NHI:
											ARX_SPELLS_RequestSymbolDraw(entities.player(), "nhi", ARX_SOUND_GetDuration(SND_SYMB_NHI));
											ARX_SOUND_PlayInterface(SND_SYMB_NHI);
											break;
										case RUNE_RHAA:
											ARX_SPELLS_RequestSymbolDraw(entities.player(), "rhaa", ARX_SOUND_GetDuration(SND_SYMB_RHAA));
											ARX_SOUND_PlayInterface(SND_SYMB_RHAA);
											break;
										case RUNE_SPACIUM:
											ARX_SPELLS_RequestSymbolDraw(entities.player(), "spacium", ARX_SOUND_GetDuration(SND_SYMB_SPACIUM));
											ARX_SOUND_PlayInterface(SND_SYMB_SPACIUM);
											break;
										case RUNE_STREGUM:
											ARX_SPELLS_RequestSymbolDraw(entities.player(), "stregum", ARX_SOUND_GetDuration(SND_SYMB_STREGUM));
											ARX_SOUND_PlayInterface(SND_SYMB_STREGUM);
											break;
										case RUNE_TAAR:
											ARX_SPELLS_RequestSymbolDraw(entities.player(), "taar", ARX_SOUND_GetDuration(SND_SYMB_TAAR));
											ARX_SOUND_PlayInterface(SND_SYMB_TAAR);
											break;
										case RUNE_TEMPUS:
											ARX_SPELLS_RequestSymbolDraw(entities.player(), "tempus", ARX_SOUND_GetDuration(SND_SYMB_TEMPUS));
											ARX_SOUND_PlayInterface(SND_SYMB_TEMPUS);
											break;
										case RUNE_TERA:
											ARX_SPELLS_RequestSymbolDraw(entities.player(), "tera", ARX_SOUND_GetDuration(SND_SYMB_TERA));
											ARX_SOUND_PlayInterface(SND_SYMB_TERA);
											break;
										case RUNE_VISTA:
											ARX_SPELLS_RequestSymbolDraw(entities.player(), "vista", ARX_SOUND_GetDuration(SND_SYMB_VISTA));
											ARX_SOUND_PlayInterface(SND_SYMB_VISTA);
											break;
										case RUNE_VITAE:
											ARX_SPELLS_RequestSymbolDraw(entities.player(), "vitae", ARX_SOUND_GetDuration(SND_SYMB_VITAE));
											ARX_SOUND_PlayInterface(SND_SYMB_VITAE);
											break;
										case RUNE_YOK:
											ARX_SPELLS_RequestSymbolDraw(entities.player(), "yok", ARX_SOUND_GetDuration(SND_SYMB_YOK));
											ARX_SOUND_PlayInterface(SND_SYMB_YOK);
											break;
										}
									}

									LastRune=i;
							}
						}
					}
				}
			}

			GRenderer->SetCulling(Renderer::CullCCW);

			if(!found2)
				LastRune=-1;

			// Now Draws Spells for this level...
			ARX_PLAYER_ComputePlayerFullStats();

			float posx=0;
			float posy=0;
			float fPosX = 0;
			float fPosY = 0;
			bool	bFlyingOver = false;

			for(size_t i=0; i < SPELL_TYPES_COUNT; i++) {
				if(spellicons[i].level==Book_SpellPage && !spellicons[i].bSecret) {
					// check if player can cast it
					bool bOk = true;
					long j = 0;

					while(j < 4 && (spellicons[i].symbols[j] != RUNE_NONE)) {
						if(!(player.rune_flags & (RuneFlag)(1<<spellicons[i].symbols[j]))) {
							bOk = false;
						}

						j++;
					}

					if(bOk) {
						fPosX = 170.f + posx * 85.f;
						fPosY = 135.f + posy * 70.f;
						long flyingover = 0;

						if(MouseInBookRect(fPosX, fPosY, fPosX + 48, fPosY + 48)) {
							bFlyingOver = true;
							flyingover = 1;

							SpecialCursor=CURSOR_INTERACTION_ON;
							DrawBookTextCenter(hFontInBook, Vec2f(208, 90), spellicons[i].name, Color::none);

							for(size_t si = 0; si < MAX_SPEECH; si++) {
								if(speech[si].timecreation > 0)
									FLYING_OVER=0;
							}

							OLD_FLYING_OVER = FLYING_OVER;
							pTextManage->Clear();
							UNICODE_ARXDrawTextCenteredScroll(hFontInGame,
								static_cast<float>(g_size.center().x),
								12,
								(g_size.center().x)*0.82f,
								spellicons[i].description,
								Color(232,204,143),
								1000,
								0.01f,
								2,
								0);

							long count = 0;
							
							for(long j = 0; j < 6; ++j)
								if(spellicons[i].symbols[j] != RUNE_NONE)
									++count;

							GRenderer->GetTextureStage(0)->setMagFilter(TextureStage::FilterLinear);
							for(int j = 0; j < 6; ++j) {
								if(spellicons[i].symbols[j] != RUNE_NONE) {
									pos.x = 240 - (count * 32) * 0.5f + j * 32;
									pos.y = 306;
									DrawBookInterfaceItem(gui::necklace.pTexTab[spellicons[i].symbols[j]], Vec2f(pos));
								}
							}
							GRenderer->GetTextureStage(0)->setMagFilter(TextureStage::FilterNearest);
						}

						if(spellicons[i].tc) {
							GRenderer->SetRenderState(Renderer::AlphaBlending, true);
							GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);
							
							Color color;
							if(flyingover) {
								color = Color::white;

								if((EERIEMouseButton & 1)  && !(LastMouseClick & 1)) {
									player.SpellToMemorize.bSpell = true;

									for(long j = 0; j < 6; j++) {
										player.SpellToMemorize.iSpellSymbols[j] = spellicons[i].symbols[j];
									}

									player.SpellToMemorize.lTimeCreation = (unsigned long)(arxtime);
								}
							} else {
								color = Color(168, 208, 223, 255);
							}
							
							GRenderer->GetTextureStage(0)->setMagFilter(TextureStage::FilterLinear);
							DrawBookInterfaceItem(spellicons[i].tc, Vec2f(fPosX, fPosY), color);
							GRenderer->GetTextureStage(0)->setMagFilter(TextureStage::FilterNearest);

							GRenderer->SetRenderState(Renderer::AlphaBlending, false);
						}

						posx ++;

						if(posx >= 2) {
							posx = 0;
							posy ++;
						}
					}
				}
			}

			if(!bFlyingOver) {
				OLD_FLYING_OVER = -1;
				FLYING_OVER = -1;
			}

			*light = tl;
			
			SetActiveCamera(oldcam);
			PrepareCamera(oldcam, g_size);
		}
	}
}

void ARX_INTERFACE_ManageOpenedBook() {
	arx_assert(entities.player());
	
	GRenderer->SetRenderState(Renderer::Fog, false);
	
	BOOKDEC.x = 0;
	BOOKDEC.y = 0;
	
	GRenderer->GetTextureStage(0)->setMinFilter(TextureStage::FilterLinear);
	GRenderer->GetTextureStage(0)->setMagFilter(TextureStage::FilterLinear);
	
	if(ARXmenu.currentmode != AMCM_NEWQUEST) {
		switch(Book_Mode) {
			case BOOKMODE_STATS: {
				DrawBookInterfaceItem(ITC.playerbook, Vec2f(97, 64), Color::white, 0.9999f);
				break;
			}
			case BOOKMODE_SPELLS: {
				DrawBookInterfaceItem(ITC.ptexspellbook, Vec2f(97, 64), Color::white, 0.9999f);
				break;
			}
			case BOOKMODE_MINIMAP: {
				DrawBookInterfaceItem(ITC.questbook, Vec2f(97, 64), Color::white, 0.9999f);
				break;
			}
			case BOOKMODE_QUESTS: {
				gui::manageQuestBook();
				break;
			}
		}
	} else {
		arx_assert(ITC.playerbook);
		float x = (640 - ITC.playerbook->m_dwWidth) / 2.f;
		float y = (480 - ITC.playerbook->m_dwHeight) / 2.f;
		
		DrawBookInterfaceItem(ITC.playerbook, Vec2f(x, y));

		BOOKDEC.x = x - 97;
		// TODO copy paste error ?
		BOOKDEC.y = x - 64 + 19;
	}
	
	if(ARXmenu.currentmode != AMCM_NEWQUEST) {
		bool bOnglet[11];
		long max_onglet = 0;
		long Book_Page = 1;

		// Checks Clicks in bookmarks
		
		// Character Sheet
		if(Book_Mode != BOOKMODE_STATS) {
			Vec2f pos = Vec2f(BOOKMARKS_POS_X, BOOKMARKS_POS_Y);
			
			TextureContainer* tcBookmarkChar = ITC.bookmark_char;
			DrawBookInterfaceItem(tcBookmarkChar, pos);

			// Check for cursor on charcter sheet bookmark
			if(MouseInBookRect(pos.x, pos.y, pos.x + tcBookmarkChar->m_dwWidth, pos.y + tcBookmarkChar->m_dwHeight)) {
				// Draw highlighted Character sheet icon
				GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
				GRenderer->SetRenderState(Renderer::AlphaBlending, true);
				DrawBookInterfaceItem(tcBookmarkChar, pos, Color::grayb(0x55));
				GRenderer->SetRenderState(Renderer::AlphaBlending, false);

				// Set cursor to interacting
				SpecialCursor=CURSOR_INTERACTION_ON;

				// Check for click
				if(bookclick) {
					ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
					openBookPage(BOOKMODE_STATS);
					pTextManage->Clear();
				}
			}
		}

		if(Book_Mode != BOOKMODE_SPELLS) {
			if(player.rune_flags) {
				Vec2f pos = Vec2f(BOOKMARKS_POS_X + 32, BOOKMARKS_POS_Y);
				
				DrawBookInterfaceItem(ITC.bookmark_magic, pos);

				if(NewSpell == 1) {
					NewSpell = 2;
					for(long nk = 0; nk < 2; nk++) {
						MagFX(Vec3f(BOOKDEC.x + 220.f, BOOKDEC.y + 49.f, 0.000001f));
					}
				}
				
				if(MouseInBookRect(pos.x, pos.y, pos.x + ITC.bookmark_magic->m_dwWidth, pos.y + ITC.bookmark_magic->m_dwHeight)) {
					// Draw highlighted Magic sheet icon
					GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
					GRenderer->SetRenderState(Renderer::AlphaBlending, true);
					DrawBookInterfaceItem(ITC.bookmark_magic, pos, Color::grayb(0x55));
					GRenderer->SetRenderState(Renderer::AlphaBlending, false);

					// Set cursor to interacting
					SpecialCursor=CURSOR_INTERACTION_ON;

					// Check for click
					if(bookclick) {
						ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
						openBookPage(BOOKMODE_SPELLS);
						pTextManage->Clear();
					}
				}
			}
		}

		if(Book_Mode != BOOKMODE_MINIMAP) {
			Vec2f pos = Vec2f(BOOKMARKS_POS_X + 64, BOOKMARKS_POS_Y);
			
			DrawBookInterfaceItem(ITC.bookmark_map, pos);

			if(MouseInBookRect(pos.x, pos.y, pos.x + ITC.bookmark_map->m_dwWidth, pos.y + ITC.bookmark_map->m_dwHeight)) {
				GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
				GRenderer->SetRenderState(Renderer::AlphaBlending, true);
				DrawBookInterfaceItem(ITC.bookmark_map, pos, Color::grayb(0x55));
				GRenderer->SetRenderState(Renderer::AlphaBlending, false);

				// Set cursor to interacting
				SpecialCursor=CURSOR_INTERACTION_ON;

				// Check for click
				if(bookclick) {
					ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
					openBookPage(BOOKMODE_MINIMAP);
					pTextManage->Clear();
				}
			}
		}

		if(Book_Mode != BOOKMODE_QUESTS) {
			Vec2f pos = Vec2f(BOOKMARKS_POS_X + 96, BOOKMARKS_POS_Y);
			
			DrawBookInterfaceItem(ITC.bookmark_quest, pos);

			if(MouseInBookRect(pos.x, pos.y, pos.x + ITC.bookmark_quest->m_dwWidth, pos.y + ITC.bookmark_quest->m_dwHeight)) {
				GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
				GRenderer->SetRenderState(Renderer::AlphaBlending, true);
				DrawBookInterfaceItem(ITC.bookmark_quest, pos, Color::grayb(0x55));
				GRenderer->SetRenderState(Renderer::AlphaBlending, false);

				// Set cursor to interacting
				SpecialCursor=CURSOR_INTERACTION_ON;

				// Check for click
				if(bookclick) {
					ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
					openBookPage(BOOKMODE_QUESTS);
					pTextManage->Clear();
				}
			}
		}
		
		if(Book_Mode == BOOKMODE_MINIMAP)
			max_onglet=8;
		else
			max_onglet=10;

		if(Book_Mode == BOOKMODE_SPELLS)
			Book_Page = Book_SpellPage;
		else
			Book_Page = Book_MapPage;

		std::fill_n(bOnglet, 11, false);

		// calcul de la page de spells
		if(Book_Mode == BOOKMODE_SPELLS) {
			for(size_t i = 0; i < SPELL_TYPES_COUNT; ++i) {
				if(spellicons[i].bSecret == false) {
					bool bOk = true;

					for(long j = 0; j < 4 && spellicons[i].symbols[j] != RUNE_NONE; ++j) {
						if(!(player.rune_flags & (RuneFlag)(1<<spellicons[i].symbols[j])))
							bOk = false;
					}

					if(bOk)
						bOnglet[spellicons[i].level] = true;
				}
			}
		} else {
			memset(bOnglet, true, (max_onglet + 1) * sizeof(*bOnglet));
		}
		
		if(Book_Mode == BOOKMODE_SPELLS || Book_Mode == BOOKMODE_MINIMAP) {
			if(bOnglet[1]) {
				if(Book_Page!=1) {
					Vec2f pos = Vec2f(100.f, 82.f);
					
					DrawBookInterfaceItem(ITC.accessible_1, pos);

					if(MouseInBookRect(pos.x, pos.y, pos.x + 32, pos.y + 32)) {
						GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
						GRenderer->SetRenderState(Renderer::AlphaBlending, true);
						DrawBookInterfaceItem(ITC.accessible_1, pos, Color::grayb(0x55));
						GRenderer->SetRenderState(Renderer::AlphaBlending, false);
						SpecialCursor=CURSOR_INTERACTION_ON;
						if(bookclick) {
							Book_Page=1;
							ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
						}
					}
				}
				else DrawBookInterfaceItem(ITC.current_1, Vec2f(102.f, 82.f));
			}

			if(bOnglet[2]) {
				if(Book_Page!=2) {
					Vec2f pos = Vec2f(98.f, 112.f);
					
					DrawBookInterfaceItem(ITC.accessible_2, pos);

					if(MouseInBookRect(pos.x, pos.y, pos.x + 32, pos.y + 32)) {
						GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
						GRenderer->SetRenderState(Renderer::AlphaBlending, true);
						DrawBookInterfaceItem(ITC.accessible_2, pos, Color::grayb(0x55));
						GRenderer->SetRenderState(Renderer::AlphaBlending, false);
						SpecialCursor=CURSOR_INTERACTION_ON;
						if(bookclick) {
							Book_Page=2;
							ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
						}
					}
				}
				else DrawBookInterfaceItem(ITC.current_2, Vec2f(100.f, 114.f));
			}

			if(bOnglet[3]) {
				if(Book_Page!=3) {
					Vec2f pos = Vec2f(97.f, 143.f);
					
					DrawBookInterfaceItem(ITC.accessible_3, pos);

					if(MouseInBookRect(pos.x, pos.y, pos.x + 32, pos.y + 32)) {
						GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
						GRenderer->SetRenderState(Renderer::AlphaBlending, true);
						DrawBookInterfaceItem(ITC.accessible_3, pos, Color::grayb(0x55));
						GRenderer->SetRenderState(Renderer::AlphaBlending, false);
						SpecialCursor=CURSOR_INTERACTION_ON;
						if(bookclick) {
							Book_Page=3;
							ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
						}
					}
				}
				else DrawBookInterfaceItem(ITC.current_3, Vec2f(101.f, 141.f));
			}

			if(bOnglet[4]) {
				if(Book_Page!=4) {
					Vec2f pos = Vec2f(95.f, 170.f);
					
					DrawBookInterfaceItem(ITC.accessible_4, pos);

					if(MouseInBookRect(pos.x, pos.y, pos.x + 32, pos.y + 32)) {
						GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
						GRenderer->SetRenderState(Renderer::AlphaBlending, true);
						DrawBookInterfaceItem(ITC.accessible_4, pos, Color::grayb(0x55));
						GRenderer->SetRenderState(Renderer::AlphaBlending, false);
						SpecialCursor=CURSOR_INTERACTION_ON;
						if(bookclick) {
							Book_Page=4;
							ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
						}
					}
				}
				else DrawBookInterfaceItem(ITC.current_4, Vec2f(100.f, 170.f));
			}

			if(bOnglet[5]) {
				if(Book_Page!=5) {
					Vec2f pos = Vec2f(95.f, 200.f);
					
					DrawBookInterfaceItem(ITC.accessible_5, pos);

					if(MouseInBookRect(pos.x, pos.y, pos.x + 32, pos.y + 32)) {
						GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
						GRenderer->SetRenderState(Renderer::AlphaBlending, true);
						DrawBookInterfaceItem(ITC.accessible_5, pos, Color::grayb(0x55));
						GRenderer->SetRenderState(Renderer::AlphaBlending, false);
						SpecialCursor=CURSOR_INTERACTION_ON;
						if(bookclick) {
							Book_Page=5;
							ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
						}
					}
				}
				else DrawBookInterfaceItem(ITC.current_5, Vec2f(97.f, 199.f));
			}

			if(bOnglet[6]) {
				if(Book_Page!=6) {
					Vec2f pos = Vec2f(94.f, 229.f);
					
					DrawBookInterfaceItem(ITC.accessible_6, pos);

					if(MouseInBookRect(pos.x, pos.y, pos.x + 32, pos.y + 32)) {
						GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
						GRenderer->SetRenderState(Renderer::AlphaBlending, true);
						DrawBookInterfaceItem(ITC.accessible_6, pos, Color::grayb(0x55));
						GRenderer->SetRenderState(Renderer::AlphaBlending, false);
						SpecialCursor=CURSOR_INTERACTION_ON;
						if(bookclick) {
							Book_Page=6;
							ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
						}
					}
				}
				else DrawBookInterfaceItem(ITC.current_6, Vec2f(103.f, 226.f));
			}

			if(bOnglet[7]) {
				if(Book_Page!=7) {
					Vec2f pos = Vec2f(94.f, 259.f);
					
					DrawBookInterfaceItem(ITC.accessible_7, pos);

					if(MouseInBookRect(pos.x, pos.y, pos.x + 32, pos.y + 32)) {
						GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
						GRenderer->SetRenderState(Renderer::AlphaBlending, true);
						DrawBookInterfaceItem(ITC.accessible_7, pos, Color::grayb(0x55));
						GRenderer->SetRenderState(Renderer::AlphaBlending, false);
						SpecialCursor=CURSOR_INTERACTION_ON;
						if(bookclick) {
							Book_Page=7;
							ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
						}
					}
				}
				else DrawBookInterfaceItem(ITC.current_7, Vec2f(101.f, 255.f));
			}

			if(bOnglet[8]) {
				if(Book_Page!=8) {
					Vec2f pos = Vec2f(92.f, 282.f);
					
					DrawBookInterfaceItem(ITC.accessible_8, pos);

					if(MouseInBookRect(pos.x, pos.y, pos.x + 32, pos.y + 32)) {
						GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
						GRenderer->SetRenderState(Renderer::AlphaBlending, true);
						DrawBookInterfaceItem(ITC.accessible_8, pos, Color::grayb(0x55));
						GRenderer->SetRenderState(Renderer::AlphaBlending, false);
						SpecialCursor=CURSOR_INTERACTION_ON;
						if(bookclick) {
							Book_Page=8;
							ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
						}
					}
				}
				else DrawBookInterfaceItem(ITC.current_8, Vec2f(99.f, 283.f));
			}

			if(bOnglet[9]) {
				if(Book_Page!=9) {
					Vec2f pos = Vec2f(90.f, 308.f);
					
					DrawBookInterfaceItem(ITC.accessible_9, pos);

					if(MouseInBookRect(pos.x, pos.y, pos.x + 32, pos.y + 32)) {
						GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
						GRenderer->SetRenderState(Renderer::AlphaBlending, true);
						DrawBookInterfaceItem(ITC.accessible_9, pos, Color::grayb(0x55));
						GRenderer->SetRenderState(Renderer::AlphaBlending, false);
						SpecialCursor=CURSOR_INTERACTION_ON;
						if(bookclick) {
							Book_Page=9;
							ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
						}
					}
				}
				else DrawBookInterfaceItem(ITC.current_9, Vec2f(99.f, 307.f));
			}

			if (bOnglet[10]) {
				if (Book_Page!=10) {
					Vec2f pos = Vec2f(97.f, 331.f);
					
					DrawBookInterfaceItem(ITC.accessible_10, pos);

					if(MouseInBookRect(pos.x, pos.y, pos.x + 32, pos.y + 32)) {
						GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
						GRenderer->SetRenderState(Renderer::AlphaBlending, true);
						DrawBookInterfaceItem(ITC.accessible_10, pos, Color::grayb(0x55));
						GRenderer->SetRenderState(Renderer::AlphaBlending, false);
						SpecialCursor=CURSOR_INTERACTION_ON;
						if(bookclick) {
							Book_Page=10;
							ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
						}
					}
				}
				else DrawBookInterfaceItem(ITC.current_10, Vec2f(104.f, 331.f));
			}
			
			if(Book_Mode == BOOKMODE_SPELLS) {
				Book_SpellPage = Book_Page;
			} else if(Book_Mode == BOOKMODE_MINIMAP) {
				Book_MapPage = Book_Page;
			}
		}

		bookclick = false;
	}
	
	GRenderer->GetTextureStage(0)->setMinFilter(TextureStage::FilterNearest);
	GRenderer->GetTextureStage(0)->setMagFilter(TextureStage::FilterNearest);

	if (Book_Mode == BOOKMODE_STATS)
	{
		FLYING_OVER = 0;
		std::string tex;
		Color color(0, 0, 0);

		ARX_PLAYER_ComputePlayerFullStats();

		std::stringstream ss;
		ss << ITC.Level << " " << std::setw(3) << (int)player.level;
		tex = ss.str();
		DrawBookTextCenter(hFontInBook, Vec2f(398, 74), tex, color);

		std::stringstream ss2;
		ss2 << ITC.Xp << " " << std::setw(8) << player.xp;
		tex = ss2.str();
		DrawBookTextCenter(hFontInBook, Vec2f(510, 74), tex, color);

		if (MouseInBookRect(463, 74, 550, 94))
			FLYING_OVER = WND_XP;

		if (MouseInBookRect(97+41,64+62, 97+41+32, 64+62+32))
			FLYING_OVER = WND_AC;
		else if (MouseInBookRect(97+41,64+120, 97+41+32, 64+120+32))
			FLYING_OVER = WND_RESIST_MAGIC;
		else if (MouseInBookRect(97+41,64+178, 97+41+32, 64+178+32))
			FLYING_OVER = WND_RESIST_POISON;
		else if (MouseInBookRect(97+211,64+62, 97+211+32, 64+62+32))
			FLYING_OVER = WND_HP;
		else if (MouseInBookRect(97+211,64+120, 97+211+32, 64+120+32))
			FLYING_OVER = WND_MANA;
		else if (MouseInBookRect(97+211,64+178, 97+211+32, 64+178+32))
			FLYING_OVER = WND_DAMAGE;

		if(!((player.Attribute_Redistribute == 0) && (ARXmenu.currentmode != AMCM_NEWQUEST))) {
			// Main Player Attributes
			if(CheckAttributeClick(379, 95, &player.m_attribute.strength, ITC.ic_strength)) {
				FLYING_OVER = BOOK_STRENGTH;
				SpecialCursor = CURSOR_REDIST;
				lCursorRedistValue = player.Attribute_Redistribute;
			}

			if(CheckAttributeClick(428, 95, &player.m_attribute.mind, ITC.ic_mind)) {
				FLYING_OVER = BOOK_MIND;
				SpecialCursor = CURSOR_REDIST;
				lCursorRedistValue = player.Attribute_Redistribute;
			}

			if(CheckAttributeClick(477, 95, &player.m_attribute.dexterity, ITC.ic_dexterity)) {
				FLYING_OVER = BOOK_DEXTERITY;
				SpecialCursor = CURSOR_REDIST;
				lCursorRedistValue = player.Attribute_Redistribute;
			}

			if(CheckAttributeClick(526, 95, &player.m_attribute.constitution, ITC.ic_constitution)) {
				FLYING_OVER = BOOK_CONSTITUTION;
				SpecialCursor = CURSOR_REDIST;
				lCursorRedistValue = player.Attribute_Redistribute;
			}
		}

		if(!((player.Skill_Redistribute == 0) && (ARXmenu.currentmode != AMCM_NEWQUEST))) {
			if (CheckSkillClick(389, 177, &player.m_skill.stealth, ITC.ic_stealth, &player.m_skillOld.stealth)) {
				FLYING_OVER = BOOK_STEALTH;
				SpecialCursor = CURSOR_REDIST;
				lCursorRedistValue = player.Skill_Redistribute;
			}

			if(CheckSkillClick(453, 177, &player.m_skill.mecanism, ITC.ic_mecanism, &player.m_skillOld.mecanism)) {
				FLYING_OVER = BOOK_MECANISM;
				SpecialCursor = CURSOR_REDIST;
				lCursorRedistValue = player.Skill_Redistribute;
			}

			if(CheckSkillClick(516, 177, &player.m_skill.intuition, ITC.ic_intuition, &player.m_skillOld.intuition)) {
				FLYING_OVER = BOOK_INTUITION;
				SpecialCursor = CURSOR_REDIST;
				lCursorRedistValue = player.Skill_Redistribute;
			}

			if(CheckSkillClick(389, 230, &player.m_skill.etheralLink, ITC.ic_etheral_link, &player.m_skillOld.etheralLink)) {
				FLYING_OVER = BOOK_ETHERAL_LINK;
				SpecialCursor = CURSOR_REDIST;
				lCursorRedistValue = player.Skill_Redistribute;
			}

			if(CheckSkillClick(453, 230, &player.m_skill.objectKnowledge, ITC.ic_object_knowledge, &player.m_skillOld.objectKnowledge)) {
				FLYING_OVER = BOOK_OBJECT_KNOWLEDGE;
				SpecialCursor = CURSOR_REDIST;
				lCursorRedistValue = player.Skill_Redistribute;

				if((BOOKBUTTON & 1) && !(LASTBOOKBUTTON & 1)) {
					ARX_INVENTORY_IdentifyAll();
					ARX_EQUIPMENT_IdentifyAll();
				}

				ARX_PLAYER_ComputePlayerFullStats();
			}

			if(CheckSkillClick(516, 230, &player.m_skill.casting, ITC.ic_casting, &player.m_skillOld.casting)) {
				FLYING_OVER = BOOK_CASTING;
				SpecialCursor = CURSOR_REDIST;
				lCursorRedistValue = player.Skill_Redistribute;
			}

			if(CheckSkillClick(389, 284, &player.m_skill.closeCombat, ITC.ic_close_combat, &player.m_skillOld.closeCombat)) {
				FLYING_OVER = BOOK_CLOSE_COMBAT;
				SpecialCursor = CURSOR_REDIST;
				lCursorRedistValue = player.Skill_Redistribute;
			}

			if(CheckSkillClick(453, 284, &player.m_skill.projectile, ITC.ic_projectile, &player.m_skillOld.projectile)) {
				FLYING_OVER = BOOK_PROJECTILE;
				SpecialCursor = CURSOR_REDIST;
				lCursorRedistValue = player.Skill_Redistribute;
			}

			if(CheckSkillClick(516, 284, &player.m_skill.defense, ITC.ic_defense, &player.m_skillOld.defense)) {
				FLYING_OVER = BOOK_DEFENSE;
				SpecialCursor = CURSOR_REDIST;
				lCursorRedistValue = player.Skill_Redistribute;
			}
		} else {
			//------------------------------------PRIMARY
			if (MouseInBookRect(379,95, 379+32, 95+32))
				FLYING_OVER=BOOK_STRENGTH;
			else if (MouseInBookRect(428,95, 428+32, 95+32))
				FLYING_OVER=BOOK_MIND;
			else if (MouseInBookRect(477,95, 477+32, 95+32))
				FLYING_OVER=BOOK_DEXTERITY;
			else if (MouseInBookRect(526,95, 526+32, 95+32))
				FLYING_OVER=BOOK_CONSTITUTION;

			//------------------------------------SECONDARY
			if (MouseInBookRect(389,177, 389+32, 177+32))
				FLYING_OVER=BOOK_STEALTH;
			else if (MouseInBookRect(453,177, 453+32, 177+32))
				FLYING_OVER=BOOK_MECANISM;
			else if (MouseInBookRect(516,177, 516+32, 177+32))
				FLYING_OVER=BOOK_INTUITION;
			else if (MouseInBookRect(389,230, 389+32, 230+32))
				FLYING_OVER=BOOK_ETHERAL_LINK;
			else if (MouseInBookRect(453,230, 453+32, 230+32))
				FLYING_OVER=BOOK_OBJECT_KNOWLEDGE;
			else if (MouseInBookRect(516,230, 516+32, 230+32))
				FLYING_OVER=BOOK_CASTING;
			else if (MouseInBookRect(389,284, 389+32, 284+32))
				FLYING_OVER=BOOK_CLOSE_COMBAT;
			else if (MouseInBookRect(453,284, 453+32, 284+32))
				FLYING_OVER=BOOK_PROJECTILE;
			else if (MouseInBookRect(516,284, 516+32, 284+32))
				FLYING_OVER=BOOK_DEFENSE;
		}

		//------------------------------ SEB 04/12/2001
		if(ARXmenu.mda && !ARXmenu.mda->flyover[FLYING_OVER].empty()) {
			
			float fRandom = rnd() * 2;

			int t = checked_range_cast<int>(fRandom);

			pTextManage->Clear();
			OLD_FLYING_OVER=FLYING_OVER;

			std::string toDisplay;

			// Nuky Note: the text used never scrolls, centered function with wordwrap would be enough
			if(FLYING_OVER == WND_XP) {
				std::stringstream ss;
				ss << ARXmenu.mda->flyover[WND_XP] << " " << std::setw(8) << GetXPforLevel(player.level+1)-player.xp;

				toDisplay = ss.str();
			} else {
				toDisplay = ARXmenu.mda->flyover[FLYING_OVER];
			}

			UNICODE_ARXDrawTextCenteredScroll(hFontInGame,
				(g_size.width()*0.5f),
				4,
				(g_size.center().x)*0.82f,
				toDisplay,
				Color(232+t,204+t,143+t),
				1000,
				0.01f,
				3,
				0);
		} else {
			OLD_FLYING_OVER=-1;
		}

		//------------------------------
		
		std::stringstream ss3;
		ss3 << std::setw(3) << std::setprecision(0) << std::fixed << player.m_attributeFull.strength;
		tex = ss3.str();
		
		if(player.m_attributeMod.strength < 0.f)
			color = Color::red;
		else if(player.m_attributeMod.strength > 0.f)
			color = Color::blue;
		else
			color = Color::black;
		
		if(ARXmenu.currentmode == AMCM_NEWQUEST) {
			if(player.m_attributeFull.strength == 6)
				color = Color::red;
		}
		
		DrawBookTextCenter(hFontInBook, Vec2f(391, 129), tex, color);
		
		ss3.str(""); // clear the stream
		ss3 << player.m_attributeFull.mind;
		tex = ss3.str();
		
		if(player.m_attributeMod.mind < 0.f)
			color = Color::red;
		else if(player.m_attributeMod.mind > 0.f)
			color = Color::blue;
		else
			color = Color::black;
		
		if(ARXmenu.currentmode == AMCM_NEWQUEST) {
			if(player.m_attributeFull.mind == 6)
				color = Color::red;
		}
		
		DrawBookTextCenter(hFontInBook, Vec2f(440, 129), tex, color);
		
		ss3.str("");
		ss3 << player.m_attributeFull.dexterity;
		tex = ss3.str();

		if(player.m_attributeMod.dexterity < 0.f)
			color = Color::red;
		else if(player.m_attributeMod.dexterity > 0.f)
			color = Color::blue;
		else
			color = Color::black;
		
		if(ARXmenu.currentmode == AMCM_NEWQUEST) {
			if(player.m_attributeFull.dexterity == 6)
				color = Color::red;
		}
		
		DrawBookTextCenter(hFontInBook, Vec2f(490, 129), tex, color);
		ss3.str("");
		ss3 << player.m_attributeFull.constitution;
		tex = ss3.str();
		
		if(player.m_attributeMod.constitution < 0.f)
			color = Color::red;
		else if(player.m_attributeMod.constitution > 0.f)
			color = Color::blue;
		else
			color = Color::black;
		
		if(ARXmenu.currentmode == AMCM_NEWQUEST) {
			if(player.m_attributeFull.constitution == 6)
				color = Color::red;
		}
		
		DrawBookTextCenter(hFontInBook, Vec2f(538, 129), tex, color);
		
		// Player Skills
		ss3.str("");
		ss3 << player.m_skillFull.stealth;
		tex = ss3.str();
		
		if (player.m_skillMod.stealth < 0.f)
			color = Color::red;
		else if (player.m_skillMod.stealth > 0.f)
			color = Color::blue;
		else
			color = Color::black;
		
		if(ARXmenu.currentmode == AMCM_NEWQUEST) {
			if(player.m_skill.stealth == 0)
				color = Color::red;
		}
		
		DrawBookTextCenter(hFontInBook, Vec2f(405, 210), tex, color);
		
		ss3.str("");
		ss3 << player.m_skillFull.mecanism;
		tex = ss3.str();
		
		if (player.m_skillMod.mecanism < 0.f)
			color = Color::red;
		else if (player.m_skillMod.mecanism > 0.f)
			color = Color::blue;
		else
			color = Color::black;
		
		if(ARXmenu.currentmode == AMCM_NEWQUEST) {
			if(player.m_skill.mecanism == 0)
				color = Color::red;
		}
		
		DrawBookTextCenter(hFontInBook, Vec2f(469, 210), tex, color);
		
		ss3.str("");
		ss3 << player.m_skillFull.intuition;
		tex = ss3.str();
		
		if (player.m_skillMod.intuition < 0.f)
			color = Color::red;
		else if (player.m_skillMod.intuition > 0.f)
			color = Color::blue;
		else
			color = Color::black;
		
		if(ARXmenu.currentmode == AMCM_NEWQUEST) {
			if(player.m_skill.intuition == 0)
				color = Color::red;
		}

		DrawBookTextCenter(hFontInBook, Vec2f(533, 210), tex, color);
		
		ss3.str("");
		ss3 << player.m_skillFull.etheralLink;
		tex = ss3.str();

		if(player.m_skillMod.etheralLink < 0.f)
			color = Color::red;
		else if(player.m_skillMod.etheralLink > 0.f)
			color = Color::blue;
		else
			color = Color::black;
		
		if(ARXmenu.currentmode == AMCM_NEWQUEST) {
			if(player.m_skill.etheralLink == 0)
				color = Color::red;
		}

		DrawBookTextCenter(hFontInBook, Vec2f(405, 265), tex, color);
		
		ss3.str("");
		ss3 << player.m_skillFull.objectKnowledge;
		tex = ss3.str();

		if(player.m_skillMod.objectKnowledge < 0.f)
			color = Color::red;
		else if(player.m_skillMod.objectKnowledge > 0.f)
			color = Color::blue;
		else
			color = Color::black;
		
		if(ARXmenu.currentmode == AMCM_NEWQUEST) {
			if(player.m_skill.objectKnowledge == 0)
				color = Color::red;
		}
		
		DrawBookTextCenter(hFontInBook, Vec2f(469, 265), tex, color);
		
		ss3.str("");
		ss3 << player.m_skillFull.casting;
		tex = ss3.str();
		
		if (player.m_skillMod.casting < 0.f)
			color = Color::red;
		else if (player.m_skillMod.casting > 0.f)
			color = Color::blue;
		else
			color = Color::black;
		
		if(ARXmenu.currentmode == AMCM_NEWQUEST) {
			if(player.m_skill.casting == 0)
				color = Color::red;
		}

		DrawBookTextCenter(hFontInBook, Vec2f(533, 265), tex, color);
		
		ss3.str("");
		ss3 << player.m_skillFull.closeCombat;
		tex = ss3.str();

		if (player.m_skillMod.closeCombat < 0.f)
			color = Color::red;
		else if (player.m_skillMod.closeCombat > 0.f)
			color = Color::blue;
		else
			color = Color::black;
		
		if(ARXmenu.currentmode == AMCM_NEWQUEST) {
			if(player.m_skill.closeCombat == 0)
				color = Color::red;
		}

		DrawBookTextCenter(hFontInBook, Vec2f(405, 319), tex, color);

		
		ss3.str("");
		ss3 << player.m_skillFull.projectile;
		tex = ss3.str();

		if(player.m_skillMod.projectile < 0.f)
			color = Color::red;
		else if(player.m_skillMod.projectile > 0.f)
			color = Color::blue;
		else
			color = Color::black;
		
		if(ARXmenu.currentmode == AMCM_NEWQUEST) {
			if(player.m_skill.projectile == 0)
				color = Color::red;
		}

		DrawBookTextCenter(hFontInBook, Vec2f(469, 319), tex, color);
		
		ss3.str("");
		ss3 << player.m_skillFull.defense;
		tex = ss3.str();

		if (player.m_skillMod.defense < 0.f)
			color = Color::red;
		else if (player.m_skillMod.defense > 0.f)
			color = Color::blue;
		else
			color = Color::black;
		
		if(ARXmenu.currentmode == AMCM_NEWQUEST) {
			if(player.m_skill.defense == 0)
				color = Color::red;
		}
		
		DrawBookTextCenter(hFontInBook, Vec2f(533, 319), tex, color);
		
		// Secondary Attributes
		std::stringstream ss4;
		ss4.str("");
		ss4 << F2L_RoundUp(player.Full_maxlife);
		tex = ss4.str();
		
		if(player.Full_maxlife < player.lifePool.max) {
			color = Color::red;
		} else if(player.Full_maxlife > player.lifePool.max) {
			color = Color::blue;
		} else {
			color = Color::black;
		}
		
		DrawBookTextCenter(hFontInBook, Vec2f(324, 158), tex, color);
		
		ss4.str("");
		ss4 << F2L_RoundUp(player.Full_maxmana);
		tex = ss4.str();

		if(player.Full_maxmana < player.manaPool.max) {
			color = Color::red;
		} else if(player.Full_maxmana > player.manaPool.max) {
			color = Color::blue;
		} else {
			color = Color::black;
		}
		
		DrawBookTextCenter(hFontInBook, Vec2f(324, 218), tex, color);
		
		ss4.str("");
		ss4 << F2L_RoundUp(player.m_miscFull.damages);
		tex = ss4.str();
		
		if (player.m_miscMod.damages < 0.f)
			color = Color::red;
		else if (player.m_miscMod.damages > 0.f)
			color = Color::blue;
		else
			color = Color::black;
		
		DrawBookTextCenter(hFontInBook, Vec2f(324, 278), tex, color);
		
		float ac = player.m_miscFull.armorClass;
		ss4.str("");
		ss4 << F2L_RoundUp(ac);
		tex = ss4.str();
		
		if (player.m_miscMod.armorClass < 0.f)
			color = Color::red;
		else if (player.m_miscMod.armorClass > 0.f)
			color = Color::blue;
		else
			color = Color::black;
		
		DrawBookTextCenter(hFontInBook, Vec2f(153, 158), tex, color);
		
		ss4.str("");
		ss4 << std::setw(3) << std::setprecision(0) << F2L_RoundUp( player.m_miscFull.resistMagic );
		tex = ss4.str();
		
		if (player.m_miscMod.resistMagic < 0.f)
			color = Color::red;
		else if (player.m_miscMod.resistMagic > 0.f)
			color = Color::blue;
		else
			color = Color::black;
		
		DrawBookTextCenter(hFontInBook, Vec2f(153, 218), tex, color);
		
		ss4.str("");
		ss4 << F2L_RoundUp( player.m_miscFull.resistPoison );
		tex = ss4.str();

		if (player.m_miscMod.resistPoison<0.f)
			color = Color::red;
		else if (player.m_miscMod.resistPoison>0.f)
			color = Color::blue;
		else
			color = Color::black;
		
		DrawBookTextCenter(hFontInBook, Vec2f(153, 278), tex, color);
	} else if (Book_Mode == BOOKMODE_MINIMAP) {
		long SHOWLEVEL = Book_MapPage - 1;

		if(SHOWLEVEL >= 0 && SHOWLEVEL < 32)
			g_miniMap.showBookEntireMap(SHOWLEVEL);

		SHOWLEVEL = ARX_LEVELS_GetRealNum(CURRENTLEVEL);

		if(SHOWLEVEL >= 0 && SHOWLEVEL < 32)
			g_miniMap.showBookMiniMap(SHOWLEVEL);
	}

	if((Book_Mode == BOOKMODE_STATS) && entities.player()->obj) {
		
		GRenderer->SetRenderState(Renderer::DepthWrite, true);

		Rect rec;
		if (BOOKZOOM) {
			
			rec = Rect(s32((120.f + BOOKDEC.x) * g_sizeRatio.x), s32((69.f + BOOKDEC.y) * g_sizeRatio.y),
			           s32((330.f + BOOKDEC.x) * g_sizeRatio.x), s32((300.f + BOOKDEC.y) * g_sizeRatio.y));
			GRenderer->Clear(Renderer::DepthBuffer, Color::none, 1.f, 1, &rec);

			if(ARXmenu.currentmode != AMCM_OFF) {
				Rect vp = Rect(Vec2i(s32(139.f * g_sizeRatio.x), 0), s32(139.f * g_sizeRatio.x), s32(310.f * g_sizeRatio.y));
				GRenderer->SetScissor(vp);
			}
		} else {
			
			rec = Rect(s32((118.f + BOOKDEC.x) * g_sizeRatio.x), s32((69.f + BOOKDEC.y) * g_sizeRatio.y),
			          s32((350.f + BOOKDEC.x) * g_sizeRatio.x), s32((338.f + BOOKDEC.y) * g_sizeRatio.y));
			GRenderer->Clear(Renderer::DepthBuffer, Color::none, 1.f, 1, &rec);

			rec.right -= 50;
		}

		if (ARXmenu.currentmode==AMCM_OFF)
			BOOKZOOM=0;

		Vec3f pos;
		EERIE_LIGHT eLight1;
		EERIE_LIGHT eLight2;
		
		eLight1.pos = Vec3f(50.f, 50.f, 200.f);
		eLight1.exist = 1;
		eLight1.rgb = Color3f(0.15f, 0.06f, 0.003f);
		eLight1.intensity = 8.8f;
		eLight1.fallstart = 2020;
		eLight1.fallend = eLight1.fallstart + 60;
		RecalcLight(&eLight1);
		
		eLight2.exist = 1;
		eLight2.pos = Vec3f(-50.f, -50.f, -200.f);
		eLight2.rgb = Color3f::gray(0.6f);
		eLight2.intensity=3.8f;
		eLight2.fallstart = 0;
		eLight2.fallend = eLight2.fallstart + 3460.f;
		RecalcLight(&eLight2);
		
		EERIE_LIGHT * SavePDL[2];
		SavePDL[0]=PDL[0];
		SavePDL[1]=PDL[1];
		int			iSavePDL=TOTPDL;

		PDL[0] = &eLight1;
		PDL[1] = &eLight2;
		TOTPDL = 2;

		EERIE_CAMERA * oldcam = ACTIVECAM;
		bookcam.center = rec.center();
		SetActiveCamera(&bookcam);
		PrepareCamera(&bookcam, g_size);

		Anglef ePlayerAngle = Anglef::ZERO;

		if(BOOKZOOM) {
			Rect vp;
			vp.left = static_cast<int>(rec.left + 52.f * g_sizeRatio.x);
			vp.top = rec.top;
			vp.right = static_cast<int>(rec.right - 21.f * g_sizeRatio.x);
			vp.bottom = static_cast<int>(rec.bottom - 17.f * g_sizeRatio.y);
			GRenderer->SetScissor(vp);

			switch (player.skin)
			{
				case 0:
					ePlayerAngle.setPitch(-25.f);
					break;
				case 1:
					ePlayerAngle.setPitch(-10.f);
					break;
				case 2:
					ePlayerAngle.setPitch(20.f);
					break;
				case 3:
					ePlayerAngle.setPitch(35.f);
					break;
			}

			pos.x=8;
			pos.y=162.f;
			pos.z=75.f;
			eLight1.pos.z=-90.f;
		} else {
			GRenderer->SetScissor(rec);

			ePlayerAngle.setPitch(-20.f);
			pos.x=20.f;
			pos.y=96.f;
			pos.z=260.f;

			ARX_EQUIPMENT_AttachPlayerWeaponToHand();
		}

		bool ti = player.m_improve;
		player.m_improve = false;


		float invisibility = entities.player()->invisibility;

		if(invisibility > 0.5f)
			invisibility = 0.5f;

		IN_BOOK_DRAW=1;
		std::vector<EERIE_VERTEX> vertexlist = entities.player()->obj->vertexlist3;

		arx_assert(player.bookAnimation[0].cur_anim);

		EERIEDrawAnimQuat(entities.player()->obj, player.bookAnimation, ePlayerAngle, pos,
						  checked_range_cast<unsigned long>(Original_framedelay), NULL, true, invisibility);

		IN_BOOK_DRAW=0;
		
		if(ARXmenu.currentmode == AMCM_NEWQUEST) {
			GRenderer->SetRenderState(Renderer::DepthTest, true);
			GRenderer->GetTextureStage(0)->setMipFilter(TextureStage::FilterNone);
			GRenderer->SetRenderState(Renderer::AlphaBlending, false);
			PopAllTriangleList();
			GRenderer->SetRenderState(Renderer::AlphaBlending, true);
			PopAllTriangleListTransparency();
			GRenderer->SetRenderState(Renderer::AlphaBlending, false);
			GRenderer->GetTextureStage(0)->setMipFilter(TextureStage::FilterLinear);
			GRenderer->SetRenderState(Renderer::DepthTest, false);
		}

		PDL[0]=SavePDL[0];
		PDL[1]=SavePDL[1];
		TOTPDL=iSavePDL;

		entities.player()->obj->vertexlist3 = vertexlist;
		vertexlist.clear();

		player.m_improve = ti;
		
		GRenderer->SetScissor(Rect::ZERO);
		
		GRenderer->SetRenderState(Renderer::AlphaBlending, false);
		GRenderer->SetCulling(Renderer::CullNone);
		SetActiveCamera(oldcam);
		PrepareCamera(oldcam, g_size);

			player.bookAnimation[0].cur_anim = herowaitbook;

			if(player.equiped[EQUIP_SLOT_WEAPON] && ValidIONum(player.equiped[EQUIP_SLOT_WEAPON])) {
				if(entities[player.equiped[EQUIP_SLOT_WEAPON]]->type_flags & OBJECT_TYPE_2H) {
					player.bookAnimation[0].cur_anim = herowait_2h;
				}
			}

			GRenderer->SetCulling(Renderer::CullNone);

			if(player.equiped[EQUIP_SLOT_ARMOR] && ValidIONum(player.equiped[EQUIP_SLOT_ARMOR])) {
				Entity *tod = entities[player.equiped[EQUIP_SLOT_ARMOR]];
				if(tod) {
					tod->bbox2D.min = Vec2f(195.f, 116.f);
					tod->bbox2D.max = Vec2f(284.f, 182.f);

					tod->bbox2D.min = (tod->bbox2D.min + BOOKDEC) * g_sizeRatio;
					tod->bbox2D.max = (tod->bbox2D.max + BOOKDEC) * g_sizeRatio;

					tod->ioflags |= IO_ICONIC;
				}
			}

			if(player.equiped[EQUIP_SLOT_LEGGINGS] && ValidIONum(player.equiped[EQUIP_SLOT_LEGGINGS])) {
				Entity *tod = entities[player.equiped[EQUIP_SLOT_LEGGINGS]];
				if(tod) {
					tod->bbox2D.min = Vec2f(218.f, 183.f);
					tod->bbox2D.max = Vec2f(277.f, 322.f);

					tod->bbox2D.min = (tod->bbox2D.min + BOOKDEC) * g_sizeRatio;
					tod->bbox2D.max = (tod->bbox2D.max + BOOKDEC) * g_sizeRatio;

					tod->ioflags |= IO_ICONIC;
				}
			}

			if(player.equiped[EQUIP_SLOT_HELMET] && ValidIONum(player.equiped[EQUIP_SLOT_HELMET])) {
				Entity *tod = entities[player.equiped[EQUIP_SLOT_HELMET]];
				if(tod) {
					tod->bbox2D.min = Vec2f(218.f, 75.f);
					tod->bbox2D.max = Vec2f(260.f, 115.f);

					tod->bbox2D.min = (tod->bbox2D.min + BOOKDEC) * g_sizeRatio;
					tod->bbox2D.max = (tod->bbox2D.max + BOOKDEC) * g_sizeRatio;

					tod->ioflags |= IO_ICONIC;
				}
			}

			TextureContainer * tc;
			TextureContainer * tc2=NULL;
			GRenderer->SetCulling(Renderer::CullNone);

			if(player.equiped[EQUIP_SLOT_RING_LEFT] && ValidIONum(player.equiped[EQUIP_SLOT_RING_LEFT])) {
				Entity *todraw = entities[player.equiped[EQUIP_SLOT_RING_LEFT]];

				tc = todraw->inv;

				if(NeedHalo(todraw))
					tc2 = todraw->inv->getHalo();

				if(tc) {
					todraw->bbox2D.min = Vec2f(146.f, 312.f);

					Color color = (todraw->poisonous && todraw->poisonous_count != 0) ? Color::green : Color::white;
					DrawBookInterfaceItem(tc, todraw->bbox2D.min, color, 0);

					if(tc2) {
						ARX_INTERFACE_HALO_Draw(todraw, tc, tc2, (todraw->bbox2D.min + BOOKDEC) * g_sizeRatio, g_sizeRatio);
					}
					
					todraw->bbox2D.max = todraw->bbox2D.min + Vec2f(tc->size());
					
					todraw->bbox2D.min = (todraw->bbox2D.min + BOOKDEC) * g_sizeRatio;
					todraw->bbox2D.max = (todraw->bbox2D.max + BOOKDEC) * g_sizeRatio;

					todraw->ioflags |= IO_ICONIC;
				}
			}

			tc2=NULL;

			if(player.equiped[EQUIP_SLOT_RING_RIGHT] && ValidIONum(player.equiped[EQUIP_SLOT_RING_RIGHT])) {
				Entity *todraw = entities[player.equiped[EQUIP_SLOT_RING_RIGHT]];

				tc = todraw->inv;

				if(NeedHalo(todraw))
					tc2 = todraw->inv->getHalo();

				if(tc) {
					todraw->bbox2D.min = Vec2f(296.f, 312.f);

					Color color = (todraw->poisonous && todraw->poisonous_count != 0) ? Color::green : Color::white;
					DrawBookInterfaceItem(tc, todraw->bbox2D.min, color, 0);

					if(tc2) {
						ARX_INTERFACE_HALO_Draw(todraw, tc, tc2, (todraw->bbox2D.min + BOOKDEC) * g_sizeRatio, g_sizeRatio);
					}
					
					todraw->bbox2D.max = todraw->bbox2D.min + Vec2f(tc->size());
					
					todraw->bbox2D.min = (todraw->bbox2D.min + BOOKDEC) * g_sizeRatio;
					todraw->bbox2D.max = (todraw->bbox2D.max + BOOKDEC) * g_sizeRatio;

					todraw->ioflags |= IO_ICONIC;
				}
			}

			if(!BOOKZOOM)
				ARX_EQUIPMENT_AttachPlayerWeaponToBack();

			Halo_Render();
	}
}



//-----------------------------------------------------------------------------
void ArxGame::manageEditorControls() {
	
	ARX_PROFILE_FUNC();
	
	eMouseState = MOUSE_IN_WORLD;

	if(   TRUE_PLAYER_MOUSELOOK_ON
	   && config.input.autoReadyWeapon == false
	   && config.input.mouseLookToggle
	) {
		DANAEMouse = g_size.center();
	}
	
	playerInterfaceFader.updateFirst();
	
	// on ferme
	if((player.Interface & INTER_COMBATMODE) || player.doingmagic >= 2) {
		Entity * io = NULL;

		if(SecondaryInventory)
			io = SecondaryInventory->io;
		else if (player.Interface & INTER_STEAL)
			io = ioSteal;

		if(io) {
			InventoryDir = -1;
			SendIOScriptEvent(io, SM_INVENTORY2_CLOSE);
			TSecondaryInventory = SecondaryInventory;
			SecondaryInventory = NULL;
		}
	}
	
	playerInterfaceFader.update();
	cinematicBorder.update();
	
	if(EERIEMouseButton & 1) {
		static Vec2s dragThreshold = Vec2s_ZERO;
		
		if(!(LastMouseClick & 1)) {
			
			STARTDRAG = DANAEMouse;
			DRAGGING = false;
			dragThreshold = Vec2s_ZERO;
		} else {
			dragThreshold += GInput->getMousePosRel();
			if((abs(DANAEMouse.x - STARTDRAG.x) > 2 && abs(DANAEMouse.y - STARTDRAG.y) > 2)
			   || (abs(dragThreshold.x) > 2 || abs(dragThreshold.y) > 2)) {
				DRAGGING = true;
			}
		}
	} else {
		DRAGGING = false;
	}
	
	manageEditorControlsHUD();
	
	// gros player book
	if(player.Interface & INTER_MAP) {
		Vec2f pos(97 * g_sizeRatio.x, 64 * g_sizeRatio.y);
		
		TextureContainer* playerbook = ITC.playerbook;
		arx_assert(ITC.playerbook);
		
		const Rect mouseTestRect(
		pos.x,
		pos.y,
		pos.x + playerbook->m_dwWidth * g_sizeRatio.x,
		pos.y + playerbook->m_dwHeight * g_sizeRatio.y
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
	
	manageEditorControlsHUD2();
	
	// Single Click On Object
	if(   (LastMouseClick & 1)
	   && !(EERIEMouseButton & 1)
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
		   && !InPlayerInventoryPos(DANAEMouse)
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
					// TODO
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
		if(!(EERIEMouseButton & 1) && (LastMouseClick & 1) && DRAGINTER) {
			//if (ARX_EQUIPMENT_PutOnPlayer(DRAGINTER))
			if(InInventoryPos(DANAEMouse)) {// Attempts to put it in inventory
				PutInInventory();
			} else if(eMouseState == MOUSE_IN_INVENTORY_ICON) {
				PutInInventory();
			} else if(ARX_INTERFACE_MouseInBook()) {
				if(Book_Mode == BOOKMODE_STATS) {
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
					
					bool res = Manage3DCursor(false);
					// Throw Object
					if(!res) {
						Entity * io=DRAGINTER;
						ARX_PLAYER_Remove_Invisibility();
						io->obj->pbox->active=1;
						io->obj->pbox->stopcount=0;
						io->pos = player.pos + Vec3f(0.f, 80.f, 0.f);
						io->velocity = Vec3f_ZERO;
						io->stopped = 1;
						
						float y_ratio=(float)((float)DANAEMouse.y-(float)g_size.center().y)/(float)g_size.height()*2;
						float x_ratio=-(float)((float)DANAEMouse.x-(float)g_size.center().x)/(float)g_size.center().x;
						Vec3f viewvector;
						viewvector.x = -std::sin(glm::radians(player.angle.getPitch()+(x_ratio*30.f))) * std::cos(glm::radians(player.angle.getYaw()));
						viewvector.y =  std::sin(glm::radians(player.angle.getYaw())) + y_ratio;
						viewvector.z =  std::cos(glm::radians(player.angle.getPitch()+(x_ratio*30.f))) * std::cos(glm::radians(player.angle.getYaw()));
						
						io->soundtime=0;
						io->soundcount=0;
						
						EERIE_PHYSICS_BOX_Launch(io->obj, io->pos, io->angle, viewvector);
						ARX_SOUND_PlaySFX(SND_WHOOSH, &io->pos);
						
						io->show=SHOW_FLAG_IN_SCENE;
						Set_DragInter(NULL);
					}
				}
			}
		}
		
		if(COMBINE) {
			if(!player.torch || (player.torch && (COMBINE != player.torch))) {
				Vec3f pos = GetItemWorldPosition(COMBINE);
				
				if(fartherThan(pos, player.pos, 300.f))
					COMBINE = NULL;
			}
		}
		
		if((EERIEMouseButton & 1) && !(LastMouseClick & 1) && (COMBINE || COMBINEGOLD)) {
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
				float fMaxdist = 300;
	
				if(player.m_telekinesis)
					fMaxdist = 850;
	
				for(size_t i = 0; i < MAX_LIGHTS; i++) {
					EERIE_LIGHT * light = GLight[i];
					
					if(   light
					   && light->exist
					   && !fartherThan(light->pos, player.pos, fMaxdist)
					   && !(light->extras & EXTRAS_NO_IGNIT)
					   && light->m_screenRect.toRect().contains(Vec2i(DANAEMouse))
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
					bQuitCombine = inventoryGuiupdateInputPROXY();
				}
			}
	
			if(bQuitCombine) {
				COMBINE=NULL;
				EERIEMouseButton &= ~1;
			}
		}
		
		//lights
		if(COMBINE) {
			float fMaxdist = 300;
			
			if(player.m_telekinesis)
				fMaxdist = 850;
			
			for(size_t i = 0; i < MAX_LIGHTS; i++) {
				EERIE_LIGHT * light = GLight[i];
				
				if(   light
				   && light->exist
				   && !fartherThan(light->pos, player.pos, fMaxdist)
				   && !(light->extras & EXTRAS_NO_IGNIT)
				   && light->m_screenRect.toRect().contains(Vec2i(DANAEMouse))
				) {
					SpecialCursor = CURSOR_INTERACTION_ON;
				}
			}
		}

		// Double Clicked and not already combining.
		if((EERIEMouseButton & 4) && (COMBINE==NULL)) {
			bool accept_combine = true;
			
			if((SecondaryInventory!=NULL) && (InSecondaryInventoryPos(DANAEMouse))) {
				Entity * io = SecondaryInventory->io;
				
				if(io->ioflags & IO_SHOP)
					accept_combine = false;
			}
			
			if(accept_combine) {
				if(FlyingOverIO && ((FlyingOverIO->ioflags & IO_ITEM) && !(FlyingOverIO->ioflags & IO_MOVABLE))) {
					COMBINE=FlyingOverIO;
					GetInfosCombine();
					EERIEMouseButton &= ~4;
				}
				else if(InInventoryPos(DANAEMouse))
					EERIEMouseButton &= 4;
			}
		}
		
		// Checks for Object Dragging
		if(DRAGGING
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

						if(DRAGINTER->show==SHOW_FLAG_ON_PLAYER) {
							ARX_EQUIPMENT_UnEquip(entities.player(),DRAGINTER);
							RemoveFromAllInventories(DRAGINTER);
							DRAGINTER->bbox2D.max.x = -1;
						}

						if((io->ioflags & IO_NPC) || (io->ioflags & IO_FIX)) {
							Set_DragInter(NULL);
							goto suivant;
						}

						if(io->ioflags & IO_UNDERWATER) {
							io->ioflags&=~IO_UNDERWATER;
							ARX_SOUND_PlayInterface(SND_PLOUF, 0.8F + 0.4F * rnd());
						}

						DRAGINTER->show=SHOW_FLAG_NOT_DRAWN;
						ARX_SOUND_PlayInterface(SND_INVSTD);
					}
				} else {
					Set_DragInter(NULL);
				}

				suivant:
					;
			}
			else
				ARX_PLAYER_Remove_Invisibility();
		}
	
		// Debug Selection
		if((LastMouseClick & 1) && !(EERIEMouseButton & 1)) {
			Entity * io = GetFirstInterAtPos(DANAEMouse);

			if(io) {
				LastSelectedIONum = io->index();
			} else {
				if(LastSelectedIONum == EntityHandle::Invalid)
					LastSelectedIONum = PlayerEntityHandle;
				else
					LastSelectedIONum = EntityHandle::Invalid;
			}
		}
	}
}
