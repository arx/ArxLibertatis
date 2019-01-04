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

#include "crashreporter/qhexedit/XByteArray.h"

XByteArray::XByteArray() {
	_oldSize = -99;
	_addressNumbers = 4;
	_addressOffset = 0;
	_realAddressNumbers = 0;
}

int XByteArray::addressOffset() {
	return _addressOffset;
}

void XByteArray::setAddressOffset(int offset) {
	_addressOffset = offset;
}

int XByteArray::addressWidth() {
	return _addressNumbers;
}

void XByteArray::setAddressWidth(int width) {
	if(width >= 0 && width <= 6) {
		_addressNumbers = width;
	}
}

QByteArray & XByteArray::data() {
	return _data;
}

void XByteArray::setData(const QByteArray & data) {
	_data = data;
	_changedData = QByteArray(data.length(), char(0));
}

bool XByteArray::dataChanged(int i) {
	return bool(_changedData[i]);
}

QByteArray XByteArray::dataChanged(int i, int len) {
	return _changedData.mid(i, len);
}

void XByteArray::setDataChanged(int i, bool state) {
	_changedData[i] = char(state);
}

void XByteArray::setDataChanged(int i, const QByteArray & state) {
	int length = state.length();
	int len = (i + length > _changedData.length()) ? _changedData.length() - i : length;
	_changedData.replace(i, len, state);
}

int XByteArray::realAddressNumbers() {
	
	if(_oldSize != _data.size()) {
		// Is addressNumbers wide enought?
		QString test = QString("%1").arg(_data.size() + _addressOffset, _addressNumbers, 16, QChar('0'));
		_realAddressNumbers = test.size();
	}
	
	return _realAddressNumbers;
}

int XByteArray::size() {
	return _data.size();
}

QByteArray & XByteArray::insert(int i, char ch) {
	
	_data.insert(i, ch);
	_changedData.insert(i, char(1));
	
	return _data;
}

QByteArray & XByteArray::insert(int i, const QByteArray & ba) {
	
	_data.insert(i, ba);
	_changedData.insert(i, QByteArray(ba.length(), char(1)));
	
	return _data;
}

QByteArray & XByteArray::remove(int i, int len) {
	
	_data.remove(i, len);
	_changedData.remove(i, len);
	
	return _data;
}

QByteArray & XByteArray::replace(int index, char ch) {
	
	_data[index] = ch;
	_changedData[index] = char(1);
	
	return _data;
}

QByteArray & XByteArray::replace(int index, const QByteArray & ba) {
	return replace(index, ba.length(), ba);
}

QByteArray & XByteArray::replace(int index, int length, const QByteArray & ba) {
	
	int len = (index + length > _data.length()) ? _data.length() - index : length;
	_data.replace(index, len, ba.mid(0, len));
	_changedData.replace(index, len, QByteArray(len, char(1)));
	
	return _data;
}

QChar XByteArray::asciiChar(int index) {
	
	char ch = _data[index];
	if(ch < 0x20 || ch > 0x7e) {
		ch = '.';
	}
	
	return QChar(ch);
}

QString XByteArray::toRedableString(int start, int end) {
	
	int adrWidth = realAddressNumbers();
	if(_addressNumbers > adrWidth) {
		adrWidth = _addressNumbers;
	}
	if(end < 0) {
		end = _data.size();
	}
	
	QString result;
	for(int i = start; i < end; i += 16) {
		QString adrStr = QString("%1").arg(_addressOffset + i, adrWidth, 16, QChar('0'));
		QString hexStr;
		QString ascStr;
		for(int j = 0; j < 16; j++) {
			if(i + j < _data.size()) {
				hexStr.append(" ").append(_data.mid(i + j, 1).toHex());
				ascStr.append(asciiChar(i + j));
			}
		}
		result += adrStr + " " + QString("%1").arg(hexStr, -48) + "  " + QString("%1").arg(ascStr, -17) + "\n";
	}
	
	return result;
}
