
#include "graphics/texture/Texture.h"

#include "graphics/Renderer.h"

bool Texture2D::Init(const fs::path & strFileName, bool pCreateMipmaps) {
	
	mFileName = strFileName;
	mHasMipmaps = pCreateMipmaps;
	return Restore();
}

bool Texture2D::Init(const Image & pImage, bool pCreateMipmaps) {
	
	mFileName.clear();
	mImage = pImage;
	mHasMipmaps = pCreateMipmaps;
	return Restore();
}

bool Texture2D::Init(unsigned int pWidth, unsigned int pHeight, Image::Format pFormat) {
	
	mFileName.clear();
	
	mImage.Create(pWidth, pHeight, pFormat);
	mWidth  = pWidth;
	mHeight = pHeight;
	mFormat = pFormat;
	mHasMipmaps = false;
	
	return Create();
}

bool Texture2D::Restore() {
	
	bool bRestored = false;

	if(!mFileName.empty()) {
		mImage.LoadFromFile(mFileName);

		// Original arx only applied color keying to bmp textures...
		if(mFileName.ext() == ".bmp")
			mImage.ApplyColorKeyToAlpha();		
	}

	if(mImage.IsValid()) {
		mFormat = mImage.GetFormat();
		mWidth  = mImage.GetWidth();
		mHeight = mImage.GetHeight();

		bool bCreated = Create();
		if(!bCreated) {
			return false;
		}

		Upload();

		bRestored = true;
	} 

	if(!mFileName.empty()) {
		mImage.Reset();
	}
	
	return bRestored;
}
