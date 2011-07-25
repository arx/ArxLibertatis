
#ifndef ARX_GRAPHIC_TEXTURE_TEXTURE_H
#define ARX_GRAPHIC_TEXTURE_TEXTURE_H

#include <vector>

#include "graphics/image/Image.h"
#include "io/FilePath.h"

class Texture {
	
public:
	
	virtual ~Texture() { }
	
	virtual void Upload() = 0;
	virtual void Destroy() = 0;
	
	unsigned int GetWidth() const { return mWidth; }
	
	unsigned int GetHeight() const { return mHeight; }
	
	unsigned int GetDepth() const { return mDepth; }
	
	Image::Format GetFormat() const { return mFormat; }
	
	bool HasMipmaps() const { return mHasMipmaps; }
	
protected:
	
	Texture() : mFormat(Image::Format_Unknown), mHasMipmaps(false), mWidth(0), mHeight(0), mDepth(0) { }
	
	virtual bool Create() = 0;
	
	Image::Format mFormat;
	bool mHasMipmaps;
	unsigned int mWidth;
	unsigned int mHeight;
	unsigned int mDepth;
	
};

class Texture2D : public Texture {
	
public:
	
	virtual ~Texture2D() { }
	
	bool Init(const fs::path & strFileName, bool pCreateMipmaps = false);
	bool Init(const Image & image, bool createMipmaps = true);
	bool Init(unsigned int width, unsigned int height, Image::Format format);
	
	bool Restore();
	
	inline Image & GetImage() { return mImage; }
	
protected:
	
	Texture2D() { } 
	
	Image mImage;
	fs::path mFileName;
	
};

#endif // ARX_GRAPHIC_TEXTURE_TEXTURE_H
