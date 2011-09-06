
#include "graphics/texture/Texture.h"

#include "graphics/Renderer.h"

bool Texture2D::Init(const fs::path & strFileName, TextureFlags newFlags) {
	
	mFileName = strFileName;
	flags = newFlags;
	return Restore();
}

bool Texture2D::Init(const Image & pImage, TextureFlags newFlags) {
	
	mFileName.clear();
	mImage = pImage;
	flags = newFlags;
	return Restore();
}

bool Texture2D::Init(unsigned int pWidth, unsigned int pHeight, Image::Format pFormat) {
	
	mFileName.clear();
	
	size = Vec2i(pWidth, pHeight);	
	mImage.Create(pWidth, pHeight, pFormat);
	mFormat = pFormat;
	flags = 0;
	
	return Create();
}

bool Texture2D::Restore() {
	
	bool bRestored = false;

	if(!mFileName.empty()) {
		mImage.LoadFromFile(mFileName);

		if((flags & HasColorKey) && !mImage.HasAlpha()) {
			mImage.ApplyColorKeyToAlpha();
		}
	}

	if(mImage.IsValid()) {
		mFormat = mImage.GetFormat();
		size = Vec2i(mImage.GetWidth(), mImage.GetHeight());

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
