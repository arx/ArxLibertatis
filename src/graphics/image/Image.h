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

#ifndef ARX_GRAPHICS_IMAGE_IMAGE_H
#define ARX_GRAPHICS_IMAGE_IMAGE_H

#include <stddef.h>
#include <ostream>

#include "graphics/Color.h"

namespace fs { class path; }
namespace res { class path; }

class Image {
	
public:
	
	enum Format {
		Format_L8,
		Format_A8,
		Format_L8A8,
		Format_R8G8B8,
		Format_B8G8R8,
		Format_R8G8B8A8,
		Format_B8G8R8A8,
		Format_Unknown,
		Format_Num,
	};
	
	Image();
	Image(const Image & pOther);
	virtual ~Image();
	
	const Image& operator=(const Image & pOther);
	
	bool LoadFromFile(const res::path & filename);
	bool LoadFromMemory(void * pData, size_t size, const char * file = NULL);
	
	void Create(size_t width, size_t height, Format format);
	
	bool ConvertTo(Format format);
	
	// reset to fresh constructor state
	void Reset();
	
	// zero image data with memset
	void Clear();
	
	// info accessors
	size_t GetWidth() const { return mWidth; }
	size_t GetHeight() const { return mHeight; }
	Format GetFormat() const { return mFormat; }
	size_t GetDataSize() const { return mDataSize;   }
	size_t GetNumChannels() const { return GetNumChannels(mFormat); }
	
	// bool accessors
	bool IsValid() const { return mData != NULL; }
	bool HasAlpha() const { return hasAlpha(mFormat); }
	static bool hasAlpha(Format format) {
		return format == Format_A8
		    || format == Format_L8A8
		    || format == Format_R8G8B8A8
		    || format == Format_B8G8R8A8;
	}
	
	//! Access to internal data.
	const unsigned char * GetData() const { return mData; }
	unsigned char * GetData() { return mData; }
	
	// conversions
	
	void FlipY();
	bool ToGrayscale(Format newFormat = Format_L8);
	
	void ResizeFrom(const Image & source, size_t width, size_t height, bool flip_vertical = false);
	
	//! Set the alpha of pixels matching the color key to 0. Will add an alpha channel if needed.
	void ApplyColorKeyToAlpha(Color colorKey = Color::black, bool antialias = false);
	
	/*!
	 * Extend the image and fill ne new space by sampling at the nearest border
	 * of the original image.
	 */
	void extendClampToEdgeBorder(const Image & srcImage);
	
	/*!
	 * \brief Copy an image into this image's buffer
	 *
	 * Works only with uncompressed formats
	 */
	bool Copy(const Image & srcImage, size_t dstX, size_t dstY);
	bool Copy(const Image & srcImage, size_t dstX, size_t dstY,
	          size_t srcX, size_t srcY, size_t width, size_t height);
	
	bool save(const fs::path & filename) const;
	
	// processing functions
	// destructively adjust image content
	
	//! blur using gaussian kernel
	void Blur(size_t radius);
	
	//! scales value and normalizes by max component value
	void QuakeGamma(float pGamma);
	//! Copy the alpha of img to this image.
	void SetAlpha(const Image& img, bool bInvertAlpha);
	
	void ApplyThreshold(unsigned char threshold, int component_mask);
	
	// statics
	static size_t GetSize(Format format, size_t width, size_t height);
	static size_t GetNumChannels(Format pFormat);
	
private:
	
	size_t mWidth;
	size_t mHeight;
	
	Format mFormat; //!< Image format.
	
	unsigned char * mData; //!< Pointer to image data buffer.
	size_t mDataSize; //!< Size of image buffer.
	
};

std::ostream & operator<<(std::ostream & os, Image::Format format);

#endif // ARX_GRAPHICS_IMAGE_IMAGE_H
