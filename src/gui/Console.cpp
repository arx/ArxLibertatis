/*
 * Copyright 2016-2018 Arx Libertatis Team (see the AUTHORS file)
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

#include <boost/algorithm/string/predicate.hpp>

#include "core/Application.h"
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
#include "scene/Interactive.h"
#include "script/ScriptEvent.h"
#include "util/Unicode.h"
#include "window/RenderWindow.h"

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
			if(!mod.alt) {
				execute();
				clear();
				return true;
			}
			break;
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
			
			case Keyboard::Key_C: {
				if(!mod.shift) {
					clear();
					return true;
				}
				break;
			}
			
			case Keyboard::Key_D: {
				if(text().empty()) {
					close();
					return true;
				}
				break;
			}
			
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

void ScriptConsole::parse(bool allowEmptyPrefix) {
	
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
		
		size_t startPos = cursorPos();
		if(allowEmptyPrefix) {
			startPos++;
		}
		
		if(startPos > m_contextBegin && cursorPos() <= m_contextEnd) {
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
		} else if(startPos > m_commandBegin && cursorPos() <= commandEnd) {
			m_suggestionPos = m_commandBegin;
			std::string command = text().substr(m_commandBegin, cursorPos() - m_commandBegin);
			ScriptEvent::autocomplete(command, addCommandSuggestion, this);
		} else if(startPos > m_contextBegin && cursorPos() <= m_contextEnd) {
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
				for(size_t j = 0; j < max && m_suggestions[i].second[j] == m_suggestions[0].second[j]; j++) {
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
		
		if(cursorPos() <= m_contextEnd && ValidIONum(LastSelectedIONum)
		   && LastSelectedIONum != EntityHandle_Player) {
			std::string selected = entities[LastSelectedIONum]->idString();
			if(cursorPos() < m_contextBegin
			   || boost::starts_with(selected, text().substr(m_contextBegin, cursorPos() - m_contextBegin))) {
				m_completion = Suggestion(0, selected + ".");
			}
		}
		
	}
	
	if(!hasContext) {
		m_contextEnd = m_contextBegin;
	}
	
}

void ScriptConsole::textUpdated() {
	parse();
}

void ScriptConsole::cursorUpdated() {
	parse();
}

void ScriptConsole::paste(const std::string & text) {
	
	m_updateSuggestions = false;
	size_t p = 0;
	while(true) {
		size_t newline = text.find('\n', p);
		if(newline == std::string::npos) {
			Base::paste(text.substr(p));
			break;
		}
		Base::paste(text.substr(p, newline - p));
		execute();
		clear();
		p = newline + 1;
	}
	m_updateSuggestions = true;
	
	textUpdated();
}

void ScriptConsole::open() {
	if(!m_enabled) {
		config.input.allowConsole = true;
		m_enabled = true;
		m_wasPaused = (g_gameTime.isPaused() & GameTime::PauseUser) != 0;
		g_gameTime.pause(GameTime::PauseUser);
		textUpdated();
	}
}

void ScriptConsole::close() {
	if(m_enabled) {
		GInput->stopTextInput();
		m_enabled = false;
		if(!m_wasPaused) {
			g_gameTime.resume(GameTime::PauseUser);
		}
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
	
	if(!text().empty()) {
		m_history.erase(std::remove(m_history.begin(), m_history.end(), text()), m_history.end());
		m_history.push_back(text());
		if(m_history.size() > MaxHistorySize) {
			m_history.pop_front();
		}
	}
	
	ARX_LOG(Logger::Console) << "> " << text();
	
	Entity * entity = contextEntity();
	if(!entity) {
		LogError << "Unknown entity: " + context();
		return;
	}
	
	EERIE_SCRIPT es;
	es.valid = true;
	es.data = command() + "\naccept\n";
	// TODO Some script commands (timers, etc.) store references to the script
	
	// TODO Allow the "context.command" syntax in scripts too
	size_t pos = 0;
	ScriptEvent::resume(&es, entity, pos);
	
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
	
	int selection = glm::clamp(m_selection + dir, -int(m_history.size()), int(m_suggestions.size()));
	if(selection < 0) {
		std::string prefix = m_originalText.substr(0, m_originalCursorPos);
		while(true) {
			if(selection == 0) {
				// Reached history start
				break;
			} else if(selection < -int(m_history.size())) {
				// Reached history end
				return;
			} else if(boost::starts_with(m_history[int(m_history.size()) + selection], prefix)) {
				// Found history entry
				break;
			}
			selection += dir;
		}
	}
	
	if(selection == m_selection) {
		return;
	}
	m_selection = selection;
	
	if(m_selection < 0) {
		arx_assert(int(m_history.size()) + m_selection >= 0);
		arx_assert(int(m_history.size()) + m_selection < int(m_history.size()));
		size_t newCursorPos = (m_originalCursorPos == 0) ? size_t(-1) : m_originalCursorPos;
		m_suggestions.clear();
		m_updateSuggestions = false;
		setText(m_history[int(m_history.size()) + m_selection], newCursorPos);
		m_updateSuggestions = true;
	} else if(m_selection == 0) {
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
	
	if(m_selection != 0) {
		// Commit the selection, otherwise we will have no useful tab completion
		textUpdated();
	} else if(m_completion.second.empty() && m_suggestions.empty()) {
		parse(true);
		return;
	}
	
	arx_assert(m_completion.first <= cursorPos());
	size_t n = cursorPos() - m_completion.first;
	for(; n < m_completion.second.size(); n++) {
		if(!util::UTF8::isContinuationByte(m_completion.second[n])) {
			if(characters == 0) {
				break;
			} else {
				characters--;
			}
		}
	}
	
	if(n != cursorPos() - m_completion.first) {
		arx_assert(m_originalCursorPos == cursorPos());
		applySuggestion(Suggestion(m_completion.first, m_completion.second.substr(0, n)));
	}
	
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
	
	// Update suggestion if the selected entity changed
	EntityHandle entity = ValidIONum(LastSelectedIONum) ? LastSelectedIONum : EntityHandle_Player;
	if(m_selection == 0 && entity != m_lastSelectedEntity) {
		m_lastSelectedEntity = entity;
		textUpdated();
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
	
	{
		Rectf box = Rectf(g_size);
		box.bottom = box.top + float((m_buffer.lines() + 1) * hFontDebug->getLineHeight()) + 4.f;
		EERIEDrawBitmap(box, 0.f, NULL, background);
	}
	
	Vec2i pos(0);
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
		Rectf box = Rectf(Rect(pos + Vec2i(left, 0), right - left, height));
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
	bool blink = true;
	if(mainApp->getWindow()->hasFocus()) {
		blink = timeWaveSquare(g_platformTime.frameStart(), PlatformDurationMs(1200));
	}
	if(blink) {
		int cursor = x;
		if(cursorPos() != displayText.size()) {
			cursor = pos.x + hFontDebug->getTextSize(begin, begin + displayCursorPos).next();
		}
		drawLine(Vec2f(cursor, pos.y), Vec2f(cursor, pos.y + hFontDebug->getLineHeight()), 0.f, line);
		if(editCursorLength() > 0) {
			int endX = pos.x + hFontDebug->getTextSize(begin, begin + displayCursorPos + editCursorLength()).next();
			drawLine(Vec2f(endX, pos.y), Vec2f(endX, pos.y + hFontDebug->getLineHeight()), 0.f, line);
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
		hFontDebug->draw(errorPos + Vec2i(1), m_error.second, Color::black);
		hFontDebug->draw(errorPos, m_error.second, Color::red);
	} else if(!m_suggestions.empty()) {
		Vec2i suggestionPos = pos;
		size_t position = 0;
		size_t start = 0;
		if(m_selection >= int(MaxVisibleSuggestions)) {
			start = m_selection - MaxVisibleSuggestions + 1;
		}
		for(size_t i = start; i < m_suggestions.size(); i++) {
			if(i == start + MaxVisibleSuggestions - (start != 0)) {
				std::ostringstream oss;
				oss << "... " << (start + 1) << " - " << (start + MaxVisibleSuggestions - (start != 0))
				    << " / " << m_suggestions.size();
				hFontDebug->draw(suggestionPos + Vec2i(1), oss.str(), Color::black);
				hFontDebug->draw(suggestionPos, oss.str(), Color::red);
				break;
			}
			if(m_suggestions[i].first != position) {
				position = m_suggestions[i].first;
				suggestionPos.x = pos.x + hFontDebug->getTextSize(text().begin(), text().begin() + position).advance();
			}
			if(start != 0 && i == start) {
				hFontDebug->draw(suggestionPos + Vec2i(1), "...", Color::black);
				hFontDebug->draw(suggestionPos, "...", Color::red);
				suggestionPos.y += hFontDebug->getLineHeight();
			}
			if(int(i) + 1 == m_selection) {
				int width = hFontDebug->getTextSize(m_suggestions[i].second).width();
				int height = hFontDebug->getLineHeight();
				Rectf highlight = Rectf(Rect(suggestionPos - Vec2i(2, 1), width + 3, height + 2));
				EERIEDrawBitmap(highlight, 0.f, NULL, selection);
				drawLineRectangle(highlight, 0.f, background);
			}
			hFontDebug->draw(suggestionPos + Vec2i(1), m_suggestions[i].second, Color::black);
			hFontDebug->draw(suggestionPos, m_suggestions[i].second, Color::white);
			suggestionPos.y += hFontDebug->getLineHeight();
		}
	}
	
}

ScriptConsole g_console;
