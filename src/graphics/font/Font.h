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
		
		Info(const res::path & fontFile, unsigned fontSize, unsigned fontWeight)
			: name(fontFile), size(fontSize), weight(fontWeight) { }
		
		bool operator==(const Info & other) const {
			return name == other.name && size == other.size && weight == other.weight;
		}
		
		res::path name;
		unsigned size;
		unsigned weight;
		
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
		size_t texture;
		
		Glyph()
			: index(0)
			, size(0)
			, draw_offset(0)
			, advance(0.f)
			, lsb_delta(0)
			, rsb_delta(0)
			, uv_start(0.f)
			, uv_end(0.f)
			, texture(0)
		{ }
		
	};
	
public:
	
	class TextSize {
		
		Vec2i m_anchor;
		s32 m_start;
		s32 m_end;
		s32 m_next;
		s32 m_height;
		
	public:
		
		TextSize(Vec2i anchor, s32 start, s32 end, s32 next, s32 height)
			: m_anchor(anchor)
			, m_start(start)
			, m_end(end)
			, m_next(next)
			, m_height(height)
		{ }
		
		Vec2i anchor() const {
			return m_anchor;
		}
		
		s32 start() const {
			return m_start;
		}
		
		s32 end() const {
			return m_end;
		}
		
		s32 width() const {
			return m_end - m_start;
		}
		
		s32 height() const {
			return m_height;
		}
		
		s32 advance() const {
			return m_next - m_anchor.x;
		}
		
		s32 head() const {
			return m_start - m_anchor.x;
		}
		
		s32 tail() const {
			return m_next - m_end;
		}
		
		s32 next() const {
			return m_next;
		}
		
		Vec2i size() const {
			return Vec2i(width(), height());
		}
		
		operator Vec2i() const {
			return size();
		}
		
	};
	
	typedef u32 Char;
	
	typedef std::string::const_iterator text_iterator;
	
	const Info & getInfo() const { return m_info; }
	const res::path & getName() const { return m_info.name; }
	unsigned getSize() const { return m_info.size; }
	unsigned getWeight() const { return m_info.weight; }
	
	TextSize draw(const Vec2i & p, const std::string & str, const Color & color) {
		return draw(p.x, p.y, str, color);
	}
	
	TextSize draw(int x, int y, const std::string & str, Color color) {
		return draw(x, y, str.begin(), str.end(), color);
	}
	
	TextSize draw(int x, int y, text_iterator start, text_iterator end, Color color);
	
	TextSize getTextSize(const std::string & str) {
		return getTextSize(str.begin(), str.end());
	}
	
	TextSize getTextSize(text_iterator start, text_iterator end);
	
	size_t getPosition(const std::string & str, int x) {
		return getPosition(str.begin(), str.end(), x) - str.begin();
	}
	
	text_iterator getPosition(text_iterator start, text_iterator end, int x);
	
	int getLineHeight() const;
	int getMaxAdvance() const;
	
private:
	
	// Construction/destruction handled by FontCache only
	Font(const res::path & file, unsigned size, unsigned weight, struct FT_FaceRec_ * face);
	~Font();
	
	//! Maps the given character to a placeholder glyph
	void insertPlaceholderGlyph(Char character);
	
	/*!
	 * Inserts a single glyph
	 * Always maps the character to a glyph - uses a placeholder if there
	 * is no glyph for the given character
	 * \return true if the glyph textures were changed
	 */
	bool insertGlyph(Char character);
	
	/*!
	 * Inserts any missing glyphs for the characters in the UTF-8 string [begin, end)
	 * \return true if the glyph textures were changed
	 */
	bool insertMissingGlyphs(text_iterator begin, text_iterator end);
	
private:
	
	template <bool Draw>
	TextSize process(int pX, int pY, text_iterator start, text_iterator end, Color color);
	
	Info m_info;
	unsigned int m_referenceCount;
	
	struct FT_SizeRec_ * m_size;
	std::map<Char, Glyph> m_glyphs;
	typedef std::map<Char, Glyph>::const_iterator glyph_iterator;
	
	/*!
	 * Parses UTF-8 input and returns the glyph for the first character
	 * Inserts missing glyphs if possible.
	 * \return a glyph iterator or m_Glyphs.end()
	 */
	glyph_iterator getNextGlyph(text_iterator & it, text_iterator end);
	
	class PackedTexture * m_textures;
	
};

#endif // ARX_GRAPHICS_FONT_FONT_H
