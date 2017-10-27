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

#include "graphics/image/Image.h"

#include <algorithm>
#include <sstream>
#include <cstring>
#include <limits>

#include "graphics/image/stb_image.h"
#include "graphics/image/stb_image_write.h"

#include "graphics/Math.h"
#include "io/fs/FilePath.h"
#include "io/resource/PakReader.h"
#include "io/log/Logger.h"
#include "platform/CrashHandler.h"


Image::Image() : mData(0) {
	Reset();
}

Image::Image(const Image & pOther) : mData(NULL) {
	*this = pOther;
}

Image::~Image() {
	delete[] mData;
}

void Image::Reset() {
	delete[] mData, mData = NULL;
	mWidth = 0;
	mHeight = 0;
	mFormat = Format_Unknown;
	mDataSize = 0;
}

const Image& Image::operator=(const Image & pOther) {
	
	// Ignore self copy!
	if(&pOther == this) {
		return *this;
	}
	
	delete[] mData, mData = NULL;
	
	mWidth      = pOther.mWidth;
	mHeight     = pOther.mHeight;
	mFormat     = pOther.mFormat;
	mDataSize   = pOther.mDataSize;
	mData       = new unsigned char[mDataSize];
	
	memcpy(mData, pOther.mData, mDataSize);
	
	return *this;
}

size_t Image::GetSize(Image::Format format, size_t width, size_t height) {
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
		case Format_Num: ARX_DEAD_CODE();
	}
	
	return 0;
}

bool Image::load(const res::path & filename) {
	
	size_t size = 0;
	void * data = g_resources->readAlloc(filename, size);
	
	if(!data) {
		return false;
	}
	
	bool ret = LoadFromMemory(data, size, filename.string().c_str());
	
	free(data);
	
	return ret;
}

bool Image::LoadFromMemory(void * pData, size_t size, const char * file) {
	
	if(!pData) {
		return false;
	}
	
	arx_assert(size <= std::numeric_limits<int>::max());
	
	int width, height, bpp, fmt, req_bpp = 0;
	
	// 2bpp TGAs needs to be converted!
	int ret = stbi::stbi_info_from_memory((const stbi::stbi_uc*)pData, int(size), &width, &height, &bpp, &fmt);
	if(ret && fmt == stbi::STBI_tga && bpp == 2)
		req_bpp = 3;
	
	unsigned char* data = stbi::stbi_load_from_memory((const stbi::stbi_uc*)pData, int(size), &width, &height, &bpp, req_bpp);
	if(!data) {
		std::ostringstream message;
		message << "error loading image";
		if(file) {
			message << " \"" << file << '"';
		}
		LogError << message.str();
		return false;
	}

	if(req_bpp != 0)
		bpp = req_bpp;

	mWidth  = width;
	mHeight = height;

	switch(bpp) {
		case stbi::STBI_grey:       mFormat = Format_L8; break;
		case stbi::STBI_grey_alpha: mFormat = Format_L8A8; break;
		case stbi::STBI_rgb:        mFormat = Format_R8G8B8; break;
		case stbi::STBI_rgb_alpha:  mFormat = Format_R8G8B8A8; break;
		default: arx_assert_msg(false, "Invalid bpp");
	}
	
	size_t dataSize = GetSize(mFormat, mWidth, mHeight);
	
	// Delete previous buffer if size don't match
	if(mData && mDataSize != dataSize) {
		delete[] mData, mData = NULL;
	}
	
	// Create a new buffer if needed
	if(!mData) {
		mData = new unsigned char[dataSize];
	}
	
	// Copy image data to our buffer
	if(mData) {
		mDataSize = dataSize;
		memcpy(mData, data, mDataSize);
	}
	
	// Release resources
	stbi::stbi_image_free(data);
	
	return (mData != NULL);
}

void Image::Create(size_t width, size_t height, Format format) {
	
	arx_assert_msg(width > 0, "[Image::Create] Width is 0!");
	arx_assert_msg(height > 0, "[Image::Create] Width is 0!");
	arx_assert_msg(format < Format_Unknown, "[Image::Create] Unknown texture format!");
	
	mWidth  = width;
	mHeight = height;
	mFormat = format;
	
	size_t dataSize = GetSize(mFormat, mWidth, mHeight);
	if(mData && dataSize != mDataSize) {
		delete[] mData, mData = NULL;
	}
	mDataSize = dataSize;
	if(!mData) {
		mData = new unsigned char[mDataSize];
	}
}


bool Image::ConvertTo(Format format) {
	
	arx_assert_msg(getNumChannels() == getNumChannels(format),
	               "[Image::ConvertTo] Conversion of images with different BPP not supported yet!");
	if(getNumChannels() != getNumChannels(format)) {
		return false;
	}
	
	if(mFormat == format)
		return true;
	
	size_t numComponents = getNumChannels();
	size_t size = mWidth * mHeight;
	unsigned char * data = mData;

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
		arx_assert_msg(false, "[Image::ConvertTo] Unsupported conversion!");
		return false;
	};

	mFormat = format;
	return true;
}

// creates an image of the desired size and rescales the source into it
// performs only nearest-neighbour interpolation of the image
// supports only RGB format
void Image::ResizeFrom(const Image & source, size_t desired_width, size_t desired_height, bool flip_vertical) {
	
	Create(desired_width, desired_height, Format_R8G8B8);
	
	// span, size of one line in pixels (doesn't allow for byte padding)
	size_t src_span = source.GetWidth();
	size_t dest_span = GetWidth();
	
	// number of bytes per pixel
	// since we assume RGB format, this is 3 for both source and destination
	size_t src_pixel = 3;
	size_t dest_pixel = 3;
	
	// find fractional source y_delta
	float y_source = 0.0f;
	const float y_delta = source.GetHeight() / (float)GetHeight();
	
	for(size_t y = 0; y < GetHeight(); y++) {
		
		// find pointer to the beginning of this destination line
		unsigned char * dest_p = GetData() + (flip_vertical ? GetHeight() - 1 - y : y) * dest_span * dest_pixel;
		
		// truncate y_source coordinate and premultiply by line width / span
		size_t src_y = size_t(y_source) * src_span;
		
		// find fractional source x_delta
		float x_source = 0.0f;
		const float x_delta = source.GetWidth() / (float)GetWidth();
		
		for(size_t x = 0; x < GetWidth(); x++) {
			
			// truncate x_source coordinate
			size_t src_x = size_t(x_source);
			
			// find offset in bytes for the current source coordinate
			size_t src_offset = (src_x + src_y) * src_pixel;
			
			// copy pixel from source to dest, assuming 24-bit format (RGB or BGR, etc)
			dest_p[0] = source.GetData()[src_offset + 0];
			dest_p[1] = source.GetData()[src_offset + 1];
			dest_p[2] = source.GetData()[src_offset + 2];
			
			// move destination pointer ahead by one pixel
			dest_p += dest_pixel;
			
			// increment fractional source coordinate by one destination pixel, horizontal
			x_source += x_delta;
		}
		
		// increment fractional source coordinate by one destination pixel, vertical
		y_source += y_delta;
	}
	
}

void Image::Clear() {
	memset(mData, 0, mDataSize);
}

bool Image::Copy(const Image & srcImage, size_t dstX, size_t dstY,
                 size_t srcX, size_t srcY, size_t width, size_t height) {
	
	size_t bpp = getNumChannels();
	
	// Format must match.
	if(srcImage.GetFormat() != mFormat) {
		return false;
	}
	
	// Must fit inside boundaries
	if(dstX + width > mWidth || dstY + height > mHeight) {
		return false;
	}
	
	// Must fit inside boundaries
	if(srcX + width > srcImage.GetWidth() || srcY + height > srcImage.GetHeight()) {
		return false;
	}
	
	unsigned char * dst = mData + dstY * mWidth * bpp + dstX * bpp;
	const unsigned char * src = srcImage.mData + srcY * srcImage.mWidth * bpp + srcX * bpp;
	
	// Copy in one step
	if(dstX == 0 && srcX == 0 && width == srcImage.mWidth && width == mWidth) {
		memcpy(dst, src, bpp * width * height);
		return true;
	}
	
	// Copy line by line
	for(size_t i = 0; i < height; i++, src += srcImage.mWidth * bpp, dst += mWidth * bpp) {
		memcpy(dst, src, bpp * width);
	}
	
	return true;
}

bool Image::Copy(const Image & srcImage, size_t destX, size_t destY) {
	return Copy(srcImage, destX, destY, 0, 0, srcImage.GetWidth(), srcImage.GetHeight());
}

void Image::QuakeGamma(float pGamma) {
	
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
	// using a pGamma < 1.0 will have no effect
	
	size_t numComponents = getNumChannels();
	size_t size = mWidth * mHeight;
	unsigned char * data = mData;
	
	const size_t MAX_COMPONENTS = 4;
	const float COMPONENT_RANGE = 255.0f;
	
	float components[MAX_COMPONENTS];
	
	// Nothing to do in this case!
	if(pGamma == 1.0f) {
		return;
	}
	
	// Go through every pixel in the image
	for(size_t i = 0; i < size; i++, data += numComponents) {
		
		float max_component = 0.0f;
		
		for(size_t j = 0; j < numComponents; j++) {
			
			// scale the component's value
			components[j] = float(data[j]) * pGamma;
			
			// find the max component
			max_component = std::max(max_component, components[j]);
			
		}
		
		if(max_component > COMPONENT_RANGE) {
			
			float reciprocal = COMPONENT_RANGE / max_component;
			
			for(size_t j = 0; j < numComponents; j++) {
				// normalize the components by max component value
				components[j] *= reciprocal;
				data[j] = (unsigned char)components[j];
			}
			
		} else {
			
			for(size_t j = 0; j < numComponents; j++) {
				data[j] = (unsigned char)components[j];
			}
			
		}
		
	}
	
}

void Image::ApplyThreshold(unsigned char threshold, int component_mask) {
	
	size_t numComponents = getNumChannels();
	size_t size = mWidth * mHeight;
	unsigned char * data = mData;
	
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
static void extendImageBottomRight(u8 * in, u8 * out, size_t win, size_t wout, size_t h) {
	for(size_t y = 0; y < h; y++) {
		for(size_t x = win; x < wout; x++, out += N) {
			std::memcpy(out, in, N);
		}
		out += win * N;
	}
}

template <>
void extendImageBottomRight<1>(u8 * in, u8 * out, size_t win, size_t wout, size_t h) {
	for(size_t y = 0; y < h; y++, out += wout) {
		std::memset(out, *in, wout - win);
	}
}

void Image::extendClampToEdgeBorder(const Image & src) {
	
	arx_assert_msg(mFormat == src.mFormat, "extendClampToEdgeBorder Cannot change format!");
	arx_assert_msg(mWidth >= src.mWidth && mHeight >= src.mHeight, "extendClampToEdgeBorder Cannot decrease size!");
	
	Copy(src, 0, 0);
	
	size_t pixsize = getNumChannels();
	size_t insize = pixsize * src.mWidth, outsize = pixsize * mWidth;
	
	if(mWidth > src.mWidth) {
		u8 * in =  mData + (src.mWidth - 1) * pixsize;
		switch(pixsize) {
			case 1: extendImageRight<1>(in, src.mWidth, mWidth, src.mHeight); break;
			case 2: extendImageRight<2>(in, src.mWidth, mWidth, src.mHeight); break;
			case 3: extendImageRight<3>(in, src.mWidth, mWidth, src.mHeight); break;
			case 4: extendImageRight<4>(in, src.mWidth, mWidth, src.mHeight); break;
			default: ARX_DEAD_CODE();
		}
	}
	
	if(mHeight > src.mHeight) {
		u8 * in = mData + outsize * (src.mHeight - 1);
		u8 * out = mData + outsize * src.mHeight;
		for(size_t y = src.mHeight; y < mHeight; y++, out += outsize) {
			std::memcpy(out, in, insize);
		}
	}
	
	if(mWidth > src.mWidth && mHeight > src.mHeight) {
		u8 * in = mData + outsize * (src.mHeight - 1) + pixsize * (src.mWidth - 1);
		u8 * out = mData + outsize * src.mHeight + insize;
		size_t h = mHeight - src.mHeight;
		switch(pixsize) {
			case 1: extendImageBottomRight<1>(in, out, src.mWidth, mWidth, h); break;
			case 2: extendImageBottomRight<2>(in, out, src.mWidth, mWidth, h); break;
			case 3: extendImageBottomRight<3>(in, out, src.mWidth, mWidth, h); break;
			case 4: extendImageBottomRight<4>(in, out, src.mWidth, mWidth, h); break;
			default: ARX_DEAD_CODE();
		}
	}
}

bool Image::ToGrayscale(Format newFormat) {
	
	size_t srcNumChannels = getNumChannels();
	size_t dstNumChannels = getNumChannels(newFormat);
	
	if(srcNumChannels < 3) {
		return false;
	}
		
	size_t newSize = GetSize(newFormat, mWidth, mHeight);
	unsigned char * newData = new unsigned char[newSize];
	
	unsigned char * src = mData;
	unsigned char * dst = newData;
	
	for(size_t i = 0; i < newSize; i += dstNumChannels) {
		unsigned char grayVal = (77 * src[0] + 151 * src[1] + 28 * src[2] + 128) >> 8;
		for(size_t i = 0; i < dstNumChannels; i++) {
			dst[i] = grayVal;
		}
		src += srcNumChannels;
		dst += dstNumChannels;
	}
	
	delete[] mData;
	mData = newData;
	mDataSize = newSize;
	
	mFormat = newFormat;
	
	return true;
}

void Image::Blur(size_t radius) {
	
	// Create kernel and precompute multiplication table
	size_t kernelSize = 1 + radius * 2;
	size_t * kernel = new size_t[kernelSize];
	size_t * mult = new size_t[kernelSize << 8];
	
	memset(kernel, 0, kernelSize*sizeof(*kernel));
	memset(mult, 0, (kernelSize << 8)*sizeof(*mult));
	
	kernel[kernelSize - 1] = 0;
	for(size_t i = 1; i< radius; i++) {
		size_t szi = radius - i;
		kernel[radius + i] = kernel[szi] = szi * szi;
		for(size_t j = 0; j < 256; j++) {
			mult[((radius + i) << 8) + j] = mult[(szi << 8) + j] = kernel[szi] * j;
		}
	}
	
	kernel[radius] = radius * radius;
	for(int j = 0; j < 256; j++) {
		mult[(radius << 8) + j] = kernel[radius] * j;
	}
	
	// Split color channels into separated array to simplify handling of multiple image format...
	// Could easilly be refactored
	size_t numChannels = getNumChannels();
	unsigned char * channel[4] = {};
	unsigned char * blurredChannel[4] = {};
	for(size_t c = 0; c < numChannels; c++) {
		channel[c] = new unsigned char[mWidth * mHeight];
		blurredChannel[c] = new unsigned char[mWidth * mHeight];
		for(size_t i = 0; i < mWidth * mHeight; i++) {
			channel[c][i] = mData[i * numChannels + c];
		}
	}
	
	// Blur horizontally using our separable kernel
	size_t yi = 0;
	for(size_t yl = 0; yl < mHeight; yl++) {
		for(size_t xl = 0; xl < mWidth; xl++) {
			size_t channelVals[4] = {0, 0, 0, 0};
			size_t sum = 0;
			ptrdiff_t ri = ptrdiff_t(xl) - ptrdiff_t(radius);
			for(size_t i = 0; i < kernelSize; i++) {
				ptrdiff_t read = ri + ptrdiff_t(i);
				if(read >= 0 && size_t(read) < mWidth) {
					read += yi;
					for(size_t c = 0; c < numChannels; c++) {
						channelVals[c] += mult[(i << 8) + channel[c][read]];
					}
					sum += kernel[i];
				}
			}
			ri = ptrdiff_t(yi) + ptrdiff_t(xl);
			for(size_t c = 0; c < numChannels; c++) {
				blurredChannel[c][ri] = channelVals[c] / sum;
			}
		}
		yi += mWidth;
	}
	
	// Blur vertically using our separable kernel
	yi = 0;
	for (size_t yl = 0; yl < mHeight; yl++) {
		ptrdiff_t ym = ptrdiff_t(yl) - ptrdiff_t(radius);
		ptrdiff_t riw = ym * ptrdiff_t(mWidth);
		for(size_t xl = 0; xl < mWidth; xl++) {
			size_t channelVals[4] = {0, 0, 0, 0};
			size_t sum = 0;
			ptrdiff_t ri = ym;
			ptrdiff_t read = ptrdiff_t(xl) + riw;
			for(size_t i = 0; i < kernelSize; i++) {
				if(ri >= 0 && size_t(ri) < mHeight) {
					for(size_t c = 0; c < numChannels; c++) {
						channelVals[c] += mult[(i << 8) + blurredChannel[c][read]];
					}
					sum += kernel[i];
				}
				ri++;
				read += mWidth;
			}
			for(size_t c = 0; c < numChannels; c++) {
				mData[(xl + yi) * numChannels + c] = channelVals[c] / sum;
			}
		}
		yi += mWidth;
	}
	
	// Clean up mess
	for(size_t c = 0; c < numChannels; c++) {
		delete[] channel[c];
		delete[] blurredChannel[c];
	}
	delete[] kernel;
	delete[] mult;
	
}

void Image::SetAlpha(const Image & img, bool bInvertAlpha) {
	
	arx_assert(mWidth == img.mWidth);
	arx_assert(mHeight == img.mHeight);
	arx_assert(img.HasAlpha());
	arx_assert(HasAlpha());
	
	size_t srcChannelCount = img.getNumChannels();
	size_t dstChannelCount = getNumChannels();
	
	unsigned char * src = img.mData;
	unsigned char * dst = mData;
	
	// Offset the data pointers before the start of the loop
	// All our current image formats have their alpha in the last channel
	src += srcChannelCount - 1;
	dst += dstChannelCount - 1;
	
	size_t pixelCount = mWidth * mHeight;
	
	if(!bInvertAlpha) {
		for(size_t i = 0; i < pixelCount; i++, src += srcChannelCount, dst += dstChannelCount) {
			*dst = *src; // Copy alpha
		}
	} else {
		for(size_t i = 0; i < pixelCount; i++, src += srcChannelCount, dst += dstChannelCount) {
			*dst = 255 - *src; // Copy inverted alpha
		}
	}
	
}

void Image::FlipY() {
	
	size_t imageSize = GetSize(mFormat, mWidth, mHeight);
	size_t lineSize = imageSize / mHeight;
	
	unsigned char * swapTmp = (unsigned char *)malloc(lineSize);
	arx_assert(swapTmp);
	
	unsigned char * top = mData;
	unsigned char * bottom = top + (imageSize - lineSize);
	
	for(size_t i = 0; i < mHeight / 2; i++) {
		
		memcpy(swapTmp, bottom, lineSize);
		memcpy(bottom, top, lineSize);
		memcpy(top, swapTmp, lineSize);
		
		top += lineSize;
		bottom -= lineSize;
	}
	
	free(swapTmp);
	
}

bool Image::save(const fs::path & filename) const {
	
	if(mFormat >= Format_Unknown) {
		return false;
	}
	
	int ret = 0;
	if(filename.ext() == ".bmp") {
		ret = stbi::stbi_write_bmp(filename.string().c_str(), int(mWidth), int(mHeight),
		                           int(getNumChannels()), mData);
	} else if(filename.ext() == ".tga") {
		ret = stbi::stbi_write_tga(filename.string().c_str(), int(mWidth), int(mHeight),
		                           int(getNumChannels()), mData);
	} else {
		LogError << "Unsupported file extension: " << filename.ext();
	}
	
	return ret != 0;
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
		default: return os << "(invalid)";
	}
}
