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

#include "graphics/font/Font.h"

#include <sstream>
#include <iomanip>
#include <iterator>
#include <cmath>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_SIZES_H
#include FT_OUTLINE_H

#include "graphics/Draw.h"
#include "graphics/Renderer.h"
#include "graphics/texture/Texture.h"
#include "graphics/texture/PackedTexture.h"
#include "graphics/texture/TextureStage.h"
#include "graphics/Vertex.h"

#include "io/fs/FilePath.h"
#include "io/resource/ResourcePath.h"
#include "io/log/Logger.h"

#include "util/Unicode.h"

//! Pre-load all visible characters below this one when creating a font object
static const Font::Char FONT_PRELOAD_LIMIT = 127;

Font::Font(const res::path & file, unsigned size, unsigned weight, FT_Face face)
	: m_info(file, size, weight)
	, m_referenceCount(0)
	, m_size(NULL)
	, m_textures(NULL) {
	
	FT_New_Size(face, &m_size);
	
	FT_Activate_Size(m_size);
	
	// Windows default is 96dpi
	// FreeType default is 72dpi
	FT_Set_Char_Size(m_size->face, 0, m_info.size * 64, 64, 64);
	
	// TODO Compute optimal size using m_FTFace->bbox
	const unsigned int TEXTURE_SIZE = 512;
	
	// Insert all the glyphs into texture pages
	m_textures = new PackedTexture(TEXTURE_SIZE, Image::Format_A8);
	
	// Insert the replacement characters first as they may be needed if others are missing
	insertGlyph('?');
	insertGlyph(util::REPLACEMENT_CHAR);
	
	// Pre-load glyphs for displayable ASCII characters
	for(Char chr = 32; chr < FONT_PRELOAD_LIMIT; ++chr) {
		if(chr != '?') {
			insertGlyph(chr);
		}
	}
	
	m_textures->upload();
	
}

Font::~Font() {
	
	if(m_size) {
		FT_Done_Size(m_size);
	}
	
	delete m_textures;
	
}

void Font::insertPlaceholderGlyph(Char character) {
	
	// Glyph does not exist - insert something so we don't look it up for every render pass
	if(character == util::REPLACEMENT_CHAR) {
		
		// Use '?' as a fallback replacement character
		arx_assert(m_glyphs.find('?') != m_glyphs.end());
		m_glyphs[character] = m_glyphs['?'];
		
	} else if(character < 32 || character == '?') {
		
		// Ignore non-displayable ANSI characters
		Glyph & glyph = m_glyphs[character];
		glyph.size = Vec2i(0);
		glyph.advance = Vec2f(0.f);
		glyph.lsb_delta = glyph.rsb_delta = 0;
		glyph.draw_offset = Vec2i(0);
		glyph.uv_start = Vec2f(0.f);
		glyph.uv_end = Vec2f(0.f);
		glyph.texture = 0;
		
	} else {
		
		LogWarning << "No glyph for character U+" << std::hex << character
		           << " (" << util::encode<util::UTF8>(character) << ") in font "
		           << m_info.name;
		
		arx_assert(m_glyphs.find(util::REPLACEMENT_CHAR) != m_glyphs.end());
		m_glyphs[character] = m_glyphs[util::REPLACEMENT_CHAR];
		
	}
}

bool Font::insertGlyph(Char character) {
	
	if(!m_size) {
		insertPlaceholderGlyph(character);
		return false;
	}
	
	FT_UInt glyphIndex = FT_Get_Char_Index(m_size->face, character);
	if(!glyphIndex) {
		insertPlaceholderGlyph(character);
		return false;
	}
	
	FT_Int32 flags = FT_LOAD_TARGET_LIGHT;
	if(m_info.weight != 0) {
		flags |= FT_LOAD_FORCE_AUTOHINT;
	}
	if(FT_Load_Glyph(m_size->face, glyphIndex, flags)) {
		insertPlaceholderGlyph(character);
		return false;
	}
	
	FT_GlyphSlot ftGlyph = m_size->face->glyph;
	
	if(m_info.weight != 0 && ftGlyph->format == FT_GLYPH_FORMAT_OUTLINE) {
		FT_Pos strength = FT_MulFix(m_size->face->units_per_EM, m_size->metrics.y_scale) / 24 * m_info.weight / 4;
		FT_Outline_Embolden(&ftGlyph->outline, strength);
	}
	
	if(FT_Render_Glyph(ftGlyph, FT_RENDER_MODE_NORMAL)) {
		insertPlaceholderGlyph(character);
		return false;
	}
	
	// Fill in info for this glyph.
	Glyph & glyph = m_glyphs[character];
	glyph.index = glyphIndex;
	glyph.size.x = ftGlyph->bitmap.width;
	glyph.size.y = ftGlyph->bitmap.rows;
	glyph.advance.x = float(ftGlyph->linearHoriAdvance) / 65536.f;
	glyph.advance.y = float(ftGlyph->linearVertAdvance) / 65536.f;
	glyph.lsb_delta = ftGlyph->lsb_delta;
	glyph.rsb_delta = ftGlyph->rsb_delta;
	glyph.draw_offset.x = ftGlyph->bitmap_left;
	glyph.draw_offset.y = ftGlyph->bitmap_top - ftGlyph->bitmap.rows;
	glyph.uv_start = Vec2f(0.f);
	glyph.uv_end = Vec2f(0.f);
	glyph.texture = 0;
	
	// Some glyphs like spaces have a size of 0...
	if(glyph.size.x != 0 && glyph.size.y != 0) {
		
		Image imgGlyph;
		imgGlyph.create(size_t(glyph.size.x), size_t(glyph.size.y), Image::Format_A8);
		
		FT_Bitmap & srcBitmap = ftGlyph->bitmap;
		arx_assert(srcBitmap.pitch >= 0);
		arx_assert(unsigned(srcBitmap.pitch) == unsigned(srcBitmap.width));
		
		// Copy pixels
		unsigned char * src = srcBitmap.buffer;
		unsigned char * dst = imgGlyph.getData();
		memcpy(dst, src, glyph.size.x * glyph.size.y);
		
		Vec2i offset;
		if(!m_textures->insertImage(imgGlyph, glyph.texture, offset)) {
			LogWarning << "Could not upload glyph for character U+" << std::hex << character
			           << " (" << util::encode<util::UTF8>(character) << ") in font "
			           << m_info.name;
			insertPlaceholderGlyph(character);
			return false;
		}
		
		// Compute UV mapping for each glyph.
		const float textureSize = float(m_textures->getTextureSize());
		glyph.uv_start = Vec2f(offset) / Vec2f(textureSize);
		glyph.uv_end = Vec2f(offset + glyph.size) / Vec2f(textureSize);
		
	}
	
	return true;
}

bool Font::insertMissingGlyphs(text_iterator begin, text_iterator end) {
	
	Char chr;
	bool changed = false;
	
	for(text_iterator it = begin; (chr = util::UTF8::read(it, end)) != util::INVALID_CHAR; ) {
		if(m_glyphs.find(chr) == m_glyphs.end()) {
			if(chr >= FONT_PRELOAD_LIMIT && insertGlyph(chr)) {
				changed = true;
			}
		}
	}
	
	return changed;
}

Font::glyph_iterator Font::getNextGlyph(text_iterator & it, text_iterator end) {
	
	Char chr = util::UTF8::read(it, end);
	if(chr == util::INVALID_CHAR) {
		return m_glyphs.end();
	}
	
	glyph_iterator glyph = m_glyphs.find(chr);
	if(glyph != m_glyphs.end()) {
		return glyph; // an existing glyph
	}
	
	if(chr < FONT_PRELOAD_LIMIT) {
		// We pre-load all glyphs for ASCII characters, so there is no point in checking again
		return m_glyphs.end();
	}
	
	if(!insertGlyph(chr)) {
		// No new glyph was inserted but the character was mapped to an existing one
		return m_glyphs.find(chr);
	}
	
	arx_assert(m_glyphs.find(chr) != m_glyphs.end());
	
	// As we need to re-upload the textures now, first check for more missing glyphs
	insertMissingGlyphs(it, end);
	
	return m_glyphs.find(chr); // the newly inserted glyph
}

static void addGlyphVertices(std::vector<TexturedVertex> & vertices,
                             const Font::Glyph & glyph, const Vec2f & pos, Color color) {
	
	float w = glyph.size.x;
	float h = -glyph.size.y;
	float uStart = glyph.uv_start.x;
	float vStart = glyph.uv_end.y;
	float uEnd = glyph.uv_end.x;
	float vEnd = glyph.uv_start.y;
	
	Vec2f p(std::floor(pos.x + glyph.draw_offset.x) - .5f, std::floor(pos.y - glyph.draw_offset.y) - .5f);
	
	TexturedVertex quad[4];
	quad[0].p = Vec3f(p.x, p.y, 0);
	quad[0].uv = Vec2f(uStart, vStart);
	quad[0].color = color.toRGBA();
	quad[0].w = 1.0f;

	quad[1].p = Vec3f(p.x + w, p.y, 0);
	quad[1].uv = Vec2f(uEnd, vStart);
	quad[1].color = color.toRGBA();
	quad[1].w = 1.0f;

	quad[2].p = Vec3f(p.x + w, p.y + h, 0);
	quad[2].uv = Vec2f(uEnd, vEnd);
	quad[2].color = color.toRGBA();
	quad[2].w = 1.0f;

	quad[3].p = Vec3f(p.x, p.y + h, 0);
	quad[3].uv = Vec2f(uStart, vEnd);
	quad[3].color = color.toRGBA();
	quad[3].w = 1.0f;

	vertices.push_back(quad[0]);
	vertices.push_back(quad[1]);
	vertices.push_back(quad[2]);

	vertices.push_back(quad[0]);
	vertices.push_back(quad[2]);
	vertices.push_back(quad[3]);
}

template <bool DoDraw>
Font::TextSize Font::process(int x, int y, text_iterator start, text_iterator end, Color color) {
	
	FT_Activate_Size(m_size);
	
	Vec2f pen(x, y);
		
	int startX = 0;
	int endX = 0;
	
	if(DoDraw) {
		// Subtract one line height (since we flipped the Y origin to be like GDI)
		pen.y += m_size->metrics.ascender >> 6;
	}
	
	FT_UInt prevGlyphIndex = 0;
	FT_Pos prevRsbDelta = 0;
	
	std::vector< std::vector<TexturedVertex> > mapTextureVertices;
	
	for(text_iterator it = start; it != end; ) {
		
		// Get glyph in glyph map
		glyph_iterator itGlyph = getNextGlyph(it, end);
		if(itGlyph == m_glyphs.end()) {
			continue;
		}
		const Glyph & glyph = itGlyph->second;
		
		// Kerning
		if(FT_HAS_KERNING(m_size->face)) {
			if(prevGlyphIndex != 0) {
				FT_Vector delta;
				FT_Get_Kerning(m_size->face, prevGlyphIndex, glyph.index, FT_KERNING_DEFAULT, &delta);
				pen.x += delta.x >> 6;
			}
			prevGlyphIndex = glyph.index;
		}
		
		// Auto hinting adjustments
		if(prevRsbDelta - glyph.lsb_delta >= 32) {
			pen.x--;
		} else if(prevRsbDelta - glyph.lsb_delta < -32) {
			pen.x++;
		}
		prevRsbDelta = glyph.rsb_delta;
		
		// Draw
		if(DoDraw && glyph.size.x != 0 && glyph.size.y != 0) {
			if(glyph.texture >= mapTextureVertices.size()) {
				mapTextureVertices.resize(glyph.texture + 1);
			}
			addGlyphVertices(mapTextureVertices[glyph.texture], glyph, pen, color);
		} else {
			ARX_UNUSED(pen), ARX_UNUSED(color);
		}
		
		// If this is the first drawn char, note the start position
		if(startX == endX) {
			startX = glyph.draw_offset.x;
		}
		endX = s32(pen.x) + glyph.draw_offset.x + glyph.size.x;
		
		// Advance
		pen.x += glyph.advance.x;
	}
	
	if(DoDraw && !mapTextureVertices.empty()) {
		
		m_textures->upload();
		
		UseRenderState state(render2D());
		UseTextureState textureState(TextureStage::FilterNearest, TextureStage::WrapClamp);
		
		// Fixed pipeline texture stage operation
		GRenderer->GetTextureStage(0)->setColorOp(TextureStage::OpDisable);
		
		for(size_t texture = 0; texture < mapTextureVertices.size(); texture++) {
			const std::vector<TexturedVertex> & vertices = mapTextureVertices[texture];
			if(!vertices.empty()) {
				GRenderer->SetTexture(0, &m_textures->getTexture(texture));
				EERIEDRAWPRIM(Renderer::TriangleList, &vertices[0], vertices.size());
			}
		}
		
		GRenderer->ResetTexture(0);
		GRenderer->GetTextureStage(0)->setColorOp(TextureStage::OpModulate);
		
	}
	
	return TextSize(Vec2i(x, y), startX, endX, s32(pen.x), getLineHeight());
}

Font::TextSize Font::draw(int x, int y, text_iterator start, text_iterator end, Color color) {
	return process<true>(x, y, start, end, color);
}

Font::TextSize Font::getTextSize(text_iterator start, text_iterator end) {
	return process<false>(0, 0, start, end, Color::none);
}

Font::text_iterator Font::getPosition(text_iterator start, text_iterator end, int x) {
	
	if(start == end || x < 0) {
		return start;
	}
	
	int last = 0;
	
	for(text_iterator p = start; ; ++p) {
		if(p + 1 != end && util::UTF8::isContinuationByte(*p)) {
			continue;
		}
		int pos = getTextSize(start, p + 1).advance();
		if(pos >= x || p + 1 == end) {
			if(x - last <= pos - x) {
				return p;
			} else {
				return p + 1;
			}
		}
	}
	
}

int Font::getLineHeight() const {
	return int(m_size->metrics.height >> 6);
}

int Font::getMaxAdvance() const {
	return int(m_size->metrics.max_advance >> 6);
}
