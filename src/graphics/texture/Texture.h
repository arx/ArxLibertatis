
#ifndef ARX_GRAPHIC_TEXTURE_TEXTURE_H
#define ARX_GRAPHIC_TEXTURE_TEXTURE_H

#include <vector>

#include "graphics/image/Image.h"
#include "io/FilePath.h"
#include "math/Vector2.h"
#include "platform/Flags.h"

class Texture {
	
public:
	
	enum TextureFlag {
		HasMipmaps  = (1<<0),
		HasColorKey = (1<<1)
	};
	DECLARE_FLAGS(TextureFlag, TextureFlags)
	
	virtual ~Texture() { }
	
	virtual void Upload() = 0;
	virtual void Destroy() = 0;
	
	const Vec2i & getSize() const { return size; }
	const Vec2i & getStoredSize() const { return storedSize; }
	
	unsigned int GetDepth() const { return mDepth; }
	
	Image::Format GetFormat() const { return mFormat; }
	
	inline bool hasMipmaps() const { return (flags & HasMipmaps) == HasMipmaps; }
	
protected:
	
	Texture() : mFormat(Image::Format_Unknown), flags(0), size(Vec2i::ZERO), storedSize(Vec2i::ZERO), mDepth(0) { }
	
	virtual bool Create() = 0;
	
	Image::Format mFormat;
	TextureFlags flags;
	
	Vec2i size;
	Vec2i storedSize;
	
	unsigned int mDepth;
	
};

DECLARE_FLAGS_OPERATORS(Texture::TextureFlags)

class Texture2D : public Texture {
	
public:
	
	virtual ~Texture2D() { }
	
	bool Init(const fs::path & strFileName, TextureFlags flags = HasColorKey);
	bool Init(const Image & image, TextureFlags flags = HasMipmaps);
	bool Init(unsigned int width, unsigned int height, Image::Format format);
	
	bool Restore();
	
	inline Image & GetImage() { return mImage; }
	inline const fs::path & getFileName() { return mFileName; }
	
protected:
	
	Texture2D() { } 
	
	Image mImage;
	fs::path mFileName;
	
};

#endif // ARX_GRAPHIC_TEXTURE_TEXTURE_H
