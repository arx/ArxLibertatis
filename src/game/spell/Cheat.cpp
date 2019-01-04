/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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

#include "game/spell/Cheat.h"

#include <cstring>

#include "core/Core.h"
#include "core/GameTime.h"

#include "game/Entity.h"
#include "game/Player.h"
#include "game/Inventory.h"
#include "game/Equipment.h"

#include "gui/Console.h"
#include "gui/MiniMap.h"
#include "gui/Notification.h"
#include "gui/Speech.h"

#include "scene/Interactive.h"
#include "scene/GameSound.h"

#include "graphics/Math.h"
#include "graphics/Color.h"
#include "graphics/Draw.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/particle/ParticleEffects.h"


long passwall = 0;
long cur_mx = 0;
static long cur_pnux = 0;
long cur_pom = 0;
long cur_rf = 0;
long cur_mr = 0;
static long cur_sm = 0;
static long cur_bh = 0;

long sp_arm = 0;
static long cur_arm = 0;
static long cur_sos = 0;
static long cur_console = 0;

long cur_mega = 0;
static PlatformInstant sp_max_start = 0;
long sp_wep = 0;
short uw_mode = 0;

static short uw_mode_pos = 0;

long sp_max = 0;

static float sp_max_y[64];
static Color sp_max_col[64];
static std::string sp_max_ch;

void CheatDrawText() {
	
	if(sp_max_start == 0) {
		return;
	}
	
	PlatformDuration elapsed = g_platformTime.frameStart() - sp_max_start;
	
	if(elapsed < PlatformDurationMs(20000)) {
		
		float modi = (PlatformDurationMs(20000) - elapsed) / PlatformDurationMs(2000) * 0.1f;
		float sizX = 16;
		
		Vec2f p = Vec2f(g_size.center());
		p.x -= sp_max_ch.length() * 0.5f * sizX;
		
		for(size_t i = 0; i < sp_max_ch.length(); i++) {
			Vec2f d = p + Vec2f(sizX * i, sp_max_y[i]);
			
			sp_max_y[i] = std::sin(d.x + elapsed / PlatformDurationMs(100)) * 30.f * modi;
			std::string tex(1, sp_max_ch[i]);
			
			UNICODE_ARXDrawTextCenter(hFontInGame, d + Vec2f(-1, -1), tex, Color::none);
			UNICODE_ARXDrawTextCenter(hFontInGame, d + Vec2f(1, 1), tex, Color::none);
			UNICODE_ARXDrawTextCenter(hFontInGame, d, tex, sp_max_col[i]);
		}
		
	}
	
}

static void DisplayCheatText(const char * text) {
	sp_max_ch = text;
	sp_max_start = g_platformTime.frameStart();
}

static void MakeSpCol() {
	
	ARX_SPSound();
	
	for(long i = 0; i < 64; i++) {
		sp_max_y[i] = 0;
	}
	
	sp_max_col[0] = Color::blue;
	sp_max_col[1] = Color::green;
	sp_max_col[2] = Color::red;
	sp_max_col[3] = Color::cyan;
	sp_max_col[4] = Color::magenta;
	sp_max_col[5] = Color::yellow;
	
	for(size_t i = 6; i < 24; i++) {
		sp_max_col[i] = sp_max_col[i - 6];
	}
	
	for(size_t i = 24; i < 27; i++) {
		sp_max_col[i] = sp_max_col[i - 3];
	}
	
	for(size_t i = 27; i < 33; i++) {
		sp_max_col[i] = sp_max_col[i - 9];
	}
}


void CheatReset() {
	
	sp_max_start = 0;
	
	sp_arm = 0;
	cur_arm = 0;
	cur_sm = 0;
	sp_wep = 0;
	sp_max = 0;
	cur_mx = 0;
	cur_pom = 0;
	cur_rf = 0;
	cur_mr = 0;
	cur_console = 0;
}

void CheatDetectionReset() {
	
	cur_arm = 0;
	cur_mega = 0;
	passwall = 0;
	
	if(cur_mr != 3) {
		cur_mr = 0;
	}
	
	if(cur_mx != 3) {
		cur_mx = 0;
	}
	
	if(cur_rf != 3) {
		cur_rf = 0;
	}
	
	if(cur_pom != 3) {
		cur_pom = 0;
	}
	
	if(cur_pnux < 3) {
		cur_pnux = 0;
	}
	
	if(cur_sm < 3) {
		cur_sm = 0;
	}
	
	cur_bh = 0;
	cur_sos = 0;
	cur_console = 0;
	
}


long BH_MODE = 0;
static void EERIE_OBJECT_SetBHMode() {
	if(BH_MODE) {
		BH_MODE = 0;
	} else {
		BH_MODE = 1;
		MakeCoolFx(player.pos);
		MakeSpCol();
		DisplayCheatText("!!!_Super-Deformed_!!!");
	}
}

static void ApplySPWep() {
	
	if(!sp_wep) {
		
		ARX_SPSound();
		
		res::path cls = "graph/obj3d/interactive/items/weapons/sword_mx/sword_mx";
		Entity * ioo = AddItem(cls);
		if(ioo) {
			sp_wep = 1;
			MakeCoolFx(player.pos);
			MakeCoolFx(player.pos);
			ioo->scriptload = 1;
			SendInitScriptEvent(ioo);
			
			giveToPlayer(ioo);
			
			MakeSpCol();
			DisplayCheatText("!!!_Grosbillite_!!!");
		}
	}
}



static void ApplyCurSOS() {
	MakeSpCol();
	g_miniMap.reveal();
	DisplayCheatText("!!!_Temple of Elemental Lavis_!!!");
}

static void ApplySPBow() {
	
	ARX_SPSound();
	
	const char * cls = "graph/obj3d/interactive/items/weapons/bow_mx/bow_mx";
	Entity * ioo = AddItem(cls);
	if(ioo) {
		
		MakeCoolFx(player.pos);
		MakeCoolFx(player.pos);
		
		ioo->scriptload = 1;
		SendInitScriptEvent(ioo);
		
		giveToPlayer(ioo);
		
		MakeSpCol();
		DisplayCheatText("!!!_Bow to Samy & Anne_!!!");
	}
}

static void ApplySPArm() {
	ARX_SPSound();
	
	res::path cls;
	switch (sp_arm) {
		case 0: cls = "graph/obj3d/interactive/items/armor/helmet_plate_cm/helmet_plate_cm"; break;
		case 1: cls = "graph/obj3d/interactive/items/armor/legging_plate_cm/legging_plate_cm"; break;
		case 2: cls = "graph/obj3d/interactive/items/armor/chest_plate_cm/chest_plate_cm"; break;
		default: return;
	}
	
	Entity * ioo = AddItem(cls);
	if(ioo) {
		
		sp_wep = 1;
		MakeCoolFx(player.pos);
		MakeCoolFx(player.pos);
		ioo->scriptload = 1;
		SendInitScriptEvent(ioo);
		
		giveToPlayer(ioo);
		
		MakeSpCol();
		
		const char * text = "!! Toi aussi cherches les Cheats !!";
		
		switch (sp_arm) {
			case 0: text = "------ZoliChapo------"; break;
			case 1: text = "-----TiteBottine-----"; break;
			case 2: text = "-----Roooo-La-La-----"; break;
			default: return;
		}
		DisplayCheatText(text);
	}
	
	sp_arm++;
}

static void ApplyCurPNux() {
	
	MakeSpCol();
	DisplayCheatText("! PhilNux & Gluonne !");
	
	player.m_cheatPnuxActive = (player.m_cheatPnuxActive + 1) % 3;
	
	// TODO Create a post-processing effect for that cheat... see original source...
	
	cur_pnux = 0;
}

static void ApplyPasswall() {
	MakeSpCol();
	DisplayCheatText("!!! PassWall !!!");
	USE_PLAYERCOLLISIONS = !USE_PLAYERCOLLISIONS;
}

static void ApplySPRf() {
	if(cur_rf == 3) {
		MakeSpCol();
		DisplayCheatText("!!! RaFMode !!!");
	}
}

static void ApplyCurMr() {
	if(cur_mr == 3) {
		MakeSpCol();
		DisplayCheatText("!!! Marianna !!!");
	}
}

static void ApplySPuw() {
	uw_mode_pos = 0;
	uw_mode = ~uw_mode;
	ARX_SOUND_PlayCinematic("menestrel_uw2", true);
	MakeCoolFx(player.pos);
	if(uw_mode) {
		MakeSpCol();
		DisplayCheatText("~-__-~~-__.U.W.__-~~-__-~");
	}
}

static void ApplySPMax() {
	
	MakeCoolFx(player.pos);
	sp_max = ~sp_max;
	
	if (sp_max) {
		MakeSpCol();
		DisplayCheatText("!!!_FaNt0mAc1e_!!!");
		
		player.skin = 4;
		
		ARX_EQUIPMENT_RecreatePlayerMesh();
		
		ARX_PLAYER_Rune_Add_All();
		std::string text = "!!!!!!! FanTomAciE !!!!!!!";
		notification_add(text);
		player.Attribute_Redistribute += 10;
		player.Skill_Redistribute += 50;
		player.level = std::max(player.level, short(10));
		player.xp = GetXPforLevel(10);
	} else {
		TextureContainer * tcm;
		tcm = TextureContainer::Load("graph/obj3d/textures/npc_human_cm_hero_head");
		if(tcm) {
			delete tcm;
			player.heads[0] = TextureContainer::Load("graph/obj3d/textures/npc_human_base_hero_head");
			player.heads[1] = TextureContainer::Load("graph/obj3d/textures/npc_human_base_hero2_head");
			player.heads[2] = TextureContainer::Load("graph/obj3d/textures/npc_human_base_hero3_head");
			ARX_EQUIPMENT_RecreatePlayerMesh();
		}
	}
}

static void ApplyConsole() {
	MakeSpCol();
	DisplayCheatText("!!! Arx Libertatis !!!");
	g_console.open();
}

static TextureContainer * Mr_tc = NULL;

void CheckMr() {
	
	if(cur_mr == 3) {
		if(GRenderer && Mr_tc) {
			Vec2f pos = Vec2f(g_size.topRight()) + Vec2f(-128.f * g_sizeRatio.x, 0.f);
			Vec2f size = Vec2f(128.f, 128.f) * g_sizeRatio;
			Rectf rect = Rectf(pos, size.x, size.y);
			UseRenderState state(render2D().blendAdditive());
			EERIEDrawBitmap(rect, 0.0001f, Mr_tc, Color::gray(0.5f + PULSATE * 0.1f));
		} else {
			Mr_tc = TextureContainer::LoadUI("graph/particles/(fx)_mr");
		}
	}
	
}


void handleCheatRuneDetection(CheatRune rune) {
	switch(rune) {
		case CheatRune_AAM: {
			if(cur_console == 0) {
				cur_console++;
			} else {
				cur_console = -1;
			}
			break;
		}
		case CheatRune_COMUNICATUM: {
			if(cur_console == 3) {
				cur_console++;
			} else {
				cur_console = -1;
			}
			break;
		}
		case CheatRune_KAOM: {
			if(cur_arm >= 0 && (cur_arm & 1)) {
				cur_arm++;
				if(cur_arm > 20) {
					ApplySPArm();
				}
			} else {
				cur_arm = -1;
			}
			break;
		}
		case CheatRune_MEGA: {
			if(cur_arm >= 0 && !(cur_arm & 1)) {
				cur_arm++;
			} else {
				cur_arm = -1;
			}
			
			if(cur_console == 1) {
				cur_console++;
			} else {
				cur_console = -1;
			}
			break;
		}
		case CheatRune_SPACIUM: {
			if(cur_console == 4) {
				ApplyConsole();
			} else {
				cur_console = -1;
			}
			break;
		}
		case CheatRune_STREGUM: {
			if(cur_console == 2) {
				cur_console++;
			} else {
				cur_console = -1;
			}
			break;
		}
		case CheatRune_U: {
			if(uw_mode_pos == 0) {
				uw_mode_pos++;
			}
			break;
		}
		case CheatRune_W: {
			if(uw_mode_pos == 1) {
				ApplySPuw();
			}
			break;
		}
		case CheatRune_S: {
			if(cur_sm == 0) {
				cur_sm++;
			}
			
			if(cur_bh == 0) {
				cur_bh++;
			}
			
			if(cur_bh == 2) {
				cur_bh++;
			}
			
			if(cur_sos == 0) {
				cur_sos++;
			}
			
			if(cur_sos == 2) {
				cur_sos = 0;
				ApplyCurSOS();
			}
			break;
		}
		case CheatRune_P: {
			if(cur_pom == 0) {
				cur_pom++;
			}
			if(cur_pnux == 0) {
				cur_pnux++;
			}
			
			if(cur_pnux == 2) {
				cur_pnux++;
			}
			
			if(cur_bh == 1) {
				cur_bh++;
			}
			
			if(cur_bh == 3) {
				cur_bh = 0;
				EERIE_OBJECT_SetBHMode();
			}
			break;
		}
		case CheatRune_M: {
			if(cur_sm == 2) {
				cur_sm++;
				ApplySPBow();
			}
			
			if(cur_mx == 0) {
				cur_mx = 1;
			}
			
			if(cur_mr == 0) {
				cur_mr = 1;
			}
			
			if(cur_pom == 2) {
				cur_pom++;
				ApplySPWep();
			}
			break;
		}
		case CheatRune_A: {
			if(cur_mr == 1) {
				cur_mr = 2;
				MakeCoolFx(player.pos);
			}
			
			if(cur_mx == 1) {
				cur_mx = 2;
				MakeCoolFx(player.pos);
			}

			if(cur_rf == 1) {
				cur_rf = 2;
				MakeCoolFx(player.pos);
			}

			if(cur_sm == 1) {
				cur_sm++;
			}
			break;
		}
		case CheatRune_X: {
			if(cur_mx == 2) {
				cur_mx = 3;
				ApplySPMax();
			}
			break;
		}
		case CheatRune_26: {
			if(cur_pnux == 1) {
				cur_pnux++;
			}
			
			if(cur_pnux == 3) {
				cur_pnux++;
				ApplyCurPNux();
			}
			break;
		}
		case CheatRune_O: {
			if(cur_pom == 1) {
				cur_pom++;
			}
			
			if(cur_sos == 1) {
				cur_sos++;
			}
			break;
		}
		case CheatRune_R: {
			if(cur_mr == 2) {
				cur_mr = 3;
				MakeCoolFx(player.pos);
				ApplyCurMr();
			}
			
			if(cur_rf == 0) {
				cur_rf = 1;
			}
			break;
		}
		case CheatRune_F: {
			if(cur_rf == 2) {
				cur_rf = 3;
				MakeCoolFx(player.pos);
				ApplySPRf();
			}
			break;
		}
		case CheatRune_Passwall: {
			passwall++;
			if(passwall == 3) {
				passwall = 0;
				ApplyPasswall();
			}
			break;
		}
		case CheatRune_ChangeSkin: {
			player.skin++;

			if(player.skin == 4 && Random::getf() < 0.9f) {
				player.skin++;
			}
			
			if(player.skin > 5) {
				player.skin = 0;
			}
			ARX_EQUIPMENT_RecreatePlayerMesh();
			break;
		}
		default: {
			break;
		}
	}
}
