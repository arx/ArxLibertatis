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

#ifndef ARX_GUI_CONSOLE_H
#define ARX_GUI_CONSOLE_H

#include <vector>
#include <string>
#include <utility>
#include <stddef.h>

#include "core/TimeTypes.h"
#include "input/TextInput.h"
#include "io/log/LogBackend.h"

class Entity;

class ConsoleBuffer {
	
	std::vector<std::string> m_lines;
	size_t m_width;
	size_t m_line;
	size_t m_pos;
	bool m_newline;
	
public:
	
	ConsoleBuffer(size_t lines, size_t width)
		: m_lines(lines)
		, m_width(width)
		, m_line(0)
		, m_pos(0)
		, m_newline(false)
	{ }
	
	size_t lines() const { return m_lines.size(); }
	
	std::string & line(size_t index) {
		return  m_lines[(m_line + 1 + index) % m_lines.size()];
	}
	
	const std::string & line(size_t index) const {
		return  m_lines[(m_line + 1 + index) % m_lines.size()];
	}
	
	void append(const std::string & text);
	
};

class MemoryLogger : public logger::Backend {
	
	ConsoleBuffer * m_buffer;
	
public:
	
	explicit MemoryLogger(ConsoleBuffer * buffer) : m_buffer(buffer) { }
	
	void log(const logger::Source & file, int line, Logger::LogLevel level, const std::string & str);
	
};

class ScriptConsole : protected BasicTextInput {
	typedef BasicTextInput  Base;
	
	static const size_t MaxSuggestions = 100;
	static const size_t MaxVisibleSuggestions = 20;
	static const size_t ScrollbackLines = 10;
	static const size_t ScrollbackColumns = 100;
	
	bool m_enabled;
	
	typedef std::pair<size_t, std::string> Suggestion;
	
	ConsoleBuffer m_buffer;
	bool m_updateSuggestions;
	std::string m_originalText;
	std::vector<Suggestion> m_suggestions;
	size_t m_originalCursorPos;
	Suggestion m_completion;
	Suggestion m_error;
	int m_selection;
	PlatformDuration m_blinkTime;
	bool m_blink;
	
	size_t m_contextBegin;
	size_t m_contextEnd;
	size_t m_commandBegin;
	size_t m_suggestionPos;
	
	bool keyPressed(Keyboard::Key key, KeyModifiers mod);
	void textUpdated();
	void cursorUpdated();
	
	void select(int dir);
	void autocomplete(size_t characters = size_t(-1));
	void applySuggestion(const Suggestion & suggestion);
	
	static bool addContextSuggestion(void * self, const std::string & suggestion);
	static bool addCommandSuggestion(void * self, const std::string & suggestion);
	
public:
	
	ScriptConsole()
		: m_enabled(false)
		, m_buffer(ScrollbackLines, ScrollbackColumns)
		, m_updateSuggestions(true)
		, m_originalCursorPos(true)
		, m_selection(0)
		, m_blinkTime(0)
		, m_blink(true)
		, m_contextBegin(0)
		, m_contextEnd(0)
		, m_commandBegin(0)
		, m_suggestionPos(0)
	{ }
	
	void open();
	void close();
	
	std::string context() { return text().substr(m_contextBegin, m_contextEnd - m_contextBegin); }
	std::string command() { return text().substr(m_commandBegin); }
	Entity * contextEntity();
	
	void execute();
	
	void update();
	void draw();
	
	ConsoleBuffer & buffer() { return m_buffer; }
	
};

extern ScriptConsole g_console;

#endif // ARX_GUI_CONSOLE_H
