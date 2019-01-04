/*
 * Copyright 2011-2019 Arx Libertatis Team (see the AUTHORS file)
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

#include "graphics/image/Image.h"

#include <algorithm>
#include <sstream>
#include <cstring>
#include <limits>

#include "graphics/image/stb_image.h"

#include "graphics/Math.h"
#include "io/fs/FilePath.h"
#include "io/resource/PakReader.h"
#include "io/log/Logger.h"
#include "platform/CrashHandler.h"


Image::Image() : m_data(0) {
	reset();
}

Image::Image(const Image & other) : m_data(NULL) {
	*this = other;
}

Image::~Image() {
	delete[] m_data;
}

void Image::reset() {
	delete[] m_data, m_data = NULL;
	m_width = 0;
	m_height = 0;
	m_format = Format_Unknown;
}

Image & Image::operator=(const Image & other) {
	
	// Ignore self copy!
	if(&other == this) {
		return *this;
	}
	
	create(other.getWidth(), other.getHeight(), other.getFormat());
	
	memcpy(getData(), other.getData(), getSize());
	
	return *this;
}

size_t Image::getSize(Image::Format format, size_t width, size_t height) {
	return std::max(width, size_t(1)) * std::max(height, size_t(1)) * getNumChannels(format);
}

size_t Image::getNumChannels(Image::Format format) {
	
	switch(format) {
		case Format_L8:       return 1;
		case Format_A8:       return 1;
		case Format_L8A8:     return 2;
		case Format_R8G8B8:   return 3;
		case Format_B8G8R8:   return 3;
		case Format_R8G8B8A8: return 4;
		case Format_B8G8R8A8: return 4;
		case Format_Unknown:  return 0;
		case Format_Num: arx_unreachable();
	}
	
	return 0;
}

bool Image::load(const res::path & filename) {
	
	std::string buffer = g_resources->read(filename);
	if(buffer.empty()) {
		return false;
	}
	
	return load(buffer.data(), buffer.size(), filename.string().c_str());
}

bool Image::load(const char * data, size_t size, const char * file) {
	
	if(!data) {
		return false;
	}
	
	arx_assert(size <= size_t(std::numeric_limits<int>::max()));
	
	int width, height, bpp, fmt, req_bpp = 0;
	
	// 2bpp TGAs needs to be converted!
	const stbi::stbi_uc * raw = reinterpret_cast<const stbi::stbi_uc *>(data);
	int ret = stbi::stbi_info_from_memory(raw, int(size), &width, &height, &bpp, &fmt);
	if(ret && fmt == stbi::STBI_tga && bpp == 2)
		req_bpp = 3;
	
	unsigned char * pixels = stbi::stbi_load_from_memory(raw, int(size), &width, &height, &bpp, req_bpp);
	if(!pixels) {
		std::ostringstream message;
		message << "error loading image";
		if(file) {
			message << " \"" << file << '"';
		}
		LogError << message.str();
		return false;
	}
	
	if(req_bpp != 0) {
		bpp = req_bpp;
	}
	
	Format format = Format_Unknown;
	switch(bpp) {
		case stbi::STBI_grey:       format = Format_L8; break;
		case stbi::STBI_grey_alpha: format = Format_L8A8; break;
		case stbi::STBI_rgb:        format = Format_R8G8B8; break;
		case stbi::STBI_rgb_alpha:  format = Format_R8G8B8A8; break;
		default: arx_assert_msg(false, "Invalid bpp");
	}
	
	create(size_t(width), size_t(height), format);
	
	// Copy image data to our buffer
	memcpy(getData(), pixels, getSize());
	
	// Release resources
	stbi::stbi_image_free(pixels);
	
	return true;
}

void Image::create(size_t width, size_t height, Format format) {
	
	arx_assert(width > 0);
	arx_assert(height > 0);
	arx_assert_msg(format < Format_Unknown, "Unknown texture format!");
	
	size_t oldSize = getSize();
	size_t newSize = getSize(format, width, height);
	if(m_data && newSize != oldSize) {
		delete[] m_data, m_data = NULL;
	}
	if(!m_data) {
		m_data = new unsigned char[newSize];
	}
	
	m_width  = width;
	m_height = height;
	m_format = format;
	
}


bool Image::convertTo(Format format) {
	
	arx_assert_msg(getNumChannels() == getNumChannels(format),
	               "Conversion of images with different BPP not supported!");
	if(getNumChannels() != getNumChannels(format)) {
		return false;
	}
	
	if(getFormat() == format) {
		return true;
	}
	
	size_t numComponents = getNumChannels();
	size_t size = getWidth() * getHeight();
	unsigned char * data = getData();
	
	switch(format) {
		case Format_R8G8B8:
		case Format_B8G8R8:
		case Format_R8G8B8A8:
		case Format_B8G8R8A8:
			for(size_t i = 0; i < size; i++, data += numComponents) {
				std::swap(data[0], data[2]);
			}
			break;
		default:
			arx_assert_msg(false, "Unsupported conversion!");
			return false;
	};
	
	m_format = format;
	
	return true;
}

void Image::resizeFrom(const Image & source, size_t width, size_t height, bool flipY) {
	
	create(width, height, Format_R8G8B8);
	
	// span, size of one line in pixels (doesn't allow for byte padding)
	size_t src_span = source.getWidth();
	size_t dest_span = getWidth();
	
	// number of bytes per pixel
	// since we assume RGB format, this is 3 for both source and destination
	size_t src_pixel = 3;
	size_t dest_pixel = 3;
	
	// find fractional source y_delta
	float y_source = 0.0f;
	const float y_delta = float(source.getHeight()) / float(getHeight());
	
	for(size_t y = 0; y < getHeight(); y++) {
		
		// find pointer to the beginning of this destination line
		unsigned char * dest_p = getData() + (flipY ? getHeight() - 1 - y : y) * dest_span * dest_pixel;
		
		// truncate y_source coordinate and premultiply by line width / span
		size_t src_y = size_t(y_source) * src_span;
		
		// find fractional source x_delta
		float x_source = 0.0f;
		const float x_delta = float(source.getWidth()) / float(getWidth());
		
		for(size_t x = 0; x < getWidth(); x++) {
			
			// truncate x_source coordinate
			size_t src_x = size_t(x_source);
			
			// find offset in bytes for the current source coordinate
			size_t src_offset = (src_x + src_y) * src_pixel;
			
			// copy pixel from source to dest, assuming 24-bit format (RGB or BGR, etc)
			dest_p[0] = source.getData()[src_offset + 0];
			dest_p[1] = source.getData()[src_offset + 1];
			dest_p[2] = source.getData()[src_offset + 2];
			
			// move destination pointer ahead by one pixel
			dest_p += dest_pixel;
			
			// increment fractional source coordinate by one destination pixel, horizontal
			x_source += x_delta;
		}
		
		// increment fractional source coordinate by one destination pixel, vertical
		y_source += y_delta;
	}
	
}

void Image::clear() {
	memset(getData(), 0, getSize());
}

bool Image::copy(const Image & srcImage, size_t dstX, size_t dstY,
                 size_t srcX, size_t srcY, size_t width, size_t height) {
	
	size_t bpp = getNumChannels();
	
	// Format must match.
	if(srcImage.getFormat() != getFormat()) {
		return false;
	}
	
	// Must fit inside boundaries
	if(dstX + width > getWidth() || dstY + height > getHeight()) {
		return false;
	}
	
	// Must fit inside boundaries
	if(srcX + width > srcImage.getWidth() || srcY + height > srcImage.getHeight()) {
		return false;
	}
	
	unsigned char * dst = getData() + dstY * getWidth() * bpp + dstX * bpp;
	const unsigned char * src = srcImage.getData() + srcY * srcImage.getWidth() * bpp + srcX * bpp;
	
	// Copy in one step
	if(dstX == 0 && srcX == 0 && width == srcImage.getWidth() && width == getWidth()) {
		memcpy(dst, src, bpp * width * height);
		return true;
	}
	
	// Copy line by line
	for(size_t i = 0; i < height; i++, src += srcImage.getWidth() * bpp, dst += getWidth() * bpp) {
		memcpy(dst, src, bpp * width);
	}
	
	return true;
}

bool Image::copy(const Image & srcImage, size_t dstX, size_t dstY) {
	return copy(srcImage, dstX, dstY, 0, 0, srcImage.getWidth(), srcImage.getHeight());
}

void Image::applyGamma(float gamma) {
	
	// This function was taken from a couple engines that I saw,
	// which most likely originated from the Aftershock engine.
	// Kudos to them!  What it does is increase/decrease the intensity
	// of the lightmap so that it isn't so dark.  Quake uses hardware to
	// do this, but we will do it in code.
	
	// actually this is only adjusting the "value" of the image in RGB.
	//
	// each pixel is also normalized based upon it's peak component value,
	// rather than clipping. this will prevent modification of chroma.
	// pixels with any saturated component will not be modified.
	//
	// if the image has alpha == 1.0, those pixels will get no effect
	// using a gamma < 1.0 will have no effect
	
	size_t numComponents = getNumChannels();
	size_t size = getWidth() * getHeight();
	unsigned char * data = getData();
	
	const size_t MAX_COMPONENTS = 4;
	const float COMPONENT_RANGE = 255.0f;
	
	float components[MAX_COMPONENTS];
	
	// Nothing to do in this case!
	if(gamma == 1.0f) {
		return;
	}
	
	// Go through every pixel in the image
	for(size_t i = 0; i < size; i++, data += numComponents) {
		
		float max_component = 0.0f;
		
		for(size_t j = 0; j < numComponents; j++) {
			
			// scale the component's value
			components[j] = float(data[j]) * gamma;
			
			// find the max component
			max_component = std::max(max_component, components[j]);
			
		}
		
		if(max_component > COMPONENT_RANGE) {
			
			float reciprocal = COMPONENT_RANGE / max_component;
			
			for(size_t j = 0; j < numComponents; j++) {
				// normalize the components by max component value
				components[j] *= reciprocal;
				data[j] = static_cast<unsigned char>(components[j]);
			}
			
		} else {
			
			for(size_t j = 0; j < numComponents; j++) {
				data[j] = static_cast<unsigned char>(components[j]);
			}
			
		}
		
	}
	
}

void Image::applyThreshold(unsigned char threshold, int component_mask) {
	
	size_t numComponents = getNumChannels();
	size_t size = getWidth() * getHeight();
	unsigned char * data = getData();
	
	// Go through every pixel in the image
	for(size_t i = 0; i < size; i++, data += numComponents) {
		for(size_t j = 0; j < numComponents; j++) {
			if((component_mask >> j) & 1) {
				data[j] = (data[j] > threshold ? 255 : 0);
			}
		}
	}
	
}

template <size_t N>
static void extendImageRight(u8 * in, size_t win, size_t wout, size_t h) {
	u8 * out = in + N;
	for(size_t y = 0; y < h; y++, in += wout * N) {
		for(size_t x = win; x < wout; x++, out += N) {
			std::memcpy(out, in, N);
		}
		out += win * N;
	}
}

template <>
void extendImageRight<1>(u8 * in, size_t win, size_t wout, size_t h) {
	u8 * out = in + 1;
	for(size_t y = 0; y < h; y++, in += win, out += wout) {
		std::memset(out, *in, wout - win);
	}
}

template <size_t N>
static void extendImageBottomRight(const u8 * in, u8 * out, size_t win, size_t wout, size_t h) {
	for(size_t y = 0; y < h; y++) {
		for(size_t x = win; x < wout; x++, out += N) {
			std::memcpy(out, in, N);
		}
		out += win * N;
	}
}

template <>
void extendImageBottomRight<1>(const u8 * in, u8 * out, size_t win, size_t wout, size_t h) {
	for(size_t y = 0; y < h; y++, out += wout) {
		std::memset(out, *in, wout - win);
	}
}

void Image::extendClampToEdgeBorder(const Image & src) {
	
	arx_assert_msg(getFormat() == src.getFormat(), "extendClampToEdgeBorder Cannot change format!");
	arx_assert_msg(getWidth() >= src.getWidth() && getHeight() >= src.getHeight(), "extendClampToEdgeBorder Cannot decrease size!");
	
	copy(src, 0, 0);
	
	size_t pixsize = getNumChannels();
	size_t insize = pixsize * src.getWidth(), outsize = pixsize * getWidth();
	
	if(getWidth() > src.getWidth()) {
		u8 * in =  getData() + (src.getWidth() - 1) * pixsize;
		switch(pixsize) {
			case 1: extendImageRight<1>(in, src.getWidth(), getWidth(), src.getHeight()); break;
			case 2: extendImageRight<2>(in, src.getWidth(), getWidth(), src.getHeight()); break;
			case 3: extendImageRight<3>(in, src.getWidth(), getWidth(), src.getHeight()); break;
			case 4: extendImageRight<4>(in, src.getWidth(), getWidth(), src.getHeight()); break;
			default: arx_unreachable();
		}
	}
	
	if(getHeight() > src.getHeight()) {
		u8 * in = getData() + outsize * (src.getHeight() - 1);
		u8 * out = getData() + outsize * src.getHeight();
		for(size_t y = src.getHeight(); y < getHeight(); y++, out += outsize) {
			std::memcpy(out, in, insize);
		}
	}
	
	if(getWidth() > src.getWidth() && getHeight() > src.getHeight()) {
		u8 * in = getData() + outsize * (src.getHeight() - 1) + pixsize * (src.getWidth() - 1);
		u8 * out = getData() + outsize * src.getHeight() + insize;
		size_t h = getHeight() - src.getHeight();
		switch(pixsize) {
			case 1: extendImageBottomRight<1>(in, out, src.getWidth(), getWidth(), h); break;
			case 2: extendImageBottomRight<2>(in, out, src.getWidth(), getWidth(), h); break;
			case 3: extendImageBottomRight<3>(in, out, src.getWidth(), getWidth(), h); break;
			case 4: extendImageBottomRight<4>(in, out, src.getWidth(), getWidth(), h); break;
			default: arx_unreachable();
		}
	}
}

bool Image::toGrayscale(Format newFormat) {
	
	size_t srcNumChannels = getNumChannels();
	size_t dstNumChannels = getNumChannels(newFormat);
	
	if(srcNumChannels < 3) {
		return false;
	}
	
	size_t newSize = getSize(newFormat, getWidth(), getHeight());
	unsigned char * newData = new unsigned char[newSize];
	
	unsigned char * src = getData();
	unsigned char * dst = newData;
	
	for(size_t i = 0; i < newSize; i += dstNumChannels) {
		unsigned char grayVal = (77 * src[0] + 151 * src[1] + 28 * src[2] + 128) >> 8;
		for(size_t j = 0; j < dstNumChannels; j++) {
			dst[j] = grayVal;
		}
		src += srcNumChannels;
		dst += dstNumChannels;
	}
	
	delete[] m_data;
	m_data = newData;
	
	m_format = newFormat;
	
	return true;
}

void Image::blur(size_t radius) {
	
	// Create kernel and precompute multiplication table
	size_t kernelSize = 1 + radius * 2;
	size_t * kernel = new size_t[kernelSize];
	size_t * mult = new size_t[kernelSize << 8];
	
	memset(kernel, 0, kernelSize * sizeof(*kernel));
	memset(mult, 0, (kernelSize << 8) * sizeof(*mult));
	
	kernel[kernelSize - 1] = 0;
	for(size_t i = 1; i < radius; i++) {
		size_t szi = radius - i;
		kernel[radius + i] = kernel[szi] = szi * szi;
		for(size_t j = 0; j < 256; j++) {
			mult[((radius + i) << 8) + j] = mult[(szi << 8) + j] = kernel[szi] * j;
		}
	}
	
	kernel[radius] = radius * radius;
	for(size_t j = 0; j < 256; j++) {
		mult[(radius << 8) + j] = kernel[radius] * j;
	}
	
	// Split color channels into separated array to simplify handling of multiple image format...
	// Could easilly be refactored
	size_t numChannels = getNumChannels();
	unsigned char * channel[4] = {};
	unsigned char * blurredChannel[4] = {};
	for(size_t c = 0; c < numChannels; c++) {
		channel[c] = new unsigned char[getWidth() * getHeight()];
		blurredChannel[c] = new unsigned char[getWidth() * getHeight()];
		for(size_t i = 0; i < getWidth() * getHeight(); i++) {
			channel[c][i] = getData()[i * numChannels + c];
		}
	}
	
	// Blur horizontally using our separable kernel
	size_t yi = 0;
	for(size_t yl = 0; yl < getHeight(); yl++) {
		for(size_t xl = 0; xl < getWidth(); xl++) {
			size_t channelVals[4] = {0, 0, 0, 0};
			size_t sum = 0;
			ptrdiff_t ri = ptrdiff_t(xl) - ptrdiff_t(radius);
			for(size_t i = 0; i < kernelSize; i++) {
				ptrdiff_t read = ri + ptrdiff_t(i);
				if(read >= 0 && size_t(read) < getWidth()) {
					read += yi;
					for(size_t c = 0; c < numChannels; c++) {
						channelVals[c] += mult[(i << 8) + channel[c][read]];
					}
					sum += kernel[i];
				}
			}
			ri = ptrdiff_t(yi) + ptrdiff_t(xl);
			for(size_t c = 0; c < numChannels; c++) {
				blurredChannel[c][ri] = u8(channelVals[c] / sum);
			}
		}
		yi += getWidth();
	}
	
	// Blur vertically using our separable kernel
	yi = 0;
	for(size_t yl = 0; yl < getHeight(); yl++) {
		ptrdiff_t ym = ptrdiff_t(yl) - ptrdiff_t(radius);
		ptrdiff_t riw = ym * ptrdiff_t(getWidth());
		for(size_t xl = 0; xl < getWidth(); xl++) {
			size_t channelVals[4] = {0, 0, 0, 0};
			size_t sum = 0;
			ptrdiff_t ri = ym;
			ptrdiff_t read = ptrdiff_t(xl) + riw;
			for(size_t i = 0; i < kernelSize; i++) {
				if(ri >= 0 && size_t(ri) < getHeight()) {
					for(size_t c = 0; c < numChannels; c++) {
						channelVals[c] += mult[(i << 8) + blurredChannel[c][read]];
					}
					sum += kernel[i];
				}
				ri++;
				read += getWidth();
			}
			for(size_t c = 0; c < numChannels; c++) {
				getData()[(xl + yi) * numChannels + c] = u8(channelVals[c] / sum);
			}
		}
		yi += getWidth();
	}
	
	// Clean up mess
	for(size_t c = 0; c < numChannels; c++) {
		delete[] channel[c];
		delete[] blurredChannel[c];
	}
	delete[] kernel;
	delete[] mult;
	
}

void Image::flipY() {
	
	size_t imageSize = getSize();
	size_t lineSize = getSize(getFormat(), getWidth(), 1);
	
	unsigned char * swapTmp = new unsigned char[lineSize];
	arx_assert(swapTmp);
	
	unsigned char * top = getData();
	unsigned char * bottom = top + (imageSize - lineSize);
	
	for(size_t i = 0; i < getHeight() / 2; i++) {
		
		memcpy(swapTmp, bottom, lineSize);
		memcpy(bottom, top, lineSize);
		memcpy(top, swapTmp, lineSize);
		
		top += lineSize;
		bottom -= lineSize;
	}
	
	delete[] swapTmp;
	
}

std::ostream & operator<<(std::ostream & os, Image::Format format) {
	switch(format) {
		case Image::Format_L8: return os << "L";
		case Image::Format_A8: return os << "A";
		case Image::Format_L8A8: return os << "LA";
		case Image::Format_R8G8B8: return os << "RGB";
		case Image::Format_B8G8R8: return os << "BGR";
		case Image::Format_R8G8B8A8: return os << "RGBA";
		case Image::Format_B8G8R8A8: return os << "BGRA";
		case Image::Format_Unknown: return os << "(invalid)";
		case Image::Format_Num: arx_unreachable();
	}
	return os;
}
