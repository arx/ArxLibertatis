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

	virtual void Init() = 0;
	virtual void Kill() = 0;

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
        , mHeight(1)
        , mDepth(1)
    {
    }
    
protected:
    Image::Format   mFormat;
    bool            mHasMipmaps;
    unsigned int    mWidth;
    unsigned int    mHeight;
    unsigned int    mDepth;
};


class Texture1D : public Texture
{
public:
    virtual ~Texture1D() {}
    
    bool Create( const Image& pImage, bool pCreateMipmaps = true );
    bool Create( unsigned int pWidth, Image::Format pFormat );

    Image&  GetImage();
    void    Update();

protected:
    Texture1D() {}

    Image   mImage;
};


class Texture2D : public Texture
{
public:
    virtual ~Texture2D() {}

    bool Create( const Image& pImage, bool pCreateMipmaps = true );
    bool Create( unsigned int pWidth, unsigned int pHeight, Image::Format pFormat );

    Image&  GetImage();
    void    Update();

protected:
    Texture2D() {} 

    Image   mImage;
};


class Texture3D : public Texture
{
public:
    virtual ~Texture3D() {}

    bool Create( const Image& pImage, bool pCreateMipmaps = true );
    bool Create( unsigned int pWidth, unsigned int pHeight, unsigned int pDepth, Image::Format pFormat );
       
    Image&  GetImage();
    void    Update();

protected:
    Texture3D() {}

    Image   mImage;
};


class Cubemap : public Texture
{
public:
    enum CubemapFace
    {
        PositiveX,
        NegativeX,
        PositiveY,
        NegativeY,
        PositiveZ,
        NegativeZ,
        NumFaces
    };

public:
    virtual ~Cubemap() {}

    bool Create( const std::vector<Image*>& pImages, bool pCreateMipmaps = true );
    bool Create( unsigned int pWidth, unsigned int pHeight, Image::Format pFormat );

    Image&  GetImage( CubemapFace pFace );
    void    Update();

protected:
    Cubemap() {}   

    Image   mImages[NumFaces];
};


#endif // _TEXTURE_H_
