/*
 * Copyright 2011-2017 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_TOOLS_CRASHREPORTER_QHEXEDIT_QHEXEDITPRIVATE_H
#define ARX_TOOLS_CRASHREPORTER_QHEXEDIT_QHEXEDITPRIVATE_H

//! \cond docNever

#include <QtGui>
#include <QScrollArea>
#include <QUndoStack>

#include "crashreporter/qhexedit/XByteArray.h"

class QHexEditPrivate : public QWidget {
	
Q_OBJECT
	
public:
	
	explicit QHexEditPrivate(QScrollArea * parent);
	
	void setAddressAreaColor(QColor const & color);
	QColor addressAreaColor();
	
	void setAddressOffset(int offset);
	int addressOffset();
	
	void setCursorPos(int position);
	int cursorPos();
	
	void setData(QByteArray const & data);
	QByteArray data();
	
	void setHighlightingColor(QColor const & color);
	QColor highlightingColor();
	
	void setOverwriteMode(bool overwriteMode);
	bool overwriteMode();
	
	void setReadOnly(bool readOnly);
	bool isReadOnly();
	
	void setSelectionColor(QColor const & color);
	QColor selectionColor();
	
	XByteArray & xData();
	
	void insert(int index, const QByteArray & ba);
	void insert(int index, char ch);
	void remove(int index, int len = 1);
	void replace(int index, char ch);
	void replace(int index, const QByteArray & ba);
	
	void setAddressArea(bool addressArea);
	void setAddressWidth(int addressWidth);
	void setAsciiArea(bool asciiArea);
	void setHighlighting(bool mode);
	virtual void setFont(const QFont & font);
	
	void undo();
	void redo();
	
	QString toRedableString();
	QString selectionToReadableString();
	
signals:
	
	void currentAddressChanged(int address);
	void currentSizeChanged(int size);
	void dataChanged();
	void overwriteModeChanged(bool state);
	
protected:
	
	void keyPressEvent(QKeyEvent * event);
	void mouseMoveEvent(QMouseEvent * event);
	void mousePressEvent(QMouseEvent * event);
	
	void paintEvent(QPaintEvent * event);
	
	//! calc cursorpos from graphics position. DOES NOT STORE POSITION
	int cursorPos(QPoint pos);
	
	void resetSelection(int pos);
	
	//! set min (if below init) or max (if greater init)
	void setSelection(int pos);
	
	int getSelectionBegin();
	int getSelectionEnd();
	
private slots:
	
	void updateCursor();
	
private:
	
	void adjust();
	
	QColor _addressAreaColor;
	QColor _highlightingColor;
	QColor _selectionColor;
	QScrollArea * _scrollArea;
	QTimer _cursorTimer;
	QUndoStack * _undoStack;
	
	XByteArray _xData;                  //!< holds the content of the hex editor
	
	bool _blink;                        //!< true: then cursor blinks
	bool _addressArea;                  //!< left area of QHexEdit
	bool _asciiArea;                    //!< medium area
	bool _highlighting;                 //!< highlighting of changed bytes
	bool _overwriteMode;
	bool _readOnly;                     //!< true: the user can only look and navigate
	
	int _charWidth, _charHeight;        //!< char dimensions (dpendend on font)
	int _cursorX, _cursorY;             //!< graphics position of the cursor
	int _cursorPosition;                //!< charakter positioin in stream (on byte ends in to steps)
	int _xPosAdr, _xPosHex, _xPosAscii; //!< graphics x-position of the areas
	
	int _selectionBegin;                //!< First selected char
	int _selectionEnd;                  //!< Last selected char
	int _selectionInit;                 //!< That's, where we pressed the mouse button
	
	int _size;
	
};

//! \endcond docNever

#endif // ARX_TOOLS_CRASHREPORTER_QHEXEDIT_QHEXEDITPRIVATE_H
