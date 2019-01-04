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

#include "input/TextInput.h"

#include <algorithm>

#include "core/Application.h"
#include "util/Unicode.h"
#include "window/RenderWindow.h"

bool TextInputHandler::keyPressed(Keyboard::Key key, KeyModifiers mod) {
	
	switch(key) {
		
		case Keyboard::Key_0:
		case Keyboard::Key_1:
		case Keyboard::Key_2:
		case Keyboard::Key_3:
		case Keyboard::Key_4:
		case Keyboard::Key_5:
		case Keyboard::Key_6:
		case Keyboard::Key_7:
		case Keyboard::Key_8:
		case Keyboard::Key_9:
		case Keyboard::Key_A:
		case Keyboard::Key_B:
		case Keyboard::Key_C:
		case Keyboard::Key_D:
		case Keyboard::Key_E:
		case Keyboard::Key_F:
		case Keyboard::Key_G:
		case Keyboard::Key_H:
		case Keyboard::Key_I:
		case Keyboard::Key_J:
		case Keyboard::Key_K:
		case Keyboard::Key_L:
		case Keyboard::Key_M:
		case Keyboard::Key_N:
		case Keyboard::Key_O:
		case Keyboard::Key_P:
		case Keyboard::Key_Q:
		case Keyboard::Key_R:
		case Keyboard::Key_S:
		case Keyboard::Key_T:
		case Keyboard::Key_U:
		case Keyboard::Key_V:
		case Keyboard::Key_W:
		case Keyboard::Key_X:
		case Keyboard::Key_Y:
		case Keyboard::Key_Z:
		case Keyboard::Key_NumPadEnter:
		case Keyboard::Key_NumSubtract:
		case Keyboard::Key_NumAdd:
		case Keyboard::Key_NumMultiply:
		case Keyboard::Key_NumDivide:
		case Keyboard::Key_LeftBracket:
		case Keyboard::Key_RightBracket:
		case Keyboard::Key_Spacebar:
		case Keyboard::Key_Slash:
		case Keyboard::Key_Backslash:
		case Keyboard::Key_Comma:
		case Keyboard::Key_Semicolon:
		case Keyboard::Key_Period:
		case Keyboard::Key_Grave:
		case Keyboard::Key_Apostrophe:
		case Keyboard::Key_Minus:
		case Keyboard::Key_Equals:
			return !mod.control;
		
		case Keyboard::Key_NumPad0:
		case Keyboard::Key_NumPad1:
		case Keyboard::Key_NumPad2:
		case Keyboard::Key_NumPad3:
		case Keyboard::Key_NumPad4:
		case Keyboard::Key_NumPad5:
		case Keyboard::Key_NumPad6:
		case Keyboard::Key_NumPad7:
		case Keyboard::Key_NumPad8:
		case Keyboard::Key_NumPad9:
		case Keyboard::Key_NumPoint:
			return mod.num && !mod.control;
		
		default: return false;
		
	}
	
}

void BasicTextInput::newText(const std::string & text) {
	if(m_selected) {
		setText(std::string());
	}
	if(!text.empty() || !m_editText.empty()) {
		m_text.insert(m_cursorPos, text);
		m_cursorPos += text.length();
		m_editText.clear();
		m_editCursorPos = 0;
		m_editCursorLength = 0;
		textUpdated();
	}
}

void BasicTextInput::setText(const std::string & text, size_t cursorPos) {
	if(text != m_text) {
		m_text = text;
		m_cursorPos = std::min(cursorPos, m_text.length());
		m_selected = false;
		while(m_cursorPos < m_text.size() && util::UTF8::isContinuationByte(m_text[m_cursorPos])) {
			m_cursorPos++;
		}
		textUpdated();
	} else {
		setCursorPos(cursorPos);
	}
}

void BasicTextInput::setCursorPos(size_t cursorPos) {
	m_selected = false;
	if(cursorPos != m_cursorPos) {
		m_cursorPos = std::min(cursorPos, m_text.length());
		while(m_cursorPos < m_text.size() && util::UTF8::isContinuationByte(m_text[m_cursorPos])) {
			m_cursorPos++;
		}
		cursorUpdated();
	}
}

void BasicTextInput::insert(const std::string & text) {
	if(m_selected) {
		setText(std::string());
	}
	if(!text.empty()) {
		m_text.insert(m_cursorPos, text);
		m_cursorPos += text.length();
		textUpdated();
	}
}

void BasicTextInput::editingText(const std::string & composition, size_t cursorStart, size_t cursorLength) {
	if(composition != m_editText || cursorStart != m_editCursorPos || cursorLength != m_editCursorLength) {
		arx_assert(cursorStart <= composition.size());
		arx_assert(cursorStart + cursorLength <= composition.size());
		m_editText = composition;
		m_editCursorPos = cursorStart;
		m_editCursorLength = cursorLength;
		editUpdated();
	}
}

void BasicTextInput::clear() {
	if(!m_text.empty()) {
		m_text.clear();
		m_cursorPos = 0;
		textUpdated();
	}
}

void BasicTextInput::moveLeft() {
	if(m_cursorPos > 0) {
		while(m_cursorPos > 0 && util::UTF8::isContinuationByte(m_text[m_cursorPos - 1])) {
			m_cursorPos--;
		}
		m_cursorPos--;
		cursorUpdated();
	}
}

void BasicTextInput::moveRight() {
	m_selected = false;
	if(m_cursorPos < m_text.size()) {
		m_cursorPos++;
		while(m_cursorPos < m_text.size() && util::UTF8::isContinuationByte(m_text[m_cursorPos])) {
			m_cursorPos++;
		}
		cursorUpdated();
	}
}

void BasicTextInput::eraseLeft() {
	if(m_selected) {
		setText(std::string());
	}
	if(m_cursorPos > 0) {
		size_t count = 1;
		while(m_cursorPos > 0 && util::UTF8::isContinuationByte(m_text[m_cursorPos - 1])) {
			count++;
			m_cursorPos--;
		}
		m_text.erase(m_cursorPos - 1, count);
		m_cursorPos--;
		textUpdated();
	}
}

void BasicTextInput::eraseRight() {
	if(m_selected) {
		setText(std::string());
	}
	if(m_cursorPos < m_text.size()) {
		size_t end = m_cursorPos + 1;
		while(end < m_text.size() && util::UTF8::isContinuationByte(m_text[end])) {
			end++;
		}
		m_text.erase(m_cursorPos, end - m_cursorPos);
		textUpdated();
	}
}

bool BasicTextInput::isWordSeparator(char c) {
	switch(c) {
		case '\t': return true;
		case ' ':  return true;
		case ',':  return true;
		case ';':  return true;
		case ':':  return true;
		case '.':  return true;
		case '!':  return true;
		case '+':  return true;
		case '<':  return true;
		case '>':  return true;
		case '(':  return true;
		case ')':  return true;
		default:   return false;
	}
}

size_t BasicTextInput::findWordLeft() const {
	size_t newPos = m_cursorPos;
	while(newPos > 0 && isWordSeparator(m_text[newPos - 1])) {
		newPos--;
	}
	while(newPos > 0 && !isWordSeparator(m_text[newPos - 1])) {
		newPos--;
	}
	return newPos;
}

size_t BasicTextInput::findWordRight() const {
	size_t newPos = m_cursorPos;
	while(newPos < m_text.size() && isWordSeparator(m_text[newPos])) {
		newPos++;
	}
	while(newPos < m_text.size() && !isWordSeparator(m_text[newPos])) {
		newPos++;
	}
	return newPos;
}

void BasicTextInput::moveWordLeft() {
	m_selected = false;
	if(m_cursorPos > 0) {
		m_cursorPos = findWordLeft();
		cursorUpdated();
	}
}

void BasicTextInput::moveWordRight() {
	m_selected = false;
	if(m_cursorPos < m_text.size()) {
		m_cursorPos = findWordRight();
		cursorUpdated();
	}
}

void BasicTextInput::eraseWordLeft() {
	if(m_selected) {
		setText(std::string());
	}
	if(m_cursorPos > 0) {
		size_t start = findWordLeft();
		m_text.erase(start, m_cursorPos - start);
		m_cursorPos = start;
		textUpdated();
	}
}

void BasicTextInput::eraseWordRight() {
	if(m_selected) {
		setText(std::string());
	}
	if(m_cursorPos < m_text.size()) {
		size_t end = findWordRight();
		m_text.erase(m_cursorPos, end - m_cursorPos);
		textUpdated();
	}
}

void BasicTextInput::moveStart() {
	m_selected = false;
	if(m_cursorPos > 0) {
		m_cursorPos = 0;
		cursorUpdated();
	}
}

void BasicTextInput::moveEnd() {
	m_selected = false;
	if(m_cursorPos < m_text.size()) {
		m_cursorPos = m_text.size();
		cursorUpdated();
	}
}

void BasicTextInput::eraseStart() {
	if(m_selected) {
		setText(std::string());
	}
	if(m_cursorPos > 0) {
		m_text.erase(0, m_cursorPos);
		m_cursorPos = 0;
		textUpdated();
	}
}

void BasicTextInput::eraseEnd() {
	if(m_selected) {
		setText(std::string());
	}
	if(m_cursorPos < m_text.size()) {
		m_text.erase(m_cursorPos);
		textUpdated();
	}
}

void BasicTextInput::paste(const std::string & text) {
	std::string filtered = text;
	std::replace(filtered.begin(), filtered.end(), '\n', ' ');
	std::replace(filtered.begin(), filtered.end(), '\t', ' ');
	insert(filtered);
}

bool BasicTextInput::keyPressed(Keyboard::Key key, KeyModifiers mod) {
	
	switch(key) {
		
		case Keyboard::Key_NumPad4: {
			if(mod.num) {
				break;
			}
		} /* fall-through */
		case Keyboard::Key_LeftArrow: {
			if(mod.shift) {
				moveStart();
			} else if(mod.control) {
				moveWordLeft();
			} else {
				moveLeft();
			}
			return true;
		}
		
		case Keyboard::Key_NumPad6: {
			if(mod.num) {
				break;
			}
		} /* fall-through */
		case Keyboard::Key_RightArrow: {
			if(mod.shift) {
				moveEnd();
			} else if(mod.control) {
				moveWordRight();
			} else {
				moveRight();
			}
			return true;
		}
		
		case Keyboard::Key_NumPad7: {
			if(mod.num) {
				break;
			}
		} /* fall-through */
		case Keyboard::Key_Home: {
			moveStart();
			return true;
		}
		
		case Keyboard::Key_NumPad1: {
			if(mod.num) {
				break;
			}
		} /* fall-through */
		case Keyboard::Key_End: {
			moveEnd();
			return true;
		}
		
		case Keyboard::Key_Backspace: {
			if(mod.control && mod.shift) {
				eraseStart();
			} else if(mod.control) {
				eraseWordLeft();
			} else {
				eraseLeft();
			}
			return true;
		}
		
		case Keyboard::Key_NumPoint: {
			if(mod.num) {
				break;
			}
		} /* fall-through */
		case Keyboard::Key_Delete: {
			if(mod.control && mod.shift) {
				eraseEnd();
			} else if(mod.control) {
				eraseWordRight();
			} else {
				eraseRight();
			}
			return true;
		}
		
		case Keyboard::Key_Insert: {
			if(mod.shift) {
				paste(mainApp->getWindow()->getClipboardText());
				return true;
			} else if(mod.control) {
				mainApp->getWindow()->setClipboardText(text());
				return true;
			}
		}
		
		default: break;
	}
	
	if(mod.control) {
		switch(key) {
			
			case Keyboard::Key_A: {
				moveStart();
				return true;
			}
			
			case Keyboard::Key_B: {
				moveLeft();
				return true;
			}
			
			case Keyboard::Key_C: {
				mainApp->getWindow()->setClipboardText(text());
				return true;
			}
			
			case Keyboard::Key_D: {
				eraseRight();
				return true;
			}
			
			case Keyboard::Key_E: {
				moveEnd();
				return true;
			}
			
			case Keyboard::Key_F: {
				moveRight();
				return true;
			}
			
			case Keyboard::Key_H: {
				eraseLeft();
				return true;
			}
			
			case Keyboard::Key_K: {
				eraseEnd();
				return true;
			}
			
			case Keyboard::Key_U: {
				eraseStart();
				return true;
			}
			
			case Keyboard::Key_V: {
				paste(mainApp->getWindow()->getClipboardText());
				return true;
			}
			
			case Keyboard::Key_W: {
				eraseWordLeft();
				return true;
			}
			
			default: break;
		}
	}
	
	return TextInputHandler::keyPressed(key, mod);
}

void BasicTextInput::droppedText(const std::string & text) {
	paste(text);
}

void BasicTextInput::selectAll() {
	
	setCursorPos();
	
	m_selected = true;
	
}
