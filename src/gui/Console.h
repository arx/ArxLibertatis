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

#ifndef ARX_GUI_CONSOLE_H
#define ARX_GUI_CONSOLE_H

#include <vector>
#include <string>
#include <utility>
#include <deque>
#include <stddef.h>

#include "core/TimeTypes.h"
#include "game/GameTypes.h"
#include "input/TextInput.h"
#include "io/log/LogBackend.h"
#include "platform/Platform.h"

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

class ScriptConsole arx_final : protected BasicTextInput {
	typedef BasicTextInput  Base;
	
	static const size_t MaxSuggestions = 1000;
	static const size_t MaxVisibleSuggestions = 20;
	static const size_t ScrollbackLines = 10;
	static const size_t ScrollbackColumns = 100;
	static const size_t MaxHistorySize = 100;
	
	bool m_enabled;
	bool m_wasPaused;
	
	typedef std::pair<size_t, std::string> Suggestion;
	
	ConsoleBuffer m_buffer;
	bool m_updateSuggestions; //!< Controls whether \ref parse() updates the suggestions/auto-completion
	std::deque<std::string> m_history; //!< History of unique commands, from oldest to most recently used
	std::vector<Suggestion> m_suggestions; //!< Suggestions and corresponding start positions in \ref text()
	std::string m_originalText; //!< \ref text() that was used to generate the suggestions / search the history
	size_t m_originalCursorPos; //!< Cursor position in the original text
	Suggestion m_completion; //!< Tab completion and corresponding start position in \ref text()
	Suggestion m_error; //!< Error message and corresponding position in \ref text()
	int m_selection; //!< The selected suggestion (positive) history item (negative) or current line (0)
	EntityHandle m_lastSelectedEntity;
	
	size_t m_contextBegin; //!< Start of the context entity ID in \ref text()
	size_t m_contextEnd; //!< End of the context entity ID in \ref text()
	size_t m_commandBegin; //!< Start of the command in \ref text()
	size_t m_suggestionPos; //!< Start of the command in \ref text()
	
	bool keyPressed(Keyboard::Key key, KeyModifiers mod);
	void textUpdated();
	void cursorUpdated();
	void paste(const std::string & text);
	
	/*!
	 * Parse the current command and update suggestions
	 */
	void parse(bool allowEmptyPrefix = false);
	
	/*!
	 * Select a history entry (negative), a suggestion (positive) or the current command (0)
	 *
	 * \param dir The direction in which to move the selection. Must be 1 or -1.
	 */
	void select(int dir);
	
	//! Finalize the selection, apply characters from \ref m_completion, or suggest all entities and commands
	void autocomplete(size_t characters = size_t(-1));
	
	//! Edit a suggestion into \ref m_originalText at \ref m_originalCursorPos and use it as the new text
	void applySuggestion(const Suggestion & suggestion);
	
	static bool addContextSuggestion(void * self, const std::string & suggestion);
	static bool addCommandSuggestion(void * self, const std::string & suggestion);
	
public:
	
	ScriptConsole()
		: m_enabled(false)
		, m_wasPaused(false)
		, m_buffer(ScrollbackLines, ScrollbackColumns)
		, m_updateSuggestions(true)
		, m_originalCursorPos(true)
		, m_selection(0)
		, m_lastSelectedEntity(EntityHandle_Player)
		, m_contextBegin(0)
		, m_contextEnd(0)
		, m_commandBegin(0)
		, m_suggestionPos(0)
	{ }
	
	//! Show the console and unlock the hotkey
	void open();
	
	//! Hide the console
	void close();
	
	//! ID of the current context entity or an empty string
	std::string context() { return text().substr(m_contextBegin, m_contextEnd - m_contextBegin); }
	
	//! Current command
	std::string command() { return text().substr(m_commandBegin); }
	
	//! Current context entity
	Entity * contextEntity();
	
	//!< Execute the current command
	void execute();
	
	void update();
	void draw();
	
	//! The console's output buffer
	ConsoleBuffer & buffer() { return m_buffer; }
	
};

extern ScriptConsole g_console;

#endif // ARX_GUI_CONSOLE_H
