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

#ifndef ARX_GUI_WIDGET_CYCLETEXTWIDGET_H
#define ARX_GUI_WIDGET_CYCLETEXTWIDGET_H

#include <vector>
#include <boost/function.hpp>

#include "gui/widget/ButtonWidget.h"
#include "gui/widget/Widget.h"

class CycleTextWidget: public Widget {
	
public:
	explicit CycleTextWidget();
	virtual ~CycleTextWidget();
	
	void setValue(int value) { iPos = value; }
	int getValue() const { return iPos; }
	void setOldValue(int value) { iOldPos = value; }
	int getOldValue() const { return iOldPos; }
	
	void selectLast();
	
	void AddText(TextWidget * text);
	
	void Move(const Vec2f & offset);
	bool OnMouseClick();
	void Update();
	void Render();
	void RenderMouseOver();
	void EmptyFunction();
	virtual void setEnabled(bool enable);
	
	boost::function<void(int, const std::string &)> valueChanged;
	
	virtual WidgetType type() const {
		return WidgetType_CycleText;
	};
	
private:
	ButtonWidget		*	pLeftButton;
	ButtonWidget		*	pRightButton;
	std::vector<TextWidget*>	vText;
	int					iPos;
	int					iOldPos;
};

#endif // ARX_GUI_WIDGET_CYCLETEXTWIDGET_H
