#ifndef _TEXTURE_H_
#define _TEXTURE_H_


#include "graphics/image/Image.h"

#include <vector>

class Texture
{
public:
    virtual ~Texture()
    {
    }

	virtual void Upload() = 0;
	virtual void Destroy() = 0;

    unsigned int GetWidth() const
    {
        return mWidth;
    }

    unsigned int GetHeight() const
    {
        return mHeight; 
    }

    unsigned int GetDepth() const
    {
        return mDepth; 
    }

    Image::Format GetFormat() const
    {
        return mFormat;
    }

    bool HasMipmaps() const
    {
        return mHasMipmaps;
    }
    
protected:
    Texture()
        : mFormat(Image::Format_Unknown)
        , mHasMipmaps(false)
        , mWidth(0)
        , mHeight(0)
        , mDepth(0)
    {
    }

	virtual bool Create() = 0;
    
protected:
    Image::Format   mFormat;
    bool            mHasMipmaps;
    unsigned int    mWidth;
    unsigned int    mHeight;
    unsigned int    mDepth;
};

class Texture2D : public Texture
{
public:
    virtual ~Texture2D() 
	{
	}

	bool Init( const std::string& strFileName );
    bool Init( const Image& image, bool createMipmaps = true );
    bool Init( unsigned int width, unsigned int height, Image::Format format );

	bool Restore();

    Image& GetImage();

protected:
	Texture2D()
	{
	} 

    Image			mImage;
	std::string		mFileName;
};


#endif // _TEXTURE_H_
