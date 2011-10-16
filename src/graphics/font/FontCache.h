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

#ifndef ARX_GRAPHICS_FONT_FONTCACHE_H
#define ARX_GRAPHICS_FONT_FONTCACHE_H

#include <string>
#include <map>

#include "graphics/font/Font.h"


class FontCache
{
public:
	static void Initialize();
	static void Shutdown();

	static Font* GetFont( const std::string& fontFile, unsigned int fontSize );
    static void ReleaseFont( Font* pFont );
    
protected:
    // Disable creation from outside.
    FontCache();
	~FontCache();

	Font* Create( const std::string& fontFile, unsigned int fontSize );

private:
	typedef std::map<Font::Info,Font*> FontMap;

    FontMap				m_LoadedFonts;
    static FontCache*	m_Instance;
};

#endif // ARX_GRAPHICS_FONT_FONTCACHE_H
