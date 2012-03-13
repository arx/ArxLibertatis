/*
 * Copyright 2011 Arx Libertatis Team (see the AUTHORS file)
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
#include <iostream>

#include "animation/Animation.h"

#include "core/Application.h"
#include "core/ArxGame.h"
#include "core/Config.h"
#include "core/GameTime.h"
#include "core/Localisation.h"
#include "core/Core.h"

#include "game/Damage.h"
#include "game/NPC.h"
#include "game/Equipment.h"
#include "game/Player.h"
#include "game/Levels.h"
#include "game/Inventory.h"

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
#include "graphics/particle/ParticleEffects.h"
#include "graphics/texture/TextureStage.h"
#include "graphics/texture/Texture.h"

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
#include "math/Vector2.h"
#include "math/Vector3.h"

#include "physics/Box.h"
#include "physics/Collisions.h"

#include "platform/String.h"

#include "scene/LinkedObject.h"
#include "scene/GameSound.h"
#include "scene/Interactive.h"
#include "scene/Light.h"

#include "script/Script.h"

#include "window/RenderWindow.h"

using std::min;
using std::max;
using std::string;
using std::vector;

extern float MagicSightFader;
extern long NEED_TEST_TEXT;
extern float Original_framedelay;
extern EERIE_3DOBJ *arrowobj;
extern void InitTileLights();
extern long USE_LIGHT_OPTIM;

long Manage3DCursor(long flags); // flags & 1 == simulation
long IN_BOOK_DRAW=0;
int TSU_TEST_COLLISIONS = 0;
extern float INVISIBILITY_OVERRIDE;
//-----------------------------------------------------------------------------
struct ARX_INTERFACE_HALO_STRUCT
{
	INTERACTIVE_OBJ  * io;
	TextureContainer * tc;
	TextureContainer * tc2;
	float POSX;
	float POSY;
	float fRatioX;
	float fRatioY;
};
//-----------------------------------------------------------------------------
#define GL_DECAL_ICONS		0
#define BOOKMARKS_POS_X		216.f
#define BOOKMARKS_POS_Y		60.f
#define PAGE_CHAR_SIZE		4096

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
extern TextureContainer * iconequip[];
extern TextureContainer * Movable;
extern TextureContainer * ChangeLevel;
extern TextureContainer * inventory_font;
extern TextureContainer * scursor[];
extern TextureContainer * pTCCrossHair;
extern TextureContainer * mecanism_tc;
extern TextureContainer * arrow_left_tc;
extern FOG_DEF fogparam;
extern TexturedVertex LATERDRAWHALO[];
extern EERIE_LIGHT lightparam;
extern INTERACTIVE_OBJ * CURRENT_TORCH;
extern Notification speech[];
extern std::string WILLADDSPEECH;
extern float PLAYER_ROTATION;
extern float SLID_VALUE;

extern long BOOKBUTTON;
extern long LASTBOOKBUTTON;
extern long FORCE_NO_HIDE;
extern long LastSelectedIONum;
extern long lSLID_VALUE;

extern long CHANGE_LEVEL_ICON;
extern long BLOCK_PLAYER_CONTROLS;
extern long DeadTime;
extern long HALOCUR;
extern long ALLOW_CHEATS;
extern long LOOKING_FOR_SPELL_TARGET;
extern long WILLRETURNTOFREELOOK;
extern float BOW_FOCAL;
extern Vec2s DANAEMouse;
extern short sActiveInventory;
extern unsigned long WILLADDSPEECHTIME;
extern unsigned long LOOKING_FOR_SPELL_TARGET_TIME;

extern float ARXTimeMenu;
extern float ARXOldTimeMenu;
extern float ARXDiffTimeMenu;

extern bool bGToggleCombatModeWithKey;
extern long PlayerWeaponBlocked;
extern unsigned char ucFlick;

extern TextManager *pTextManageFlyingOver;

bool IsPlayerStriking();
void OptmizeInventory(unsigned int);

extern long SHOW_INGAME_MINIMAP;

//-----------------------------------------------------------------------------
TextureContainer *	BasicInventorySkin=NULL;
TextureContainer *	ThrowObject=NULL;
ARX_INTERFACE_HALO_STRUCT * aiHalo=NULL;
E_ARX_STATE_MOUSE	eMouseState;
Vec2s bookclick;
Vec2s MemoMouse;

INVENTORY_DATA *	TSecondaryInventory;
INTERACTIVE_OBJ * FlyingOverIO=NULL;
INTERACTIVE_OBJ *	STARTED_ACTION_ON_IO=NULL;
INTERFACE_TC		ITC;

static gui::Note openNote;
static gui::Note questBook;

bool				bBookHalo = false;
bool				bGoldHalo = false;
bool				bHitFlash = false;
bool				bInventoryClosing = false;
bool				bInventorySwitch = false;
bool				bIsAiming = false;
float				fHitFlash = 0;
unsigned long		ulHitFlash = 0;
unsigned long		ulBookHaloTime = 0;
unsigned long		ulGoldHaloTime = 0;
float				InventoryX=-60.f;
float				InventoryDir=0; // 0 stable, 1 to right, -1 to left

float				CINEMA_DECAL=0.f;
float				SLID_START=0.f; // Charging Weapon
float				BOOKDECX=0.f;
float				BOOKDECY=0.f;
float				PROGRESS_BAR_TOTAL=0;
float				PROGRESS_BAR_COUNT=0;
float				OLD_PROGRESS_BAR_COUNT=0;

bool				PLAYER_MOUSELOOK_ON = false;
bool				TRUE_PLAYER_MOUSELOOK_ON = false;
bool				LAST_PLAYER_MOUSELOOK_ON = false;
bool				MEMO_PLAYER_MOUSELOOK_ON = false;

long				COMBINEGOLD=0;
ARX_INTERFACE_BOOK_MODE	Book_Mode=BOOKMODE_STATS;
long				Book_MapPage=1;
long				Book_SpellPage=1;
long				CINEMASCOPE=0;
float				g_TimeStartCinemascope = 0;
long				CINEMA_INC=0;
long				SMOOTHSLID=0;
long				currpos=50;
long				DRAGGING = 0;
long				INVERTMOUSE=0;
long				PLAYER_INTERFACE_HIDE_COUNT=0;
long				MAGICMODE=-1;
long				SpecialCursor=0;
long				FLYING_OVER		= 0;
long				OLD_FLYING_OVER	= 0;
long				LastRune=-1;
long				BOOKZOOM=0;
Anglef			ePlayerAngle;
long				CURCURTIME=0;
long				CURCURDELAY=70;
long				CURCURPOS=0;
long				INTERFACE_HALO_NB=0;
long				INTERFACE_HALO_MAX_NB=0;
long				PRECAST_NUM=0;
long				LastMouseClick=0;
long				MOVETYPE=MOVE_WAIT;

//used to redist points - attributes and skill
long				lCursorRedistValue = 0;
long				lFadeMapTime = 0;

unsigned long		COMBAT_MODE_ON_START_TIME = 0;
long SPECIAL_DRAW_WEAPON=0;
bool bGCroucheToggle=false;

int iHighLight=0;
float fHighLightAng=0.f;

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
INTERACTIVE_OBJ *pIOChangeWeapon=NULL;

static long lTimeToDrawMecanismCursor=0;
static long lNbToDrawMecanismCursor=0;
static long lOldInterface;



namespace gui {

namespace {

//! return true if the note was clicked
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
			return true;
			EERIEMouseButton &= ~2;
		}
	}
	
	return false;
}


/// Update QuestBook_Cache_Text if it needs to. Otherwise does nothing.
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


//-----------------------------------------------------------------------------
bool MouseInBookRect(const float x, const float y, const float cx, const float cy)
{
	return ((DANAEMouse.x>=(x+BOOKDECX)*Xratio)
		&& (DANAEMouse.x<=(cx+BOOKDECX)*Xratio)
		&& (DANAEMouse.y>=(y+BOOKDECY)*Yratio)
	        && (DANAEMouse.y <= (cy + BOOKDECY) * Yratio));
}

//-----------------------------------------------------------------------------
bool MouseInCam(EERIE_CAMERA * cam)
{
	return ((DANAEMouse.x>cam->clip.left) && (DANAEMouse.x<cam->clip.right) &&
		(DANAEMouse.x>cam->clip.top) && (DANAEMouse.y<cam->clip.bottom));
}

//-----------------------------------------------------------------------------
bool MouseInRect(const float x0, const float y0, const float x1=32, const float y1=32)
{
	return (	(DANAEMouse.x>=x0)
		&&	(DANAEMouse.x<=x1)
		&&	(DANAEMouse.y>=y0)
		&&	(DANAEMouse.y<=y1) );
}

//-----------------------------------------------------------------------------
bool ARX_INTERFACE_MouseInBook()
{		
	if ((player.Interface & INTER_MAP ) &&  (!(player.Interface & INTER_COMBATMODE))) 
		return (MouseInBookRect(99,65,599,372));

	else return false;
}

static void ARX_INTERFACE_DrawItem(TextureContainer * tc, float x, float y, float z = 0.001f, Color col = Color::white) {
	if(tc) {
		EERIEDrawBitmap(x, y, INTERFACE_RATIO_DWORD(tc->m_dwWidth), INTERFACE_RATIO_DWORD(tc->m_dwHeight), z, tc, col);
	}
}

static void ARX_INTERFACE_DrawNumber(const float x, const float y, const long num, const int _iNb, const Color color) {
	
	ColorBGRA col = color.toBGRA();
	
	TexturedVertex v[4];
	v[0] = TexturedVertex(Vec3f::ZERO, 1.f, 1, 1, Vec2f::ZERO);
	v[1] = TexturedVertex(Vec3f::ZERO, 1.f, 1, 1, Vec2f::X_AXIS);
	v[2] = TexturedVertex(Vec3f::ZERO, 1.f, 1, 1, Vec2f(1.f, 1.f));
	v[3] = TexturedVertex(Vec3f::ZERO, 1.f, 1, 1, Vec2f::Y_AXIS);
	
	v[0].p.z = v[1].p.z = v[2].p.z = v[3].p.z = 0.0000001f;

	if(inventory_font) {
		
		char tx[7];
		long tt;
		float ttx;
		float divideX = 1.f/((float) inventory_font->m_dwWidth);
		float divideY = 1.f/((float) inventory_font->m_dwHeight);
		
		sprintf(tx, "%*ld", _iNb, num); // TODO use a safe string function.
		long removezero=1;

		for(long i = 0; i < 6 && tx[i] != '\0'; i++) {

			tt=tx[i]-'0';

			if ((tt==0) && removezero) continue;

			if (tt>=0)
			{
				removezero=0;
				v[0].p.x=v[3].p.x=x + i*INTERFACE_RATIO(10);
				v[1].p.x = v[2].p.x = v[0].p.x + INTERFACE_RATIO(10);
				v[0].p.y=v[1].p.y=y;
				v[2].p.y=v[3].p.y=y + INTERFACE_RATIO(10);
				v[0].color=v[1].color=v[2].color=v[3].color=col;

				ttx = (float)((float)tt * (float)11.f) + 1.5f;
				v[3].uv.x=v[0].uv.x=ttx*divideX;
				v[1].uv.x=v[2].uv.x=(ttx+10.f)*divideX;

				ttx=0.5f*divideY;
				v[1].uv.y = v[0].uv.y = divideY + ttx;
				v[2].uv.y = v[3].uv.y = divideY * 12;
				GRenderer->SetTexture(0, inventory_font);

				EERIEDRAWPRIM(Renderer::TriangleFan, v, 4);
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Interface Texture Containers creation
//-----------------------------------------------------------------------------
void CreateInterfaceTextureContainers()
{
    ITC.Reset();

	ITC.Set("aim_empty", "graph/interface/bars/aim_empty");
	ITC.Set("aim_maxi", "graph/interface/bars/aim_maxi");
	ITC.Set("aim_hit", "graph/interface/bars/flash_gauge");
	ITC.Set("hero_inventory", "graph/interface/inventory/hero_inventory");
	ITC.Set("hero_inventory_up", "graph/interface/inventory/scroll_up");
	ITC.Set("hero_inventory_down", "graph/interface/inventory/scroll_down");
	ITC.Set("hero_inventory_link", "graph/interface/inventory/hero_inventory_link");
	ITC.Set("inventory_pickall", "graph/interface/inventory/inv_pick");
	ITC.Set("inventory_close", "graph/interface/inventory/inv_close");
	ITC.Set("icon_lvl_up", "graph/interface/icons/lvl_up");

	ITC.Set("backpack", "graph/interface/icons/backpack");
	ITC.Set("gold", "graph/interface/inventory/gold");
	ITC.Set("book", "graph/interface/icons/book");
	ITC.Set("steal", "graph/interface/icons/steal");
	ITC.Set("item_cant_steal", "graph/interface/icons/cant_steal_item");
	ITC.Set("empty_gauge_red", "graph/interface/bars/empty_gauge_red");
	ITC.Set("empty_gauge_blue", "graph/interface/bars/empty_gauge_blue");
	ITC.Set("filled_gauge_red", "graph/interface/bars/filled_gauge_red");
	ITC.Set("filled_gauge_blue", "graph/interface/bars/filled_gauge_blue");
	ITC.Set("target_on", "graph/interface/cursors/target_on");
	ITC.Set("target_off", "graph/interface/cursors/target_off");
	ITC.Set("interaction_on", "graph/interface/cursors/interaction_on");
	ITC.Set("interaction_off", "graph/interface/cursors/interaction_off");
	ITC.Set("magic", "graph/interface/cursors/magic");
	
	BasicInventorySkin = TextureContainer::LoadUI("graph/interface/inventory/ingame_inventory");
	ThrowObject = TextureContainer::LoadUI("graph/interface/cursors/throw");
}

//-----------------------------------------------------------------------------

void KillInterfaceTextureContainers() {
	ITC.Reset();
}

INTERFACE_TC::INTERFACE_TC()
{
}

INTERFACE_TC::~INTERFACE_TC() {
	
	arx_assert(m_Textures.empty());
	
	/*
	 * Because this is the destructor of a global object we cannot cleanup textures here.
	 * Doing so would mean accessing other global objects who'se destrcutors could be called before this one.
	 */
	
}

void INTERFACE_TC::Set(const std::string& textureName, TextureContainer* pTexture)
{
	m_Textures[textureName] = pTexture;
}

void INTERFACE_TC::Set(const std::string& textureName, const std::string& fileName)
{
	m_Textures[textureName] = TextureContainer::LoadUI(fileName.c_str());
}

TextureContainer* INTERFACE_TC::Get(const std::string& name)
{
	TextureDictionary::iterator it = m_Textures.find(name);
	return it != m_Textures.end() ? (*it).second : NULL;		
}

void INTERFACE_TC::Reset()
{
	// For each item in the map...
	for(TextureDictionary::iterator it = m_Textures.begin(); it != m_Textures.end(); ++it)
	{
		// Free the textures...
		delete (*it).second;
	}

	m_Textures.clear();

	ITC.Level.clear();
	ITC.Xp.clear();
}

static void DrawBookInterfaceItem(TextureContainer * tc, float x, float y, Color color = Color::white, float z = 0.000001f) {
	if(tc) {
		EERIEDrawBitmap2(
			(x + BOOKDECX) * Xratio,
			(y + BOOKDECY) * Yratio,
			tc->m_dwWidth * Xratio,
			tc->m_dwHeight * Yratio,
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
	power -= EEsin(arxtime.get_frame_time()*0.01f) * 0.3f;

	_fR = clamp(_fR * power, 0, 1);
	_fG = clamp(_fG * power, 0, 1);
	_fB = clamp(_fB * power, 0, 1);
	Color col=Color4f(_fR,_fG,_fB).to<u8>();

	if (_lHaloType & HALO_NEGATIVE)
	{
		GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);
	}
	else
	{
		GRenderer->SetBlendFunc(Renderer::BlendSrcAlpha, Renderer::BlendOne);
	}

	GRenderer->SetRenderState(Renderer::AlphaBlending, true);

	float fSizeX = haloTexture->m_dwWidth * fRatioX;
	float fSizeY = haloTexture->m_dwHeight * fRatioY;
	
	EERIEDrawBitmap(POSX - TextureContainer::HALO_RADIUS, POSY - TextureContainer::HALO_RADIUS, fSizeX, fSizeY, 0.00001f, haloTexture, col);

	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
}

void ARX_INTERFACE_HALO_Draw(INTERACTIVE_OBJ * io, TextureContainer * tc, TextureContainer * tc2, float POSX, float POSY, float _fRatioX = 1, float _fRatioY = 1) {
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

//-----------------------------------------------------------------------------
void ReleaseHalo()
{
	if (aiHalo)
	{
		free(aiHalo);
		aiHalo=NULL;
	}
}

//-----------------------------------------------------------------------------
void ARX_INTERFACE_HALO_Flush() {
	
	for (long i=0;i<INTERFACE_HALO_NB;i++)
		ARX_INTERFACE_HALO_Render(
		aiHalo[i].io->halo.color.r, aiHalo[i].io->halo.color.g, aiHalo[i].io->halo.color.b,
		aiHalo[i].io->halo.flags,
		aiHalo[i].tc2,aiHalo[i].POSX,aiHalo[i].POSY, aiHalo[i].fRatioX, aiHalo[i].fRatioY);

	INTERFACE_HALO_NB=0;
}

//-----------------------------------------------------------------------------
bool NeedHalo(INTERACTIVE_OBJ * io)
{
	if (	(!io)
		||	(!(io->ioflags & IO_ITEM))	)
		return false;

	if (io->halo.flags & HALO_ACTIVE)
	{
		if (io->inv)
		{
			io->inv->getHalo();
		}

		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
void InventoryOpenClose(unsigned long t) // 0 switch 1 forceopen 2 forceclose
{
	if ((t==1) && (player.Interface & INTER_INVENTORY)) return;

	if ((t==2) && (!(player.Interface & INTER_INVENTORY))) return;

	ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());

	if ((player.Interface & INTER_INVENTORY)||(player.Interface & INTER_INVENTORYALL))
	{
		bInventoryClosing = true;

		if (WILLRETURNTOFREELOOK)
		{
			TRUE_PLAYER_MOUSELOOK_ON = true;
			SLID_START=float(arxtime);
			WILLRETURNTOFREELOOK = 0;
		}
	}
	else
	{
		player.Interface |= INTER_INVENTORY;
		InventoryY = static_cast<long>(INTERFACE_RATIO(100.f));

		if (TRUE_PLAYER_MOUSELOOK_ON)
		{
			WILLRETURNTOFREELOOK = 1;
		}
	}

	if ((((player.Interface & INTER_INVENTORYALL)||(TRUE_PLAYER_MOUSELOOK_ON)))&&(player.Interface & INTER_NOTE))
	{
		ARX_INTERFACE_NoteClose();
	}

	if (!bInventoryClosing && config.input.autoReadyWeapon == false)
	{
		TRUE_PLAYER_MOUSELOOK_ON = false;
	}
}

void ARX_INTERFACE_NoteClear() {
	player.Interface &= ~INTER_NOTE;
	openNote.clear();
}

void ARX_INTERFACE_NoteOpen(gui::Note::Type type, const string & text) {
	
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

//-----------------------------------------------------------------------------
void ARX_INTERFACE_BookOpenClose(unsigned long t) // 0 switch 1 forceopen 2 forceclose
{
	if ((t==1) && (player.Interface & INTER_MAP)) return;

	if ((t==2) && (!(player.Interface & INTER_MAP))) return;


	if (player.Interface & INTER_MAP)
	{
		ARX_SOUND_PlayInterface(SND_BOOK_CLOSE, 0.9F + 0.2F * rnd());
		SendIOScriptEvent(inter.iobj[0],SM_BOOK_CLOSE);
		player.Interface &=~ INTER_MAP;
		ARX_MINIMAP_PurgeTC();

		if(ARXmenu.mda) {
			for(long i = 0; i < MAX_FLYOVER; i++) {
				ARXmenu.mda->flyover[i].clear();
			}
			free(ARXmenu.mda);
			ARXmenu.mda=NULL;
		}
	}
	else
	{
		SendIOScriptEvent(inter.iobj[0],SM_NULL,"","book_open");

		ARX_SOUND_PlayInterface(SND_BOOK_OPEN, 0.9F + 0.2F * rnd());
		SendIOScriptEvent(inter.iobj[0],SM_BOOK_OPEN);
		ARX_INTERFACE_NoteClose();
		player.Interface |= INTER_MAP;
		Book_MapPage=ARX_LEVELS_GetRealNum(CURRENTLEVEL)+1;

		if (Book_MapPage>8) Book_MapPage=8;

		if (Book_MapPage<1) Book_MapPage=1;

		if(!ARXmenu.mda)
		{
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

	if (player.Interface&INTER_COMBATMODE)
	{
		player.Interface&=~INTER_COMBATMODE;
		ARX_EQUIPMENT_LaunchPlayerUnReadyWeapon();
	}

	if (player.Interface & INTER_INVENTORYALL)
	{
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

//-----------------------------------------------------------------------------
void ResetPlayerInterface()
{
	player.Interface |= INTER_LIFE_MANA;
	SLID_VALUE = 0;
	lSLID_VALUE = 0;
	SLID_START=float(arxtime);
}

extern long MouseDragX, MouseDragY;

//-----------------------------------------------------------------------------
void ReleaseInfosCombine()
{
	INTERACTIVE_OBJ * io = NULL;

	if (player.bag)
	for (int iNbBag=0; iNbBag<player.bag; iNbBag++)
	for (size_t j=0;j<INVENTORY_Y;j++)
	for (size_t i=0;i<INVENTORY_X;i++)
	{
		io = inventory[iNbBag][i][j].io;

		if(io)
		{
			io->ioflags&=~IO_CAN_COMBINE;
		}
	}

	if (SecondaryInventory)
	{
		for (long j=0;j<SecondaryInventory->sizey;j++)
			for (long i=0;i<SecondaryInventory->sizex;i++)
			{
				io=SecondaryInventory->slot[i][j].io;

				if ( io )
				{
					io->ioflags&=~IO_CAN_COMBINE;
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

void GetInfosCombineWithIO(INTERACTIVE_OBJ * _pWithIO)
{
	if(!COMBINE)
	{
		return;
	}

	std::string tcIndent = COMBINE->long_name();

		char tTxtCombineDest[256];

		if(	(_pWithIO)&&
			(_pWithIO!=COMBINE)&&
			(_pWithIO->script.data) )
		{
			char* pCopyScript=new char[_pWithIO->script.size + 1];
			pCopyScript[_pWithIO->script.size] = '\0';
			memcpy(pCopyScript,_pWithIO->script.data,_pWithIO->script.size);

			char* pCopyOverScript=NULL;

			if(_pWithIO->over_script.data)
			{
				pCopyOverScript=new char[_pWithIO->over_script.size + 1];
				pCopyOverScript[_pWithIO->over_script.size] = '\0';
				memcpy(pCopyOverScript,_pWithIO->over_script.data,_pWithIO->over_script.size);
			}

			char *pcFound=NULL;

			if(pCopyOverScript)
			{
				pcFound=strstr((char*)pCopyOverScript,"on combine");

				if(pcFound)
				{
					unsigned int uiNbOpen=0;

					char *pcToken=strtok(pcFound,"\r\n");

					if(strstr(pcToken,"{"))
					{
						uiNbOpen++;
					}

					while(pcToken)
					{
						pcToken=strtok(NULL,"\r\n");
	
						bool bCanCombine=false;
						char* pStartString;
						char* pEndString;

						pStartString = findParam(pcToken, "isclass");
						if(pStartString)
						{
							pStartString=strstr(pStartString,"\"");

							if(pStartString)
							{
								pStartString++;
								pEndString=strstr(pStartString,"\"");

								if(pEndString)
								{
									memcpy(tTxtCombineDest,pStartString,pEndString-pStartString);
									tTxtCombineDest[pEndString-pStartString]=0;

									if(tTxtCombineDest == COMBINE->short_name()) {
										//same class
										bCanCombine=true;
									}
								}
							}
						}
						else
						{
							pStartString = findParam(pcToken, "==");
							if(pStartString)
							{
								pStartString=strstr(pStartString,"\"");

								if(pStartString)
								{
									pStartString++;
									pEndString=strstr(pStartString,"\"");

									if(pEndString)
									{
										memcpy(tTxtCombineDest,pStartString,pEndString-pStartString);
										tTxtCombineDest[pEndString-pStartString]=0;

										if(tTxtCombineDest == tcIndent)
										{
											//same class
											bCanCombine=true;
										}
									}
								}
							}
							else
							{
								pStartString = findParam(pcToken, "isgroup");
								if(pStartString)
								{
									pStartString=strstr(pStartString," ");

									if(pStartString)
									{
										pStartString++;
										pEndString=strstr(pStartString," ");
										char* pEndString2=strstr(pStartString,")");

										if(pEndString2<pEndString)
										{
											pEndString=pEndString2;
										}

										if(pEndString)
										{
											memcpy(tTxtCombineDest,pStartString,pEndString-pStartString);
											tTxtCombineDest[pEndString-pStartString]=0;
											if(COMBINE->groups.find(tTxtCombineDest) != COMBINE->groups.end()) {
													//same class
													bCanCombine = true;
											}
										}
									}
								}
							}
						}

						if(strstr(pcToken,"{"))
						{
							uiNbOpen++;
						}

						if(strstr(pcToken,"}"))
						{
							uiNbOpen--;
						}

						if(bCanCombine)
						{
							uiNbOpen=0;
							_pWithIO->ioflags|=IO_CAN_COMBINE;
						}
						else
						{
							_pWithIO->ioflags&=~IO_CAN_COMBINE;
						}

						if(!uiNbOpen)
						{
							break;
						}
					}
				}
			}

			if(_pWithIO->ioflags&IO_CAN_COMBINE)
			{
				delete[] pCopyScript;
				delete[] pCopyOverScript;
				return;
			}

			pcFound=strstr((char*)pCopyScript,"on combine");

			if(pcFound)
			{
				unsigned int uiNbOpen=0;

				char *pcToken=strtok(pcFound,"\r\n");

				if(strstr(pcToken,"{"))
				{
					uiNbOpen++;
				}

				while(pcToken)
				{
					pcToken=strtok(NULL,"\r\n");
	
					bool bCanCombine=false;
					char* pStartString;
					char* pEndString;

					pStartString = findParam(pcToken, "isclass");
					if(pStartString)
					{
						pStartString=strstr(pStartString,"\"");

						if(pStartString)
						{
							pStartString++;
							pEndString=strstr(pStartString,"\"");

							if(pEndString)
							{
								memcpy(tTxtCombineDest,pStartString,pEndString-pStartString);
								tTxtCombineDest[pEndString-pStartString]=0;

								if(tTxtCombineDest == COMBINE->short_name()) {
									//same class
									bCanCombine=true;
								}
							}
						}
					}
					else
					{
						pStartString = findParam(pcToken, "==");
						if(pStartString)
						{
							pStartString=strstr(pStartString,"\"");

							if(pStartString)
							{
								pStartString++;
								pEndString=strstr(pStartString,"\"");

								if(pEndString)
								{
									memcpy(tTxtCombineDest,pStartString,pEndString-pStartString);
									tTxtCombineDest[pEndString-pStartString]=0;

									if(tTxtCombineDest == tcIndent) {
										//same class
										bCanCombine=true;
									}
								}
							}
						}
						else
						{
							pStartString = findParam(pcToken, "isgroup");
							if(pStartString)
							{
								pStartString=strstr(pStartString," ");

								if(pStartString)
								{
									pStartString++;
									pEndString=strstr(pStartString," ");
									char* pEndString2=strstr(pStartString,")");

									if(pEndString2<pEndString)
									{
										pEndString=pEndString2;
									}

									if(pEndString)
									{
										memcpy(tTxtCombineDest,pStartString,pEndString-pStartString);
										tTxtCombineDest[pEndString-pStartString]=0;
										
										if(COMBINE->groups.find(tTxtCombineDest) != COMBINE->groups.end()) {
											// same class
											bCanCombine = true;
										}
									}
								}
							}
						}
					}

					if(strstr(pcToken,"{"))
					{
						uiNbOpen++;
					}

					if(strstr(pcToken,"}"))
					{
						uiNbOpen--;
					}

					if(bCanCombine)
					{
						uiNbOpen=0;
						_pWithIO->ioflags|=IO_CAN_COMBINE;
					}
					else
					{
						_pWithIO->ioflags&=~IO_CAN_COMBINE;
					}

					if(!uiNbOpen)
					{
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
	INTERACTIVE_OBJ * io = NULL;

	if (player.bag)
	for (int iNbBag=0; iNbBag<player.bag; iNbBag++)
	for (size_t j=0;j<INVENTORY_Y;j++)
	for (size_t i=0;i<INVENTORY_X;i++)
	{
		io = inventory[iNbBag][i][j].io;
		GetInfosCombineWithIO(io);
	}

	if (SecondaryInventory)
	{
		for (long j=0;j<SecondaryInventory->sizey;j++)
		{
			for (long i=0;i<SecondaryInventory->sizex;i++)
			{
				io=SecondaryInventory->slot[i][j].io;
				GetInfosCombineWithIO(io);
			}
		}
	}
}

//-----------------------------------------------------------------------------
bool ArxGame::ManageEditorControls()
{
	Vec3f trans;

	eMouseState = MOUSE_IN_WORLD;

	if (TRUE_PLAYER_MOUSELOOK_ON && (config.input.autoReadyWeapon == false) && (config.input.mouseLookToggle))
	{

		float fX =  DANAESIZX * 0.5f;
		float fY =	DANAESIZY * 0.5f;
		DANAEMouse.x = checked_range_cast<short>(fX);
		DANAEMouse.y = checked_range_cast<short>(fY);


	}


	/////////////////////////////////////////////////////
	// begining to count time for sliding interface
	if ((!PLAYER_INTERFACE_HIDE_COUNT) && (SMOOTHSLID==0))
	{
		bool bOk = true;

		if (TRUE_PLAYER_MOUSELOOK_ON)
		{
			if (!(player.Interface & INTER_COMBATMODE) && (player.doingmagic!=2) && !InInventoryPos(&DANAEMouse))
			{
				bOk = false;

				float t=float(arxtime);

				if (t-SLID_START>10000.f)
				{
					SLID_VALUE+=(float)Original_framedelay*( 1.0f / 10 );

					if (SLID_VALUE>100.f) SLID_VALUE=100.f;

					lSLID_VALUE = SLID_VALUE;
				}
				else
				{
					bOk = true;

				}

			}
		}

		if (bOk)
		{
			SLID_VALUE-=(float)Original_framedelay*( 1.0f / 10 );

			if (SLID_VALUE<0.f) SLID_VALUE=0.f;

			lSLID_VALUE = SLID_VALUE;
		}
	}

	// on ferme
	if ((player.Interface & INTER_COMBATMODE) || (player.doingmagic>=2))
	{
		INTERACTIVE_OBJ * io = NULL;

		if (SecondaryInventory!=NULL)
		{
			io = (INTERACTIVE_OBJ *)SecondaryInventory->io;
		}
		else if (player.Interface & INTER_STEAL)
		{
			io = ioSteal;
		}

		if (io!=NULL)
		{
			InventoryDir=-1;
			SendIOScriptEvent(io,SM_INVENTORY2_CLOSE);
			TSecondaryInventory=SecondaryInventory;
			SecondaryInventory=NULL;

		}
	}


	if (SMOOTHSLID==1)
	{
		SLID_VALUE+=(float)Original_framedelay*( 1.0f / 10 );

		if (SLID_VALUE > 100.f)
		{
			SLID_VALUE = 100.f;
			SMOOTHSLID = 0;
		}

		lSLID_VALUE = SLID_VALUE;
	}
	else if (SMOOTHSLID==-1)
	{
		SLID_VALUE-=(float)Original_framedelay*( 1.0f / 10 );

		if (SLID_VALUE < 0.f)
		{
			SLID_VALUE = 0.f;
			SMOOTHSLID = 0;
		}

		lSLID_VALUE = SLID_VALUE;
	}

	if (CINEMA_INC==1)
	{
		CINEMA_DECAL+=(float)Original_framedelay*( 1.0f / 10 );

		if (CINEMA_DECAL > 100.f)
		{
			CINEMA_DECAL = 100.f;
			CINEMA_INC = 0;
		}
	}
	else if (CINEMA_INC==-1)
	{
		CINEMA_DECAL-=(float)Original_framedelay*( 1.0f / 10 );

		if (CINEMA_DECAL < 0.f)
		{
			CINEMA_DECAL = 0.f;
			CINEMA_INC = 0;
		}
	}

	/////////////////////////////////////////////////////

	if  (EERIEMouseButton & 1)
	{
		if ( !(LastMouseClick &1) )
		{
			STARTDRAG.x=DANAEMouse.x;
			STARTDRAG.y=DANAEMouse.y;
			DRAGGING=0;

			if (config.input.autoReadyWeapon == false)
			{
				MouseDragX = 0;
				MouseDragY = 0;
			}
		}
		else
		{
			// mode systemshock
			if (config.input.autoReadyWeapon == false)
			{
				MouseDragX += GInput->getMousePosRel().x;
				MouseDragY += GInput->getMousePosRel().y;
			}

			if (((abs(DANAEMouse.x-STARTDRAG.x)>2) && (abs(DANAEMouse.y-STARTDRAG.y)>2))
			   || ((config.input.autoReadyWeapon == false) && ((abs(MouseDragX) > 2) || (abs(MouseDragY) > 2))))
			{
				DRAGGING=1;
			}
		}
	}
	else
	{
		DRAGGING=0;
	}

	//-------------------------------------------------------------------------
	// interface
	//-------------------------------------------------------------------------
	// torch
	float px = 0;
	float py = 0;

	if (!BLOCK_PLAYER_CONTROLS)
	{
		if ((!(player.Interface & INTER_COMBATMODE)))
		{

			if (!TRUE_PLAYER_MOUSELOOK_ON)
			{
				px = INTERFACE_RATIO(InventoryX) + INTERFACE_RATIO(110);

				if (px < INTERFACE_RATIO(10)) px = INTERFACE_RATIO(10);

				py = DANAESIZY - INTERFACE_RATIO(158+32);

				if (CURRENT_TORCH != NULL)
				{
					if (MouseInRect(px,py, px + INTERFACE_RATIO(32), py + INTERFACE_RATIO(64)))
					{
						eMouseState=MOUSE_IN_TORCH_ICON;
						SpecialCursor=CURSOR_INTERACTION_ON;

						if ((LastMouseClick & 1) && (!(EERIEMouseButton & 1)) )
						{
							INTERACTIVE_OBJ * temp = CURRENT_TORCH;

							if(temp && !temp->locname.empty()) {
								
								if (((CURRENT_TORCH->ioflags & IO_ITEM) && CURRENT_TORCH->_itemdata->equipitem)
									&& (player.Full_Skill_Object_Knowledge + player.Full_Attribute_Mind
									>= CURRENT_TORCH->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_Identify_Value].value) )
								{
									SendIOScriptEvent(FlyingOverIO,SM_IDENTIFY);
								}

								WILLADDSPEECH = getLocalised(temp->locname);

								if (temp->ioflags & IO_GOLD)
								{
									std::stringstream ss;
									ss << temp->_itemdata->price << " " << WILLADDSPEECH;
									WILLADDSPEECH = ss.str();
								}

								if ((temp->poisonous>0) && (temp->poisonous_count!=0))
								{
									std::string Text = getLocalised("description_poisoned", "error");
									std::stringstream ss;
									ss << WILLADDSPEECH << " (" << Text << " " << (int)temp->poisonous << ")";
									WILLADDSPEECH = ss.str();
								}

								if ((temp->ioflags & IO_ITEM) && (temp->durability<100.f))
								{
									std::string Text = getLocalised("description_durability", "error");
									std::stringstream ss;
									ss << WILLADDSPEECH << " " << Text << " " << std::fixed << std::setw(3) << std::setprecision(0) << temp->durability << std::setw(0) << "/" << std::setw(3) << temp->max_durability;
									WILLADDSPEECH = ss.str();
								}


								WILLADDSPEECHTIME = (unsigned long)(arxtime);
							}
						}

						if  ((EERIEMouseButton & 1) && (LastMouseClick &1))
						{
							if ( (abs(DANAEMouse.x-STARTDRAG.x)>2) ||
								(abs(DANAEMouse.y-STARTDRAG.y)>2) )	DRAGGING = 1;
						}

						if ((DRAGINTER == NULL)  && (!PLAYER_MOUSELOOK_ON) && DRAGGING)
						{
							INTERACTIVE_OBJ * io=CURRENT_TORCH;
							CURRENT_TORCH->show=SHOW_FLAG_IN_SCENE;
							ARX_SOUND_PlaySFX(SND_TORCH_END);
							ARX_SOUND_Stop(SND_TORCH_LOOP);
							CURRENT_TORCH=NULL;
							SHOW_TORCH=0;
							DynLight[0].exist=0;
							Set_DragInter(io);
							DRAGINTER->ignition=1;
						}
						else
						{
							if ((EERIEMouseButton & 4) && (COMBINE == NULL))
							{
								COMBINE = CURRENT_TORCH;
							}

							if (!(EERIEMouseButton & 2) && (LastMouseClick & 2))
							{
								ARX_PLAYER_ClickedOnTorch(CURRENT_TORCH);
								EERIEMouseButton &= ~2;
								TRUE_PLAYER_MOUSELOOK_ON = false;
							}
						}
					}
				}

				// redist
				if ((player.Skill_Redistribute) || (player.Attribute_Redistribute))
				{
					px = DANAESIZX - INTERFACE_RATIO(35) + lSLID_VALUE + GL_DECAL_ICONS;
					py = DANAESIZY - INTERFACE_RATIO(218);

					if (MouseInRect(px, py, px + INTERFACE_RATIO(32), py + INTERFACE_RATIO(32)))
					{
						eMouseState=MOUSE_IN_REDIST_ICON;
						SpecialCursor=CURSOR_INTERACTION_ON;

						if ((EERIEMouseButton & 1) && !(LastMouseClick & 1))
						{
							ARX_INTERFACE_BookOpenClose(1);
							EERIEMouseButton &=~1;
						}

						return false;
					}
				}


				// gold
				if (player.gold>0)
				{
					px = DANAESIZX - INTERFACE_RATIO(35) + lSLID_VALUE + GL_DECAL_ICONS;
					py = DANAESIZY - INTERFACE_RATIO(183);

					if (MouseInRect(px,py, px + INTERFACE_RATIO(32), py + INTERFACE_RATIO(32)))
					{
						eMouseState=MOUSE_IN_GOLD_ICON;
						SpecialCursor=CURSOR_INTERACTION_ON;

						if ((player.gold > 0)
							&& (!GInput->actionPressed(CONTROLS_CUST_MAGICMODE))
							&& (COMBINE==NULL) && (!COMBINEGOLD))
						{
							if (EERIEMouseButton & 4)
								COMBINEGOLD=1;
						}

						if (DRAGINTER == NULL)
							return false;
					}
				}

				// book
				px = DANAESIZX - INTERFACE_RATIO(35) + lSLID_VALUE + GL_DECAL_ICONS;
				py = DANAESIZY - INTERFACE_RATIO(148);

				if (MouseInRect(px, py, px + INTERFACE_RATIO(32), py + INTERFACE_RATIO(32)))
				{
					eMouseState=MOUSE_IN_BOOK_ICON;
					SpecialCursor=CURSOR_INTERACTION_ON;

					if ((EERIEMouseButton & 1) && !(LastMouseClick & 1))
					{
						ARX_INTERFACE_BookOpenClose(0);
						EERIEMouseButton &=~1;
					}

					return false;
				}

				// inventaire
				px = DANAESIZX - INTERFACE_RATIO(35) + lSLID_VALUE + GL_DECAL_ICONS;
				py = DANAESIZY - INTERFACE_RATIO(113);
				static float flDelay=0;

				if (MouseInRect(px, py, px + INTERFACE_RATIO(32), py + INTERFACE_RATIO(32))||flDelay)
				{
					eMouseState=MOUSE_IN_INVENTORY_ICON;
					SpecialCursor=CURSOR_INTERACTION_ON;

					if (EERIEMouseButton & 4)
					{
						ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());

						if (player.Interface | INTER_INVENTORYALL)
						{
							OptmizeInventory(0);
							OptmizeInventory(1);
							OptmizeInventory(2);
						}
						else
						{
							OptmizeInventory(sActiveInventory);
						}

						flDelay=0;
						EERIEMouseButton&=~4;
					}
					else if (((EERIEMouseButton & 1) && !(LastMouseClick & 1)) || flDelay)
					{
						if (!flDelay)
						{
							flDelay=arxtime.get_updated();
							return false;
						}
						else
						{
							if ((arxtime.get_updated() - flDelay) < 300)
							{
								return false;
							}
							else
							{
								flDelay=0;
							}
						}

						if (player.Interface & INTER_INVENTORYALL)
						{
							ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
							bInventoryClosing = true;

						}
						else
						{
							bInverseInventory=!bInverseInventory;
							lOldTruePlayerMouseLook=TRUE_PLAYER_MOUSELOOK_ON;
						}

						EERIEMouseButton &=~1;
					}
					else if ((EERIEMouseButton & 2) && !(LastMouseClick & 2))
					{
						ARX_INTERFACE_BookOpenClose(2);
						ARX_INVENTORY_OpenClose(NULL);

						if (player.Interface & INTER_INVENTORYALL)
						{
							bInventoryClosing = true;
						}
						else
						{
							if ((player.Interface & INTER_INVENTORY))
							{
								ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
								bInventoryClosing = true;
								bInventorySwitch = true;
							}
							else
							{
								ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
								player.Interface |= INTER_INVENTORYALL;

								float fInventoryY = INTERFACE_RATIO(121.f) * (player.bag);
								InventoryY = checked_range_cast<long>(fInventoryY);

								ARX_INTERFACE_NoteClose();

								if (TRUE_PLAYER_MOUSELOOK_ON)
								{
									WILLRETURNTOFREELOOK = 1;
								}
							}
						}

						EERIEMouseButton &= ~2;
						TRUE_PLAYER_MOUSELOOK_ON = false;
					}

					if (DRAGINTER == NULL)
						return false;
				}
			}

			// steal
			if (player.Interface & INTER_STEAL)
			{
				px = static_cast<float>(-lSLID_VALUE);
				py = DANAESIZY - INTERFACE_RATIO(78 + 32);

				if (MouseInRect(px, py, px + INTERFACE_RATIO(32), py + INTERFACE_RATIO(32)))
				{
					eMouseState=MOUSE_IN_STEAL_ICON;
					SpecialCursor=CURSOR_INTERACTION_ON;

					if ((EERIEMouseButton & 1) && !(LastMouseClick & 1))
					{
						ARX_INVENTORY_OpenClose(ioSteal);

						if (player.Interface&(INTER_INVENTORY | INTER_INVENTORYALL))
						{
							ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
						}

						if (SecondaryInventory != NULL)
						{
							SendIOScriptEvent(ioSteal, SM_STEAL);

							bForceEscapeFreeLook=true;
						    lOldTruePlayerMouseLook=!TRUE_PLAYER_MOUSELOOK_ON;
						}

						EERIEMouseButton &=~1;
					}

					if (DRAGINTER == NULL)
						return false;
				}
			}
		}
	}

	// gros player book
	if (player.Interface & INTER_MAP)
	{
		px = 97 * Xratio;
		py = 64 * Yratio;
		
		TextureContainer* playerbook = ITC.Get("playerbook");
		if (playerbook)
		{
			if (MouseInRect(px, py, px + playerbook->m_dwWidth * Xratio, py + playerbook->m_dwHeight * Yratio))
			{
				eMouseState = MOUSE_IN_BOOK;
			}
		}
	}
	
	// gros book/note
	if(player.Interface & INTER_NOTE) {
		if(openNote.area().contains(Vec2f(DANAEMouse))) {
			eMouseState = MOUSE_IN_NOTE;
			return false;
		}
	}
	
	if (!PLAYER_INTERFACE_HIDE_COUNT && (TSecondaryInventory!=NULL))
	{
		px = INTERFACE_RATIO(InventoryX) + INTERFACE_RATIO(16);
		py = INTERFACE_RATIO_DWORD(BasicInventorySkin->m_dwHeight) - INTERFACE_RATIO(16);
		INTERACTIVE_OBJ * temp=(INTERACTIVE_OBJ *)TSecondaryInventory->io;

		if (temp && !(temp->ioflags & IO_SHOP) && !(temp == ioSteal))
		{
			if (MouseInRect(px,py, px + INTERFACE_RATIO(16), py + INTERFACE_RATIO(16)))
			{
				eMouseState = MOUSE_IN_INVENTORY_PICKALL_ICON;
				SpecialCursor=CURSOR_INTERACTION_ON;

				if ((EERIEMouseButton & 1) && !(LastMouseClick & 1))
				{
					if (TSecondaryInventory)
					{
						// play un son que si un item est pris
						ARX_INVENTORY_TakeAllFromSecondaryInventory();
					}

					EERIEMouseButton &=~1;
				}

				if (DRAGINTER == NULL)
					return false;
			}
		}

		//py = 20;
		px = INTERFACE_RATIO(InventoryX) + INTERFACE_RATIO_DWORD(BasicInventorySkin->m_dwWidth) - INTERFACE_RATIO(32);

		if (MouseInRect(px,py, px + INTERFACE_RATIO(16), py + INTERFACE_RATIO(16)))
		{
			eMouseState = MOUSE_IN_INVENTORY_CLOSE_ICON;
			SpecialCursor=CURSOR_INTERACTION_ON;

			if ((EERIEMouseButton & 1) && !(LastMouseClick & 1))
			{
				INTERACTIVE_OBJ * io = NULL;

				if (SecondaryInventory!=NULL)
				{
					io = (INTERACTIVE_OBJ *)SecondaryInventory->io;
				}
				else if (player.Interface & INTER_STEAL)
				{
					io = ioSteal;
				}

				if (io!=NULL)
				{
					ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
					InventoryDir=-1;
					SendIOScriptEvent(io,SM_INVENTORY2_CLOSE);
					TSecondaryInventory=SecondaryInventory;
					SecondaryInventory=NULL;
				}

				EERIEMouseButton &=~1;
			}

			if (DRAGINTER == NULL)
				return false;
		}
	}

	//-------------------------------------------------------------------------


	// Single Click On Object
	if ( ( LastMouseClick & 1 ) && ( !( EERIEMouseButton & 1 ) ) )
	{
		if ( FlyingOverIO && ( !DRAGINTER ) )
		{
			SendIOScriptEvent( FlyingOverIO, SM_CLICKED );
			bool bOk = true;

			if ( SecondaryInventory != NULL )
			{
				INTERACTIVE_OBJ * temp = (INTERACTIVE_OBJ *)SecondaryInventory->io;

				if (IsInSecondaryInventory(FlyingOverIO))
					if ( temp->ioflags & IO_SHOP )
						bOk = false;
			}

			if ( !( FlyingOverIO->ioflags & IO_MOVABLE ) )
				if ( ( FlyingOverIO->ioflags & IO_ITEM ) && bOk )
				{
					if ( GInput->actionPressed( CONTROLS_CUST_STEALTHMODE ) )
					{
						if ( !InPlayerInventoryPos( &DANAEMouse ) && !ARX_INTERFACE_MouseInBook() )
						{
							long sx, sy;
							bool bSecondary = false;

							sx = sy = 0 ;

							if ( TSecondaryInventory && IsInSecondaryInventory( FlyingOverIO ) )
							{
								if ( SecondaryInventory )
								{
									bool bfound = true;

									for ( long j = 0 ; j < SecondaryInventory->sizey && bfound ; j++ )
									{
										for ( long i = 0 ; i < SecondaryInventory->sizex && bfound ; i++ )
										{
											if ( SecondaryInventory->slot[i][j].io == FlyingOverIO )
											{
												sx		= i;
												sy		= j;
												bfound	= false;
											}
										}
									}

									if (bfound) ARX_DEAD_CODE();
								}

								bSecondary = true;
							}

							RemoveFromAllInventories( FlyingOverIO );
							FlyingOverIO->show = SHOW_FLAG_IN_INVENTORY;

							if ( FlyingOverIO->ioflags & IO_GOLD )
							{
								ARX_SOUND_PlayInterface( SND_GOLD );
							}

							ARX_SOUND_PlayInterface( SND_INVSTD );

							if ( !CanBePutInInventory( FlyingOverIO ) )
							{
								if ( TSecondaryInventory && bSecondary )
								{
									extern short sInventory, sInventoryX, sInventoryY;
									sInventory = 2;
									sInventoryX = checked_range_cast<short>(sx);
									sInventoryY = checked_range_cast<short>(sy);

									CanBePutInSecondaryInventory( TSecondaryInventory, FlyingOverIO, &sx, &sy );
								}

								if ( !bSecondary )
								{
									FlyingOverIO->show = SHOW_FLAG_IN_SCENE;
								}
							}

							if (DRAGINTER == FlyingOverIO)
							{
								DRAGINTER = NULL;
							}

							FlyingOverIO = NULL;
						}
					}
				}
		}
	}

	if (!(player.Interface & INTER_COMBATMODE))
	{
		// Dropping an Interactive Object that has been dragged
		if ((!(EERIEMouseButton & 1)) && (LastMouseClick & 1) && (DRAGINTER!=NULL))
		{
			//if (ARX_EQUIPMENT_PutOnPlayer(DRAGINTER))
			if (InInventoryPos(&DANAEMouse)) // Attempts to put it in inventory
			{
				if (!PutInInventory())
				{
				}
			}
			else if (eMouseState == MOUSE_IN_INVENTORY_ICON)
			{
				if (!PutInInventory())
				{
				}
			}
			else if (ARX_INTERFACE_MouseInBook())
			{
				if (Book_Mode == BOOKMODE_STATS)
				{
					SendIOScriptEvent(DRAGINTER,SM_INVENTORYUSE);
					COMBINE=NULL;
				}
			} else if (DRAGINTER->ioflags & IO_GOLD) {
				ARX_PLAYER_AddGold(DRAGINTER);
				Set_DragInter(NULL);
				
			} else if(DRAGINTER) {
#ifdef BUILD_EDITOR
				if (!EDITMODE) // test for NPC & FIX
				{
					if ((DRAGINTER->ioflags & IO_NPC) || (DRAGINTER->ioflags & IO_FIX) )
					{
						Set_DragInter(NULL);
						goto suivant2;
					}
				}
#endif

				if (!((DRAGINTER->ioflags & IO_ITEM) && (DRAGINTER->_itemdata->count > 1)))
					if ((DRAGINTER->obj) && (DRAGINTER->obj->pbox))
					{

						if (
							(!InInventoryPos(&DANAEMouse))) //Put object in fromt of player
						{
							if (ARX_MOUSE_OVER & ARX_MOUSE_OVER_BOOK)
								 goto suivant2;

							long res=Manage3DCursor(0);

						if (res==0) // Throw Object
						{
							INTERACTIVE_OBJ * io=DRAGINTER;
							ARX_PLAYER_Remove_Invisibility();
							io->obj->pbox->active=1;
							io->obj->pbox->stopcount=0;
							Vec3f pos;
								pos.x = io->pos.x = player.pos.x;
								pos.z = io->pos.z = player.pos.z;
							pos.y=io->pos.y=player.pos.y+80.f;
							io->velocity.x=0.f;
							io->velocity.y=0.f;
							io->velocity.z=0.f;
							io->stopped=1;
							float y_ratio=(float)((float)DANAEMouse.y-(float)DANAECENTERY)/(float)DANAESIZY*2;
							float x_ratio=-(float)((float)DANAEMouse.x-(float)DANAECENTERX)/(float)DANAECENTERX;
							Vec3f viewvector;
							viewvector.x=-(float)EEsin(radians(player.angle.b+(x_ratio*30.f)))*EEcos(radians(player.angle.a));
								viewvector.y = EEsin(radians(player.angle.a)) + y_ratio; 
							viewvector.z= (float)EEcos(radians(player.angle.b+(x_ratio*30.f)))*EEcos(radians(player.angle.a));
							io->soundtime=0;
							io->soundcount=0;
							EERIE_PHYSICS_BOX_Launch(io->obj,&pos,&viewvector);
							ARX_SOUND_PlaySFX(SND_WHOOSH, &pos);
							io->show=SHOW_FLAG_IN_SCENE;
							Set_DragInter(NULL);
						}
				}
						}

			suivant2:
				;
						}
					}

	if (COMBINE)
	{
		if ((!CURRENT_TORCH) || (CURRENT_TORCH && (COMBINE != CURRENT_TORCH)))
		{
			Vec3f pos;

			if (GetItemWorldPosition(COMBINE,&pos))
			{
				if(fartherThan(pos, player.pos, 300.f))
					COMBINE=NULL;
			}
			else COMBINE=NULL;
		}
	}

	if ((EERIEMouseButton & 1) && !(LastMouseClick & 1) && ((COMBINE !=NULL) || COMBINEGOLD))
	{
			ReleaseInfosCombine();

		INTERACTIVE_OBJ * io;

		if ((io=FlyingOverIO)!=NULL)
		{
			if (COMBINEGOLD)
			{
				char temp[256];
				strcpy(temp,"gold_coin");
				SendIOScriptEvent(io,SM_COMBINE,temp);
			}
			else
			{
				if (io!=COMBINE)
				{
					std::string temp = COMBINE->long_name();
					EVENT_SENDER=COMBINE;

					if (!specialstrcmp( COMBINE->short_name(),"keyring"))
						ARX_KEYRING_Combine(io);
					else
						SendIOScriptEvent(io,SM_COMBINE,temp);
				}
			}
		}

		else // GLights
		{
			float fMaxdist = 300;

			if (Project.telekinesis) fMaxdist = 850;

			for(size_t i = 0; i < MAX_LIGHTS; i++) {
				if ((GLight[i]!=NULL) &&
					(GLight[i]->exist) &&
					!fartherThan(GLight[i]->pos, player.pos, fMaxdist) &&
					(!(GLight[i]->extras & EXTRAS_NO_IGNIT)))
				{
					if (MouseInRect(GLight[i]->mins.x, GLight[i]->mins.y, GLight[i]->maxs.x, GLight[i]->maxs.y))
					{
						if (COMBINE->ioflags & IO_ITEM)
						{
							if ((COMBINE == CURRENT_TORCH) || (COMBINE->_itemdata->LightValue == 1))
							{
								if (GLight[i]->status != 1)
								{
									GLight[i]->status = 1;
									ARX_SOUND_PlaySFX(SND_TORCH_START, &GLight[i]->pos);
								}
							}

							if (COMBINE->_itemdata->LightValue == 0)
							{
								if (GLight[i]->status != 0)
								{
									GLight[i]->status = 0;
									ARX_SOUND_PlaySFX(SND_TORCH_END, &GLight[i]->pos);
									SendIOScriptEvent(COMBINE, SM_CUSTOM, "douse");
								}
							}
						}
					}
				}
			}
		}

		COMBINEGOLD=0;
		bool bQuitCombine = true;

		if ((player.Interface & INTER_INVENTORY))
		{
			if (player.bag)
			{

					float fCenterX	= DANAECENTERX + INTERFACE_RATIO(-320 + 35) + INTERFACE_RATIO_DWORD(ITC.Get("hero_inventory")->m_dwWidth) - INTERFACE_RATIO(32 + 3) ;
					float fSizY		= DANAESIZY - INTERFACE_RATIO(101) + INTERFACE_RATIO_LONG(InventoryY) + INTERFACE_RATIO(- 3 + 25) ;


				float posx = ARX_CAST_TO_INT_THEN_FLOAT( fCenterX );
				float posy = ARX_CAST_TO_INT_THEN_FLOAT( fSizY );

				if (sActiveInventory > 0)
				{
					if (MouseInRect(posx, posy, posx+INTERFACE_RATIO(32), posy+INTERFACE_RATIO(32)))
						bQuitCombine = false;
				}

				if (sActiveInventory < player.bag-1)
				{

					float fRatio	= INTERFACE_RATIO(32 + 5);

					posy += checked_range_cast<int>(fRatio);

					if (MouseInRect(posx, posy, posx+INTERFACE_RATIO(32), posy+INTERFACE_RATIO(32)))
						bQuitCombine = false;
				}
			}
		}

		if (bQuitCombine)
		{
			COMBINE=NULL;
			EERIEMouseButton &= ~1;
		}
	}

		//lights
	if (COMBINE)
	{
		float fMaxdist = 300;

		if (Project.telekinesis) fMaxdist = 850;

		for(size_t i = 0; i < MAX_LIGHTS; i++) {
			if ((GLight[i]!=NULL) &&
				(GLight[i]->exist) &&

				!fartherThan(GLight[i]->pos, player.pos, fMaxdist) &&
				(!(GLight[i]->extras & EXTRAS_NO_IGNIT)))
			{
				if (MouseInRect(GLight[i]->mins.x, GLight[i]->mins.y, GLight[i]->maxs.x, GLight[i]->maxs.y))
				{
					SpecialCursor = CURSOR_INTERACTION_ON;
				}
			}
		}
	}

	// Double Clicked and not already combining.
	if ((EERIEMouseButton & 4) && (COMBINE==NULL))
	{
		long accept_combine=1;

		if ((SecondaryInventory!=NULL) && (InSecondaryInventoryPos(&DANAEMouse)))
		{
			INTERACTIVE_OBJ * io=(INTERACTIVE_OBJ *)SecondaryInventory->io;

			if (io->ioflags & IO_SHOP) accept_combine=0;
		}

		if (accept_combine)
		{

			if ((FlyingOverIO) &&
				( (FlyingOverIO->ioflags & IO_ITEM) && !(FlyingOverIO->ioflags & IO_MOVABLE)))
			{
				COMBINE=FlyingOverIO;
				GetInfosCombine();
				EERIEMouseButton&=~4;
			}
			else if (InInventoryPos(&DANAEMouse)) EERIEMouseButton&=4;
		}
	}

	// Checks for Object Dragging
	if (!EDITMODE)
		if (( DRAGGING && !PLAYER_MOUSELOOK_ON &&
			(!GInput->actionPressed(CONTROLS_CUST_MAGICMODE)) &&
			(DRAGINTER==NULL)
			)
			|| // mode system shock
			( DRAGGING && (config.input.autoReadyWeapon == false) &&
			(!GInput->actionPressed(CONTROLS_CUST_MAGICMODE)) &&
			(DRAGINTER==NULL))
			)
		{
			if ( !TakeFromInventory(&STARTDRAG))
			{
				bool bOk = false;

				INTERACTIVE_OBJ *io = InterClick(&STARTDRAG);

				if (io && !BLOCK_PLAYER_CONTROLS)
				{
					if (ARX_MOUSE_OVER & ARX_MOUSE_OVER_BOOK)
					{
						if (io->show == SHOW_FLAG_ON_PLAYER)
							bOk = true;
					}
					else
					{
						bOk = true;
					}
				}

				if (bOk)
				{
					Set_DragInter(io);

					if (io!=NULL)
					{
						ARX_PLAYER_Remove_Invisibility();

						if (DRAGINTER->show==SHOW_FLAG_ON_PLAYER)
						{
							ARX_EQUIPMENT_UnEquip(inter.iobj[0],DRAGINTER);
							RemoveFromAllInventories(DRAGINTER);
							DRAGINTER->bbox2.x=-1;
									}

						if (!EDITMODE)
						{
							if ((io->ioflags & IO_NPC) || (io->ioflags & IO_FIX))
							{
								Set_DragInter(NULL);
								goto suivant;
							}
						}

						if (io->ioflags & IO_UNDERWATER)
						{
							io->ioflags&=~IO_UNDERWATER;
							ARX_SOUND_PlayInterface(SND_PLOUF, 0.8F + 0.4F * rnd());
						}

						DRAGINTER->show=SHOW_FLAG_NOT_DRAWN;
						ARX_SOUND_PlayInterface(SND_INVSTD);
					}
				}
				else
					Set_DragInter(NULL);

				suivant:
					;
			}
			else ARX_PLAYER_Remove_Invisibility();
		}
	
	}
	
	return false;
}

//-----------------------------------------------------------------------------
// Switches from/to combat Mode i=-1 for switching from actual configuration
// 2 to force Draw Weapon
// 1 to force Combat Mode On
// 0 to force Combat Mode Off
//-----------------------------------------------------------------------------
void ARX_INTERFACE_Combat_Mode(long i)
{
	if ( (i>=1) &&  (player.Interface & INTER_COMBATMODE) ) return;

	if ( (i==0) &&  !(player.Interface & INTER_COMBATMODE) ) return;

	if (EDITMODE) return;

	if  ((player.Interface & INTER_COMBATMODE) && (inter.iobj[0]))
	{
		player.Interface&=~INTER_COMBATMODE;
		player.Interface&=~INTER_NO_STRIKE;

		ARX_EQUIPMENT_LaunchPlayerUnReadyWeapon();
		long weapontype=ARX_EQUIPMENT_GetPlayerWeaponType();

		if (inter.iobj[0] && arrowobj && (weapontype == WEAPON_BOW))
		{
			EERIE_LINKEDOBJ_UnLinkObjectFromObject(inter.iobj[0]->obj, arrowobj);
		}

		player.doingmagic=0;
	}

	else  if (((inter.iobj[0]->animlayer[1].cur_anim == NULL) || (inter.iobj[0]->animlayer[1].cur_anim == inter.iobj[0]->anims[ANIM_WAIT]))
		&& (inter.iobj[0]))
	{
		ARX_INTERFACE_BookOpenClose(2);

		player.Interface|=INTER_COMBATMODE;

		if (i==2)
		{
			player.Interface|=INTER_NO_STRIKE;

			if (config.input.mouseLookToggle)
			{
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

extern long DISABLE_JUMP;
extern unsigned long REQUEST_JUMP;
//-----------------------------------------------------------------------------
void ArxGame::ManagePlayerControls()
{
	if (((EERIEMouseButton & 4) && (!EDITMODE) && !(player.Interface & INTER_COMBATMODE))
		&& (!player.doingmagic)
		&& (!(ARX_MOUSE_OVER & ARX_MOUSE_OVER_BOOK))
		&& (eMouseState != MOUSE_IN_NOTE)
		)
	{
		INTERACTIVE_OBJ * t;

		t=InterClick(&DANAEMouse);

		if (t!=NULL)
		{
			if (t->ioflags & IO_NPC)
			{
				if (t->script.data!=NULL)
				{
					if (t->_npcdata->life>0.f)
					{
						SendIOScriptEvent(t,SM_CHAT);
						EERIEMouseButton&=~4;

						if (DRAGGING) DRAGGING = 0;
					}
					else
					{
						if (t->inventory!=NULL)
						{
							if (player.Interface & INTER_STEAL)
								if (ioSteal && t != ioSteal)
								{
									SendIOScriptEvent(ioSteal, SM_STEAL,"off");
									player.Interface &= ~INTER_STEAL;
								}

								ARX_INVENTORY_OpenClose(t);

								if (player.Interface&(INTER_INVENTORY | INTER_INVENTORYALL))
								{
									ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
								}

								if (SecondaryInventory)
								{
									bForceEscapeFreeLook=true;
									lOldTruePlayerMouseLook=!TRUE_PLAYER_MOUSELOOK_ON;
								}
						}
					}
				}
			}
			else
			{
				if (t->inventory!=NULL)
				{
					if (player.Interface & INTER_STEAL)
					{
						if (ioSteal && t != ioSteal)
						{
							SendIOScriptEvent(ioSteal, SM_STEAL,"off");
							player.Interface &= ~INTER_STEAL;
						}
					}

					ARX_INVENTORY_OpenClose(t);

					if (SecondaryInventory)
					{
						bForceEscapeFreeLook=true;
						lOldTruePlayerMouseLook=!TRUE_PLAYER_MOUSELOOK_ON;
					}
				}
				else if (t->script.data!=NULL)
				{
					SendIOScriptEvent(t,SM_ACTION);
				}

				EERIEMouseButton&=~4;

				if (DRAGGING) DRAGGING = 0;

				EERIEMouseButton = 0;
			}

			EERIEMouseButton&=~4;
			EERIEMouseButton = 0;
		}
	}

	float MoveDiv;

	Vec3f tm;
	tm.x=tm.y=tm.z=0.f;

	// Checks STEALTH Key Status.
	if (GInput->actionPressed(CONTROLS_CUST_STEALTHMODE) )
	{
		MoveDiv=0.02f;
		player.Current_Movement|=PLAYER_MOVE_STEALTH;
	}
	else MoveDiv=0.0333333f;

	{
		long NOMOREMOVES=0;
		float FD = 1.f;

		if (eyeball.exist==2)
		{
			FD=18.f;
			Vec3f old = eyeball.pos;

			// Checks WALK_FORWARD Key Status.
			if (GInput->actionPressed(CONTROLS_CUST_WALKFORWARD) )
			{
				float tr=radians(eyeball.angle.b);
				eyeball.pos.x+=-(float)EEsin(tr)*20.f*(float)FD*0.033f;
				eyeball.pos.z+=+(float)EEcos(tr)*20.f*(float)FD*0.033f;
				NOMOREMOVES=1;
			}

			// Checks WALK_BACKWARD Key Status.
			if (GInput->actionPressed(CONTROLS_CUST_WALKBACKWARD) )
			{
				float tr=radians(eyeball.angle.b);
				eyeball.pos.x+=(float)EEsin(tr)*20.f*(float)FD*0.033f;
				eyeball.pos.z+=-(float)EEcos(tr)*20.f*(float)FD*0.033f;
				NOMOREMOVES=1;
			}

			// Checks STRAFE_LEFT Key Status.
			if( (GInput->actionPressed(CONTROLS_CUST_STRAFELEFT)||
				(GInput->actionPressed(CONTROLS_CUST_STRAFE)&&GInput->actionPressed(CONTROLS_CUST_TURNLEFT)))
				&& !NOMOREMOVES)
			{
				float tr=radians(MAKEANGLE(eyeball.angle.b+90.f));
				eyeball.pos.x+=-(float)EEsin(tr)*10.f*(float)FD*0.033f;
				eyeball.pos.z+=+(float)EEcos(tr)*10.f*(float)FD*0.033f;
				NOMOREMOVES=1;			
			}

			// Checks STRAFE_RIGHT Key Status.
			if( (GInput->actionPressed(CONTROLS_CUST_STRAFERIGHT)||
				(GInput->actionPressed(CONTROLS_CUST_STRAFE)&&GInput->actionPressed(CONTROLS_CUST_TURNRIGHT)))
				&& !NOMOREMOVES)
			{
				float tr=radians(MAKEANGLE(eyeball.angle.b-90.f));
				eyeball.pos.x+=-(float)EEsin(tr)*10.f*(float)FD*0.033f;
				//eyeball.pos.y+=FD*0.33f;
				eyeball.pos.z+=(float)EEcos(tr)*10.f*(float)FD*0.033f;
				NOMOREMOVES=1;
			}

			IO_PHYSICS phys;
			phys.cyl.height=-110.f;
			phys.cyl.origin.x=eyeball.pos.x;
			phys.cyl.origin.y=eyeball.pos.y+70.f;
			phys.cyl.origin.z=eyeball.pos.z;
			phys.cyl.radius=45.f;

			EERIE_CYLINDER test;
			memcpy(&test,&phys.cyl,sizeof(EERIE_CYLINDER));
			bool npc = AttemptValidCylinderPos(&test, NULL, CFLAG_JUST_TEST | CFLAG_NPC);
			float val=CheckAnythingInCylinder(&phys.cyl,inter.iobj[0],CFLAG_NO_NPC_COLLIDE | CFLAG_JUST_TEST);

			if ((val > -40.f))
			{
				if (val <= 70.f)
				{
					eyeball.pos.y+=val-70.f;
				}

				if (!npc)
				{
					MagicSightFader+=_framedelay*( 1.0f / 200 );

					if (MagicSightFader>1.f) MagicSightFader=1.f;
				}
			}
			else
			{
				eyeball.pos = old;
			}
		}

		float t;
		float multi;

		if (EDITMODE || arxtime.is_paused()) FD=40.f;

		bool left=GInput->actionPressed(CONTROLS_CUST_STRAFELEFT);

		if(!left)
		{
			if(GInput->actionPressed(CONTROLS_CUST_STRAFE)&&GInput->actionPressed(CONTROLS_CUST_TURNLEFT))
			{
				left=true;
			}
		}

		bool right=GInput->actionPressed(CONTROLS_CUST_STRAFERIGHT);

		if(!right)
		{
			if(GInput->actionPressed(CONTROLS_CUST_STRAFE)&&GInput->actionPressed(CONTROLS_CUST_TURNRIGHT))
			{
				right=true;
			}
		}

		// Checks WALK_BACKWARD Key Status.
		if (	GInput->actionPressed(CONTROLS_CUST_WALKBACKWARD)
			&&	!NOMOREMOVES	)
		{
			CurrFightPos=3;
			multi = 1;

			if (MoveDiv==0.02f) MOVETYPE=MOVE_WALK;
			else MOVETYPE=MOVE_RUN;

			if (left || right)
			{
				multi = 0.8f;
			}

			if (Project.improvespeed) multi+=1.2f;

			t=radians(player.angle.b);
			multi=5.f*(float)FD*MoveDiv*multi;
			tm.x+=(float)EEsin(t)*multi;
			tm.z-=(float)EEcos(t)*multi;

			if(!USE_PLAYERCOLLISIONS)
			{
				t=radians(player.angle.a);
				tm.y-=(float)EEsin(t)*multi;
			}

			player.Current_Movement|=PLAYER_MOVE_WALK_BACKWARD;

			if (GInput->actionNowPressed(CONTROLS_CUST_WALKBACKWARD) )
				MOVE_PRECEDENCE=PLAYER_MOVE_WALK_BACKWARD;
		}
		else if (MOVE_PRECEDENCE==PLAYER_MOVE_WALK_BACKWARD) MOVE_PRECEDENCE=0;

		// Checks WALK_FORWARD Key Status.
		if (GInput->actionPressed(CONTROLS_CUST_WALKFORWARD)
			&& !NOMOREMOVES)
		{
			CurrFightPos=2;
			multi = 1;

			if (MoveDiv==0.02f) MOVETYPE=MOVE_WALK;
			else MOVETYPE=MOVE_RUN;

			if (left || right)
			{
				multi=0.8f;
			}

			if (Project.improvespeed) multi+=1.5;

			t=radians(player.angle.b);
			multi=10.f*(float)FD*MoveDiv*multi;
			tm.x-=(float)EEsin(t)*multi;
			tm.z+=(float)EEcos(t)*multi;

			if(!USE_PLAYERCOLLISIONS)
			{
				t=radians(player.angle.a);
				tm.y+=(float)EEsin(t)*multi;
			}

			player.Current_Movement|=PLAYER_MOVE_WALK_FORWARD;

			if (GInput->actionNowPressed(CONTROLS_CUST_WALKFORWARD) )
				MOVE_PRECEDENCE=PLAYER_MOVE_WALK_FORWARD;
		}
		else if (MOVE_PRECEDENCE==PLAYER_MOVE_WALK_FORWARD) MOVE_PRECEDENCE=0;

		// Checks STRAFE_LEFT Key Status.
		if (left && !NOMOREMOVES)
		{
			CurrFightPos=0;
			t=radians(MAKEANGLE(player.angle.b+90.f));
			multi=6.f*(float)FD*MoveDiv;
			tm.x-=(float)EEsin(t)*multi;
			tm.z+=(float)EEcos(t)*multi;

			player.Current_Movement|=PLAYER_MOVE_STRAFE_LEFT;

			if (GInput->actionNowPressed(CONTROLS_CUST_STRAFELEFT) )
				MOVE_PRECEDENCE=PLAYER_MOVE_STRAFE_LEFT;
		}
		else if (MOVE_PRECEDENCE==PLAYER_MOVE_STRAFE_LEFT) MOVE_PRECEDENCE=0;

		// Checks STRAFE_RIGHT Key Status.
		if ( right && !NOMOREMOVES)
		{
			CurrFightPos=1;
			t=radians(MAKEANGLE(player.angle.b-90.f));
			multi=6.f*(float)FD*MoveDiv;
			tm.x-=(float)EEsin(t)*multi;
			tm.z+=(float)EEcos(t)*multi;

			player.Current_Movement|=PLAYER_MOVE_STRAFE_RIGHT;

			if (GInput->actionNowPressed(CONTROLS_CUST_STRAFERIGHT) )
				MOVE_PRECEDENCE=PLAYER_MOVE_STRAFE_RIGHT;
		}
		else if (MOVE_PRECEDENCE==PLAYER_MOVE_STRAFE_RIGHT) MOVE_PRECEDENCE=0;

		moveto.x=player.pos.x+tm.x;
		moveto.y=player.pos.y+tm.y;
		moveto.z=player.pos.z+tm.z;
	}

	// To remove for FINAL_RELEASE---------------------------------------
#ifdef BUILD_EDITOR
	if(ALLOW_CHEATS || GAME_EDITOR) {
#else
	if(ALLOW_CHEATS) {
#endif
		if (GInput->isKeyPressedNowPressed(Keyboard::Key_NumPad5))
		{
			moveto.y=player.pos.y=FirstPolyPosY(player.pos.x,player.pos.z)-180.f;
			player.angle.a=0.f;
			player.desiredangle.a=0.f;
		}

		if (GInput->isKeyPressed(Keyboard::Key_A) && (GInput->isKeyPressed(Keyboard::Key_LeftShift) || GInput->isKeyPressed(Keyboard::Key_RightShift)))
		{
			BLOCK_PLAYER_CONTROLS=0;
			player.life=player.Full_maxlife;
			player.mana=player.Full_maxmana;
			player.poison=0.f;
			player.hunger=100;
			DeadTime=0;
			ARX_SOUND_PlayInterface(SND_PLAYER_FILLLIFEMANA, 0.9F + 0.2F * rnd());
		}
	}

	// End of things to remove-------------------------------------------

	// Checks CROUCH Key Status.
	if (GInput->actionNowPressed(CONTROLS_CUST_CROUCHTOGGLE))
	{
		bGCroucheToggle=!bGCroucheToggle;
	}

	if(	GInput->actionPressed(CONTROLS_CUST_CROUCH)||
		bGCroucheToggle )
	{
		player.Current_Movement|=PLAYER_CROUCH;
	}

	if (GInput->actionNowPressed(CONTROLS_CUST_UNEQUIPWEAPON))
	{
		ARX_EQUIPMENT_UnEquipPlayerWeapon();
	}

	// Can only lean outside of combat mode
	if (!(player.Interface & INTER_COMBATMODE))
	{
		// Checks LEAN_LEFT Key Status.
		if (GInput->actionPressed(CONTROLS_CUST_LEANLEFT) )
		{
			player.Current_Movement|=PLAYER_LEAN_LEFT;
		}

		// Checks LEAN_RIGHT Key Status.
		if (GInput->actionPressed(CONTROLS_CUST_LEANRIGHT) )
		{
			player.Current_Movement|=PLAYER_LEAN_RIGHT;
		}
	}

	// Checks JUMP Key Status.
	if ((player.jumpphase==0) &&
		GInput->actionNowPressed(CONTROLS_CUST_JUMP) )
	{
		REQUEST_JUMP = (unsigned long)(arxtime);
	}


	// MAGIC
	if (GInput->actionPressed(CONTROLS_CUST_MAGICMODE))
	{
		if (!(player.Current_Movement & PLAYER_CROUCH) && (!BLOCK_PLAYER_CONTROLS)
			&& (ARXmenu.currentmode==AMCM_OFF))
		{
			if (!ARX_SOUND_IsPlaying(SND_MAGIC_AMBIENT))
				ARX_SOUND_PlaySFX(SND_MAGIC_AMBIENT, NULL, 1.0F, ARX_SOUND_PLAY_LOOPED);
		}
	}
	else
	{
		ARX_SOUND_Stop(SND_MAGIC_AMBIENT);
		ARX_SOUND_Stop(SND_MAGIC_DRAW);
	}

	if (GInput->actionNowPressed(CONTROLS_CUST_DRINKPOTIONLIFE))
	{
		SendInventoryObjectCommand("graph/obj3d/textures/item_potion_life", SM_INVENTORYUSE);
	}

	if (GInput->actionNowPressed(CONTROLS_CUST_DRINKPOTIONMANA))
	{
		SendInventoryObjectCommand("graph/obj3d/textures/item_potion_mana", SM_INVENTORYUSE);
	}

	if (GInput->actionNowPressed(CONTROLS_CUST_TORCH))
	{
		if (CURRENT_TORCH)
		{
			ARX_PLAYER_KillTorch();
		}
		else
		{
			INTERACTIVE_OBJ * io = ARX_INVENTORY_GetTorchLowestDurability();

			if (io)
			{
				INTERACTIVE_OBJ * ioo = io;

				if (io->_itemdata->count>1)
				{
					ioo=CloneIOItem(io);
					MakeTemporaryIOIdent(ioo);
					ioo->show=SHOW_FLAG_NOT_DRAWN;
					ioo->scriptload=1;
					ioo->_itemdata->count=1;
					io->_itemdata->count--;
				}

				ARX_PLAYER_ClickedOnTorch(ioo);
			}
		}
	}

	if (GInput->actionNowPressed(CONTROLS_CUST_MINIMAP))
	{
		SHOW_INGAME_MINIMAP=!SHOW_INGAME_MINIMAP;
	}

	if (GInput->actionNowPressed(CONTROLS_CUST_PREVIOUS))
	{
		if (eMouseState == MOUSE_IN_BOOK)
		{
			if (player.Interface & INTER_MAP)
			{
				if (Book_Mode > BOOKMODE_STATS)
				{
					if (Book_Mode == BOOKMODE_QUESTS)
						Book_Mode = BOOKMODE_MINIMAP;
					else if (Book_Mode == BOOKMODE_MINIMAP)
						Book_Mode = BOOKMODE_SPELLS;
					else if (Book_Mode == BOOKMODE_SPELLS)
						Book_Mode = BOOKMODE_STATS;
					ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
				}
			}
		}
		else if (InPlayerInventoryPos(&DANAEMouse))
		{
			if (!PLAYER_INTERFACE_HIDE_COUNT)
			{
				if ((player.Interface & INTER_INVENTORY))
				{
					if (player.bag)
					{
						if (sActiveInventory > 0)
						{
							ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
							sActiveInventory --;
						}
					}
				}
			}
		}
		else
		{
			if (player.Interface & INTER_MAP)
			{
				if (Book_Mode > BOOKMODE_STATS)
				{
					if (Book_Mode == BOOKMODE_QUESTS)
						Book_Mode = BOOKMODE_MINIMAP;
					else if (Book_Mode == BOOKMODE_MINIMAP)
						Book_Mode = BOOKMODE_SPELLS;
					else if (Book_Mode == BOOKMODE_SPELLS)
						Book_Mode = BOOKMODE_STATS;
					ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
				}
			}
			else
			{
				if (!PLAYER_INTERFACE_HIDE_COUNT)
				{
					if ((player.Interface & INTER_INVENTORY))
					{
						if (player.bag)
						{
							if (sActiveInventory > 0)
							{
								ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
								sActiveInventory --;
							}
						}
					}
				}
			}
		}
	}

	if (GInput->actionNowPressed(CONTROLS_CUST_NEXT))
	{
		if (eMouseState == MOUSE_IN_BOOK)
		{
			if (player.Interface & INTER_MAP)
			{
				if (Book_Mode < BOOKMODE_QUESTS)
				{
					if (Book_Mode == BOOKMODE_STATS)
						Book_Mode = BOOKMODE_SPELLS;
					else if (Book_Mode == BOOKMODE_SPELLS)
						Book_Mode = BOOKMODE_MINIMAP;
					else if (Book_Mode == BOOKMODE_MINIMAP)
						Book_Mode = BOOKMODE_QUESTS;
					ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
				}
			}
		}
		else if (InPlayerInventoryPos(&DANAEMouse))
		{
			if (!PLAYER_INTERFACE_HIDE_COUNT)
			{
				if ((player.Interface & INTER_INVENTORY))
				{
					if (player.bag)
					{
						if (sActiveInventory < player.bag - 1)
						{
							ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
							sActiveInventory ++;
						}
					}
				}
			}
		}
		else
		{
			if (player.Interface & INTER_MAP)
			{
				if (Book_Mode < BOOKMODE_QUESTS)
				{
					if (Book_Mode == BOOKMODE_STATS)
						Book_Mode = BOOKMODE_SPELLS;
					else if (Book_Mode == BOOKMODE_SPELLS)
						Book_Mode = BOOKMODE_MINIMAP;
					else if (Book_Mode == BOOKMODE_MINIMAP)
						Book_Mode = BOOKMODE_QUESTS;
					ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
				}
			}
			else
			{
				if (!PLAYER_INTERFACE_HIDE_COUNT)
				{
					if ((player.Interface & INTER_INVENTORY))
					{
						if (player.bag)
						{
							if (sActiveInventory < player.bag - 1)
							{
								ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
								sActiveInventory ++;
							}
						}
					}
				}
			}
		}
	}

	if (GInput->actionNowPressed(CONTROLS_CUST_BOOKCHARSHEET))
	{
		if (!(player.Interface & INTER_MAP))
		{
			Book_Mode = BOOKMODE_STATS;
			ARX_INTERFACE_BookOpenClose(0);
		}
		else if ((player.Interface & INTER_MAP) && (Book_Mode != BOOKMODE_STATS))
			Book_Mode = BOOKMODE_STATS;
		else
		{
			ARX_INTERFACE_BookOpenClose(2);
		}
	}

	if (GInput->actionNowPressed(CONTROLS_CUST_BOOKSPELL))
	{
		if (!(player.Interface & INTER_MAP))
		{
			if (player.rune_flags)
			{
				Book_Mode = BOOKMODE_SPELLS;
				ARX_INTERFACE_BookOpenClose(0);
			}
		}
		else if ((player.Interface & INTER_MAP) && (Book_Mode != BOOKMODE_SPELLS))
			Book_Mode = BOOKMODE_SPELLS;
		else
		{
			ARX_INTERFACE_BookOpenClose(2);
		}	  
	}

	if (GInput->actionNowPressed(CONTROLS_CUST_BOOKMAP))
	{
		if (!(player.Interface & INTER_MAP))
		{
			Book_Mode = BOOKMODE_MINIMAP;
			ARX_INTERFACE_BookOpenClose(0);
		}
		else if ((player.Interface & INTER_MAP) && (Book_Mode != BOOKMODE_MINIMAP))
			Book_Mode = BOOKMODE_MINIMAP;
		else
		{
			ARX_INTERFACE_BookOpenClose(2);
		}
	}

	if (GInput->actionNowPressed(CONTROLS_CUST_BOOKQUEST))
	{
		if (!(player.Interface & INTER_MAP))
		{
			Book_Mode = BOOKMODE_QUESTS;
			ARX_INTERFACE_BookOpenClose(0);
		}
		else if ((player.Interface & INTER_MAP) && (Book_Mode != BOOKMODE_QUESTS))
			Book_Mode = BOOKMODE_QUESTS;
		else
		{
			ARX_INTERFACE_BookOpenClose(2);
		}
	}

	if (GInput->actionNowPressed(CONTROLS_CUST_CANCELCURSPELL))
	{
		for (long i=MAX_SPELLS-1;i>=0;i--)
		{
			if ((spells[i].exist) && (spells[i].caster==0))
				if (spellicons[spells[i].type].bDuration)
				{
					ARX_SPELLS_AbortSpellSound();
					spells[i].tolive=0;
					break;
				}
		}
	}

	if (GInput->actionNowPressed(CONTROLS_CUST_PRECAST1))
	{
		if (((player.Interface & INTER_COMBATMODE) && !bIsAiming) || !player.doingmagic)
			if (Precast[0].typ != -1)
				ARX_SPELLS_Precast_Launch(0);
	}

	if (GInput->actionNowPressed(CONTROLS_CUST_PRECAST2))
	{
		if (((player.Interface & INTER_COMBATMODE) && !bIsAiming) || !player.doingmagic)
			if (Precast[1].typ != -1)
				ARX_SPELLS_Precast_Launch(1);
	}

	if (GInput->actionNowPressed(CONTROLS_CUST_PRECAST3))
	{
		if (((player.Interface & INTER_COMBATMODE) && !bIsAiming) || !player.doingmagic)
			if (Precast[2].typ != -1)
				ARX_SPELLS_Precast_Launch(2);
	}

	if (GInput->actionNowPressed(CONTROLS_CUST_WEAPON)||lChangeWeapon)
	{
		bool bGo = true; 

		if (lChangeWeapon > 0)
		{
			if (lChangeWeapon == 2)
			{
				lChangeWeapon--;
			}
			else
			{
				if(	(inter.iobj[0]->animlayer[1].cur_anim==NULL)||
					(inter.iobj[0]->animlayer[1].cur_anim == inter.iobj[0]->anims[ANIM_WAIT]))
				{
					lChangeWeapon--;

					if (pIOChangeWeapon)
					{
						SendIOScriptEvent(pIOChangeWeapon,SM_INVENTORYUSE,"");
						pIOChangeWeapon=NULL;
					}
				}
				else
				{
					bGo=false;
				}
			}
		}

		if (bGo)
		{
			if (player.Interface & INTER_COMBATMODE)
			{
				ARX_INTERFACE_Combat_Mode(0);
				bGToggleCombatModeWithKey=false;
				SPECIAL_DRAW_WEAPON=0;

				if (config.input.mouseLookToggle)
					TRUE_PLAYER_MOUSELOOK_ON=MEMO_PLAYER_MOUSELOOK_ON;
			}
			else
			{
				MEMO_PLAYER_MOUSELOOK_ON=TRUE_PLAYER_MOUSELOOK_ON;
				SPECIAL_DRAW_WEAPON=1;
				TRUE_PLAYER_MOUSELOOK_ON = true;
				SLID_START=float(arxtime);
				lFadeMapTime = (long)arxtime;
				ARX_INTERFACE_Combat_Mode(2);
				bGToggleCombatModeWithKey=true;

			}
		}
	}

	if(EERIEMouseButton&1) bGToggleCombatModeWithKey=false;

	if(bForceEscapeFreeLook)
	{
		TRUE_PLAYER_MOUSELOOK_ON = false;

		if (!GInput->actionPressed(CONTROLS_CUST_FREELOOK))
		{
			bForceEscapeFreeLook=false;
		}
	}
	else
	{
		if(eMouseState!=MOUSE_IN_INVENTORY_ICON)
		{
			if (!config.input.mouseLookToggle)
			{
				if (GInput->actionPressed(CONTROLS_CUST_FREELOOK))
				{
					if (!TRUE_PLAYER_MOUSELOOK_ON)
					{
						TRUE_PLAYER_MOUSELOOK_ON = true;
						SLID_START=float(arxtime);
					}
				}
				else
				{
					TRUE_PLAYER_MOUSELOOK_ON = false;
				}
			}
			else
			{
				if (GInput->actionNowPressed(CONTROLS_CUST_FREELOOK))
				{
					if (!TRUE_PLAYER_MOUSELOOK_ON)
					{
						TRUE_PLAYER_MOUSELOOK_ON = true;
						SLID_START=float(arxtime);
					}
					else
					{
						TRUE_PLAYER_MOUSELOOK_ON = false;

						if (player.Interface & INTER_COMBATMODE)
							ARX_INTERFACE_Combat_Mode(0);			
					}
				}
			}
		}
	}


	if(	(player.Interface&INTER_COMBATMODE)&&
		(GInput->actionNowReleased(CONTROLS_CUST_FREELOOK)) )
	{
		ARX_INTERFACE_Combat_Mode(0);
	}

	if (EDITMODE) return;//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	// Checks INVENTORY Key Status.
	if (GInput->actionNowPressed(CONTROLS_CUST_INVENTORY))
	{
		if (player.Interface & INTER_COMBATMODE)
		{
			ARX_INTERFACE_Combat_Mode(0);
		}

		bInverseInventory=!bInverseInventory;
		lOldTruePlayerMouseLook=TRUE_PLAYER_MOUSELOOK_ON;

		if (!config.input.mouseLookToggle)
		{
			bForceEscapeFreeLook=true;
		}
	}

	// Checks BOOK Key Status.
	if (GInput->actionNowPressed(CONTROLS_CUST_BOOK))
		ARX_INTERFACE_BookOpenClose(0);

	//	Check For Combat Mode ON/OFF
	if (	(EERIEMouseButton & 1)
#ifdef BUILD_EDITOR
		&&	(EDITION==EDITION_IO)
#endif
		&&	(!(player.Interface & INTER_COMBATMODE))
		&&	(!(ARX_MOUSE_OVER & ARX_MOUSE_OVER_BOOK)) 
		&&	(!EDITMODE)	
		&&	(!SpecialCursor)
		&&  (PLAYER_MOUSELOOK_ON)
		&&	(DRAGINTER==NULL)
		&&	(!InInventoryPos(&DANAEMouse)
		&& (config.input.autoReadyWeapon))
		)
	{
		if (!(LastMouseClick & 1))
		{
			COMBAT_MODE_ON_START_TIME = (unsigned long)(arxtime);
		}
		else 
		{
			if (float(arxtime) - COMBAT_MODE_ON_START_TIME>10)
			{
				ARX_INTERFACE_Combat_Mode(1);

				if (! config.input.autoReadyWeapon)
					bGToggleCombatModeWithKey=true;
			}
		}
	}

	if	(lOldTruePlayerMouseLook!=TRUE_PLAYER_MOUSELOOK_ON)
	{
		bInverseInventory=false;

		if (TRUE_PLAYER_MOUSELOOK_ON)
		{
			if (!CSEND)
			{
				CSEND=1;
				SendIOScriptEvent(inter.iobj[0],SM_EXPLORATIONMODE);
			}
		}
	}
	else				
	{
		if (CSEND)
		{
			CSEND=0;
			SendIOScriptEvent(inter.iobj[0],SM_CURSORMODE);
		}
	}

	static long lOldInterfaceTemp=0;

	if (TRUE_PLAYER_MOUSELOOK_ON)
	{
		if(bInverseInventory)
		{
			bRenderInCursorMode=true;

			if(MAGICMODE<0)
			{
				InventoryOpenClose(1);
			}
		}
		else
		{
			if(!bInventoryClosing)
			{

				lTimeToDrawMecanismCursor=0;
				lNbToDrawMecanismCursor=0;

				if (player.Interface & INTER_INVENTORYALL)
				{
					ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
					bInventoryClosing = true;
					lOldInterfaceTemp=INTER_INVENTORYALL;
				}


				bRenderInCursorMode=false;
				InventoryOpenClose(2);

				if((player.Interface &INTER_INVENTORY))
				{
					INTERACTIVE_OBJ * io = NULL;

					if (SecondaryInventory!=NULL)
					{
						io = (INTERACTIVE_OBJ *)SecondaryInventory->io;
					}
					else if (player.Interface & INTER_STEAL)
					{
						io = ioSteal;
					}

					if (io!=NULL)
					{
						InventoryDir=-1;
						SendIOScriptEvent(io,SM_INVENTORY2_CLOSE);
						TSecondaryInventory=SecondaryInventory;
						SecondaryInventory=NULL;
					}
				}

				if(config.input.mouseLookToggle)
				{
					TRUE_PLAYER_MOUSELOOK_ON = true;
					SLID_START=float(arxtime);
				}
			}
		}
	}
	else
	{
		if(bInverseInventory)
		{
			if(!bInventoryClosing)
			{

				bRenderInCursorMode=false;

				InventoryOpenClose(2);

				if((player.Interface &INTER_INVENTORY))
				{
					INTERACTIVE_OBJ * io = NULL;

					if (SecondaryInventory!=NULL)
					{
						io = (INTERACTIVE_OBJ *)SecondaryInventory->io;
					}
					else if (player.Interface & INTER_STEAL)
					{
						io = ioSteal;
					}

					if (io!=NULL)
					{
						InventoryDir=-1;
						SendIOScriptEvent(io,SM_INVENTORY2_CLOSE);
						TSecondaryInventory=SecondaryInventory;
						SecondaryInventory=NULL;
					}
				}

				if(config.input.mouseLookToggle)
				{
					TRUE_PLAYER_MOUSELOOK_ON = true;
					SLID_START=float(arxtime);
				}
			}
		}
		else
		{

			bRenderInCursorMode=true;

			if(MAGICMODE<0)
			{

				if(lOldInterfaceTemp)
				{
					lOldInterface=lOldInterfaceTemp;
					lOldInterfaceTemp=0;
					ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
				}

				if(lOldInterface)
				{
					player.Interface|=lOldInterface;
					player.Interface&=~INTER_INVENTORY;
				}
				else
					InventoryOpenClose(1);
			}
		}

		if (bRenderInCursorMode)
		{
			if (eyeball.exist != 0)
			{
				long lNumSpell=ARX_SPELLS_GetInstance(SPELL_FLYING_EYE);

				if (lNumSpell >= 0)
				{
					ARX_SPELLS_Kill(lNumSpell);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
void ARX_INTERFACE_Reset()
{
	SMOOTHSLID=0;
	PLAYER_INTERFACE_HIDE_COUNT=0;
	BLOCK_PLAYER_CONTROLS=0;
	SLID_VALUE=0;
	CINEMASCOPE=0;
	CINEMA_INC=0;
	CINEMA_DECAL=0;
}

//-----------------------------------------------------------------------------
void ARX_INTERFACE_SetCinemascope(long status,long smooth)
{
	if (status)
	{
		CINEMASCOPE=1;//++;
		g_TimeStartCinemascope = arxtime.get_updated();
	}
	else
	{
		CINEMASCOPE=0;//--;
		g_TimeStartCinemascope = 0;
	}

	arx_assert(CINEMASCOPE >= 0);

	if (CINEMASCOPE)
	{
		if (smooth)
		{
			CINEMA_INC=1;
		}
		else CINEMA_DECAL=100;
	}
	else
	{
		if (smooth)
		{
			CINEMA_INC=-1;
		}
		else CINEMA_DECAL=0;
	}

	if (player.Interface & INTER_INVENTORY)
	{
		player.Interface &=~ INTER_INVENTORY;
	}

	if (player.Interface & INTER_INVENTORYALL)
	{
		player.Interface &=~ INTER_INVENTORYALL;
	}
}

//-----------------------------------------------------------------------------
void ARX_INTERFACE_PlayerInterfaceModify(long showhide,long smooth)
{
	if (showhide == 0)
	{
		InventoryOpenClose(2);
		ARX_INTERFACE_BookOpenClose(2);
		ARX_INTERFACE_NoteClose();
	}

	if (showhide) PLAYER_INTERFACE_HIDE_COUNT = 0;
	else PLAYER_INTERFACE_HIDE_COUNT = 1;

	if (PLAYER_INTERFACE_HIDE_COUNT<0)
		PLAYER_INTERFACE_HIDE_COUNT=0;

	if (smooth)
	{
		if (showhide) SMOOTHSLID=-1;
		else SMOOTHSLID=1;
	}
	else
	{
		if (showhide) SLID_VALUE=0.f;
		else SLID_VALUE=100.f;

		lSLID_VALUE = SLID_VALUE;
	}
}

void ArxGame::ManageKeyMouse() {
	
	if (ARXmenu.currentmode == AMCM_OFF)
	{
		INTERACTIVE_OBJ * pIO = NULL;

		if (!BLOCK_PLAYER_CONTROLS)
		{
			if (TRUE_PLAYER_MOUSELOOK_ON && !(player.Interface & INTER_COMBATMODE)
				&& (eMouseState != MOUSE_IN_NOTE)
				)
			{
				Vec2s poss;
				poss.x=MemoMouse.x;
				poss.y=MemoMouse.y;

				// mode systemshock
				if (config.input.mouseLookToggle && config.input.autoReadyWeapon == false)
				{

					float fX =  DANAESIZX * 0.5f;
					float fY =	DANAESIZY * 0.5f;
					DANAEMouse.x = checked_range_cast<short>(fX);
					DANAEMouse.y = checked_range_cast<short>(fY);


					pIO = FlyingOverObject(&DANAEMouse);

					if (pIO)
					{
						FlyingOverIO = pIO;
						MemoMouse.x = DANAEMouse.x;
						MemoMouse.y = DANAEMouse.y;
					}
				}
				else
					pIO = FlyingOverObject(&poss);
			}
			else
				pIO = FlyingOverObject(&DANAEMouse);
		}

		if (pIO && (ARX_MOUSE_OVER & ARX_MOUSE_OVER_BOOK))
		{
			for (long i=0;i<MAX_EQUIPED;i++)
			{
				if ((player.equiped[i]!=0)
					&&	ValidIONum(player.equiped[i])
					&&	(inter.iobj[player.equiped[i]] == pIO))
					FlyingOverIO = pIO;
			}
		}

		if ((pIO)
			&& (pIO->GameFlags & GFLAG_INTERACTIVITY)
			&& !(ARX_MOUSE_OVER & ARX_MOUSE_OVER_BOOK)
			&& (eMouseState != MOUSE_IN_NOTE)
			)
		{
			if (!(EERIEMouseButton & 2) && (LastMouseClick & 2)
				&&  (STARTED_ACTION_ON_IO == pIO))
			{
				if (pIO->ioflags & IO_ITEM)
				{
					FlyingOverIO = pIO;
					COMBINE=NULL;

					if (DRAGINTER == NULL)
					{
						bool bOk = true;

						if (SecondaryInventory!=NULL)
						{
							INTERACTIVE_OBJ * temp=(INTERACTIVE_OBJ *)SecondaryInventory->io;

							if (IsInSecondaryInventory(FlyingOverIO))
								if (temp->ioflags & IO_SHOP)
									bOk = false;
						}

							INTERACTIVE_OBJ * io=inter.iobj[0];
							ANIM_USE * useanim=&io->animlayer[1];
							long type=ARX_EQUIPMENT_GetPlayerWeaponType();

							switch (type)
							{
							case WEAPON_DAGGER:

								if(useanim->cur_anim==io->anims[ANIM_DAGGER_UNREADY_PART_1]) bOk=false;

								break;
							case WEAPON_1H:

								if(useanim->cur_anim==io->anims[ANIM_1H_UNREADY_PART_1]) bOk=false;

								break;
							case WEAPON_2H:

								if(useanim->cur_anim==io->anims[ANIM_2H_UNREADY_PART_1]) bOk=false;

								break;
							case WEAPON_BOW:

								if(useanim->cur_anim==io->anims[ANIM_MISSILE_UNREADY_PART_1]) bOk=false;

								break;
							default:
								break;
							}

						if (bOk)
						{
							if (!((FlyingOverIO->_itemdata->playerstacksize <= 1) && (FlyingOverIO->_itemdata->count > 1)))
							{
								SendIOScriptEvent(FlyingOverIO,SM_INVENTORYUSE);

								if (!((config.input.autoReadyWeapon == false) && (config.input.mouseLookToggle)))
								{
									TRUE_PLAYER_MOUSELOOK_ON = false;
								}
							}
						}
					}

					if ((config.input.autoReadyWeapon == false) && (config.input.mouseLookToggle))
					{
						EERIEMouseButton &= ~2;
					}
				}
				}
			else //!TRUE_PLAYER_MOUSELOOK_ON
			{
				if ((EERIEMouseButton & 2) && !(LastMouseClick & 2))
				{
					STARTED_ACTION_ON_IO=FlyingOverIO;
				}
			}
		}

		if ((eMouseState == MOUSE_IN_WORLD) ||
			((eMouseState == MOUSE_IN_BOOK) && (!((ARX_MOUSE_OVER & ARX_MOUSE_OVER_BOOK) && (Book_Mode != BOOKMODE_MINIMAP))))
			)
		{
			if (config.input.mouseLookToggle)
			{
				if (eMouseState != MOUSE_IN_NOTE)
				{
					if ((EERIEMouseButton & 2) && !(LastMouseClick & 2)&&(config.input.linkMouseLookToUse))
					{
						if (!(FlyingOverIO && (FlyingOverIO->ioflags & IO_ITEM)) || DRAGINTER)
						{
							if (!TRUE_PLAYER_MOUSELOOK_ON)
							{
								if (!InInventoryPos(&DANAEMouse))
								{
									if (!((player.Interface & INTER_MAP) && Book_Mode != BOOKMODE_MINIMAP))
									{
										TRUE_PLAYER_MOUSELOOK_ON = true;
										EERIEMouseButton &= ~2;
										SLID_START=float(arxtime);
										lFadeMapTime = (long)arxtime;
									}
								}
							}
							else
							{
								if (!((config.input.autoReadyWeapon == false) && (config.input.mouseLookToggle) && FlyingOverIO && (FlyingOverIO->ioflags & IO_ITEM)))
								{
									TRUE_PLAYER_MOUSELOOK_ON = false;
									if (player.Interface & INTER_COMBATMODE && !(player.Interface & INTER_NOTE))
										ARX_INTERFACE_Combat_Mode(0);
								}
							}
						}
					}
				}
			}
			else
			{
				if (eMouseState != MOUSE_IN_NOTE)
				{
					if(	(EERIEMouseButton & 2) &&
						(!(ARX_MOUSE_OVER & ARX_MOUSE_OVER_BOOK & (Book_Mode != BOOKMODE_MINIMAP))) &&
						(!TRUE_PLAYER_MOUSELOOK_ON || SPECIAL_DRAW_WEAPON)&&
					        (config.input.linkMouseLookToUse))
					{
						if (SPECIAL_DRAW_WEAPON)
							SPECIAL_DRAW_WEAPON=0;
						else if (!InInventoryPos(&DANAEMouse))
						{
							if (SPECIAL_DRAW_WEAPON)
							{
								TRUE_PLAYER_MOUSELOOK_ON = false;
								SPECIAL_DRAW_WEAPON=0;
							}
							else
							{
								TRUE_PLAYER_MOUSELOOK_ON = true;
								SLID_START=float(arxtime);
								lFadeMapTime = (long)arxtime;
							}
						}
					}
					else if ((!(EERIEMouseButton & 2)) && config.input.linkMouseLookToUse && (LastMouseClick & 2))
					{
						if (!SPECIAL_DRAW_WEAPON)
						{
							if (!GInput->actionPressed(CONTROLS_CUST_FREELOOK))
								TRUE_PLAYER_MOUSELOOK_ON = false;

							if ((player.Interface & INTER_COMBATMODE) && !GInput->actionPressed(CONTROLS_CUST_FREELOOK))
								ARX_INTERFACE_Combat_Mode(0);
						}

						EERIEMouseButton &= ~2;
					}
				}

				if (TRUE_PLAYER_MOUSELOOK_ON && (!(EERIEMouseButton & 2)) && !SPECIAL_DRAW_WEAPON)
				{
					if (!GInput->actionPressed(CONTROLS_CUST_FREELOOK))
						TRUE_PLAYER_MOUSELOOK_ON = false;
				}
			}
		}

		PLAYER_MOUSELOOK_ON=TRUE_PLAYER_MOUSELOOK_ON;

		if ((player.doingmagic==2)&& (config.input.mouseLookToggle))
			PLAYER_MOUSELOOK_ON = false;
	}

	if(ARXmenu.currentmode!=AMCM_OFF)
	{
		PLAYER_MOUSELOOK_ON = false;
	}

	// Checks For MouseGrabbing/Restoration after Grab
	bool bRestoreCoordMouse=true;

	if((PLAYER_MOUSELOOK_ON) && (!LAST_PLAYER_MOUSELOOK_ON))
	{
		MemoMouse.x=DANAEMouse.x;
		MemoMouse.y=DANAEMouse.y;
		EERIEMouseGrab=1;
	}
	else if ((!PLAYER_MOUSELOOK_ON) && (LAST_PLAYER_MOUSELOOK_ON))
	{
		EERIEMouseGrab=0;
				
		DANAEMouse.x=MemoMouse.x;
		DANAEMouse.y=MemoMouse.y;

		if(mainApp->GetWindow()->IsFullScreen())
		{
			GInput->setMousePosAbs(DANAEMouse);
		}

		bRestoreCoordMouse=false;
	}

	LAST_PLAYER_MOUSELOOK_ON=PLAYER_MOUSELOOK_ON;
	PLAYER_ROTATION=0;

	long mouseDiffX = GInput->getMousePosRel().x;
	long mouseDiffY = GInput->getMousePosRel().y;
	
	ARX_Menu_Manage();
	Vec3f tm;
	tm.x=tm.y=tm.z=0.f;

	MOVETYPE=MOVE_WAIT;

	if(bRestoreCoordMouse) {
		DANAEMouse=GInput->getMousePosAbs();
	}

	// Player/Eyeball Freelook Management
	if (!BLOCK_PLAYER_CONTROLS)
	{
		GetInventoryObj_INVENTORYUSE(&DANAEMouse);

		if ((!(player.Interface & INTER_MAP )) || ((player.Interface & INTER_MAP ) && ((!(ARX_MOUSE_OVER & ARX_MOUSE_OVER_BOOK & (Book_Mode != BOOKMODE_MINIMAP))/*ARX_INTERFACE_MouseInBook()*/) || (Book_Mode == BOOKMODE_MINIMAP) || (Book_Mode == BOOKMODE_QUESTS) || (Book_Mode != -1)))
			||
			(player.Interface & INTER_COMBATMODE))
		{
			static int flPushTimeX[2]={0,0};
			static int flPushTimeY[2]={0,0};
			bool bKeySpecialMove=false;

			if(!GInput->actionPressed(CONTROLS_CUST_STRAFE))
			{

				float fTime = arxtime.get_updated();

				int	iTime = checked_range_cast<int>(fTime);

				if(GInput->actionPressed(CONTROLS_CUST_TURNLEFT))
				{
					if(!flPushTimeX[0])
					{
						flPushTimeX[0]	=	iTime;
					}

					bKeySpecialMove=true;
				}
				else flPushTimeX[0]=0;

				if(GInput->actionPressed(CONTROLS_CUST_TURNRIGHT))
				{
					if(!flPushTimeX[1])
					{
						flPushTimeX[1]	=	iTime;
					}

					bKeySpecialMove=true;
				}
				else flPushTimeX[1]=0;
			}

			if (USE_PLAYERCOLLISIONS)
			{

				float fTime = arxtime.get_updated();
				int iTime = checked_range_cast<int>(fTime);

				if(GInput->actionPressed(CONTROLS_CUST_LOOKUP))
				{
					if(!flPushTimeY[0])
					{
						flPushTimeY[0]	=	iTime;
					}

					bKeySpecialMove=true;
				}
				else flPushTimeY[0]=0;

				if(GInput->actionPressed(CONTROLS_CUST_LOOKDOWN))
				{
					if(!flPushTimeY[1])
					{
						flPushTimeY[1]	=	iTime;
					}

					bKeySpecialMove=true;
				}
				else flPushTimeY[1]=0;
			}

			if(bKeySpecialMove)
			{
				int iAction=0;

				if(	flPushTimeX[0]||
					flPushTimeX[1] )
				{
					if(flPushTimeX[0]<flPushTimeX[1]) mouseDiffX=10;
					else mouseDiffX=-10;

					iAction|=1;
				}

				if(	flPushTimeY[0]||
					flPushTimeY[1] )
				{
					if(flPushTimeY[0]<flPushTimeY[1]) mouseDiffY=10;
					else mouseDiffY=-10;

					iAction|=2;
				}

				if(!(iAction&1)) mouseDiffX=0;

				if(!(iAction&2)) mouseDiffY=0;
			}
			else
			{
				if (bRenderInCursorMode)
				{
					Vec2s mousePosRel = GInput->getMousePosRel();
					if(	(DANAEMouse.x==(DANAESIZX-1))&&
							(mousePosRel.x > 8) )
					{
						mouseDiffY=0;
						mouseDiffX=mousePosRel.x;
						bKeySpecialMove=true;
					}
					else
					{
						if( (!DANAEMouse.x)&&
								(mousePosRel.x < -8))
						{
							mouseDiffY=0;
							mouseDiffX=mousePosRel.x;
							bKeySpecialMove=true;
						}
					}

					if(	(DANAEMouse.y==(DANAESIZY-1))&&
						    (mousePosRel.y > 8))
					{
						mouseDiffY=mousePosRel.y;
						mouseDiffX=0;
						bKeySpecialMove=true;
					}
					else
					{
						if(	(!DANAEMouse.y)&&
							    (mousePosRel.y < -8))
						{
							mouseDiffY=mousePosRel.y;
							mouseDiffX=0;
							bKeySpecialMove=true;
						}
					}
				}
			}

			if(GInput->actionPressed(CONTROLS_CUST_CENTERVIEW))
			{
				eyeball.angle.a=eyeball.angle.g=0.f;
				player.desiredangle.a=player.desiredangle.g=player.angle.a=player.angle.g=0.f;
			}

			float fd = (((float)GInput->getMouseSensitivity()) + 1.f) * 0.1f * ((640.f / (float)DANAESIZX));
			if (fd > 200) {
				fd=200;
			}			

			fd *= ((float)DANAESIZX) * ( 1.0f / 640 ); 

			if ((eyeball.exist==2) && (PLAYER_MOUSELOOK_ON||bKeySpecialMove))
			{
				if (mouseDiffY!=0)
				{
					float ia=((float)mouseDiffY*( 1.0f / 5 ))*fd;

					if (INVERTMOUSE) ia=-ia;

					if (eyeball.angle.a<70.f)
					{
						if (eyeball.angle.a+ia<70.f) eyeball.angle.a+=ia;
					}
					else if (eyeball.angle.a>300.f)
					{
						if (eyeball.angle.a+ia>300.f) eyeball.angle.a+=ia;
					}

					eyeball.angle.a=MAKEANGLE(eyeball.angle.a);
				}

				if (mouseDiffX!=0)
				{
					float ib=((float)mouseDiffX*( 1.0f / 5 ))*fd;

					eyeball.angle.b=MAKEANGLE(eyeball.angle.b-ib);
				}
			}
			else if (PLAYER_MOUSELOOK_ON || bKeySpecialMove)
				if (ARXmenu.currentmode != AMCM_NEWQUEST)
					{
					if ((mouseDiffY != 0))
						{
							float ia = ((float)mouseDiffY * ( 1.0f / 5 ) * fd);

							if ((inter.iobj[0]) && EEfabs(ia)>2.f) inter.iobj[0]->lastanimtime=0;

							if (INVERTMOUSE) ia=-ia;

							float iangle=player.angle.a;

							player.desiredangle.a=player.angle.a;
							player.desiredangle.a+=ia;
							player.desiredangle.a=MAKEANGLE(player.desiredangle.a);

							if ((player.desiredangle.a>=74.9f) && (player.desiredangle.a<=301.f))
							{
								if (iangle<75.f) player.desiredangle.a=74.9f; //69
								else player.desiredangle.a=301.f;
							}

						}

					if ((mouseDiffX != 0))
						{
							float ib = ((float)mouseDiffX * ( 1.0f / 5 ) * fd); 

							if (ib!=0.f) player.Current_Movement|=PLAYER_ROTATE;

							player.desiredangle.b=player.angle.b;
							player.desiredangle.b=MAKEANGLE(player.desiredangle.b-ib);
							PLAYER_ROTATION=ib;
						}
				}
		}
	}

	{

		if ((!BLOCK_PLAYER_CONTROLS) && !(player.Interface & INTER_COMBATMODE))
			{
				if (DRAGINTER == NULL) {
					if ((LastMouseClick & 1) && !(EERIEMouseButton & 1) && !(EERIEMouseButton & 4) && !(LastMouseClick & 4))
					{
						INTERACTIVE_OBJ * temp;
					temp = FlyingOverIO;

						if(temp && !temp->locname.empty()) {
							
							if (((FlyingOverIO->ioflags & IO_ITEM) && FlyingOverIO->_itemdata->equipitem)
								&& (player.Full_Skill_Object_Knowledge + player.Full_Attribute_Mind
								>= FlyingOverIO->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_Identify_Value].value) )
							{
								SendIOScriptEvent(FlyingOverIO,SM_IDENTIFY);
							}

							WILLADDSPEECH = getLocalised(temp->locname);

							if (temp->ioflags & IO_GOLD)
							{
								std::stringstream ss;
								ss << temp->_itemdata->price << " " << WILLADDSPEECH;
								WILLADDSPEECH = ss.str();
							}

							if ((temp->poisonous>0) && (temp->poisonous_count!=0))
							{
								std::string Text = getLocalised("description_poisoned", "error");
								std::stringstream ss;
								ss << " (" << Text << " " << (int)temp->poisonous << ")";
								WILLADDSPEECH += ss.str();
							}

							if ((temp->ioflags & IO_ITEM) && (temp->durability<100.f))
							{
								std::string Text = getLocalised("description_durability", "error");
								std::stringstream ss;
								ss << " " << Text << " " << std::fixed << std::setw(3) << std::setprecision(0) << temp->durability << "/" << temp->max_durability;
								WILLADDSPEECH += ss.str();
							}

						WILLADDSPEECHTIME = (unsigned long)(arxtime);

						bool bAddText = true;

							if( (temp->obj)&&
								(temp->obj->pbox)&&
							    (temp->obj->pbox->active == 1))
						{
								bAddText=false;
							}

						if(bAddText) {
								Rect::Num x = checked_range_cast<Rect::Num>(120 * Xratio);
								Rect::Num y = checked_range_cast<Rect::Num>(14 * Yratio);
								Rect::Num w = checked_range_cast<Rect::Num>((120 + 500) * Xratio);
								Rect::Num h = checked_range_cast<Rect::Num>((14 + 200) * Yratio);
								Rect rDraw(x, y, w, h);
								pTextManage->Clear();
								pTextManage->AddText(hFontInBook, WILLADDSPEECH, rDraw, Color(232, 204, 143), 2000 + WILLADDSPEECH.length()*60);
							}

							WILLADDSPEECH.clear();
						}
					}
					else
					{
						if(config.input.autoDescription)
					{

								INTERACTIVE_OBJ * temp;
						temp = FlyingOverIO;

								if(temp && !temp->locname.empty()) {
									
									if (((FlyingOverIO->ioflags & IO_ITEM) && FlyingOverIO->_itemdata->equipitem)
										&& (player.Full_Skill_Object_Knowledge + player.Full_Attribute_Mind
										>= FlyingOverIO->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_Identify_Value].value) )
									{
										SendIOScriptEvent(FlyingOverIO,SM_IDENTIFY);
									}

									WILLADDSPEECH = getLocalised(temp->locname);

									if (temp->ioflags & IO_GOLD)
									{
										std::stringstream ss;
										ss << temp->_itemdata->price << " " << WILLADDSPEECH;
										WILLADDSPEECH = ss.str();
									}

									if ((temp->poisonous>0) && (temp->poisonous_count!=0))
									{
										std::string Text = getLocalised("description_poisoned", "error");
										std::stringstream ss;
										ss << WILLADDSPEECH << " (" << Text << " " << (int)temp->poisonous << ")";
										WILLADDSPEECH = ss.str();
									}

									if ((temp->ioflags & IO_ITEM) && (temp->durability<100.f))
									{
										std::string Text = getLocalised("description_durability", "error");
										std::stringstream ss;
										ss << WILLADDSPEECH << " " << Text << " " << std::fixed << std::setw(3) << std::setprecision(0) << temp->durability << "/" << temp->max_durability;
										WILLADDSPEECH = ss.str();
									}

									WILLADDSPEECHTIME = (unsigned long)(arxtime);//treat warning C4244 conversion from 'float' to 'unsigned long'
									bool bAddText=true;

									if( (temp->obj)&&
										(temp->obj->pbox)&&
							        (temp->obj->pbox->active == 1))
							{
										bAddText=false;
									}

							if(bAddText) {
								Rect::Num x = checked_range_cast<Rect::Num>(120 * Xratio);
								Rect::Num y = checked_range_cast<Rect::Num>(14 * Yratio);
								Rect::Num w = checked_range_cast<Rect::Num>((120 + 500 ) * Xratio);
								Rect::Num h = checked_range_cast<Rect::Num>((14 + 200 ) * Yratio);
								Rect rDraw(x, y, w, h);
								pTextManage->Clear();
								pTextManage->AddText(hFontInBook, WILLADDSPEECH, rDraw, Color(232, 204, 143));
							}

									WILLADDSPEECH.clear();
								}
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
}

static float fDecPulse;
//-----------------------------------------------------------------------------
void ARX_INTERFACE_DrawSecondaryInventory(bool _bSteal) {
	
	if(TSecondaryInventory->io && !TSecondaryInventory->io->inventory_skin.empty()) {
		
		res::path file = "graph/interface/inventory" / TSecondaryInventory->io->inventory_skin;
		
		TextureContainer * tc = TextureContainer::LoadUI(file);

		if(tc) {
			ITC.Set("ingame_inventory", tc);
		} else {
			ITC.Set("ingame_inventory", BasicInventorySkin);
		}
		
	} else if(ITC.Get("ingame_inventory") != BasicInventorySkin) {
		ITC.Set("ingame_inventory", BasicInventorySkin);
	}

	ARX_INTERFACE_DrawItem(ITC.Get("ingame_inventory"), INTERFACE_RATIO(InventoryX), 0.f);


	long i,j;

	for (j=0;j<TSecondaryInventory->sizey;j++)
	{
		for (i=0;i<TSecondaryInventory->sizex;i++)
		{
			INTERACTIVE_OBJ * io=TSecondaryInventory->slot[i][j].io;

			if (io!=NULL)
			{
				bool bItemSteal = false;
				TextureContainer * tc=io->inv;
				TextureContainer * tc2=NULL;

				if (NeedHalo(io)) tc2 = io->inv->getHalo();

				if (_bSteal)
				{
					if (!ARX_PLAYER_CanStealItem(io))
					{
						bItemSteal = true;
						tc = ITC.Get("item_cant_steal");
						tc2 = NULL;
					}
				}

				if ((tc!=NULL) && (TSecondaryInventory->slot[i][j].show || bItemSteal) )
				{
					if (io->ioflags & IO_GOLD)
					{
						long num=0;

						if (io->_itemdata->price<=3)
							num=io->_itemdata->price-1;
						else if (io->_itemdata->price<=8)
							num=3;
						else if (io->_itemdata->price<=20)
							num=4;
						else if (io->_itemdata->price<=50)
							num=5;
						else
							num=6;

						io->obj=GoldCoinsObj[num];
						io->inv=GoldCoinsTC[num];
					}

					float px = INTERFACE_RATIO(InventoryX) + (float)i*INTERFACE_RATIO(32) + INTERFACE_RATIO(2);
					float py = (float)j*INTERFACE_RATIO(32) + INTERFACE_RATIO(13);

					Color color = (io->poisonous && io->poisonous_count!=0) ? Color::green : Color::white;
					EERIEDrawBitmap(px, py, INTERFACE_RATIO_DWORD(tc->m_dwWidth),
					                INTERFACE_RATIO_DWORD(tc->m_dwHeight), 0.001f, tc, color);

					if (!bItemSteal && (io==FlyingOverIO))
					{
						GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
						GRenderer->SetRenderState(Renderer::AlphaBlending, true);
						EERIEDrawBitmap(px, py, INTERFACE_RATIO_DWORD(tc->m_dwWidth),
						                INTERFACE_RATIO_DWORD(tc->m_dwHeight), 0.001f, tc, Color::white);
						GRenderer->SetRenderState(Renderer::AlphaBlending, false);
					}
					else if(!bItemSteal && (io->ioflags & IO_CAN_COMBINE)) {
						GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
						GRenderer->SetRenderState(Renderer::AlphaBlending, true);
						
						float fColorPulse = fabs(cos(radians(fDecPulse)));
						EERIEDrawBitmap(px, py, INTERFACE_RATIO_DWORD(tc->m_dwWidth),
						                INTERFACE_RATIO_DWORD(tc->m_dwHeight), 0.001f, tc,
						                Color::gray(fColorPulse));
						GRenderer->SetRenderState(Renderer::AlphaBlending, false);
					}

					if (tc2!=NULL)
					{
						ARX_INTERFACE_HALO_Draw(io,tc,tc2,
							px,
							py, INTERFACE_RATIO(1), INTERFACE_RATIO(1));
					}

					if ((io->ioflags & IO_ITEM) && (io->_itemdata->count!=1))
						ARX_INTERFACE_DrawNumber(px, py, io->_itemdata->count, 3, Color::white);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
void ARX_INTERFACE_DrawInventory(short _sNum, int _iX=0, int _iY=0)
{
	fDecPulse += FrameDiff * 0.5f;

	float fCenterX	= DANAECENTERX - INTERFACE_RATIO(320) + INTERFACE_RATIO(35) + _iX ;
	float fSizY		= DANAESIZY - INTERFACE_RATIO(101) + INTERFACE_RATIO_LONG(InventoryY) + _iY;

	float fPosX = ARX_CAST_TO_INT_THEN_FLOAT( fCenterX );
	float fPosY = ARX_CAST_TO_INT_THEN_FLOAT( fSizY );

	ARX_INTERFACE_DrawItem(ITC.Get("hero_inventory"), fPosX, fPosY - INTERFACE_RATIO(5));

	for (size_t j=0;j<INVENTORY_Y;j++)
		for (size_t i=0;i<INVENTORY_X;i++)
		{
			INTERACTIVE_OBJ * io = inventory[_sNum][i][j].io;

			if ((io!=NULL) && (inventory[_sNum][i][j].show))
			{
				TextureContainer * tc=io->inv;
				TextureContainer * tc2=NULL;

				if (NeedHalo(io))
					tc2 = io->inv->getHalo();

				if(tc != NULL) {
					
					float px = fPosX + i*INTERFACE_RATIO(32) + INTERFACE_RATIO(7);
					float py = fPosY + j*INTERFACE_RATIO(32) + INTERFACE_RATIO(6);
					
					Color color = (io->poisonous && io->poisonous_count != 0) ? Color::green : Color::white;
					EERIEDrawBitmap(px, py, INTERFACE_RATIO_DWORD(tc->m_dwWidth),
					                INTERFACE_RATIO_DWORD(tc->m_dwHeight), 0.001f, tc, color);
					
					if(io == FlyingOverIO) {
						GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
						GRenderer->SetRenderState(Renderer::AlphaBlending, true);
						EERIEDrawBitmap(px, py, INTERFACE_RATIO_DWORD(tc->m_dwWidth),
						                INTERFACE_RATIO_DWORD(tc->m_dwHeight), 0.001f, tc, Color::white);
						GRenderer->SetRenderState(Renderer::AlphaBlending, false);
					} else if(io->ioflags & IO_CAN_COMBINE) {
						float fColorPulse = fabs(cos(radians(fDecPulse)));
						GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
						GRenderer->SetRenderState(Renderer::AlphaBlending, true);
						EERIEDrawBitmap(px, py, INTERFACE_RATIO_DWORD(tc->m_dwWidth),
						                INTERFACE_RATIO_DWORD(tc->m_dwHeight), 0.001f, tc, 
						                Color::gray(fColorPulse));
						GRenderer->SetRenderState(Renderer::AlphaBlending, false);
					}

					if (tc2)
					{
						ARX_INTERFACE_HALO_Render(
							io->halo.color.r, io->halo.color.g, io->halo.color.b,
							io->halo.flags,
							tc2,
							px,
							py, INTERFACE_RATIO(1), INTERFACE_RATIO(1));
					}

					if ((io->ioflags & IO_ITEM) && (io->_itemdata->count!=1))
						ARX_INTERFACE_DrawNumber(px, py, io->_itemdata->count, 3, Color::white);
				}
			}
		}
}

extern TextureContainer * stealth_gauge_tc;
extern float CURRENT_PLAYER_COLOR;
extern long EXTERNALVIEW;
//-----------------------------------------------------------------------------
// Stealth Gauge Drawing
//-----------------------------------------------------------------------------
void ARX_INTERFACE_Draw_Stealth_Gauge()
{
	if ((stealth_gauge_tc) && (!CINEMASCOPE))
	{
		float v=GetPlayerStealth();

		if (CURRENT_PLAYER_COLOR<v)
		{
			float px = INTERFACE_RATIO(InventoryX) + INTERFACE_RATIO(110);
			if (px < INTERFACE_RATIO(10)) px = INTERFACE_RATIO(10);

			float py = DANAESIZY - INTERFACE_RATIO(126 + 32);
			float t=v-CURRENT_PLAYER_COLOR;

			if (t>=15) v=1.f;
			else v=(t*( 1.0f / 15 ))*0.9f+0.1f;

			GRenderer->SetRenderState(Renderer::AlphaBlending, true);
			GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
			EERIEDrawBitmap(px, py, INTERFACE_RATIO_DWORD(stealth_gauge_tc->m_dwWidth),
			                INTERFACE_RATIO_DWORD(stealth_gauge_tc->m_dwHeight), 0.01f,
			                stealth_gauge_tc, Color::gray(v));
			GRenderer->SetRenderState(Renderer::AlphaBlending, false);
		}
	}
}

//-----------------------------------------------------------------------------
// Damaged Equipment Drawing
//-----------------------------------------------------------------------------
void ARX_INTERFACE_DrawDamagedEquipment()
{
	if (CINEMASCOPE || BLOCK_PLAYER_CONTROLS) return;

	if (player.Interface & INTER_INVENTORYALL) return;

	long needdraw=0;

	for (long i=0;i<5;i++)
	{
		if (iconequip[i])
		{
			long eq=-1;

			switch (i)
			{
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

			if (player.equiped[eq]>0)
			{
				INTERACTIVE_OBJ * io=inter.iobj[player.equiped[eq]];
				float ratio=io->durability/io->max_durability;

				if (ratio<=0.5f)
				{
					needdraw|=1<<i;
				}
			}
		}
	}

	if (needdraw)
	{
		GRenderer->SetRenderState(Renderer::AlphaBlending, true);
		GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);

		GRenderer->SetCulling(Renderer::CullNone);
		GRenderer->SetRenderState(Renderer::DepthWrite, true);
		GRenderer->SetRenderState(Renderer::Fog, false);

		float px = INTERFACE_RATIO(InventoryX) + INTERFACE_RATIO(10 + 32 + 100);

		if ( px < INTERFACE_RATIO( 10 + 32 ) ) px = INTERFACE_RATIO( 10 + 32 );

		float py = DANAESIZY - INTERFACE_RATIO(158);

		for (long i=0;i<5;i++)
		{
			if ( (needdraw&(1<<i))&&
				 (iconequip[i]) )
			{
				long eq=-1;

				switch (i)
				{
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

				if (player.equiped[eq]>0)
				{
					INTERACTIVE_OBJ * io=inter.iobj[player.equiped[eq]];
					float ratio=io->durability/io->max_durability;
					Color col = Color3f(1.f-ratio, ratio, 0).to<u8>();
					EERIEDrawBitmap2(px, py, INTERFACE_RATIO_DWORD(iconequip[i]->m_dwWidth),
					                 INTERFACE_RATIO_DWORD(iconequip[i]->m_dwHeight), 0.001f, iconequip[i], col);
				}
			}
		}

		currpos += static_cast<long>(INTERFACE_RATIO(33.f));
		GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	}
}

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
	float t;
	t=*val;

	if (MouseInBookRect(x,y,x+32,y+32))
	{
		rval=true;

		if (((BOOKBUTTON & 1) || (BOOKBUTTON & 2))&& (tc))
			DrawBookInterfaceItem(tc, x, y);

		if (!(BOOKBUTTON & 1) && (LASTBOOKBUTTON & 1))
		{
			if ((player.Attribute_Redistribute > 0))
			{
				player.Attribute_Redistribute--;
				t++;
				*val=t;
				ARX_INTERFACE_RELEASESOUND();
			}
			else ARX_INTERFACE_ERRORSOUND();
		}

		if (!(BOOKBUTTON & 2) && (LASTBOOKBUTTON & 2))
		{
			if (ARXmenu.currentmode == AMCM_NEWQUEST)
			{
				if ((t >6) && (player.level==0))
				{
					player.Attribute_Redistribute++;
					t --;
					*val=t;
					ARX_INTERFACE_RELEASESOUND();
				}
				else ARX_INTERFACE_ERRORSOUND();
			}
			else ARX_INTERFACE_ERRORSOUND();
		}
	}

	return rval;
}

//-----------------------------------------------------------------------------
bool CheckSkillClick(float x, float y, float * val, TextureContainer * tc, float * oldval)
{
	bool rval=false;
	float t,ot;
	t=*val;
	ot=*oldval;

	if ( MouseInBookRect( x, y, x + 32, y + 32 ) )
	{
		rval=true;

		if (((BOOKBUTTON & 1) || (BOOKBUTTON & 2))&& (tc))
			DrawBookInterfaceItem(tc, x, y);

		if (!(BOOKBUTTON & 1) && (LASTBOOKBUTTON & 1))
		{
			if ((player.Skill_Redistribute > 0))
			{
				player.Skill_Redistribute--;
				t++;
				*val=t;
				ARX_INTERFACE_RELEASESOUND();
			}
			else ARX_INTERFACE_ERRORSOUND();
		}

		if (!(BOOKBUTTON & 2) && (LASTBOOKBUTTON & 2))
		{
			if (ARXmenu.currentmode == AMCM_NEWQUEST)
			{
				if ((t > ot) && (player.level==0))
				{
					player.Skill_Redistribute++;
					t --;
					*val=t;
					ARX_INTERFACE_RELEASESOUND();
				}
				else ARX_INTERFACE_ERRORSOUND();
			}
			else ARX_INTERFACE_ERRORSOUND();
		}
	}

	return rval;
}

//AFFICHAGE ICONE DE SPELLS DE DURATION

static void StdDraw(float posx, float posy, Color color, TextureContainer * tcc, long flag, long i) {
	
	TextureContainer * tc;

	if (tcc==NULL)
	{
		if (ITC.Get("unknown")==NULL)
			ITC.Set("unknown", TextureContainer::Load("graph/interface/icons/spell_unknown"));

		tc=ITC.Get("unknown");
	}
	else tc=tcc;

	if (tc)
	{
		EERIEDrawBitmap(posx, posy, INTERFACE_RATIO_DWORD(tc->m_dwWidth) * 0.5f, INTERFACE_RATIO_DWORD(tc->m_dwHeight) * 0.5f, 0.01f, tc, color);

		if(flag & 2) {
			GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendOne);
			EERIEDrawBitmap(posx - 1, posy - 1, INTERFACE_RATIO_DWORD(tc->m_dwWidth) * 0.5f, INTERFACE_RATIO_DWORD(tc->m_dwHeight) * 0.5f, 0.0001f, tc, color);
			EERIEDrawBitmap(posx + 1, posy + 1, INTERFACE_RATIO_DWORD(tc->m_dwWidth) * 0.5f, INTERFACE_RATIO_DWORD(tc->m_dwHeight) * 0.5f, 0.0001f, tc, color);
			GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
		}

		if (!(flag & 1))
		{
			if (!(player.Interface & INTER_COMBATMODE))
				if (MouseInRect(posx,posy, posx+INTERFACE_RATIO(32),posy+INTERFACE_RATIO(32)))
				{
					SpecialCursor=CURSOR_INTERACTION_ON;

					if ((LastMouseClick & 1) && (!(EERIEMouseButton & 1)) )
					{
						if (flag & 2)
						{
							if (Precast[PRECAST_NUM].typ >= 0)
								WILLADDSPEECH = spellicons[Precast[PRECAST_NUM].typ].name;

							WILLADDSPEECHTIME = (unsigned long)(arxtime);
						}
						else
						{
							if (spells[i].type >= 0)
								WILLADDSPEECH = spellicons[spells[i].type].name;

							WILLADDSPEECHTIME = (unsigned long)(arxtime);
						}
					}

					if (EERIEMouseButton & 4)
					{
						if (flag & 2)
						{
							ARX_SPELLS_Precast_Launch(PRECAST_NUM);
							EERIEMouseButton&=~4;
						}
						else
						{
							ARX_SPELLS_AbortSpellSound();
							EERIEMouseButton&=~4;
							spells[i].tolive=0;
						}
					}
				}
		}
	}
}
//---------------------------------------------------------------------------
void ManageSpellIcon(long i,float rrr,long flag)
{
	float POSX = DANAESIZX-INTERFACE_RATIO(35);
	Color color;
	float posx = POSX+lSLID_VALUE;
	float posy = (float)currpos;
	Spell typ=spells[i].type;

	if(flag & 1) {
		color = Color3f(rrr, 0, 0).to<u8>();
	} else if(flag & 2) {
		color = Color3f(0, rrr * (1.0f/2), rrr).to<u8>();
		float px = INTERFACE_RATIO(InventoryX) + INTERFACE_RATIO(110);
		if(px < INTERFACE_RATIO(10)) {
			px = INTERFACE_RATIO(10);
		}
		posx = px + INTERFACE_RATIO(33 + 33 + 33) + PRECAST_NUM * INTERFACE_RATIO(33);
		posy = DANAESIZY - INTERFACE_RATIO(126+32); // niveau du stealth
		typ = (Spell)i; // TODO ugh
	} else {
		color = Color::gray(rrr);
	}

	bool bOk=true;

	if (spells[i].bDuration)
	{
		if(	(player.mana<20)||
			((spells[i].timcreation+spells[i].tolive - float(arxtime)) < 2000) )
		{
			if(ucFlick&1) bOk=false;
		}
	}
	else
	{
		if(player.mana<20)
		{
			if(ucFlick&1) bOk=false;
		}
	}

	if ( ( (bOk) && (typ>=0) &&	((size_t)typ<SPELL_COUNT) ) || (flag == 2) )
		StdDraw(posx,posy,color,spellicons[typ].tc,flag,i);

	currpos += static_cast<long>(INTERFACE_RATIO(33.f));
}
extern float GLOBAL_LIGHT_FACTOR;
//-----------------------------------------------------------------------------
void ARX_INTERFACE_ManageOpenedBook_Finish()
{
	GRenderer->SetRenderState(Renderer::DepthWrite, true);

	if ((player.Interface & INTER_MAP ) &&  (!(player.Interface & INTER_COMBATMODE)))
	{
		if (Book_Mode == BOOKMODE_SPELLS)
		{
			Anglef angle;
			Vec3f pos;

			EERIE_LIGHT tl;
			memcpy(&tl,&DynLight[0],sizeof(EERIE_LIGHT));

			DynLight[0].pos.x=500.f;
			DynLight[0].pos.y=-1960.f;
			DynLight[0].pos.z=1590.f;

				DynLight[0].exist=1;
				DynLight[0].rgb.r=0.6f;
				DynLight[0].rgb.g=0.7f;
				DynLight[0].rgb.b=0.9f;
				DynLight[0].intensity=1.8f;

			DynLight[0].fallstart=4520.f;
			DynLight[0].fallend = DynLight[0].fallstart + 600.f;
			DynLight[0].rgb255.r=DynLight[0].rgb.r*255.f;
			DynLight[0].rgb255.g=DynLight[0].rgb.g*255.f;
			DynLight[0].rgb255.b=DynLight[0].rgb.b*255.f;
			DynLight[0].falldiff=DynLight[0].fallend-DynLight[0].fallstart;
			DynLight[0].falldiffmul=1.f/DynLight[0].falldiff;
			DynLight[0].precalc=DynLight[0].intensity*GLOBAL_LIGHT_FACTOR;

			EERIE_CAMERA * oldcam=ACTIVECAM;
			bookcam.centerx = DANAECENTERX;
			bookcam.centery = DANAECENTERY;
			SetActiveCamera(&bookcam);
			PrepareCamera(&bookcam);
			pos.x = 0.f;
			pos.y = 0.f;
			pos.z = 2100.f;

			angle.a=0.f;
			angle.b=0.f;
			angle.g=0.f;

			PDL[0]=&DynLight[0];
			TOTPDL=1;

			long found2=0;
			float n;
			long xpos=0;
			long ypos=0;

			for(size_t i = 0; i < RUNE_COUNT; i++) {
				
				if (necklace.runes[i])
				{
					bookcam.centerx = (382 + xpos * 45 + BOOKDECX) * Xratio;
					bookcam.centery = (100 + ypos * 64 + BOOKDECY) * Yratio;

					SetActiveCamera(&bookcam);
					PrepareCamera(&bookcam);

					// First draw the lace
					angle.b=0.f;

					if (player.rune_flags & (RuneFlag)(1<<i))
					{
						DrawEERIEInter(necklace.lacet,&angle,&pos,NULL);

						if (necklace.runes[i]->angle.b!=0.f)
						{
							if (necklace.runes[i]->angle.b>300.f)
								necklace.runes[i]->angle.b=300.f;

							angle.b=EEsin(arxtime.get_updated() * ( 1.0f / 200 ))*necklace.runes[i]->angle.b*( 1.0f / 40 );
						}

						necklace.runes[i]->angle.b-=_framedelay*0.2f;

						if (necklace.runes[i]->angle.b<0.f) necklace.runes[i]->angle.b=0.f;

						DynLight[0].exist=0;
						float tt;
						
						bookcam.centerx = (382 + xpos * 45 + BOOKDECX + 3) * Xratio;
						bookcam.centery = (100 + ypos * 64 + BOOKDECY + 2) * Yratio;
						SetActiveCamera(&bookcam);
						PrepareCamera(&bookcam);

						tt=angle.b;
						angle.b=-20.f;
						angle.b=tt;
						angle.b-=20.f;

						angle.b+=20.f;
						GRenderer->SetRenderState(Renderer::DepthWrite, true);
						GRenderer->SetRenderState(Renderer::AlphaBlending, false);
						DynLight[0].exist=1;	
						
						bookcam.centerx = (382 + xpos * 45 + BOOKDECX) * Xratio;
						bookcam.centery = (100 + ypos * 64 + BOOKDECY) * Yratio;
						SetActiveCamera(&bookcam);
						PrepareCamera(&bookcam);

						// Now draw the rune
						DrawEERIEInter(necklace.runes[i],&angle,&pos,NULL);

						PopAllTriangleList();

						xpos++;

						if (xpos > 4)
						{
							xpos = 0;
							ypos++;
						}

						// Checks for Mouse floating over a rune...
						if ((!found2) &&
							MouseInRect(BBOXMIN.x, BBOXMIN.y, BBOXMAX.x, BBOXMAX.y))
						{
							long r=0;

							for (size_t j=0;j<necklace.runes[i]->facelist.size();j++)
							{
								n=PtIn2DPolyProj( necklace.runes[i], &necklace.runes[i]->facelist[j] , (float)DANAEMouse.x, (float)DANAEMouse.y);

								if (n!=0.f)
								{
									r=1;
									break;
								}
							}

							if (r)
							{
								GRenderer->SetRenderState(Renderer::AlphaBlending, true);
								GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
								DrawEERIEInter(necklace.runes[i],&angle,&pos,NULL);

								necklace.runes[i]->angle.b+=_framedelay*2.f;

								PopAllTriangleList();
								
								GRenderer->SetRenderState(Renderer::AlphaBlending, false);

								SpecialCursor=CURSOR_INTERACTION_ON;

								if ((EERIEMouseButton & 1)  && !(LastMouseClick & 1))
									if ((size_t)LastRune!=i)
									{
										switch(i)
										{
										case RUNE_AAM:
											ARX_SPELLS_RequestSymbolDraw(inter.iobj[0], "aam", ARX_SOUND_GetDuration(SND_SYMB_AAM));
											ARX_SOUND_PlayInterface(SND_SYMB_AAM);
											break;
										case RUNE_CETRIUS:
											ARX_SPELLS_RequestSymbolDraw(inter.iobj[0], "cetrius", ARX_SOUND_GetDuration(SND_SYMB_CETRIUS));
											ARX_SOUND_PlayInterface(SND_SYMB_CETRIUS);
											break;
										case RUNE_COMUNICATUM:
											ARX_SPELLS_RequestSymbolDraw(inter.iobj[0], "comunicatum", ARX_SOUND_GetDuration(SND_SYMB_COMUNICATUM));
											ARX_SOUND_PlayInterface(SND_SYMB_COMUNICATUM);
											break;
										case RUNE_COSUM:
											ARX_SPELLS_RequestSymbolDraw(inter.iobj[0], "cosum", ARX_SOUND_GetDuration(SND_SYMB_COSUM));
											ARX_SOUND_PlayInterface(SND_SYMB_COSUM);
											break;
										case RUNE_FOLGORA:
											ARX_SPELLS_RequestSymbolDraw(inter.iobj[0], "folgora", ARX_SOUND_GetDuration(SND_SYMB_FOLGORA));
											ARX_SOUND_PlayInterface(SND_SYMB_FOLGORA);
											break;
										case RUNE_FRIDD:
											ARX_SPELLS_RequestSymbolDraw(inter.iobj[0], "fridd", ARX_SOUND_GetDuration(SND_SYMB_FRIDD));
											ARX_SOUND_PlayInterface(SND_SYMB_FRIDD);
											break;
										case RUNE_KAOM:
											ARX_SPELLS_RequestSymbolDraw(inter.iobj[0], "kaom", ARX_SOUND_GetDuration(SND_SYMB_KAOM));
											ARX_SOUND_PlayInterface(SND_SYMB_KAOM);
											break;
										case RUNE_MEGA:
											ARX_SPELLS_RequestSymbolDraw(inter.iobj[0], "mega", ARX_SOUND_GetDuration(SND_SYMB_MEGA));
											ARX_SOUND_PlayInterface(SND_SYMB_MEGA);
											break;
										case RUNE_MORTE:
											ARX_SPELLS_RequestSymbolDraw(inter.iobj[0], "morte", ARX_SOUND_GetDuration(SND_SYMB_MORTE));
											ARX_SOUND_PlayInterface(SND_SYMB_MORTE);
											break;
										case RUNE_MOVIS:
											ARX_SPELLS_RequestSymbolDraw(inter.iobj[0], "movis", ARX_SOUND_GetDuration(SND_SYMB_MOVIS));
											ARX_SOUND_PlayInterface(SND_SYMB_MOVIS);
											break;
										case RUNE_NHI:
											ARX_SPELLS_RequestSymbolDraw(inter.iobj[0], "nhi", ARX_SOUND_GetDuration(SND_SYMB_NHI));
											ARX_SOUND_PlayInterface(SND_SYMB_NHI);
											break;
										case RUNE_RHAA:
											ARX_SPELLS_RequestSymbolDraw(inter.iobj[0], "rhaa", ARX_SOUND_GetDuration(SND_SYMB_RHAA));
											ARX_SOUND_PlayInterface(SND_SYMB_RHAA);
											break;
										case RUNE_SPACIUM:
											ARX_SPELLS_RequestSymbolDraw(inter.iobj[0], "spacium", ARX_SOUND_GetDuration(SND_SYMB_SPACIUM));
											ARX_SOUND_PlayInterface(SND_SYMB_SPACIUM);
											break;
										case RUNE_STREGUM:
											ARX_SPELLS_RequestSymbolDraw(inter.iobj[0], "stregum", ARX_SOUND_GetDuration(SND_SYMB_STREGUM));
											ARX_SOUND_PlayInterface(SND_SYMB_STREGUM);
											break;
										case RUNE_TAAR:
											ARX_SPELLS_RequestSymbolDraw(inter.iobj[0], "taar", ARX_SOUND_GetDuration(SND_SYMB_TAAR));
											ARX_SOUND_PlayInterface(SND_SYMB_TAAR);
											break;
										case RUNE_TEMPUS:
											ARX_SPELLS_RequestSymbolDraw(inter.iobj[0], "tempus", ARX_SOUND_GetDuration(SND_SYMB_TEMPUS));
											ARX_SOUND_PlayInterface(SND_SYMB_TEMPUS);
											break;
										case RUNE_TERA:
											ARX_SPELLS_RequestSymbolDraw(inter.iobj[0], "tera", ARX_SOUND_GetDuration(SND_SYMB_TERA));
											ARX_SOUND_PlayInterface(SND_SYMB_TERA);
											break;
										case RUNE_VISTA:
											ARX_SPELLS_RequestSymbolDraw(inter.iobj[0], "vista", ARX_SOUND_GetDuration(SND_SYMB_VISTA));
											ARX_SOUND_PlayInterface(SND_SYMB_VISTA);
											break;
										case RUNE_VITAE:
											ARX_SPELLS_RequestSymbolDraw(inter.iobj[0], "vitae", ARX_SOUND_GetDuration(SND_SYMB_VITAE));
											ARX_SOUND_PlayInterface(SND_SYMB_VITAE);
											break;
										case RUNE_YOK:
											ARX_SPELLS_RequestSymbolDraw(inter.iobj[0], "yok", ARX_SOUND_GetDuration(SND_SYMB_YOK));
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

			if (!found2) LastRune=-1;

			// Now Draws Spells for this level...
			ARX_PLAYER_ComputePlayerFullStats();

			float posx=0;
			float posy=0;
			float fPosX = 0;
			float fPosY = 0;
			bool	bFlyingOver = false;

			for (size_t i=0; i < SPELL_COUNT; i++)
			{
				if ((spellicons[i].level==Book_SpellPage) && (!spellicons[i].bSecret))
				{
					// check if player can cast it
					bool bOk = true;
					long j = 0;

					while ((j < 4) && (spellicons[i].symbols[j] != RUNE_NONE))
					{
						if (!(player.rune_flags & (RuneFlag)(1<<spellicons[i].symbols[j])))
						{
							bOk = false;
						}

						j++;
					}

					if (bOk)
					{
						fPosX = 170.f+posx*85.f;
						fPosY = 135.f+posy*70.f;
						long flyingover = 0;

						if (MouseInBookRect(fPosX, fPosY, fPosX+48, fPosY+48))
						{
							bFlyingOver = true;
							flyingover = 1;

							SpecialCursor=CURSOR_INTERACTION_ON;
							DrawBookTextCenter(hFontInBook, 208, 90, spellicons[i].name, Color::none);

							for(size_t si = 0; si < MAX_SPEECH; si++) {
								if(speech[si].timecreation > 0)
									FLYING_OVER=0;
							}

							OLD_FLYING_OVER = FLYING_OVER;
							pTextManage->Clear();
							UNICODE_ARXDrawTextCenteredScroll(hFontInGame,
								static_cast<float>(DANAECENTERX),
								12,
								(DANAECENTERX)*0.82f,
								spellicons[i].description,
								Color(232,204,143),
								1000,
								0.01f,
								2,
								0);

							long count = 0;
							
							for (long j = 0; j < 6; ++j)
								if (spellicons[i].symbols[j] != RUNE_NONE)
									++count;

							GRenderer->GetTextureStage(0)->SetMagFilter(TextureStage::FilterLinear);
							for(int j = 0; j < 6; ++j) {
								if(spellicons[i].symbols[j] != RUNE_NONE) {
									pos.x = (240-(count*32)*0.5f+j*32);
									pos.y = (306);
									DrawBookInterfaceItem(necklace.pTexTab[spellicons[i].symbols[j]], pos.x, pos.y);
								}
							}
							GRenderer->GetTextureStage(0)->SetMagFilter(TextureStage::FilterNearest);
						}

						if (spellicons[i].tc)
						{
							GRenderer->SetRenderState(Renderer::AlphaBlending, true);
							GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);
							
							Color color;
							if(flyingover) {
								color = Color::white;

								if ((EERIEMouseButton & 1)  && !(LastMouseClick & 1))
								{
									player.SpellToMemorize.bSpell = true;

									for (long j=0;j<6;j++)
									{
										player.SpellToMemorize.iSpellSymbols[j] = spellicons[i].symbols[j];
									}

									player.SpellToMemorize.lTimeCreation = (unsigned long)(arxtime);
								}
							} else {
								color = Color::fromBGRA(0xFFa8d0df);
							}
							
							GRenderer->GetTextureStage(0)->SetMagFilter(TextureStage::FilterLinear);
							DrawBookInterfaceItem(spellicons[i].tc, fPosX, fPosY, color);
							GRenderer->GetTextureStage(0)->SetMagFilter(TextureStage::FilterNearest);

							GRenderer->SetRenderState(Renderer::AlphaBlending, false);
						}

						posx ++;

						if (posx>=2)
						{
							posx = 0;
							posy ++;
						}
					}
				}
			}

			if (!bFlyingOver)
			{
				OLD_FLYING_OVER = -1;
				FLYING_OVER = -1;
			}

			memcpy(&DynLight[0],&tl,sizeof(EERIE_LIGHT));
			SetActiveCamera(oldcam);
			PrepareCamera(oldcam);
		}
	}
	
	
}

//-----------------------------------------------------------------------------
void ARX_INTERFACE_ManageOpenedBook()
{
	GRenderer->SetRenderState(Renderer::Fog, false);
	
	if (ITC.Get("questbook")==NULL)
	{
		ITC.Set("playerbook", "graph/interface/book/character_sheet/char_sheet_book");
		ITC.Set("ic_casting", "graph/interface/book/character_sheet/buttons_carac/icone_casting");
		ITC.Set("ic_close_combat", "graph/interface/book/character_sheet/buttons_carac/icone_close_combat");
		ITC.Set("ic_constitution", "graph/interface/book/character_sheet/buttons_carac/icone_constit");
		ITC.Set("ic_defense", "graph/interface/book/character_sheet/buttons_carac/icone_defense");
		ITC.Set("ic_dexterity", "graph/interface/book/character_sheet/buttons_carac/icone_dext");
		ITC.Set("ic_etheral_link", "graph/interface/book/character_sheet/buttons_carac/icone_etheral_link");
		ITC.Set("ic_mind", "graph/interface/book/character_sheet/buttons_carac/icone_intel");
		ITC.Set("ic_intuition", "graph/interface/book/character_sheet/buttons_carac/icone_intuition");
		ITC.Set("ic_mecanism", "graph/interface/book/character_sheet/buttons_carac/icone_mecanism");
		ITC.Set("ic_object_knowledge", "graph/interface/book/character_sheet/buttons_carac/icone_obj_knowledge");
		ITC.Set("ic_projectile", "graph/interface/book/character_sheet/buttons_carac/icone_projectile");
		ITC.Set("ic_stealth", "graph/interface/book/character_sheet/buttons_carac/icone_stealth");
		ITC.Set("ic_strength", "graph/interface/book/character_sheet/buttons_carac/icone_strenght");
		
		ITC.Set("questbook", "graph/interface/book/questbook");
		ITC.Set("ptexspellbook", "graph/interface/book/spellbook");
		ITC.Set("bookmark_char", "graph/interface/book/bookmark_char");
		ITC.Set("bookmark_magic", "graph/interface/book/bookmark_magic");
		ITC.Set("bookmark_map", "graph/interface/book/bookmark_map");
		ITC.Set("bookmark_quest", "graph/interface/book/bookmark_quest");

		ITC.Set("accessible_1", "graph/interface/book/accessible/accessible_1");
		ITC.Set("accessible_2", "graph/interface/book/accessible/accessible_2");
		ITC.Set("accessible_3", "graph/interface/book/accessible/accessible_3");
		ITC.Set("accessible_4", "graph/interface/book/accessible/accessible_4");
		ITC.Set("accessible_5", "graph/interface/book/accessible/accessible_5");
		ITC.Set("accessible_6", "graph/interface/book/accessible/accessible_6");
		ITC.Set("accessible_7", "graph/interface/book/accessible/accessible_7");
		ITC.Set("accessible_8", "graph/interface/book/accessible/accessible_8");
		ITC.Set("accessible_9", "graph/interface/book/accessible/accessible_9");
		ITC.Set("accessible_10", "graph/interface/book/accessible/accessible_10");
		ITC.Set("current_1", "graph/interface/book/current_page/current_1");
		ITC.Set("current_2", "graph/interface/book/current_page/current_2");
		ITC.Set("current_3", "graph/interface/book/current_page/current_3");
		ITC.Set("current_4", "graph/interface/book/current_page/current_4");
		ITC.Set("current_5", "graph/interface/book/current_page/current_5");
		ITC.Set("current_6", "graph/interface/book/current_page/current_6");
		ITC.Set("current_7", "graph/interface/book/current_page/current_7");
		ITC.Set("current_8", "graph/interface/book/current_page/current_8");
		ITC.Set("current_9", "graph/interface/book/current_page/current_9");
		ITC.Set("current_10", "graph/interface/book/current_page/current_10");
		
		ITC.Set("ptexcursorredist", "graph/interface/cursors/add_points");
		
		ITC.Level = getLocalised("system_charsheet_player_lvl");
		ITC.Xp = getLocalised("system_charsheet_player_xp");
		
		ANIM_Set(&player.useanim,herowaitbook);

		player.useanim.flags |= EA_LOOP;

		ARXOldTimeMenu=ARXTimeMenu=arxtime.get_updated();
		ARXDiffTimeMenu=0;
	}

	BOOKDECX = 0;
	BOOKDECY = 0;
	
	GRenderer->GetTextureStage(0)->SetMinFilter(TextureStage::FilterLinear);
	GRenderer->GetTextureStage(0)->SetMagFilter(TextureStage::FilterLinear);
	
	if(ARXmenu.currentmode != AMCM_NEWQUEST) {
		switch(Book_Mode) {
			case BOOKMODE_STATS: {
				DrawBookInterfaceItem(ITC.Get("playerbook"), 97, 64, Color::white, 0.9999f); 
				break;
			}
			case BOOKMODE_SPELLS: {
				DrawBookInterfaceItem(ITC.Get("ptexspellbook"), 97, 64, Color::white, 0.9999f);
				break;
			}
			case BOOKMODE_MINIMAP: {
				DrawBookInterfaceItem( ITC.Get("questbook"), 97, 64, Color::white, 0.9999f);
				break;
			}
			case BOOKMODE_QUESTS: {
				gui::manageQuestBook();
				break;
			}
		}
	}
	else
	{
		float x = 0;
		
		if ( ITC.Get("playerbook") )
		{
			x = static_cast<float>( ( 640 - ITC.Get("playerbook")->m_dwWidth ) / 2 );
			float y = static_cast<float>( ( 480 - ITC.Get("playerbook")->m_dwHeight ) / 2 );

			DrawBookInterfaceItem(ITC.Get("playerbook"), x, y);
		}

		BOOKDECX = x - 97;
		BOOKDECY = x - 64 + 19;
	}
	
	if (ARXmenu.currentmode != AMCM_NEWQUEST)
	{
		bool bOnglet[11];
		long max_onglet = 0;
		long Book_Page = 1;

		//---------------------------------------------------------------------
		// Checks Clicks in bookmarks
		
		// Character Sheet
		if (Book_Mode != BOOKMODE_STATS)
		{
			float px=BOOKMARKS_POS_X;
			float py=BOOKMARKS_POS_Y;
			TextureContainer* tcBookmarkChar = ITC.Get("bookmark_char");
			DrawBookInterfaceItem(tcBookmarkChar, px, py);

			// Check for cursor on charcter sheet bookmark
			if (	tcBookmarkChar
				&&	MouseInBookRect(px,py,px+tcBookmarkChar->m_dwWidth,py+tcBookmarkChar->m_dwHeight))
			{
				// Draw highlighted Character sheet icon
				GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
				GRenderer->SetRenderState(Renderer::AlphaBlending, true);
				DrawBookInterfaceItem(tcBookmarkChar, px, py, Color::grayb(0x55));
				GRenderer->SetRenderState(Renderer::AlphaBlending, false);

				// Set cursor to interacting
				SpecialCursor=CURSOR_INTERACTION_ON;

				// Check for click
				if (bookclick.x!=-1)
				{
					ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
					Book_Mode = BOOKMODE_STATS;
					pTextManage->Clear();
				}
			}
		}

		if (Book_Mode != BOOKMODE_SPELLS)
		{
			if (player.rune_flags)
			{
				float px=BOOKMARKS_POS_X+32;
				float py=BOOKMARKS_POS_Y;
				DrawBookInterfaceItem(ITC.Get("bookmark_magic"), px, py);

				//////////////// TO BE REDONE/REMOVED - START
				if (NewSpell==1)
				{
					NewSpell=2;

					for (long nk=0;nk<2;nk++) MagFX(BOOKDECX+220.f,BOOKDECY+49.f,0.000001f);
				}

				//////////////// TO BE REDONE/REMOVED - END
				if (	ITC.Get("bookmark_magic") 
					&&	MouseInBookRect(px,py,px+ITC.Get("bookmark_magic")->m_dwWidth,py+ITC.Get("bookmark_magic")->m_dwHeight))
				{
					// Draw highlighted Magic sheet icon
					GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
					GRenderer->SetRenderState(Renderer::AlphaBlending, true);
					DrawBookInterfaceItem(ITC.Get("bookmark_magic"), px, py, Color::grayb(0x55));
					GRenderer->SetRenderState(Renderer::AlphaBlending, false);

					// Set cursor to interacting
					SpecialCursor=CURSOR_INTERACTION_ON;

					// Check for click
					if (bookclick.x!=-1)
					{
						ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
						Book_Mode = BOOKMODE_SPELLS;
						pTextManage->Clear();
					}
				}
			}
		}

		if (Book_Mode != BOOKMODE_MINIMAP)
		{
			float px=BOOKMARKS_POS_X+64;
			float py=BOOKMARKS_POS_Y;

			DrawBookInterfaceItem(ITC.Get("bookmark_map"), px, py);

			if (	ITC.Get("bookmark_map")
				&&	MouseInBookRect(px,py,px+ITC.Get("bookmark_map")->m_dwWidth,py+ITC.Get("bookmark_map")->m_dwHeight))
			{
				GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
				GRenderer->SetRenderState(Renderer::AlphaBlending, true);
				DrawBookInterfaceItem(ITC.Get("bookmark_map"), px, py, Color::grayb(0x55));
				GRenderer->SetRenderState(Renderer::AlphaBlending, false);

				// Set cursor to interacting
				SpecialCursor=CURSOR_INTERACTION_ON;

				// Check for click
				if (bookclick.x!=-1)
				{
					ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
					Book_Mode = BOOKMODE_MINIMAP;
					pTextManage->Clear();
				}
			}
		}

		if (Book_Mode != BOOKMODE_QUESTS)
		{
			float px=BOOKMARKS_POS_X+96;
			float py=BOOKMARKS_POS_Y;
			DrawBookInterfaceItem(ITC.Get("bookmark_quest"), px, py);

			if (	ITC.Get("bookmark_quest")
				&&	MouseInBookRect(px,py,px+ITC.Get("bookmark_quest")->m_dwWidth,py+ITC.Get("bookmark_quest")->m_dwHeight))
			{
				GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
				GRenderer->SetRenderState(Renderer::AlphaBlending, true);
				DrawBookInterfaceItem(ITC.Get("bookmark_quest"), px, py, Color::grayb(0x55));
				GRenderer->SetRenderState(Renderer::AlphaBlending, false);

				// Set cursor to interacting
				SpecialCursor=CURSOR_INTERACTION_ON;

				// Check for click
				if (bookclick.x!=-1)
				{
					ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
					Book_Mode = BOOKMODE_QUESTS;
					pTextManage->Clear();
				}
			}
		}
		
		if (Book_Mode == BOOKMODE_MINIMAP) max_onglet=8;
		else max_onglet=10;

		if (Book_Mode == BOOKMODE_SPELLS) Book_Page = Book_SpellPage;
		else Book_Page = Book_MapPage;

		std::fill_n(bOnglet, 11, false);

		// calcul de la page de spells
		if (Book_Mode == BOOKMODE_SPELLS)
		{
			for (size_t i = 0; i < SPELL_COUNT; ++i)
			{
				if (spellicons[i].bSecret == false)
				{
					bool bOk = true;

					for(long j = 0; j < 4 && spellicons[i].symbols[j] != RUNE_NONE; ++j) {
						if(!(player.rune_flags & (RuneFlag)(1<<spellicons[i].symbols[j])))
							bOk = false;
					}

					if (bOk)
						bOnglet[spellicons[i].level] = true;
				}
			}
		}
		else
		{
			memset(bOnglet, true, (max_onglet + 1) * sizeof(*bOnglet));
		}
		
		if ((Book_Mode==BOOKMODE_SPELLS) || (Book_Mode==BOOKMODE_MINIMAP))
		{
			if (bOnglet[1])
			{
				if (Book_Page!=1) 
				{
					float px=100.f;
					float py=82.f;
					DrawBookInterfaceItem(ITC.Get("accessible_1"), px, py);

					if(MouseInBookRect(px, py, px + 32, py + 32)) {
						GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
						GRenderer->SetRenderState(Renderer::AlphaBlending, true);
						DrawBookInterfaceItem(ITC.Get("accessible_1"), px, py, Color::grayb(0x55));
						GRenderer->SetRenderState(Renderer::AlphaBlending, false);
						SpecialCursor=CURSOR_INTERACTION_ON;

						if (bookclick.x!=-1)
						{
							Book_Page=1;
							ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
						}
					}
				} else {
					DrawBookInterfaceItem(ITC.Get("current_1"), 102.f, 82.f);
				}
			}

			if (bOnglet[2])
			{
				if (Book_Page!=2) 
				{
					float px=98.f;
					float py=112.f;
					DrawBookInterfaceItem(ITC.Get("accessible_2"), px, py);

					if (MouseInBookRect(px,py,px+32,py+32))
					{
						GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
						GRenderer->SetRenderState(Renderer::AlphaBlending, true);
						DrawBookInterfaceItem(ITC.Get("accessible_2"), px, py, Color::grayb(0x55));
						GRenderer->SetRenderState(Renderer::AlphaBlending, false);
						SpecialCursor=CURSOR_INTERACTION_ON;

						if (bookclick.x!=-1)
						{
							Book_Page=2;
							ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
						}
					}
				}
				else DrawBookInterfaceItem(ITC.Get("current_2"), 100.f, 114.f);
			}

			if (bOnglet[3])
			{
				if (Book_Page!=3) 
				{
					float px=97.f;
					float py=143.f;
					DrawBookInterfaceItem(ITC.Get("accessible_3"), px, py);

					if(MouseInBookRect(px, py, px + 32, py + 32)) {
						GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
						GRenderer->SetRenderState(Renderer::AlphaBlending, true);
						DrawBookInterfaceItem(ITC.Get("accessible_3"), px, py, Color::grayb(0x55));
						GRenderer->SetRenderState(Renderer::AlphaBlending, false);
						SpecialCursor=CURSOR_INTERACTION_ON;
						if(bookclick.x != -1) {
							Book_Page=3;
							ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
						}
					}
				}
				else DrawBookInterfaceItem(ITC.Get("current_3"), 101.f, 141.f);
			}

			if (bOnglet[4])
			{
				if (Book_Page!=4)
				{
					float px=95.f;
					float py=170.f;
					DrawBookInterfaceItem(ITC.Get("accessible_4"), px, py);

					if(MouseInBookRect(px, py, px + 32, py + 32)) {
						GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
						GRenderer->SetRenderState(Renderer::AlphaBlending, true);
						DrawBookInterfaceItem(ITC.Get("accessible_4"), px, py, Color::grayb(0x55));
						GRenderer->SetRenderState(Renderer::AlphaBlending, false);
						SpecialCursor=CURSOR_INTERACTION_ON;
						if(bookclick.x != -1) {
							Book_Page=4;
							ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
						}
					}
				}
				else DrawBookInterfaceItem(ITC.Get("current_4"), 100.f, 170.f);
			}

			if (bOnglet[5])
			{
				if (Book_Page!=5) 
				{
					float px=95.f;
					float py=200.f;
					DrawBookInterfaceItem(ITC.Get("accessible_5"), px, py);

					if(MouseInBookRect(px, py, px + 32, py + 32)) {
						GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
						GRenderer->SetRenderState(Renderer::AlphaBlending, true);
						DrawBookInterfaceItem(ITC.Get("accessible_5"), px, py, Color::grayb(0x55));
						GRenderer->SetRenderState(Renderer::AlphaBlending, false);
						SpecialCursor=CURSOR_INTERACTION_ON;
						if(bookclick.x != -1) {
							Book_Page=5;
							ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
						}
					}
				}
				else DrawBookInterfaceItem(ITC.Get("current_5"), 97.f, 199.f);
			}

			if (bOnglet[6])
			{
				if (Book_Page!=6) 
				{
					float px=94.f;
					float py=229.f;
					DrawBookInterfaceItem(ITC.Get("accessible_6"), px, py);

					if(MouseInBookRect(px, py, px + 32, py + 32)) {
						GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
						GRenderer->SetRenderState(Renderer::AlphaBlending, true);
						DrawBookInterfaceItem(ITC.Get("accessible_6"), px, py, Color::grayb(0x55));
						GRenderer->SetRenderState(Renderer::AlphaBlending, false);
						SpecialCursor=CURSOR_INTERACTION_ON;
						if(bookclick.x != -1) {
							Book_Page=6;
							ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
						}
					}
				}
				else DrawBookInterfaceItem(ITC.Get("current_6"), 103.f, 226.f);
			}

			if (bOnglet[7])
			{
				if (Book_Page!=7)
				{
					float px=94.f;
					float py=259.f;
					DrawBookInterfaceItem(ITC.Get("accessible_7"), px, py);

					if(MouseInBookRect(px, py, px + 32, py + 32)) {
						GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
						GRenderer->SetRenderState(Renderer::AlphaBlending, true);
						DrawBookInterfaceItem(ITC.Get("accessible_7"), px, py, Color::grayb(0x55));
						GRenderer->SetRenderState(Renderer::AlphaBlending, false);
						SpecialCursor=CURSOR_INTERACTION_ON;
						if(bookclick.x != -1) {
							Book_Page=7;
							ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
						}
					}
				}
				else DrawBookInterfaceItem(ITC.Get("current_7"), 101.f, 255.f);
			}

			if (bOnglet[8])
			{
				if (Book_Page!=8) 
				{
					float px=92.f;
					float py=282.f;
					DrawBookInterfaceItem(ITC.Get("accessible_8"), px, py);

					if(MouseInBookRect(px, py, px + 32, py + 32)) {
						GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
						GRenderer->SetRenderState(Renderer::AlphaBlending, true);
						DrawBookInterfaceItem(ITC.Get("accessible_8"), px, py, Color::grayb(0x55));
						GRenderer->SetRenderState(Renderer::AlphaBlending, false);
						SpecialCursor=CURSOR_INTERACTION_ON;
						if(bookclick.x != -1) {
							Book_Page=8;
							ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
						}
					}
				}
				else DrawBookInterfaceItem(ITC.Get("current_8"), 99.f, 283.f);
			}

			if (bOnglet[9])
			{
				if (Book_Page!=9) 
				{
					float px=90.f;
					float py=308.f;
					DrawBookInterfaceItem(ITC.Get("accessible_9"), px, py);

					if(MouseInBookRect(px, py, px + 32, py + 32)) {
						GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
						GRenderer->SetRenderState(Renderer::AlphaBlending, true);
						DrawBookInterfaceItem(ITC.Get("accessible_9"), px, py, Color::grayb(0x55));
						GRenderer->SetRenderState(Renderer::AlphaBlending, false);
						SpecialCursor=CURSOR_INTERACTION_ON;
						if(bookclick.x != -1) {
							Book_Page=9;
							ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
						}
					}
				}
				else DrawBookInterfaceItem(ITC.Get("current_9"), 99.f, 307.f);
			}

			if (bOnglet[10])
			{
				if (Book_Page!=10) 
				{
					float px=97.f;
					float py=331.f;
					DrawBookInterfaceItem(ITC.Get("accessible_10"), px, py);

					if(MouseInBookRect(px, py, px + 32, py + 32)) {
						GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
						GRenderer->SetRenderState(Renderer::AlphaBlending, true);
						DrawBookInterfaceItem(ITC.Get("accessible_10"), px, py, Color::grayb(0x55));
						GRenderer->SetRenderState(Renderer::AlphaBlending, false);
						SpecialCursor=CURSOR_INTERACTION_ON;
						if(bookclick.x != -1) {
							Book_Page=10;
							ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
						}
					}
				}
				else DrawBookInterfaceItem(ITC.Get("current_10"), 104.f, 331.f);
			}		

			if (Book_Mode==1) Book_SpellPage=Book_Page;
			else if (Book_Mode==2) Book_MapPage=Book_Page;
		}

		bookclick.x=-1;
	}
	
	GRenderer->GetTextureStage(0)->SetMinFilter(TextureStage::FilterNearest);
	GRenderer->GetTextureStage(0)->SetMagFilter(TextureStage::FilterNearest);

	if (Book_Mode == BOOKMODE_STATS)
	{
		FLYING_OVER = 0;
		std::string tex;
		Color color(0, 0, 0);

		ARX_PLAYER_ComputePlayerFullStats();

		std::stringstream ss;
		ss << ITC.Level << " " << std::setw(3) << (int)player.level;
		tex = ss.str();
		DrawBookTextCenter(hFontInBook, 398, 74, tex, color);

		std::stringstream ss2;
		ss2 << ITC.Xp << " " << std::setw(8) << player.xp;
		tex = ss2.str();
		DrawBookTextCenter(hFontInBook, 510, 74, tex, color);

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

		if (!((player.Attribute_Redistribute == 0) && (ARXmenu.currentmode != AMCM_NEWQUEST)))
		{
			// Main Player Attributes
			if (CheckAttributeClick(379,95,&player.Attribute_Strength,		ITC.Get("ic_strength")))
			{
				FLYING_OVER=BOOK_STRENGTH;
				SpecialCursor = CURSOR_REDIST;
				lCursorRedistValue = player.Attribute_Redistribute;
			}

			if (CheckAttributeClick(428,95,&player.Attribute_Mind,			ITC.Get("ic_mind")))
			{
				FLYING_OVER=BOOK_MIND;
				SpecialCursor = CURSOR_REDIST;
				lCursorRedistValue = player.Attribute_Redistribute;
			}

			if (CheckAttributeClick(477,95,&player.Attribute_Dexterity,		ITC.Get("ic_dexterity")))
			{
				FLYING_OVER=BOOK_DEXTERITY;
				SpecialCursor = CURSOR_REDIST;
				lCursorRedistValue = player.Attribute_Redistribute;
			}

			if (CheckAttributeClick(526,95,&player.Attribute_Constitution,	ITC.Get("ic_constitution")))
			{
				FLYING_OVER=BOOK_CONSTITUTION;
				SpecialCursor = CURSOR_REDIST;
				lCursorRedistValue = player.Attribute_Redistribute;
			}
		}

		if (!((player.Skill_Redistribute == 0) && (ARXmenu.currentmode != AMCM_NEWQUEST)))
		{
			if (CheckSkillClick(389,177,&player.Skill_Stealth,		ITC.Get("ic_stealth"),&player.Old_Skill_Stealth))
			{
				FLYING_OVER=BOOK_STEALTH;
				SpecialCursor = CURSOR_REDIST;
				lCursorRedistValue = player.Skill_Redistribute;
			}

			if (CheckSkillClick(453,177,&player.Skill_Mecanism,		ITC.Get("ic_mecanism"),&player.Old_Skill_Mecanism))
			{
				FLYING_OVER=BOOK_MECANISM;
				SpecialCursor = CURSOR_REDIST;
				lCursorRedistValue = player.Skill_Redistribute;
			}

			if (CheckSkillClick(516,177,&player.Skill_Intuition,	ITC.Get("ic_intuition"),&player.Old_Skill_Intuition))
			{
				FLYING_OVER=BOOK_INTUITION;
				SpecialCursor = CURSOR_REDIST;
				lCursorRedistValue = player.Skill_Redistribute;
			}

			if (CheckSkillClick(389,230,&player.Skill_Etheral_Link,	ITC.Get("ic_etheral_link"),&player.Old_Skill_Etheral_Link))
			{
				FLYING_OVER=BOOK_ETHERAL_LINK;
				SpecialCursor = CURSOR_REDIST;
				lCursorRedistValue = player.Skill_Redistribute;
			}

			if (CheckSkillClick(453,230,&player.Skill_Object_Knowledge,ITC.Get("ic_object_knowledge"),&player.Old_Skill_Object_Knowledge))
			{
				FLYING_OVER=BOOK_OBJECT_KNOWLEDGE;
				SpecialCursor = CURSOR_REDIST;
				lCursorRedistValue = player.Skill_Redistribute;

				if ((BOOKBUTTON & 1) && !(LASTBOOKBUTTON & 1))
				{
					ARX_INVENTORY_IdentifyAll();
					ARX_EQUIPMENT_IdentifyAll();
				}

				ARX_PLAYER_ComputePlayerFullStats();
			}

			if (CheckSkillClick(516,230,&player.Skill_Casting,		ITC.Get("ic_casting"),&player.Old_Skill_Casting))
			{
				FLYING_OVER=BOOK_CASTING;
				SpecialCursor = CURSOR_REDIST;
				lCursorRedistValue = player.Skill_Redistribute;
			}

			if (CheckSkillClick(389,284,&player.Skill_Close_Combat,	ITC.Get("ic_close_combat"),&player.Old_Skill_Close_Combat))
			{
				FLYING_OVER=BOOK_CLOSE_COMBAT;
				SpecialCursor = CURSOR_REDIST;
				lCursorRedistValue = player.Skill_Redistribute;
			}

			if (CheckSkillClick(453,284,&player.Skill_Projectile,	ITC.Get("ic_projectile"),&player.Old_Skill_Projectile))
			{
				FLYING_OVER=BOOK_PROJECTILE;
				SpecialCursor = CURSOR_REDIST;
				lCursorRedistValue = player.Skill_Redistribute;
			}

			if (CheckSkillClick(516,284,&player.Skill_Defense,		ITC.Get("ic_defense"),&player.Old_Skill_Defense))
			{
				FLYING_OVER=BOOK_DEFENSE;
				SpecialCursor = CURSOR_REDIST;
				lCursorRedistValue = player.Skill_Redistribute;
			}
		}
		else
		{
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
		if (ARXmenu.mda && !ARXmenu.mda->flyover[FLYING_OVER].empty()) //=ARXmenu.mda->flyover[FLYING_OVER];
		{
			float fRandom = rnd() * 2;

			int t = checked_range_cast<int>(fRandom);

			pTextManage->Clear();
			OLD_FLYING_OVER=FLYING_OVER;

			// Nuky Note: the text used never scrolls, centered function with wordwrap would be enough
			if (FLYING_OVER == WND_XP)
			{
				std::stringstream ss;
				ss << ARXmenu.mda->flyover[WND_XP] << " " << std::setw(8) << GetXPforLevel(player.level+1)-player.xp;
				UNICODE_ARXDrawTextCenteredScroll( hFontInGame,
					                                (DANAESIZX*0.5f),
					                                4,
					                                (DANAECENTERX)*0.82f,
					                                ss.str(),
					                                Color(232+t,204+t,143+t),
					                                1000,
					                                0.01f,
					                                3,
					                                0);
			}
			else
			{
				UNICODE_ARXDrawTextCenteredScroll(hFontInGame,
					(DANAESIZX*0.5f),
					4,
					(DANAECENTERX)*0.82f,
					ARXmenu.mda->flyover[FLYING_OVER],
					Color(232+t,204+t,143+t),
					1000,
					0.01f,
					3,
					0);
			}
		}
		else
		{
			OLD_FLYING_OVER=-1;
		}

		//------------------------------
		
		std::stringstream ss3;
		ss3 << std::setw(3) << std::setprecision(0) << std::fixed << player.Full_Attribute_Strength;
		tex = ss3.str();

		if (player.Mod_Attribute_Strength<0.f)
			color = Color::red;
		else if (player.Mod_Attribute_Strength>0.f)
			color = Color::blue;
		else color = Color::black;

		if (ARXmenu.currentmode==AMCM_NEWQUEST)
		{
			if (player.Full_Attribute_Strength == 6)
				color = Color::red;
		}

		DrawBookTextCenter(hFontInBook, 391, 129, tex, color);
		
		ss3.str(""); // clear the stream
		ss3 << player.Full_Attribute_Mind;
		tex = ss3.str();

		if (player.Mod_Attribute_Mind<0.f)
			color = Color::red;
		else if (player.Mod_Attribute_Mind>0.f)
			color = Color::blue;
		else color = Color::black;

		if (ARXmenu.currentmode==AMCM_NEWQUEST)
		{
			if (player.Full_Attribute_Mind == 6)
				color = Color::red;
		}

		DrawBookTextCenter(hFontInBook, 440, 129, tex, color);
		
		ss3.str("");
		ss3 << player.Full_Attribute_Dexterity;
		tex = ss3.str();

		if (player.Mod_Attribute_Dexterity<0.f)
			color = Color::red;
		else if (player.Mod_Attribute_Dexterity>0.f)
			color = Color::blue;
		else color = Color::black;

		if (ARXmenu.currentmode==AMCM_NEWQUEST)
		{
			if (player.Full_Attribute_Dexterity == 6)
				color = Color::red;
		}

		DrawBookTextCenter(hFontInBook, 490, 129, tex, color);
		ss3.str("");
		ss3 << player.Full_Attribute_Constitution;
		tex = ss3.str();

		if (player.Mod_Attribute_Constitution<0.f)
			color = Color::red;
		else if (player.Mod_Attribute_Constitution>0.f)
			color = Color::blue;
		else color = Color::black;

		if (ARXmenu.currentmode==AMCM_NEWQUEST)
		{
			if (player.Full_Attribute_Constitution == 6)
				color = Color::red;
		}

		DrawBookTextCenter(hFontInBook, 538, 129, tex, color);

		// Player Skills
		ss3.str("");
		ss3 << player.Full_Skill_Stealth;
		tex = ss3.str();

		if (player.Mod_Skill_Stealth<0.f)
			color = Color::red;
		else if (player.Mod_Skill_Stealth>0.f)
			color = Color::blue;
		else color = Color::black;

		if (ARXmenu.currentmode==AMCM_NEWQUEST)
		{
			if (player.Skill_Stealth == 0)
				color = Color::red;
		}

		DrawBookTextCenter(hFontInBook, 405, 210, tex, color);
		
		ss3.str("");
		ss3 << player.Full_Skill_Mecanism;
		tex = ss3.str();

		if (player.Mod_Skill_Mecanism<0.f)
			color = Color::red;
		else if (player.Mod_Skill_Mecanism>0.f)
			color = Color::blue;
		else color = Color::black;

		if (ARXmenu.currentmode==AMCM_NEWQUEST)
		{
			if (player.Skill_Mecanism == 0)
				color = Color::red;
		}

		DrawBookTextCenter(hFontInBook, 469, 210, tex, color);
		
		ss3.str("");
		ss3 << player.Full_Skill_Intuition;
		tex = ss3.str();

		if (player.Mod_Skill_Intuition<0.f)
			color = Color::red;
		else if (player.Mod_Skill_Intuition>0.f)
			color = Color::blue;
		else color = Color::black;

		if (ARXmenu.currentmode==AMCM_NEWQUEST)
		{
			if (player.Skill_Intuition == 0)
				color = Color::red;
		}

		DrawBookTextCenter(hFontInBook, 533, 210, tex, color);
		
		ss3.str("");
		ss3 << player.Full_Skill_Etheral_Link;
		tex = ss3.str();

		if (player.Mod_Skill_Etheral_Link<0.f)
			color = Color::red;
		else if (player.Mod_Skill_Etheral_Link>0.f)
			color = Color::blue;
		else color = Color::black;

		if (ARXmenu.currentmode==AMCM_NEWQUEST)
		{
			if (player.Skill_Etheral_Link == 0)
				color = Color::red;
		}

		DrawBookTextCenter(hFontInBook, 405, 265, tex, color);
		
		ss3.str("");
		ss3 << player.Full_Skill_Object_Knowledge;
		tex = ss3.str();

		if (player.Mod_Skill_Object_Knowledge<0.f)
			color = Color::red;
		else if (player.Mod_Skill_Object_Knowledge>0.f)
			color = Color::blue;
		else color = Color::black;

		if (ARXmenu.currentmode==AMCM_NEWQUEST)
		{
			if (player.Skill_Object_Knowledge == 0)
				color = Color::red;
		}

		DrawBookTextCenter(hFontInBook, 469, 265, tex, color);
		
		ss3.str("");
		ss3 << player.Full_Skill_Casting;
		tex = ss3.str();

		if (player.Mod_Skill_Casting<0.f)
			color = Color::red;
		else if (player.Mod_Skill_Casting>0.f)
			color = Color::blue;
		else color = Color::black;

		if (ARXmenu.currentmode==AMCM_NEWQUEST)
		{
			if (player.Skill_Casting == 0)
				color = Color::red;
		}

		DrawBookTextCenter(hFontInBook, 533, 265, tex, color);
		
		ss3.str("");
		ss3 << player.Full_Skill_Close_Combat;
		tex = ss3.str();

		if (player.Mod_Skill_Close_Combat<0.f)
			color = Color::red;
		else if (player.Mod_Skill_Close_Combat>0.f)
			color = Color::blue;
		else color = Color::black;

		if (ARXmenu.currentmode==AMCM_NEWQUEST)
		{
			if (player.Skill_Close_Combat == 0)
				color = Color::red;
		}

		DrawBookTextCenter(hFontInBook, 405, 319, tex, color);

		
		ss3.str("");
		ss3 << player.Full_Skill_Projectile;
		tex = ss3.str();

		if (player.Mod_Skill_Projectile<0.f)
			color = Color::red;
		else if (player.Mod_Skill_Projectile>0.f)
			color = Color::blue;
		else color = Color::black;

		if (ARXmenu.currentmode==AMCM_NEWQUEST)
		{
			if (player.Skill_Projectile == 0)
				color = Color::red;
		}

		DrawBookTextCenter(hFontInBook, 469, 319, tex, color);
		
		ss3.str("");
		ss3 << player.Full_Skill_Defense;
		tex = ss3.str();

		if (player.Mod_Skill_Defense<0.f)
			color = Color::red;
		else if (player.Mod_Skill_Defense>0.f)
			color = Color::blue;
		else color = Color::black;

		if (ARXmenu.currentmode==AMCM_NEWQUEST)
		{
			if (player.Skill_Defense == 0)
				color = Color::red;
		}

		DrawBookTextCenter(hFontInBook, 533, 319, tex, color);

		// Secondary Attributes
		std::stringstream ss4;
		ss4.str("");
		ss4 << F2L_RoundUp(player.Full_maxlife);
		tex = ss4.str();

		if ((player.Mod_maxlife<0.f) || (player.Full_maxlife < player.maxlife))
			color = Color::red;
		else if ((player.Mod_maxlife>0.f) || (player.Full_maxlife > player.maxlife))
			color = Color::blue;
		else color = Color::black;

		DrawBookTextCenter( hFontInBook, 324, 158, tex, color );
		
		ss4.str("");
		ss4 << F2L_RoundUp(player.Full_maxmana);
		tex = ss4.str();

		if ((player.Mod_maxmana<0.f) || (player.Full_maxmana < player.maxmana))
			color = Color::red;
		else if ((player.Mod_maxmana>0.f) || (player.Full_maxmana > player.maxmana))
			color = Color::blue;
		else color = Color::black;

		DrawBookTextCenter( hFontInBook, 324, 218, tex, color );
		
		ss4.str("");
		ss4 << F2L_RoundUp(player.Full_damages);
		tex = ss4.str();

		if (player.Mod_damages<0.f)
			color = Color::red;
		else if (player.Mod_damages>0.f)
			color = Color::blue;
		else color = Color::black;

		DrawBookTextCenter(hFontInBook, 324, 278, tex, color);

		float ac = player.Full_armor_class;
		ss4.str("");
		ss4 << F2L_RoundUp(ac);
		tex = ss4.str();

		if (player.Mod_armor_class<0.f)
			color = Color::red;
		else if (player.Mod_armor_class>0.f)
			color = Color::blue;
		else color = Color::black;

		DrawBookTextCenter(hFontInBook, 153, 158, tex, color);

		ss4.str("");
		ss4 << std::setw(3) << std::setprecision(0) << F2L_RoundUp( player.Full_resist_magic );
		tex = ss4.str();

		if (player.Mod_resist_magic<0.f)
			color = Color::red;
		else if (player.Mod_resist_magic>0.f)
			color = Color::blue;
		else color = Color::black;

		DrawBookTextCenter(hFontInBook, 153, 218, tex, color);
		
		ss4.str("");
		ss4 << F2L_RoundUp( player.Full_resist_poison );
		tex = ss4.str();

		if (player.Mod_resist_poison<0.f)
			color = Color::red;
		else if (player.Mod_resist_poison>0.f)
			color = Color::blue;
		else color = Color::black;

		DrawBookTextCenter(hFontInBook, 153, 278, tex, color);
	}
	else if (Book_Mode == BOOKMODE_MINIMAP)
	{
		long SHOWLEVEL = Book_MapPage - 1;

		if (SHOWLEVEL >= 0 && SHOWLEVEL < 32)
			ARX_MINIMAP_Show( SHOWLEVEL, 0);

		SHOWLEVEL = ARX_LEVELS_GetRealNum(CURRENTLEVEL);

		if (SHOWLEVEL >= 0 && SHOWLEVEL < 32)
			ARX_MINIMAP_Show( SHOWLEVEL, 1);
		
	}

	if ((Book_Mode == BOOKMODE_STATS) && (inter.iobj[0]->obj != NULL))
	{

		GRenderer->SetRenderState(Renderer::DepthWrite, true);

		Rect rec;
		if (BOOKZOOM) {
			
			rec = Rect(s32((118.f + BOOKDECX) * Xratio), s32((69.f + BOOKDECY) * Yratio),
			           s32((280.f + BOOKDECY) * Xratio), s32((310.f + BOOKDECY) * Yratio));
			GRenderer->Clear(Renderer::DepthBuffer, Color::none, 1.f, 1, &rec);

			if(ARXmenu.currentmode != AMCM_OFF) {
				Rect vp = Rect(Vec2i(s32(139.f * Xratio), 0), s32(139.f * Xratio), s32(310.f * Yratio));
				GRenderer->SetViewport(vp);
			}
		} else {
			
			rec = Rect(s32((118.f + BOOKDECX) * Xratio), s32((69.f + BOOKDECY) * Yratio),
			          s32((350.f + BOOKDECX) * Xratio), s32((338.f + BOOKDECY) * Yratio));
			GRenderer->Clear(Renderer::DepthBuffer, Color::none, 1.f, 1, &rec);

			rec.right -= 50;
		}

		if (ARXmenu.currentmode==AMCM_OFF)
			BOOKZOOM=0;

		Vec3f pos;
		EERIE_LIGHT eLight1;
		EERIE_LIGHT eLight2;
		eLight1.pos.x=50.f;
		eLight1.pos.y=50.f;
		eLight1.pos.z=200.f;

		eLight1.exist=1;
		eLight1.rgb.r = 0.15f;
		eLight1.rgb.g = 0.06f;
		eLight1.rgb.b = 0.003f;
		eLight1.intensity = 8.8f;
		eLight1.fallstart = 2020;
		eLight1.fallend=eLight1.fallstart+60;
		eLight1.rgb255.r=eLight1.rgb.r*255.f;
		eLight1.rgb255.g=eLight1.rgb.g*255.f;
		eLight1.rgb255.b=eLight1.rgb.b*255.f;
		eLight1.falldiff=eLight1.fallend-eLight1.fallstart;
		eLight1.falldiffmul=1.f/eLight1.falldiff;
		eLight1.precalc=eLight1.intensity*GLOBAL_LIGHT_FACTOR;

		eLight2.exist=1;
		eLight2.pos.x=-50.f;
		eLight2.pos.y=-50.f;
		eLight2.pos.z = -200.f;
		eLight2.rgb.r=0.6f;
		eLight2.rgb.g=0.6f;
		eLight2.rgb.b=0.6f;
		eLight2.intensity=3.8f;
		eLight2.fallstart = 0;
		eLight2.fallend=eLight2.fallstart+3460.f;
		eLight2.rgb255.r=eLight2.rgb.r*255.f;
		eLight2.rgb255.g=eLight2.rgb.g*255.f;
		eLight2.rgb255.b=eLight2.rgb.b*255.f;
		eLight2.falldiff=eLight2.fallend-eLight2.fallstart;
		eLight2.falldiffmul=1.f/eLight2.falldiff;
		eLight2.precalc=eLight2.intensity*GLOBAL_LIGHT_FACTOR;

		EERIE_LIGHT * SavePDL[2];
		SavePDL[0]=PDL[0];
		SavePDL[1]=PDL[1];
		int			iSavePDL=TOTPDL;

		PDL[0] = &eLight1;
		PDL[1] = &eLight2;
		TOTPDL = 2;

		EERIE_CAMERA * oldcam=ACTIVECAM;
		bookcam.centerx = rec.center().x;
		bookcam.centery = rec.center().y;
		SetActiveCamera(&bookcam);
		PrepareCamera(&bookcam);

		if(BOOKZOOM) {
			Rect vp;
			vp.left = static_cast<int>(rec.left + 52.f * Xratio);
			vp.top = rec.top;
			vp.right = static_cast<int>(rec.right - 21.f * Xratio);
			vp.bottom = static_cast<int>(rec.bottom - 17.f * Yratio);
			GRenderer->SetViewport(vp);
		} else {
			GRenderer->SetViewport(rec);
		}

		ePlayerAngle.a=0.f;
		ePlayerAngle.g=0.f;

		if (BOOKZOOM)
		{
			pos.x=8;
			pos.y=162.f;
			pos.z=75.f;
			eLight1.pos.z=-90.f;
		}
		else
		{
			ePlayerAngle.b=-20.f;
			pos.x=20.f;
			pos.y=96.f;
			pos.z=260.f;
		}

		if (!BOOKZOOM)
			ARX_EQUIPMENT_AttachPlayerWeaponToHand();

		long ti=Project.improve;
		Project.improve=0;

		if (inter.iobj[0]->invisibility>0.f)
		{
			GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
			GRenderer->SetRenderState(Renderer::AlphaBlending, true);
		}

		INVISIBILITY_OVERRIDE=inter.iobj[0]->invisibility;

		if (INVISIBILITY_OVERRIDE>0.5f)
			INVISIBILITY_OVERRIDE=0.5f;

		FORCE_NO_HIDE=1;
		IN_BOOK_DRAW=1;
		vector<EERIE_VERTEX> vertexlist = inter.iobj[0]->obj->vertexlist3;

		if(player.useanim.cur_anim != NULL) {
			EERIEDrawAnimQuat(inter.iobj[0]->obj, &player.useanim, &ePlayerAngle, &pos,
			                  checked_range_cast<unsigned long>(Original_framedelay), NULL);
		} else {
			DrawEERIEInter(inter.iobj[0]->obj, &ePlayerAngle, &pos, NULL);
		}

		INVISIBILITY_OVERRIDE=0;
		IN_BOOK_DRAW=0;

		if(ARXmenu.currentmode == AMCM_NEWQUEST) {
			GRenderer->SetRenderState(Renderer::DepthTest, true);
			GRenderer->GetTextureStage(0)->SetMipFilter(TextureStage::FilterNone);
			GRenderer->SetRenderState(Renderer::AlphaBlending, false);
			PopAllTriangleList();
			GRenderer->SetRenderState(Renderer::AlphaBlending, true);
			PopAllTriangleListTransparency();
			GRenderer->SetRenderState(Renderer::AlphaBlending, false);
			GRenderer->GetTextureStage(0)->SetMipFilter(TextureStage::FilterLinear);
			GRenderer->SetRenderState(Renderer::DepthTest, false);
		}

		PDL[0]=SavePDL[0];
		PDL[1]=SavePDL[1];
		TOTPDL=iSavePDL;

		inter.iobj[0]->obj->vertexlist3 = vertexlist;
		vertexlist.clear();
		FORCE_NO_HIDE=0;
		Project.improve=ti;

		GRenderer->SetViewport(Rect(DANAESIZX, DANAESIZY));

		GRenderer->SetRenderState(Renderer::AlphaBlending, false);
		GRenderer->SetCulling(Renderer::CullNone);
		SetActiveCamera(oldcam);

		INTERACTIVE_OBJ * io=inter.iobj[0];

		if (io)
		{
			player.useanim.cur_anim = herowaitbook;

			if (	(player.equiped[EQUIP_SLOT_WEAPON]!=0)
				&&	ValidIONum(player.equiped[EQUIP_SLOT_WEAPON]	))
			{
				if (inter.iobj[player.equiped[EQUIP_SLOT_WEAPON]]->type_flags & OBJECT_TYPE_2H)
				{
					player.useanim.cur_anim = herowait_2h;
				}
			}

			GRenderer->SetCulling(Renderer::CullNone);

			if (	(player.equiped[EQUIP_SLOT_ARMOR]!=0)
				&&	ValidIONum(player.equiped[EQUIP_SLOT_ARMOR]	))
			{
				INTERACTIVE_OBJ * tod=inter.iobj[player.equiped[EQUIP_SLOT_ARMOR]];

				if (tod)
				{


					tod->bbox1.x = 195;
					tod->bbox1.y = 116;
					tod->bbox2.x = 284;
					tod->bbox2.y = 182;

					float fX1 = (tod->bbox1.x+BOOKDECX) * Xratio ;
					float fX2 = (tod->bbox2.x+BOOKDECX) * Xratio ;
					float fY1 = (tod->bbox1.y+BOOKDECY) * Yratio ;
					float fY2 = (tod->bbox2.y+BOOKDECY) * Yratio ;

					tod->bbox1.x = checked_range_cast<short>(fX1);
					tod->bbox2.x = checked_range_cast<short>(fX2);
					tod->bbox1.y = checked_range_cast<short>(fY1);
					tod->bbox2.y = checked_range_cast<short>(fY2);

					tod->ioflags |= IO_ICONIC;
				}
			}

			if (	(player.equiped[EQUIP_SLOT_LEGGINGS]!=0)
				&&	ValidIONum(player.equiped[EQUIP_SLOT_LEGGINGS]	))
			{
				INTERACTIVE_OBJ * tod=inter.iobj[player.equiped[EQUIP_SLOT_LEGGINGS]];

				if (tod)
				{


					tod->bbox1.x = 218;
					tod->bbox1.y = 183;
					tod->bbox2.x = 277;
					tod->bbox2.y = 322;

					float fX1 = (tod->bbox1.x+BOOKDECX) * Xratio ;
					float fX2 = (tod->bbox2.x+BOOKDECX) * Xratio ;
					float fY1 = (tod->bbox1.y+BOOKDECY) * Yratio ;
					float fY2 = (tod->bbox2.y+BOOKDECY) * Yratio ;

					tod->bbox1.x = checked_range_cast<short>(fX1);
					tod->bbox2.x = checked_range_cast<short>(fX2);
					tod->bbox1.y = checked_range_cast<short>(fY1);
					tod->bbox2.y = checked_range_cast<short>(fY2);

					tod->ioflags|=IO_ICONIC;
				}
			}

			if (	(player.equiped[EQUIP_SLOT_HELMET]!=0)
				&&	ValidIONum(player.equiped[EQUIP_SLOT_HELMET]	))
			{
				INTERACTIVE_OBJ * tod=inter.iobj[player.equiped[EQUIP_SLOT_HELMET]];

				if (tod)
				{


					tod->bbox1.x = 218;
					tod->bbox1.y = 75;
					tod->bbox2.x = 260;
					tod->bbox2.y = 115;

					float fX1 = (tod->bbox1.x+BOOKDECX) * Xratio ;
					float fX2 = (tod->bbox2.x+BOOKDECX) * Xratio ;
					float fY1 = (tod->bbox1.y+BOOKDECY) * Yratio ;
					float fY2 = (tod->bbox2.y+BOOKDECY) * Yratio ;

					tod->bbox1.x = checked_range_cast<short>(fX1);
					tod->bbox2.x = checked_range_cast<short>(fX2);
					tod->bbox1.y = checked_range_cast<short>(fY1);
					tod->bbox2.y = checked_range_cast<short>(fY2);

					tod->ioflags |= IO_ICONIC;
				}
			}

			TextureContainer * tc;
			TextureContainer * tc2=NULL;
			GRenderer->SetCulling(Renderer::CullNone);

			if (	(player.equiped[EQUIP_SLOT_RING_LEFT]!=0)
				&&	ValidIONum(player.equiped[EQUIP_SLOT_RING_LEFT]	))
			{
				INTERACTIVE_OBJ * todraw=inter.iobj[player.equiped[EQUIP_SLOT_RING_LEFT]];

				tc=todraw->inv;

				if (NeedHalo(todraw)) tc2 = todraw->inv->getHalo();

				if (tc)
				{
					todraw->bbox1.x=146;
					todraw->bbox1.y=312;

					Color color = (todraw->poisonous && todraw->poisonous_count != 0) ? Color::green : Color::white;
					DrawBookInterfaceItem(tc, todraw->bbox1.x, todraw->bbox1.y, color, 0);

					if (tc2!=NULL)
					{
						ARX_INTERFACE_HALO_Draw(todraw,tc,tc2,todraw->bbox1.x*Xratio,todraw->bbox1.y*Yratio, Xratio, Yratio);
					}

					float fWidth  = todraw->bbox1.x + static_cast<float>( tc->m_dwWidth );
					float fHeight = todraw->bbox1.y + static_cast<float>( tc->m_dwHeight );
					todraw->bbox2.x = checked_range_cast<short>(fWidth);
					todraw->bbox2.y = checked_range_cast<short>(fHeight);

					float fX1 = (todraw->bbox1.x+BOOKDECX) * Xratio ;
					float fX2 = (todraw->bbox2.x+BOOKDECX) * Xratio ;
					float fY1 = (todraw->bbox1.y+BOOKDECY) * Yratio ;
					float fY2 = (todraw->bbox2.y+BOOKDECY) * Yratio ;
					todraw->bbox1.x = checked_range_cast<short>(fX1);
					todraw->bbox2.x = checked_range_cast<short>(fX2);
					todraw->bbox1.y = checked_range_cast<short>(fY1);
					todraw->bbox2.y = checked_range_cast<short>(fY2);

					todraw->ioflags |= IO_ICONIC;
				}
			}

			tc2=NULL;

			if (	(player.equiped[EQUIP_SLOT_RING_RIGHT]!=0)
				&&	ValidIONum(player.equiped[EQUIP_SLOT_RING_RIGHT]	))
			{
				INTERACTIVE_OBJ * todraw=inter.iobj[player.equiped[EQUIP_SLOT_RING_RIGHT]];

				tc=todraw->inv;

				if (NeedHalo(todraw)) tc2=todraw->inv->getHalo();

				if (tc)
				{
					todraw->bbox1.x=296;
					todraw->bbox1.y=312;

					Color color = (todraw->poisonous && todraw->poisonous_count != 0) ? Color::green : Color::white;
					DrawBookInterfaceItem(tc, todraw->bbox1.x, todraw->bbox1.y, color, 0);

					if (tc2!=NULL)
					{
						ARX_INTERFACE_HALO_Draw(todraw,tc,tc2,todraw->bbox1.x*Xratio,todraw->bbox1.y*Yratio, Xratio, Yratio);
					}

					float fWidth  = todraw->bbox1.x + static_cast<float>( tc->m_dwWidth );
					float fHeight = todraw->bbox1.y + static_cast<float>( tc->m_dwHeight );
					todraw->bbox2.x = checked_range_cast<short>(fWidth);
					todraw->bbox2.y = checked_range_cast<short>(fHeight);

					float fX1 = (todraw->bbox1.x+BOOKDECX) * Xratio ;
					float fX2 = (todraw->bbox2.x+BOOKDECX) * Xratio ;
					float fY1 = (todraw->bbox1.y+BOOKDECY) * Yratio ;
					float fY2 = (todraw->bbox2.y+BOOKDECY) * Yratio ;

					todraw->bbox1.x = checked_range_cast<short>(fX1);
					todraw->bbox2.x = checked_range_cast<short>(fX2);
					todraw->bbox1.y = checked_range_cast<short>(fY1);
					todraw->bbox2.y = checked_range_cast<short>(fY2);

					todraw->ioflags |= IO_ICONIC;
				}
			}

			if (!BOOKZOOM)
				ARX_EQUIPMENT_AttachPlayerWeaponToBack();

			//blue halo rendering (keyword : BLUE HALO RENDERING HIGHLIGHT AURA)
			if (HALOCUR>0)
			{
				GRenderer->ResetTexture(0);
				GRenderer->SetBlendFunc(Renderer::BlendSrcColor, Renderer::BlendOne);
				GRenderer->SetRenderState(Renderer::AlphaBlending, true);			
				GRenderer->SetCulling(Renderer::CullNone);
				GRenderer->SetRenderState(Renderer::DepthWrite, false);

				for (int i=0;i<HALOCUR;i++)
				{
					TexturedVertex * vert=&LATERDRAWHALO[(i<<2)];

					if (vert[2].color == 0)
					{
						GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);
						vert[2].color =0xFF000000;
						EERIEDRAWPRIM(Renderer::TriangleFan, vert, 4); //>>> DO NOT USE VERTEX BUFFER HERE <<<
						GRenderer->SetBlendFunc(Renderer::BlendSrcColor, Renderer::BlendOne);
					}
					else EERIEDRAWPRIM(Renderer::TriangleFan, vert, 4); //>>> DO NOT USE VERTEX BUFFER HERE <<<
				}

				HALOCUR=0;
				GRenderer->SetRenderState(Renderer::AlphaBlending, false);
			}
		}
	}	
}

//-----------------------------------------------------------------------------
void ArxGame::DrawAllInterfaceFinish()
{
	currpos = static_cast<long>(INTERFACE_RATIO(50.f));
	float rrr;
	rrr=1.f-PULSATE*0.5f;

	if (rrr>1.f) rrr=1.f;
	else if (rrr<0.f) rrr=0.f;

	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	PRECAST_NUM=0;

	for(size_t i = 0; i < MAX_SPELLS; i++) {
		if ((spells[i].exist) && (spells[i].caster==0))
			if (spellicons[spells[i].type].bDuration)
				ManageSpellIcon(i,rrr,0);
	}

	if (inter.iobj[0])
	{
		for (int i=0;i<inter.iobj[0]->nb_spells_on;i++)
		{
			if (spells[inter.iobj[0]->spells_on[i]].caster!=0)
				if (spellicons[spells[i].type].bDuration)
					ManageSpellIcon(inter.iobj[0]->spells_on[i],rrr,1);
		}
	}

	if (!(player.Interface & INTER_INVENTORYALL) && !(player.Interface & INTER_MAP))
	{
		for(size_t i = 0; i < MAX_PRECAST; i++) {
			PRECAST_NUM=i;

			if (Precast[i].typ!=-1)
			{
				float val=rrr;

				if ((Precast[i].launch_time>0) &&(float(arxtime) >= Precast[i].launch_time))
				{
					float tt=(float(arxtime) - Precast[i].launch_time)*( 1.0f / 1000 );

					if (tt>1.f) tt=1.f;

					val*=(1.f-tt);
				}

				ManageSpellIcon(Precast[i].typ,val,2);
			}
		}
	}
}

void ARX_INTERFACE_DrawCurrentTorch()
{
	if ((player.Interface & INTER_NOTE) && (TSecondaryInventory != NULL)
		&& ((openNote.type() == gui::Note::BigNote) || (openNote.type() == gui::Note::Book))
		)
		return;

	float px, py;

	px = INTERFACE_RATIO(InventoryX) + INTERFACE_RATIO(110);

	if (px < INTERFACE_RATIO(10)) px = INTERFACE_RATIO(10);

	py = DANAESIZY - INTERFACE_RATIO(158+32);

	EERIEDrawBitmap(px, py, INTERFACE_RATIO_DWORD(CURRENT_TORCH->inv->m_dwWidth),
	                INTERFACE_RATIO_DWORD(CURRENT_TORCH->inv->m_dwHeight),
	                0.001f, CURRENT_TORCH->inv, Color::white);

	if ( rnd() > 0.2f )
	{
		long j=ARX_PARTICLES_GetFree();

		if ( ( j != -1 ) && ( !arxtime.is_paused() ) )
		{
			ParticleCount++;
			PARTICLE_DEF * pd	=	&particle[j];
			pd->special			=	FIRE_TO_SMOKE;
			pd->exist			=	true;
			pd->zdec			=	0;
			pd->ov.x 			=	px + INTERFACE_RATIO( 12 ) + rnd() * INTERFACE_RATIO( 3.f ) - rnd() * INTERFACE_RATIO( 6.f );
			pd->ov.y 			=	py + rnd() * INTERFACE_RATIO( 6.f );
			pd->ov.z			=	0.0000001f;
			pd->move.x 			=	INTERFACE_RATIO(1.5f)-rnd()*INTERFACE_RATIO(3.f);
			pd->move.y 			=	-INTERFACE_RATIO(5.f)+rnd()*INTERFACE_RATIO(1.f);
			pd->move.z 			=	0.f;
			pd->scale.x 		=	1.8f;
			pd->scale.y 		=	1.8f;
			pd->scale.z 		=	1.f;
			pd->timcreation		=	(long)arxtime;
			pd->tolive			=	Random::get(500, 900);
			pd->tc				=	fire2;
			pd->rgb = Color3f(1.f, .6f, .5f);
			pd->siz				=	INTERFACE_RATIO(14.f);
			pd->type			=	PARTICLE_2D;
		}
	}
}

extern float GLOBAL_SLOWDOWN;
extern long SPLASH_THINGS_STAGE;

void ArxGame::DrawAllInterface() {
	
	GRenderer->GetTextureStage(0)->SetMinFilter(TextureStage::FilterLinear);
	GRenderer->GetTextureStage(0)->SetMagFilter(TextureStage::FilterNearest);
	GRenderer->GetTextureStage(0)->SetWrapMode(TextureStage::WrapClamp);

	if (!EDITMODE)
	{
		//---------------------------------------------------------------------
		if (player.Interface & INTER_COMBATMODE)
		{
			float j;

			if (AimTime==0) j=0.2f;
			else
			{
				if (BOW_FOCAL)
				{
					j=(float)(BOW_FOCAL)/710.f;
				}
				else
				{
					float at=float(arxtime)-(float)AimTime;

					if (at>0.f)
						bIsAiming = true;
					else
						bIsAiming = false;

					at=at*(1.f+(1.f-GLOBAL_SLOWDOWN));
					float aim = static_cast<float>(player.Full_AimTime);
					j=at/aim;
				}

				if (j>1.f) j=1.f;
				else if (j<0.2f) j=0.2f;

			}

			GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);

			GRenderer->SetRenderState(Renderer::AlphaBlending, true);
			ARX_INTERFACE_DrawItem(ITC.Get("aim_maxi"), DANAECENTERX + INTERFACE_RATIO(-320+262.f), DANAESIZY + INTERFACE_RATIO(-72.f), 0.0001f, Color::gray(j));
			GRenderer->SetRenderState(Renderer::AlphaBlending, false);
			ARX_INTERFACE_DrawItem(ITC.Get("aim_empty"), DANAECENTERX + INTERFACE_RATIO(-320+262.f), DANAESIZY + INTERFACE_RATIO(-72.f), 0.0001f, Color::white);

			if(bHitFlash && player.Full_Skill_Etheral_Link >= 40){
				float j = 1.0f - fHitFlash;
				GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
				GRenderer->SetRenderState(Renderer::AlphaBlending, true);
				Color col = (j < 0.5f) ? Color3f(j*2.0f, 1, 0).to<u8>() : Color3f(1, fHitFlash, 0).to<u8>();
				ARX_INTERFACE_DrawItem(ITC.Get("aim_hit"), DANAECENTERX + INTERFACE_RATIO(-320+262.f-25), DANAESIZY + INTERFACE_RATIO(-72.f-30), 0.0001f, col);
				GRenderer->SetRenderState(Renderer::AlphaBlending, false);
			}
		}

		if (bHitFlash)
		{
			float fCalc = ulHitFlash + Original_framedelay ;

			ulHitFlash = checked_range_cast<unsigned long>(fCalc);

			if (ulHitFlash >= 500)
			{
				bHitFlash = false;
				ulHitFlash = 0;
			}
		}

		//---------------------------------------------------------------------
		INTERACTIVE_OBJ * io = NULL;

		if (SecondaryInventory!=NULL)
		{
			io = (INTERACTIVE_OBJ *)SecondaryInventory->io;
		}
		else if (player.Interface & INTER_STEAL)
		{
			io = ioSteal;
		}

		if (io!=NULL)
		{
			float dist = fdist(io->pos, player.pos + (Vec3f::Y_AXIS * 80.f));

			if (Project.telekinesis)
			{
				if (dist > 900.f)
				{
					if (InventoryDir != -1)
					{
						ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());

						InventoryDir=-1;
						SendIOScriptEvent(io,SM_INVENTORY2_CLOSE);
						TSecondaryInventory=SecondaryInventory;
						SecondaryInventory=NULL;
					}
					else
					{
						if (player.Interface & INTER_STEAL)
						{
							player.Interface &= ~INTER_STEAL;
						}
					}
				}
			}
			else if (dist > 350.f)
			{
				if (InventoryDir != -1)
				{
					ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());

					InventoryDir=-1;
					SendIOScriptEvent(io,SM_INVENTORY2_CLOSE);
					TSecondaryInventory=SecondaryInventory;
					SecondaryInventory=NULL;
				}
				else
				{
					if (player.Interface & INTER_STEAL)
					{
						player.Interface &= ~INTER_STEAL;
					}
				}
			}
		}
		else if (InventoryDir != -1)
		{
			InventoryDir = -1;
		}

		if (!PLAYER_INTERFACE_HIDE_COUNT && (TSecondaryInventory != NULL))
		{
			ARX_INTERFACE_DrawSecondaryInventory((bool)((player.Interface & INTER_STEAL) != 0));
		}

		if (!PLAYER_INTERFACE_HIDE_COUNT)
		{
			if ((player.Interface & INTER_INVENTORY))
			{

				if ((player.Interface & INTER_COMBATMODE) || (player.doingmagic>=2))
				{
					long t = Original_framedelay * ( 1.0f / 5 ) + 2;
					InventoryY += static_cast<long>(INTERFACE_RATIO_LONG(t));

					if ( InventoryY > INTERFACE_RATIO( 110.f ) ) InventoryY = static_cast<long>( INTERFACE_RATIO( 110.f ) );


				}
				else
				{
					if (bInventoryClosing)
					{
						long t = Original_framedelay * ( 1.0f / 5 ) + 2;
						InventoryY += static_cast<long>(INTERFACE_RATIO_LONG(t));

						if (InventoryY > INTERFACE_RATIO(110))
						{
							InventoryY = static_cast<long>( INTERFACE_RATIO( 110.f ) );
							bInventoryClosing = false;

							player.Interface &=~ INTER_INVENTORY;

							if (bInventorySwitch)
							{
								bInventorySwitch = false;
								ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
								player.Interface |= INTER_INVENTORYALL;
								ARX_INTERFACE_NoteClose();
								InventoryY = static_cast<long>( INTERFACE_RATIO( 121.f ) * (player.bag) );
								lOldInterface=INTER_INVENTORYALL;
							}
						}
					}
					else if (InventoryY>0)
					{
						InventoryY -= static_cast<long>( INTERFACE_RATIO( ( Original_framedelay * ( 1.0f / 5 ) ) + 2.f ) );


						if (InventoryY<0) InventoryY=0;
					}
				}

				if (player.bag)
				{
					ARX_INTERFACE_DrawInventory(sActiveInventory);

					arx_assert(ITC.Get("hero_inventory") != NULL);
					float fCenterX	= DANAECENTERX + INTERFACE_RATIO(-320 + 35) + INTERFACE_RATIO_DWORD(ITC.Get("hero_inventory")->m_dwWidth) - INTERFACE_RATIO(32 + 3) ;
					float fSizY		= DANAESIZY - INTERFACE_RATIO(101) + INTERFACE_RATIO_LONG(InventoryY) + INTERFACE_RATIO(- 3 + 25) ;

					float posx = ARX_CAST_TO_INT_THEN_FLOAT( fCenterX );
					float posy = ARX_CAST_TO_INT_THEN_FLOAT( fSizY );

					if (sActiveInventory > 0)
					{
						ARX_INTERFACE_DrawItem(ITC.Get("hero_inventory_up"),	posx, posy);

						if (MouseInRect(posx, posy, posx+INTERFACE_RATIO(32), posy+INTERFACE_RATIO(32)))
						{
							GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
							GRenderer->SetRenderState(Renderer::AlphaBlending, true);
							SpecialCursor=CURSOR_INTERACTION_ON;
							ARX_INTERFACE_DrawItem(ITC.Get("hero_inventory_up"), posx, posy);
							GRenderer->SetRenderState(Renderer::AlphaBlending, false);
							SpecialCursor=CURSOR_INTERACTION_ON;

							if (((EERIEMouseButton & 1)  && !(LastMouseClick & 1))
								|| ((!(EERIEMouseButton & 1)) && (LastMouseClick & 1) && (DRAGINTER!=NULL)))
							{
								if (sActiveInventory > 0)
								{
									ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
									sActiveInventory --;
								}

								EERIEMouseButton &=~1;
							}
						}
					}

					if (sActiveInventory < player.bag-1)
					{

						float fRatio = INTERFACE_RATIO(32 + 5);

						posy += checked_range_cast<int>(fRatio);

						ARX_INTERFACE_DrawItem(ITC.Get("hero_inventory_down"),	posx, DANAESIZY - INTERFACE_RATIO(101) + INTERFACE_RATIO_LONG(InventoryY) + INTERFACE_RATIO(-3 + 64));

						if (MouseInRect(posx, posy, posx+INTERFACE_RATIO(32), posy+INTERFACE_RATIO(32)))
						{
							GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
							GRenderer->SetRenderState(Renderer::AlphaBlending, true);
							ARX_INTERFACE_DrawItem(ITC.Get("hero_inventory_down"),	posx, DANAESIZY - INTERFACE_RATIO(101) + INTERFACE_RATIO_LONG(InventoryY) + INTERFACE_RATIO(-3 + 64));
							GRenderer->SetRenderState(Renderer::AlphaBlending, false);
							SpecialCursor=CURSOR_INTERACTION_ON;

							if (((EERIEMouseButton & 1)  && !(LastMouseClick & 1))
								|| ((!(EERIEMouseButton & 1)) && (LastMouseClick & 1) && (DRAGINTER!=NULL)))
							{
								if (sActiveInventory < player.bag-1)
								{
									ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
									sActiveInventory ++;
								}

								EERIEMouseButton &=~1;
							}
						}
					}
				}
			}
			else if ((player.Interface & INTER_INVENTORYALL) || bInventoryClosing)
			{
				float fSpeed = ( 1.0f / 3 );

				if ((player.Interface & INTER_COMBATMODE) || (player.doingmagic>=2))
				{
					if (InventoryY < INTERFACE_RATIO(121)*player.bag)
					{

						InventoryY += static_cast<long>(INTERFACE_RATIO((Original_framedelay * fSpeed) + 2.f));
					}
				}
				else
				{
					if (bInventoryClosing)
					{
						InventoryY += static_cast<long>(INTERFACE_RATIO((Original_framedelay * fSpeed) + 2.f));

						if ( InventoryY > INTERFACE_RATIO(121) * player.bag )
						{
							bInventoryClosing = false;

							if (player.Interface & INTER_INVENTORYALL)
							{
								player.Interface &=~ INTER_INVENTORYALL;
							}

							lOldInterface=0;
						}
					}
					else if (InventoryY>0)
					{
						InventoryY -= static_cast<long>(INTERFACE_RATIO((Original_framedelay * fSpeed) + 2.f));

						if (InventoryY<0)
						{
							InventoryY=0;
						}
					}
				}

				const float fBag		= (player.bag-1)*INTERFACE_RATIO(-121);
				float fCenterX	= DANAECENTERX + INTERFACE_RATIO(-320+35);
				float fSizY		= DANAESIZY - INTERFACE_RATIO(101) + INTERFACE_RATIO_LONG(InventoryY) + INTERFACE_RATIO(-3.f + 25 - 32);
				const float fOffsetY	= INTERFACE_RATIO(121);

				int iOffsetY = checked_range_cast<int>(fBag + fOffsetY);
				int posx = checked_range_cast<int>(fCenterX);
				int posy = checked_range_cast<int>(fSizY);

				for (int i=0; i<player.bag; i++)
				{
					ARX_INTERFACE_DrawItem(ITC.Get("hero_inventory_link"), posx + INTERFACE_RATIO(45), static_cast<float>(posy + iOffsetY)) ;

					ARX_INTERFACE_DrawItem(ITC.Get("hero_inventory_link"), posx+INTERFACE_RATIO_DWORD(ITC.Get("hero_inventory")->m_dwWidth)*0.5f + INTERFACE_RATIO(-16), posy+iOffsetY + INTERFACE_RATIO(-5));
					ARX_INTERFACE_DrawItem(ITC.Get("hero_inventory_link"), posx+INTERFACE_RATIO_DWORD(ITC.Get("hero_inventory")->m_dwWidth) + INTERFACE_RATIO(-45-32), posy+iOffsetY + INTERFACE_RATIO(-15));

					iOffsetY += checked_range_cast<int>(fOffsetY);
				}

				iOffsetY = checked_range_cast<int>(fBag);

				for(short i = 0; i < player.bag; i++) {
					ARX_INTERFACE_DrawInventory(i, 0, iOffsetY);
					iOffsetY += checked_range_cast<int>(fOffsetY);
				}

			}
		}

		if (((FlyingOverIO) && !(PLAYER_MOUSELOOK_ON) && !(player.Interface & INTER_COMBATMODE)
			&& (!GInput->actionPressed(CONTROLS_CUST_MAGICMODE)))
			|| 
			(((FlyingOverIO) && (config.input.autoReadyWeapon == false) && !(player.Interface & INTER_COMBATMODE)
			&& (!GInput->actionPressed(CONTROLS_CUST_MAGICMODE))))
			)
		{
			if ((FlyingOverIO->ioflags & IO_ITEM) && (!DRAGINTER))
				if (SecondaryInventory!=NULL)
				{
					INTERACTIVE_OBJ * temp=(INTERACTIVE_OBJ *)SecondaryInventory->io;

					if (temp->ioflags & IO_SHOP)
					{
						float px	=	DANAEMouse.x;
						float py	=	static_cast<float>(DANAEMouse.y - 10);

						if (InSecondaryInventoryPos(&DANAEMouse))
						{
							long amount=ARX_INTERACTIVE_GetPrice(FlyingOverIO,temp);
							// achat
							float famount	= amount - amount * ( (float)player.Full_Skill_Intuition ) * 0.005f;
							// check should always be OK because amount is supposed positive
							amount = checked_range_cast<long>(famount);


							if ( amount <= player.gold )
								ARX_INTERFACE_DrawNumber(px, py, amount, 6, Color::green);
							else
								ARX_INTERFACE_DrawNumber(px, py, amount, 6, Color::red);
						}
						else if (InPlayerInventoryPos(&DANAEMouse))
						{
							long amount = static_cast<long>( ARX_INTERACTIVE_GetPrice( FlyingOverIO, temp ) / 3.0f );
							// achat
							float famount	= amount + amount * ( (float) player.Full_Skill_Intuition ) * 0.005f;
							// check should always be OK because amount is supposed positive
							amount = checked_range_cast<long>( famount );


							if (amount)
							{
								if (temp->shop_category.empty() ||
									FlyingOverIO->groups.find(temp->shop_category) != FlyingOverIO->groups.end())
									ARX_INTERFACE_DrawNumber(px, py, amount, 6, Color::green);
								else
									ARX_INTERFACE_DrawNumber(px, py, amount, 6, Color::red);
							}
						}
					}
				}

				SpecialCursor=CURSOR_INTERACTION_ON;
		}

		ARX_INTERFACE_DrawDamagedEquipment();

		if ((!(player.Interface & INTER_COMBATMODE)))
		{
			if (player.Interface & INTER_MINIBACK)
			{
				// Draw/Manage Book Icon
				float px=DANAESIZX - INTERFACE_RATIO(35) + lSLID_VALUE+GL_DECAL_ICONS;
				float py=DANAESIZY - INTERFACE_RATIO(148);
				ARX_INTERFACE_DrawItem(ITC.Get("book"), px, py);

				if (eMouseState == MOUSE_IN_BOOK_ICON)
				{
					GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
					GRenderer->SetRenderState(Renderer::AlphaBlending, true);
					ARX_INTERFACE_DrawItem(ITC.Get("book"), px, py);
					GRenderer->SetRenderState(Renderer::AlphaBlending, false);
				}

				// Draw/Manage BackPack Icon
				px=DANAESIZX - INTERFACE_RATIO(35) + lSLID_VALUE+GL_DECAL_ICONS;
				py=DANAESIZY - INTERFACE_RATIO(113);
				ARX_INTERFACE_DrawItem(ITC.Get("backpack"),px,py);

				if (eMouseState == MOUSE_IN_INVENTORY_ICON)
				{
					GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
					GRenderer->SetRenderState(Renderer::AlphaBlending, true);
					ARX_INTERFACE_DrawItem(ITC.Get("backpack"),px,py);
					GRenderer->SetRenderState(Renderer::AlphaBlending, false);
				}

				// Draw/Manage Steal Icon
				if (player.Interface & INTER_STEAL)
				{
					px = static_cast<float>(-lSLID_VALUE);
					py = DANAESIZY - INTERFACE_RATIO(78.f + 32);
					ARX_INTERFACE_DrawItem(ITC.Get("steal"), px, py);

					if (eMouseState == MOUSE_IN_STEAL_ICON)
					{
						GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
						GRenderer->SetRenderState(Renderer::AlphaBlending, true);
						ARX_INTERFACE_DrawItem(ITC.Get("steal"), px, py);
						GRenderer->SetRenderState(Renderer::AlphaBlending, false);
					}
				}

				// Draw / Manage Pick All - Close Secondary inventory icon
				if (!PLAYER_INTERFACE_HIDE_COUNT && (TSecondaryInventory!=NULL))
				{
					px = INTERFACE_RATIO(InventoryX) + INTERFACE_RATIO(16);
					py = INTERFACE_RATIO_DWORD(BasicInventorySkin->m_dwHeight) - INTERFACE_RATIO(16);
					INTERACTIVE_OBJ * temp=(INTERACTIVE_OBJ *)TSecondaryInventory->io;

					if (temp && !(temp->ioflags & IO_SHOP) && !(temp == ioSteal))
					{
						ARX_INTERFACE_DrawItem(ITC.Get("inventory_pickall"), px, py);

						if (eMouseState == MOUSE_IN_INVENTORY_PICKALL_ICON)
						{
							GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
							GRenderer->SetRenderState(Renderer::AlphaBlending, true);
							ARX_INTERFACE_DrawItem(ITC.Get("inventory_pickall"), px, py);
							GRenderer->SetRenderState(Renderer::AlphaBlending, false);
						}
					}

					px = INTERFACE_RATIO(InventoryX) + INTERFACE_RATIO_DWORD(BasicInventorySkin->m_dwWidth) - INTERFACE_RATIO(32);

					ARX_INTERFACE_DrawItem(ITC.Get("inventory_close"), px, py);

					if (eMouseState == MOUSE_IN_INVENTORY_CLOSE_ICON)
					{
						GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
						GRenderer->SetRenderState(Renderer::AlphaBlending, true);
						ARX_INTERFACE_DrawItem(ITC.Get("inventory_close"), px, py);
						GRenderer->SetRenderState(Renderer::AlphaBlending, false);
					}
				}

				// Draw/Manage Advancement Icon
				if ((player.Skill_Redistribute) || (player.Attribute_Redistribute))
				{
					px=DANAESIZX - INTERFACE_RATIO(35) + lSLID_VALUE+GL_DECAL_ICONS;
					py=DANAESIZY - INTERFACE_RATIO(218);
					ARX_INTERFACE_DrawItem(ITC.Get("icon_lvl_up"),px,py);		

					if (eMouseState == MOUSE_IN_REDIST_ICON)
					{
						GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
						GRenderer->SetRenderState(Renderer::AlphaBlending, true);
						ARX_INTERFACE_DrawItem(ITC.Get("icon_lvl_up"),px,py);		
						GRenderer->SetRenderState(Renderer::AlphaBlending, false);	
					}			  
				}

				// Draw/Manage Gold Purse Icon
				if (player.gold>0)
				{
					px = DANAESIZX - INTERFACE_RATIO(35) + lSLID_VALUE+2+GL_DECAL_ICONS;
					py = DANAESIZY - INTERFACE_RATIO(183);
					ARX_INTERFACE_DrawItem(ITC.Get("gold"), px, py);

					if (eMouseState == MOUSE_IN_GOLD_ICON)
					{
						GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
						GRenderer->SetRenderState(Renderer::AlphaBlending, true);
						SpecialCursor=CURSOR_INTERACTION_ON;
						ARX_INTERFACE_DrawItem(ITC.Get("gold"), px, py);
						GRenderer->SetRenderState(Renderer::AlphaBlending, false);
						ARX_INTERFACE_DrawNumber(px - INTERFACE_RATIO(30), py + INTERFACE_RATIO(10 - 25), player.gold, 6, Color::white);
					}
				}

				if (bGoldHalo)
				{
					float fCalc = ulGoldHaloTime + Original_framedelay;
					ulGoldHaloTime = checked_range_cast<unsigned long>(fCalc);


					if (ulGoldHaloTime >= 1000) //1seconde
					{
						bGoldHalo = false;
					}

					TextureContainer *tc = ITC.Get("gold");
					TextureContainer *halo = tc->getHalo();

					if (halo)
					{
						ARX_INTERFACE_HALO_Render(0.9f, 0.9f, 0.1f, HALO_ACTIVE, halo, px, py, INTERFACE_RATIO(1), INTERFACE_RATIO(1));
					}
				}

				if (bBookHalo)
				{


					float fCalc = ulBookHaloTime + Original_framedelay;
					ulBookHaloTime = checked_range_cast<unsigned long>(fCalc);


					if (ulBookHaloTime >= 3000) //3secondes
					{
						bBookHalo = false;
					}

					float POSX = DANAESIZX-INTERFACE_RATIO(35)+lSLID_VALUE+GL_DECAL_ICONS;
					float POSY = DANAESIZY-INTERFACE_RATIO(148);
					TextureContainer *tc = ITC.Get("book");
					TextureContainer *halo = tc->getHalo();

					if (halo)
					{
						ARX_INTERFACE_HALO_Render(0.2f, 0.4f, 0.8f, HALO_ACTIVE, halo, POSX, POSY, INTERFACE_RATIO(1), INTERFACE_RATIO(1));
					}
				}
			}

			if (CURRENT_TORCH)
				ARX_INTERFACE_DrawCurrentTorch();
		}

		if ((CHANGE_LEVEL_ICON>-1) && (ChangeLevel) )
		{

			//Setting px and py as float to avoid warning on function ARX_INTERFACE_DrawItem and MouseInRect
			float px = DANAESIZX - INTERFACE_RATIO_DWORD(ChangeLevel->m_dwWidth);
			float py = 0;

			float vv = 0.9f - EEsin(arxtime.get_frame_time()*( 1.0f / 50 ))*0.5f+rnd()*( 1.0f / 10 );

			if ( vv < 0.f ) vv = 0;
			else if ( vv > 1.f ) vv = 1.f;

			ARX_INTERFACE_DrawItem(ChangeLevel, px, py, 0.0001f, Color::gray(vv));

			if (MouseInRect(px, py, px + INTERFACE_RATIO_DWORD(ChangeLevel->m_dwWidth), py + INTERFACE_RATIO_DWORD(ChangeLevel->m_dwHeight)))
			{
				SpecialCursor=CURSOR_INTERACTION_ON;

				if (!(EERIEMouseButton & 1) && (LastMouseClick & 1))
				{
					CHANGE_LEVEL_ICON=200;
				}
			}
		}


		// Draw stealth gauge
		if (SPLASH_THINGS_STAGE<11)
			ARX_INTERFACE_Draw_Stealth_Gauge();

		// book
		if ((player.Interface & INTER_MAP )&&  (!(player.Interface & INTER_COMBATMODE)))
		{
			ARX_INTERFACE_ManageOpenedBook();
			ARX_INTERFACE_ManageOpenedBook_Finish();
		}


		if (CurrSpellSymbol || player.SpellToMemorize.bSpell)
		{
			int count = 0;
			int count2 = 0;

			for (long j=0;j<6;j++)
			{
				if (player.SpellToMemorize.iSpellSymbols[j] != RUNE_NONE)
					count++;

				if (SpellSymbol[j] != RUNE_NONE)
					count2 ++;
			}

			count = (count2>count)?count2:count;
			Vec3f pos;
			pos.x = DANAESIZX - ((count) * INTERFACE_RATIO(32));

			if (CHANGE_LEVEL_ICON>-1) pos.x -= INTERFACE_RATIO(32);

			pos.y = 0;

			for (int i=0; i<6; i++)
			{
				bool bHalo = false;

				if (SpellSymbol[i] != RUNE_NONE)
				{
					if (SpellSymbol[i] == player.SpellToMemorize.iSpellSymbols[i])
					{
						bHalo = true;
					}
					else
					{
						player.SpellToMemorize.iSpellSymbols[i] = SpellSymbol[i];

						for (int j=i+1; j<6; j++)
						{
							player.SpellToMemorize.iSpellSymbols[j] = RUNE_NONE;
						}
					}
				}

				if (player.SpellToMemorize.iSpellSymbols[i] != RUNE_NONE)
				{
					EERIEDrawBitmap2(pos.x, pos.y, INTERFACE_RATIO(32), INTERFACE_RATIO(32), 0,
						necklace.pTexTab[player.SpellToMemorize.iSpellSymbols[i]], Color::white);

					if (bHalo)
					{
						TextureContainer *tc = necklace.pTexTab[player.SpellToMemorize.iSpellSymbols[i]];
						TextureContainer *halo = tc->getHalo();

						if (halo)
						{
							ARX_INTERFACE_HALO_Render(0.2f, 0.4f, 0.8f, HALO_ACTIVE, halo, pos.x, pos.y, INTERFACE_RATIO(1), INTERFACE_RATIO(1));
						}
					}

					if (!(player.rune_flags & (RuneFlag)(1<<player.SpellToMemorize.iSpellSymbols[i])))
					{
						GRenderer->SetBlendFunc(Renderer::BlendInvDstColor, Renderer::BlendOne);
						GRenderer->SetRenderState(Renderer::AlphaBlending, true);

						EERIEDrawBitmap2(pos.x, pos.y, INTERFACE_RATIO(32), INTERFACE_RATIO(32), 0,
							Movable, Color::gray(.8f));
						GRenderer->SetRenderState(Renderer::AlphaBlending, false);
					}

					pos.x += INTERFACE_RATIO(32);
				}
			}

			if (float(arxtime) - player.SpellToMemorize.lTimeCreation > 30000)
			{
				player.SpellToMemorize.bSpell = false;
			}
		}

		if (player.Interface & INTER_LIFE_MANA)
		{
			TexturedVertex v[4];
			float px, py;
			v[0] = TexturedVertex(Vec3f(0, 0, .001f), 1.f, Color::white.toBGR(), 1, Vec2f::ZERO);
			v[1] = TexturedVertex(Vec3f(0, 0, .001f), 1.f, Color::white.toBGR(), 1, Vec2f::X_AXIS);
			v[2] = TexturedVertex(Vec3f(0, 0, .001f), 1.f, Color::white.toBGR(), 1, Vec2f(1.f, 1.f));
			v[3] = TexturedVertex(Vec3f(0, 0, .001f), 1.f, Color::white.toBGR(), 1, Vec2f::Y_AXIS);

			px = DANAESIZX - INTERFACE_RATIO(33) + INTERFACE_RATIO(1) + lSLID_VALUE;
			py = DANAESIZY - INTERFACE_RATIO(81);
			ARX_INTERFACE_DrawItem(ITC.Get("empty_gauge_blue"), px, py, 0.f); //399

			float fnl=(float)player.life/(float)player.Full_maxlife;
			float fnm=(float)player.mana/(float)player.Full_maxmana;


			//---------------------------------------------------------------------
			//RED GAUGE
			Color ulcolor = Color::red;
			float fSLID_VALUE_neg = static_cast<float>(-lSLID_VALUE);

			if(player.poison > 0.f) {
				float val = min(player.poison, 0.2f) * 255.f * 5.f;
				long g = val;
				ulcolor = Color(u8(255 - g), u8(g) , 0);
			}

			EERIEDrawBitmap2DecalY(fSLID_VALUE_neg, DANAESIZY - INTERFACE_RATIO(78),
				INTERFACE_RATIO_DWORD(ITC.Get("filled_gauge_red")->m_dwWidth),
				INTERFACE_RATIO_DWORD(ITC.Get("filled_gauge_red")->m_dwHeight),
				0.f, ITC.Get("filled_gauge_red"), ulcolor, (1.f - fnl));

			if (!(player.Interface & INTER_COMBATMODE))
			{
				if (MouseInRect(fSLID_VALUE_neg, DANAESIZY - INTERFACE_RATIO(78), fSLID_VALUE_neg + INTERFACE_RATIO_DWORD(ITC.Get("filled_gauge_red")->m_dwWidth), DANAESIZY - INTERFACE_RATIO(78) + INTERFACE_RATIO_DWORD(ITC.Get("filled_gauge_red")->m_dwHeight)))
				{
					if(	(EERIEMouseButton & 1)&&
						(!(LastMouseClick & 1)) ) {
							std::stringstream ss;
							ss << checked_range_cast<int>(player.life);
							ARX_SPEECH_Add(ss.str());
					}
				}
			}

			//---------------------------------------------------------------------
			//END RED GAUGE

			px = 0.f-lSLID_VALUE;
			py = DANAESIZY - INTERFACE_RATIO(78);
			ARX_INTERFACE_DrawItem(ITC.Get("empty_gauge_red"), px, py, 0.001f);

			//---------------------------------------------------------------------
			//BLUE GAUGE

			float LARGG=INTERFACE_RATIO_DWORD(ITC.Get("filled_gauge_blue")->m_dwWidth);
			float HAUTT=INTERFACE_RATIO_DWORD(ITC.Get("filled_gauge_blue")->m_dwHeight);

			EERIEDrawBitmap2DecalY(DANAESIZX - INTERFACE_RATIO(33) + INTERFACE_RATIO(1) + lSLID_VALUE,
				DANAESIZY - INTERFACE_RATIO(81), LARGG, HAUTT, 0.f,
				ITC.Get("filled_gauge_blue"), Color::white, (1.f - fnm));

			if (!(player.Interface & INTER_COMBATMODE))
			{
				if(MouseInRect(DANAESIZX - INTERFACE_RATIO(33) + lSLID_VALUE,DANAESIZY - INTERFACE_RATIO(81),DANAESIZX - INTERFACE_RATIO(33) + lSLID_VALUE+LARGG,DANAESIZY - INTERFACE_RATIO(81)+HAUTT))
				{
					if(	(EERIEMouseButton & 1)&&
						(!(LastMouseClick & 1)) ) {
							std::stringstream ss;
							ss << checked_range_cast<int>(player.mana);
							ARX_SPEECH_Add(ss.str());
					}
				}
			}

			//---------------------------------------------------------------------
			//END BLUE GAUGE
			if (bRenderInCursorMode)
			{
				GRenderer->SetRenderState(Renderer::AlphaBlending, true);
				GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);

				if (mecanism_tc && (MAGICMODE < 0) && (lNbToDrawMecanismCursor < 3))
				{
					Color lcolorMecanism = Color::white;
					if(lTimeToDrawMecanismCursor > 300) {
						lcolorMecanism = Color::black;
						if(lTimeToDrawMecanismCursor > 400) {
							lTimeToDrawMecanismCursor=0;
							lNbToDrawMecanismCursor++;
						}
					}

					lTimeToDrawMecanismCursor += static_cast<long>(FrameDiff);
					EERIEDrawBitmap(0, 0, INTERFACE_RATIO_DWORD(mecanism_tc->m_dwWidth), INTERFACE_RATIO_DWORD(mecanism_tc->m_dwHeight), 0.01f, mecanism_tc, lcolorMecanism);
				}

				if (arrow_left_tc)
				{
					float fSizeX=INTERFACE_RATIO_DWORD(arrow_left_tc->m_dwWidth);
					float fSizeY=INTERFACE_RATIO_DWORD(arrow_left_tc->m_dwHeight);
					Color lcolor = Color::gray(.5f);
					static float fArrowMove=0.f;
					fArrowMove+=.5f*FrameDiff;

					if (fArrowMove > 180.f)
						fArrowMove=0.f;

					float fMove=fabs(sin(radians(fArrowMove)))*fSizeX*.5f;

					// Left
					EERIEDrawBitmap(0 + fMove, DANAECENTERY - (fSizeY * .5f), fSizeX, fSizeY, 0.01f,
						arrow_left_tc, lcolor);

					// Right
					EERIEDrawBitmapUVs(DANAESIZX - fSizeX - fMove, DANAECENTERY - (fSizeY * .5f), fSizeX, fSizeY,
						.01f, arrow_left_tc, lcolor, 1.f, 0.f, 0.f, 0.f, 1.f, 1.f, 0.f, 1.f);

					// Up
					EERIEDrawBitmapUVs(DANAECENTERX - (fSizeY * .5f), 0.f + fMove, fSizeY, fSizeX, .01f,
						arrow_left_tc, lcolor, 0.f, 1.f, 0.f, 0.f, 1.f, 1.f, 1.f, 0.f);

					// Down
					EERIEDrawBitmapUVs(DANAECENTERX - (fSizeY * .5f), (DANAESIZY - fSizeX) - fMove, fSizeY, fSizeX,
						.01f, arrow_left_tc, lcolor, 1.f, 1.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f);
				}

				GRenderer->SetRenderState(Renderer::AlphaBlending, false);
			}
		}
	}

	GRenderer->GetTextureStage(0)->SetMinFilter(TextureStage::FilterLinear);
	GRenderer->GetTextureStage(0)->SetMagFilter(TextureStage::FilterLinear);
	GRenderer->GetTextureStage(0)->SetWrapMode(TextureStage::WrapRepeat);
}

extern long FRAME_COUNT;
extern INTERACTIVE_OBJ * DESTROYED_DURING_RENDERING;
extern float STARTED_ANGLE;
long SPECIAL_DRAGINTER_RENDER=0;
long CANNOT_PUT_IT_HERE=0;

long Manage3DCursor(long flags)
{
	if (BLOCK_PLAYER_CONTROLS)
		return 0;

	float ag=player.angle.a;

	if (ag>180) ag=ag-360;

	float drop_miny=(float)(DANAECENTERY)-DANAECENTERY*(ag*( 1.0f / 70 ));

	if ((DANAEMouse.y<drop_miny) && (!EDITMODE))
		return 0;

	INTERACTIVE_OBJ * io=DRAGINTER;

	if (!io) return 0;

	Anglef temp;

	if (io->ioflags & IO_INVERTED)
	{
		temp.a=180.f;
		temp.b = -MAKEANGLE(io->angle.b - ((player.angle.b) - (STARTED_ANGLE + 90)));
	}
	else
	{
		temp.a = 0;
		temp.b = MAKEANGLE(io->angle.b - ((player.angle.b) - (STARTED_ANGLE + 90)));
	}

	temp.g = 0;
	Vec3f pos;
	float angle=radians(MAKEANGLE(player.angle.b));
	float angle2=radians(MAKEANGLE(player.angle.b-90.f));
				
	float zrange=(DANAESIZY-DANAEMouse.y)/(DANAESIZY-drop_miny); //between 0 (bottom) and 1 (top)
	float va=player.angle.a;

	if (va>180) va=0;
				
				float vd=(100.f-va)*( 1.0f / 90 );
				float mod=va-50.f;

				if (mod<0) mod=0;

				mod*=( 1.0f / 20 );
	va = (vd) * (1.3f + 0.3f * mod); 
				
				vd=((1.f-zrange)*0.6f-vd)*150.f;

				if (va<0) va=0;

				if (vd<0) vd=0;

				float mx = DANAEMouse.x;

				if (TRUE_PLAYER_MOUSELOOK_ON && (config.input.autoReadyWeapon))
				{
					mx = MemoMouse.x;
				}

				pos.x=player.pos.x+EEsin(angle2)*(DANAECENTERX-mx)*0.7f*va
					-EEsin(angle)*(va*zrange*400.f+vd);

				pos.z=player.pos.z-EEcos(angle2)*(DANAECENTERX-mx)*0.7f*va
					+EEcos(angle)*(va*zrange*400.f+vd);

				pos.y=player.pos.y;

				{
					Vec3f objcenter(0, 0, 0);
					float maxdist= 0.f;
					float miny=  99999999.f;
					float maxy= -99999999.f;
					Vec3f minoff;
					Vec3f maxoff;
					maxoff.x=minoff.x=io->obj->vertexlist[0].v.x;
					maxoff.y=minoff.y=io->obj->vertexlist[0].v.y;
					maxoff.z=minoff.z=io->obj->vertexlist[0].v.z;

					for (size_t i=0;i<io->obj->vertexlist.size();i++)
					{
						maxoff.x=max(maxoff.x,io->obj->vertexlist[i].v.x);
						maxoff.y=max(maxoff.y,io->obj->vertexlist[i].v.y);
						maxoff.z=max(maxoff.z,io->obj->vertexlist[i].v.z);
						minoff.x=min(minoff.x,io->obj->vertexlist[i].v.x);
						minoff.y=min(minoff.y,io->obj->vertexlist[i].v.y);
						minoff.z=min(minoff.z,io->obj->vertexlist[i].v.z);
						miny=min(miny,io->obj->vertexlist[i].v.y);
						maxy=max(maxy,io->obj->vertexlist[i].v.y);
					}


					EERIE_CYLINDER cyl;
					cyl.origin.x=pos.x-(maxoff.x-minoff.x)*0.5f;
					cyl.origin.y=pos.y;
					cyl.origin.z=pos.z-(maxoff.z-minoff.z)*0.5f;
					cyl.height=-50.f;
					cyl.radius=40.f;

					Vec3f orgn,dest,mvectx;
		mvectx.x = -(float)EEsin(radians(player.angle.b - 90.f)); 
		mvectx.y = 0; 
		mvectx.z = +(float)EEcos(radians(player.angle.b - 90.f)); 
					mvectx.normalize();

					float xmod=(float)(DANAEMouse.x-DANAECENTERX)/(float)DANAECENTERX*160.f;
					float ymod=(float)(DANAEMouse.y-DANAECENTERY)/(float)DANAECENTERY*220.f;
					mvectx *= xmod;
					Vec3f mvecty(0, ymod, 0);

					orgn.x=player.pos.x-(float)EEsin(radians(player.angle.b))*(float)EEcos(radians(player.angle.a))*50.f
		         + mvectx.x; 
					orgn.y=player.pos.y+(float)EEsin(radians(player.angle.a))*50.f
							+mvectx.y+mvecty.y;
					orgn.z=player.pos.z+(float)EEcos(radians(player.angle.b))*(float)EEcos(radians(player.angle.a))*50.f
		         + mvectx.z; 
	
					dest.x=player.pos.x-(float)EEsin(radians(player.angle.b))*(float)EEcos(radians(player.angle.a))*10000.f
		         + mvectx.x; 
					dest.y=player.pos.y+(float)EEsin(radians(player.angle.a))*10000.f
							+mvectx.y+mvecty.y*5.f;
					dest.z=player.pos.z+(float)EEcos(radians(player.angle.b))*(float)EEcos(radians(player.angle.a))*10000.f
		         + mvectx.z; 
					pos.x=orgn.x;
					pos.y=orgn.y;
					pos.z=orgn.z;

					Vec3f movev = (dest - orgn).getNormalized();

					float lastanything = 0.f;
					float height = -( maxy - miny );

					if ( height > -30.f ) height = -30.f;
					
		objcenter.x	=	minoff.x + (maxoff.x - minoff.x) * 0.5f;  
		objcenter.y	=	0;	
		objcenter.z	=	minoff.z + (maxoff.z - minoff.z) * 0.5f;  
					
					for ( size_t i = 0 ; i < io->obj->vertexlist.size() ; i++ )
					{
						maxdist = max(maxdist, dist(Vec2f(objcenter.x, objcenter.z),
			                               Vec2f(io->obj->vertexlist[i].v.x, io->obj->vertexlist[i].v.z)) - 4.f);
					}

					if (io->obj->pbox)
					{
						for (int i=1;i<io->obj->pbox->nb_physvert;i++)
						{
							maxdist = max(maxdist, dist(Vec2f(io->obj->pbox->vert[0].initpos.x,
												io->obj->pbox->vert[0].initpos.z),
												Vec2f(io->obj->pbox->vert[i].initpos.x,
				                               io->obj->pbox->vert[i].initpos.z)) + 14.f);
						}
					}

					VRotateY(&objcenter,temp.b);

					if ( maxdist < 15.f ) maxdist = 15.f;

					if ( maxdist > 150.f ) maxdist = 150.f;

		bool			bCollidposNoInit = true;
					Vec3f		collidpos;
					EERIE_CYLINDER	cyl2;
					float			inc			=	10.f;
					long			iterating	=	40;

		collidpos.x = collidpos.y = collidpos.z = 0;
		
		cyl2.height	=	min(-30.f, height); 
		cyl2.radius	=	max(20.f, maxdist); 
					

					while ( iterating>0 )
					{

			cyl2.origin.x = pos.x + movev.x * inc;
			cyl2.origin.y = pos.y + maxy + movev.y * inc;
			cyl2.origin.z = pos.z + movev.z * inc;

						float anything = CheckAnythingInCylinder( &cyl2, io, CFLAG_JUST_TEST | CFLAG_COLLIDE_NOCOL | CFLAG_NO_NPC_COLLIDE );

						if ( anything < 0.f )
						{
							if ( iterating == 40 )
							{
								CANNOT_PUT_IT_HERE = 1;
								return -1;
							}

							iterating = 0;

							collidpos = cyl2.origin;
				bCollidposNoInit = false;

							if ( lastanything < 0.f )
							{
								pos.y		+= lastanything;
								collidpos.y	+= lastanything;
							}
						}
						else
						{
							pos.x			= cyl2.origin.x;
							pos.y			= cyl2.origin.y;
							pos.z			= cyl2.origin.z;
							lastanything	= anything;
						}

						iterating--;
					}

					collidpos.x	-=	objcenter.x;
					collidpos.z	-=	objcenter.z;
					pos.x		-=	objcenter.x;
					pos.z		-=	objcenter.z;

					if ( iterating != -1 )
					{
						CANNOT_PUT_IT_HERE = 1;
						return 0;
					}

		if ((iterating == -1) && closerThan(player.pos, pos, 300.f))
					{
						if ( flags & 1 )
						{
							io->obj->drawflags |= DRAWFLAG_HIGHLIGHT;
							ARX_INTERACTIVE_Teleport( io, &pos, 1 );

							int iOldFrameCount	=	FRAME_COUNT;
							FRAME_COUNT			=	0;
							io->GameFlags		&=	~GFLAG_NOCOMPUTATION;

							float old			=	io->invisibility;


							if( bCollidposNoInit )
							{
					ARX_DEAD_CODE();
							}

							if (SPECIAL_DRAGINTER_RENDER)
							{

							if (((lastanything<0.f) && (EEfabs(lastanything)>EEfabs(height)))
								|| (lastanything>EEfabs(height)))
							{
						io->invisibility = 0.5f;
								DrawEERIEInter(io->obj,&temp,&collidpos,io);
								io->invisibility=old;
							}
							else if (lastanything>0.f)
						DrawEERIEInter( io->obj, &temp, &pos, io);
							else
								DrawEERIEInter(io->obj,&temp,&pos,io);
							}

							PrecalcIOLighting(NULL,0,1);
							FRAME_COUNT=iOldFrameCount;

							if (	(!SPECIAL_DRAGINTER_RENDER)
								&&	(INTERTRANSPOLYSPOS)	)
								ARXDRAW_DrawAllInterTransPolyPos();

							if (!SPECIAL_DRAGINTER_RENDER && !DESTROYED_DURING_RENDERING)
							{
								io->obj->drawflags&=~DRAWFLAG_HIGHLIGHT;

								if (io->ignition>0.f)
									ManageIgnition(io);
							}
						}
						else
						{
				if (EEfabs(lastanything) > min(EEfabs(height), 12.0f))
							{
								INTERACTIVE_OBJ * io=DRAGINTER;
								ARX_PLAYER_Remove_Invisibility();
								io->obj->pbox->active=1;
								io->obj->pbox->stopcount=0;
								Vec3f pos;
					pos.x = io->pos.x = collidpos.x;
					pos.z = io->pos.z = collidpos.z;
								pos.y=io->pos.y=collidpos.y;
								io->velocity.x=0.f;
								io->velocity.y=0.f;
								io->velocity.z=0.f;

								io->stopped=1;
								

								movev.x*=0.0001f;
								movev.z*=0.0001f;
								movev.y=0.1f;
								Vec3f viewvector = movev;

								Anglef angle;
								angle.a=temp.a;
								angle.b=temp.b;
								angle.g=temp.g;
								io->soundtime=0;
								io->soundcount=0;
								EERIE_PHYSICS_BOX_Launch(io->obj,&pos,&viewvector,1,&angle);
								ARX_SOUND_PlaySFX(SND_WHOOSH, &pos);
								io->show=SHOW_FLAG_IN_SCENE;
								Set_DragInter(NULL);
							}
							else
							{

								ARX_PLAYER_Remove_Invisibility();
								ARX_SOUND_PlayInterface(SND_INVSTD);
								ARX_INTERACTIVE_Teleport(io,&pos,1);

								io->angle.a=temp.a;
								io->angle.b=270.f-temp.b;
								io->angle.g=temp.g;
								io->stopped=0;
								io->show=SHOW_FLAG_IN_SCENE;
								io->obj->pbox->active=0;
								Set_DragInter(NULL);
							}
						}

						GRenderer->SetCulling(Renderer::CullNone);
						return 1;
					}
					else
					{
						CANNOT_PUT_IT_HERE=-1;
					}
				}

				return 0;

}
//-----------------------------------------------------------------------------
void ARX_INTERFACE_RenderCursorInternal(long flag)
{
	if (!SPECIAL_DRAGINTER_RENDER)
	{
		ManageIgnition_2(DRAGINTER);
		GRenderer->GetTextureStage(0)->SetMinFilter(TextureStage::FilterNearest);
		GRenderer->GetTextureStage(0)->SetMagFilter(TextureStage::FilterNearest);
		GRenderer->GetTextureStage(0)->SetWrapMode(TextureStage::WrapClamp);
	}

	TextureContainer * surf;

	if (!SPECIAL_DRAGINTER_RENDER)
	{
		if (LOOKING_FOR_SPELL_TARGET)
		{
			if (float(arxtime) > LOOKING_FOR_SPELL_TARGET_TIME+7000)
			{
				ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE, &player.pos);
				ARX_SPELLS_CancelSpellTarget();
			}

			if (	(FlyingOverIO)
				&&	(	((LOOKING_FOR_SPELL_TARGET & 1) && (FlyingOverIO->ioflags & IO_NPC))
				|| ((LOOKING_FOR_SPELL_TARGET & 2) && (FlyingOverIO->ioflags & IO_ITEM))	)	)
			{
				surf=ITC.Get("target_on");

				if (!(EERIEMouseButton & 1) && (LastMouseClick & 1))
				{
					ARX_SPELLS_LaunchSpellTarget(FlyingOverIO);
				}
			}
			else
			{
				surf=ITC.Get("target_off");

				if(GInput->actionPressed(CONTROLS_CUST_MAGICMODE))
				{
					ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE, &player.pos);
					ARX_SPELLS_CancelSpellTarget();
				}
			}

			float POSX=DANAEMouse.x;
			float POSY=DANAEMouse.y;

			if (TRUE_PLAYER_MOUSELOOK_ON)
			{
				POSX = MemoMouse.x;
				POSY = MemoMouse.y;
			}


			float fTexSizeX = INTERFACE_RATIO_DWORD(surf->m_dwWidth);
			float fTexSizeY = INTERFACE_RATIO_DWORD(surf->m_dwHeight);

			EERIEDrawBitmap((float)(POSX-(fTexSizeX*0.5f)), (float)(POSY-(surf->m_dwHeight*0.5f)), fTexSizeX, fTexSizeY, 0.f, surf, Color::white);

			return;
		}
	}

	if (flag || ((!BLOCK_PLAYER_CONTROLS) &&
		(!PLAYER_INTERFACE_HIDE_COUNT)))
	{

		if (!SPECIAL_DRAGINTER_RENDER)
			GRenderer->SetCulling(Renderer::CullNone);

		if ((COMBINE) || (COMBINEGOLD))
		{
			if (SpecialCursor==CURSOR_INTERACTION_ON)
				SpecialCursor=CURSOR_COMBINEON;
			else SpecialCursor=CURSOR_COMBINEOFF;
		}

		if ((SpecialCursor) || !PLAYER_MOUSELOOK_ON || (DRAGINTER!=NULL)
		        || ((FlyingOverIO) && PLAYER_MOUSELOOK_ON && !(ARX_MOUSE_OVER & ARX_MOUSE_OVER_BOOK)
			&& (eMouseState != MOUSE_IN_NOTE)
			&& (FlyingOverIO->ioflags & IO_ITEM)
			&& (FlyingOverIO->GameFlags & GFLAG_INTERACTIVITY)
			&& (config.input.autoReadyWeapon == false))
			|| ((MAGICMODE==1) && PLAYER_MOUSELOOK_ON))
		{

			if (!SPECIAL_DRAGINTER_RENDER)
			{
				if(FlyingOverIO||DRAGINTER)
				{
					fHighLightAng+=(float)(FrameDiff*0.5);

					if(fHighLightAng>90.f) fHighLightAng=90.f;


					float fHLight	= 100.f*sin(radians(fHighLightAng));

					iHighLight = checked_range_cast<int>(fHLight);


				}
				else
				{
					 fHighLightAng=0.f;
					iHighLight = 0;
				}
			}


			CANNOT_PUT_IT_HERE=0;
			float ag=player.angle.a;

			if (ag>180) ag=ag-360;

			float drop_miny=(float)(DANAECENTERY)-DANAECENTERY*(ag*( 1.0f / 70 ));

			if ((DANAEMouse.y>drop_miny) && DRAGINTER && !InInventoryPos(&DANAEMouse)
			        && !(ARX_MOUSE_OVER & ARX_MOUSE_OVER_BOOK))
			{
				if (Manage3DCursor(1)==0)
					CANNOT_PUT_IT_HERE = -1;

				if (SPECIAL_DRAGINTER_RENDER)
				{

					CANNOT_PUT_IT_HERE=0;
					return;
				}

			}
			else CANNOT_PUT_IT_HERE = -1;

			if (SPECIAL_DRAGINTER_RENDER)
				return;

			float POSX,POSY;

			POSX=(float)DANAEMouse.x;
			POSY=(float)DANAEMouse.y;


			if ((SpecialCursor) && (!DRAGINTER))
			{
				if (((COMBINE!=NULL) && (COMBINE->inv!=NULL)) || COMBINEGOLD)
				{
					if (TRUE_PLAYER_MOUSELOOK_ON && (config.input.autoReadyWeapon))
					{
						POSX = MemoMouse.x;
						POSY = MemoMouse.y;
					}

					TextureContainer * tc;

					if (COMBINEGOLD) tc=GoldCoinsTC[5];
					else tc=COMBINE->inv;

					float MODIF=0.f;


					float fTexSizeX = INTERFACE_RATIO_DWORD(tc->m_dwWidth);
					float fTexSizeY = INTERFACE_RATIO_DWORD(tc->m_dwHeight);


					if (SpecialCursor==CURSOR_COMBINEON)
					{
						EERIEDrawBitmap(POSX + MODIF, POSY + MODIF, fTexSizeX, fTexSizeY, .00001f, tc, Color::white);

						if ((FlyingOverIO!=NULL) && (FlyingOverIO->ioflags & IO_BLACKSMITH))
						{
							float v=ARX_DAMAGES_ComputeRepairPrice(COMBINE,FlyingOverIO);

							if (v>0.f)
							{								
								long t = v;
								ARX_INTERFACE_DrawNumber(POSX + MODIF - 16, POSY + MODIF - 10, t, 6, Color::cyan);
							}
						}
					}
					else
						EERIEDrawBitmap(POSX + MODIF, POSY + MODIF, fTexSizeX, fTexSizeY, 0.00001f, tc, Color::fromBGRA(0xFFFFAA66));
				}

				switch (SpecialCursor)
				{
				case CURSOR_REDIST:
					{
						surf = ITC.Get("ptexcursorredist");
					}
					break;
				case CURSOR_COMBINEOFF:
					surf=ITC.Get("target_off");
						POSX -= 16.f;
						POSY -= 16.f;
					break;
				case CURSOR_COMBINEON:
					surf=ITC.Get("target_on");

					if (surf)
							POSX -= 16.f;

						POSY -= 16.f;
					break;
				case CURSOR_FIREBALLAIM: {
					surf=ITC.Get("target_on");

					Vec2i size;
					if (surf)
					{
						size.x	=	surf->m_dwWidth;
						size.y	=	surf->m_dwHeight;
					}

					else
					{
						ARX_DEAD_CODE();
						size.x = 0;
						size.y = 0;
					}



					POSX = 320.f - size.x / 2.f;
					POSY = 280.f - size.y / 2.f;
					break;
				}
				case CURSOR_INTERACTION_ON:

					CURCURTIME += checked_range_cast<long>(Original_framedelay);


					if (CURCURPOS!=3)
					{
						while(CURCURTIME>CURCURDELAY)
						{
							CURCURTIME-=CURCURDELAY;
							CURCURPOS++;
						}
					}

					if (CURCURPOS>7) CURCURPOS=0;

					surf=scursor[CURCURPOS];
					break;
				default:
					if (CURCURPOS!=0)
					{

						CURCURTIME += checked_range_cast<long>(Original_framedelay);

						while(CURCURTIME>CURCURDELAY)
						{
							CURCURTIME-=CURCURDELAY;
							CURCURPOS++;
						}
					}

					if (CURCURPOS>7) CURCURPOS=0;

					surf=scursor[CURCURPOS];
					break;
				}

				if (surf)
				{

					if (SpecialCursor == CURSOR_REDIST)
					{
						EERIEDrawBitmap(POSX, POSY, surf->m_dwWidth * Xratio, surf->m_dwHeight * Yratio,
						                0.f, surf, Color::white);
						
						std::stringstream ss;
						ss << std::setw(3) << lCursorRedistValue;
						ARX_TEXT_Draw(hFontInBook, DANAEMouse.x + 6* Xratio, DANAEMouse.y + 11* Yratio, ss.str(), Color::black);
					}
					else
					{

						float fTexSizeX = INTERFACE_RATIO_DWORD(surf->m_dwWidth);
						float fTexSizeY = INTERFACE_RATIO_DWORD(surf->m_dwHeight);

						EERIEDrawBitmap(POSX, POSY, fTexSizeX, fTexSizeY, 0.f, surf, Color::white);
					}
				}

				SpecialCursor=0;
			}
			else
			{
				if (!(player.Current_Movement & PLAYER_CROUCH) && (!BLOCK_PLAYER_CONTROLS
					&& (GInput->actionPressed(CONTROLS_CUST_MAGICMODE)))
					&& (ARXmenu.currentmode==AMCM_OFF))
				{
					if (MAGICMODE<0)
					{
						if (player.Interface & INTER_MAP )
						{
							ARX_INTERFACE_BookOpenClose(2); // Forced Closing
						}

						MAGICMODE=1;
					}

					surf=ITC.Get("magic");

					float POSX=DANAEMouse.x;
					float POSY=DANAEMouse.y;

					if (TRUE_PLAYER_MOUSELOOK_ON)
					{
						POSX = MemoMouse.x;
						POSY = MemoMouse.y;
					}


					float fTexSizeX = INTERFACE_RATIO_DWORD(surf->m_dwWidth);
					float fTexSizeY = INTERFACE_RATIO_DWORD(surf->m_dwHeight);


					EERIEDrawBitmap(POSX - (fTexSizeX*0.5f), POSY - (fTexSizeY*0.5f), fTexSizeX, fTexSizeY,
					                0.f, surf, Color::white);
				}
				else
				{
					if (MAGICMODE>-1)
					{
						ARX_SOUND_Stop(SND_MAGIC_DRAW);
						MAGICMODE=-1;
					}

					if ((DRAGINTER!=NULL) && (DRAGINTER->inv!=NULL))
					{
						TextureContainer * tc;
						TextureContainer * tc2=NULL;
						tc=DRAGINTER->inv;

						if (NeedHalo(DRAGINTER)) tc2=DRAGINTER->inv->getHalo();//>_itemdata->halo_tc;

						Color color = (DRAGINTER->poisonous && DRAGINTER->poisonous_count != 0) ? Color::green : Color::white;

						float mx = POSX;
						float my = POSY;

						if (TRUE_PLAYER_MOUSELOOK_ON && (config.input.autoReadyWeapon))
						{
							mx = MemoMouse.x;
							my = MemoMouse.y;
						}

						float fTexSizeX = INTERFACE_RATIO_DWORD(tc->m_dwWidth);
						float fTexSizeY = INTERFACE_RATIO_DWORD(tc->m_dwHeight);


						if (!(DRAGINTER->ioflags & IO_MOVABLE))
						{
							EERIEDrawBitmap(mx, my, fTexSizeX, fTexSizeY, .00001f, tc, color);

							if ((DRAGINTER->ioflags & IO_ITEM) && (DRAGINTER->_itemdata->count!=1))
								ARX_INTERFACE_DrawNumber(mx + 2.f, my + 13.f, DRAGINTER->_itemdata->count, 3, Color::white);
						}
						else
						{
							if ((InInventoryPos(&DANAEMouse) || InSecondaryInventoryPos(&DANAEMouse))
								|| (CANNOT_PUT_IT_HERE != -1)) {
								EERIEDrawBitmap(mx, my, fTexSizeX, fTexSizeY, .00001f, tc, color);
							}
						}

						//cross not over inventory icon
						if ((CANNOT_PUT_IT_HERE!=0) && (eMouseState != MOUSE_IN_INVENTORY_ICON))
						{
							if (!InInventoryPos(&DANAEMouse) && !InSecondaryInventoryPos(&DANAEMouse) && !ARX_INTERFACE_MouseInBook())
							{
								TextureContainer * tcc=Movable;

								if (CANNOT_PUT_IT_HERE==-1)
									tcc=ThrowObject;

								if ((tcc) && (tcc!=tc)) // to avoid movable double red cross...
									EERIEDrawBitmap(mx + 16, my, INTERFACE_RATIO_DWORD(tcc->m_dwWidth),
									                INTERFACE_RATIO_DWORD(tcc->m_dwHeight), 0.00001f, tcc, Color::white);
							}
						}

						if (tc2)
						{
							ARX_INTERFACE_HALO_Draw(DRAGINTER,tc,tc2,mx,my, INTERFACE_RATIO(1), INTERFACE_RATIO(1));
						}
					}
					else
					{
						if(CURCURPOS != 0) {
							CURCURTIME += checked_range_cast<long>(Original_framedelay);
							while(CURCURTIME > CURCURDELAY) {
								CURCURTIME -= CURCURDELAY;
								CURCURPOS++;
							}
						}

							if (CURCURPOS>7) CURCURPOS=0;

							surf=scursor[CURCURPOS];

						if (surf)
						{
							EERIEDrawBitmap(POSX, POSY, INTERFACE_RATIO_DWORD(surf->m_dwWidth),
							                INTERFACE_RATIO_DWORD(surf->m_dwHeight), 0.f, surf, Color::white);
						}
					}
				}
			}
		}
		else //mode system shock
		{
			if (SPECIAL_DRAGINTER_RENDER)
				return;

			if(FlyingOverIO||DRAGINTER)
			{
				fHighLightAng+=(float)(FrameDiff*0.5f);

				if(fHighLightAng>90.f) fHighLightAng=90.f;

				float fHLight	= 100.f*sin(radians(fHighLightAng));
				iHighLight = checked_range_cast<int>(fHLight);
			}
			else
			{
				fHighLightAng=0.f;
				iHighLight = 0;
			}

			if(TRUE_PLAYER_MOUSELOOK_ON && config.video.showCrosshair) {
				if(!(player.Interface & (INTER_COMBATMODE | INTER_NOTE | INTER_MAP))) {
					
					CURCURPOS = 0;
					
					surf = pTCCrossHair ? pTCCrossHair : ITC.Get("target_off");
					
					if(surf) {
						
						GRenderer->SetRenderState(Renderer::AlphaBlending, true);
						GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
						
						float POSX = DANAECENTERX - surf->m_dwWidth * .5f;
						float POSY = DANAECENTERY - surf->m_dwHeight * .5f;
						
						EERIEDrawBitmap(POSX, POSY, float(surf->m_dwWidth),
						                float(surf->m_dwHeight), 0.f, surf, Color::gray(.5f));
						
						GRenderer->SetRenderState(Renderer::AlphaBlending, false);
					}
				}
			}
		}
	}
}

void ARX_INTERFACE_RenderCursor(long flag)
{
	ARX_INTERFACE_RenderCursorInternal(flag);

	// Ensure filtering settings are restored in all cases
	if (!SPECIAL_DRAGINTER_RENDER)
	{
		GRenderer->GetTextureStage(0)->SetMinFilter(TextureStage::FilterLinear);
		GRenderer->GetTextureStage(0)->SetMagFilter(TextureStage::FilterLinear);
		GRenderer->GetTextureStage(0)->SetWrapMode(TextureStage::WrapRepeat);
	}
}
