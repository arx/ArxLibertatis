/*
 * Copyright 2018 Arx Libertatis Team (see the AUTHORS file)
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
/* Copyright: stbiw-0.92 - public domain - http://nothings.org/stb/stb_image_write.h
   writes out PNG/BMP/TGA images to C stdio - Sean Barrett 2010
                            no warranty implied; use at your own risk
*/

#include "graphics/image/Image.h"

#include "io/fs/FilePath.h"
#include "io/fs/FileStream.h"
#include "io/log/Logger.h"

#include "platform/Platform.h"

namespace {

void writeU16LE(fs::ofstream & f, u16 x) {
	char buffer[2] = { char(u8(x)), char(u8(x >> 8)) };
	f.write(buffer, 2);
}

void writeU32LE(fs::ofstream & f, u32 x) {
	char buffer[4] = { char(u8(x)), char(u8(x >> 8)), char(u8(x >> 16)), char(u8(x >> 24)) };
	f.write(buffer, 4);
}

void writePadding(fs::ofstream & f, size_t size) {
	
	const char padding[] = { 0, 0, 0, 0 };
	
	while(size) {
		size_t n = std::min(sizeof(padding), size);
		f.write(padding, std::streamsize(n));
		size -= n;
	}
	
}

} // anonymous namespace

bool Image::save(const fs::path & filename) const {
	
	if(getFormat() >= Format_Unknown || getWidth() == 0 || getHeight() == 0) {
		return false;
	}
	
	if(filename.ext() != ".bmp") {
		LogWarning << "Unexpected file extension for BMP: " << filename.ext();
	}
	
	fs::ofstream f(filename, fs::fstream::out | fs::fstream::binary | fs::fstream::trunc);
	if(!f.is_open()) {
		return false;
	}
	
	const size_t width = getWidth();
	const size_t height = getHeight();
	const size_t inputChannels = getNumChannels();
	const size_t outputChannels = hasAlpha() ? 4 : 3;
	
	// BITMAPFILEHEADER
	const char magic[] = { 'B', 'M' };
	f.write(magic, sizeof(magic));
	const size_t fileHeaderSize = 14;
	const size_t bitmapinfoheaderSize = 40;
	const size_t colorspaceInfoSize = 48;
	const size_t bitmapv4headerSize = bitmapinfoheaderSize + 16 + 4 + colorspaceInfoSize;
	const size_t infoHeaderSize = hasAlpha() ? bitmapv4headerSize : bitmapinfoheaderSize;
	const size_t scanlinePadding = ((width * outputChannels) % 4 == 0) ? 0 : 4 - (width * outputChannels) % 4;
	const size_t dataOffset = fileHeaderSize + infoHeaderSize;
	const size_t fileSize = dataOffset + (width * outputChannels + scanlinePadding) * height;
	writeU32LE(f, u32(fileSize));
	writeU16LE(f, 0);
	writeU16LE(f, 0);
	writeU32LE(f, u32(dataOffset));
	
	arx_assert(!f.good() || f.tellp() == std::streamoff(fileHeaderSize));
	
	// BITMAPINFOHEADER for BGR or BITMAPV3INFOHEADER for BGRA
	const size_t planes = 1;
	const size_t bpp = outputChannels * 8;
	const size_t compression = hasAlpha() ? 3 /* BITFIELDS */ : 0 /* RGB */;
	const size_t imageSize = 0;
	const size_t resolutionX = 0;
	const size_t resolutionY = 0;
	const size_t usedColors = 0;
	const size_t importantColors = 0;
	writeU32LE(f, u32(infoHeaderSize));
	writeU32LE(f, u32(width));
	writeU32LE(f, u32(height));
	writeU16LE(f, u16(planes));
	writeU16LE(f, u16(bpp));
	writeU32LE(f, u32(compression));
	writeU32LE(f, u32(imageSize));
	writeU32LE(f, u32(resolutionX));
	writeU32LE(f, u32(resolutionY));
	writeU32LE(f, u32(usedColors));
	writeU32LE(f, u32(importantColors));
	if(hasAlpha()) {
		// Always store as BGRA
		const size_t redMask   = 0x00ff0000;
		const size_t greenMask = 0x0000ff00;
		const size_t blueMask  = 0x000000ff;
		const size_t alphaMask = 0xff000000;
		writeU32LE(f, u32(redMask));
		writeU32LE(f, u32(greenMask));
		writeU32LE(f, u32(blueMask));
		writeU32LE(f, u32(alphaMask));
		// Could use BITMAPV3INFOHEADER and skip the remaining fields
		// However this is an undocumented variant and not everything can read it
		const size_t colorspaceType = 0x73524742; // sRGB
		writeU32LE(f, u32(colorspaceType));
		writePadding(f, colorspaceInfoSize);
	}
	
	arx_assert(!f.good() || f.tellp() == std::streamoff(dataOffset));
	
	if(!f.good()) {
		return false;
	}
	
	for(size_t y = height; y != 0; y--) {
		
		for(size_t x = 0; x < width; x++) {
			const unsigned char * d = getData() + ((y - 1) * width + x) * inputChannels;
			
			char buffer[4];
			switch(getFormat()) {
				
				case Format_L8: {
					buffer[0] = buffer[1] = buffer[2] = char(d[0]);
					break;
				}
				
				case Format_A8: {
					buffer[0] = buffer[1] = buffer[2] = 0;
					buffer[3] = char(d[0]);
					break;
				}
				
				case Format_L8A8: {
					buffer[0] = buffer[1] = buffer[2] = char(d[0]);
					buffer[3] = char(d[1]);
					break;
				}
				
				case Format_R8G8B8: {
					buffer[0] = char(d[2]);
					buffer[1] = char(d[1]);
					buffer[2] = char(d[0]);
					break;
				}
				
				case Format_B8G8R8: {
					buffer[0] = char(d[0]);
					buffer[1] = char(d[1]);
					buffer[2] = char(d[2]);
					break;
				}
				
				case Format_R8G8B8A8: {
					buffer[0] = char(d[2]);
					buffer[1] = char(d[1]);
					buffer[2] = char(d[0]);
					buffer[3] = char(d[3]);
					break;
				}
				
				case Format_B8G8R8A8: {
					buffer[0] = char(d[0]);
					buffer[1] = char(d[1]);
					buffer[2] = char(d[2]);
					buffer[3] = char(d[3]);
					break;
				}
				
				case Format_Unknown:
				case Format_Num: {
					arx_unreachable();
				}
				
			}
			
			f.write(buffer, outputChannels);
			
		}
		
		writePadding(f, scanlinePadding);
		
		if(!f.good()) {
			return false;
		}
		
	}
	
	return true;
}
