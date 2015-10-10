/*
 * Copyright 2015 Arx Libertatis Team (see the AUTHORS file)
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

#include "gui/widget/Widget.h"

#include "gui/MenuWidgets.h"
#include "gui/widget/CycleTextWidget.h"
#include "input/Input.h"

Widget::Widget(MENUSTATE _ms)
	: bTestYDouble(false)
	, pRef(NULL)
	, rZone(0, 0, 0, 0)
	, iID(BUTTON_INVALID)
	, m_savegame(0)
	, enabled(true)
	, bCheck(true)
{
	ePlace=NOCENTER;
	eState=TNOP;
	eMenuState=_ms;
	iShortCut=-1;
}

extern TextWidget * pMenuElementApply;
extern TextWidget * pLoadConfirm;
extern TextWidget * pDeleteConfirm;
extern TextWidget * pDeleteButton;
extern CycleTextWidget * pMenuSliderResol;
extern CheckboxWidget * fullscreenCheckbox;

Widget::~Widget() {

	if(this == pMenuElementApply) {
		pMenuElementApply = NULL;
	}

	if(this == pLoadConfirm) {
		pLoadConfirm = NULL;
	}

	if(this == pDeleteConfirm) {
		pDeleteConfirm = NULL;
	}

	if(this == pDeleteButton) {
		pDeleteButton = NULL;
	}

	if(this == pMenuSliderResol) {
		pMenuSliderResol = NULL;
	}
	
	if(this == fullscreenCheckbox) {
		fullscreenCheckbox = NULL;
	}
}

Widget* Widget::OnShortCut() {

	if(iShortCut == -1)
		return NULL;

	if(GInput->isKeyPressedNowUnPressed(iShortCut)) {
		return this;
	}

	return NULL;
}

void Widget::Move(const Vec2i & offset) {
	rZone.move(offset.x, offset.y);
}

void Widget::SetPos(Vec2i pos) {

	int iWidth  = rZone.right - rZone.left;
	int iHeight = rZone.bottom - rZone.top;
	
	rZone.left   = pos.x;
	rZone.top    = pos.y;
	rZone.right  = pos.x + abs(iWidth);
	rZone.bottom = pos.y + abs(iHeight);
}

Widget * Widget::IsMouseOver(const Vec2s& mousePos) const {

	int iYDouble=0;

	if(bTestYDouble) {
		iYDouble=(rZone.bottom-rZone.top)>>1;
	}

	if(   mousePos.x >= rZone.left
	   && mousePos.y >= rZone.top - iYDouble
	   && mousePos.x <= rZone.right
	   && mousePos.y <= rZone.bottom + iYDouble
	) {
		return pRef;
	}

	return NULL;
}

Widget *Widget::GetZoneWithID(int zoneId) {
	return (iID == zoneId) ? this : NULL;
}

void Widget::SetShortCut(int _iShortCut) {
	iShortCut = _iShortCut;
}

void Widget::setEnabled(bool enable) {
	enabled = enable;
}

void Widget::SetCheckOff() {
	bCheck = false;
}

void Widget::SetCheckOn() {
	bCheck = true;
}

bool Widget::getCheck() {
	return bCheck;
}
