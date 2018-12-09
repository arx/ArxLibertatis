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
	
	size_t w = getWidth();
	size_t h = getHeight();
	size_t c = getNumChannels();
	
	// File header
	const char magic[] = { 'B', 'M' };
	f.write(magic, sizeof(magic));
	size_t scanline_pad = size_t((-int(w) * 3) & 3);
	writeU32LE(f, u32(14 + 40 + (w * 3 + scanline_pad) * h));
	writeU16LE(f, 0);
	writeU16LE(f, 0);
	writeU32LE(f, 14 + 40);
	
	// Bitmap header
	writeU32LE(f, 40);
	writeU32LE(f, u32(w));
	writeU32LE(f, u32(h));
	writeU16LE(f, 1);
	writeU16LE(f, 24);
	writeU32LE(f, 0);
	writeU32LE(f, 0);
	writeU32LE(f, 0);
	writeU32LE(f, 0);
	writeU32LE(f, 0);
	writeU32LE(f, 0);
	if(!f.good()) {
		return false;
	}
	
	for(size_t y = h; y != 0; y--) {
		
		for(size_t x = 0; x < w; x++) {
			const unsigned char * d = getData() + ((y - 1) * w + x) * c;
			
			char buffer[3];
			switch(getFormat()) {
				
				case Format_L8:
				case Format_A8:
				case Format_L8A8: {
					buffer[0] = buffer[1] = buffer[2] = char(d[0]);
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
					// Composite against pink background
					const unsigned char bg[3] = { 255, 0, 255 };
					for(size_t k = 0; k < 3; ++k) {
						buffer[k] = char(u8(bg[k] + ((d[2 - k] - bg[k]) * d[3]) / 255));
					}
					break;
				}
				
				case Format_B8G8R8A8: {
					// Composite against pink background
					const unsigned char bg[3] = { 255, 0, 255 };
					for(size_t k = 0; k < 3; ++k) {
						buffer[k] = char(u8(bg[k] + ((d[k] - bg[k]) * d[3]) / 255));
					}
					break;
				}
				
				case Format_Unknown:
				case Format_Num: {
					arx_unreachable();
				}
				
			}
			
			if(!f.write(buffer, 3).good()) {
				return false;
			}
			
		}
		
		if(scanline_pad > 0) {
			const char padding[] = { 0, 0 };
			arx_assert(scanline_pad <= sizeof(padding));
			if(!f.write(padding, std::streamsize(scanline_pad)).good()) {
				return false;
			}
		}
		
	}
	
	return true;
}
