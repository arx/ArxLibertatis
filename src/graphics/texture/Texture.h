/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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

#include <stddef.h>

#include "graphics/image/Image.h"
#include "io/resource/ResourcePath.h"
#include "math/Vector.h"
#include "util/Flags.h"

class Texture {
	
public:
	
	enum TextureFlag {
		HasMipmaps    = 1 << 0,
		ApplyColorKey = 1 << 1,
		Intensity     = 1 << 2,
	};
	DECLARE_FLAGS(TextureFlag, TextureFlags)
	
	virtual ~Texture() { }
	
	bool create(const res::path & strFileName, TextureFlags flags);
	bool create(const Image & image, TextureFlags flags = HasMipmaps);
	bool create(size_t width, size_t height, Image::Format format);
	
	virtual void upload() = 0;
	virtual void destroy() = 0;
	
	bool restore();
	
	const Vec2i & getSize() const { return m_size; }
	const Vec2i & getStoredSize() const { return m_storedSize; }
	
	Image::Format getFormat() const { return m_format; }
	bool hasAlpha() const { return Image::hasAlpha(getFormat()); }
	
	bool hasMipmaps() const { return (m_flags & HasMipmaps) != 0; }
	bool isIntensity() const { return (m_flags & Intensity) != 0; }
	
	Image & getImage() { return m_image; }
	const res::path & getFileName() const { return m_filename; }
	
	bool hasColorKey() { return (m_flags & ApplyColorKey) && hasAlpha() && !getFileName().empty(); }
	
protected:
	
	Texture()
		: m_format(Image::Format_Unknown)
		, m_flags(0)
		, m_size(0)
		, m_storedSize(0)
	{ }
	
	virtual bool create() = 0;
	
	Image::Format m_format;
	TextureFlags m_flags;
	
	Vec2i m_size;
	Vec2i m_storedSize;
	
	Image m_image;
	res::path m_filename;
	
};

DECLARE_FLAGS_OPERATORS(Texture::TextureFlags)

#endif // ARX_GRAPHICS_TEXTURE_TEXTURE_H
