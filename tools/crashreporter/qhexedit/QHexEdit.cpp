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

#include "crashreporter/qhexedit/QHexEdit.h"

#include <QtGui>

QHexEdit::QHexEdit(QWidget * parent) : QScrollArea(parent) {
	
	qHexEdit_p = new QHexEditPrivate(this);
	setWidget(qHexEdit_p);
	setWidgetResizable(true);
	
	connect(qHexEdit_p, SIGNAL(currentAddressChanged(int)), this, SIGNAL(currentAddressChanged(int)));
	connect(qHexEdit_p, SIGNAL(currentSizeChanged(int)), this, SIGNAL(currentSizeChanged(int)));
	connect(qHexEdit_p, SIGNAL(dataChanged()), this, SIGNAL(dataChanged()));
	connect(qHexEdit_p, SIGNAL(overwriteModeChanged(bool)), this, SIGNAL(overwriteModeChanged(bool)));
	setFocusPolicy(Qt::NoFocus);
	
}

void QHexEdit::insert(int i, const QByteArray & ba) {
	qHexEdit_p->insert(i, ba);
}

void QHexEdit::insert(int i, char ch) {
	qHexEdit_p->insert(i, ch);
}

void QHexEdit::remove(int pos, int len) {
	qHexEdit_p->remove(pos, len);
}

QString QHexEdit::toReadableString() {
	return qHexEdit_p->toRedableString();
}

QString QHexEdit::selectionToReadableString() {
	return qHexEdit_p->selectionToReadableString();
}

void QHexEdit::setAddressArea(bool addressArea) {
	qHexEdit_p->setAddressArea(addressArea);
}

void QHexEdit::redo() {
	qHexEdit_p->redo();
}

void QHexEdit::undo() {
	qHexEdit_p->undo();
}

void QHexEdit::setAddressWidth(int addressWidth) {
	qHexEdit_p->setAddressWidth(addressWidth);
}

void QHexEdit::setAsciiArea(bool asciiArea) {
	qHexEdit_p->setAsciiArea(asciiArea);
}

void QHexEdit::setHighlighting(bool mode) {
	qHexEdit_p->setHighlighting(mode);
}

void QHexEdit::setAddressOffset(int offset) {
	qHexEdit_p->setAddressOffset(offset);
}

int QHexEdit::addressOffset() {
	return qHexEdit_p->addressOffset();
}

void QHexEdit::setData(const QByteArray & data) {
	qHexEdit_p->setData(data);
}

QByteArray QHexEdit::data() {
	return qHexEdit_p->data();
}

void QHexEdit::setAddressAreaColor(const QColor & color) {
	qHexEdit_p->setAddressAreaColor(color);
}

QColor QHexEdit::addressAreaColor() {
	return qHexEdit_p->addressAreaColor();
}

void QHexEdit::setHighlightingColor(const QColor & color) {
	qHexEdit_p->setHighlightingColor(color);
}

QColor QHexEdit::highlightingColor() {
	return qHexEdit_p->highlightingColor();
}

void QHexEdit::setSelectionColor(const QColor & color) {
	qHexEdit_p->setSelectionColor(color);
}

QColor QHexEdit::selectionColor() {
	return qHexEdit_p->selectionColor();
}

void QHexEdit::setOverwriteMode(bool overwriteMode) {
	qHexEdit_p->setOverwriteMode(overwriteMode);
}

bool QHexEdit::overwriteMode() {
	return qHexEdit_p->overwriteMode();
}

void QHexEdit::setReadOnly(bool readOnly) {
	qHexEdit_p->setReadOnly(readOnly);
}

bool QHexEdit::isReadOnly() {
	return qHexEdit_p->isReadOnly();
}

void QHexEdit::setFont(const QFont & font) {
	qHexEdit_p->setFont(font);
}

const QFont & QHexEdit::font() const {
	return qHexEdit_p->font();
}
