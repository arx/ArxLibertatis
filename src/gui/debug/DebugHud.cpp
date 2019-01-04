/*
 * Copyright 2013-2018 Arx Libertatis Team (see the AUTHORS file)
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

#include "gui/debug/DebugHud.h"

#include <algorithm>
#include <cstdio>
#include <string>
#include <iomanip>
#include <deque>
#include <numeric>

#include <boost/circular_buffer.hpp>
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
#include "gui/debug/DebugPanel.h"

#include "ai/PathFinderManager.h"
#include "script/ScriptEvent.h"
#include "scene/Interactive.h"
#include "game/EntityManager.h"
#include "game/NPC.h"
#include "game/magic/SpellRecognition.h"

#include "graphics/Renderer.h"
#include "graphics/DrawLine.h"
#include "graphics/effects/PolyBoom.h"
#include "graphics/particle/Spark.h"

#include "window/RenderWindow.h"
#include "platform/profiler/Profiler.h"


const FlagName<EntityFlags> EntityFlagNames[] = {
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

const FlagName<GameFlags> GameFlagNames[] = {
	{GFLAG_INTERACTIVITY     , "INTERACTIVITY"},
	{GFLAG_ISINTREATZONE     , "ISINTREATZONE"},
	{GFLAG_WASINTREATZONE    , "WASINTREATZONE"},
	{GFLAG_NEEDINIT          , "NEEDINIT"},
	{GFLAG_INTERACTIVITYHIDE , "INTERACTIVITYHIDE"},
	{GFLAG_DOOR              , "DOOR"},
	{GFLAG_INVISIBILITY      , "INVISIBILITY"},
	{GFLAG_NO_PHYS_IO_COL    , "NO_PHYS_IO_COL"},
	{GFLAG_VIEW_BLOCKER      , "VIEW_BLOCKER"},
	{GFLAG_PLATFORM          , "PLATFORM"},
	{GFLAG_ELEVATOR          , "ELEVATOR"},
	{GFLAG_MEGAHIDE          , "MEGAHIDE"},
	{GFLAG_HIDEWEAPON        , "HIDEWEAPON"},
	{GFLAG_NOGORE            , "NOGORE"},
	{GFLAG_GOREEXPLODE       , "GOREEXPLODE"},
	{GFLAG_NOCOMPUTATION     , "NOCOMPUTATION"}
};

const FlagName<Behaviour> BehaviourFlagNames[] = {
	{BEHAVIOUR_NONE          , "NONE"},
	{BEHAVIOUR_FRIENDLY      , "FRIENDLY"},
	{BEHAVIOUR_MOVE_TO       , "MOVE_TO"},
	{BEHAVIOUR_WANDER_AROUND , "WANDER_AROUND"},
	{BEHAVIOUR_FLEE          , "FLEE"},
	{BEHAVIOUR_HIDE          , "HIDE"},
	{BEHAVIOUR_LOOK_FOR      , "LOOK_FOR"},
	{BEHAVIOUR_SNEAK         , "SNEAK"},
	{BEHAVIOUR_FIGHT         , "FIGHT"},
	{BEHAVIOUR_DISTANT       , "DISTANT"},
	{BEHAVIOUR_MAGIC         , "MAGIC"},
	{BEHAVIOUR_GUARD         , "GUARD"},
	{BEHAVIOUR_GO_HOME       , "GO_HOME"},
	{BEHAVIOUR_LOOK_AROUND   , "LOOK_AROUND"},
	{BEHAVIOUR_STARE_AT      , "STARE_AT"}
};

const FlagName<AnimUseType> AnimUseFlagNames[] = {
	{EA_LOOP       , "LOOP"},
	{EA_REVERSE    , "REVERSE"},
	{EA_PAUSED     , "PAUSED"},
	{EA_ANIMEND    , "ANIMEND"},
	{EA_STATICANIM , "STATICANIM"},
	{EA_STOPEND    , "STOPEND"},
	{EA_FORCEPLAY  , "FORCEPLAY"},
	{EA_EXCONTROL  , "EXCONTROL"}
};

static const char * entityVisilibityToString(EntityVisilibity value) {
	switch (value) {
		case SHOW_FLAG_NOT_DRAWN:    return "NOT_DRAWN";
		case SHOW_FLAG_IN_SCENE:     return "IN_SCENE";
		case SHOW_FLAG_LINKED:       return "LINKED";
		case SHOW_FLAG_IN_INVENTORY: return "IN_INVENTORY";
		case SHOW_FLAG_HIDDEN:       return "HIDDEN";
		case SHOW_FLAG_TELEPORTING:  return "TELEPORTING";
		case SHOW_FLAG_KILLED:       return "KILLED";
		case SHOW_FLAG_MEGAHIDE:     return "MEGAHIDE";
		case SHOW_FLAG_ON_PLAYER:    return "ON_PLAYER";
		case SHOW_FLAG_DESTROYED:    return "DESTROYED";
	}
	return "Unknown";
}

static std::string prettyUs(s64 us) {
	std::ostringstream s;
	s << std::setw(7)
	  << us / 1000000ll
	  << "s"
	  << std::setfill('0')
	  << std::setw(3)
	  << (us % 1000000ll) / 1000ll
	  << "ms";
	return s.str();
}

void ShowInfoText() {
	
	DebugBox frameInfo = DebugBox(Vec2i(10, 10), "FrameInfo");
	frameInfo.add("Platform time", prettyUs(toUs(g_platformTime.frameStart())));
	frameInfo.add("Game time", prettyUs(toUs(g_gameTime.now())));
	frameInfo.add("Prims", EERIEDrawnPolys);
	frameInfo.add("Particles", getParticleCount());
	frameInfo.add("Sparks", ParticleSparkCount());
	frameInfo.add("Polybooms", long(PolyBoomCount()));
	frameInfo.print();
	
	DebugBox camBox = DebugBox(Vec2i(10, frameInfo.size().y + 5), "Camera");
	camBox.add("Position", g_camera->m_pos);
	camBox.add("Rotation", g_camera->angle);
	camBox.add("Focal", g_camera->focal);
	camBox.print();
	
	DebugBox playerBox = DebugBox(Vec2i(10, camBox.size().y + 5), "Player");
	playerBox.add("Position", player.pos);
	playerBox.add("Rotation", player.angle);
	playerBox.add("Velocity", player.physics.velocity);
	
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
	
	long zap = IsAnyPolyThere(player.pos.x, player.pos.z);
	
	playerBox.add("Ground Slope", slope);
	playerBox.add("Ground truePolyY", truePolyY);
	playerBox.add("Ground POLY", zap);
	playerBox.add("Color", CURRENT_PLAYER_COLOR);
	playerBox.add("Stealth", GetPlayerStealth());
	
	playerBox.add("Jump", player.jumplastposition);
	playerBox.add("OFFGRND", (!player.onfirmground ? "OFFGRND" : ""));
	
	playerBox.add("Life", player.lifePool);
	playerBox.add("Mana", player.manaPool);
	playerBox.add("Poisoned", player.poison);
	playerBox.add("Hunger", player.hunger);
	playerBox.add("Magic", static_cast<long>(player.doingmagic));
	playerBox.print();
	
	DebugBox miscBox = DebugBox(Vec2i(10, playerBox.size().y + 5), "Misc");
	miscBox.add(arx_name + " version", arx_version);
	miscBox.add("Level", LastLoadedScene.string());
	miscBox.add("Spell failed seq", LAST_FAILED_SEQUENCE);
	miscBox.add("Cinema", cinematicBorder.CINEMA_DECAL);
	miscBox.add("Mouse", Vec2i(DANAEMouse));
	miscBox.add("Pathfind queue", EERIE_PATHFINDER_Get_Queued_Number());
	miscBox.add("Pathfind status", EERIE_PATHFINDER_Is_Busy() ? "Working" : "Idled");
	miscBox.print();
	
	{
	struct ScriptDebugReport {
		std::string entityName;
		long events;
		long sends;
		
		ScriptDebugReport()
			: entityName("")
			, events(0)
			, sends(0)
		{}
	};
	
	ScriptDebugReport maxEvents;
	Entity * io = ARX_SCRIPT_Get_IO_Max_Events();
	if(io) {
		maxEvents.entityName = io->idString();
		maxEvents.events = io->stat_count;
	}
	
	ScriptDebugReport maxSender;
	io = ARX_SCRIPT_Get_IO_Max_Events_Sent();
	if(io) {
		maxSender.entityName = io->idString();
		maxSender.sends = io->stat_sent;
	}
	
	DebugBox scriptBox = DebugBox(Vec2i(10, miscBox.size().y + 5), "Script");
	scriptBox.add("Events", ScriptEvent::totalCount);
	scriptBox.add("Timers", ARX_SCRIPT_CountTimers());
	scriptBox.add("Max events", maxEvents.entityName);
	scriptBox.add("Max events#", maxEvents.events);
	scriptBox.add("Max sender", maxSender.entityName);
	scriptBox.add("Max sender#", maxSender.sends);
	scriptBox.print();
	}
	
		Entity * io = entities.get(LastSelectedIONum);
		if(io) {
			DebugBox entityBox = DebugBox(Vec2i(500, 10), "Entity " + io->idString());
			entityBox.add("Pos", io->pos);
			entityBox.add("Angle", io->angle);
			entityBox.add("Room", static_cast<long>(io->room));
			entityBox.add("Move", io->move);
			entityBox.add("Flags", flagNames(EntityFlagNames, io->ioflags));
			entityBox.add("GFlags", flagNames(GameFlagNames, io->gameFlags));
			entityBox.add("Show", entityVisilibityToString(io->show));
			entityBox.print();
			
			if(io->ioflags & IO_NPC) {
				IO_NPCDATA * npcData = io->_npcdata;
				
				DebugBox npcBox = DebugBox(Vec2i(500, entityBox.size().y + 5), "NPC");
				npcBox.add("Life", npcData->lifePool);
				npcBox.add("Mana", npcData->manaPool);
				npcBox.add("Poisoned", npcData->poisonned);
				npcBox.add("ArmorClass", ARX_INTERACTIVE_GetArmorClass(io));
				npcBox.add("Absorb", npcData->absorb);
				
				npcBox.add("Moveproblem", npcData->moveproblem);
				npcBox.add("Pathfind listpos", static_cast<long>(npcData->pathfind.listpos));
				npcBox.add("Pathfind listnb", npcData->pathfind.listnb);
				npcBox.add("Pathfind targ", npcData->pathfind.truetarget.handleData());
				npcBox.add("Behavior", flagNames(BehaviourFlagNames, npcData->behavior));
				
				// TODO should those really be flags ?
				PathfindFlags pflag = io->_npcdata->pathfind.flags;
				std::string pflags;
				if(pflag & PATHFIND_ALWAYS)    pflags += "ALWAYS ";
				if(pflag & PATHFIND_ONCE)      pflags += "ONCE ";
				if(pflag & PATHFIND_NO_UPDATE) pflags += "NO_UPDATE ";
				npcBox.add("Pathfind flgs", pflags);
				
				npcBox.print();
			}

			if(io->ioflags & (IO_FIX | IO_ITEM)) {
				DebugBox itemBox = DebugBox(Vec2i(500, entityBox.size().y + 5), "Item");
				
				itemBox.add("Durability", io->durability);
				itemBox.add("Durability max", io->max_durability);
				itemBox.add("Poisonous", static_cast<long>(io->poisonous));
				itemBox.add("Poisonous count", static_cast<long>(io->poisonous_count));
				itemBox.print();
			}
			
			long column2y = 400;
			
			for(size_t i = 0; i < MAX_ANIM_LAYERS; i++) {
				AnimLayer & layer = io->animlayer[i];
				
				DebugBox animLayerBox = DebugBox(Vec2i(500, column2y), str(boost::format("Anim Layer %1%") % i));
				animLayerBox.add("ctime", long(layer.ctime.t));
				animLayerBox.add("flags", flagNames(AnimUseFlagNames, layer.flags));
				
				animLayerBox.add("currentFrame", layer.currentFrame);
				if(layer.cur_anim) {
					animLayerBox.add("cur_anim", layer.cur_anim->path.string());
				} else {
					animLayerBox.add("cur_anim", "none");
				}
				
				animLayerBox.print();
				column2y = animLayerBox.size().y + 5;
			}
		}
	
	ARX_SCRIPT_Init_Event_Stats();
}

void ShowFPS() {
	std::ostringstream oss;
	oss << std::fixed << std::setprecision(2) << FPS << " FPS";
	hFontDebug->draw(Vec2i(10, 10), oss.str(), Color::white);
}


static boost::circular_buffer<float> frameDurationPlotValues;
static std::vector<TexturedVertex> frameDurationPlotVertices;

void ShowFrameDurationPlot() {
	
	Vec2i windowSize = mainApp->getWindow()->getSize();
	size_t maxSamples = size_t(windowSize.x);
	
	if(maxSamples != frameDurationPlotValues.capacity()) {
		frameDurationPlotValues.set_capacity(maxSamples);
	}
	if(maxSamples != frameDurationPlotVertices.size()) {
		frameDurationPlotVertices.resize(maxSamples);
	}
	
	GRenderer->ResetTexture(0);
	
	frameDurationPlotValues.push_front(toMs(g_platformTime.lastFrameDuration()));
	
	float avg = std::accumulate(frameDurationPlotValues.begin(), frameDurationPlotValues.end(), 0.f) / frameDurationPlotValues.size();
	float worst = *std::max_element(frameDurationPlotValues.begin(), frameDurationPlotValues.end());
	
	const float OFFSET_Y = 80.f;
	const float SCALE_Y = 4.0f;

	for(size_t i = 0; i < frameDurationPlotValues.size(); ++i)
	{
		float time = frameDurationPlotValues[i];
		frameDurationPlotVertices[i].color = Color::white.toRGB();
		frameDurationPlotVertices[i].p.x = i;
		frameDurationPlotVertices[i].p.y = OFFSET_Y + (time * SCALE_Y);
		frameDurationPlotVertices[i].p.z = 1.0f;
		frameDurationPlotVertices[i].w = 1.0f;
	}

	EERIEDRAWPRIM(Renderer::LineStrip, &frameDurationPlotVertices[0], frameDurationPlotValues.size());

	Color avgColor = Color::blue * 0.5f + Color::white * 0.5f;
	float avgPos = OFFSET_Y + (avg * SCALE_Y);
	drawLine(Vec2f(0, avgPos), Vec2f(windowSize.x, avgPos), 1.0f, Color::blue);

	Color worstColor = Color::red * 0.5f + Color::white * 0.5f;
	float worstPos = OFFSET_Y + (worst * SCALE_Y);
	drawLine(Vec2f(0, worstPos), Vec2f(windowSize.x, worstPos), 1.0f, Color::red);

	Font * font = hFontDebug;
	float lineOffset = font->getLineHeight() + 2;

	std::string labels[3] = { "Average: ", "Worst: ", "Current: " };
	Color colors[3] = { avgColor, worstColor, Color::white };
	float values[3] = { avg, worst, frameDurationPlotValues[0] };

	std::string texts[3];
	float widths[3];
	static float labelWidth = 0.f;
	static float valueWidth = 0.f;
	for(size_t i = 0; i < 3; i++) {
		// Format value
		std::ostringstream oss;
		oss << std::fixed << std::setprecision(2) << values[i] << " ms ("<< 1.f / (values[i] * 0.001f) << " FPS)";
		texts[i] = oss.str();
		// Calculate widths (could be done more efficiently for monospace fonts...)
		labelWidth = std::max(labelWidth, float(font->getTextSize(labels[i]).width()));
		widths[i] = font->getTextSize(texts[i]).width();
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
