#ifndef _FONT_H_
#define _FONT_H_

#include "core/math/Vector2.h"

#include <windows.h>	// TEMP... needed for COLORREF
#include <string>
#include <map>



class Font
{
	friend class FontCache;

public:
	struct Info
	{
		Info( const std::string& fontFile, unsigned int fontSize ) : m_Name(fontFile), m_Size(fontSize) {}

        bool operator == ( const Info& pOther ) const { return m_Name == pOther.m_Name && m_Size == pOther.m_Size; }
		bool operator < ( const Info& pOther ) const  { return m_Name == pOther.m_Name ? m_Size < pOther.m_Size : m_Name < pOther.m_Name; }
        
		std::string     m_Name;
        unsigned int    m_Size;
	};
    
	//! Representation of a glyph.
    struct Glyph
    {
        Vector2i		size;           //!< Size of the glyph.
        Vector2i		draw_offset;    //!< Offset to use when drawing.
        Vector2f		advance;        //!< Pen advance after write this glyph.

        Vector2f		uv_start;       //!< UV coordinates.
        Vector2f		uv_end;         //!< UV coordinates.

		unsigned int	texture;        //!< Texture page on which the glyph can be found.
    };

public:
	const Info&	GetInfo() const { return m_Info; }
	const std::string& GetName() const { return m_Info.m_Name; }
	unsigned int GetSize() const { return m_Info.m_Size; }

	void Draw( int pX, int pY, const std::string& str, COLORREF color ) const;
	void Draw( int pX, int pY, std::string::const_iterator itStart, std::string::const_iterator itEnd, COLORREF color ) const;

	Vector2i GetTextSize( const std::string& str ) const;
	Vector2i GetTextSize( std::string::const_iterator itStart, std::string::const_iterator itEnd ) const;

	Vector2i GetCharSize( unsigned int character ) const;
	int		 GetLineHeight() const;

	// For debugging purpose... will write one image file per page
	// under "name_style_size_pagen.png"
	void WriteToDisk();

private:
	// Construction/destruction handled by FontCache only
	Font(const std::string& fontFile, unsigned int fontSize, struct FT_FaceRec_* face);
	~Font();

	// Disable copy of Font objects
    Font( const Font& pOther );
    const Font& operator = ( const Font& pOther );

	bool InsertGlyph( unsigned int character );
	
private:
	Info							m_Info;
	unsigned int					m_RefCount;

	struct FT_FaceRec_*				m_FTFace;
	std::map<unsigned int, Glyph>	m_Glyphs;

	class PackedTexture*			m_Textures;
};


#endif // _FONT_H_
