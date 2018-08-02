/*
 * Copyright 2018 Arx Libertatis Team (see the AUTHORS file)
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

#include "core/Core.h"
#include "core/GameTime.h"
#include "graphics/Draw.h"
#include "graphics/Renderer.h"
#include "graphics/font/Font.h"
#include "gui/Text.h"
#include "gui/TextManager.h"
#include "io/log/Logger.h"
#include "math/Rectangle.h"

struct Notification {
	
	GameInstant timecreation;
	GameDuration duration;
	std::string text;
	
	void clear() {
		timecreation = 0;
		duration = 0;
		text.clear();
	}
	
};

static const size_t MAX_SPEECH = 9;
Notification g_notification[MAX_SPEECH];

extern TextureContainer * arx_logo_tc;

void notification_init() {

	for(size_t i = 0 ; i < MAX_SPEECH ; i++ )
		g_notification[i].clear();
}

static void ARX_SPEECH_MoveUp() {
	
	if(g_notification[0].timecreation != 0) {
		g_notification[0].text.clear();
	}
	
	for(size_t j = 0; j < MAX_SPEECH - 1; j++) {
		g_notification[j] = g_notification[j + 1];
	}
	
	g_notification[MAX_SPEECH - 1].clear();
	
}

void notification_ClearAll()
{
	for(size_t i = 0; i < MAX_SPEECH; i++) {
		
		if(g_notification[i].timecreation == 0) {
			continue;
		}
		
		g_notification[i].clear();
	}
}

void notification_add(const std::string & text) {
	
	if(text.empty())
		return;
	
	GameInstant now = std::max(g_gameTime.now(), GameInstantMs(1));
	
	if(g_notification[MAX_SPEECH - 1].timecreation != 0) {
		ARX_SPEECH_MoveUp();
	}
	
	for(size_t i = 0; i < MAX_SPEECH; i++) {
		
		if(g_notification[i].timecreation != 0) {
			continue;
		}
		
		// Sets creation time
		g_notification[i].timecreation = now;
		g_notification[i].duration = GameDurationMs(2000 + text.length() * 60);
		g_notification[i].text = text;
		return;
	}
	
	LogInfo << "Failed to add speech: " << text;
}

static bool isLastSpeech(size_t index) {
	
	for(size_t i = index + 1; i < MAX_SPEECH; i++) {
		
		if(g_notification[i].timecreation == 0) {
			continue;
		}
		
		if(!g_notification[i].text.empty())
			return false;
	}
	
	return true;
}

static void ARX_SPEECH_Render() {
	
	long igrec = 14;
	
	Vec2i sSize = hFontInGame->getTextSize("p");
	sSize.y *= 3;
	
	int iEnd = igrec + sSize.y;
	
	UseRenderState state(render2D());
	
	for(size_t i = 0; i < MAX_SPEECH; i++) {
		
		if(g_notification[i].timecreation == 0 || g_notification[i].text.empty()) {
			continue;
		}
		
		Rectf rect(
			Vec2f(120 * g_sizeRatio.x - 16 * minSizeRatio(), igrec),
			16 * minSizeRatio(),
			16 * minSizeRatio()
		);
		
		EERIEDrawBitmap(rect, .00001f, arx_logo_tc, Color::white);
		
		igrec += ARX_UNICODE_DrawTextInRect(hFontInGame, Vec2f(120.f * g_sizeRatio.x, igrec), 500 * g_sizeRatio.x,
		                           ' ' + g_notification[i].text, Color::white, NULL);
		
		if(igrec > iEnd && !isLastSpeech(i)) {
			ARX_SPEECH_MoveUp();
			break;
		}
	}
	
}

void notification_check()
{
	bool bClear = false;
	long exist = 0;

	for(size_t i = 0; i < MAX_SPEECH; i++) {
		
		if(g_notification[i].timecreation == 0) {
			continue;
		}
		
		GameDuration elapsed = g_gameTime.now() - g_notification[i].timecreation;
		if(elapsed > g_notification[i].duration) {
			ARX_SPEECH_MoveUp();
			i--;
		} else {
			exist++;
		}

		bClear = true;
	}

	if(bClear && pTextManage) {
		pTextManage->Clear();
	}

	if(exist)
		ARX_SPEECH_Render();
}
