/*
 * Copyright 2018-2022 Arx Libertatis Team (see the AUTHORS file)
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

#include "gui/Notification.h"

#include <algorithm>
#include <utility>
#include <vector>

#include "core/Core.h"
#include "core/GameTime.h"
#include "core/Localisation.h"
#include "graphics/Draw.h"
#include "graphics/Renderer.h"
#include "gui/Text.h"
#include "gui/TextManager.h"
#include "math/Rectangle.h"

struct Notification {
	
	std::string text;
	GameInstant deadline;
	
};

static std::vector<Notification> g_notifications;

extern TextureContainer * arx_logo_tc;

void notification_ClearAll() {
	g_notifications.clear();
}

void notification_add(std::string && text) {
	
	if(text.empty()) {
		return;
	}
	
	if(g_notifications.size() > 3) {
		g_notifications.erase(g_notifications.begin());
	}
	
	Notification & notification = g_notifications.emplace_back();
	notification.deadline = g_gameTime.now() + 2s + getLocalised(text).length() * 60ms;
	notification.text = std::move(text);
	
}

void notification_check() {
	
	g_notifications.erase(std::remove_if(g_notifications.begin(), g_notifications.end(),
	                                     [](const auto & entry) { return g_gameTime.now() > entry.deadline; }),
	                      g_notifications.end());
	
	if(g_notifications.empty()) {
		return;
	}
	
	pTextManage->Clear();
	
	long igrec = 14;
	
	UseRenderState state(render2D());
	
	for(Notification & notification : g_notifications) {
		
		arx_assert(!notification.text.empty());
		
		Rectf rect(
			Vec2f(120 * g_sizeRatio.x - 16 * minSizeRatio(), igrec),
			16 * minSizeRatio(),
			16 * minSizeRatio()
		);
		
		EERIEDrawBitmap(rect, .00001f, arx_logo_tc, Color::white);
		
		igrec += ARX_UNICODE_DrawTextInRect(hFontInGame, Vec2f(120.f * g_sizeRatio.x, igrec), 500 * g_sizeRatio.x,
		                                    std::string(" ") += getLocalised(notification.text),
		                                    Color::white, nullptr);
		
	}
	
}
