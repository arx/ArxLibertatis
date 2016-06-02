/*
 * Copyright 2015 Arx Libertatis Team (see the AUTHORS file)
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

#include "gui/book/Book.h"

#include <iomanip>

#include "animation/AnimationRender.h"
#include "core/Application.h"
#include "core/Core.h"
#include "core/GameTime.h"
#include "core/Localisation.h"
#include "game/EntityManager.h"
#include "game/Equipment.h"
#include "game/Inventory.h"
#include "game/Levels.h"
#include "game/Player.h"
#include "graphics/Draw.h"
#include "graphics/Renderer.h"
#include "graphics/effects/Halo.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/texture/TextureStage.h"
#include "gui/Menu.h"
#include "gui/MiniMap.h"
#include "gui/Speech.h"
#include "gui/TextManager.h"
#include "scene/GameSound.h"
#include "scene/Interactive.h"
#include "script/Script.h"

ARX_INTERFACE_BOOK_MODE g_guiBookCurrentTopTab = BOOKMODE_STATS;

long Book_MapPage = 0;
long Book_SpellPage = 0;

long BOOKZOOM = 0;
long IN_BOOK_DRAW = 0;

long FLYING_OVER = 0;
long OLD_FLYING_OVER = 0;

//used to redist points - attributes and skill
long lCursorRedistValue = 0;

static void onBookClosePage() {
	
	if(g_guiBookCurrentTopTab == BOOKMODE_SPELLS) {
		// Closing spell page - clean up any rune flares
		ARX_SPELLS_ClearAllSymbolDraw();
	}
	
}

static bool canOpenBookPage(ARX_INTERFACE_BOOK_MODE page) {
	switch(page) {
		case BOOKMODE_SPELLS:  return !!player.rune_flags;
		default:               return true;
	}
}

void openBookPage(ARX_INTERFACE_BOOK_MODE newPage, bool toggle) {
	
	if((player.Interface & INTER_MAP) && g_guiBookCurrentTopTab == newPage) {
		
		if(toggle) {
			// Close the book
			ARX_INTERFACE_BookClose();
		}
		
		return; // nothing to do
	}
	
	if(!canOpenBookPage(newPage)) {
		return;
	}
	
	if(player.Interface & INTER_MAP) {
		
		onBookClosePage();
		
		// If the book is already open, play the page turn sound
		ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, Random::getf(0.9f, 1.1f));
		
	} else {
		// Otherwise open the book
		ARX_INTERFACE_BookToggle();
	}
	
	g_guiBookCurrentTopTab = newPage;
}

ARX_INTERFACE_BOOK_MODE nextBookPage() {
	ARX_INTERFACE_BOOK_MODE nextPage = g_guiBookCurrentTopTab, oldPage;
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
	return g_guiBookCurrentTopTab;
}

ARX_INTERFACE_BOOK_MODE prevBookPage() {
	ARX_INTERFACE_BOOK_MODE prevPage = g_guiBookCurrentTopTab, oldPage;
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
	return g_guiBookCurrentTopTab;
}

void ARX_INTERFACE_BookOpen() {
	if((player.Interface & INTER_MAP))
		return;
	
	ARX_INTERFACE_BookToggle();
}

void ARX_INTERFACE_BookClose() {
	if(!(player.Interface & INTER_MAP))
		return;
	
	ARX_INTERFACE_BookToggle();
}

void ARX_INTERFACE_BookToggle() {
	
	if(player.Interface & INTER_MAP) {
		ARX_SOUND_PlayInterface(SND_BOOK_CLOSE, Random::getf(0.9f, 1.1f));
		SendIOScriptEvent(entities.player(),SM_BOOK_CLOSE);
		player.Interface &=~ INTER_MAP;
		g_miniMap.purgeTexContainer();

		if(ARXmenu.mda) {
			for(size_t i = 0; i < MAX_FLYOVER; i++) {
				ARXmenu.mda->flyover[i].clear();
			}
			delete ARXmenu.mda;
			ARXmenu.mda=NULL;
		}
		
		onBookClosePage();
	} else {
		SendIOScriptEvent(entities.player(),SM_NULL,"","book_open");

		ARX_SOUND_PlayInterface(SND_BOOK_OPEN, Random::getf(0.9f, 1.1f));
		SendIOScriptEvent(entities.player(),SM_BOOK_OPEN);
		ARX_INTERFACE_NoteClose();
		player.Interface |= INTER_MAP;
		Book_MapPage = ARX_LEVELS_GetRealNum(CURRENTLEVEL);
		Book_MapPage = glm::clamp(Book_MapPage, 0l, 7l);
		
		if(!ARXmenu.mda) {
			ARXmenu.mda = new MENU_DYNAMIC_DATA();
		}
	}

	if(player.Interface & INTER_COMBATMODE) {
		player.Interface&=~INTER_COMBATMODE;
		ARX_EQUIPMENT_LaunchPlayerUnReadyWeapon();
	}

	if(player.Interface & INTER_INVENTORYALL) {
		ARX_SOUND_PlayInterface(SND_BACKPACK, Random::getf(0.9f, 1.1f));
		bInventoryClosing = true;
	}

	BOOKZOOM = 0;
	pTextManage->Clear();

	TRUE_PLAYER_MOUSELOOK_ON = false;
}


static Rectf scaleRectPosAndSize(const Rectf & r, const Vec2f & scale) {
	
	return Rectf(
	r.left * scale.x,
	r.top * scale.y,
	r.right * scale.x,
	r.bottom * scale.y);
}

static bool MouseInBookRect(const Vec2f pos, const Vec2f size) {
	
	Rectf rect = scaleRectPosAndSize(Rectf(pos + BOOKDEC, size.x, size.y), g_sizeRatio);
	
	return rect.contains(Vec2f(DANAEMouse));
}

bool ARX_INTERFACE_MouseInBook() {
	if((player.Interface & INTER_MAP) && !(player.Interface & INTER_COMBATMODE)) {
		return MouseInBookRect(Vec2f(99, 65), Vec2f(500, 307));
	} else {
		return false;
	}
}


static void DrawBookInterfaceItem(TextureContainer * tc, Vec2f pos, Color color, float z) {
	arx_assert(tc);
	
	Rectf rect = scaleRectPosAndSize(Rectf((pos + BOOKDEC), tc->m_size.x, tc->m_size.y), g_sizeRatio);
	EERIEDrawBitmap2(rect, z, tc, color);
}

static void RenderBookPlayerCharacter() {
	
	// TODO use assert ?
	if(!entities.player()->obj)
		return;
	
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
	
	if(ARXmenu.currentmode == AMCM_OFF)
		BOOKZOOM = 0;
	
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
	eLight2.intensity = 3.8f;
	eLight2.fallstart = 0;
	eLight2.fallend = eLight2.fallstart + 3460.f;
	RecalcLight(&eLight2);
	
	EERIE_LIGHT * SavePDL[2];
	SavePDL[0] = PDL[0];
	SavePDL[1] = PDL[1];
	size_t iSavePDL = TOTPDL;
	
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
		
		switch(player.skin) {
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
		
		pos = Vec3f(8, 162, 75);
		eLight1.pos.z = -90.f;
	} else {
		GRenderer->SetScissor(rec);
		
		ePlayerAngle.setPitch(-20.f);
		pos = Vec3f(20.f, 96.f, 260.f);
		
		ARX_EQUIPMENT_AttachPlayerWeaponToHand();
	}
	
	bool ti = player.m_improve;
	player.m_improve = false;
	
	
	float invisibility = entities.player()->invisibility;
	
	if(invisibility > 0.5f)
		invisibility = 0.5f;
	
	IN_BOOK_DRAW = 1;
	std::vector<EERIE_VERTEX> vertexlist = entities.player()->obj->vertexlist3;
	
	arx_assert(player.bookAnimation[0].cur_anim);
	
	{
		EERIE_3DOBJ * eobj = entities.player()->obj;
		unsigned long time = checked_range_cast<unsigned long>(Original_framedelay);

		EERIEDrawAnimQuatUpdate(eobj, player.bookAnimation, ePlayerAngle, pos, time, NULL, true);
		EERIEDrawAnimQuatRender(eobj, pos, NULL, invisibility);
	}
	
	IN_BOOK_DRAW = 0;
	
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
	
	PDL[0] = SavePDL[0];
	PDL[1] = SavePDL[1];
	TOTPDL = iSavePDL;
	
	entities.player()->obj->vertexlist3 = vertexlist;
	vertexlist.clear();
	
	player.m_improve = ti;
	
	GRenderer->SetScissor(Rect::ZERO);
	
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	GRenderer->SetCulling(CullNone);
	SetActiveCamera(oldcam);
	PrepareCamera(oldcam, g_size);
	
	player.bookAnimation[0].cur_anim = herowaitbook;
	
	if(ValidIONum(player.equiped[EQUIP_SLOT_WEAPON])) {
		if(entities[player.equiped[EQUIP_SLOT_WEAPON]]->type_flags & OBJECT_TYPE_2H) {
			player.bookAnimation[0].cur_anim = herowait_2h;
		}
	}
	
	GRenderer->SetCulling(CullNone);
	
	if(ValidIONum(player.equiped[EQUIP_SLOT_ARMOR])) {
		Entity *tod = entities[player.equiped[EQUIP_SLOT_ARMOR]];
		if(tod) {
			tod->bbox2D.min = Vec2f(195.f, 116.f);
			tod->bbox2D.max = Vec2f(284.f, 182.f);
			
			tod->bbox2D.min = (tod->bbox2D.min + BOOKDEC) * g_sizeRatio;
			tod->bbox2D.max = (tod->bbox2D.max + BOOKDEC) * g_sizeRatio;
			
			tod->ioflags |= IO_ICONIC;
		}
	}
	
	if(ValidIONum(player.equiped[EQUIP_SLOT_LEGGINGS])) {
		Entity *tod = entities[player.equiped[EQUIP_SLOT_LEGGINGS]];
		if(tod) {
			tod->bbox2D.min = Vec2f(218.f, 183.f);
			tod->bbox2D.max = Vec2f(277.f, 322.f);
			
			tod->bbox2D.min = (tod->bbox2D.min + BOOKDEC) * g_sizeRatio;
			tod->bbox2D.max = (tod->bbox2D.max + BOOKDEC) * g_sizeRatio;
			
			tod->ioflags |= IO_ICONIC;
		}
	}
	
	if(ValidIONum(player.equiped[EQUIP_SLOT_HELMET])) {
		Entity *tod = entities[player.equiped[EQUIP_SLOT_HELMET]];
		if(tod) {
			tod->bbox2D.min = Vec2f(218.f, 75.f);
			tod->bbox2D.max = Vec2f(260.f, 115.f);
			
			tod->bbox2D.min = (tod->bbox2D.min + BOOKDEC) * g_sizeRatio;
			tod->bbox2D.max = (tod->bbox2D.max + BOOKDEC) * g_sizeRatio;
			
			tod->ioflags |= IO_ICONIC;
		}
	}
	
	GRenderer->SetCulling(CullNone);
	
	if(ValidIONum(player.equiped[EQUIP_SLOT_RING_LEFT])) {
		Entity *todraw = entities[player.equiped[EQUIP_SLOT_RING_LEFT]];
		
		TextureContainer * tc = todraw->m_icon;
		TextureContainer * tc2 = NULL;
		
		if(NeedHalo(todraw))
			tc2 = todraw->m_icon->getHalo();
		
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
	
	if(ValidIONum(player.equiped[EQUIP_SLOT_RING_RIGHT])) {
		Entity *todraw = entities[player.equiped[EQUIP_SLOT_RING_RIGHT]];
		
		TextureContainer * tc = todraw->m_icon;
		TextureContainer * tc2 = NULL;
		
		if(NeedHalo(todraw))
			tc2 = todraw->m_icon->getHalo();
		
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

static void ARX_INTERFACE_RELEASESOUND() {
	ARX_SOUND_PlayInterface(SND_MENU_RELEASE);
}

static void ARX_INTERFACE_ERRORSOUND() {
	ARX_SOUND_PlayInterface(SND_MENU_CLICK);
}

static bool CheckAttributeClick(Vec2f pos, float * val, TextureContainer * tc) {
	
	bool rval=false;
	float t = *val;

	if(MouseInBookRect(pos, Vec2f(32, 32))) {
		rval = true;

		if((eeMousePressed1() || eeMousePressed2()) && tc)
			DrawBookInterfaceItem(tc, pos, Color::white, 0.000001f);

		if(eeMouseUp1()) {
			if(player.Attribute_Redistribute > 0) {
				player.Attribute_Redistribute--;
				t++;
				*val=t;
				ARX_INTERFACE_RELEASESOUND();
			}
			else
				ARX_INTERFACE_ERRORSOUND();
		}

		if(eeMouseUp2()) {
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

static bool CheckSkillClick(Vec2f pos, float * val, TextureContainer * tc,
                            float * oldval) {
	
	bool rval=false;

	float t = *val;
	float ot = *oldval;

	if(MouseInBookRect(pos, Vec2f(32, 32))) {
		rval=true;

		if((eeMousePressed1() || eeMousePressed2()) && tc)
			DrawBookInterfaceItem(tc, pos, Color::white, 0.000001f);

		if(eeMouseUp1()) {
			if(player.Skill_Redistribute > 0) {
				player.Skill_Redistribute--;
				t++;
				*val=t;
				ARX_INTERFACE_RELEASESOUND();
			}
			else
				ARX_INTERFACE_ERRORSOUND();
		}

		if(eeMouseUp2()) {
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


namespace gui {

/*!
 * Manage forward and backward buttons on notes and the quest book.
 * \return true if the note was clicked
 */
bool manageNoteActions(Note & note) {
	
	if(note.prevPageButton().contains(Vec2f(DANAEMouse))) {
		SpecialCursor = CURSOR_INTERACTION_ON;
		if(eeMouseUp1()) {
			ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, Random::getf(0.9f, 1.1f));
			arx_assert(note.page() >= 2);
			note.setPage(note.page() - 2);
		}
		
	} else if(note.nextPageButton().contains(Vec2f(DANAEMouse))) {
		SpecialCursor = CURSOR_INTERACTION_ON;
		if(eeMouseUp1()) {
			ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, Random::getf(0.9f, 1.1f));
			note.setPage(note.page() + 2);
		}
		
	} else if(note.area().contains(Vec2f(DANAEMouse))) {
		if((eeMouseDown1() && TRUE_PLAYER_MOUSELOOK_ON) || eeMouseDown2()) {
			return true;
		}
	}
	
	return false;
}

static gui::Note questBook;

//! Update and render the quest book.
static void manageQuestBook() {
	
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

void updateQuestBook() {
	// Clear the quest book cache - it will be re-created when needed
	questBook.clear();
}

} // namespace gui

//-----------------------------------------------------------------------------


static void ARX_INTERFACE_ManageOpenedBook_TopTabs() {
	
	static const Vec2f BOOKMARKS_POS = Vec2f(216.f, 60.f);
	
	if(g_guiBookCurrentTopTab != BOOKMODE_STATS) {
		Vec2f pos = BOOKMARKS_POS;
		
		TextureContainer* tcBookmarkChar = g_bookResouces.bookmark_char;
		DrawBookInterfaceItem(tcBookmarkChar, pos, Color::white, 0.000001f);
		
		// Check for cursor on charcter sheet bookmark
		if(MouseInBookRect(pos, Vec2f(tcBookmarkChar->m_size.x, tcBookmarkChar->m_size.y))) {
			// Draw highlighted Character sheet icon
			GRenderer->SetBlendFunc(BlendOne, BlendOne);
			GRenderer->SetRenderState(Renderer::AlphaBlending, true);
			DrawBookInterfaceItem(tcBookmarkChar, pos, Color::grayb(0x55), 0.000001f);
			GRenderer->SetRenderState(Renderer::AlphaBlending, false);
			
			// Set cursor to interacting
			SpecialCursor=CURSOR_INTERACTION_ON;
			
			// Check for click
			if(eeMouseDown1() || eeMouseDown2()) {
				ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, Random::getf(0.9f, 1.1f));
				openBookPage(BOOKMODE_STATS);
				pTextManage->Clear();
			}
		}
	}
	
	if(g_guiBookCurrentTopTab != BOOKMODE_SPELLS) {
		if(player.rune_flags) {
			Vec2f pos = BOOKMARKS_POS + Vec2f(32, 0);
			
			DrawBookInterfaceItem(g_bookResouces.bookmark_magic, pos, Color::white, 0.000001f);

			if(NewSpell == 1) {
				NewSpell = 2;
				for(long nk = 0; nk < 2; nk++) {
					// TODO this effect is barely visible
					MagFX(Vec3f(pos * g_sizeRatio, 0.000001f), 1.f);
				}
			}
			
			if(MouseInBookRect(pos, Vec2f(g_bookResouces.bookmark_magic->m_size.x, g_bookResouces.bookmark_magic->m_size.y))) {
				// Draw highlighted Magic sheet icon
				GRenderer->SetBlendFunc(BlendOne, BlendOne);
				GRenderer->SetRenderState(Renderer::AlphaBlending, true);
				DrawBookInterfaceItem(g_bookResouces.bookmark_magic, pos, Color::grayb(0x55), 0.000001f);
				GRenderer->SetRenderState(Renderer::AlphaBlending, false);
				
				// Set cursor to interacting
				SpecialCursor=CURSOR_INTERACTION_ON;
				
				// Check for click
				if(eeMouseDown1() || eeMouseDown2()) {
					ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, Random::getf(0.9f, 1.1f));
					openBookPage(BOOKMODE_SPELLS);
					pTextManage->Clear();
				}
			}
		}
	}
	
	if(g_guiBookCurrentTopTab != BOOKMODE_MINIMAP) {
		Vec2f pos = BOOKMARKS_POS + Vec2f(64, 0);
		
		DrawBookInterfaceItem(g_bookResouces.bookmark_map, pos, Color::white, 0.000001f);
		
		if(MouseInBookRect(pos, Vec2f(g_bookResouces.bookmark_map->m_size.x, g_bookResouces.bookmark_map->m_size.y))) {
			GRenderer->SetBlendFunc(BlendOne, BlendOne);
			GRenderer->SetRenderState(Renderer::AlphaBlending, true);
			DrawBookInterfaceItem(g_bookResouces.bookmark_map, pos, Color::grayb(0x55), 0.000001f);
			GRenderer->SetRenderState(Renderer::AlphaBlending, false);
			
			// Set cursor to interacting
			SpecialCursor=CURSOR_INTERACTION_ON;
			
			// Check for click
			if(eeMouseDown1() || eeMouseDown2()) {
				ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, Random::getf(0.9f, 1.1f));
				openBookPage(BOOKMODE_MINIMAP);
				pTextManage->Clear();
			}
		}
	}
	
	if(g_guiBookCurrentTopTab != BOOKMODE_QUESTS) {
		Vec2f pos = BOOKMARKS_POS + Vec2f(96, 0);
		
		DrawBookInterfaceItem(g_bookResouces.bookmark_quest, pos, Color::white, 0.000001f);
		
		if(MouseInBookRect(pos, Vec2f(g_bookResouces.bookmark_quest->m_size.x, g_bookResouces.bookmark_quest->m_size.y))) {
			GRenderer->SetBlendFunc(BlendOne, BlendOne);
			GRenderer->SetRenderState(Renderer::AlphaBlending, true);
			DrawBookInterfaceItem(g_bookResouces.bookmark_quest, pos, Color::grayb(0x55), 0.000001f);
			GRenderer->SetRenderState(Renderer::AlphaBlending, false);
			
			// Set cursor to interacting
			SpecialCursor=CURSOR_INTERACTION_ON;
			
			// Check for click
			if(eeMouseDown1() || eeMouseDown2()) {
				ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, Random::getf(0.9f, 1.1f));
				openBookPage(BOOKMODE_QUESTS);
				pTextManage->Clear();
			}
		}
	}
}

static void ARX_INTERFACE_ManageOpenedBook_LeftTabs_OneTab(bool tabVisibility[10], long & activeTab, int t, Vec2f pos, Vec2f activePos) {
	
	if(tabVisibility[t]) {
		if(activeTab != t) {
			
			DrawBookInterfaceItem(g_bookResouces.accessibleTab[t], pos, Color::white, 0.000001f);

			if(MouseInBookRect(pos, Vec2f(32, 32))) {
				GRenderer->SetBlendFunc(BlendOne, BlendOne);
				GRenderer->SetRenderState(Renderer::AlphaBlending, true);
				DrawBookInterfaceItem(g_bookResouces.accessibleTab[t], pos, Color::grayb(0x55), 0.000001f);
				GRenderer->SetRenderState(Renderer::AlphaBlending, false);
				SpecialCursor=CURSOR_INTERACTION_ON;
				if(eeMouseDown1() || eeMouseDown2()) {
					activeTab = t;
					ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, Random::getf(0.9f, 1.1f));
				}
			}
		}
		else DrawBookInterfaceItem(g_bookResouces.currentTab[t], activePos, Color::white, 0.000001f);
	}
}

static void ARX_INTERFACE_ManageOpenedBook_LeftTabs(bool tabVisibility[10], long & activeTab) {
	
	{
	int t = 0;
	Vec2f pos = Vec2f(100.f, 82.f);
	Vec2f activePos = Vec2f(102.f, 82.f);
	
	ARX_INTERFACE_ManageOpenedBook_LeftTabs_OneTab(tabVisibility, activeTab, t, pos, activePos);
	}
	
	{
	int t = 1;
	Vec2f pos = Vec2f(98.f, 112.f);
	Vec2f activePos = Vec2f(100.f, 114.f);
	
	ARX_INTERFACE_ManageOpenedBook_LeftTabs_OneTab(tabVisibility, activeTab, t, pos, activePos);
	}
	
	{
	int t = 2;
	Vec2f pos = Vec2f(97.f, 143.f);
	Vec2f activePos = Vec2f(101.f, 141.f);
	
	ARX_INTERFACE_ManageOpenedBook_LeftTabs_OneTab(tabVisibility, activeTab, t, pos, activePos);
	}

	{
	int t = 3;
	Vec2f pos = Vec2f(95.f, 170.f);
	Vec2f activePos = Vec2f(100.f, 170.f);
	
	ARX_INTERFACE_ManageOpenedBook_LeftTabs_OneTab(tabVisibility, activeTab, t, pos, activePos);
	}
	
	{
	int t = 4;
	Vec2f pos = Vec2f(95.f, 200.f);
	Vec2f activePos = Vec2f(97.f, 199.f);
	
	ARX_INTERFACE_ManageOpenedBook_LeftTabs_OneTab(tabVisibility, activeTab, t, pos, activePos);
	}
	
	{
	int t = 5;
	Vec2f pos = Vec2f(94.f, 229.f);
	Vec2f activePos = Vec2f(103.f, 226.f);
	
	ARX_INTERFACE_ManageOpenedBook_LeftTabs_OneTab(tabVisibility, activeTab, t, pos, activePos);
	}
	
	{
	int t = 6;
	Vec2f pos = Vec2f(94.f, 259.f);
	Vec2f activePos = Vec2f(101.f, 255.f);
	
	ARX_INTERFACE_ManageOpenedBook_LeftTabs_OneTab(tabVisibility, activeTab, t, pos, activePos);
	}
	
	{
	int t = 7;
	Vec2f pos = Vec2f(92.f, 282.f);
	Vec2f activePos = Vec2f(99.f, 283.f);
	
	ARX_INTERFACE_ManageOpenedBook_LeftTabs_OneTab(tabVisibility, activeTab, t, pos, activePos);
	}
	
	{
	int t = 8;
	Vec2f pos = Vec2f(90.f, 308.f);
	Vec2f activePos = Vec2f(99.f, 307.f);
	
	ARX_INTERFACE_ManageOpenedBook_LeftTabs_OneTab(tabVisibility, activeTab, t, pos, activePos);
	}
	
	{
	int t = 9;
	Vec2f pos = Vec2f(97.f, 331.f);
	Vec2f activePos = Vec2f(104.f, 331.f);
	
	ARX_INTERFACE_ManageOpenedBook_LeftTabs_OneTab(tabVisibility, activeTab, t, pos, activePos);
	}
}

static void ARX_INTERFACE_ManageOpenedBook_LeftTabs_Spells() {
	
	bool tabVisibility[10] = {false};

	for(size_t i = 0; i < SPELL_TYPES_COUNT; ++i) {
		if(spellicons[i].bSecret == false) {
			bool bOk = true;

			for(long j = 0; j < 4 && spellicons[i].symbols[j] != RUNE_NONE; ++j) {
				if(!player.hasRune(spellicons[i].symbols[j]))
					bOk = false;
			}

			if(bOk)
				tabVisibility[spellicons[i].level - 1] = true;
		}
	}
	
	ARX_INTERFACE_ManageOpenedBook_LeftTabs(tabVisibility, Book_SpellPage);
}

static void ARX_INTERFACE_ManageOpenedBook_LeftTabs_Map() {
	
	bool tabVisibility[10] = {false};
	
	long max_onglet = 7;
	memset(tabVisibility, true, (max_onglet + 1) * sizeof(*tabVisibility));
	
	ARX_INTERFACE_ManageOpenedBook_LeftTabs(tabVisibility, Book_MapPage);
}

static Color attrubuteModToColor(float modValue, float baseValue = 0.f) {
	if(modValue < baseValue)
		return Color::red;
	else if(modValue > baseValue)
		return Color::blue;
	else
		return Color::black;
}

static void DrawBookTextCenter(Font* font, const Vec2f & pos, const std::string& text, Color col) {
	
	UNICODE_ARXDrawTextCenter(font, (BOOKDEC + pos) * g_sizeRatio, text, col);
}

static void ARX_INTERFACE_ManageOpenedBook_Stats()
{
	FLYING_OVER = 0;
	
	ARX_PLAYER_ComputePlayerFullStats();
	
	{
		std::stringstream ss;
		ss << g_bookResouces.Level << " " << std::setw(3) << player.level;
		DrawBookTextCenter(hFontInBook, Vec2f(398, 74), ss.str(), Color::black);
	}
	
	{
		std::stringstream ss;
		ss << g_bookResouces.Xp << " " << std::setw(8) << player.xp;
		DrawBookTextCenter(hFontInBook, Vec2f(510, 74), ss.str(), Color::black);
	}
	
	if (MouseInBookRect(Vec2f(463, 74), Vec2f(87, 20)))
		FLYING_OVER = WND_XP;

	if (MouseInBookRect(Vec2f(97+41,64+62), Vec2f(32, 32)))
		FLYING_OVER = WND_AC;
	else if (MouseInBookRect(Vec2f(97+41,64+120), Vec2f(32, 32)))
		FLYING_OVER = WND_RESIST_MAGIC;
	else if (MouseInBookRect(Vec2f(97+41,64+178), Vec2f(32, 32)))
		FLYING_OVER = WND_RESIST_POISON;
	else if (MouseInBookRect(Vec2f(97+211,64+62), Vec2f(32, 32)))
		FLYING_OVER = WND_HP;
	else if (MouseInBookRect(Vec2f(97+211,64+120), Vec2f(32, 32)))
		FLYING_OVER = WND_MANA;
	else if (MouseInBookRect(Vec2f(97+211,64+178), Vec2f(32, 32)))
		FLYING_OVER = WND_DAMAGE;

	if(!((player.Attribute_Redistribute == 0) && (ARXmenu.currentmode != AMCM_NEWQUEST))) {
		// Main Player Attributes
		if(CheckAttributeClick(Vec2f(379, 95), &player.m_attribute.strength, g_bookResouces.ic_strength)) {
			FLYING_OVER = BOOK_STRENGTH;
			SpecialCursor = CURSOR_REDIST;
			lCursorRedistValue = player.Attribute_Redistribute;
		}

		if(CheckAttributeClick(Vec2f(428, 95), &player.m_attribute.mind, g_bookResouces.ic_mind)) {
			FLYING_OVER = BOOK_MIND;
			SpecialCursor = CURSOR_REDIST;
			lCursorRedistValue = player.Attribute_Redistribute;
		}

		if(CheckAttributeClick(Vec2f(477, 95), &player.m_attribute.dexterity, g_bookResouces.ic_dexterity)) {
			FLYING_OVER = BOOK_DEXTERITY;
			SpecialCursor = CURSOR_REDIST;
			lCursorRedistValue = player.Attribute_Redistribute;
		}

		if(CheckAttributeClick(Vec2f(526, 95), &player.m_attribute.constitution, g_bookResouces.ic_constitution)) {
			FLYING_OVER = BOOK_CONSTITUTION;
			SpecialCursor = CURSOR_REDIST;
			lCursorRedistValue = player.Attribute_Redistribute;
		}
	}

	if(!((player.Skill_Redistribute == 0) && (ARXmenu.currentmode != AMCM_NEWQUEST))) {
		if (CheckSkillClick(Vec2f(389, 177), &player.m_skill.stealth, g_bookResouces.ic_stealth, &player.m_skillOld.stealth)) {
			FLYING_OVER = BOOK_STEALTH;
			SpecialCursor = CURSOR_REDIST;
			lCursorRedistValue = player.Skill_Redistribute;
		}

		if(CheckSkillClick(Vec2f(453, 177), &player.m_skill.mecanism, g_bookResouces.ic_mecanism, &player.m_skillOld.mecanism)) {
			FLYING_OVER = BOOK_MECANISM;
			SpecialCursor = CURSOR_REDIST;
			lCursorRedistValue = player.Skill_Redistribute;
		}

		if(CheckSkillClick(Vec2f(516, 177), &player.m_skill.intuition, g_bookResouces.ic_intuition, &player.m_skillOld.intuition)) {
			FLYING_OVER = BOOK_INTUITION;
			SpecialCursor = CURSOR_REDIST;
			lCursorRedistValue = player.Skill_Redistribute;
		}

		if(CheckSkillClick(Vec2f(389, 230), &player.m_skill.etheralLink, g_bookResouces.ic_etheral_link, &player.m_skillOld.etheralLink)) {
			FLYING_OVER = BOOK_ETHERAL_LINK;
			SpecialCursor = CURSOR_REDIST;
			lCursorRedistValue = player.Skill_Redistribute;
		}

		if(CheckSkillClick(Vec2f(453, 230), &player.m_skill.objectKnowledge, g_bookResouces.ic_object_knowledge, &player.m_skillOld.objectKnowledge)) {
			FLYING_OVER = BOOK_OBJECT_KNOWLEDGE;
			SpecialCursor = CURSOR_REDIST;
			lCursorRedistValue = player.Skill_Redistribute;

			if(eeMouseDown1()) {
				ARX_INVENTORY_IdentifyAll();
				ARX_EQUIPMENT_IdentifyAll();
			}

			ARX_PLAYER_ComputePlayerFullStats();
		}

		if(CheckSkillClick(Vec2f(516, 230), &player.m_skill.casting, g_bookResouces.ic_casting, &player.m_skillOld.casting)) {
			FLYING_OVER = BOOK_CASTING;
			SpecialCursor = CURSOR_REDIST;
			lCursorRedistValue = player.Skill_Redistribute;
		}

		if(CheckSkillClick(Vec2f(389, 284), &player.m_skill.closeCombat, g_bookResouces.ic_close_combat, &player.m_skillOld.closeCombat)) {
			FLYING_OVER = BOOK_CLOSE_COMBAT;
			SpecialCursor = CURSOR_REDIST;
			lCursorRedistValue = player.Skill_Redistribute;
		}

		if(CheckSkillClick(Vec2f(453, 284), &player.m_skill.projectile, g_bookResouces.ic_projectile, &player.m_skillOld.projectile)) {
			FLYING_OVER = BOOK_PROJECTILE;
			SpecialCursor = CURSOR_REDIST;
			lCursorRedistValue = player.Skill_Redistribute;
		}

		if(CheckSkillClick(Vec2f(516, 284), &player.m_skill.defense, g_bookResouces.ic_defense, &player.m_skillOld.defense)) {
			FLYING_OVER = BOOK_DEFENSE;
			SpecialCursor = CURSOR_REDIST;
			lCursorRedistValue = player.Skill_Redistribute;
		}
	} else {
		//------------------------------------PRIMARY
		if (MouseInBookRect(Vec2f(379,95), Vec2f(32, 32)))
			FLYING_OVER=BOOK_STRENGTH;
		else if (MouseInBookRect(Vec2f(428,95), Vec2f(32, 32)))
			FLYING_OVER=BOOK_MIND;
		else if (MouseInBookRect(Vec2f(477,95), Vec2f(32, 32)))
			FLYING_OVER=BOOK_DEXTERITY;
		else if (MouseInBookRect(Vec2f(526,95), Vec2f(32, 32)))
			FLYING_OVER=BOOK_CONSTITUTION;

		//------------------------------------SECONDARY
		if (MouseInBookRect(Vec2f(389,177), Vec2f(32, 32)))
			FLYING_OVER=BOOK_STEALTH;
		else if (MouseInBookRect(Vec2f(453,177), Vec2f(32, 32)))
			FLYING_OVER=BOOK_MECANISM;
		else if (MouseInBookRect(Vec2f(516,177), Vec2f(32, 32)))
			FLYING_OVER=BOOK_INTUITION;
		else if (MouseInBookRect(Vec2f(389,230), Vec2f(32, 32)))
			FLYING_OVER=BOOK_ETHERAL_LINK;
		else if (MouseInBookRect(Vec2f(453,230), Vec2f(32, 32)))
			FLYING_OVER=BOOK_OBJECT_KNOWLEDGE;
		else if (MouseInBookRect(Vec2f(516,230), Vec2f(32, 32)))
			FLYING_OVER=BOOK_CASTING;
		else if (MouseInBookRect(Vec2f(389,284), Vec2f(32, 32)))
			FLYING_OVER=BOOK_CLOSE_COMBAT;
		else if (MouseInBookRect(Vec2f(453,284), Vec2f(32, 32)))
			FLYING_OVER=BOOK_PROJECTILE;
		else if (MouseInBookRect(Vec2f(516,284), Vec2f(32, 32)))
			FLYING_OVER=BOOK_DEFENSE;
	}

	//------------------------------ SEB 04/12/2001
	if(ARXmenu.mda && !ARXmenu.mda->flyover[FLYING_OVER].empty()) {
		
		int t = Random::get(0, 2);

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
	
	{
	Vec2f pos = Vec2f(391, 129);
	
	std::stringstream ss3;
	ss3 << std::setw(3) << std::setprecision(0) << std::fixed << player.m_attributeFull.strength;
	
	Color color = attrubuteModToColor(player.m_attributeMod.strength);
	
	if(ARXmenu.currentmode == AMCM_NEWQUEST) {
		if(player.m_attributeFull.strength == 6)
			color = Color::red;
	}
	
	DrawBookTextCenter(hFontInBook, pos, ss3.str(), color);
	}
	
	{
	Vec2f pos = Vec2f(440, 129);
	
	std::stringstream ss3;
	ss3 << std::setw(3) << std::setprecision(0) << std::fixed << player.m_attributeFull.mind;
	
	Color color = attrubuteModToColor(player.m_attributeMod.mind);
	
	if(ARXmenu.currentmode == AMCM_NEWQUEST) {
		if(player.m_attributeFull.mind == 6)
			color = Color::red;
	}
	
	DrawBookTextCenter(hFontInBook, pos, ss3.str(), color);
	}
	
	{
	Vec2f pos = Vec2f(490, 129);
	
	std::stringstream ss3;
	ss3 << std::setw(3) << std::setprecision(0) << std::fixed << player.m_attributeFull.dexterity;
	
	Color color = attrubuteModToColor(player.m_attributeMod.dexterity);
	
	if(ARXmenu.currentmode == AMCM_NEWQUEST) {
		if(player.m_attributeFull.dexterity == 6)
			color = Color::red;
	}
	
	DrawBookTextCenter(hFontInBook, pos, ss3.str(), color);
	}
	
	{
	Vec2f pos = Vec2f(538, 129);
	
	std::stringstream ss3;
	ss3 << std::setw(3) << std::setprecision(0) << std::fixed << player.m_attributeFull.constitution;
	
	Color color = attrubuteModToColor(player.m_attributeMod.constitution);
	
	if(ARXmenu.currentmode == AMCM_NEWQUEST) {
		if(player.m_attributeFull.constitution == 6)
			color = Color::red;
	}
	
	DrawBookTextCenter(hFontInBook, pos, ss3.str(), color);
	}
	
	// Player Skills
	{
	Vec2f pos = Vec2f(405, 210);
	
	std::stringstream ss3;
	ss3 << std::setw(3) << std::setprecision(0) << std::fixed << player.m_skillFull.stealth;
	
	Color color = attrubuteModToColor(player.m_skillMod.stealth);
	
	if(ARXmenu.currentmode == AMCM_NEWQUEST) {
		if(player.m_skill.stealth == 0)
			color = Color::red;
	}
	
	DrawBookTextCenter(hFontInBook, pos, ss3.str(), color);
	}
	
	{
	Vec2f pos = Vec2f(469, 210);
	
	std::stringstream ss3;
	ss3 << std::setw(3) << std::setprecision(0) << std::fixed << player.m_skillFull.mecanism;
	
	Color color = attrubuteModToColor(player.m_skillMod.mecanism);
	
	if(ARXmenu.currentmode == AMCM_NEWQUEST) {
		if(player.m_skill.mecanism == 0)
			color = Color::red;
	}
	
	DrawBookTextCenter(hFontInBook, pos, ss3.str(), color);
	}
	
	{
	Vec2f pos = Vec2f(533, 210);
	
	std::stringstream ss3;
	ss3 << std::setw(3) << std::setprecision(0) << std::fixed << player.m_skillFull.intuition;
	
	Color color = attrubuteModToColor(player.m_skillMod.intuition);
	
	if(ARXmenu.currentmode == AMCM_NEWQUEST) {
		if(player.m_skill.intuition == 0)
			color = Color::red;
	}

	DrawBookTextCenter(hFontInBook, pos, ss3.str(), color);
	}
	
	{
	Vec2f pos = Vec2f(405, 265);
	
	std::stringstream ss3;
	ss3 << std::setw(3) << std::setprecision(0) << std::fixed << player.m_skillFull.etheralLink;
	
	Color color = attrubuteModToColor(player.m_skillMod.etheralLink);
	
	if(ARXmenu.currentmode == AMCM_NEWQUEST) {
		if(player.m_skill.etheralLink == 0)
			color = Color::red;
	}

	DrawBookTextCenter(hFontInBook, pos, ss3.str(), color);
	}
	
	{
	Vec2f pos = Vec2f(469, 265);
	
	std::stringstream ss3;
	ss3 << std::setw(3) << std::setprecision(0) << std::fixed << player.m_skillFull.objectKnowledge;
	
	Color color = attrubuteModToColor(player.m_skillMod.objectKnowledge);
	
	if(ARXmenu.currentmode == AMCM_NEWQUEST) {
		if(player.m_skill.objectKnowledge == 0)
			color = Color::red;
	}
	
	DrawBookTextCenter(hFontInBook, pos, ss3.str(), color);
	}
	
	{
	Vec2f pos = Vec2f(533, 265);
	
	std::stringstream ss3;
	ss3 << std::setw(3) << std::setprecision(0) << std::fixed << player.m_skillFull.casting;
	
	Color color = attrubuteModToColor(player.m_skillMod.casting);
	
	if(ARXmenu.currentmode == AMCM_NEWQUEST) {
		if(player.m_skill.casting == 0)
			color = Color::red;
	}

	DrawBookTextCenter(hFontInBook, pos, ss3.str(), color);
	}
	
	{
	Vec2f pos = Vec2f(405, 319);
	
	std::stringstream ss3;
	ss3 << std::setw(3) << std::setprecision(0) << std::fixed << player.m_skillFull.closeCombat;
	
	Color color = attrubuteModToColor(player.m_skillMod.closeCombat);
	
	if(ARXmenu.currentmode == AMCM_NEWQUEST) {
		if(player.m_skill.closeCombat == 0)
			color = Color::red;
	}

	DrawBookTextCenter(hFontInBook, pos, ss3.str(), color);
	}

	{
	Vec2f pos = Vec2f(469, 319);
	
	std::stringstream ss3;
	ss3 << std::setw(3) << std::setprecision(0) << std::fixed << player.m_skillFull.projectile;
	
	Color color = attrubuteModToColor(player.m_skillMod.projectile);
	
	if(ARXmenu.currentmode == AMCM_NEWQUEST) {
		if(player.m_skill.projectile == 0)
			color = Color::red;
	}

	DrawBookTextCenter(hFontInBook, pos, ss3.str(), color);
	}
	
	{
	Vec2f pos = Vec2f(533, 319);
	
	std::stringstream ss3;
	ss3 << std::setw(3) << std::setprecision(0) << std::fixed << player.m_skillFull.defense;
	
	Color color = attrubuteModToColor(player.m_skillMod.defense);
	
	if(ARXmenu.currentmode == AMCM_NEWQUEST) {
		if(player.m_skill.defense == 0)
			color = Color::red;
	}
	
	DrawBookTextCenter(hFontInBook, pos, ss3.str(), color);
	}
	
	// Secondary Attributes
	{
	Vec2f pos = Vec2f(324, 158);
	
	std::stringstream ss4;
	ss4 << F2L_RoundUp(player.Full_maxlife);
	
	Color color = attrubuteModToColor(player.Full_maxlife, player.lifePool.max);
	
	DrawBookTextCenter(hFontInBook, pos, ss4.str(), color);
	}
	
	{
	Vec2f pos = Vec2f(324, 218);
	
	std::stringstream ss4;
	ss4 << F2L_RoundUp(player.Full_maxmana);
	
	Color color = attrubuteModToColor(player.Full_maxmana, player.manaPool.max);
	
	DrawBookTextCenter(hFontInBook, pos, ss4.str(), color);
	}
	
	{
	Vec2f pos = Vec2f(324, 278);
	
	std::stringstream ss4;
	ss4 << F2L_RoundUp(player.m_miscFull.damages);
	
	Color color = attrubuteModToColor(player.m_miscMod.damages);
	
	DrawBookTextCenter(hFontInBook, pos, ss4.str(), color);
	}
	
	{
	Vec2f pos = Vec2f(153, 158);
	
	std::stringstream ss4;
	ss4 << F2L_RoundUp(player.m_miscFull.armorClass);
	
	Color color = attrubuteModToColor(player.m_miscMod.armorClass);
	
	DrawBookTextCenter(hFontInBook, pos, ss4.str(), color);
	}
	
	{
	Vec2f pos = Vec2f(153, 218);
	
	std::stringstream ss4;
	ss4 << std::setw(3) << std::setprecision(0) << F2L_RoundUp( player.m_miscFull.resistMagic );
	
	Color color = attrubuteModToColor(player.m_miscMod.resistMagic);
	
	DrawBookTextCenter(hFontInBook, pos, ss4.str(), color);
	}
	
	{
	Vec2f pos = Vec2f(153, 278);
	
	std::stringstream ss4;
	ss4 << std::setw(3) << std::setprecision(0) << F2L_RoundUp( player.m_miscFull.resistPoison );
	
	Color color = attrubuteModToColor(player.m_miscMod.resistPoison);
	
	DrawBookTextCenter(hFontInBook, pos, ss4.str(), color);
	}
	
	RenderBookPlayerCharacter();	
}

static void ARX_INTERFACE_ManageOpenedBook_Map()
{
	long SHOWLEVEL = Book_MapPage;

	if(SHOWLEVEL >= 0 && SHOWLEVEL < 32)
		g_miniMap.showBookEntireMap(SHOWLEVEL);

	SHOWLEVEL = ARX_LEVELS_GetRealNum(CURRENTLEVEL);

	if(SHOWLEVEL >= 0 && SHOWLEVEL < 32)
		g_miniMap.showBookMiniMap(SHOWLEVEL);
}

void ARX_INTERFACE_ManageOpenedBook() {
	arx_assert(entities.player());
	
	GRenderer->SetRenderState(Renderer::Fog, false);
	
	BOOKDEC.x = 0;
	BOOKDEC.y = 0;
	
	GRenderer->GetTextureStage(0)->setMinFilter(TextureStage::FilterLinear);
	GRenderer->GetTextureStage(0)->setMagFilter(TextureStage::FilterLinear);
	
	if(ARXmenu.currentmode != AMCM_NEWQUEST) {
		switch(g_guiBookCurrentTopTab) {
			case BOOKMODE_STATS: {
				DrawBookInterfaceItem(g_bookResouces.playerbook, Vec2f(97, 64), Color::white, 0.9999f);
				break;
			}
			case BOOKMODE_SPELLS: {
				DrawBookInterfaceItem(g_bookResouces.ptexspellbook, Vec2f(97, 64), Color::white, 0.9999f);
				ARX_INTERFACE_ManageOpenedBook_LeftTabs_Spells();
				break;
			}
			case BOOKMODE_MINIMAP: {
				DrawBookInterfaceItem(g_bookResouces.questbook, Vec2f(97, 64), Color::white, 0.9999f);
				ARX_INTERFACE_ManageOpenedBook_LeftTabs_Map();
				break;
			}
			case BOOKMODE_QUESTS: {
				gui::manageQuestBook();
				break;
			}
		}
		
		ARX_INTERFACE_ManageOpenedBook_TopTabs();
	} else {
		arx_assert(g_bookResouces.playerbook);
		float x = (640 - g_bookResouces.playerbook->m_size.x) / 2.f;
		float y = (480 - g_bookResouces.playerbook->m_size.y) / 2.f;
		
		DrawBookInterfaceItem(g_bookResouces.playerbook, Vec2f(x, y), Color::white, 0.000001f);

		BOOKDEC.x = x - 97;
		// TODO copy paste error ?
		BOOKDEC.y = x - 64 + 19;
	}
	
	GRenderer->GetTextureStage(0)->setMinFilter(TextureStage::FilterNearest);
	GRenderer->GetTextureStage(0)->setMagFilter(TextureStage::FilterNearest);

	if(g_guiBookCurrentTopTab == BOOKMODE_STATS) {
		ARX_INTERFACE_ManageOpenedBook_Stats();
	} else if (g_guiBookCurrentTopTab == BOOKMODE_MINIMAP) {
		ARX_INTERFACE_ManageOpenedBook_Map();
	}
}





void ARX_INTERFACE_ManageOpenedBook_SpellsDraw() {
	
	// Now Draws Spells for this level...
	ARX_PLAYER_ComputePlayerFullStats();
	
	Vec2f tmpPos = Vec2f_ZERO;
	bool	bFlyingOver = false;
	
	for(size_t i=0; i < SPELL_TYPES_COUNT; i++) {
		const SPELL_ICON & spellInfo = spellicons[i];
		
		if(spellInfo.level != (Book_SpellPage + 1) || spellInfo.bSecret)
			continue;
		
		// check if player can cast it
		bool bOk = true;
		long j = 0;
		
		while(j < 4 && (spellInfo.symbols[j] != RUNE_NONE)) {
			if(!player.hasRune(spellInfo.symbols[j])) {
				bOk = false;
			}
			
			j++;
		}
		
		if(!bOk)
			continue;
			
		Vec2f fPos = Vec2f(170.f, 135.f) + tmpPos * Vec2f(85.f, 70.f);
		long flyingover = 0;
		
		if(MouseInBookRect(fPos, Vec2f(48, 48))) {
			bFlyingOver = true;
			flyingover = 1;
			
			SpecialCursor=CURSOR_INTERACTION_ON;
			DrawBookTextCenter(hFontInBook, Vec2f(208, 90), spellInfo.name, Color::none);
			
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
				spellInfo.description,
				Color(232,204,143),
				1000,
				0.01f,
				2,
				0);
			
			long count = 0;
			
			for(long j = 0; j < 6; ++j)
				if(spellInfo.symbols[j] != RUNE_NONE)
					++count;
			
			GRenderer->GetTextureStage(0)->setMagFilter(TextureStage::FilterLinear);
			for(int j = 0; j < 6; ++j) {
				if(spellInfo.symbols[j] != RUNE_NONE) {
					Vec2f pos;
					pos.x = 240 - (count * 32) * 0.5f + j * 32;
					pos.y = 306;
					DrawBookInterfaceItem(gui::necklace.pTexTab[spellInfo.symbols[j]], Vec2f(pos), Color::white, 0.000001f);
				}
			}
			GRenderer->GetTextureStage(0)->setMagFilter(TextureStage::FilterNearest);
		}
		
		if(spellInfo.tc) {
			GRenderer->SetRenderState(Renderer::AlphaBlending, true);
			GRenderer->SetBlendFunc(BlendZero, BlendInvSrcColor);
			
			Color color;
			if(flyingover) {
				color = Color::white;
				
				if(eeMouseDown1()) {
					player.SpellToMemorize.bSpell = true;
					
					for(long j = 0; j < 6; j++) {
						player.SpellToMemorize.iSpellSymbols[j] = spellInfo.symbols[j];
					}
					
					player.SpellToMemorize.lTimeCreation = arxtime.now_ul();
				}
			} else {
				color = Color(168, 208, 223, 255);
			}
			
			GRenderer->GetTextureStage(0)->setMagFilter(TextureStage::FilterLinear);
			DrawBookInterfaceItem(spellInfo.tc, fPos, color, 0.000001f);
			GRenderer->GetTextureStage(0)->setMagFilter(TextureStage::FilterNearest);
			
			GRenderer->SetRenderState(Renderer::AlphaBlending, false);
		}
		
		tmpPos.x ++;
		
		if(tmpPos.x >= 2) {
			tmpPos.x = 0;
			tmpPos.y ++;
		}
	}
	
	if(!bFlyingOver) {
		OLD_FLYING_OVER = -1;
		FLYING_OVER = -1;
	}
}
