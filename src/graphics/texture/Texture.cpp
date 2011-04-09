#include "graphics/texture/Texture.h"

#include "graphics/Renderer.h"

bool Texture2D::Init( const std::string& strFileName )
{
	mFileName = strFileName;

	bool bLoaded = mImage.LoadFromFile(strFileName);
	if(!bLoaded)
	{
		mFileName = "";
		return false;
	}

	mFileName = strFileName;
	mFormat = mImage.GetFormat();
    mWidth  = mImage.GetWidth();
    mHeight = mImage.GetHeight();
	
	bool bCreated = Create();
	if(!bCreated)
		return false;

	Upload();

	return true;
}

bool Texture2D::Init( const Image& pImage, bool pCreateMipmaps )
{
	mFileName = "";

    mImage  = pImage;
    mFormat = pImage.GetFormat();
    mWidth  = pImage.GetWidth();
    mHeight = pImage.GetHeight();
    mHasMipmaps = pCreateMipmaps;

	bool bCreated = Create();
	if(!bCreated)
		return false;

	Upload();
    return true;
}

bool Texture2D::Init( unsigned int pWidth, unsigned int pHeight, Image::Format pFormat )
{
	mFileName = "";

	mImage.Create(pWidth, pHeight, pFormat);
    mWidth  = pWidth;
    mHeight = pHeight;
    mFormat = pFormat;
    mHasMipmaps = false;

	return Create();
}

bool Texture2D::Restore()
{
	if(mImage.IsValid())
	{
		bool bCreated = Create();
		if(!bCreated)
			return false;

		Upload();
	}
	else if(!mFileName.empty())
	{
		bool bCreated = Create();
		if(!bCreated)
			return false;

		mImage.LoadFromFile(mFileName);
		Upload();
		mImage.Reset();
	}
	else
	{
		return false;
	}
    
	return true;
}

Image& Texture2D::GetImage()
{
    return mImage;
}
