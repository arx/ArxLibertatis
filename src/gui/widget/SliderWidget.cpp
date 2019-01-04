/*
 * Copyright 2015-2018 Arx Libertatis Team (see the AUTHORS file)
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

#include "gui/widget/SliderWidget.h"

#include <algorithm>

#include "core/Core.h"
#include "graphics/DrawLine.h"
#include "graphics/Renderer.h"
#include "graphics/data/TextureContainer.h"
#include "gui/MenuPublic.h"
#include "gui/menu/MenuCursor.h"
#include "gui/widget/ButtonWidget.h"
#include "gui/widget/TextWidget.h"
#include "input/Input.h"

SliderWidget::SliderWidget(const Vec2f & size, Font * font, const std::string & label)
	: m_label(new TextWidget(font, label))
	, m_left(new ButtonWidget(Vec2f(size.y), "graph/interface/menus/menu_slider_button_left"))
	, m_right(new ButtonWidget(Vec2f(size.y), "graph/interface/menus/menu_slider_button_right"))
	, m_textureOff(TextureContainer::Load("graph/interface/menus/menu_slider_off"))
	, m_textureOn(TextureContainer::Load("graph/interface/menus/menu_slider_on"))
	, m_slider(10 * size.y / 2, size.y)
	, m_minimum(0)
	, m_value(0)
{
	
	m_label->forceDisplay(TextWidget::Dynamic);
	
	arx_assert(m_textureOff);
	arx_assert(m_textureOn);
	
	float minWidth = m_left->m_rect.width() + m_slider.width() + m_right->m_rect.width();
	m_rect = Rectf(std::max(minWidth, size.x), std::max(m_slider.height(), m_label->m_rect.height()));
	
	m_right->setPosition(Vec2f(m_rect.right - m_right->m_rect.width(),
	                           m_rect.center().y - m_right->m_rect.height() / 2));
	m_slider.moveTo(Vec2f(m_right->m_rect.left - m_slider.width(),
	                      m_rect.center().y - m_slider.height() / 2));
	m_left->setPosition(Vec2f(m_slider.left - m_left->m_rect.width(),
	                          m_rect.center().y - m_left->m_rect.height() / 2));
	
}

SliderWidget::~SliderWidget() {
	delete m_label;
	delete m_left;
	delete m_right;
}

void SliderWidget::setMinimum(int minimum) {
	m_minimum = minimum;
	m_value = std::max(m_minimum, m_value);
}

void SliderWidget::move(const Vec2f & offset) {
	Widget::move(offset);
	m_label->move(offset);
	m_left->move(offset);
	m_slider.move(offset);
	m_right->move(offset);
}

void SliderWidget::hover() {

	if(GInput->isKeyPressedNowPressed(Keyboard::Key_LeftArrow) || GInput->getMouseWheelDir() < 0) {
		newValue(m_value - 1);
	} else if(GInput->isKeyPressedNowPressed(Keyboard::Key_RightArrow) || GInput->getMouseWheelDir() > 0) {
		newValue(m_value + 1);
	}
	
}

bool SliderWidget::click() {
	
	bool result = Widget::click();
	
	const Vec2f cursor = Vec2f(GInput->getMousePosition());
	
	if(m_rect.contains(cursor)) {
		if(m_left->m_rect.contains(cursor)) {
			newValue(m_value - 1);
		} else if(m_right->m_rect.contains(cursor)) {
			newValue(m_value + 1);
		} else if(m_slider.contains(cursor)) {
			newValue(int(glm::round((cursor.x - m_slider.left) / m_slider.width() * 10)));
		} else {
			newValue((m_value + 1) % 11);
		}
	}
	
	return result;
}

void SliderWidget::update() {
	
	m_left->update();
	m_right->update();
	
}

void SliderWidget::render(bool mouseOver) {
	
	m_label->render(mouseOver);
	
	const Vec2f cursor = Vec2f(GInput->getMousePosition());
	
	if(m_enabled) {
		m_left->render(m_left->m_rect.contains(cursor));
		m_right->render(m_right->m_rect.contains(cursor));
	}
	
	UseRenderState state(render2D().blendAdditive());
	
	Rectf rect(m_slider.topLeft(), m_slider.width() / 10, m_slider.height());
	
	for(int i = 0; i < 10; i++) {
		TextureContainer * texture = (i < m_value) ? m_textureOn : m_textureOff;
		Color color = m_enabled ? Color::white : Color::gray(0.25f);
		EERIEDrawBitmap(rect, 0, texture, color);
		if(m_enabled && m_slider.contains(cursor) && cursor.x > rect.center().x) {
			EERIEDrawBitmap(rect, 0, texture, color);
		}
		rect.move(m_slider.width() / 10, 0.f);
	}
	
}

void SliderWidget::newValue(int value) {
	
	value = arx::clamp(value, m_minimum, 10);
	
	if(value == m_value) {
		return;
	}
	
	m_value = value;
	
	if(valueChanged) {
		valueChanged(m_value);
	}
	
}
