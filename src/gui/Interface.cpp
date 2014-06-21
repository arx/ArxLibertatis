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

#include "scene/LinkedObject.h"
#include "scene/GameSound.h"
#include "scene/Interactive.h"
#include "scene/Light.h"

#include "script/Script.h"

#include "window/RenderWindow.h"

extern float MagicSightFader;
extern float Original_framedelay;
extern EERIE_3DOBJ *arrowobj;
extern void InitTileLights();

//! Hide the quick save indicator
static void hideQuickSaveIcon();

long IN_BOOK_DRAW=0;

extern EntityHandle LastSelectedIONum;

//-----------------------------------------------------------------------------
struct ARX_INTERFACE_HALO_STRUCT
{
	Entity  * io;
	TextureContainer * tc;
	TextureContainer * tc2;
	float POSX;
	float POSY;
	float fRatioX;
	float fRatioY;
};
//-----------------------------------------------------------------------------
static const float BOOKMARKS_POS_X = 216.f;
static const float BOOKMARKS_POS_Y = 60.f;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
extern TextureContainer * Movable;
extern TextureContainer * inventory_font;
extern Notification speech[];
extern std::string WILLADDSPEECH;
extern float PLAYER_ROTATION;
extern float SLID_VALUE;

extern long BOOKBUTTON;
extern long LASTBOOKBUTTON;
static long lSLID_VALUE = 0;

extern long CHANGE_LEVEL_ICON;
extern bool BLOCK_PLAYER_CONTROLS;
extern long DeadTime;

extern bool WILLRETURNTOFREELOOK;
extern float BOW_FOCAL;
extern Vec2s DANAEMouse;
extern short sActiveInventory;
extern unsigned long WILLADDSPEECHTIME;

extern unsigned char ucFlick;

bool IsPlayerStriking();

extern bool SHOW_INGAME_MINIMAP;

//-----------------------------------------------------------------------------
TextureContainer *	BasicInventorySkin=NULL;

ARX_INTERFACE_HALO_STRUCT * aiHalo=NULL;
E_ARX_STATE_MOUSE	eMouseState;
bool bookclick;
Vec2s MemoMouse;

INVENTORY_DATA *	TSecondaryInventory;
Entity * FlyingOverIO=NULL;
Entity *	STARTED_ACTION_ON_IO=NULL;

INTERFACE_TC ITC = INTERFACE_TC();

static gui::Note openNote;
static gui::Note questBook;

bool				bBookHalo = false;
bool				bGoldHalo = false;
bool				bInventoryClosing = false;
bool				bInventorySwitch = false;
bool				bIsAiming = false;
unsigned long		ulBookHaloTime = 0;
unsigned long		ulGoldHaloTime = 0;
float				InventoryX=-60.f;
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
long				INVERTMOUSE=0;
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

static long lOldInterface;

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
float ARX_CAST_TO_INT_THEN_FLOAT( float _f )
{
	return ( ( _f >= 0 ) ? floor( _f ) : ceil( _f ) );
}

bool MouseInBookRect(const float x, const float y, const float cx, const float cy) {
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

static void ARX_INTERFACE_DrawItem(TextureContainer * tc, float x, float y, float z = 0.001f, Color col = Color::white) {
	if(tc) {
		EERIEDrawBitmap(Rectf(Vec2f(x, y), tc->m_dwWidth, tc->m_dwHeight), z, tc, col);
	}
}

void ARX_INTERFACE_DrawNumber(const Vec2f & pos, const long num, const int _iNb, const Color color) {
	
	ColorBGRA col = color.toBGRA();
	
	TexturedVertex v[4];
	v[0] = TexturedVertex(Vec3f_ZERO, 1.f, 1, Vec2f_ZERO);
	v[1] = TexturedVertex(Vec3f_ZERO, 1.f, 1, Vec2f_X_AXIS);
	v[2] = TexturedVertex(Vec3f_ZERO, 1.f, 1, Vec2f(1.f, 1.f));
	v[3] = TexturedVertex(Vec3f_ZERO, 1.f, 1, Vec2f_Y_AXIS);
	
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
void ARX_INTERFACE_HALO_Render(float _fR, float _fG, float _fB,
							   long _lHaloType,
							   TextureContainer * haloTexture,
							   float POSX, float POSY, float fRatioX = 1, float fRatioY = 1)
{
	float power = 0.9f;
	power -= std::sin(arxtime.get_frame_time()*0.01f) * 0.3f;

	_fR = clamp(_fR * power, 0, 1);
	_fG = clamp(_fG * power, 0, 1);
	_fB = clamp(_fB * power, 0, 1);
	Color col=Color4f(_fR,_fG,_fB).to<u8>();

	if(_lHaloType & HALO_NEGATIVE) {
		GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);
	} else {
		GRenderer->SetBlendFunc(Renderer::BlendSrcAlpha, Renderer::BlendOne);
	}
	
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	
	float x = POSX - TextureContainer::HALO_RADIUS * fRatioX;
	float y = POSY - TextureContainer::HALO_RADIUS * fRatioY;
	float width = haloTexture->m_dwWidth * fRatioX;
	float height = haloTexture->m_dwHeight * fRatioY;
	
	EERIEDrawBitmap(Rectf(Vec2f(x, y), width, height), 0.00001f, haloTexture, col);

	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
}

void ARX_INTERFACE_HALO_Draw(Entity * io, TextureContainer * tc, TextureContainer * tc2, float POSX, float POSY, float _fRatioX, float _fRatioY) {
	INTERFACE_HALO_NB++;
	
	if(INTERFACE_HALO_NB > INTERFACE_HALO_MAX_NB) {
		INTERFACE_HALO_MAX_NB = INTERFACE_HALO_NB;
		aiHalo = (ARX_INTERFACE_HALO_STRUCT *)realloc(aiHalo,sizeof(ARX_INTERFACE_HALO_STRUCT)*INTERFACE_HALO_NB);
	}
	
	aiHalo[INTERFACE_HALO_NB-1].io=io;
	aiHalo[INTERFACE_HALO_NB-1].tc=tc;
	aiHalo[INTERFACE_HALO_NB-1].tc2=tc2;
	aiHalo[INTERFACE_HALO_NB-1].POSX=POSX;
	aiHalo[INTERFACE_HALO_NB-1].POSY=POSY;
	aiHalo[INTERFACE_HALO_NB-1].fRatioX = _fRatioX;
	aiHalo[INTERFACE_HALO_NB-1].fRatioY = _fRatioY;
}

void ReleaseHalo() {
	free(aiHalo);
	aiHalo=NULL;
}

void ARX_INTERFACE_HALO_Flush() {
	
	for (long i=0;i<INTERFACE_HALO_NB;i++)
		ARX_INTERFACE_HALO_Render(
		aiHalo[i].io->halo.color.r, aiHalo[i].io->halo.color.g, aiHalo[i].io->halo.color.b,
		aiHalo[i].io->halo.flags,
		aiHalo[i].tc2,aiHalo[i].POSX,aiHalo[i].POSY, aiHalo[i].fRatioX, aiHalo[i].fRatioY);

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
void InventoryOpenClose(unsigned long t) // 0 switch 1 forceopen 2 forceclose
{
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

void ReleaseInfosCombine() {
	
	Entity * io = NULL;

	if (player.bag)
	for (int iNbBag=0; iNbBag<player.bag; iNbBag++)
	for (size_t j=0;j<INVENTORY_Y;j++)
	for (size_t i=0;i<INVENTORY_X;i++) {
		io = inventory[iNbBag][i][j].io;

		if(io)
			io->ioflags &= ~IO_CAN_COMBINE;
	}

	if(SecondaryInventory) {
		for(long j=0;j<SecondaryInventory->sizey;j++) {
			for(long i=0;i<SecondaryInventory->sizex;i++) {
				io=SecondaryInventory->slot[i][j].io;

				if(io)
					io->ioflags &= ~IO_CAN_COMBINE;
			}
		}
	}
}

//-----------------------------------------------------------------------------
char* findParam(char* pcToken, const char* param)
{
	char* pStartString = 0;

	if(strstr(pcToken,"^$param1"))
		pStartString = strstr(pcToken, param);

	return pStartString;
}

void GetInfosCombineWithIO(Entity * _pWithIO)
{
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

//-------------------------------------------------------------------------------
void GetInfosCombine()
{
	Entity * io = NULL;

	if(player.bag)
	for(int iNbBag = 0; iNbBag < player.bag; iNbBag++)
	for(size_t j = 0; j < INVENTORY_Y; j++)
	for(size_t i = 0; i < INVENTORY_X; i++) {
		io = inventory[iNbBag][i][j].io;
		GetInfosCombineWithIO(io);
	}

	if(SecondaryInventory) {
		for(long j = 0; j < SecondaryInventory->sizey; j++) {
			for(long i = 0; i < SecondaryInventory->sizex; i++) {
				io = SecondaryInventory->slot[i][j].io;
				GetInfosCombineWithIO(io);
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Switches from/to combat Mode i=-1 for switching from actual configuration
// 2 to force Draw Weapon
// 1 to force Combat Mode On
// 0 to force Combat Mode Off
//-----------------------------------------------------------------------------
void ARX_INTERFACE_Combat_Mode(long i)
{
	arx_assert(arrowobj);
	
	if(i >= 1 && (player.Interface & INTER_COMBATMODE))
		return;

	if(i == 0 && !(player.Interface & INTER_COMBATMODE))
		return;

	if((player.Interface & INTER_COMBATMODE) && entities.player()) {
		player.Interface&=~INTER_COMBATMODE;
		player.Interface&=~INTER_NO_STRIKE;

		ARX_EQUIPMENT_LaunchPlayerUnReadyWeapon();
		WeaponType weapontype = ARX_EQUIPMENT_GetPlayerWeaponType();

		if(entities.player() && (weapontype == WEAPON_BOW)) {
			EERIE_LINKEDOBJ_UnLinkObjectFromObject(entities.player()->obj, arrowobj);
		}

		player.doingmagic=0;
	} else if((!entities.player()->animlayer[1].cur_anim
	           || entities.player()->animlayer[1].cur_anim == entities.player()->anims[ANIM_WAIT])
	          && entities.player()) {
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
void ArxGame::managePlayerControls()
{
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
				float tr=radians(eyeball.angle.getPitch());
				eyeball.pos.x += -std::sin(tr) * 20.f * FD * 0.033f;
				eyeball.pos.z += +std::cos(tr) * 20.f * FD * 0.033f;
				NOMOREMOVES=1;
			}

			// Checks WALK_BACKWARD Key Status.
			if(GInput->actionPressed(CONTROLS_CUST_WALKBACKWARD)) {
				float tr=radians(eyeball.angle.getPitch());
				eyeball.pos.x +=  std::sin(tr) * 20.f * FD * 0.033f;
				eyeball.pos.z += -std::cos(tr) * 20.f * FD * 0.033f;
				NOMOREMOVES=1;
			}

			// Checks STRAFE_LEFT Key Status.
			if( (GInput->actionPressed(CONTROLS_CUST_STRAFELEFT)||
				(GInput->actionPressed(CONTROLS_CUST_STRAFE)&&GInput->actionPressed(CONTROLS_CUST_TURNLEFT)))
				&& !NOMOREMOVES)
			{
				float tr=radians(MAKEANGLE(eyeball.angle.getPitch()+90.f));
				eyeball.pos.x += -std::sin(tr) * 10.f * FD * 0.033f;
				eyeball.pos.z += +std::cos(tr) * 10.f * FD * 0.033f;
				NOMOREMOVES=1;			
			}

			// Checks STRAFE_RIGHT Key Status.
			if( (GInput->actionPressed(CONTROLS_CUST_STRAFERIGHT)||
				(GInput->actionPressed(CONTROLS_CUST_STRAFE)&&GInput->actionPressed(CONTROLS_CUST_TURNRIGHT)))
				&& !NOMOREMOVES)
			{
				float tr=radians(MAKEANGLE(eyeball.angle.getPitch()-90.f));
				eyeball.pos.x += -std::sin(tr) * 10.f * FD * 0.033f;
				//eyeball.pos.y+=FD*0.33f;
				eyeball.pos.z +=  std::cos(tr) * 10.f * FD * 0.033f;
				NOMOREMOVES=1;
			}

			IO_PHYSICS phys;
			phys.cyl.height = -110.f;
			phys.cyl.origin = eyeball.pos + Vec3f(0.f, 70.f, 0.f);
			phys.cyl.radius = 45.f;

			EERIE_CYLINDER test = phys.cyl;
			
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

			float t = radians(player.angle.getPitch());
			multi = 5.f * FD * MoveDiv * multi;
			tm.x += std::sin(t) * multi;
			tm.z -= std::cos(t) * multi;

			if(!USE_PLAYERCOLLISIONS) {
				t=radians(player.angle.getYaw());
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

			float t = radians(player.angle.getPitch());
			multi = 10.f * FD * MoveDiv * multi;
			tm.x -= std::sin(t) * multi;
			tm.z += std::cos(t) * multi;

			if(!USE_PLAYERCOLLISIONS) {
				t=radians(player.angle.getYaw());
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
			float t = radians(MAKEANGLE(player.angle.getPitch()+90.f));
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
			float t = radians(MAKEANGLE(player.angle.getPitch()-90.f));
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
			
			if(spell->m_exist && spell->m_caster == PlayerEntityHandle)
				if(spellicons[spell->m_type].bDuration) {
					ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE);
					spell->m_tolive = 0;
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

	static long lOldInterfaceTemp=0;

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
							Entity * temp = (Entity *)SecondaryInventory->io;

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
		EERIEMouseGrab = 1;
		
	} else if(!PLAYER_MOUSELOOK_ON && LAST_PLAYER_MOUSELOOK_ON) {
		
		EERIEMouseGrab = 0;
		DANAEMouse = MemoMouse;
		
		if(mainApp->getWindow()->isFullScreen()) {
			GInput->setMousePosAbs(DANAEMouse);
		}
		
		bRestoreCoordMouse=false;
	}
	
	LAST_PLAYER_MOUSELOOK_ON=PLAYER_MOUSELOOK_ON;
	PLAYER_ROTATION=0;

	long mouseDiffX = GInput->getMousePosRel().x;
	long mouseDiffY = GInput->getMousePosRel().y;
	
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
					mouseDiffX=10;
				else
					mouseDiffX=-10;

				iAction |= 1;
			}

			if(flPushTimeY[0] || flPushTimeY[1]) {
				if(flPushTimeY[0]<flPushTimeY[1])
					mouseDiffY=10;
				else
					mouseDiffY=-10;

				iAction |= 2;
			}

			if(!(iAction & 1))
				mouseDiffX=0;

			if(!(iAction & 2))
				mouseDiffY=0;
		} else {
			if(bRenderInCursorMode) {
				Vec2s mousePosRel = GInput->getMousePosRel();
				if(DANAEMouse.x == g_size.width() - 1 && mousePosRel.x > 8) {
					mouseDiffY = 0;
					mouseDiffX = mousePosRel.x;
					bKeySpecialMove = true;
				} else if(DANAEMouse.x == 0 && mousePosRel.x < -8) {
					mouseDiffY = 0;
					mouseDiffX = mousePosRel.x;
					bKeySpecialMove = true;
				}
				if(DANAEMouse.y == g_size.height() - 1 && mousePosRel.y > 8) {
					mouseDiffY = mousePosRel.y;
					mouseDiffX = 0;
					bKeySpecialMove = true;
				} else if(DANAEMouse.y == 0 && mousePosRel.y < -8) {
					mouseDiffY = mousePosRel.y;
					mouseDiffX = 0;
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

		float mouseSensitivityY = mouseSensitivity;
		float mouseSensitivityX = mouseSensitivity;

		if(INVERTMOUSE)
			mouseSensitivityY *= -1.f;

		float ia = (float)mouseDiffY * mouseSensitivityY;
		float ib = (float)mouseDiffX * mouseSensitivityX;


		if(PLAYER_MOUSELOOK_ON || bKeySpecialMove) {

			if(eyeball.exist == 2) {
				if(eyeball.angle.getYaw() < 70.f) {
					if(eyeball.angle.getYaw() + ia < 70.f)
						eyeball.angle.setYaw(eyeball.angle.getYaw() + ia);
				} else if(eyeball.angle.getYaw() > 300.f) {
					if(eyeball.angle.getYaw() + ia > 300.f)
						eyeball.angle.setYaw(eyeball.angle.getYaw() + ia);
				}

				eyeball.angle.setYaw(MAKEANGLE(eyeball.angle.getYaw()));
				eyeball.angle.setPitch(MAKEANGLE(eyeball.angle.getPitch() - ib));
			} else if(ARXmenu.currentmode != AMCM_NEWQUEST) {

				float iangle = player.angle.getYaw();

				player.desiredangle.setYaw(player.angle.getYaw());
				player.desiredangle.setYaw(player.desiredangle.getYaw() + ia);
				player.desiredangle.setYaw(MAKEANGLE(player.desiredangle.getYaw()));

				if(player.desiredangle.getYaw() >= 74.9f && player.desiredangle.getYaw() <= 301.f) {
					if(iangle < 75.f)
						player.desiredangle.setYaw(74.9f); //69
					else
						player.desiredangle.setYaw(301.f);
				}

				if(entities.player() && EEfabs(ia)>2.f)
					entities.player()->animBlend.lastanimtime = 0;

				if(ib != 0.f)
					player.Current_Movement|=PLAYER_ROTATE;

				PLAYER_ROTATION = ib;

				player.desiredangle.setPitch(player.angle.getPitch());
				player.desiredangle.setPitch(MAKEANGLE(player.desiredangle.getPitch() - ib));
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

static float fDecPulse;
//-----------------------------------------------------------------------------

void ARX_INTERFACE_RELEASESOUND()
{
	ARX_SOUND_PlayInterface(SND_MENU_RELEASE);
}

//-----------------------------------------------------------------------------
void ARX_INTERFACE_ERRORSOUND()
{
	ARX_SOUND_PlayInterface(SND_MENU_CLICK);
}

//-----------------------------------------------------------------------------
bool CheckAttributeClick(float x, float y, float * val, TextureContainer * tc)
{
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

//-----------------------------------------------------------------------------
bool CheckSkillClick(float x, float y, float * val, TextureContainer * tc, float * oldval)
{
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
				if(necklace.runes[i]) {
					bookcam.center.x = (382 + xpos * 45 + BOOKDEC.x) * g_sizeRatio.x;
					bookcam.center.y = (100 + ypos * 64 + BOOKDEC.y) * g_sizeRatio.y;

					SetActiveCamera(&bookcam);
					PrepareCamera(&bookcam, g_size);

					// First draw the lace
					angle.setPitch(0.f);

					if(player.rune_flags & (RuneFlag)(1<<i)) {
						
						TransformInfo t1(pos, glm::toQuat(toRotationMatrix(angle)));
						DrawEERIEInter(necklace.lacet, t1, NULL);

						if(necklace.runes[i]->angle.getPitch() != 0.f) {
							if(necklace.runes[i]->angle.getPitch() > 300.f)
								necklace.runes[i]->angle.setPitch(300.f);

							angle.setPitch(std::sin(arxtime.get_updated() * (1.0f / 200)) * necklace.runes[i]->angle.getPitch() * (1.0f / 40));
						}

						necklace.runes[i]->angle.setPitch(necklace.runes[i]->angle.getPitch() - framedelay * 0.2f);

						if(necklace.runes[i]->angle.getPitch() < 0.f)
							necklace.runes[i]->angle.setPitch(0.f);

						GRenderer->SetRenderState(Renderer::DepthWrite, true);
						GRenderer->SetRenderState(Renderer::AlphaBlending, false);
						
						// Now draw the rune
						TransformInfo t2(pos, glm::toQuat(toRotationMatrix(angle)));
						DrawEERIEInter(necklace.runes[i], t2, NULL);

						EERIE_2D_BBOX runeBox;
						UpdateBbox2d(*necklace.runes[i], runeBox);

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

							for(size_t j = 0; j < necklace.runes[i]->facelist.size(); j++) {
								n=PtIn2DPolyProj(necklace.runes[i], &necklace.runes[i]->facelist[j], (float)DANAEMouse.x, (float)DANAEMouse.y);

								if(n!=0.f) {
									r=1;
									break;
								}
							}

							if(r) {
								GRenderer->SetRenderState(Renderer::AlphaBlending, true);
								GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
								
								TransformInfo t(pos, glm::toQuat(toRotationMatrix(angle)));
								DrawEERIEInter(necklace.runes[i], t, NULL);

								necklace.runes[i]->angle.setPitch(necklace.runes[i]->angle.getPitch() + framedelay*2.f);

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
									DrawBookInterfaceItem(necklace.pTexTab[spellicons[i].symbols[j]], Vec2f(pos));
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
								color = Color::fromBGRA(0xFFa8d0df);
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

void ARX_INTERFACE_ManageOpenedBook()
{
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
		float x = 0;
		
		arx_assert(ITC.playerbook);
		x = static_cast<float>((640 - ITC.playerbook->m_dwWidth) / 2);
		float y = static_cast<float>((480 - ITC.playerbook->m_dwHeight) / 2);
		
		DrawBookInterfaceItem(ITC.playerbook, Vec2f(x, y));

		BOOKDEC.x = x - 97;
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
				GRenderer->SetViewport(vp);
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
			GRenderer->SetViewport(vp);

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
			GRenderer->SetViewport(rec);

			ePlayerAngle.setPitch(-20.f);
			pos.x=20.f;
			pos.y=96.f;
			pos.z=260.f;

			ARX_EQUIPMENT_AttachPlayerWeaponToHand();
		}

		long ti=Project.improve;
		Project.improve=0;


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

		Project.improve=ti;

		GRenderer->SetViewport(Rect(g_size.width(), g_size.height()));

		GRenderer->SetRenderState(Renderer::AlphaBlending, false);
		GRenderer->SetCulling(Renderer::CullNone);
		SetActiveCamera(oldcam);
		PrepareCamera(oldcam, g_size);

		Entity *io = entities.player();

		if(io) {
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
						ARX_INTERFACE_HALO_Draw(
							todraw, tc, tc2,
							(todraw->bbox2D.min.x + BOOKDEC.x) * g_sizeRatio.x,
							(todraw->bbox2D.min.y + BOOKDEC.y) * g_sizeRatio.y,
							g_sizeRatio.x, g_sizeRatio.y
						);
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
						ARX_INTERFACE_HALO_Draw(
							todraw, tc, tc2, 
							(todraw->bbox2D.min.x + BOOKDEC.x) * g_sizeRatio.x,
							(todraw->bbox2D.min.y + BOOKDEC.x) * g_sizeRatio.y,
							g_sizeRatio.x, g_sizeRatio.y
						);
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
}


extern float GLOBAL_SLOWDOWN;

/** EXTRACTION BEGINS HERE **/

/*!
 * \brief the hit strength diamond shown at the bottom of the UI.
 */
class HitStrengthGauge {
private:
	TextureContainer * m_emptyTex;
	TextureContainer * m_fullTex;
	TextureContainer * m_hitTex;
	
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
	}
	
	void requestFlash(float flashIntensity) {
		bHitFlash = true;
		ulHitFlash = 0;
		m_flashIntensity = flashIntensity;
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
				if(at > 0.f)
					bIsAiming = true;
				else
					bIsAiming = false;
	
				at=at*(1.f+(1.f-GLOBAL_SLOWDOWN));
				float aim = static_cast<float>(player.Full_AimTime);
				j=at/aim;
			}
			m_intensity = clamp(j, 0.2f, 1.f);
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
		Vec2f pos = Vec2f(g_size.bottomCenter());
		pos += Vec2f(-320 + 262.f, -72.f);
		
		GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
	
		GRenderer->SetRenderState(Renderer::AlphaBlending, true);
		ARX_INTERFACE_DrawItem(m_fullTex, pos.x, pos.y, 0.0001f, Color3f::gray(m_intensity).to<u8>());
		GRenderer->SetRenderState(Renderer::AlphaBlending, false);
		ARX_INTERFACE_DrawItem(m_emptyTex, pos.x, pos.y, 0.0001f, Color::white);
		
		if(bHitFlash && player.m_skillFull.etheralLink >= 40) {
			Vec2f flashPos = pos;
			flashPos += Vec2f(-25, -30);
			
			float j = 1.0f - m_flashIntensity;
			GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
			GRenderer->SetRenderState(Renderer::AlphaBlending, true);
			
			Color col = (j < 0.5f) ? Color3f(j*2.0f, 1, 0).to<u8>() : Color3f(1, m_flashIntensity, 0).to<u8>();
			ARX_INTERFACE_DrawItem(m_hitTex, flashPos.x, flashPos.y, 0.0001f, col);
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
	TextureContainer * m_tex;
	
public:
	void init() {
		m_tex = TextureContainer::LoadUI("graph/interface/icons/cant_steal_item");
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
			
			float maxDist = Project.telekinesis ? 900.f : 350.f;
			
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
		if(TSecondaryInventory) {
			bool _bSteal = (bool)((player.Interface & INTER_STEAL) != 0);
			
			if(TSecondaryInventory->io && !TSecondaryInventory->io->inventory_skin.empty()) {
				
				res::path file = "graph/interface/inventory" / TSecondaryInventory->io->inventory_skin;
				
				TextureContainer * tc = TextureContainer::LoadUI(file);
		
				if(tc) {
					ingame_inventory = tc;
				} else {
					ingame_inventory = BasicInventorySkin;
				}
				
			} else if(ingame_inventory != BasicInventorySkin) {
				ingame_inventory = BasicInventorySkin;
			}
		
			ARX_INTERFACE_DrawItem(ingame_inventory, INTERFACE_RATIO(InventoryX), 0.f);
		
			for(long j = 0; j < TSecondaryInventory->sizey; j++) {
				for(long i = 0; i < TSecondaryInventory->sizex; i++) {
					Entity *io = TSecondaryInventory->slot[i][j].io;
		
					if(io) {
						bool bItemSteal = false;
						TextureContainer *tc = io->inv;
						TextureContainer *tc2 = NULL;
		
						if(NeedHalo(io))
							tc2 = io->inv->getHalo();
		
						if(_bSteal) {
							if(!ARX_PLAYER_CanStealItem(io)) {
								bItemSteal = true;
								tc = m_tex;
								tc2 = NULL;
							}
						}
		
						if(tc && (TSecondaryInventory->slot[i][j].show || bItemSteal)) {
							UpdateGoldObject(io);
		
							float px = INTERFACE_RATIO(InventoryX) + (float)i*INTERFACE_RATIO(32) + INTERFACE_RATIO(2);
							float py = (float)j*INTERFACE_RATIO(32) + INTERFACE_RATIO(13);
							
							Vec2f size = Vec2f(tc->size());
							
							Color color = (io->poisonous && io->poisonous_count!=0) ? Color::green : Color::white;
							
							Rectf rect(
								Vec2f(px, py),
								size.x,
								size.y
							);
							EERIEDrawBitmap(rect, 0.001f, tc, color);
							
							if (!bItemSteal && (io==FlyingOverIO))
							{
								GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
								GRenderer->SetRenderState(Renderer::AlphaBlending, true);
								
								Rectf rect(
									Vec2f(px, py),
									size.x,
									size.y
								);
								EERIEDrawBitmap(rect, 0.001f, tc, Color::white);
								
								GRenderer->SetRenderState(Renderer::AlphaBlending, false);
							}
							else if(!bItemSteal && (io->ioflags & IO_CAN_COMBINE)) {
								GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
								GRenderer->SetRenderState(Renderer::AlphaBlending, true);
								
								float fColorPulse = fabs(cos(radians(fDecPulse)));
								
								Rectf rect(
									Vec2f(px, py),
									size.x,
									size.y
								);
								EERIEDrawBitmap(rect, 0.001f, tc, Color3f::gray(fColorPulse).to<u8>());
								
								GRenderer->SetRenderState(Renderer::AlphaBlending, false);
							}
		
							if(tc2) {
								ARX_INTERFACE_HALO_Draw(io, tc, tc2, px, py, INTERFACE_RATIO(1), INTERFACE_RATIO(1));
							}
		
							if((io->ioflags & IO_ITEM) && io->_itemdata->count != 1)
								ARX_INTERFACE_DrawNumber(Vec2f(px, py), io->_itemdata->count, 3, Color::white);
						}
					}
				}
			}
		}
	}
};
SecondaryInventoryGui secondaryInventory;


class InventoryGui {
private:
	TextureContainer * m_heroInventory;
	TextureContainer * m_heroInventoryLink;
	TextureContainer * m_heroInventoryUp;
	TextureContainer * m_heroInventoryDown;
	
	float fCenterX;
	float fSizY;
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
		float fCenterX	= g_size.center().x + INTERFACE_RATIO(-320 + 35) + INTERFACE_RATIO_DWORD(m_heroInventory->m_dwWidth) - INTERFACE_RATIO(32 + 3) ;
		float fSizY		= g_size.height() - INTERFACE_RATIO(101) + INTERFACE_RATIO_LONG(InventoryY) + INTERFACE_RATIO(- 3 + 25) ;

		float posx = ARX_CAST_TO_INT_THEN_FLOAT( fCenterX );
		float posy = ARX_CAST_TO_INT_THEN_FLOAT( fSizY );

		bool bQuitCombine = true;
		
		if(sActiveInventory > 0) {
			const Rect mouseTestRect(
			posx,
			posy,
			posx + INTERFACE_RATIO(32),
			posy + INTERFACE_RATIO(32)
			);
			
			if(mouseTestRect.contains(Vec2i(DANAEMouse)))
				bQuitCombine = false;
		}

		if(sActiveInventory < player.bag-1) {
			float fRatio = INTERFACE_RATIO(32 + 5);

			posy += checked_range_cast<int>(fRatio);
			
			const Rect mouseTestRect(
			posx,
			posy,
			posx + INTERFACE_RATIO(32),
			posy + INTERFACE_RATIO(32)
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
		
		fCenterX = g_size.center().x + INTERFACE_RATIO(-320 + 35) + INTERFACE_RATIO_DWORD(m_heroInventory->m_dwWidth) - INTERFACE_RATIO(32 + 3) ;
		fSizY = g_size.height() - INTERFACE_RATIO(101) + INTERFACE_RATIO_LONG(InventoryY) + INTERFACE_RATIO(- 3 + 25) ;
		m_pos.x = ARX_CAST_TO_INT_THEN_FLOAT( fCenterX );
		m_pos.y = ARX_CAST_TO_INT_THEN_FLOAT( fSizY );
	}
	
	//-----------------------------------------------------------------------------
	void ARX_INTERFACE_DrawInventory(short _sNum, int _iX=0, int _iY=0)
	{
		fDecPulse += framedelay * 0.5f;
	
		float fCenterX	= g_size.center().x - INTERFACE_RATIO(320) + INTERFACE_RATIO(35) + _iX ;
		float fSizY		= g_size.height() - INTERFACE_RATIO(101) + INTERFACE_RATIO_LONG(InventoryY) + _iY;
	
		float fPosX = ARX_CAST_TO_INT_THEN_FLOAT( fCenterX );
		float fPosY = ARX_CAST_TO_INT_THEN_FLOAT( fSizY );
	
		ARX_INTERFACE_DrawItem(m_heroInventory, fPosX, fPosY - INTERFACE_RATIO(5));
	
		for(size_t j = 0; j < INVENTORY_Y; j++) {
			for(size_t i = 0; i < INVENTORY_X; i++) {
				Entity *io = inventory[_sNum][i][j].io;
	
				if(io && inventory[_sNum][i][j].show) {
					TextureContainer *tc = io->inv;
					TextureContainer *tc2 = NULL;
	
					if(NeedHalo(io))
						tc2 = io->inv->getHalo();
	
					if(tc) {
						float px = fPosX + i*m_slotSize.x + m_slotSpacing.x;
						float py = fPosY + j*m_slotSize.y + m_slotSpacing.y;
						
						Color color = (io->poisonous && io->poisonous_count != 0) ? Color::green : Color::white;
						
						Rectf rect(
							Vec2f(px, py),
							tc->m_dwWidth,
							tc->m_dwHeight
						);
						EERIEDrawBitmap(rect, 0.001f, tc, color);
						
						if(io == FlyingOverIO) {
							GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
							GRenderer->SetRenderState(Renderer::AlphaBlending, true);
							
							Rectf rect(
								Vec2f(px, py),
								tc->m_dwWidth,
								tc->m_dwHeight
							);
							EERIEDrawBitmap(rect, 0.001f, tc, Color::white);
							GRenderer->SetRenderState(Renderer::AlphaBlending, false);
						} else if(io->ioflags & IO_CAN_COMBINE) {
							float fColorPulse = fabs(cos(radians(fDecPulse)));
							GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
							GRenderer->SetRenderState(Renderer::AlphaBlending, true);
							
							Rectf rect(
								Vec2f(px, py),
								tc->m_dwWidth,
								tc->m_dwHeight
							);
							EERIEDrawBitmap(rect, 0.001f, tc, Color3f::gray(fColorPulse).to<u8>());
							GRenderer->SetRenderState(Renderer::AlphaBlending, false);
						}
	
						if(tc2) {
							ARX_INTERFACE_HALO_Render(
								io->halo.color.r, io->halo.color.g, io->halo.color.b,
								io->halo.flags,
								tc2,
								px,
								py, INTERFACE_RATIO(1), INTERFACE_RATIO(1));
						}
	
						if((io->ioflags & IO_ITEM) && io->_itemdata->count != 1)
							ARX_INTERFACE_DrawNumber(Vec2f(px, py), io->_itemdata->count, 3, Color::white);
					}
				}
			}
		}
	}
	
	void draw() {
		if(player.Interface & INTER_INVENTORY) {		
			if(player.bag) {
				ARX_INTERFACE_DrawInventory(sActiveInventory);
				
				CalculateInventoryCoordinates();
				
				if(sActiveInventory > 0) {
					ARX_INTERFACE_DrawItem(m_heroInventoryUp, m_pos.x, m_pos.y);
					
					const Rect inventoryUpMouseTestRect(
					m_pos.x,
					m_pos.y,
					m_pos.x + INTERFACE_RATIO(32),
					m_pos.y + INTERFACE_RATIO(32)
					);
					
					if(inventoryUpMouseTestRect.contains(Vec2i(DANAEMouse))) {
						GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
						GRenderer->SetRenderState(Renderer::AlphaBlending, true);
						SpecialCursor=CURSOR_INTERACTION_ON;
						ARX_INTERFACE_DrawItem(m_heroInventoryUp, m_pos.x, m_pos.y);
						GRenderer->SetRenderState(Renderer::AlphaBlending, false);
						SpecialCursor=CURSOR_INTERACTION_ON;
						
						if (((EERIEMouseButton & 1)  && !(LastMouseClick & 1))
							|| ((!(EERIEMouseButton & 1)) && (LastMouseClick & 1) && DRAGINTER))
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
					float fRatio = INTERFACE_RATIO(32 + 5);
					
					ARX_INTERFACE_DrawItem(m_heroInventoryDown, m_pos.x, g_size.height() - INTERFACE_RATIO(101) + INTERFACE_RATIO_LONG(InventoryY) + INTERFACE_RATIO(-3 + 64));
					
					const Rect inventoryDownMouseTestRect(
					m_pos.x,
					m_pos.y + fRatio,
					m_pos.x + INTERFACE_RATIO(32),
					m_pos.y + fRatio + INTERFACE_RATIO(32)
					);
					
					if(inventoryDownMouseTestRect.contains(Vec2i(DANAEMouse))) {
						GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
						GRenderer->SetRenderState(Renderer::AlphaBlending, true);
						ARX_INTERFACE_DrawItem(m_heroInventoryDown, m_pos.x, g_size.height() - INTERFACE_RATIO(101) + INTERFACE_RATIO_LONG(InventoryY) + INTERFACE_RATIO(-3 + 64));
						GRenderer->SetRenderState(Renderer::AlphaBlending, false);
						SpecialCursor=CURSOR_INTERACTION_ON;
						
						if (((EERIEMouseButton & 1)  && !(LastMouseClick & 1))
							|| ((!(EERIEMouseButton & 1)) && (LastMouseClick & 1) && DRAGINTER))
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
			
			//TODO see about these coords, might be calculated once only
			const float fBag = (player.bag-1) * INTERFACE_RATIO(-121);
			float fCenterX = g_size.center().x + INTERFACE_RATIO(-320+35);
			float fSizY = g_size.height() - INTERFACE_RATIO(101) + INTERFACE_RATIO_LONG(InventoryY) + INTERFACE_RATIO(-3.f + 25 - 32);
			const float fOffsetY = INTERFACE_RATIO(121);
			
			int iOffsetY = checked_range_cast<int>(fBag + fOffsetY);
			int posx = checked_range_cast<int>(fCenterX);
			int posy = checked_range_cast<int>(fSizY);
			
			for(int i = 0; i < player.bag; i++) {
				ARX_INTERFACE_DrawItem(m_heroInventoryLink, posx + INTERFACE_RATIO(45), static_cast<float>(posy + iOffsetY)) ;
				
				ARX_INTERFACE_DrawItem(m_heroInventoryLink, posx + INTERFACE_RATIO_DWORD(m_heroInventory->m_dwWidth)*0.5f + INTERFACE_RATIO(-16), posy+iOffsetY + INTERFACE_RATIO(-5));
				ARX_INTERFACE_DrawItem(m_heroInventoryLink, posx + INTERFACE_RATIO_DWORD(m_heroInventory->m_dwWidth) + INTERFACE_RATIO(-45-32), posy+iOffsetY + INTERFACE_RATIO(-15));
				
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



void DrawItemPrice() {
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


class HudIconBase {
protected:
	void DrawHalo(float r, float g, float b, TextureContainer* halo, const Vec2f& coords) {
		if(halo) {
			ARX_INTERFACE_HALO_Render(r, g, b, HALO_ACTIVE, halo, coords.x, coords.y);
		}
	}
	
	//Used for drawing icons like the book or backpack icon.
	void DrawIcon(const Vec2f& coords, TextureContainer * tex, E_ARX_STATE_MOUSE hoverMouseState) {
		
		ARX_INTERFACE_DrawItem(tex, coords.x, coords.y);
		if (eMouseState == hoverMouseState) {
			GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
			GRenderer->SetRenderState(Renderer::AlphaBlending, true);
			ARX_INTERFACE_DrawItem(tex, coords.x, coords.y);
			GRenderer->SetRenderState(Renderer::AlphaBlending, false);
		}
	}
};

extern TextureContainer * healing;

class BookIconGui : public HudIconBase {
private:
	TextureContainer * m_tex;
	Vec2f m_pos;
	
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
	
public:
	void init() {
		m_tex = TextureContainer::LoadUI("graph/interface/icons/book");
		arx_assert(m_tex);
	}
	
	void requestFX() {
		MakeBookFX(Vec3f(Vec2f(g_size.bottomRight()) + Vec2f(-35, -148), 0.00001f));
	}
	
	void update() {
		m_pos = g_size.bottomRight();
		m_pos += Vec2f(-35, -148);
		m_pos.x += lSLID_VALUE;
	}
	
	void updateInput() {
		// book
		{
		Vec2f pos(g_size.width() - 35 + lSLID_VALUE, g_size.height() - 148);

		const Rect bookMouseTestRect(
		pos.x,
		pos.y,
		pos.x + INTERFACE_RATIO(32),
		pos.y + INTERFACE_RATIO(32)
		);
		
		if(bookMouseTestRect.contains(Vec2i(DANAEMouse))) {
			eMouseState = MOUSE_IN_BOOK_ICON;
			SpecialCursor = CURSOR_INTERACTION_ON;

			if((EERIEMouseButton & 1) && !(LastMouseClick & 1)) {
				ARX_INTERFACE_BookOpenClose(0);
				EERIEMouseButton &=~1;
			}
			return;
		}
		}
	}
	
	void draw() {
		DrawIcon(m_pos, m_tex, MOUSE_IN_BOOK_ICON);
	}
	
	void drawHalo() {
		if(bBookHalo) {
			float fCalc = ulBookHaloTime + Original_framedelay;
			ulBookHaloTime = checked_range_cast<unsigned long>(fCalc);
			if(ulBookHaloTime >= 3000) { // ms
				bBookHalo = false;
			}
			DrawHalo(0.2f, 0.4f, 0.8f, m_tex->getHalo(), m_pos);
		}
	}
};

static BookIconGui bookIconGui;

void bookIconGuiRequestFX() {
	bookIconGui.requestFX();
}

class BackpackIconGui : public HudIconBase {
private:
	TextureContainer * m_tex;
	Vec2f m_pos;
	
public:
	void init() {
		m_tex = TextureContainer::LoadUI("graph/interface/icons/backpack");
		arx_assert(m_tex);
	}

	void update() {
		m_pos = g_size.bottomRight();
		m_pos += Vec2f(-35, -113);
		m_pos.x += lSLID_VALUE;
	}
	
	void updateInput() {
		{
		// inventaire
		Vec2f pos(g_size.width() - (35) + lSLID_VALUE, g_size.height() - 113);
		static float flDelay=0;
		
		const Rect inventoryMouseTestRect(
		pos.x,
		pos.y,
		pos.x + INTERFACE_RATIO(32),
		pos.y + INTERFACE_RATIO(32)
		);
		
		if(inventoryMouseTestRect.contains(Vec2i(DANAEMouse)) || flDelay) {
			eMouseState = MOUSE_IN_INVENTORY_ICON;
			SpecialCursor = CURSOR_INTERACTION_ON;

			if(EERIEMouseButton & 4) {
				ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());

				playerInventory.optimize();

				flDelay=0;
				EERIEMouseButton&=~4;
			} else if(((EERIEMouseButton & 1) && !(LastMouseClick & 1)) || flDelay) {
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
			} else if((EERIEMouseButton & 2) && !(LastMouseClick & 2)) {
				ARX_INTERFACE_BookOpenClose(2);
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
		DrawIcon(m_pos, m_tex, MOUSE_IN_INVENTORY_ICON);
	}
};

static BackpackIconGui backpackIconGui;


class StealIconGui : public HudIconBase {
private:
	TextureContainer * m_tex;
	Vec2f m_pos;
public:
	void init() {
		m_tex = TextureContainer::LoadUI("graph/interface/icons/steal");
		arx_assert(m_tex);
	}
	
	void update() {
		m_pos.x = static_cast<float>(-lSLID_VALUE);
		m_pos.y = g_size.height() - INTERFACE_RATIO(78.f + 32);
	}
	
	void updateInput() {
		
		// steal
		if(player.Interface & INTER_STEAL) {
			Vec2f pos(static_cast<float>(-lSLID_VALUE), g_size.height() - (78 + 32));
			
			const Rect mouseTestRect(
			pos.x,
			pos.y,
			pos.x + INTERFACE_RATIO(32),
			pos.y + INTERFACE_RATIO(32)
			);
			
			if(mouseTestRect.contains(Vec2i(DANAEMouse))) {
				eMouseState=MOUSE_IN_STEAL_ICON;
				SpecialCursor=CURSOR_INTERACTION_ON;

				if((EERIEMouseButton & 1) && !(LastMouseClick & 1)) {
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
		DrawIcon(m_pos, m_tex, MOUSE_IN_STEAL_ICON);
	}
};

static StealIconGui stealIconGui;

class PickAllIconGui : public HudIconBase {
private:
	TextureContainer * m_tex;
	Vec2f m_pos;
public:
	void init() {
		m_tex = TextureContainer::LoadUI("graph/interface/inventory/inv_pick");
	}
	
	void update() {
		m_pos.x = INTERFACE_RATIO(InventoryX);
		m_pos.y = INTERFACE_RATIO_DWORD(BasicInventorySkin->m_dwHeight);
		m_pos += Vec2f(16, -16);
	}
	
	void updateInput() {
		Vec2f pos(InventoryX + 16, BasicInventorySkin->m_dwHeight - 16);
		
		const Rect mouseTestRect(
		pos.x,
		pos.y,
		pos.x + INTERFACE_RATIO(16),
		pos.y + INTERFACE_RATIO(16)
		);
		
		if(mouseTestRect.contains(Vec2i(DANAEMouse))) {
			eMouseState = MOUSE_IN_INVENTORY_PICKALL_ICON;
			SpecialCursor=CURSOR_INTERACTION_ON;

			if((EERIEMouseButton & 1) && !(LastMouseClick & 1)) {
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
	
	void draw() {
		DrawIcon(m_pos, m_tex, MOUSE_IN_INVENTORY_PICKALL_ICON);
	}
};

static PickAllIconGui pickAllIconGui;

class CloseSecondaryInventoryIconGui : public HudIconBase {
private:
	TextureContainer * m_tex;
	Vec2f m_pos;
public:
	void init() {
		m_tex = TextureContainer::LoadUI("graph/interface/inventory/inv_close");
		arx_assert(m_tex);
	}
	
	void update() {
		m_pos.x = INTERFACE_RATIO(InventoryX) + INTERFACE_RATIO_DWORD(BasicInventorySkin->m_dwWidth);
		m_pos.y = INTERFACE_RATIO_DWORD(BasicInventorySkin->m_dwHeight);
		m_pos += Vec2f(-32, -16);
	}
	
	void updateInput() {
		Vec2f pos(InventoryX + BasicInventorySkin->m_dwWidth - 32, BasicInventorySkin->m_dwHeight - 16);

		const Rect mouseTestRect(
		pos.x,
		pos.y,
		pos.x + INTERFACE_RATIO(16),
		pos.y + INTERFACE_RATIO(16)
		);
		
		if(mouseTestRect.contains(Vec2i(DANAEMouse))) {
			eMouseState = MOUSE_IN_INVENTORY_CLOSE_ICON;
			SpecialCursor=CURSOR_INTERACTION_ON;

			if((EERIEMouseButton & 1) && !(LastMouseClick & 1)) {
				Entity * io = NULL;

				if(SecondaryInventory)
					io = (Entity *)SecondaryInventory->io;
				else if (player.Interface & INTER_STEAL)
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
	
	void draw() {
		DrawIcon(m_pos, m_tex, MOUSE_IN_INVENTORY_CLOSE_ICON);
	}
	
};

static CloseSecondaryInventoryIconGui closeSecondaryInventoryIconGui;

class LevelUpIconGui : public HudIconBase {
private:
	TextureContainer * m_tex;
	Vec2f m_pos;
public:
	void init() {
		m_tex = TextureContainer::LoadUI("graph/interface/icons/lvl_up");
		arx_assert(m_tex);
	}
	
	void updateInput() {
		
		// redist
		if((player.Skill_Redistribute) || (player.Attribute_Redistribute)) {
			Vec2f pos(g_size.width() - 35 + lSLID_VALUE, g_size.height() - INTERFACE_RATIO(218));
			
			const Rect mouseTestRect(
			pos.x,
			pos.y,
			pos.x + INTERFACE_RATIO(32),
			pos.y + INTERFACE_RATIO(32)
			);

			if(mouseTestRect.contains(Vec2i(DANAEMouse))) {
				eMouseState = MOUSE_IN_REDIST_ICON;
				SpecialCursor = CURSOR_INTERACTION_ON;

				if((EERIEMouseButton & 1) && !(LastMouseClick & 1)) {
					ARX_INTERFACE_BookOpenClose(1);
					EERIEMouseButton &=~1;
				}
				return;
			}
		}
	}
	
	void update() {
		m_pos = g_size.bottomRight();
		m_pos += Vec2f(-35, -218);
		m_pos.x += lSLID_VALUE;
	}
	
	void draw() {
		DrawIcon(m_pos, m_tex, MOUSE_IN_REDIST_ICON);
	}
};

LevelUpIconGui levelUpIconGui;


class PurseIconGui : public HudIconBase {
private:
	TextureContainer * m_tex;
	Vec2f m_pos;
	
public:
	void init() {
		m_tex = TextureContainer::LoadUI("graph/interface/inventory/gold");
		arx_assert(m_tex);
	}
	
	void update() {
		m_pos = g_size.bottomRight();
		m_pos += Vec2f(-35, -183);
		m_pos.x += lSLID_VALUE;
		m_pos.x += 2;
	}
	
	void updateInput() {
		// gold
		if(player.gold > 0) {
			Vec2f pos(g_size.width() - (35) + lSLID_VALUE, g_size.height() - 183);
			
			const Rect mouseTestRect(
			pos.x,
			pos.y,
			pos.x + INTERFACE_RATIO(32),
			pos.y + INTERFACE_RATIO(32)
			);
			
			if(mouseTestRect.contains(Vec2i(DANAEMouse))) {
				eMouseState = MOUSE_IN_GOLD_ICON;
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
		DrawIcon(m_pos, m_tex, MOUSE_IN_GOLD_ICON);
		if(eMouseState == MOUSE_IN_GOLD_ICON) {
			SpecialCursor=CURSOR_INTERACTION_ON;
			
			Vec2f numberPos = m_pos;
			numberPos += Vec2f(- INTERFACE_RATIO(30), + INTERFACE_RATIO(10 - 25));
			
			ARX_INTERFACE_DrawNumber(numberPos, player.gold, 6, Color::white);
		}
	}
	
	void drawHalo() {
		//A halo is drawn on the character's stats icon (book) when leveling up, for example.
		if(bGoldHalo) {
			float fCalc = ulGoldHaloTime + Original_framedelay;
			ulGoldHaloTime = checked_range_cast<unsigned long>(fCalc);
			if(ulGoldHaloTime >= 1000) { // ms
				bGoldHalo = false;
			}
			DrawHalo(0.9f, 0.9f, 0.1f, m_tex->getHalo(), m_pos);
		}
	}
};

static PurseIconGui purseIconGui;

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

				if((LastMouseClick & 1) && !(EERIEMouseButton & 1)) {
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
					Set_DragInter(io);
					DRAGINTER->ignition=1;
				} else {
					if((EERIEMouseButton & 4) && !COMBINE) {
						COMBINE = player.torch;
					}

					if(!(EERIEMouseButton & 2) && (LastMouseClick & 2)) {
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
		
		PARTICLE_DEF * pd = createParticle();
		if(!pd) {
			return;
		}
		
		pd->special = FIRE_TO_SMOKE;
		pd->ov = Vec3f(px + INTERFACE_RATIO(12.f - rnd() * 3.f),
		               py + INTERFACE_RATIO(rnd() * 6.f), 0.0000001f);
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

void DrawIcons() {
	if(player.Interface & INTER_MINIBACK) {
		bookIconGui.update();
		bookIconGui.draw();
		
		backpackIconGui.update();
		backpackIconGui.draw();
						
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

		if(player.Skill_Redistribute || player.Attribute_Redistribute) {
			levelUpIconGui.update();
			levelUpIconGui.draw();
		}
		// Draw/Manage Gold Purse Icon
		if(player.gold > 0) {
			purseIconGui.update();
			purseIconGui.draw();
		}
		//A halo is drawn on the character's stats icon (book) when leveling up, for example.
		purseIconGui.drawHalo();
		bookIconGui.drawHalo();
	}
	if(player.torch) {
		currentTorchIconGui.update();
		currentTorchIconGui.draw();
	}
}

class ChangeLevelIconGui{
private:
	TextureContainer * m_tex;
	Vec2f m_pos;
	Rectf m_rect;
	float m_intensity;
	
public:
	void init() {
		m_tex = TextureContainer::LoadUI("graph/interface/icons/change_lvl");
		arx_assert(m_tex);
	}
	
	void update() {
		m_pos = g_size.topRight();
		m_pos.x -= m_tex->m_dwWidth;
		
		m_rect = Rectf(
		m_pos.x,
		m_pos.y,
		m_pos.x + INTERFACE_RATIO_DWORD(m_tex->m_dwWidth),
		m_pos.y + INTERFACE_RATIO_DWORD(m_tex->m_dwHeight)
		);

		m_intensity = 0.9f - std::sin(arxtime.get_frame_time()*( 1.0f / 50 ))*0.5f+rnd()*( 1.0f / 10 );
		m_intensity = clamp(m_intensity, 0.f, 1.f);
	}
	
	void draw() {
		EERIEDrawBitmap(m_rect, 0.0001f, m_tex, Color3f::gray(m_intensity).to<u8>());
		
	    if(m_rect.contains(Vec2f(DANAEMouse))) {
			SpecialCursor=CURSOR_INTERACTION_ON;
			if(!(EERIEMouseButton & 1) && (LastMouseClick & 1)) {
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

static void hideQuickSaveIcon() {
	quickSaveIconGui.hide();
}

class MemorizedSpellIconsGui : public HudIconBase {
private:
	int m_count;
	Vec2f m_pos;
	
public:
	void update() {
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
		m_pos.x = g_size.width() - (m_count * INTERFACE_RATIO(32));
		if(CHANGE_LEVEL_ICON > -1) {
			m_pos.x -= INTERFACE_RATIO(32);
		}
		m_pos.y = 0;
	}
	
	void draw() {
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
				EERIEDrawBitmap2(Rectf(m_pos, 32, 32), 0,
					necklace.pTexTab[player.SpellToMemorize.iSpellSymbols[i]], Color::white);
				if(bHalo) {				
					TextureContainer *tc = necklace.pTexTab[player.SpellToMemorize.iSpellSymbols[i]];
					DrawHalo(0.2f, 0.4f, 0.8f, tc->getHalo(), Vec2f(m_pos.x, m_pos.y));
				}
				if(!(player.rune_flags & (RuneFlag)(1<<player.SpellToMemorize.iSpellSymbols[i]))) {
					GRenderer->SetBlendFunc(Renderer::BlendInvDstColor, Renderer::BlendOne);
					GRenderer->SetRenderState(Renderer::AlphaBlending, true);
					EERIEDrawBitmap2(Rectf(m_pos, 32, 32), 0, Movable, Color3f::gray(.8f).to<u8>());
					GRenderer->SetRenderState(Renderer::AlphaBlending, false);
				}
				m_pos.x += INTERFACE_RATIO(32);
			}
		}
		if(float(arxtime) - player.SpellToMemorize.lTimeCreation > 30000) {
			player.SpellToMemorize.bSpell = false;
		}
	}
};


MemorizedSpellIconsGui memorizedSpellIconsGui;

class HealthGauge {
private:
	TextureContainer * m_emptyTex;
	TextureContainer * m_filledTex;
	float m_amount;
public:
	void init() {
		m_emptyTex = TextureContainer::LoadUI("graph/interface/bars/empty_gauge_red");
		m_filledTex = TextureContainer::LoadUI("graph/interface/bars/filled_gauge_red");
		arx_assert(m_emptyTex);
		arx_assert(m_filledTex);
	}
	
	void update() {
		
		m_amount = (float)player.lifePool.current/(float)player.Full_maxlife;
	}
	
	void draw() {
		
		Color ulcolor = Color::red;
		
		if(player.poison > 0.f) {
			float val = std::min(player.poison, 0.2f) * 255.f * 5.f;
			long g = val;
			ulcolor = Color(u8(255 - g), u8(g) , 0);
		}
		
		Vec2f pos = Vec2f(g_size.bottomLeft());
		pos += Vec2f(0, -78);
		pos.x += static_cast<float>(-lSLID_VALUE);
		
		Vec2f size = Vec2f(m_filledTex->size());

		EERIEDrawBitmap2DecalY(pos.x, pos.y, size.x, size.y,
			0.f, m_filledTex, ulcolor, (1.f - m_amount));
		
		ARX_INTERFACE_DrawItem(m_emptyTex, pos.x, pos.y);
		
		if(!(player.Interface & INTER_COMBATMODE)) {
			
			const Rect redGaugeMouseTestRect(
				pos.x,
				pos.y,
				pos.x + size.x,
				pos.y + size.y
			);
			
			if(redGaugeMouseTestRect.contains(Vec2i(DANAEMouse))) {
				if((EERIEMouseButton & 1) && !(LastMouseClick & 1)) {
					std::stringstream ss;
					ss << checked_range_cast<int>(player.lifePool.current);
					ARX_SPEECH_Add(ss.str());
				}
			}
		}
	}
};
HealthGauge healthGauge;

class ManaGauge {
private:
	TextureContainer * m_emptyTex;
	TextureContainer * m_filledTex;
	float m_amount;
public:
	void init() {
		m_emptyTex = TextureContainer::LoadUI("graph/interface/bars/empty_gauge_blue");
		m_filledTex = TextureContainer::LoadUI("graph/interface/bars/filled_gauge_blue");
		arx_assert(m_emptyTex);
		arx_assert(m_filledTex);
	}
	
	void update() {
		
		m_amount = player.manaPool.current / player.Full_maxmana;
	}
	
	void draw() {
		Vec2f size = Vec2f(m_filledTex->size());
		
		Vec2f pos = Vec2f(g_size.bottomRight());
		pos += Vec2f(-33, -81);
		pos.x += lSLID_VALUE;
		
		EERIEDrawBitmap2DecalY(pos.x + 1, pos.y, size.x, size.y, 0.f,
			m_filledTex, Color::white, (1.f - m_amount));
		
		ARX_INTERFACE_DrawItem(m_emptyTex, pos.x + 1, pos.y);
		
		if(!(player.Interface & INTER_COMBATMODE)) {
			
			const Rect blueGaugeMouseTestRect(
				pos.x,
				pos.y,
				pos.x + size.x,
				pos.y + size.y
			);
			
			if(blueGaugeMouseTestRect.contains(Vec2i(DANAEMouse))) {
				if((EERIEMouseButton & 1) && !(LastMouseClick & 1)) {
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
class MecanismIcon {
private:
	TextureContainer * m_tex;
	Color m_color;
	long m_timeToDraw;
	long m_nbToDraw;
	
public:
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
	}
	
	void draw() {
		if(m_nbToDraw >= 3) {
			return;
		}
		
		Vec2f size = Vec2f(m_tex->size());
		EERIEDrawBitmap(Rectf(Vec2f(0, 0), size.x, size.y), 0.01f, m_tex, m_color);
	}
};
MecanismIcon mecanismIcon;

void mecanismIconReset() {
	mecanismIcon.reset();
}


class ScreenArrows {
private:
	TextureContainer * m_arrowLeftTex;
	Vec2f m_arrowSize;

	float fArrowMove;
	float fMove;
public:
	void init() {
		m_arrowLeftTex = TextureContainer::LoadUI("graph/interface/icons/arrow_left");
		arx_assert(m_arrowLeftTex);
	}
	
	void update() {
		m_arrowSize = Vec2f(m_arrowLeftTex->size());
		
		fArrowMove += .5f * framedelay;
		if(fArrowMove > 180.f) {
			fArrowMove=0.f;
		}
		fMove=fabs(sin(radians(fArrowMove)))*m_arrowSize.x*.5f;
	}
	
	void draw() {
		Color lcolor = Color3f::gray(.5f).to<u8>();
		
		// Left
		Rectf left(Vec2f(0 + fMove, g_size.center().y - (m_arrowSize.y * .5f)), m_arrowSize.x, m_arrowSize.y);
		EERIEDrawBitmap(left, 0.01f, m_arrowLeftTex, lcolor);
		// Right
		EERIEDrawBitmapUVs(g_size.width() - m_arrowSize.x - fMove, g_size.center().y - (m_arrowSize.y * .5f),
			m_arrowSize.x, m_arrowSize.y, .01f, m_arrowLeftTex, lcolor, 1.f, 0.f, 0.f, 0.f, 1.f, 1.f, 0.f, 1.f);
		// Up
		EERIEDrawBitmapUVs(g_size.center().x - (m_arrowSize.y * .5f), 0.f + fMove,
			m_arrowSize.y, m_arrowSize.x, .01f, m_arrowLeftTex, lcolor, 0.f, 1.f, 0.f, 0.f, 1.f, 1.f, 1.f, 0.f);
		// Down
		EERIEDrawBitmapUVs(g_size.center().x - (m_arrowSize.y * .5f), (g_size.height() - m_arrowSize.x) - fMove,
			m_arrowSize.y, m_arrowSize.x, .01f, m_arrowLeftTex, lcolor, 1.f, 1.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f);
	}
};
ScreenArrows screenArrows;

class PrecastSpellsGui {
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
				
				if((LastMouseClick & 1) && !(EERIEMouseButton & 1)) {
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

public:
	
	void update() {
		m_icons.clear();
		
		if(!(!(player.Interface & INTER_INVENTORYALL) && !(player.Interface & INTER_MAP))) {
			return;
		}
		
		float intensity = 1.f - PULSATE * 0.5f;
		intensity = clamp(intensity, 0.f, 1.f);
		
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
			
			Vec2f pos = Vec2f(g_size.bottomLeft());
			pos += Vec2f(InventoryX, 0.f);
			pos += Vec2f(110, -(126 + 32));
			
			if(pos.x < INTERFACE_RATIO(10)) {
				pos.x = INTERFACE_RATIO(10);
			}
			
			pos.x += (33 + 33 + 33);
			pos.x += i * 33;
			
			SpellType typ = precastSlot.typ;
			
			TextureContainer * tc = spellicons[typ].tc;
			arx_assert(tc);
			Rectf rect(pos, tc->m_dwWidth * 0.5f, tc->m_dwHeight * 0.5f);
			
			PrecastSpellIconSlot icon;
			icon.update(rect, tc, color, PrecastHandle(i));
			
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
				
				if((LastMouseClick & 1) && !(EERIEMouseButton & 1)) {
					if(spells[spellIndex]->m_type >= 0)
						WILLADDSPEECH = spellicons[spells[spellIndex]->m_type].name;
					
					WILLADDSPEECHTIME = (unsigned long)(arxtime);
				}
				
				if(EERIEMouseButton & 4) {
					ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE);
					EERIEMouseButton &= ~4;
					spells[spellIndex]->m_tolive = 0;
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
		
		if(spell.m_bDuration) {
			if(player.manaPool.current < 20 || spell.m_timcreation + spell.m_tolive - float(arxtime) < 2000) {
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
			
			if(   spell->m_exist
			   && spell->m_caster == PlayerEntityHandle
			   && spellicons[spell->m_type].bDuration
			) {
				ManageSpellIcon(*spell, intensity, false);
			}
		}
	}
	
	void spellsOnPlayerUpdate(float intensity) {
			for(size_t i = 0; i < MAX_SPELLS; i++) {
				SpellBase * spell = spells[SpellHandle(i)];
				if(!spell->m_exist)
					continue;
				
				if(std::find(spell->m_targets.begin(), spell->m_targets.end(), PlayerEntityHandle) == spell->m_targets.end()) {
					continue;
				}
				
				if(spell->m_caster != PlayerEntityHandle && spellicons[spell->m_type].bDuration) {
					ManageSpellIcon(*spell, intensity, true);
				}
			}
	}
	
	void update() {
		currpos = static_cast<long>(INTERFACE_RATIO(50.f));
		
		float intensity = 1.f - PULSATE * 0.5f;
		intensity = clamp(intensity, 0.f, 1.f);
		
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
class DamagedEquipmentGui {
private:
	TextureContainer * iconequip[5];
public:
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
	
	void draw()
	{
		if(cinematicBorder.isActive() || BLOCK_PLAYER_CONTROLS)
			return;
	
		if(player.Interface & INTER_INVENTORYALL)
			return;
	
		long needdraw=0;
	
		for(long i = 0; i < 5; i++) {
			if(iconequip[i]) {
				long eq=-1;
	
				switch (i) {
					case 0:
						eq = EQUIP_SLOT_WEAPON;
						break;
					case 1:
						eq = EQUIP_SLOT_SHIELD;
						break;
					case 2:
						eq = EQUIP_SLOT_HELMET;
						break;
					case 3:
						eq = EQUIP_SLOT_ARMOR;
						break;
					case 4:
						eq = EQUIP_SLOT_LEGGINGS;
						break;
				}
	
				if(player.equiped[eq] > 0) {
					Entity *io = entities[player.equiped[eq]];
					float ratio = io->durability / io->max_durability;
	
					if(ratio <= 0.5f)
						needdraw |= 1<<i;
				}
			}
		}
	
		if(needdraw) {
			GRenderer->SetRenderState(Renderer::AlphaBlending, true);
			GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
	
			GRenderer->SetCulling(Renderer::CullNone);
			GRenderer->SetRenderState(Renderer::DepthWrite, true);
			GRenderer->SetRenderState(Renderer::Fog, false);
	
			Vec2f pos(InventoryX + 10 + 32 + 100, g_size.height() - 158);
			
			if(pos.x < INTERFACE_RATIO( 10 + 32 ))
				pos.x = INTERFACE_RATIO( 10 + 32 );
	
			for(long i = 0; i < 5; i++) {
				if((needdraw & (1<<i)) && iconequip[i]) {
					long eq=-1;
	
					switch(i) {
						case 0:
							eq = EQUIP_SLOT_WEAPON;
							break;
						case 1:
							eq = EQUIP_SLOT_SHIELD;
							break;
						case 2:
							eq = EQUIP_SLOT_HELMET;
							break;
						case 3:
							eq = EQUIP_SLOT_ARMOR;
							break;
						case 4:
							eq = EQUIP_SLOT_LEGGINGS;
							break;
					}
	
					if(player.equiped[eq] > 0) {
						Entity *io = entities[player.equiped[eq]];
						float ratio = io->durability / io->max_durability;
						Color col = Color3f(1.f-ratio, ratio, 0).to<u8>();
						EERIEDrawBitmap2(Rectf(pos, iconequip[i]->m_dwWidth, iconequip[i]->m_dwHeight), 0.001f, iconequip[i], col);
					}
				}
			}
			
			GRenderer->SetRenderState(Renderer::AlphaBlending, false);
		}
	}
};
DamagedEquipmentGui damagedEquipmentGui;

extern float CURRENT_PLAYER_COLOR;

/*!
 * \brief Stealth Gauge Drawing
 */
class StealthGauge {
private:
	TextureContainer * stealth_gauge_tc;
	
public:
	void init() {
		stealth_gauge_tc = TextureContainer::LoadUI("graph/interface/icons/stealth_gauge");
		arx_assert(stealth_gauge_tc);
	}

	void draw() {
		
		if(stealth_gauge_tc && !cinematicBorder.isActive()) {
			float v=GetPlayerStealth();
	
			if(CURRENT_PLAYER_COLOR < v) {
				Vec2f pos = Vec2f(InventoryX, g_size.height());
				pos += Vec2f(110, -(126 + 32));
				
				if(pos.x < INTERFACE_RATIO(10))
					pos.x = INTERFACE_RATIO(10);
	
				float t = v - CURRENT_PLAYER_COLOR;
	
				if(t >= 15)
					v = 1.f;
				else
					v = (t*( 1.0f / 15 ))* 0.9f + 0.1f;
	
				GRenderer->SetRenderState(Renderer::AlphaBlending, true);
				GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
				
				Rectf rect(pos, stealth_gauge_tc->m_dwWidth, stealth_gauge_tc->m_dwHeight);
				EERIEDrawBitmap(rect, 0.01f, stealth_gauge_tc, Color3f::gray(v).to<u8>());
				GRenderer->SetRenderState(Renderer::AlphaBlending, false);
			}
		}
	}
};
StealthGauge stealthGauge;

void UpdateInterface() {
	hitStrengthGauge.update();
	secondaryInventory.update();
	inventoryGui.update();
	mecanismIcon.update();
	screenArrows.update();
	healthGauge.update();
	manaGauge.update();
	memorizedSpellIconsGui.update();
	changeLevelIconGui.update();
	quickSaveIconGui.update();
}

void ArxGame::drawAllInterface() {
	
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
	if(!(player.Interface & INTER_COMBATMODE)) {
		DrawIcons();
	}
	if(CHANGE_LEVEL_ICON > -1) {
		changeLevelIconGui.draw();
	}
	quickSaveIconGui.draw();
	stealthGauge.draw();

	if((player.Interface & INTER_MAP) && !(player.Interface & INTER_COMBATMODE)) {
		ARX_INTERFACE_ManageOpenedBook();
		ARX_INTERFACE_ManageOpenedBook_Finish();
	}
	if(CurrSpellSymbol || player.SpellToMemorize.bSpell) {
		memorizedSpellIconsGui.draw();
	}
	if(player.Interface & INTER_LIFE_MANA) {
		healthGauge.draw();
		manaGauge.draw();
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
	GRenderer->GetTextureStage(0)->setMinFilter(TextureStage::FilterLinear);
	GRenderer->GetTextureStage(0)->setMagFilter(TextureStage::FilterLinear);
	GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapRepeat);
	
	precastSpellsGui.update();
	precastSpellsGui.draw();
	activeSpellsGui.draw();
}

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
}

//-----------------------------------------------------------------------------
void ArxGame::manageEditorControls() {

	eMouseState = MOUSE_IN_WORLD;

	if(TRUE_PLAYER_MOUSELOOK_ON && config.input.autoReadyWeapon == false && config.input.mouseLookToggle) {
		float fX = g_size.width() * 0.5f;
		float fY = g_size.height() * 0.5f;
		DANAEMouse.x = checked_range_cast<short>(fX);
		DANAEMouse.y = checked_range_cast<short>(fY);
	}
	
	playerInterfaceFader.updateFirst();
	
	// on ferme
	if((player.Interface & INTER_COMBATMODE) || player.doingmagic >= 2) {
		Entity * io = NULL;

		if(SecondaryInventory)
			io = (Entity *)SecondaryInventory->io;
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



	/////////////////////////////////////////////////////

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

	//-------------------------------------------------------------------------
	// interface
	//-------------------------------------------------------------------------
	// torch

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
	
	if(TSecondaryInventory) {
		
		Entity * temp=(Entity *)TSecondaryInventory->io;

		if(temp && !(temp->ioflags & IO_SHOP) && !(temp == ioSteal)) {
			pickAllIconGui.updateInput();
		}
		
		closeSecondaryInventoryIconGui.updateInput();
	}

	//-------------------------------------------------------------------------


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
			long sx = 0;
			long sy = 0;
			bool bSecondary = false;
			
			if(TSecondaryInventory && IsInSecondaryInventory(FlyingOverIO)) {
				if(SecondaryInventory) {
					bool bfound = true;
					
					for(long j = 0; j < SecondaryInventory->sizey && bfound; j++) {
						for (long i = 0; i < SecondaryInventory->sizex && bfound; i++) {
							if(SecondaryInventory->slot[i][j].io == FlyingOverIO) {
								sx = i;
								sy = j;
								bfound = false;
							}
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
					extern short sInventory, sInventoryX, sInventoryY;
					sInventory = 2;
					sInventoryX = checked_range_cast<short>(sx);
					sInventoryY = checked_range_cast<short>(sy);
					
					CanBePutInSecondaryInventory( TSecondaryInventory, FlyingOverIO, &sx, &sy );
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
						viewvector.x = -std::sin(radians(player.angle.getPitch()+(x_ratio*30.f))) * std::cos(radians(player.angle.getYaw()));
						viewvector.y =  std::sin(radians(player.angle.getYaw())) + y_ratio;
						viewvector.z =  std::cos(radians(player.angle.getPitch()+(x_ratio*30.f))) * std::cos(radians(player.angle.getYaw()));
						
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
				Vec3f pos;
				
				if(GetItemWorldPosition(COMBINE, &pos)) {
					if(fartherThan(pos, player.pos, 300.f))
						COMBINE=NULL;
				}
				else
					COMBINE=NULL;
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
	
				if(Project.telekinesis)
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
									if(!light->status) {
										light->status = true;
										ARX_SOUND_PlaySFX(SND_TORCH_START, &light->pos);
									}
								}
	
								if(COMBINE->_itemdata->LightValue == 0) {
									if(light->status) {
										light->status = false;
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
					bQuitCombine = inventoryGui.updateInput();
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
			
			if(Project.telekinesis)
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
				Entity * io=(Entity *)SecondaryInventory->io;
				
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
				if(LastSelectedIONum == InvalidEntityHandle)
					LastSelectedIONum = PlayerEntityHandle;
				else
					LastSelectedIONum = InvalidEntityHandle;
			}
		}
	}
}
