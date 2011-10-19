/*
 * Copyright 2011 Arx Libertatis Team (see the AUTHORS file)
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

void RenderWindow::setGamma(float brightness, float contrast, float gamma) {
	
	u16 ramp[256];
	
	float fGammaMax = (1.f / 6.f);
	float fGammaMin = 2.f;
	float fGamma = ((fGammaMax - fGammaMin) / 11.f) * (gamma + 1.f) + fGammaMin;
	
	float fLuminosityMin = -.2f;
	float fLuminosityMax = .2f;
	float fLuminosity = ((fLuminosityMax - fLuminosityMin) / 11.f) * (brightness + 1.f) + fLuminosityMin;
	
	float fContrastMax = -.3f;
	float fContrastMin = .3f;
	float fContrast = ((fContrastMax - fContrastMin) / 11.f) * (contrast + 1.f) + fContrastMin;
	
	float fRangeMin = 0.f + fContrast;
	float fRangeMax = 1.f - fContrast;
	float fdVal = (fRangeMax - fRangeMin) / 256.f;
	float fVal = 0.f;
	
	for(size_t i = 0; i < 256; i++) {
		
		int iColor = clamp(int(65536.f * (fLuminosity + pow(fVal, fGamma))), 0, 65535);
		
		ramp[i] = static_cast<u16>(iColor);
		
		fVal += fdVal;
	}
	
	setGammaRamp(ramp, ramp, ramp);
}
