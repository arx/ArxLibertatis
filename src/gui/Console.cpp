/*
 * Copyright 2016 Arx Libertatis Team (see the AUTHORS file)
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

#include "gui/Console.h"

#include <algorithm>
#include <sstream>

#include "core/Config.h"
#include "core/Core.h"
#include "core/GameTime.h"
#include "game/Entity.h"
#include "game/EntityManager.h"
#include "graphics/Draw.h"
#include "graphics/DrawLine.h"
#include "graphics/font/Font.h"
#include "gui/Interface.h"
#include "gui/Text.h"
#include "input/Input.h"
#include "io/log/Logger.h"
#include "math/Rectangle.h"
#include "script/ScriptEvent.h"
#include "util/Unicode.h"

// TODO Share some of this with the save name entry field

void ConsoleBuffer::append(const std::string & text) {
	
	std::string::const_iterator i = text.begin();
	std::string::const_iterator end = text.end();
	
	while(i != end) {
		
		// Remember newline but don't apply until there is content
		if(m_pos < m_width + 1 && *i == '\n') {
			// Delay newline
			m_pos = m_width + 1;
			++i;
		}
		
		if(i == end) {
			break;
		}
		
		// Apply delayed newline
		if(m_pos >= m_width) {
			m_newline = false;
			m_line = (m_line + 1) % m_lines.size();
			m_lines[m_line].clear();
			m_pos = 0;
		}
		
		// Find the next line break
		std::string::const_iterator line_end = i;
		size_t length = 0;
		while(line_end != end && *line_end != '\n'
		      && length < m_width - m_pos) {
			line_end = util::UTF8::next(line_end, end);
			++length;
		}
		
		m_lines[m_line].append(i, line_end);
		m_pos += length;
		i = line_end;
		
	}
	
}

void MemoryLogger::log(const logger::Source & file, int line, Logger::LogLevel level, const std::string & str) {
	std::ostringstream oss;
	if(level == Logger::Console) {
		oss << str << '\n';
	} else {
		format(oss, file, line, level, str);
	}
	m_buffer->append(oss.str());
	// TODO This might need additional locking as other threas may log while the main thread has the console open
}

bool ScriptConsole::keyPressed(Keyboard::Key key, KeyModifiers mod) {
	
	switch(key) {
		
		case Keyboard::Key_Escape: {
			close();
			return true;
		}
		
		case Keyboard::Key_Enter: {
			execute();
			clear();
			return true;
		}
		
		case Keyboard::Key_LeftCtrl:
		case Keyboard::Key_RightCtrl: {
			// We use control as a modifier, disable other uses while the console is open
			return true;
		}
		
		default: break;
	}
	
	if(mod.control) {
		switch(key) {
			
			case Keyboard::Key_J: {
				execute();
				clear();
				return true;
			}
			
			case Keyboard::Key_O: {
				execute();
				moveEnd();
				return true;
			}
			
			default: break;
		}
	}
	
	return BasicTextInput::keyPressed(key, mod);
}

void ScriptConsole::open() {
	if(!m_enabled) {
		config.input.allowConsole = true;
		m_enabled = true;
	}
}

void ScriptConsole::close() {
	if(m_enabled) {
		GInput->stopTextInput();
		m_enabled = false;
	}
}

void ScriptConsole::execute() {
	
	ARX_LOG(Logger::Console) << "> " << text();
	
	Entity * entity = entities.player();
	
	std::string script = command() + "\naccept\n";
	EERIE_SCRIPT es;
	es.size = script.size();
	es.data = const_cast<char *>(script.c_str());
	es.master = entity ? &entity->script : NULL;
	// TODO Some script commands (timers, etc.) store references to the script
	
	long pos = 0;
	ScriptEvent::send(&es, SM_EXECUTELINE, std::string(), entity, std::string(), pos);
}

void ScriptConsole::update() {
	
	if(!m_enabled) {
		if(config.input.allowConsole && GInput->actionNowPressed(CONTROLS_CUST_CONSOLE)) {
			open();
		} else {
			return;
		}
	}
	
	s32 lineHeight = hFontDebug->getLineHeight();
	Rect box = g_size;
	box.top += lineHeight * s32(m_buffer.lines());
	box.bottom = box.top + lineHeight;
	box.left += hFontDebug->getTextSize("> ").advance();
	GInput->startTextInput(box, this);
	
	{
		static const PlatformDuration BlinkDuration = PlatformDurationMs(600);
		m_blinkTime += g_platformTime.lastFrameDuration();
		if(m_blinkTime > (BlinkDuration + BlinkDuration))
			m_blinkTime = PlatformDuration_ZERO;
		m_blink = m_blinkTime > BlinkDuration;
	}
	
}

void ScriptConsole::draw() {
	
	if(!m_enabled) {
		return;
	}
	
	UseRenderState state(render2D());
	
	Color background = Color::black;
	background.a = 150;
	Color line = Color::gray(0.8f);
	Color selection = Color::yellow;
	selection.a = 40;
	
	Rectf box = Rectf(g_size);
	box.bottom = box.top + (m_buffer.lines() + 1) * hFontDebug->getLineHeight() + 4;
	EERIEDrawBitmap(box, 0.f, NULL, background);
	
	Vec2i pos = Vec2i_ZERO;
	for(size_t i = 0; i < m_buffer.lines(); i++) {
		hFontDebug->draw(pos, m_buffer.line(i), Color::white);
		pos.y += hFontDebug->getLineHeight();
	}
	
	pos.y += 1;
	
	drawLine(Vec2f(0, pos.y), Vec2f(g_size.width(), pos.y), 0.f, line);
	
	pos.y += 2;
	
	pos.x += hFontDebug->draw(pos, "> ", Color::green).advance();
	
	std::string displayText = text();
	if(!editText().empty()) {
		displayText = displayText.substr(0, cursorPos()) + editText() + displayText.substr(cursorPos());
	}
	size_t displayCursorPos = cursorPos() + editCursorPos();
	
	std::string::const_iterator begin = displayText.begin();
	std::string::const_iterator end = displayText.end();
	
	// Highlight edit area
	if(!editText().empty()) {
		int left = hFontDebug->getTextSize(begin, begin + cursorPos()).advance();
		int right = hFontDebug->getTextSize(begin, begin + cursorPos() + editText().size()).advance();
		int height = hFontDebug->getLineHeight();
		Rectf box = Rectf(pos + Vec2i(left, 0), right - left, height);
		EERIEDrawBitmap(box, 0.f, NULL, selection);
	}
	
	// Draw text
	s32 x = hFontDebug->draw(pos.x, pos.y, begin, end, Color::white).next();
	
	// Draw cursor
	if(m_blink) {
		int cursor = x;
		if(cursorPos() != displayText.size()) {
			cursor = pos.x + hFontDebug->getTextSize(begin, begin + displayCursorPos).next();
		}
		drawLine(Vec2f(cursor, pos.y), Vec2f(cursor, pos.y + hFontDebug->getLineHeight()), 0.f, line);
		if(editCursorLength() > 0) {
			int end = pos.x + hFontDebug->getTextSize(begin, begin + displayCursorPos + editCursorLength()).next();
			drawLine(Vec2f(end, pos.y), Vec2f(end, pos.y + hFontDebug->getLineHeight()), 0.f, line);
		}
	}
	
	pos.y += hFontDebug->getLineHeight();
	
	pos.y += 1;
	
	drawLine(Vec2f(0, pos.y), Vec2f(g_size.width(), pos.y), 0.f, line);
	
}

ScriptConsole g_console;
