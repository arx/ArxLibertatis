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

#include <boost/format.hpp>

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

struct EntityFlagName {
	EntityFlags flag;
	const char * name;
};

const EntityFlagName EntityFlagNames[] = {
	{IO_UNDERWATER          , "UNDERWATER"},
	{IO_FREEZESCRIPT        , "FREEZESCRIPT"},
	{IO_ITEM                , "ITEM"},
	{IO_NPC                 , "NPC"},
	{IO_FIX                 , "FIX"},
	{IO_NOSHADOW            , "NOSHADOW"},
	{IO_CAMERA              , "CAMERA"},
	{IO_MARKER              , "MARKER"},
	{IO_ICONIC              , "ICONIC"},
	{IO_NO_COLLISIONS       , "NO_COLLISIONS"},
	{IO_GOLD                , "GOLD"},
	{IO_INVULNERABILITY     , "INVULNERABILITY"},
	{IO_NO_PHYSICS_INTERPOL , "NO_PHYSICS_INTERPOL"},
	{IO_HIT                 , "HIT"},
	{IO_PHYSICAL_OFF        , "PHYSICAL_OFF"},
	{IO_MOVABLE             , "MOVABLE"},
	{IO_UNIQUE              , "UNIQUE"},
	{IO_SHOP                , "SHOP"},
	{IO_BLACKSMITH          , "BLACKSMITH"},
	{IO_NOSAVE              , "NOSAVE"},
	{IO_FORCEDRAW           , "FORCEDRAW"},
	{IO_FIELD               , "FIELD"},
	{IO_BUMP                , "BUMP"},
	{IO_ANGULAR             , "ANGULAR"},
	{IO_BODY_CHUNK          , "BODY_CHUNK"},
	{IO_ZMAP                , "ZMAP"},
	{IO_INVERTED            , "INVERTED"},
	{IO_JUST_COLLIDE        , "JUST_COLLIDE"},
	{IO_FIERY               , "FIERY"},
	{IO_NO_NPC_COLLIDE      , "NO_NPC_COLLIDE"},
	{IO_CAN_COMBINE         , "CAN_COMBINE"}
};

std::string debugPrintEntityFlags(EntityFlags flags) {
	
	std::stringstream ss;
	for(size_t i = 0; i < ARRAY_SIZE(EntityFlagNames); i++) {
		ss << ((EntityFlagNames[i].flag & flags) ? "▣" : "□") << " " << EntityFlagNames[i].name << "\n";
	}
	
	return ss.str();
}

void drawMultilineText(Vec2i originPos, const std::string & lines) {
	
	int lineHeight = hFontDebug->getLineHeight();
	int lineOffset = 0;
	
	std::stringstream ss(lines);
	std::string line;
	while(std::getline(ss, line, '\n')){
		hFontDebug->draw(originPos.x, originPos.y + lineOffset, line, Color::white);
		lineOffset += lineHeight;
	}
}

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
EntityHandle LastSelectedIONum = EntityHandle::Invalid;

void ShowInfoText() {

	unsigned long uGAT = (unsigned long)(arxtime) / 1000;
	long GAT=(long)uGAT;
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
	
	std::stringstream ss;
	
	ss << boost::format("%4.02f fps ( %3.02f - %3.02f ) [%3.0fms]\n")
	% FPS
	% fps2min
	% fps2
	% framedelay;
	
	ss << boost::format("Prims %ld, Particles %ld\n")
	% EERIEDrawnPolys
	% getParticleCount();
	
	ss << boost::format("TIME %lds\n") % GAT;
	
	ss << "Player:\n";
	ss << boost::format(" ├─ Position:  Vec3f(%6.0f,%6.0f,%6.0f)\n")
	% player.pos.x
	% (player.pos.y + player.size.y)
	% player.pos.z;
	
	ss << boost::format(" ├─ AnchorPos: Vec3f(%6.0f,%6.0f,%6.0f)\n")
	% (player.pos.x - Mscenepos.x)
	% (player.pos.y + player.size.y - Mscenepos.y)
	% (player.pos.z - Mscenepos.z);
	
	ss << boost::format(" ├─ Rotation:  Angle(%3.0f,%3.0f,%3.0f)\n")
	% player.angle.getYaw()
	% player.angle.getPitch()
	% player.angle.getRoll();
	
	ss << boost::format(" ├─ Velocity:  Vec3f(%3.0f,%3.0f,%3.0f)\n")
	% player.physics.velocity.x
	% player.physics.velocity.y
	% player.physics.velocity.z;
	
	ss << " ├─ Ground\n";
	
	EERIEPOLY * ep = CheckInPoly(player.pos);

	float truePolyY = -666.66f;
	if(ep) {
		float tempY = 0.f;
		if(GetTruePolyY(ep, player.pos, &tempY)) {
			truePolyY = tempY;
		}
	}
	
	ep = CheckInPoly(player.pos + Vec3f(0.f, -10.f, 0.f));
	float slope = 0.f;
	if(ep)
		slope = ep->norm.y;
	
	long zap = IsAnyPolyThere(player.pos.x,player.pos.z);
	
	ss << boost::format(" ├─  ├─ Slope %3.3f\n ├─  ├─ truePolyY %6.0f\n ├─  └─ POLY %ld\n")
	% slope
	% truePolyY
	% zap;
	
	ss << boost::format("Color: %3.0f; Stealth %3.0f\n")
	% CURRENT_PLAYER_COLOR
	% GetPlayerStealth();
	
	ss << boost::format("Jump %f %s\n")
	% player.jumplastposition
	% (!player.onfirmground ? "OFFGRND" : "");
	
	ss << boost::format("Life %4.0f/%4.0f Mana %4.0f/%4.0f Poisoned %3.1f Hunger %4.1f\n")
	% player.lifePool.current
	% player.lifePool.max
	% player.manaPool.current
	% player.manaPool.max
	% player.poison
	% player.hunger;
	
	ss << boost::format("Magic: %d\n")
	% player.doingmagic;
	
	ss << boost::format("Camera focal: %3.0f\n")
	% ACTIVECAM->focal;
	
	ss << boost::format("Cinema: %f; Mouse: (%d, %d); Pathfind %ld(%s)\n")
	% CINEMA_DECAL
	% DANAEMouse.x
	% DANAEMouse.y
	% EERIE_PATHFINDER_Get_Queued_Number()
	% (PATHFINDER_WORKING ? "Working" : "Idled");
	
	ss << boost::format("Events %ld\nTimers %ld\n")
	% ScriptEvent::totalCount
	% ARX_SCRIPT_CountTimers();
	
	Entity * io = ARX_SCRIPT_Get_IO_Max_Events();
	
	ss << "Max events ";
	if(io) {
		ss << boost::format("%d %s\n")
		% io->stat_count
		% io->idString();
	} else {
		ss << "\n";
	}
	
	io = ARX_SCRIPT_Get_IO_Max_Events_Sent();
	ss << "Max sender ";
	if(io) {
		ss << boost::format("%d %s\n")
		% io->stat_sent
		% io->idString();
	} else {
		ss << "\n";
	}
	
	if(ValidIONum(LastSelectedIONum)) {
		io = entities[LastSelectedIONum];

		if(io) {
			ss << boost::format("%4.0f %4.0f %4.0f - %4.0f %4.0f %4.0f\n")
			% io->pos.x % io->pos.y % io->pos.z
			% io->move.x % io->move.y % io->move.z;
			
			if(io->ioflags & IO_NPC) {
				IO_NPCDATA * npcData = io->_npcdata;
				
				ss << boost::format("Life %4.0f/%4.0f\n")
				% npcData->lifePool.current
				% npcData->lifePool.max;
				
				ss << boost::format("Mana %4.0f/%4.0f\n")
				% npcData->manaPool.current
				% npcData->manaPool.max;
				
				ss << boost::format("Poisoned %3.1f\n")
				% npcData->poisonned;

				ss << boost::format("ArmorClass %3.0f\nAbsorb %3.0f\n")
				% ARX_INTERACTIVE_GetArmorClass(io)
				% npcData->absorb;
				
				ss << boost::format("Moveproblem %3.0f %d/%ld targ %ld\nbehavior %ld\n")
				% npcData->moveproblem
				% npcData->pathfind.listpos
				% npcData->pathfind.listnb
				% npcData->pathfind.truetarget
				% npcData->behavior;

				if(io->_npcdata->pathfind.flags & PATHFIND_ALWAYS) {
					ss << "PF_ALWAYS\n";
				} else {
					ss << boost::format("PF_%ld\n") % (long)io->_npcdata->pathfind.flags;
				}
			}

			if(io->ioflags & (IO_FIX | IO_ITEM)) {
				ss << boost::format("Durability %4.0f/%4.0f Poisonous %3d count %d\n")
				% io->durability
				% io->max_durability
				% io->poisonous
				% io->poisonous_count;
			}
			
			drawMultilineText(Vec2i(10, 450), debugPrintEntityFlags(io->ioflags));
		}
	}
	
	drawMultilineText(Vec2i(10, 10), ss.str());

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
	drawLine2D(0, avgPos,  windowSize.x, avgPos, 1.0f, Color::blue);

	Color worstColor = Color::red * 0.5f + Color::white * 0.5f;
	float worstPos = windowSize.y - (worst * SCALE_Y);
	drawLine2D(0, worstPos,  windowSize.x, worstPos, 1.0f, Color::red);

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
