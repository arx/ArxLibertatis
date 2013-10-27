/*
 * Copyright 2011-2013 Arx Libertatis Team (see the AUTHORS file)
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

#include <stddef.h>
#include <sstream>
#include <string>
#include <map>
#include <utility>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "graphics/font/Font.h"
#include "io/fs/FilePath.h"
#include "io/log/Logger.h"
#include "io/resource/PakReader.h"
#include "io/resource/ResourcePath.h"
#include "platform/CrashHandler.h"

class FontCache::Impl {
	
private:
	
	Impl();
	~Impl();
	
	typedef std::map<unsigned, Font *> FontMap;
	
	struct FontFile {
		
		size_t m_size;
		char * m_data;
		
		FontFile() : m_size(0), m_data(NULL) { }
		
		FontMap m_sizes;
		
	};
	
	Font * create(const res::path & fontFile, FontFile & file, unsigned int fontSize);
	
	Font * getFont(const res::path & fontFile, unsigned int fontSize);
	
	void releaseFont(Font * font);
	
	void clean(const res::path & fontFile);
	
	typedef std::map<res::path, FontFile> FontFiles;
	FontFiles m_files;
	
	FT_Library m_library;
	
	friend class FontCache;
};

FontCache::Impl::Impl() : m_library(NULL) {
	
	FT_Init_FreeType(&m_library);

	FT_Int ftMajor, ftMinor, ftPatch;
	FT_Library_Version(m_library, &ftMajor, &ftMinor, &ftPatch);
	
	std::ostringstream version;
	version << ftMajor << '.' << ftMinor << '.' << ftPatch;
	
	CrashHandler::setVariable("FreeType version", version.str());
	LogInfo << "Using FreeType " << version.str();
}

FontCache::Impl::~Impl() {
	arx_assert_msg(m_files.size() == 0, "Someone is probably leaking fonts!");
	FT_Done_FreeType(m_library);
}

Font * FontCache::Impl::getFont(const res::path & fontFile, unsigned int fontSize) {
	
	FontFile & file = m_files[fontFile];
	
	Font * pFont = 0;
	FontMap::iterator it = file.m_sizes.find(fontSize);
	if(it == file.m_sizes.end()) {
		pFont = create(fontFile, file, fontSize);
		if(pFont) {
			file.m_sizes[fontSize] = pFont;
		}
	} else {
		pFont = (*it).second;
	}
	
	if(pFont) {
		pFont->referenceCount++;
	} else {
		clean(fontFile);
	}
	
	return pFont;
}

Font * FontCache::Impl::create(const res::path & font, FontFile & file, unsigned int size) {
	
	if(!file.m_data) {
		LogDebug("loading file " << font);
		file.m_data = resources->readAlloc(font, file.m_size);
		if(!file.m_data) {
			return NULL;
		}
	}
	
	LogDebug("creating font " << font << " @ " << size);
	
	// TODO The font face should be shared between multiple Font instances of the same file
	FT_Face face;
	const FT_Byte * data = reinterpret_cast<const FT_Byte *>(file.m_data);
	FT_Error error = FT_New_Memory_Face(m_library, data, file.m_size, 0, &face);
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

void FontCache::Impl::releaseFont(Font * font) {
	
	if(!font) {
		return;
	}
	
	font->referenceCount--;
	
	if(font->referenceCount == 0) {
		
		FontFile & file = m_files[font->getName()];
		
		LogDebug("destroying font " << font->getName() << " @ " << font->getSize());
		file.m_sizes.erase(font->getSize());
		
		clean(font->getName());
		
		delete font;
	}
}

void FontCache::Impl::clean(const res::path & fontFile) {
	
	FontFiles::iterator it = m_files.find(fontFile);
	
	if(it != m_files.end() && it->second.m_sizes.empty()) {
		LogDebug("unloading file " << fontFile);
		free(it->second.m_data);
		m_files.erase(it);
	}
}

FontCache::Impl * FontCache::instance = NULL;

void FontCache::initialize() {
	if(!instance) {
		instance = new FontCache::Impl();
	}
}

void FontCache::shutdown() {
	delete instance;
	instance = NULL;
}

Font * FontCache::getFont(const res::path & fontFile, unsigned int fontSize) {
	return instance->getFont(fontFile, fontSize);
}

void FontCache::releaseFont(Font * font) {
	instance->releaseFont(font);
}
