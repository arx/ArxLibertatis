/*
 * Copyright 2018-2021 Arx Libertatis Team (see the AUTHORS file)
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

#include "core/Core.h"
#include "core/GameTime.h"
#include "gui/debug/DebugPanel.h"
#include "input/Input.h"

bool g_debugToggles[10];
float g_debugValues[10];

static bool g_debugTogglesEnabled = false;

void debug_keysUpdate() {
	
	if(GInput->isKeyPressedNowPressed(Keyboard::Key_NumLock)) {
		g_debugTogglesEnabled = !g_debugTogglesEnabled;
	}
	
	if(!g_debugTogglesEnabled) {
		return;
	}
	
	for(size_t i = 0; i < std::size(g_debugToggles); i++) {
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

void ShowDebugToggles() {
	
	if(!g_debugTogglesEnabled) {
		return;
	}
	
	DebugBox togglesBox = DebugBox(Vec2i(10, 10), "Debug Keys");
	togglesBox.add("Key", "Tog", "Val");
	
	for(size_t i = 0; i < std::size(g_debugToggles); i++) {
		togglesBox.add(i, g_debugToggles[i] ? "on" : "off", g_debugValues[i]);
	}
	togglesBox.print(g_size.bottomRight());
	
}
