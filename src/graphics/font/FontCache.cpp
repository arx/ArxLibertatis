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

#include "graphics/font/FontCache.h"

#include <stddef.h>
#include <sstream>
#include <string>
#include <map>
#include <utility>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <boost/noncopyable.hpp>

#include "gui/Credits.h"
#include "graphics/font/Font.h"
#include "io/fs/FilePath.h"
#include "io/log/Logger.h"
#include "io/resource/PakReader.h"
#include "io/resource/ResourcePath.h"
#include "platform/CrashHandler.h"


#if __cplusplus >= 201103L && (!defined(__GLIBCXX__) || __GLIBCXX__ >= 20150623)
#define ARX_USE_MAP_EMPLACE 1
#else
#define ARX_USE_MAP_EMPLACE 0
#endif

class FontCache::Impl : private boost::noncopyable {
	
	class FontFile {
		
		typedef std::map<u32, Font *> FontMap;
		
		res::path m_file;
		#if !ARX_USE_MAP_EMPLACE
		FT_Library m_library;
		#endif
		std::string m_data;
		FontMap m_sizes;
		FT_Face m_face;
		
		static u32 sizeKey(unsigned size, unsigned weight);
		
		void create(FT_Library library);
		
	public:
		
		explicit FontFile(FT_Library library, const res::path & file);
		
		#if ARX_USE_MAP_EMPLACE
		FontFile(const FontFile & other) = delete;
		FontFile(FontFile && other) noexcept;
		#endif
		
		~FontFile();
		
		Font * getSize(unsigned size, unsigned weight);
		
		void releaseSize(Font * font);
		
		bool empty() const { return m_sizes.empty(); }
		
	};
	
	typedef std::map<res::path, FontFile> FontFiles;
	
	FontFiles m_files;
	FT_Library m_library;
	
	Impl();
	
	~Impl();
	
	Font * getFont(const res::path & file, unsigned size, unsigned weight);
	
	void releaseFont(Font * font);
	
	friend class FontCache;
	
};

u32 FontCache::Impl::FontFile::sizeKey(unsigned size, unsigned weight) {
	arx_assert(!(size & 0xff000000));
	u32 key = size;
	arx_assert(!(weight & 0xffffff00));
	key |= (weight << 24);
	return key;
}

void FontCache::Impl::FontFile::create(FT_Library library) {
	
	const FT_Byte * data = reinterpret_cast<const FT_Byte *>(m_data.data());
	FT_New_Memory_Face(library, data, FT_Long(m_data.size()), 0, &m_face);
	
}

FontCache::Impl::FontFile::FontFile(FT_Library library, const res::path & file)
	: m_file(file)
	#if !ARX_USE_MAP_EMPLACE
	, m_library(library)
	#endif
	, m_data(g_resources->read(file))
	, m_face(NULL)
{
	
	#if ARX_USE_MAP_EMPLACE
	create(library);
	#endif
	
}

#if ARX_USE_MAP_EMPLACE
FontCache::Impl::FontFile::FontFile(FontFile && other) noexcept
	: m_file(std::move(other.m_file))
	, m_data(std::move(other.m_data))
	, m_face(other.m_face)
{
	other.m_face = NULL;
}
#endif

FontCache::Impl::FontFile::~FontFile() {
	
	arx_assert(empty());
	
	if(m_face) {
		// Release FreeType face object.
		FT_Done_Face(m_face);
	}
	
}

Font * FontCache::Impl::FontFile::getSize(unsigned size, unsigned weight) {
	
	u32 key = sizeKey(size, weight);
	
	FontMap::iterator it = m_sizes.find(key);
	if(it != m_sizes.end()) {
		return it->second;
	}
	
	#if !ARX_USE_MAP_EMPLACE
	if(!m_face) {
		create(m_library);
	}
	#endif
	
	if(!m_face) {
		return NULL;
	}
	
	Font * font = new Font(m_file, size, weight, m_face);
	
	m_sizes.insert(FontMap::value_type(key, font));
	
	return font;
}

void FontCache::Impl::FontFile::releaseSize(Font * font) {
	m_sizes.erase(sizeKey(font->getSize(), font->getWeight()));
	delete font;
}

FontCache::Impl::Impl() : m_library(NULL) {
	
	FT_Init_FreeType(&m_library);
	
	FT_Int ftMajor, ftMinor, ftPatch;
	FT_Library_Version(m_library, &ftMajor, &ftMinor, &ftPatch);
	
	std::ostringstream version;
	version << ftMajor << '.' << ftMinor << '.' << ftPatch;
	
	LogInfo << "Using FreeType " << version.str();
	CrashHandler::setVariable("FreeType version", version.str());
	credits::setLibraryCredits("font", "FreeType " + version.str());
}

FontCache::Impl::~Impl() {
	
	arx_assert_msg(m_files.empty(), "Someone is probably leaking fonts!");
	
	FT_Done_FreeType(m_library);
	
}

Font * FontCache::Impl::getFont(const res::path & file, unsigned size, unsigned weight) {
	
	FontFiles::iterator it = m_files.find(file);
	if(it == m_files.end()) {
		#if ARX_USE_MAP_EMPLACE
		it = m_files.emplace(file, FontFile(m_library, file)).first;
		#else
		it = m_files.insert(FontFiles::value_type(file, FontFile(m_library, file))).first;
		#endif
	}
	
	Font * font = it->second.getSize(size, weight);
	
	if(font) {
		font->m_referenceCount++;
	} else if(it->second.empty()) {
		m_files.erase(it);
	}
	
	return font;
}

void FontCache::Impl::releaseFont(Font * font) {
	
	if(!font) {
		return;
	}
	
	font->m_referenceCount--;
	if(font->m_referenceCount != 0) {
		return;
	}
	
	FontFiles::iterator it = m_files.find(font->getName());
	arx_assert(it != m_files.end());
	
	it->second.releaseSize(font);
	
	if(it->second.empty()) {
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

Font * FontCache::getFont(const res::path & file, unsigned size, unsigned weight) {
	return instance->getFont(file, size, weight);
}

void FontCache::releaseFont(Font * font) {
	instance->releaseFont(font);
}
