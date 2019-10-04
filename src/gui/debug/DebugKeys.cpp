/*
 * Copyright 2018-2019 Arx Libertatis Team (see the AUTHORS file)
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

#include "gui/debug/DebugKeys.h"

#include <boost/range/size.hpp>

#include "core/Core.h"
#include "core/GameTime.h"
#include "gui/debug/DebugPanel.h"
#include "input/Input.h"

static const PlatformDuration g_debugTriggersDecayDuration = PlatformDurationMs(200);

bool g_debugToggles[10];
bool g_debugTriggers[10];
PlatformInstant g_debugTriggersTime[10] = { 0 };
float g_debugValues[10];

static bool g_debugTogglesEnabled = false;

void debug_keysUpdate() {
	
	if(GInput->isKeyPressedNowPressed(Keyboard::Key_NumLock)) {
		g_debugTogglesEnabled = !g_debugTogglesEnabled;
	}
	
	if(!g_debugTogglesEnabled) {
		return;
	}
	
	for(size_t i = 0; i < size_t(boost::size(g_debugToggles)); i++) {
		g_debugTriggers[i] = false;
		
		if(GInput->isKeyPressed(Keyboard::Key_NumPadEnter)) {
			if(   GInput->isKeyPressed(Keyboard::Key_NumPad0 + i)
			   && g_platformTime.frameStart() - g_debugTriggersTime[i] > g_debugTriggersDecayDuration
			) {
				g_debugTriggersTime[i] = g_platformTime.frameStart();
				g_debugTriggers[i] = true;
			}
		} else {
			if(GInput->isKeyPressedNowPressed(Keyboard::Key_NumPad0 + i)) {
				g_debugToggles[i] = !g_debugToggles[i];
			}
			if(GInput->isKeyPressed(Keyboard::Key_LeftShift)
			   && GInput->isKeyPressed(Keyboard::Key_LeftAlt)
			   && GInput->isKeyPressedNowPressed(Keyboard::Key_0 + i)) {
				g_debugToggles[i] = !g_debugToggles[i];
			}
		}
	}
}

void ShowDebugToggles() {
	
	if(!g_debugTogglesEnabled) {
		return;
	}
	
	DebugBox togglesBox = DebugBox(Vec2i(10, 10), "Debug Keys");
	togglesBox.add("Key", "Tog", "Trig", "Val");
	
	for(size_t i = 0; i < size_t(boost::size(g_debugToggles)); i++) {
		bool trigg = (g_platformTime.frameStart() - g_debugTriggersTime[i] <= g_debugTriggersDecayDuration);
		
		togglesBox.add(i, g_debugToggles[i] ? "on" : "off", trigg ? "X" : "", g_debugValues[i]);
	}
	togglesBox.print(g_size.bottomRight());
}
