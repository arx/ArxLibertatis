/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
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

#include "window/RenderWindow.h"

#include <algorithm>

#include "graphics/Math.h"

bool RenderWindow::DisplayMode::operator<(const DisplayMode & o) const {
	
	if(resolution.x != o.resolution.x) {
		return (resolution.x < o.resolution.x);
	} else if(resolution.y != o.resolution.y) {
		return (resolution.y < o.resolution.y);
	} else {
		return (depth < o.depth);
	}
}

void RenderWindow::addListener(RendererListener * listener) {
	renderListeners.push_back(listener);
}

void RenderWindow::removeListener(RendererListener * listener) {
	renderListeners.erase(std::remove(renderListeners.begin(), renderListeners.end(), listener), renderListeners.end());
}

void RenderWindow::onRendererInit() {
	for(RendererListeners::iterator i = renderListeners.begin(); i != renderListeners.end(); ++i) {
		(*i)->onRendererInit(*this);
	}
}

void RenderWindow::onRendererShutdown() {
	for(RendererListeners::iterator i = renderListeners.begin(); i != renderListeners.end(); ++i) {
		(*i)->onRendererShutdown(*this);
	}
}
