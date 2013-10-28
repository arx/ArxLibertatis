/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ARX_GRAPHICS_TEXTURE_TEXTURE_H
#define ARX_GRAPHICS_TEXTURE_TEXTURE_H

#include "graphics/image/Image.h"
#include "io/resource/ResourcePath.h"
#include "math/Vector.h"
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
	inline bool hasColorKey() const { return (flags & HasColorKey) == HasColorKey; }
	
protected:
	
	Texture() : mFormat(Image::Format_Unknown), flags(0), size(Vec2i_ZERO), storedSize(Vec2i_ZERO), mDepth(0) { }
	
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
	
	bool Init(const res::path & strFileName, TextureFlags flags = HasColorKey);
	bool Init(const Image & image, TextureFlags flags = HasMipmaps);
	bool Init(unsigned int width, unsigned int height, Image::Format format);
	
	bool Restore();
	
	inline Image & GetImage() { return mImage; }
	inline const res::path & getFileName() { return mFileName; }
	
protected:
	
	Texture2D() { } 
	
	Image mImage;
	res::path mFileName;
	
};

#endif // ARX_GRAPHICS_TEXTURE_TEXTURE_H
