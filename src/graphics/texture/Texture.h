#ifndef _TEXTURE_H_
#define _TEXTURE_H_


#include "graphics/image/Image.h"

#include <vector>

class Texture
{
public:
    enum WrapAxis
    {
        Wrap_S,
        Wrap_T,
        Wrap_AxisNum,
        Wrap_AxisMAX = 0xFFFFFFFF
    };

    enum WrapMode
    {
        Wrap_Clamp,
        Wrap_Repeat,
        Wrap_ModeNum,
        Wrap_ModeMAX = 0xFFFFFFFF
    };

    enum MagFilter
    {
        MagFilter_Nearest,
        MagFilter_Linear,
        MagFilter_Num,
        MagFilter_MAX = 0xFFFFFFFF
    };

    enum MinFilter
    {
        MinFilter_Nearest,
        MinFilter_Linear,
        MinFilter_NearestMipmapNearest,
        MinFilter_LinearMipmapNearest,
        MinFilter_NearestMipmapLinear,
        MinFilter_LinearMipmapLinear,
        MinFilter_Num,
        MinFilter_MAX = 0xFFFFFFFF
    };

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
        
    virtual void SetWrapMode( WrapAxis pWrapAxis, WrapMode pWrapMode )
    {
        mWrapMode[pWrapAxis] = pWrapMode;        
    }
    
    virtual void SetMagFilter( MagFilter pFilter )
    {
        mMagFilter = pFilter;
    }
    
    virtual void SetMinFilter( MinFilter pFilter )
    {
        mMinFilter = pFilter;
    }

    virtual void SetAnisotropy( float pAnisotropy )
    {
        mAnisotropy = pAnisotropy;
    }

    bool HasMipmaps() const
    {
        return mHasMipmaps;
    }

    MinFilter GetMinFilter() const
    {
        return mMinFilter;
    }

    MagFilter GetMagFilter() const
    {
        return mMagFilter;
    }

    WrapMode GetWrapMode( WrapAxis pWrapAxis ) const
    {
        return mWrapMode[pWrapAxis];
    }

    float GetAnisotropy() const
    {
        return mAnisotropy;
    }
    
protected:
    Texture()
        : mFormat(Image::Format_Unknown)
        , mHasMipmaps(false)
        , mMinFilter(MinFilter_NearestMipmapLinear)
        , mMagFilter(MagFilter_Linear)
        , mWidth(0)
        , mHeight(1)
        , mDepth(1)
        , mAnisotropy(0.0f)
    {
        mWrapMode[Wrap_S] = Wrap_Repeat;
        mWrapMode[Wrap_T] = Wrap_Repeat;
    }
    
protected:
    Image::Format   mFormat;
    bool            mHasMipmaps;
    MinFilter       mMinFilter;
    MagFilter       mMagFilter;
    WrapMode        mWrapMode[2];    
    unsigned int    mWidth;
    unsigned int    mHeight;
    unsigned int    mDepth;
    float           mAnisotropy;
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
