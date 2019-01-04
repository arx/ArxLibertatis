/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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
/* Based on:
	qhexedit2 by Winfried Simon
	http://code.google.com/p/qhexedit2/
	version 0.6.1
 */

#include "crashreporter/qhexedit/Commands.h"

CharCommand::CharCommand(XByteArray * xData, Cmd cmd, int charPos, char newChar, QUndoCommand * parent)
	: QUndoCommand(parent)
	, _wasChanged()
	, _oldChar()
{
	_xData = xData;
	_charPos = charPos;
	_newChar = newChar;
	_cmd = cmd;
}

bool CharCommand::mergeWith(const QUndoCommand * command) {
	
	const CharCommand * nextCommand = static_cast<const CharCommand *>(command);
	bool result = false;
	
	if(_cmd != Remove && nextCommand->_cmd == Replace && nextCommand->_charPos == _charPos) {
		_newChar = nextCommand->_newChar;
		result = true;
	}
	
	return result;
}

void CharCommand::undo() {
	
	switch(_cmd) {
		case Insert: {
			_xData->remove(_charPos, 1);
			break;
		}
		case Replace: {
			_xData->replace(_charPos, _oldChar);
			_xData->setDataChanged(_charPos, _wasChanged);
			break;
		}
		case Remove: {
			_xData->insert(_charPos, _oldChar);
			_xData->setDataChanged(_charPos, _wasChanged);
			break;
		}
	}
	
}

void CharCommand::redo() {
	
	switch(_cmd) {
		case Insert: {
			_xData->insert(_charPos, _newChar);
			break;
		}
		case Replace: {
			_oldChar = _xData->data()[_charPos];
			_wasChanged = _xData->dataChanged(_charPos);
			_xData->replace(_charPos, _newChar);
			break;
		}
		case Remove: {
			_oldChar = _xData->data()[_charPos];
			_wasChanged = _xData->dataChanged(_charPos);
			_xData->remove(_charPos, 1);
			break;
		}
	}
	
}

ArrayCommand::ArrayCommand(XByteArray * xData, Cmd cmd, int baPos, const QByteArray & newBa, int len, QUndoCommand * parent)
	: QUndoCommand(parent)
{
	_cmd = cmd;
	_xData = xData;
	_baPos = baPos;
	_newBa = newBa;
	_len = len;
}

void ArrayCommand::undo() {
	
	switch(_cmd) {
		case Insert: {
			_xData->remove(_baPos, _newBa.length());
			break;
		}
		case Replace: {
			_xData->replace(_baPos, _oldBa);
			_xData->setDataChanged(_baPos, _wasChanged);
			break;
		}
		case Remove: {
			_xData->insert(_baPos, _oldBa);
			_xData->setDataChanged(_baPos, _wasChanged);
			break;
		}
	}
	
}

void ArrayCommand::redo() {
	
	switch(_cmd) {
		case Insert: {
			_xData->insert(_baPos, _newBa);
			break;
		}
		case Replace: {
			_oldBa = _xData->data().mid(_baPos, _len);
			_wasChanged = _xData->dataChanged(_baPos, _len);
			_xData->replace(_baPos, _newBa);
			break;
		}
		case Remove: {
			_oldBa = _xData->data().mid(_baPos, _len);
			_wasChanged = _xData->dataChanged(_baPos, _len);
			_xData->remove(_baPos, _len);
			break;
		}
	}
	
}
