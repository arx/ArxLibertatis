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

#ifndef ARX_GRAPHICS_FONT_FONT_H
#define ARX_GRAPHICS_FONT_FONT_H

#include <string>
#include <map>

#include "graphics/Color.h"
#include "math/Vector2.h"

#include "io/resource/ResourcePath.h"

class Font {
	
	friend class FontCache;
	
public:
	
	struct Info {
		
		Info(const res::path & fontFile, unsigned int fontSize )
			: m_Name(fontFile), m_Size(fontSize) { }
		
		bool operator==(const Info & pOther) const {
			return m_Name == pOther.m_Name && m_Size == pOther.m_Size;
		}
		
		bool operator<(const Info & pOther) const  {
			return m_Name == pOther.m_Name ? m_Size < pOther.m_Size : m_Name < pOther.m_Name;
		}
		
		res::path m_Name;
		unsigned int m_Size;
		
	};
	
	//! Representation of a glyph.
	struct Glyph {
		
		Vec2i size;        //!< Size of the glyph.
		Vec2i draw_offset; //!< Offset to use when drawing.
		Vec2f advance;     //!< Pen advance after write this glyph.
		int lsb_delta;     //!< The difference between hinted and unhinted left side bearing while autohinting is active. Zero otherwise.
		int rsb_delta;     //!< The difference between hinted and unhinted right side bearing while autohinting is active. Zero otherwise.
		
		Vec2f uv_start; //!< UV coordinates.
		Vec2f uv_end;   //!< UV coordinates.
		
		unsigned int texture; //!< Texture page on which the glyph can be found.
		
	};
	
public:
	
	const Info & GetInfo() const { return m_Info; }
	const res::path & GetName() const { return m_Info.m_Name; }
	unsigned int GetSize() const { return m_Info.m_Size; }
	
	void Draw(int pX, int pY, const std::string & str, Color color);
	void Draw(int pX, int pY, std::string::const_iterator itStart, std::string::const_iterator itEnd, Color color);
	
	Vec2i GetTextSize(const std::string & str);
	Vec2i GetTextSize(std::string::const_iterator itStart, std::string::const_iterator itEnd);
	
	int GetLineHeight() const;
	
	// For debugging purpose... will write one image file per page
	// under "name_style_size_pagen.png"
	bool WriteToDisk();
	
private:
	
	// Construction/destruction handled by FontCache only
	Font(const res::path & fontFile, unsigned int fontSize, struct FT_FaceRec_ * face);
	~Font();
	
	// Disable copy of Font objects
	Font(const Font & pOther);
	const Font & operator=(const Font & pOther);
	
	bool InsertGlyph(unsigned int character);
	
private:
	
	Info m_Info;
	unsigned int m_RefCount;
	
	struct FT_FaceRec_ * m_FTFace;
	std::map<unsigned int, Glyph> m_Glyphs;
	
	class PackedTexture * m_Textures;
	
};

#endif // ARX_GRAPHICS_FONT_FONT_H
