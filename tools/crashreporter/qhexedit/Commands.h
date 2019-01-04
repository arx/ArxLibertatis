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

#ifndef ARX_TOOLS_CRASHREPORTER_QHEXEDIT_COMMANDS_H
#define ARX_TOOLS_CRASHREPORTER_QHEXEDIT_COMMANDS_H

//! \cond docNever

#include <QUndoCommand>

#include "crashreporter/qhexedit/XByteArray.h"

/*!
 * CharCommand is a class to prived undo/redo functionality in QHexEdit.
 * A QUndoCommand represents a single editing action on a document. CharCommand
 * is responsable for manipulations on single chars. It can insert. replace and
 * remove characters. A manipulation stores allways to actions
 * 1. redo (or do) action
 * 2. undo action.
 *
 * CharCommand also supports command compression via mergeWidht(). This allows
 * the user to execute a undo command contation e.g. 3 steps in a single command.
 * If you for example insert a new byt "34" this means for the editor doing 3
 * steps: insert a "00", replace it with "03" and the replace it with "34". These
 * 3 steps are combined into a single step, insert a "34".
 */
class CharCommand : public QUndoCommand {
	
public:
	
	enum { Id = 1234 };
	enum Cmd { Insert, Remove, Replace };
	
	CharCommand(XByteArray * xData, Cmd cmd, int charPos, char newChar,
	            QUndoCommand * parent = 0);
	
	void undo();
	void redo();
	bool mergeWith(const QUndoCommand * command);
	int id() const { return Id; }
	
private:
	
	XByteArray * _xData;
	int _charPos;
	bool _wasChanged;
	char _newChar;
	char _oldChar;
	Cmd _cmd;
	
};

/*!
 * ArrayCommand provides undo/redo functionality for handling binary strings. It
 * can undo/redo insert, replace and remove binary strins (QByteArrays).
 */
class ArrayCommand : public QUndoCommand {
	
public:
	
	enum Cmd { Insert, Remove, Replace };
	
	ArrayCommand(XByteArray * xData, Cmd cmd, int baPos, const QByteArray & newBa = QByteArray(),
	             int len = 0, QUndoCommand * parent = 0);
	void undo();
	void redo();
	
private:
	
	Cmd _cmd;
	XByteArray * _xData;
	int _baPos;
	int _len;
	QByteArray _wasChanged;
	QByteArray _newBa;
	QByteArray _oldBa;
	
};

//! \endcond docNever

#endif // ARX_TOOLS_CRASHREPORTER_QHEXEDIT_COMMANDS_H
