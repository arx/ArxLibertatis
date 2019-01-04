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
	Image(const Image & other);
	virtual ~Image();
	
	Image & operator=(const Image & other);
	
	bool load(const res::path & filename);
	bool load(const char * data, size_t size, const char * file = NULL);
	
	void create(size_t width, size_t height, Format format);
	
	bool convertTo(Format format);
	
	//! Reset to fresh constructor state
	void reset();
	
	//! Zero image data
	void clear();
	
	size_t getWidth() const { return m_width; }
	size_t getHeight() const { return m_height; }
	Format getFormat() const { return m_format; }
	size_t getSize() const { return getSize(getFormat(), getWidth(), getHeight()); }
	size_t getNumChannels() const { return getNumChannels(getFormat()); }
	
	bool isValid() const { return m_data != NULL; }
	
	bool hasAlpha() const { return hasAlpha(getFormat()); }
	
	static bool hasAlpha(Format format) {
		return format == Format_A8
		    || format == Format_L8A8
		    || format == Format_R8G8B8A8
		    || format == Format_B8G8R8A8;
	}
	
	//! Access to internal data.
	const unsigned char * getData() const { return m_data; }
	unsigned char * getData() { return m_data; }
	
	// conversions
	
	void flipY();
	
	bool toGrayscale(Format newFormat = Format_L8);
	
	/*!
	 * Create an image of the desired size and rescale the source into it
	 *
	 * Performs only nearest-neighbour interpolation of the image.
	 * Supports only RGB format.
	 */
	void resizeFrom(const Image & source, size_t width, size_t height, bool flipY = false);
	
	//! Set the alpha of pixels matching the color key to 0. Will add an alpha channel if needed.
	void applyColorKeyToAlpha(Color colorKey = Color::black, bool antialias = false);
	
	/*!
	 * Extend the image and fill ne new space by sampling at the nearest border
	 * of the original image.
	 */
	void extendClampToEdgeBorder(const Image & srcImage);
	
	//! Copy an image into this image's buffer
	bool copy(const Image & srcImage, size_t dstX, size_t dstY);
	bool copy(const Image & srcImage, size_t dstX, size_t dstY,
	          size_t srcX, size_t srcY, size_t width, size_t height);
	
	bool save(const fs::path & filename) const;
	
	// processing functions
	// destructively adjust image content
	
	//! Blur using gaussian kernel
	void blur(size_t radius);
	
	//! Scale value and normalizes by max component value
	void applyGamma(float gamma);
	
	void applyThreshold(unsigned char threshold, int component_mask);
	
	// statics
	static size_t getSize(Format format, size_t width, size_t height);
	static size_t getNumChannels(Format format);
	
private:
	
	size_t m_width;
	size_t m_height;
	
	Format m_format; //!< Image format.
	
	unsigned char * m_data; //!< Pointer to image data buffer.
	
};

std::ostream & operator<<(std::ostream & os, Image::Format format);

#endif // ARX_GRAPHICS_IMAGE_IMAGE_H
