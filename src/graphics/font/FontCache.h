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

#ifndef ARX_GRAPHICS_FONT_FONTCACHE_H
#define ARX_GRAPHICS_FONT_FONTCACHE_H

namespace res { class path; }

class Font;

class FontCache {
	
	class Impl;
	
public:
	
	static void initialize();
	static void shutdown();
	
	static Font * getFont(const res::path & file, unsigned size, unsigned weight = 0);
	
	static void releaseFont(Font * font);
	
private:
	
	// Disable creation from outside.
	FontCache();
	~FontCache();
	
	static Impl * instance;
	
};

#endif // ARX_GRAPHICS_FONT_FONTCACHE_H
