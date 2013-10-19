/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
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

#include "core/GameTime.h"

#include "game/Entity.h"
#include "game/Player.h"
#include "game/Inventory.h"
#include "game/Equipment.h"

#include "gui/MiniMap.h"
#include "gui/Speech.h"

#include "scene/Interactive.h"
#include "scene/GameSound.h"

#include "graphics/Math.h"
#include "graphics/Color.h"
#include "graphics/Draw.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/particle/ParticleEffects.h"

extern Rect g_size;

extern void ARX_SPSound();

long cur_mx=0;
long cur_pnux=0;
long cur_pom=0;
long cur_rf=0;
long cur_mr=0;
long cur_sm=0;
long cur_bh=0;

long sp_arm=0;
long cur_arm=0;
long cur_sos=0;

long cur_mega=0;
float sp_max_start = 0;
long sp_wep=0;
short uw_mode=0;

short uw_mode_pos=0;

long sp_max = 0;

float sp_max_y[64];
Color sp_max_col[64];
char sp_max_ch[64];
long sp_max_nb;

void Manage_sp_max() {

	float v = float(arxtime) - sp_max_start;

	if(sp_max_start != 0 && v < 20000) {
		float modi = (20000 - v) * ( 1.0f / 2000 ) * ( 1.0f / 10 );
		float sizX = 16;
		float px = (float)g_size.center().x - (float)sp_max_nb * ( 1.0f / 2 ) * sizX;
		float py = (float)g_size.center().y;

		for(long i = 0; i < sp_max_nb; i++) {
			float dx = px + sizX * (float)i;
			float dy = py + sp_max_y[i];
			sp_max_y[i] = EEsin(dx + (float)float(arxtime) * ( 1.0f / 100 )) * 30.f * modi;
			std::string tex(1, sp_max_ch[i]);

			UNICODE_ARXDrawTextCenter(hFontInBook, dx - 1, dy - 1, tex, Color::none);
			UNICODE_ARXDrawTextCenter(hFontInBook, dx + 1, dy + 1, tex, Color::none);
			UNICODE_ARXDrawTextCenter(hFontInBook, dx, dy, tex, sp_max_col[i]);
		}
	}
}

void MakeSpCol() {

	ARX_SPSound();

	for(long i = 0; i < 64; i++) {
		sp_max_y[i] = 0;
	}

	sp_max_col[0] = Color::fromRGBA(0x00FF0000);
	sp_max_col[1] = Color::fromRGBA(0x0000FF00);
	sp_max_col[2] = Color::fromRGBA(0x000000FF);

	sp_max_col[3] = Color::fromRGBA(0x00FFFF00);
	sp_max_col[4] = Color::fromRGBA(0x00FF00FF);
	sp_max_col[5] = Color::fromRGBA(0x0000FFFF);

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
}


long BH_MODE = 0;
void EERIE_OBJECT_SetBHMode()
{
	if (BH_MODE)
		BH_MODE=0;
	else
	{
		BH_MODE=1;
		MakeCoolFx(&player.pos);
		MakeSpCol();
		strcpy(sp_max_ch,"!!!_Super-Deformed_!!!");
		sp_max_nb=strlen(sp_max_ch);
		sp_max_start=arxtime.get_updated();
			}
}

void ApplySPWep() {

	if(!sp_wep) {

		ARX_SPSound();

		res::path cls = "graph/obj3d/interactive/items/weapons/sword_mx/sword_mx";
		Entity * ioo = AddItem(cls);
		if(ioo) {

			sp_wep = 1;
			MakeCoolFx(&player.pos);
			MakeCoolFx(&player.pos);
			ioo->scriptload = 1;
			SendInitScriptEvent(ioo);

			giveToPlayer(ioo);

			MakeSpCol();
			strcpy(sp_max_ch,"!!!_Grosbillite_!!!");
			sp_max_nb=strlen(sp_max_ch);
			sp_max_start=arxtime.get_updated();
		}
	}
}



void ApplyCurSOS() {
	MakeSpCol();
	g_miniMap.reveal();
	strcpy(sp_max_ch,"!!!_Temple of Elemental Lavis_!!!");
	sp_max_nb=strlen(sp_max_ch);
	sp_max_start=arxtime.get_updated();
}

void ApplySPBow() {

	ARX_SPSound();

	const char * cls = "graph/obj3d/interactive/items/weapons/bow_mx/bow_mx";
	Entity * ioo = AddItem(cls);
	if(ioo) {

		MakeCoolFx(&player.pos);
		MakeCoolFx(&player.pos);

		ioo->scriptload = 1;
		SendInitScriptEvent(ioo);

		giveToPlayer(ioo);

		MakeSpCol();
		strcpy(sp_max_ch,"!!!_Bow to Samy & Anne_!!!");
		sp_max_nb=strlen(sp_max_ch);
		sp_max_start=arxtime.get_updated();
	}
}

void ApplySPArm() {
	ARX_SPSound();

	res::path cls;
	switch (sp_arm) {
		case 0:
			cls = "graph/obj3d/interactive/items/armor/helmet_plate_cm/helmet_plate_cm";
		break;
		case 1:
			cls = "graph/obj3d/interactive/items/armor/legging_plate_cm/legging_plate_cm";
		break;
		case 2:
			cls = "graph/obj3d/interactive/items/armor/chest_plate_cm/chest_plate_cm";
		break;
		default:
			return;
		break;
	}

	Entity * ioo = AddItem(cls);
	if(ioo) {

		sp_wep = 1;
		MakeCoolFx(&player.pos);
		MakeCoolFx(&player.pos);
		ioo->scriptload = 1;
		SendInitScriptEvent(ioo);

		giveToPlayer(ioo);

		MakeSpCol();
		strcpy(sp_max_ch,"!! Toi aussi cherches les Cheats !!");

		switch (sp_arm)
		{
		case 0:
			strcpy(sp_max_ch,"------ZoliChapo------");
		break;
		case 1:
			strcpy(sp_max_ch,"-----TiteBottine-----");
		break;
		case 2:
			strcpy(sp_max_ch,"-----Roooo-La-La-----");
		break;
		default:
			return;
		break;
		}

		sp_max_nb=strlen(sp_max_ch);
		sp_max_start=arxtime.get_updated();
	}

	sp_arm++;
}

long SPECIAL_PNUX;
void ApplyCurPNux() {

	MakeSpCol();
	strcpy(sp_max_ch,"! PhilNux & Gluonne !");
	sp_max_nb=strlen(sp_max_ch);

	SPECIAL_PNUX = (SPECIAL_PNUX + 1) % 3;

	// TODO-RENDERING: Create a post-processing effect for that cheat... see original source...

	cur_pnux=0;
	sp_max_start=arxtime.get_updated();
}

void ApplyPasswall() {
	MakeSpCol();
	strcpy(sp_max_ch,"!!! PassWall !!!");
	sp_max_nb=strlen(sp_max_ch);
	sp_max_start=arxtime.get_updated();

	if(USE_PLAYERCOLLISIONS)
		USE_PLAYERCOLLISIONS = false;
	else
		USE_PLAYERCOLLISIONS = true;
}

void ApplySPRf() {
	if(cur_rf == 3) {
		MakeSpCol();
		strcpy(sp_max_ch,"!!! RaFMode !!!");
		sp_max_nb=strlen(sp_max_ch);
		sp_max_start=arxtime.get_updated();
	}
}

void ApplyCurMr() {
	if(cur_mr == 3) {
		MakeSpCol();
		strcpy(sp_max_ch,"!!! Marianna !!!");
		sp_max_nb=strlen(sp_max_ch);
		sp_max_start=arxtime.get_updated();
	}
}

void ApplySPuw() {
	uw_mode_pos=0;
	uw_mode=~uw_mode;
	ARX_SOUND_PlayCinematic("menestrel_uw2", true);
	MakeCoolFx(&player.pos);
	if(uw_mode) {
		MakeSpCol();
		strcpy(sp_max_ch,"~-__-~~-__.U.W.__-~~-__-~");
		sp_max_nb=strlen(sp_max_ch);
		sp_max_start=arxtime.get_updated();
	}
}

void ApplySPMax() {

	MakeCoolFx(&player.pos);
	sp_max=~sp_max;

	if (sp_max)
	{
		MakeSpCol();
		strcpy(sp_max_ch,"!!!_FaNt0mAc1e_!!!");
		sp_max_nb=strlen(sp_max_ch);
		sp_max_start=arxtime.get_updated();

			player.skin=4;

			ARX_EQUIPMENT_RecreatePlayerMesh();

		ARX_PLAYER_Rune_Add_All();
		std::string text = "!!!!!!! FanTomAciE !!!!!!!";
		ARX_SPEECH_Add(text);
		player.Attribute_Redistribute+=10;
		player.Skill_Redistribute+=50;
		player.level=std::max((int)player.level,10);
		player.xp=GetXPforLevel(10);
	}
	else
	{
		TextureContainer * tcm;
		tcm = TextureContainer::Load("graph/obj3d/textures/npc_human_cm_hero_head");
		if(tcm) {
			delete tcm;
			player.heads[0]
				= TextureContainer::Load("graph/obj3d/textures/npc_human_base_hero_head");
			player.heads[1]
				= TextureContainer::Load("graph/obj3d/textures/npc_human_base_hero2_head");
			player.heads[2]
				= TextureContainer::Load("graph/obj3d/textures/npc_human_base_hero3_head");
			ARX_EQUIPMENT_RecreatePlayerMesh();
		}
	}
}

extern float PULSATE;
static TextureContainer * Mr_tc = NULL;

void CheckMr() {

	if(cur_mr == 3) {
		if(GRenderer && Mr_tc) {
			EERIEDrawBitmap(g_size.width()-(128.f*Xratio), 0.f, (float)128*Xratio, (float)128*Yratio,0.0001f,
							Mr_tc, Color::gray(0.5f + PULSATE * (1.0f/10)));
		} else {
			Mr_tc = TextureContainer::LoadUI("graph/particles/(fx)_mr");
		}
	}
}
