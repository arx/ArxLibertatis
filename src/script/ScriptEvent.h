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

#ifndef ARX_SCRIPT_SCRIPTEVENT_H
#define ARX_SCRIPT_SCRIPTEVENT_H

#include <map>
#include <memory>
#include <string>
#include <string_view>

#include "script/Script.h"

namespace script { class Command; }

class ScriptEvent {
	
	
public:
	
	static long totalCount;
	
	static std::string_view name(ScriptMessage event);
	
	static ScriptResult send(const EERIE_SCRIPT * es, Entity * sender, Entity * entity, ScriptEventName event,
	                         ScriptParameters parameters = { }, size_t position = 0);
	
	static ScriptResult resume(const EERIE_SCRIPT * es, Entity * entity, size_t position) {
		return send(es, nullptr, entity, SM_EXECUTELINE, { }, position);
	}
	
	static void registerCommand(std::unique_ptr<script::Command> command);
	
	static void init();
	static void shutdown();
	
	typedef bool (*AutocompleteHandler)(void * context, std::string_view suggestion);
	static void autocomplete(std::string_view prefix, AutocompleteHandler handler, void * context);
	
	static bool isCommand(std::string_view command);
	
private:
	
	static std::map<std::string_view, std::unique_ptr<script::Command>> commands;
	
};

#endif // ARX_SCRIPT_SCRIPTEVENT_H
