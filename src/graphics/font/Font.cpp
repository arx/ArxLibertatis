#include "graphics/font/Font.h"

#include "graphics/Renderer.h"
#include "graphics/texture/Texture.h"
#include "graphics/texture/PackedTexture.h"

#include <sstream>
#include <ft2build.h>
#include FT_FREETYPE_H


Font::Font( const std::string& fontFile, unsigned int fontSize, FT_Face face ) 
	: m_Info(fontFile, fontSize)
	, m_RefCount(0)
	, m_Textures(0)
	, m_FTFace(face)
{
	// TODO-font: Compute optimal size using m_FTFace->bbox
	const unsigned int TEXTURE_SIZE = 512;

	// Insert all the glyphs into texture pages
	m_Textures = new PackedTexture(TEXTURE_SIZE, Image::Format_A8);
	m_Textures->BeginPacking();

    for( unsigned int car = 32; car < 256; ++car )
    {
        InsertGlyph(car);
    }    
	
	m_Textures->EndPacking();
}

Font::~Font()
{
	delete m_Textures;

	// Release FreeType face object.
    FT_Done_Face( m_FTFace );
}

bool Font::InsertGlyph( unsigned int character )
{
	FT_Error error;
	FT_UInt glyphIndex = FT_Get_Char_Index( m_FTFace, character );

    error = FT_Load_Glyph( m_FTFace, glyphIndex, FT_LOAD_DEFAULT );
    if( error )
		return false;

    error = FT_Render_Glyph( m_FTFace->glyph, FT_RENDER_MODE_NORMAL );
    if( error )
		return false;

	// Fill in info for this glyph.
	Font::Glyph& glyph	 = m_Glyphs[character];
    glyph.size.x         = m_FTFace->glyph->bitmap.width;
    glyph.size.y         = m_FTFace->glyph->bitmap.rows;
	glyph.advance.x      = m_FTFace->glyph->linearHoriAdvance / 65535.0f;
	glyph.advance.y      = m_FTFace->glyph->linearVertAdvance / 65535.0f;
    glyph.draw_offset.x  = m_FTFace->glyph->bitmap_left;
    glyph.draw_offset.y  = m_FTFace->glyph->bitmap_top - m_FTFace->glyph->bitmap.rows;
	glyph.uv_start       = Vector2f(0,0);
	glyph.uv_end		 = Vector2f(0,0);
	glyph.texture		 = 0;

	// Some glyphs like spaces have a size of 0...
	if(glyph.size.x != 0 && glyph.size.y != 0)
	{
		Image imgGlyph;
		imgGlyph.Create( glyph.size.x, glyph.size.y, Image::Format_A8 );
		
		FT_Bitmap* srcBitmap = &m_FTFace->glyph->bitmap;
		arx_assert(srcBitmap->pitch == srcBitmap->width);

		// Copy pixels
		unsigned char* src = srcBitmap->buffer;
		unsigned char* dst = imgGlyph.GetData();
		memcpy(dst, src, glyph.size.x * glyph.size.y);

		int glyphPosX;
		int glyphPosY;
		m_Textures->InsertImage( imgGlyph, glyphPosX, glyphPosY, glyph.texture );

		// Compute UV mapping for each glyph.
		const float TEXTURE_SIZE = m_Textures->GetTextureSize();
		glyph.uv_start.x = glyphPosX / TEXTURE_SIZE;
		glyph.uv_start.y = glyphPosY / TEXTURE_SIZE;
		glyph.uv_end.x   = (glyphPosX + glyph.size.x) / TEXTURE_SIZE;
		glyph.uv_end.y   = (glyphPosY + glyph.size.y) / TEXTURE_SIZE;
	}

	return true;
}

void Font::WriteToDisk()
{
	for(unsigned int i = 0; i < m_Textures->GetTextureCount(); ++i)
	{
		Texture2D& tex = m_Textures->GetTexture(i);

		std::stringstream ss;
		ss << m_FTFace->family_name;
		if(m_FTFace->style_name != NULL)
		{
			ss << "_";
			ss << m_FTFace->style_name;
		}
		ss << "_";
		ss << m_Info.m_Size;
		ss << "_Page";
		ss << i;
		ss << ".png";

		tex.GetImage().Dump(ss.str());
	}
}

void Font::Draw( int x, int y, const std::string& str, COLORREF color ) const
{
    GRenderer->SetRenderState( Renderer::Lighting, false );
    GRenderer->SetRenderState( Renderer::Blend, true );
    GRenderer->SetBlendFunc( Renderer::BlendSrcAlpha, Renderer::BlendInvSrcAlpha );
	
	GRenderer->SetRenderState( Renderer::DepthTest, false );
	GRenderer->SetRenderState( Renderer::DepthMask, false );
	GRenderer->SetCulling( Renderer::CullNone );

	// 2D projection setup... Put origin (0,0) in the top left corner like GDI... 
    int viewport[4];
    GRenderer->GetViewport( viewport );
	GRenderer->Begin2DProjection( viewport[0], viewport[2], viewport[3], viewport[1], -1, 1 );

	// Fixed pipeline texture stage operation
	GRenderer->GetTextureStage(0)->SetColorOp(TextureStage::TexOpSelectArg1, TextureStage::TexArgDiffuse, TextureStage::TexArgCurrent);
	GRenderer->GetTextureStage(0)->SetAlphaOp(TextureStage::TexOpSelectArg1, TextureStage::TexArgTexture, TextureStage::TexArgCurrent);

    unsigned int penX = x;
    unsigned int penY = y;

	// Substract one line height (since we flipped the Y origin to be like GDI)
	penY += m_FTFace->size->metrics.height >> 6;	

	FT_UInt currentGlyph;
	FT_UInt previousGlyph = 0;

    for( unsigned int car = 0; car < str.size(); ++car )
    {
        std::map<unsigned int, Glyph>::const_iterator it = m_Glyphs.find( str[car] );
		if(it == m_Glyphs.end())
			continue;

		const Glyph& glyph = (*it).second;

		if( FT_HAS_KERNING(m_FTFace) )
		{
			currentGlyph = FT_Get_Char_Index( m_FTFace, str[car] );
			if(previousGlyph != 0)
			{
				FT_Vector delta;
				FT_Get_Kerning( m_FTFace, previousGlyph, currentGlyph, FT_KERNING_DEFAULT, &delta );
				penX += delta.x >> 6;
			}
			previousGlyph = currentGlyph;
		}

		GRenderer->SetTexture( 0, m_Textures->GetTexture(glyph.texture) );
        GRenderer->DrawTexturedRect( penX + glyph.draw_offset.x, penY - glyph.draw_offset.y, glyph.size.x, -glyph.size.y, glyph.uv_start.x, glyph.uv_end.y, glyph.uv_end.x, glyph.uv_start.y, color );

        penX += glyph.advance.x;
    }

    GRenderer->ResetTexture(0);

	GRenderer->GetTextureStage(0)->SetColorOp(TextureStage::TexOpModulate, TextureStage::TexArgTexture, TextureStage::TexArgCurrent);
	GRenderer->GetTextureStage(0)->SetAlphaOp(TextureStage::TexOpSelectArg1, TextureStage::TexArgTexture, TextureStage::TexArgCurrent);
	
    GRenderer->End2DProjection();
    GRenderer->SetRenderState( Renderer::Blend, false );
	GRenderer->SetRenderState( Renderer::DepthTest, true );
	GRenderer->SetRenderState( Renderer::DepthMask, true );
	GRenderer->SetCulling( Renderer::CullCW );
}

Vector2i Font::GetTextSize( const std::string& str ) const
{
	FT_UInt currentGlyph;
	FT_UInt previousGlyph = 0;

    unsigned int penX = 0;
    Vector2i size(0, m_FTFace->size->metrics.height >> 6);

	unsigned int numCar = str.size();
    for( unsigned int car = 0; car < numCar; ++car )
    {
        std::map<unsigned int, Glyph>::const_iterator it = m_Glyphs.find( str[car] );
		if(it == m_Glyphs.end())
			continue;

		const Glyph& glyph = (*it).second;

		if( FT_HAS_KERNING(m_FTFace) )
		{
			currentGlyph = FT_Get_Char_Index( m_FTFace, str[car] );
			if(previousGlyph != 0)
			{
				FT_Vector delta;
				FT_Get_Kerning( m_FTFace, previousGlyph, currentGlyph, FT_KERNING_DEFAULT, &delta );
				penX += delta.x >> 6;
			}
			previousGlyph = currentGlyph;
		}

		if( car == 0 )
            size.x = glyph.draw_offset.x;

        if( car == numCar - 1 )
            size.x = (penX + glyph.draw_offset.x + glyph.size.x) - size.x;
		
        penX += glyph.advance.x;
    }

    return size;
}

Vector2i Font::GetCharSize( unsigned int character ) const
{	
	unsigned int sizeY = m_FTFace->size->metrics.height >> 6;

	std::map<unsigned int, Glyph>::const_iterator it = m_Glyphs.find( character );
	if(it == m_Glyphs.end())
		return Vector2i(0, sizeY);

	const Glyph& glyph = (*it).second;
	return Vector2i(glyph.draw_offset.x + glyph.size.x, glyph.draw_offset.y + sizeY);
}

