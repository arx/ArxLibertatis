
#ifndef ARX_GRAPHIC_TEXTURE_TEXTURE_H
#define ARX_GRAPHIC_TEXTURE_TEXTURE_H

#include <vector>

#include "graphics/image/Image.h"
#include "io/FilePath.h"
#include "math/Vector2.h"

class Texture {
	
public:
	
	virtual ~Texture() { }
	
	virtual void Upload() = 0;
	virtual void Destroy() = 0;
	
	const Vec2i & getSize() const { return size; }
	const Vec2i & getStoredSize() const { return storedSize; }
	
	unsigned int GetDepth() const { return mDepth; }
	
	Image::Format GetFormat() const { return mFormat; }
	
	bool HasMipmaps() const { return mHasMipmaps; }
	
protected:
	
	Texture() : mFormat(Image::Format_Unknown), mHasMipmaps(false), size(Vec2i::ZERO), storedSize(Vec2i::ZERO), mDepth(0) { }
	
	virtual bool Create() = 0;
	
	Image::Format mFormat;
	bool mHasMipmaps;
	
	Vec2i size;
	Vec2i storedSize;
	
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
