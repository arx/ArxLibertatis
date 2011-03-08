#ifndef _FONT_CACHE_H_
#define _FONT_CACHE_H_

#include <string>
#include <map>

#include "graphics/font/Font.h"


class FontCache
{
public:
	static void Initialize();
	static void Shutdown();

	static Font* GetFont( const std::string& fontFile, unsigned int fontSize );
    static void ReleaseFont( Font* pFont );
    
protected:
    // Disable creation from outside.
    FontCache();
	~FontCache();

	Font* Create( const std::string& fontFile, unsigned int fontSize );

private:
	typedef std::map<Font::Info,Font*> FontMap;

    FontMap				m_LoadedFonts;
    static FontCache*	m_Instance;
};


#endif // _FONT_CACHE_H_
