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

#ifndef ARX_GRAPHICS_FONT_FONT_H
#define ARX_GRAPHICS_FONT_FONT_H

#include <string>
#include <map>

#include <boost/noncopyable.hpp>

#include "graphics/Color.h"
#include "math/Vector.h"

#include "io/resource/ResourcePath.h"

class Font : private boost::noncopyable {
	
	friend class FontCache;
	
public:
	
	struct Info {
		
		Info(const res::path & fontFile, unsigned int fontSize)
			: name(fontFile), size(fontSize) { }
		
		bool operator==(const Info & other) const {
			return name == other.name && size == other.size;
		}
		
		bool operator<(const Info & other) const  {
			return name == other.name ? size < other.size : name < other.name;
		}
		
		res::path name;
		unsigned int size;
		
	};
	
	//! Representation of a glyph
	struct Glyph {
		
		//! Index of the glyph in the font
		unsigned int index;
		
		//! Size of the glyph
		Vec2i size;
		
		//! Offset to use when drawing
		Vec2i draw_offset;
		
		//! Pen advance after write this glyph
		Vec2f advance;
		
		/*!
		 * The difference between hinted and unhinted left side bearing
		 * while autohinting is active. Zero otherwise
		 */
		int lsb_delta;
		
		/*!
		 * The difference between hinted and unhinted right side bearing
		 * while autohinting is active. Zero otherwise
		 */
		int rsb_delta;
		
		//!< UV coordinates
		Vec2f uv_start;
		
		//!< UV coordinates
		Vec2f uv_end;
		
		//!< Texture page on which the glyph can be found
		unsigned int texture;
		
	};
	
public:
	
	typedef u32 Char;
	
	typedef std::string::const_iterator text_iterator;
	
	const Info & getInfo() const { return info; }
	const res::path & getName() const { return info.name; }
	unsigned int getSize() const { return info.size; }
	
	void draw(const Vec2i & p, const std::string & str, const Color & color) {
		draw(p.x, p.y, str, color);
	}
	
	void draw(int x, int y, const std::string & str, Color color) {
		draw(x, y, str.begin(), str.end(), color);
	}
	
	void draw(int x, int y, text_iterator start, text_iterator end, Color color);
	
	Vec2i getTextSize(const std::string & str) {
		return getTextSize(str.begin(), str.end());
	}
	
	Vec2i getTextSize(text_iterator start, text_iterator end);
	
	int getLineHeight() const;
	
	/*!
	 * For debugging purpose... will write one image file per page
	 * under "name_style_size_pagen.png"
	 */
	bool writeToDisk();
	
private:
	
	// Construction/destruction handled by FontCache only
	Font(const res::path & fontFile, unsigned int fontSize, struct FT_FaceRec_ * face);
	~Font();
	
	//! Maps the given character to a placeholder glyph
	void insertPlaceholderGlyph(Char character);
	
	/*!
	 * Inserts a single glyph
	 * Always maps the character to a glyph - uses a placeholder if there
	 * is no glyph for the given character
	 * @return true if the glyph textures were changed
	 */
	bool insertGlyph(Char character);
	
	/*!
	 * Inserts any missing glyphs for the characters in the UTF-8 string [begin, end)
	 * @return true if the glyph textures were changed
	 */
	bool insertMissingGlyphs(text_iterator begin, text_iterator end);
	
private:
	
	template <bool Draw>
	Vec2i process(int pX, int pY, text_iterator start, text_iterator end, Color color);
	
	Info info;
	unsigned int referenceCount;
	
	struct FT_FaceRec_ * face;
	std::map<Char, Glyph> glyphs;
	typedef std::map<Char, Glyph>::const_iterator glyph_iterator;
	
	/*!
	 * Parses UTF-8 input and returns the glyph for the first character
	 * Inserts missing glyphs if possible.
	 * @return a glyph iterator or m_Glyphs.end()
	 */
	glyph_iterator getNextGlyph(text_iterator & it, text_iterator end);
	
	class PackedTexture * textures;
	
};

#endif // ARX_GRAPHICS_FONT_FONT_H
