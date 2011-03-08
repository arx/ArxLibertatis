#include "graphics/texture/Texture.h"

#include "graphics/Renderer.h"


bool Texture1D::Create( const Image& pImage, bool pCreateMipmaps )
{
    arx_assert_msg( pImage.GetHeight() == 1, "[Texture1D::Create] 1D texture must be created using images of height == 1" );
    if( pImage.GetHeight() != 1 )
        return false;

    mImage  = pImage;
    mFormat = pImage.GetFormat();
    mWidth  = pImage.GetWidth();
    mHasMipmaps = pCreateMipmaps;

    // Filtering
    if( mHasMipmaps )
    {
        mMagFilter = MagFilter_Linear; 
        mMinFilter = MinFilter_LinearMipmapLinear;
    }
    else
    {
        mMagFilter = MagFilter_Linear; 
        mMinFilter = MinFilter_Linear;
    }

    mAnisotropy = GRenderer->GetMaxAnisotropy();

    return true;
}

bool Texture1D::Create( unsigned int pWidth, Image::Format pFormat)
{
    mImage  = Image();
    mWidth  = pWidth;
    mFormat = pFormat;
    mHasMipmaps = false;

    // Filtering
    mMagFilter = MagFilter_Linear; 
    mMinFilter = MinFilter_Linear;

    mAnisotropy = GRenderer->GetMaxAnisotropy();

    return true;
}

Image& Texture1D::GetImage()
{
    return mImage;
}

void Texture1D::Update()
{
    Create( mImage, mHasMipmaps );
    Init();
}

bool Texture2D::Create( const Image& pImage, bool pCreateMipmaps )
{
    mImage  = pImage;
    mFormat = pImage.GetFormat();
    mWidth  = pImage.GetWidth();
    mHeight = pImage.GetHeight();
    mHasMipmaps = pCreateMipmaps;

    // Filtering
    if( mHasMipmaps )
    {
        mMagFilter = MagFilter_Linear; 
        mMinFilter = MinFilter_LinearMipmapLinear;
    }
    else
    {
        mMagFilter = MagFilter_Linear; 
        mMinFilter = MinFilter_Linear;
    }

    mAnisotropy = GRenderer->GetMaxAnisotropy();

    return true;
}

bool Texture2D::Create( unsigned int pWidth, unsigned int pHeight, Image::Format pFormat )
{
    mImage  = Image();
    mWidth  = pWidth;
    mHeight = pHeight;
    mFormat = pFormat;
    mHasMipmaps = false;

    // Filtering
    mMagFilter = MagFilter_Linear; 
    mMinFilter = MinFilter_Linear;

    mAnisotropy = GRenderer->GetMaxAnisotropy();

    return true;
}

Image& Texture2D::GetImage()
{
    return mImage;
}

void Texture2D::Update()
{
    Create( mImage, mHasMipmaps );
    Init();
}

bool Texture3D::Create( const Image& pImage, bool pCreateMipmaps )
{
    mImage  = pImage;
    mFormat = pImage.GetFormat();
    mWidth  = pImage.GetWidth();
    mHeight = pImage.GetHeight();
    mDepth  = pImage.GetDepth();
    mHasMipmaps = pCreateMipmaps;

    // Filtering
    if( mHasMipmaps )
    {
        mMagFilter = MagFilter_Linear; 
        mMinFilter = MinFilter_LinearMipmapLinear;
    }
    else
    {
        mMagFilter = MagFilter_Linear; 
        mMinFilter = MinFilter_Linear;
    }

    mAnisotropy = GRenderer->GetMaxAnisotropy();

    return true;
}

bool Texture3D::Create( unsigned int pWidth, unsigned int pHeight, unsigned int pDepth, Image::Format pFormat )
{
    mImage  = Image();
    mWidth  = pWidth;
    mHeight = pHeight;
    mDepth  = pDepth;
    mFormat = pFormat;
    mHasMipmaps = false;

    // Filtering
    mMagFilter = MagFilter_Linear; 
    mMinFilter = MinFilter_Linear;

    mAnisotropy = GRenderer->GetMaxAnisotropy();

    return true;
}

Image& Texture3D::GetImage()
{
    return mImage;
}

void Texture3D::Update()
{
    Create( mImage, mHasMipmaps );
    Init();
}

bool Cubemap::Create( const std::vector<Image*>& pImages, bool pCreateMipmaps )
{
    // User must provide 6 images.
    arx_assert_msg( pImages.size() == Cubemap::NumFaces, "[Cubemap::Create] Six images must be given to create a cubemap." );
    if( pImages.size() != Cubemap::NumFaces )
        return false;

    Image::Format format = pImages[PositiveX]->GetFormat();
    unsigned int        width  = pImages[PositiveX]->GetWidth();
    unsigned int        height = pImages[PositiveX]->GetHeight();

    arx_assert_msg( width == height, "[Cubemap::Create] Square images must be used for the cubemap faces." );

    // Validate that all the face are of the same dimension/format.
    for( unsigned int i = 0; i < Cubemap::NumFaces; i++ )
    {
        arx_assert_msg( pImages[i]->GetFormat() == format && pImages[i]->GetWidth() == width && pImages[i]->GetHeight() == height, 
            "[Cubemap::Create] The Six cubemap images must be of the same dimension and format." );

        if( pImages[i]->GetFormat() != format || pImages[i]->GetWidth() != width || pImages[i]->GetHeight() != height )
            return false;
    }

    mImages[PositiveX] = *pImages[PositiveX];
    mImages[NegativeX] = *pImages[NegativeX];
    mImages[PositiveY] = *pImages[PositiveY];
    mImages[NegativeY] = *pImages[NegativeY];
    mImages[PositiveZ] = *pImages[PositiveZ];
    mImages[NegativeZ] = *pImages[NegativeZ];

    mFormat = format;
    mWidth  = width;
    mHeight = height;
    mHasMipmaps = pCreateMipmaps;

    // Filtering
    if( mHasMipmaps && mImages[PositiveX].GetNumMipmaps() > 1 )
    {
        mMagFilter = MagFilter_Linear; 
        mMinFilter = MinFilter_LinearMipmapLinear;
    }
    else
    {
        mMagFilter = MagFilter_Linear; 
        mMinFilter = MinFilter_Linear;
    }

    mAnisotropy = GRenderer->GetMaxAnisotropy();

    return true;
}

bool Cubemap::Create( unsigned int pWidth, unsigned int pHeight, Image::Format pFormat )
{
    mImages[PositiveX] = Image();
    mImages[NegativeX] = Image();
    mImages[PositiveY] = Image();
    mImages[NegativeY] = Image();
    mImages[PositiveZ] = Image();
    mImages[NegativeZ] = Image();

    mWidth  = pWidth;
    mHeight = pHeight;
    mFormat = pFormat;
    mHasMipmaps = false;

    // Filtering
    mMagFilter = MagFilter_Linear; 
    mMinFilter = MinFilter_Linear;

    mAnisotropy = GRenderer->GetMaxAnisotropy();

    return true;
}

Image& Cubemap::GetImage( CubemapFace pFace )
{
    arx_assert_msg( pFace > 0 && pFace < 6, "[Cubemap::GetImage] Invalid face given." );
    return mImages[pFace];
}

void Cubemap::Update()
{
    std::vector<Image*> vectImages;
    vectImages.push_back( &mImages[PositiveX] );
    vectImages.push_back( &mImages[NegativeX] );
    vectImages.push_back( &mImages[PositiveY] );
    vectImages.push_back( &mImages[NegativeY] );
    vectImages.push_back( &mImages[PositiveZ] );
    vectImages.push_back( &mImages[NegativeZ] );

    Create( vectImages, mHasMipmaps );
    Init();
}
