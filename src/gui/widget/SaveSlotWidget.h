/*
 * Copyright 2018 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GUI_WIDGET_SAVESLOTWIDGET_H
#define ARX_GUI_WIDGET_SAVESLOTWIDGET_H

#include <string>

#include "core/SaveGame.h"
#include "gui/widget/TextWidget.h"
#include "gui/widget/Widget.h"
#include "math/Vector.h"

class Font;

class SaveSlotWidget: public Widget {
	
	Font * m_font;
	SavegameHandle m_savegame;
	std::string m_text;
	bool m_selected;
	
public:
	
	boost::function<void(SaveSlotWidget * /* widget */)> clicked;
	boost::function<void(SaveSlotWidget * /* widget */)> doubleClicked;
	
	SaveSlotWidget(SavegameHandle savegame, size_t i, Font * font, const Rectf & rect);
	
	bool click();
	
	bool doubleClick();
	
	void render(bool mouseOver = false);
	
	virtual WidgetType type() const {
		return WidgetType_SaveSlot;
	}
	
	SavegameHandle savegame() const { return m_savegame; }
	
	void setSelected(bool selected) { m_selected = selected; }
	
};

#endif // ARX_GUI_WIDGET_SAVESLOTWIDGET_H
