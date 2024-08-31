/*
 * Copyright 2011-2022 Arx Libertatis Team (see the AUTHORS file)
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

#include "script/ScriptedInterface.h"

#include <utility>

#include "game/Inventory.h"
#include "game/Entity.h"
#include "game/Player.h"
#include "gui/Hud.h"
#include "gui/Interface.h"
#include "gui/Menu.h"
#include "gui/MiniMap.h"
#include "gui/hud/SecondaryInventory.h"
#include "scene/GameSound.h"
#include "script/ScriptEvent.h"
#include "script/ScriptUtils.h"
#include "util/Cast.h"


namespace script {

namespace {

class BookCommand : public Command {
	
public:
	
	BookCommand() : Command("book") { }
	
	Result execute(Context & context) override {
		
		HandleFlags("aem") {
			if(flg & flag('a')) {
				g_playerBook.forcePage(BOOKMODE_MINIMAP);
			}
			if(flg & flag('e')) {
				g_playerBook.forcePage(BOOKMODE_SPELLS);
			}
			if(flg & flag('m')) {
				g_playerBook.forcePage(BOOKMODE_QUESTS);
			}
		}
		
		std::string command = context.getWord();
		
		if(command == "open") {
			g_playerBook.open();
		} else if(command == "close") {
			g_playerBook.close();
		} else if(command == "change") {
			// Nothing to do, mode already changed by flags.
		} else {
			ScriptWarning << "unexpected command: " << options << " \"" << command << "\"";
		}
		
		DebugScript(' ' << options << ' ' << command);
		
		return Success;
	}
	
};

class CloseStealBagCommand : public Command {
	
public:
	
	CloseStealBagCommand() : Command("closestealbag") { }
	
	Result execute(Context & context) override {
		
		ARX_UNUSED(context);
		
		if(!(player.Interface & INTER_STEAL)) {
			return Success;
		}
		
		g_secondaryInventoryHud.close();
		
		return Success;
	}
	
};

class NoteCommand : public Command {
	
public:
	
	NoteCommand() : Command("note") { }
	
	Result execute(Context & context) override {
		
		Note::Type type;
		std::string tpname = context.getWord();
		if(tpname == "note") {
			type = Note::SmallNote;
		} else if(tpname == "notice") {
			type = Note::Notice;
		} else if(tpname == "book") {
			type = Note::Book;
		} else {
			ScriptWarning << "unexpected note type: " << tpname;
			type = Note::SmallNote;
		}
		
		std::string text = context.getWord();
		
		DebugScript(' ' << tpname << ' ' << text);
		
		ARX_INTERFACE_NoteOpen(type, toLocalizationKey(text));
		
		return Success;
	}
	
};

struct PrintGlobalVariables { };

std::ostream & operator<<(std::ostream & os, const PrintGlobalVariables & /* unused */) {
	
	for(const SCRIPT_VAR & var : svar) {
		os << var << '\n';
	}
	
	return os;
}

class ShowGlobalsCommand : public Command {
	
public:
	
	ShowGlobalsCommand() : Command("showglobals") { }
	
	Result execute(Context & context) override {
		
		ARX_UNUSED(context);
		
		DebugScript("");
		
		LogInfo << "Global variables:\n" << PrintGlobalVariables();
		
		return Success;
	}
	
};

struct PrintLocalVariables {
	
	Entity * m_entity;
	
	explicit PrintLocalVariables(Entity * entity) : m_entity(entity) { }
	
};

std::ostream & operator<<(std::ostream & os, const PrintLocalVariables & data) {
	
	for(const SCRIPT_VAR & var : data.m_entity->m_variables) {
		os << var << '\n';
	}
	
	return os;
}

static std::string getEventAndStackInfo(Context & context) {
	std::stringstream s;
	
	if(context.getMessage() < SM_MAXCMD) {
		s << " at Event " << ScriptEvent::name(context.getMessage());
	}
	
	if(context.getSender()) {
		s << " sent from " << context.getSender()->idString();
	}
	
	if(context.getParameters().size() > 0) {
		s << " with parameters (";
		for(std::string s2 : context.getParameters()) {
			s << s2 << " ";
		}
		s << ")";
	}
	
	s << context.getGoSubCallStack(" at GoSub callStack ", ", " + context.getPositionAndLineNumber());
	
	return s.str();
}

class ShowLocalsCommand : public Command {
	
public:
	
	ShowLocalsCommand() : Command("showlocals", AnyEntity) { }
	
	Result execute(Context & context) override {
		
		DebugScript("");
		
		LogInfo << "Local variables for " << context.getEntity()->idString() << getEventAndStackInfo(context) << ":\n"
			<< PrintLocalVariables(context.getEntity());
		
		return Success;
	}
	
};

class ShowVarsCommand : public Command {
	
public:
	
	ShowVarsCommand() : Command("showvars", AnyEntity) { }
	
	Result execute(Context & context) override {
		
		DebugScript("");
		
		LogInfo << "Local variables for " << context.getEntity()->idString() << getEventAndStackInfo(context) << ":\n"
			<< PrintLocalVariables(context.getEntity());
		LogInfo << "Global variables:\n" << PrintGlobalVariables();
		
		return Success;
	}
	
};

class PlayerInterfaceCommand : public Command {
	
public:
	
	PlayerInterfaceCommand() : Command("playerinterface") { }
	
	Result execute(Context & context) override {
		
		bool smooth = false;
		HandleFlags("s") {
			smooth = test_flag(flg, 's');
		}
		
		std::string command = context.getWord();
		
		DebugScript(' ' << options << ' ' << command);
		
		if(command == "hide") {
			g_hudRoot.playerInterfaceFader.requestFade(FadeDirection_Out, smooth);
		} else if(command == "show") {
			g_hudRoot.playerInterfaceFader.requestFade(FadeDirection_In, smooth);
		} else {
			ScriptWarning << "unknown command: " << command;
			return Failed;
		}
		
		return Success;
	}
	
};

class PopupCommand : public Command {
	
public:
	
	PopupCommand() : Command("popup") { }
	
	Result execute(Context & context) override {
		
		std::string message = context.getWord();
		
		DebugScript(' ' << message);
		
		return Success;
	}
	
};

class EndIntroCommand : public Command {
	
public:
	
	EndIntroCommand() : Command("endintro") { }
	
	Result execute(Context & context) override {
		
		ARX_UNUSED(context);
		
		DebugScript("");
		
		ARX_SOUND_MixerStop(ARX_SOUND_MixerGame);
		ARX_MENU_Launch(false);
		
		return Success;
	}
	
};

class EndGameCommand : public Command {
	
public:
	
	EndGameCommand() : Command("endgame") { }
	
	Result execute(Context & context) override {
		
		ARX_UNUSED(context);
		
		DebugScript("");
		
		ARX_SOUND_MixerStop(ARX_SOUND_MixerGame);
		ARX_MENU_Launch(false);
		ARX_MENU_Clicked_CREDITS();
		
		return Success;
	}
	
};

class MapMarkerCommand : public Command {
	
public:
	
	MapMarkerCommand() : Command("mapmarker") { }
	
	Result execute(Context & context) override {
		
		bool remove = false;
		HandleFlags("r") {
			remove = test_flag(flg, 'r');
		}
		
		if(remove) {
			
			std::string marker = context.getWord();
			
			DebugScript(' ' << options << ' ' << marker);
			
			g_miniMap.mapMarkerRemove(toLocalizationKey(marker));
			
		} else {
			
			float x = context.getFloat();
			float y = context.getFloat();
			float level = context.getFloat();
			
			std::string marker = context.getWord();
			
			DebugScript(' ' << options << ' ' << x << ' ' << y << ' ' << level << ' ' << marker);
			
			if(level < 1.f || !util::is_in_range<s16>(level)) {
				ScriptError << "Invalid map level: " << level;
				return Failed;
			}
			
			g_miniMap.mapMarkerAdd(Vec2f(x, y), MapLevel(u32(level - 1)), std::string(toLocalizationKey(marker)));
			
		}
		
		return Success;
	}
	
};

class DrawSymbolCommand : public Command {
	
public:
	
	DrawSymbolCommand() : Command("drawsymbol", AnyEntity) { }
	
	Result execute(Context & context) override {
		
		std::string symbol = context.getWord();
		
		GameDuration duration = std::chrono::duration<float, std::milli>(context.getFloat());
		
		DebugScript(' ' << symbol << ' ' << toMsf(duration));
		
		ARX_SPELLS_RequestSymbolDraw(context.getEntity(), symbol, duration);
		
		return Success;
	}
	
};

} // anonymous namespace

void setupScriptedInterface() {
	
	ScriptEvent::registerCommand(std::make_unique<BookCommand>());
	ScriptEvent::registerCommand(std::make_unique<CloseStealBagCommand>());
	ScriptEvent::registerCommand(std::make_unique<NoteCommand>());
	ScriptEvent::registerCommand(std::make_unique<ShowGlobalsCommand>());
	ScriptEvent::registerCommand(std::make_unique<ShowLocalsCommand>());
	ScriptEvent::registerCommand(std::make_unique<ShowVarsCommand>());
	ScriptEvent::registerCommand(std::make_unique<PlayerInterfaceCommand>());
	ScriptEvent::registerCommand(std::make_unique<PopupCommand>());
	ScriptEvent::registerCommand(std::make_unique<EndIntroCommand>());
	ScriptEvent::registerCommand(std::make_unique<EndGameCommand>());
	ScriptEvent::registerCommand(std::make_unique<MapMarkerCommand>());
	ScriptEvent::registerCommand(std::make_unique<DrawSymbolCommand>());
	
}

} // namespace script
