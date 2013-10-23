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

#include "graphics/font/Font.h"

#include <sstream>
#include <iomanip>
#include <iterator>

#include <ft2build.h>
#include FT_FREETYPE_H

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

Font::Font(const res::path & fontFile, unsigned int fontSize, FT_Face face) 
	: info(fontFile, fontSize)
	, referenceCount(0)
	, face(face)
	, textures(0) {
	
	// TODO-font: Compute optimal size using m_FTFace->bbox
	const unsigned int TEXTURE_SIZE = 512;
	
	// Insert all the glyphs into texture pages
	textures = new PackedTexture(TEXTURE_SIZE, Image::Format_A8);
	
	// Insert the replacement characters first as they may be needed if others are missing
	insertGlyph('?');
	insertGlyph(util::REPLACEMENT_CHAR);
	
	// Pre-load glyphs for displayable ASCII characters
	for(Char chr = 32; chr < FONT_PRELOAD_LIMIT; ++chr) {
		if(chr != '?') {
			insertGlyph(chr);
		}
	}
	
	textures->upload();
}

Font::~Font() {
	
	delete textures;
	
	// Release FreeType face object.
	FT_Done_Face(face);
}

void Font::insertPlaceholderGlyph(Char character) {
	
	// Glyph does not exist - insert something so we don't look it up for every render pass
	if(character == util::REPLACEMENT_CHAR) {
		
		// Use '?' as a fallback replacement character
		arx_assert(glyphs.find('?') != glyphs.end());
		glyphs[character] = glyphs['?'];
		
	} else if(character < 32 || character == '?') {
		
		// Ignore non-displayable ANSI characters
		Glyph & glyph = glyphs[character];
		glyph.size = Vec2i_ZERO;
		glyph.advance = Vec2f_ZERO;
		glyph.lsb_delta = glyph.rsb_delta = 0;
		glyph.draw_offset = Vec2i_ZERO;
		glyph.uv_start = Vec2f_ZERO;
		glyph.uv_end = Vec2f_ZERO;
		glyph.texture = 0;
		
	} else {
		
		std::ostringstream oss;
		util::writeUTF8(std::ostream_iterator<char>(oss), character);
		LogWarning << "No glyph for character U+" << std::hex << character
		           << " (" << oss.str() << ") in font " << info.name;
		
		arx_assert(glyphs.find(util::REPLACEMENT_CHAR) != glyphs.end());
		glyphs[character] = glyphs[util::REPLACEMENT_CHAR];
		
	}
}

bool Font::insertGlyph(Char character) {
	
	FT_Error error;
	FT_UInt glyphIndex = FT_Get_Char_Index(face, character);
	if(!glyphIndex) {
		insertPlaceholderGlyph(character);
		return false;
	}
	
	error = FT_Load_Glyph(face, glyphIndex, FT_LOAD_FORCE_AUTOHINT);
	if(error) {
		insertPlaceholderGlyph(character);
		return false;
	}
	
	error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
	if(error) {
		insertPlaceholderGlyph(character);
		return false;
	}
	
	// Fill in info for this glyph.
	Glyph & glyph = glyphs[character];
	glyph.index = glyphIndex;
	glyph.size.x = face->glyph->bitmap.width;
	glyph.size.y = face->glyph->bitmap.rows;
	glyph.advance.x = face->glyph->linearHoriAdvance / 65536.0f;
	glyph.advance.y = face->glyph->linearVertAdvance / 65536.0f;
	glyph.lsb_delta = face->glyph->lsb_delta;
	glyph.rsb_delta = face->glyph->rsb_delta;
	glyph.draw_offset.x = face->glyph->bitmap_left;
	glyph.draw_offset.y = face->glyph->bitmap_top - face->glyph->bitmap.rows;
	glyph.uv_start = Vec2f_ZERO;
	glyph.uv_end = Vec2f_ZERO;
	glyph.texture = 0;
	
	// Some glyphs like spaces have a size of 0...
	if(glyph.size.x != 0 && glyph.size.y != 0) {
		
		Image imgGlyph;
		imgGlyph.Create(glyph.size.x, glyph.size.y, Image::Format_A8);
		
		FT_Bitmap * srcBitmap = &face->glyph->bitmap;
		arx_assert(srcBitmap->pitch == srcBitmap->width);
		
		// Copy pixels
		unsigned char * src = srcBitmap->buffer;
		unsigned char * dst = imgGlyph.GetData();
		memcpy(dst, src, glyph.size.x * glyph.size.y);
		
		Vec2i offset;
		if(!textures->insertImage(imgGlyph, glyph.texture, offset)) {
			std::ostringstream oss;
			util::writeUTF8(std::ostream_iterator<char>(oss), character);
			LogWarning << "Could not upload glyph for character U+" << std::hex << character
			           << " (" << oss.str() << ") in font " << info.name;
			insertPlaceholderGlyph(character);
			return false;
		}
		
		// Compute UV mapping for each glyph.
		const float textureSize = textures->getTextureSize();
		glyph.uv_start = Vec2f(offset) / Vec2f(textureSize);
		glyph.uv_end = Vec2f(offset + glyph.size) / Vec2f(textureSize);
	}
	
	return true;
}

bool Font::writeToDisk() {
	
	bool ok = true;
	
	for(unsigned int i = 0; i < textures->getTextureCount(); ++i) {
		Texture2D & tex = textures->getTexture(i);
		
		std::stringstream ss;
		ss << face->family_name;
		if(face->style_name != NULL) {
			ss << "_";
			ss << face->style_name;
		}
		ss << "_";
		ss << info.size;
		ss << "_page";
		ss << i;
		ss << ".png";
		
		ok = ok && tex.GetImage().save(ss.str());
	}
	
	return ok;
}

bool Font::insertMissingGlyphs(text_iterator begin, text_iterator end) {
	
	Char chr;
	bool changed = false;
	
	for(text_iterator it = begin; (chr = util::readUTF8(it, end)) != util::INVALID_CHAR; ) {
		if(glyphs.find(chr) == glyphs.end()) {
			if(chr >= FONT_PRELOAD_LIMIT && insertGlyph(chr)) {
				changed = true;
			}
		}
	}
	
	return changed;
}

Font::glyph_iterator Font::getNextGlyph(text_iterator & it, text_iterator end) {
	
	Char chr = util::readUTF8(it, end);
	if(chr == util::INVALID_CHAR) {
		return glyphs.end();
	}
	
	glyph_iterator glyph = glyphs.find(chr);
	if(glyph != glyphs.end()) {
		return glyph; // an existing glyph
	}
	
	if(chr < FONT_PRELOAD_LIMIT) {
		// We pre-load all glyphs for ASCII characters, so there is no point in checking again
		return glyphs.end();
	}
	
	if(!insertGlyph(chr)) {
		// No new glyph was inserted but the character was mapped to an existing one
		return glyphs.find(chr);
	}
	
	arx_assert(glyphs.find(chr) != glyphs.end());
	
	// As we need to re-upload the textures now, first check for more missing glyphs
	insertMissingGlyphs(it, end);
	
	// Re-upload the changed textures
	textures->upload();
	
	return glyphs.find(chr); // the newly inserted glyph
}

void addGlyphVertices(std::vector<TexturedVertex>& vertices, const Font::Glyph& glyph, const Vec2f& pos, Color color) {

	float w = glyph.size.x;
	float h = -glyph.size.y;
	float uStart = glyph.uv_start.x;
	float vStart = glyph.uv_end.y;
	float uEnd = glyph.uv_end.x;
	float vEnd = glyph.uv_start.y;

	Vec2f p;
	p.x = floor(pos.x + glyph.draw_offset.x) - .5f;
	p.y = floor(pos.y - glyph.draw_offset.y) - .5f;

	TexturedVertex quad[4];
	quad[0].p = Vec3f(p.x, p.y, 0);
	quad[0].uv = Vec2f(uStart, vStart);
	quad[0].color = color.toBGRA();
	quad[0].rhw = 1.0f;

	quad[1].p = Vec3f(p.x + w, p.y, 0);
	quad[1].uv = Vec2f(uEnd, vStart);
	quad[1].color = color.toBGRA();
	quad[1].rhw = 1.0f;

	quad[2].p = Vec3f(p.x + w, p.y + h, 0);
	quad[2].uv = Vec2f(uEnd, vEnd);
	quad[2].color = color.toBGRA();
	quad[2].rhw = 1.0f;

	quad[3].p = Vec3f(p.x, p.y + h, 0);
	quad[3].uv = Vec2f(uStart, vEnd);
	quad[3].color = color.toBGRA();
	quad[3].rhw = 1.0f;

	vertices.push_back(quad[0]);
	vertices.push_back(quad[1]);
	vertices.push_back(quad[2]);

	vertices.push_back(quad[0]);
	vertices.push_back(quad[2]);
	vertices.push_back(quad[3]);
}

template <bool DoDraw>
Vec2i Font::process(int x, int y, text_iterator start, text_iterator end, Color color) {
	
	Vec2f pen(x, y);
		
	int startX = 0;
	int endX = 0;
	
	if(DoDraw) {
		// Subtract one line height (since we flipped the Y origin to be like GDI)
		pen.y += face->size->metrics.ascender >> 6;
	}
	
	FT_UInt prevGlyphIndex = 0;
	FT_Pos prevRsbDelta = 0;

	typedef std::map< unsigned int, std::vector<TexturedVertex> > MapTextureVertices;
	MapTextureVertices mapTextureVertices;
	
	for(text_iterator it = start; it != end; ) {
		
		// Get glyph in glyph map
		glyph_iterator itGlyph = getNextGlyph(it, end);
		if(itGlyph == glyphs.end()) {
			continue;
		}
		const Glyph & glyph = itGlyph->second;
		
		// Kerning
		if(FT_HAS_KERNING(face)) {
			if(prevGlyphIndex != 0) {
				FT_Vector delta;
				FT_Get_Kerning(face, prevGlyphIndex, glyph.index, FT_KERNING_DEFAULT, &delta);
				pen.x += delta.x >> 6;
			}
			prevGlyphIndex = glyph.index;
		}
		
		// Auto hinting adjustments
		if(prevRsbDelta - glyph.lsb_delta >= 32) {
			pen.x--;
		} else if( prevRsbDelta - glyph.lsb_delta < -32) {
			pen.x++;
		}
		prevRsbDelta = glyph.rsb_delta;
		
		// Draw
		if(DoDraw && glyph.size.x != 0 && glyph.size.y != 0) {
			addGlyphVertices(mapTextureVertices[glyph.texture], glyph, pen, color);
		} else {
			ARX_UNUSED(pen), ARX_UNUSED(color);
		}
		
		// If this is the first drawn char, note the start position
		if(startX == endX) {
			startX = glyph.draw_offset.x;
		}
		endX = pen.x + glyph.draw_offset.x + glyph.size.x;
		
		// Advance
		pen.x += glyph.advance.x;
	}
	
	if(DoDraw && !mapTextureVertices.empty()) {

		GRenderer->SetRenderState(Renderer::Lighting, false);
		GRenderer->SetRenderState(Renderer::AlphaBlending, true);
		GRenderer->SetBlendFunc(Renderer::BlendSrcAlpha, Renderer::BlendInvSrcAlpha);

		GRenderer->SetRenderState(Renderer::DepthTest, false);
		GRenderer->SetRenderState(Renderer::DepthWrite, false);
		GRenderer->SetCulling(Renderer::CullNone);

		// 2D projection setup... Put origin (0,0) in the top left corner like GDI...
		Rect viewport = GRenderer->GetViewport();
		GRenderer->Begin2DProjection(viewport.left, viewport.right,
									 viewport.bottom, viewport.top, -1.f, 1.f);

		// Fixed pipeline texture stage operation
		GRenderer->GetTextureStage(0)->setColorOp(TextureStage::ArgDiffuse);
		GRenderer->GetTextureStage(0)->setAlphaOp(TextureStage::ArgTexture);

		GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapClamp);
		GRenderer->GetTextureStage(0)->setMinFilter(TextureStage::FilterNearest);
		GRenderer->GetTextureStage(0)->setMagFilter(TextureStage::FilterNearest);

		for(MapTextureVertices::const_iterator it = mapTextureVertices.begin(); it != mapTextureVertices.end(); ++it) {
			
			if(!it->second.empty()) {
				GRenderer->SetTexture(0, &textures->getTexture(it->first));
				EERIEDRAWPRIM(Renderer::TriangleList, &it->second[0], it->second.size());
			}
		}
					
		GRenderer->ResetTexture(0);
		TextureStage * stage = GRenderer->GetTextureStage(0);
		stage->setColorOp(TextureStage::OpModulate,
		                  TextureStage::ArgTexture, TextureStage::ArgCurrent);
		stage->setAlphaOp(TextureStage::ArgTexture);
		stage->setWrapMode(TextureStage::WrapRepeat);
		stage->setMinFilter(TextureStage::FilterLinear);
		stage->setMagFilter(TextureStage::FilterLinear);
		
		GRenderer->End2DProjection();
		GRenderer->SetRenderState(Renderer::AlphaBlending, false);
		GRenderer->SetRenderState(Renderer::DepthWrite, true);
		GRenderer->SetCulling(Renderer::CullCCW);
		
	}
	
	int sizeX = endX - startX;
	int sizeY = face->size->metrics.height >> 6;
	
	return Vec2i(sizeX, sizeY);
}

void Font::draw(int x, int y, text_iterator start, text_iterator end, Color color) {
	process<true>(x, y, start, end, color);
}

Vec2i Font::getTextSize(text_iterator start, text_iterator end) {
	return process<false>(0, 0, start, end, Color::none);
}

int Font::getLineHeight() const {
	return face->size->metrics.height >> 6;
}
