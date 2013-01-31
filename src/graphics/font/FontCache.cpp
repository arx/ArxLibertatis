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

#include "graphics/font/FontCache.h"

#include <sstream>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "io/fs/FilePath.h"
#include "io/log/Logger.h"
#include "io/resource/PakReader.h"
#include "platform/CrashHandler.h"

static FT_Library g_FTLibrary = NULL;
FontCache * FontCache::instance = NULL;

void FontCache::initialize() {
	if(!instance) {
		instance = new FontCache();
	}
}

void FontCache::shutdown() {
	delete instance;
	instance = NULL;
}

FontCache::FontCache() {
	
	FT_Init_FreeType(&g_FTLibrary);

	FT_Int ftMajor, ftMinor, ftPatch;
	FT_Library_Version(g_FTLibrary, &ftMajor, &ftMinor, &ftPatch);
	
	std::ostringstream version;
	version << ftMajor << '.' << ftMinor << '.' << ftPatch;
	
	CrashHandler::setVariable("FreeType version", version.str());
	LogInfo << "Using FreeType " << version.str();
}

FontCache::~FontCache() {
	arx_assert_msg(files.size() == 0, "Someone is probably leaking fonts!");
	FT_Done_FreeType(g_FTLibrary);
	g_FTLibrary = NULL;
}

Font * FontCache::getFont(const res::path & fontFile, unsigned int fontSize) {
	
	FontFile & file = instance->files[fontFile];
	
	Font * pFont = 0;
	FontMap::iterator it = file.sizes.find(fontSize);
	if(it == file.sizes.end()) {
		pFont = instance->create(fontFile, file, fontSize);
		if(pFont) {
			file.sizes[fontSize] = pFont;
		}
	} else {
		pFont = (*it).second;
	}
	
	if(pFont) {
		pFont->referenceCount++;
	} else if(!file.sizes.empty()) {
		instance->files.erase(fontFile);
	}
	
	return pFont;
}

Font * FontCache::create(const res::path & font, FontFile & file, unsigned int size) {
	
	if(!file.data) {
		LogDebug("loading file " << font);
		file.data = resources->readAlloc(font, file.size);
		if(!file.data) {
			return NULL;
		}
	}
	
	LogDebug("creating font " << font << " @ " << size);
	
	FT_Face face;
	const FT_Byte * data = reinterpret_cast<const FT_Byte *>(file.data);
	FT_Error error = FT_New_Memory_Face(g_FTLibrary, data, file.size, 0, &face);
	if(error == FT_Err_Unknown_File_Format) {
		// the font file's format is unsupported
		LogError << "Font creation error: FT_Err_Unknown_File_Format";
		return NULL;
	} else if(error) {
		// ... another error code means that the font file could not
		// ... be opened or read, or simply that it is broken...
		return NULL;
	}
	
	// Windows default is 96dpi
	// Freetype default is 72dpi
	error = FT_Set_Char_Size(face, 0, size * 64, 64, 64);
	if(error) {
		return NULL;
	}
	
	return new Font(font, size, face);
}

void FontCache::releaseFont(Font * font) {
	
	if(!font) {
		return;
	}
	
	font->referenceCount--;
	
	if(font->referenceCount == 0) {
		
		FontFile & file = instance->files[font->getName()];
		
		file.sizes.erase(font->getSize());
		LogDebug("destroying font " << font->getName() << " @ " << font->getSize());
		
		if(file.sizes.empty()) {
			free(file.data);
			instance->files.erase(font->getName());
			LogDebug("unloading file " << font->getName());
		}
		
		delete font;
	}
}
