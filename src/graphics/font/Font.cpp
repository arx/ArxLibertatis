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

#include "graphics/font/Font.h"

#include <sstream>
#include <iomanip>
#include <iostream>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "graphics/Renderer.h"
#include "graphics/texture/Texture.h"
#include "graphics/texture/PackedTexture.h"
#include "graphics/texture/TextureStage.h"
#include "io/fs/FilePath.h"
#include "io/resource/ResourcePath.h"
#include "io/log/Logger.h"

using std::string;

Font::Font(const res::path & fontFile, unsigned int fontSize, FT_Face face) 
	: m_Info(fontFile, fontSize)
	, m_RefCount(0)
	, m_FTFace(face)
	, m_Textures(0) {
	
	// TODO-font: Compute optimal size using m_FTFace->bbox
	const unsigned int TEXTURE_SIZE = 512;
	
	// Insert all the glyphs into texture pages
	m_Textures = new PackedTexture(TEXTURE_SIZE, Image::Format_A8);
	
	// Pre-load common glyphs
	for(u32 chr = 32; chr < 256; ++chr) {
		insertGlyph(chr);
	}
	
	m_Textures->upload();
}

Font::~Font() {
	
	delete m_Textures;
	
	// Release FreeType face object.
	FT_Done_Face(m_FTFace);
}

void Font::insertPlaceholderGlyph(u32 character) {
	
	// Glyp does not exist - insert something so we don't look it up for every render pass.
	if(character < 256) {
		
		// ignore non-displayable ANSI characters (and some more)
		Glyph & glyph = m_Glyphs[character];
		glyph.size = Vec2i::ZERO;
		glyph.advance = Vec2f::ZERO;
		glyph.lsb_delta = glyph.rsb_delta = 0;
		glyph.draw_offset = Vec2i::ZERO;
		glyph.uv_start = Vec2f::ZERO;
		glyph.uv_end = Vec2f::ZERO;
		glyph.texture = 0;
		
	} else {
		m_Glyphs[character] = m_Glyphs['?'];
		LogWarning << "No glyph for character U+" << std::hex << character;
	}
}

bool Font::insertGlyph(u32 character) {
	
	FT_Error error;
	FT_UInt glyphIndex = FT_Get_Char_Index(m_FTFace, character);
	if(!glyphIndex) {
		insertPlaceholderGlyph(character);
		return false;
	}
	
	error = FT_Load_Glyph(m_FTFace, glyphIndex, FT_LOAD_FORCE_AUTOHINT);
	if(error) {
		insertPlaceholderGlyph(character);
		return false;
	}
	
	error = FT_Render_Glyph(m_FTFace->glyph, FT_RENDER_MODE_NORMAL);
	if(error) {
		insertPlaceholderGlyph(character);
		return false;
	}
	
	// Fill in info for this glyph.
	Glyph & glyph = m_Glyphs[character];
	glyph.size.x = m_FTFace->glyph->bitmap.width;
	glyph.size.y = m_FTFace->glyph->bitmap.rows;
	glyph.advance.x = m_FTFace->glyph->linearHoriAdvance / 65536.0f;
	glyph.advance.y = m_FTFace->glyph->linearVertAdvance / 65536.0f;
	glyph.lsb_delta = m_FTFace->glyph->lsb_delta;
	glyph.rsb_delta = m_FTFace->glyph->rsb_delta;
	glyph.draw_offset.x = m_FTFace->glyph->bitmap_left;
	glyph.draw_offset.y = m_FTFace->glyph->bitmap_top - m_FTFace->glyph->bitmap.rows;
	glyph.uv_start = Vec2f::ZERO;
	glyph.uv_end = Vec2f::ZERO;
	glyph.texture = 0;
	
	// Some glyphs like spaces have a size of 0...
	if(glyph.size.x != 0 && glyph.size.y != 0) {
		
		Image imgGlyph;
		imgGlyph.Create(glyph.size.x, glyph.size.y, Image::Format_A8);
		
		FT_Bitmap * srcBitmap = &m_FTFace->glyph->bitmap;
		arx_assert(srcBitmap->pitch == srcBitmap->width);
		
		// Copy pixels
		unsigned char * src = srcBitmap->buffer;
		unsigned char * dst = imgGlyph.GetData();
		memcpy(dst, src, glyph.size.x * glyph.size.y);
		
		Vec2i offset;
		m_Textures->insertImage(imgGlyph, glyph.texture, offset);
		
		// Compute UV mapping for each glyph.
		const float textureSize = m_Textures->getTextureSize();
		glyph.uv_start.x = offset.x / textureSize;
		glyph.uv_start.y = offset.y / textureSize;
		glyph.uv_end.x = (offset.x + glyph.size.x) / textureSize;
		glyph.uv_end.y = (offset.y + glyph.size.y) / textureSize;
	}
	
	return true;
}

bool Font::WriteToDisk() {
	
	bool ok = true;
	
	for(unsigned int i = 0; i < m_Textures->getTextureCount(); ++i) {
		Texture2D & tex = m_Textures->getTexture(i);
		
		std::stringstream ss;
		ss << m_FTFace->family_name;
		if(m_FTFace->style_name != NULL) {
			ss << "_";
			ss << m_FTFace->style_name;
		}
		ss << "_";
		ss << m_Info.m_Size;
		ss << "_page";
		ss << i;
		ss << ".png";
		
		ok = ok && tex.GetImage().save(ss.str());
	}
	
	return ok;
}

inline static bool read_utf8(Font::text_iterator & it, Font::text_iterator end, u32 & chr) {
	
	if(it == end) {
		return false;
	}
	chr = *it++;
	
	if(chr & (1 << 7)) {
		
		if(!(chr & (1 << 6))) {
			// TODO bad start position
		}
		
		if(it == end) {
			return false;
		}
		chr &= 0x3f, chr <<= 6, chr |= ((*it++) & 0x3f);
		
		if(chr & (1 << (5 + 6))) {
			
			if(it == end) {
				return false;
			}
			chr &= ~(1 << (5 + 6)), chr <<= 6, chr |= ((*it++) & 0x3f);
			
			if(chr & (1 << (4 + 6 + 6))) {
				
				if(it == end) {
					return false;
				}
				chr &= ~(1 << (4 + 6 + 6)), chr <<= 6, chr |= ((*it++) & 0x3f);
				
				if(chr & (1 << (3 + 6 + 6 + 6))) {
					// TODO bad UTF-8 string
				}
				
			}
		}
	}
	
	return true;
}

bool Font::insertMissingGlyphs(text_iterator begin, text_iterator end) {
	
	u32 chr;
	bool changed = false;
	
	for(text_iterator it = begin; read_utf8(it, end, chr); ) {
		if(m_Glyphs.find(chr) == m_Glyphs.end()) {
			if(chr >= 256 && insertGlyph(chr)) {
				changed = true;
			}
		}
	}
	
	return changed;
}

Font::glyph_iterator Font::getNextGlyph(text_iterator & it, text_iterator end, u32 & chr) {
	
	if(!read_utf8(it, end, chr)) {
		return m_Glyphs.end();
	}
	
	glyph_iterator glyph = m_Glyphs.find(chr);
	if(glyph != m_Glyphs.end()) {
		return glyph; // an existing glyph
	}
	
	if(chr < 256) {
		// We pre-load all glyphs for charactres < 256, se there is no point in checking again
		return m_Glyphs.end();
	}
	
	if(!insertGlyph(chr)) {
		// No new glyph was inserted but the character was mapped to an existing one
		return m_Glyphs.find(chr);
	}
	
	arx_assert(m_Glyphs.find(chr) != m_Glyphs.end());
	
	// As we need to re-upload the textures now, first check for more missing glyphs
	insertMissingGlyphs(it, end);
	
	// Re-upload the changed textures
	m_Textures->upload();
	
	return m_Glyphs.find(chr); // the newly inserted glyph
}

void Font::Draw(int x, int y, text_iterator itStart, text_iterator itEnd, Color color) {
	
	GRenderer->SetRenderState(Renderer::Lighting, false);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetBlendFunc(Renderer::BlendSrcAlpha, Renderer::BlendInvSrcAlpha);
	
	GRenderer->SetRenderState(Renderer::DepthTest, false);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetCulling(Renderer::CullNone);
	
	// 2D projection setup... Put origin (0,0) in the top left corner like GDI... 
	Rect viewport = GRenderer->GetViewport();
	GRenderer->Begin2DProjection(viewport.left, viewport.right, viewport.bottom, viewport.top, -1.f, 1.f);
	
	// Fixed pipeline texture stage operation
	GRenderer->GetTextureStage(0)->SetColorOp(TextureStage::ArgDiffuse);
	GRenderer->GetTextureStage(0)->SetAlphaOp(TextureStage::ArgTexture);
	
	GRenderer->GetTextureStage(0)->SetWrapMode(TextureStage::WrapClamp);
	GRenderer->GetTextureStage(0)->SetMinFilter(TextureStage::FilterNearest);
	GRenderer->GetTextureStage(0)->SetMagFilter(TextureStage::FilterNearest);
	
	float penX = x;
	float penY = y;
	
	// Substract one line height (since we flipped the Y origin to be like GDI)
	penY += m_FTFace->size->metrics.ascender >> 6;
	
	FT_UInt currentGlyphIdx;
	FT_UInt previousGlyphIdx = 0;
	FT_Pos prevRsbDelta = 0;
	
	u32 chr;
	for(text_iterator it = itStart; it != itEnd; ) {
		
		// Get glyph in glyph map
		glyph_iterator itGlyph = getNextGlyph(it, itEnd, chr);
		if(itGlyph == m_Glyphs.end()) {
			continue;
		}
		
		const Glyph & glyph = itGlyph->second;
		
		// Kerning
		if(FT_HAS_KERNING(m_FTFace)) {
			currentGlyphIdx = FT_Get_Char_Index(m_FTFace, chr);
			if(previousGlyphIdx != 0) {
				FT_Vector delta;
				FT_Get_Kerning(m_FTFace, previousGlyphIdx, currentGlyphIdx, FT_KERNING_DEFAULT, &delta);
				penX += delta.x >> 6;
			}
			previousGlyphIdx = currentGlyphIdx;
		}
		
		// Auto hinting adjustments
		if(prevRsbDelta - glyph.lsb_delta >= 32) {
			penX--;
		} else if( prevRsbDelta - glyph.lsb_delta < -32) {
			penX++;
		}
		prevRsbDelta = glyph.rsb_delta;
		
		// Draw
		if(glyph.size.x != 0 && glyph.size.y != 0) {
			GRenderer->SetTexture(0, &m_Textures->getTexture(glyph.texture));
			GRenderer->DrawTexturedRect(((int)penX) + glyph.draw_offset.x, ((int)penY) - glyph.draw_offset.y, glyph.size.x, -glyph.size.y, glyph.uv_start.x, glyph.uv_end.y, glyph.uv_end.x, glyph.uv_start.y, color);
		}
		
		// Advance
		penX += glyph.advance.x;
	}
	
	GRenderer->ResetTexture(0);
	
	GRenderer->GetTextureStage(0)->SetColorOp(TextureStage::OpModulate, TextureStage::ArgTexture, TextureStage::ArgCurrent);
	GRenderer->GetTextureStage(0)->SetAlphaOp(TextureStage::ArgTexture);
	
	GRenderer->GetTextureStage(0)->SetWrapMode(TextureStage::WrapRepeat);
	GRenderer->GetTextureStage(0)->SetMinFilter(TextureStage::FilterLinear);
	GRenderer->GetTextureStage(0)->SetMagFilter(TextureStage::FilterLinear);
	
	GRenderer->End2DProjection();
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	GRenderer->SetRenderState(Renderer::DepthWrite, true);
	GRenderer->SetCulling(Renderer::CullCCW);
}

Vec2i Font::GetTextSize(const std::string & str) {
	return GetTextSize(str.begin(), str.end());
}

Vec2i Font::GetTextSize(text_iterator itStart, text_iterator itEnd) {
	
	FT_UInt currentGlyph;
	FT_UInt previousGlyph = 0;
	
	int startX = 0;
	int endX = 0;
	float penX = 0;
	FT_Pos prevRsbDelta = 0;
	
	u32 chr;
	for(text_iterator it = itStart; it != itEnd; ) {
		
		// Get glyph in glyph map
		glyph_iterator itGlyph = getNextGlyph(it, itEnd, chr);
		if(itGlyph == m_Glyphs.end()) {
			continue;
		}
		
		const Glyph & glyph = itGlyph->second;
		
		// Kerning
		if(FT_HAS_KERNING(m_FTFace)) {
			currentGlyph = FT_Get_Char_Index(m_FTFace, chr);
			if(previousGlyph != 0) {
				FT_Vector delta;
				FT_Get_Kerning(m_FTFace, previousGlyph, currentGlyph, FT_KERNING_DEFAULT, &delta);
				penX += delta.x >> 6;
			}
			previousGlyph = currentGlyph;
		}
		
		// Auto hinting adjustments
		if(prevRsbDelta - glyph.lsb_delta >= 32) {
			penX--;
		} else if(prevRsbDelta - glyph.lsb_delta < -32) {
			penX++;
		}
		prevRsbDelta = glyph.rsb_delta;
		
		// If this is the first drawn char, note the start position
		if(startX == endX) {
			startX = glyph.draw_offset.x;
		}
		
		endX = penX + glyph.draw_offset.x + glyph.size.x;
		
		penX += glyph.advance.x;
	}
	
	int sizeX = endX - startX;
	int sizeY = m_FTFace->size->metrics.height >> 6;
	
	return Vec2i(sizeX, sizeY);
}

int Font::GetLineHeight() const {
	return m_FTFace->size->metrics.height >> 6;
}
