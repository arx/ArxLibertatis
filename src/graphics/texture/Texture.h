/*
 * Copyright 2011-2016 Arx Libertatis Team (see the AUTHORS file)
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
#include "util/Flags.h"

class Texture {
	
public:
	
	enum TextureFlag {
		HasMipmaps  = (1<<0),
		ApplyColorKey = (1<<1),
		Intensity   = (1<<2),
	};
	DECLARE_FLAGS(TextureFlag, TextureFlags)
	
	virtual ~Texture() { }
	
	virtual void Upload() = 0;
	virtual void Destroy() = 0;
	
	const Vec2i & getSize() const { return size; }
	const Vec2i & getStoredSize() const { return storedSize; }
	
	Image::Format GetFormat() const { return mFormat; }
	bool hasAlpha() const { return Image::hasAlpha(mFormat); }
	
	bool hasMipmaps() const { return (flags & HasMipmaps) == HasMipmaps; }
	
protected:
	
	Texture()
		: mFormat(Image::Format_Unknown)
		, flags(0)
		, size(Vec2i_ZERO)
		, storedSize(Vec2i_ZERO)
	{ }
	
	virtual bool Create() = 0;
	
	Image::Format mFormat;
	TextureFlags flags;
	
	Vec2i size;
	Vec2i storedSize;
	
};

DECLARE_FLAGS_OPERATORS(Texture::TextureFlags)

class Texture2D : public Texture {
	
public:
	
	virtual ~Texture2D() { }
	
	bool Init(const res::path & strFileName, TextureFlags flags);
	bool Init(const Image & image, TextureFlags flags = HasMipmaps);
	bool Init(unsigned int width, unsigned int height, Image::Format format);
	
	bool Restore();
	
	Image & GetImage() { return mImage; }
	const res::path & getFileName() const { return mFileName; }
	
	bool hasColorKey() { return (flags & ApplyColorKey) && hasAlpha() && !mFileName.empty(); }
	
protected:
	
	Texture2D() { }
	
	Image mImage;
	res::path mFileName;
	
};

#endif // ARX_GRAPHICS_TEXTURE_TEXTURE_H
