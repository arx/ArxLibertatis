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

#ifndef ARX_GRAPHICS_EFFECTS_CABAL_H
#define ARX_GRAPHICS_EFFECTS_CABAL_H

#include "graphics/Renderer.h"
#include "math/Vector.h"

class CabalFx {
public:
	CabalFx();

	/*!
	* Set random number ranges for light emitted
	* by the effect. Valid ranges are (0.0f, 1.0f).
	* For single color light only the respective color range
	* needs to be set.
	*
	* \param range random number range as Vec2f(lower, upper)
	*/
	void setLowerColorRndRange(Color3f range);
	void setUpperColorRndRange(Color3f range);

	void setStartLightColor(Color3f color);
	void setYScale(float scale);
	void setOffset(float offset);

	/*!
	* Adds a ring to draw in the ringset.
	* Adding more than 4 has no effect.
	*
	* \param color of the ring
	*/
	void addRing(Color3f color);

	void disableSecondRingSet();
	void create(Vec3f casterPos);
	void end();

	/*!
	* Update and draw the cabal effect.
	*
	* \return the position of the ringset
	*/
	Vec3f update(Vec3f casterPos);

private:
	Color3f randomizeLightColor();

	LightHandle m_lightHandle;
	float m_yaw;
	float m_scaleY;
	float m_offset;
	Color3f m_startLightColor;
	Color3f m_colorRangeLower;
	Color3f m_colorRangeUpper;
	bool m_hasTwoRingSets;
	size_t m_ringCount;
	Color3f m_ringColors[4];
};

#endif // ARX_GRAPHICS_EFFECTS_CABAL_H
