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
		
		case Keyboard::Key_Tab: {
			autocomplete();
			return true;
		}
		
		case Keyboard::Key_RightArrow: {
			if(cursorPos() == text().size()) {
				autocomplete(1);
				return true;
			}
			break;
		}
		
		case Keyboard::Key_PageUp:
		case Keyboard::Key_UpArrow: {
			select(-1);
			return true;
		}
		
		case Keyboard::Key_PageDown:
		case Keyboard::Key_DownArrow: {
			select(1);
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
			
			case Keyboard::Key_F: {
				if(cursorPos() == text().size()) {
					autocomplete(1);
					return true;
				}
				break;
			}
			
			case Keyboard::Key_I: {
				autocomplete();
				return true;
			}
			
			case Keyboard::Key_J: {
				execute();
				clear();
				return true;
			}
			
			case Keyboard::Key_N: {
				select(1);
				return true;
			}
			
			case Keyboard::Key_O: {
				execute();
				moveEnd();
				return true;
			}
			
			case Keyboard::Key_P: {
				select(-1);
				return true;
			}
			
			default: break;
		}
	}
	
	return BasicTextInput::keyPressed(key, mod);
}

static bool isScriptContextChar(char c) {
	return isalnum(c) || c == '_';
}

void ScriptConsole::textUpdated() {
	
	m_contextBegin = 0;
	while(m_contextBegin < text().size() && isspace(text()[m_contextBegin])) {
		++m_contextBegin;
	}
	m_contextEnd = m_contextBegin;
	while(m_contextEnd < text().size() && isScriptContextChar(text()[m_contextEnd])) {
		++m_contextEnd;
	}
	
	size_t dot = m_contextEnd;
	while(dot < text().size() && isspace(text()[dot])) {
		++dot;
	}
	
	m_commandBegin = m_contextBegin;
	bool hasContext = false;
	if(dot < text().size() && text()[dot] == '.') {
		hasContext = true;
		m_commandBegin = dot + 1;
		while(m_commandBegin < text().size() && isspace(text()[m_commandBegin])) {
			++m_commandBegin;
		}
	}
	size_t commandEnd = m_commandBegin;
	while(commandEnd < text().size() && !isspace(text()[commandEnd])) {
		++commandEnd;
	}
	
	m_completion = Suggestion(cursorPos(), std::string());
	m_error = Suggestion(0, std::string());
	
	if(m_updateSuggestions) {
		
		m_suggestions.clear();
		m_selection = 0;
		m_originalText = text();
		m_originalCursorPos = cursorPos();
		
		if(cursorPos() > m_contextBegin && cursorPos() <= m_contextEnd) {
			m_suggestionPos = m_contextBegin;
			std::string context = text().substr(m_contextBegin, cursorPos() - m_contextBegin);
			entities.autocomplete(context, addContextSuggestion, this);
		} else if(hasContext && cursorPos() > m_contextBegin) {
			if(contextEntity() == NULL) {
				m_error = Suggestion(m_contextBegin, "^ Unknown entity");
			}
		}
		
		if(!m_error.second.empty()) {
			// Error - no need provide suggestions
		} else if(cursorPos() > m_commandBegin && cursorPos() <= commandEnd) {
			m_suggestionPos = m_commandBegin;
			std::string command = text().substr(m_commandBegin, cursorPos() - m_commandBegin);
			ScriptEvent::autocomplete(command, addCommandSuggestion, this);
		} else if(cursorPos() > m_contextBegin && cursorPos() <= m_contextEnd) {
			m_suggestionPos = m_contextBegin;
			std::string command = text().substr(m_contextBegin, cursorPos() - m_contextBegin);
			ScriptEvent::autocomplete(command, addCommandSuggestion, this);
		} else if(cursorPos() > m_commandBegin) {
			if(!ScriptEvent::isCommand(text().substr(m_commandBegin, commandEnd - m_commandBegin))) {
				m_error = Suggestion(m_commandBegin, "^ Unknown command");
			}
		}
		
		if(m_suggestions.size() > MaxSuggestions) {
			if(m_error.second.empty()) {
				m_error = Suggestion(m_suggestions[0].first, "...");
			}
			m_suggestions.clear();
		}
		
		if(!m_suggestions.empty()) {
			size_t commonPrefix = m_suggestions[0].second.size();
			for(size_t i = 1; i < m_suggestions.size(); i++) {
				if(m_suggestions[i].first != m_suggestions[0].first) {
					commonPrefix = 0;
					break;
				}
				size_t max = std::min(commonPrefix, m_suggestions[i].second.size());
				commonPrefix = 0;
				for(size_t j = 0; j < max && m_suggestions[i].second[j] == m_suggestions[1].second[j]; j++) {
					commonPrefix++;
				}
				if(commonPrefix == 0) {
					break;
				}
			}
			if(commonPrefix != 0) {
				m_completion = m_suggestions[0];
				m_completion.second.resize(commonPrefix);
			}
		}
		
	}
	
	if(!hasContext) {
		m_contextEnd = m_contextBegin;
	}
	
}

void ScriptConsole::cursorUpdated() {
	textUpdated();
}

void ScriptConsole::open() {
	if(!m_enabled) {
		config.input.allowConsole = true;
		m_enabled = true;
		textUpdated();
	}
}

void ScriptConsole::close() {
	if(m_enabled) {
		GInput->stopTextInput();
		m_enabled = false;
	}
}

Entity * ScriptConsole::contextEntity() {
	
	Entity * entity = entities.player();
	std::string id = context();
	if(!id.empty()) {
		entity = entities.getById(id, entity);
	}
	
	return entity;
}

void ScriptConsole::execute() {
	
	ARX_LOG(Logger::Console) << "> " << text();
	
	Entity * entity = contextEntity();
	if(!entity) {
		LogError << "Unknown entity: " + context();
		return;
	}
	
	std::string script = command() + "\naccept\n";
	EERIE_SCRIPT es;
	es.size = script.size();
	es.data = const_cast<char *>(script.c_str());
	es.master = entity ? &entity->script : NULL;
	// TODO Some script commands (timers, etc.) store references to the script
	
	// TODO Allow the "context.command" syntax in scripts too
	long pos = 0;
	ScriptEvent::send(&es, SM_EXECUTELINE, std::string(), entity, std::string(), pos);
}

bool ScriptConsole::addContextSuggestion(void * self, const std::string & suggestion) {
	ScriptConsole * console = static_cast<ScriptConsole *>(self);
	console->m_suggestions.push_back(Suggestion(console->m_suggestionPos, suggestion + "."));
	return (console->m_suggestions.size() <= MaxSuggestions);
}

bool ScriptConsole::addCommandSuggestion(void * self, const std::string & suggestion) {
	ScriptConsole * console = static_cast<ScriptConsole *>(self);
	console->m_suggestions.push_back(Suggestion(console->m_suggestionPos, suggestion));
	return (console->m_suggestions.size() <= MaxSuggestions);
}

void ScriptConsole::select(int dir) {
	
	int selection = glm::clamp(m_selection + dir, 0, int(m_suggestions.size()));
	
	if(selection == m_selection) {
		return;
	}
	m_selection = selection;
	
	if(m_selection == 0) {
		setText(m_originalText, m_originalCursorPos);
	} else {
		arx_assert(m_selection > 0);
		arx_assert(m_selection <= int(m_suggestions.size()));
		m_updateSuggestions = false;
		applySuggestion(m_suggestions[m_selection - 1]);
		m_updateSuggestions = true;
	}
	
}

void ScriptConsole::autocomplete(size_t characters) {
	
	arx_assert(m_completion.first <= m_originalCursorPos);
	size_t n = m_originalCursorPos - m_completion.first;
	for(; n < m_completion.second.size(); n++) {
		if(!util::UTF8::isContinuationByte(m_completion.second[n])) {
			if(characters == 0) {
				break;
			} else {
				characters--;
			}
		}
	}
	
	if(n != m_originalCursorPos - m_completion.first) {
		m_updateSuggestions = false;
		applySuggestion(Suggestion(m_completion.first, m_completion.second.substr(0, n)));
		m_updateSuggestions = true;
	}
	
	textUpdated();
}

void ScriptConsole::applySuggestion(const Suggestion & suggestion) {
	
	arx_assert(suggestion.first <= m_originalText.size());
	arx_assert(m_originalCursorPos <= m_originalText.size());
	arx_assert(suggestion.first <= m_originalCursorPos);
	
	std::string newText = m_originalText.substr(0, suggestion.first);
	newText += suggestion.second;
	size_t tail = m_originalCursorPos;
	if(!newText.empty() && newText[newText.size() - 1] == '.') {
		while(true) {
			if(tail >= m_originalText.size()) {
				tail = m_originalCursorPos;
				break;
			} else if(m_originalText[tail] == '.') {
				break;
			} else if(!isScriptContextChar(m_originalText[tail])) {
				tail = m_originalCursorPos;
				break;
			}
			tail++;
		}
		if(tail < m_originalText.size() && isWordSeparator(m_originalText[tail])) {
			tail++;
		}
	} else {
		while(tail < m_originalText.size() && !isWordSeparator(m_originalText[tail])) {
			tail++;
		}
		if(!newText.empty() && isWordSeparator(newText[newText.size() - 1]) && tail < m_originalText.size()) {
			tail++;
		}
	}
	newText += m_originalText.substr(tail);
	
	setText(newText, suggestion.first + suggestion.second.size());
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
	
	// Preview autocomplete
	const std::string & completion = m_completion.second;
	if(cursorPos() == text().size() && cursorPos() < completion.size()) {
		hFontDebug->draw(x, pos.y, completion.begin() + cursorPos(), completion.end(), Color::gray(0.5f));
	}
	
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
	
	pos.y += 2;
	
	// Draw error message and suggestions
	if(!m_error.second.empty()) {
		Vec2i errorPos = pos;
		errorPos.x += hFontDebug->getTextSize(text().begin(), text().begin() + m_error.first).advance();
		hFontDebug->draw(errorPos + Vec2i_ONE, m_error.second, Color::black);
		hFontDebug->draw(errorPos, m_error.second, Color::red);
	} else if(!m_suggestions.empty()) {
		Vec2i suggestionPos = pos;
		size_t position = 0;
		for(size_t i = 0; i < m_suggestions.size(); i++) {
			if(m_suggestions[i].first != position) {
				position = m_suggestions[i].first;
				suggestionPos.x = pos.x + hFontDebug->getTextSize(text().begin(), text().begin() + position).advance();
			}
			if(int(i) + 1 == m_selection) {
				int width = hFontDebug->getTextSize(m_suggestions[i].second).width();
				int height = hFontDebug->getLineHeight();
				Rectf highlight(suggestionPos - Vec2i(2, 1), width + 3, height + 2);
				EERIEDrawBitmap(highlight, 0.f, NULL, selection);
				drawLineRectangle(highlight, 0.f, background);
			}
			hFontDebug->draw(suggestionPos + Vec2i_ONE, m_suggestions[i].second, Color::black);
			hFontDebug->draw(suggestionPos, m_suggestions[i].second, Color::white);
			suggestionPos.y += hFontDebug->getLineHeight();
			if(i == MaxVisibleSuggestions) {
				hFontDebug->draw(suggestionPos + Vec2i_ONE, "...", Color::black);
				hFontDebug->draw(suggestionPos, "...", Color::red);
				break;
			}
		}
	}
	
}

ScriptConsole g_console;
