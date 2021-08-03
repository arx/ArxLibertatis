/*
 * Copyright 2011-2020 Arx Libertatis Team (see the AUTHORS file)
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
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <utility>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "gui/Credits.h"
#include "graphics/font/Font.h"
#include "io/fs/FilePath.h"
#include "io/log/Logger.h"
#include "io/resource/PakReader.h"
#include "io/resource/ResourcePath.h"
#include "platform/CrashHandler.h"


class FontCache::Impl {
	
	struct FontDeleter {
		void operator()(Font * font) const { delete font; }
	};
	
	class FontFile {
		
		res::path m_file;
		std::string m_data;
		std::map<u32, std::unique_ptr<Font, FontDeleter>> m_sizes;
		FT_Face m_face;
		
		static u32 sizeKey(unsigned size, unsigned weight);
		
		void create(FT_Library library);
		
	public:
		
		FontFile(const FontFile &) = delete;
		FontFile & operator=(const FontFile &) = delete;
		
		explicit FontFile(FT_Library library, const res::path & file);
		FontFile(FontFile && other) noexcept;
		~FontFile();
		
		Font * getSize(unsigned size, unsigned weight, bool preload);
		
		void releaseSize(Font * font);
		
		bool empty() const { return m_sizes.empty(); }
		
	};
	
	std::map<res::path, FontFile> m_files;
	FT_Library m_library;
	
public:
	
	Impl(const Impl &) = delete;
	Impl & operator=(const Impl &) = delete;
	
private:
	
	Impl();
	~Impl();
	
	Font * getFont(const res::path & file, unsigned size, unsigned weight, bool preload);
	
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
	, m_data(g_resources->read(file))
	, m_face(nullptr)
{
	create(library);
}

FontCache::Impl::FontFile::FontFile(FontFile && other) noexcept
	: m_file(std::move(other.m_file))
	, m_data(std::move(other.m_data))
	, m_face(other.m_face)
{
	other.m_face = nullptr;
}

FontCache::Impl::FontFile::~FontFile() {
	
	arx_assert(empty());
	
	if(m_face) {
		// Release FreeType face object.
		FT_Done_Face(m_face);
	}
	
}

Font * FontCache::Impl::FontFile::getSize(unsigned size, unsigned weight, bool preload) {
	
	u32 key = sizeKey(size, weight);
	
	if(auto it = m_sizes.find(key); it != m_sizes.end()) {
		return it->second.get();
	}
	
	if(!m_face) {
		return nullptr;
	}
	
	return m_sizes.emplace(key, new Font(m_file, size, weight, m_face, preload)).first->second.get();
}

void FontCache::Impl::FontFile::releaseSize(Font * font) {
	m_sizes.erase(sizeKey(font->getSize(), font->getWeight()));
}

FontCache::Impl::Impl() : m_library(nullptr) {
	
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

Font * FontCache::Impl::getFont(const res::path & file, unsigned size, unsigned weight, bool preload) {
	
	auto it = m_files.find(file);
	if(it == m_files.end()) {
		it = m_files.emplace(file, FontFile(m_library, file)).first;
	}
	
	Font * font = it->second.getSize(size, weight, preload);
	
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
	
	auto it = m_files.find(font->getName());
	arx_assert(it != m_files.end());
	
	it->second.releaseSize(font);
	
	if(it->second.empty()) {
		m_files.erase(it);
	}
	
}

FontCache::Impl * FontCache::instance = nullptr;

void FontCache::initialize() {
	if(!instance) {
		instance = new FontCache::Impl();
	}
}

void FontCache::shutdown() {
	delete instance;
	instance = nullptr;
}

Font * FontCache::getFont(const res::path & file, unsigned size, unsigned weight, bool preload) {
	return instance->getFont(file, size, weight, preload);
}

void FontCache::releaseFont(Font * font) {
	instance->releaseFont(font);
}
