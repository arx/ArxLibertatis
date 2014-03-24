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

#include "gui/DebugHud.h"

#include <cstdio>
#include <string>
#include <iomanip>
#include <deque>

#include "core/Core.h"
#include "core/Application.h"
#include "core/Version.h"
#include "core/GameTime.h"

#include "game/Player.h"

#include "math/Types.h"

#include "graphics/particle/ParticleEffects.h"
#include "graphics/font/Font.h"
#include "gui/Text.h"
#include "gui/Interface.h"

#include "ai/PathFinderManager.h"
#include "script/ScriptEvent.h"
#include "scene/Interactive.h"
#include "game/EntityManager.h"
#include "game/NPC.h"

#include "graphics/Renderer.h"
#include "graphics/DrawLine.h"

#include "window/RenderWindow.h"

std::string LAST_FAILED_SEQUENCE = "none";

static long LASTfpscount = 0;
static float LASTfps2 = 0;
static float fps2 = 0;
static float fps2min = 0;

void ShowTestText() {

	char tex[256];

	Vec2i pos(10, 10);
	s32 lineOffset = hFontDebug->getLineHeight() + 2;

	hFontDebug->draw(pos, arx_version, Color::red + Color::green);
	pos.y += lineOffset;

	sprintf(tex, "Level: %s", LastLoadedScene.string().c_str());
	hFontDebug->draw(pos, tex, Color::white);
	pos.y += lineOffset;

	sprintf(tex, "Position: %5.0f %5.0f %5.0f",player.pos.x,player.pos.y,player.pos.z);
	hFontDebug->draw(pos, tex, Color::white);
	pos.y += lineOffset;

	sprintf(tex, "Last Failed Sequence: %s", LAST_FAILED_SEQUENCE.c_str());
	hFontDebug->draw(pos, tex, Color::white);
	pos.y += lineOffset;

}

extern float CURRENT_PLAYER_COLOR;
#ifdef BUILD_EDITOR
long LastSelectedIONum = -1;
#endif

void ShowInfoText() {

	unsigned long uGAT = (unsigned long)(arxtime) / 1000;
	long GAT=(long)uGAT;
	char tex[256];
	float fpss2=1000.f/framedelay;
	LASTfpscount++;

	float fps2v = std::max(fpss2, LASTfps2);
	float fps2vmin = std::min(fpss2, LASTfps2);

	if(LASTfpscount > 49)  {
		LASTfps2 = 0;
		LASTfpscount = 0;
		fps2 = fps2v;
		fps2min = fps2vmin;
	} else {
		LASTfps2 = fpss2;
	}

	sprintf(tex, "%ld Prims %4.02f fps ( %3.02f - %3.02f ) [%3.0fms]",
			EERIEDrawnPolys, FPS, fps2min, fps2, framedelay);
	hFontDebug->draw(70, 32, tex, Color::white);

	float poss = -666.66f;
	EERIEPOLY * ep = CheckInPoly(player.pos.x, player.pos.y, player.pos.z);
	float tempo = 0.f;

	if(ep && GetTruePolyY(ep, player.pos, &tempo))
		poss = tempo;

	sprintf(tex, "Position  x:%7.0f y:%7.0f [%7.0f] z:%6.0f a%3.0f b%3.0f FOK %3.0f",
			player.pos.x, player.pos.y + player.size.y, poss, player.pos.z,
			player.angle.getYaw(), player.angle.getPitch(),
			ACTIVECAM->focal);
	hFontDebug->draw(70, 48, tex, Color::white);

	sprintf(tex, "AnchorPos x:%6.0f y:%6.0f z:%6.0f TIME %lds Part %ld - %d",
			player.pos.x - Mscenepos.x,
			player.pos.y + player.size.y - Mscenepos.y,
			player.pos.z - Mscenepos.z,
			GAT, getParticleCount(), player.doingmagic);
	hFontDebug->draw(70, 64, tex, Color::white);

	if(player.onfirmground == 0)
		hFontDebug->draw(200, 280, "OFFGRND", Color::white);

	sprintf(tex, "Jump %f cinema %f %d %d - Pathfind %ld(%s)",
			player.jumplastposition, CINEMA_DECAL,
			DANAEMouse.x, DANAEMouse.y,
			EERIE_PATHFINDER_Get_Queued_Number(),
			PATHFINDER_WORKING ? "Working" : "Idled");
	hFontDebug->draw(70, 80, tex, Color::white);

	Entity * io=ARX_SCRIPT_Get_IO_Max_Events();

	if(!io) {
		sprintf(tex, "Events %ld (IOmax N/A) Timers %ld",
				ScriptEvent::totalCount, ARX_SCRIPT_CountTimers());
	} else {
		sprintf(tex, "Events %ld (IOmax %s %d) Timers %ld",
				ScriptEvent::totalCount, io->idString().c_str(),
				io->stat_count, ARX_SCRIPT_CountTimers());
	}
	hFontDebug->draw(70, 94, tex, Color::white);

	io = ARX_SCRIPT_Get_IO_Max_Events_Sent();

	if(io) {
		sprintf(tex, "Max SENDER %s %d)", io->idString().c_str(), io->stat_sent);
		hFontDebug->draw(70, 114, tex, Color::white);
	}

	float slope = 0.f;
	ep = CheckInPoly(player.pos.x, player.pos.y - 10.f, player.pos.z);

	if(ep)
		slope = ep->norm.y;

	sprintf(tex, "Velocity %3.0f %3.0f %3.0f Slope %3.3f",
			player.physics.velocity.x, player.physics.velocity.y, player.physics.velocity.z, slope);
	hFontDebug->draw(70, 128, tex, Color::white);

#ifdef BUILD_EDITOR
	if(ValidIONum(LastSelectedIONum)) {
		io = entities[LastSelectedIONum];

		if(io) {
			if(io == entities.player()) {
				sprintf(tex, "%4.0f %4.0f %4.0f - %4.0f %4.0f %4.0f -- %3.0f %d/%ld targ %ld beh %ld",
						io->pos.x, io->pos.y, io->pos.z,
						io->move.x, io->move.y, io->move.z,
						io->_npcdata->moveproblem, io->_npcdata->pathfind.listpos, io->_npcdata->pathfind.listnb,
						io->_npcdata->pathfind.truetarget, (long)io->_npcdata->behavior);
				hFontDebug->draw(170, 420, tex, Color::white);

				sprintf(tex, "Life %4.0f/%4.0f Mana %4.0f/%4.0f Poisoned %3.1f Hunger %4.1f",
						player.life, player.maxlife, player.mana, player.maxmana, player.poison, player.hunger);
				hFontDebug->draw(170, 320, tex, Color::white);
			} else {
				if(io->ioflags & IO_NPC) {
					sprintf(tex, "%4.0f %4.0f %4.0f - %4.0f %4.0f %4.0f -- %3.0f %d/%ld targ %ld beh %ld",
							io->pos.x, io->pos.y, io->pos.z,
							io->move.x, io->move.y, io->move.z,
							io->_npcdata->moveproblem, io->_npcdata->pathfind.listpos, io->_npcdata->pathfind.listnb,
							io->_npcdata->pathfind.truetarget, (long)io->_npcdata->behavior);
					hFontDebug->draw(170, 420, tex, Color::white);

					sprintf(tex, "Life %4.0f/%4.0f Mana %4.0f/%4.0f Poisoned %3.1f",
							io->_npcdata->life, io->_npcdata->maxlife, io->_npcdata->mana,
							io->_npcdata->maxmana, io->_npcdata->poisonned);
					hFontDebug->draw(170, 320, tex, Color::white);

					sprintf(tex, "AC %3.0f Absorb %3.0f",
							ARX_INTERACTIVE_GetArmorClass(io), io->_npcdata->absorb);
					hFontDebug->draw(170, 335, tex, Color::white);

					if(io->_npcdata->pathfind.flags & PATHFIND_ALWAYS) {
						hFontDebug->draw(170, 360, "PF_ALWAYS", Color::white);
					} else {
						sprintf(tex, "PF_%ld", (long)io->_npcdata->pathfind.flags);
						hFontDebug->draw(170, 360, tex, Color::white);
					}
				}

				if(io->ioflags & IO_FIX) {
					sprintf(tex, "Durability %4.0f/%4.0f Poisonous %3d count %d",
							io->durability, io->max_durability, io->poisonous, io->poisonous_count);
					hFontDebug->draw(170, 320, tex, Color::white);
				}

				if(io->ioflags & IO_ITEM) {
					sprintf(tex, "Durability %4.0f/%4.0f Poisonous %3d count %d",
							io->durability, io->max_durability, io->poisonous, io->poisonous_count);
					hFontDebug->draw(170, 320, tex, Color::white);
				}
			}
		}
	}
#endif // BUILD_EDITOR

	long zap = IsAnyPolyThere(player.pos.x,player.pos.z);
	sprintf(tex, "POLY %ld", zap);
	hFontDebug->draw(270, 220, tex, Color::white);

	sprintf(tex, "COLOR %3.0f Stealth %3.0f",
			CURRENT_PLAYER_COLOR, GetPlayerStealth());
	hFontDebug->draw(270, 200, tex, Color::white);

	ARX_SCRIPT_Init_Event_Stats();
}

void ShowFPS() {
	std::ostringstream oss;
	oss << std::fixed << std::setprecision(2) << FPS << " FPS";
	hFontDebug->draw(Vec2i(10, 10), oss.str(), Color::white);
}

void ShowDebugToggles() {
	for(size_t i = 0; i < ARRAY_SIZE(g_debugToggles); i++) {
		std::stringstream textStream;
		textStream << "Toggle " << i << ": " << (g_debugToggles[i] ? "true" : "false");
		hFontDebug->draw(0.f, i * hFontDebug->getLineHeight(), textStream.str(), Color::white);
	}
}

void ShowFpsGraph() {

	GRenderer->ResetTexture(0);

	static std::deque<float> lastFPSArray;
	lastFPSArray.push_front(1000 / arxtime.get_frame_delay());

	Vec2i windowSize = mainApp->getWindow()->getSize();
	if(lastFPSArray.size() == size_t(windowSize.x))
	{
		lastFPSArray.pop_back();
	}

	float avg = 0;
	float worst = lastFPSArray[0];

	std::vector<TexturedVertex> vertices;
	vertices.resize(lastFPSArray.size());

	const float SCALE_Y = 2.0f;

	for(size_t i = 0; i < lastFPSArray.size(); ++i)
	{
		float time = lastFPSArray[i];

		avg += lastFPSArray[i];
		worst = std::min(worst, lastFPSArray[i]);

		vertices[i].color = 0xFFFFFFFF;
		vertices[i].p.x = i;
		vertices[i].p.y = windowSize.y - (time * SCALE_Y);
		vertices[i].p.z = 1.0f;
		vertices[i].rhw = 1.0f;
	}
	avg /= lastFPSArray.size();

	EERIEDRAWPRIM(Renderer::LineStrip, &vertices[0], vertices.size());

	Color avgColor = Color::blue * 0.5f + Color::white * 0.5f;
	float avgPos = windowSize.y - (avg * SCALE_Y);
	EERIEDraw2DLine(0, avgPos,  windowSize.x, avgPos, 1.0f, Color::blue);

	Color worstColor = Color::red * 0.5f + Color::white * 0.5f;
	float worstPos = windowSize.y - (worst * SCALE_Y);
	EERIEDraw2DLine(0, worstPos,  windowSize.x, worstPos, 1.0f, Color::red);

	Font * font = hFontDebug;
	float lineOffset = font->getLineHeight() + 2;

	std::string labels[3] = { "Average: ", "Worst: ", "Current: " };
	Color colors[3] = { avgColor, worstColor, Color::white };
	float values[3] = { avg, worst, lastFPSArray[0] };

	std::string texts[3];
	float widths[3];
	static float labelWidth = 0.f;
	static float valueWidth = 0.f;
	for(size_t i = 0; i < 3; i++) {
		// Format value
		std::ostringstream oss;
		oss << std::fixed << std::setprecision(2) << values[i] << " FPS";
		texts[i] = oss.str();
		// Calculate widths (could be done more efficiently for monospace fonts...)
		labelWidth = std::max(labelWidth, float(font->getTextSize(labels[i]).x));
		widths[i] = font->getTextSize(texts[i]).x;
		valueWidth = std::max(valueWidth, widths[i]);
	}

	float x = 10;
	float y = 10;
	float xend = x + labelWidth + 10 + valueWidth;
	for(size_t i = 0; i < 3; i++) {
		font->draw(Vec2i(x, y), labels[i], Color::gray(0.8f));
		font->draw(Vec2i(xend - widths[i], y), texts[i], colors[i]);
		y += lineOffset;
	}

}
