/*
 * Copyright 2011-2017 Arx Libertatis Team (see the AUTHORS file)
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

#include "graphics/texture/Texture.h"

#include "core/Config.h"

bool Texture::create(const res::path & filename, TextureFlags flags) {
	
	m_filename = filename;
	m_flags = flags;
	
	return restore();
}

bool Texture::create(const Image & image, TextureFlags flags) {
	
	m_filename.clear();
	m_image = image;
	m_flags = flags;
	
	return restore();
}

bool Texture::create(size_t width, size_t height, Image::Format format) {
	
	m_filename.clear();
	m_image.create(width, height, format);
	m_flags = 0;
	
	m_size = Vec2i(s32(width), s32(height));
	m_format = format;
	
	return create();
}

bool Texture::restore() {
	
	bool restored = false;
	
	if(!getFileName().empty()) {
		
		m_image.load(getFileName());
		
		if((m_flags & ApplyColorKey) && !m_image.hasAlpha()) {
			m_image.applyColorKeyToAlpha(Color::black, config.video.colorkeyAntialiasing);
		}
		
		if(isIntensity()) {
			m_image.toGrayscale();
		}
		
	}
	
	if(m_image.isValid()) {
		
		m_format = m_image.getFormat();
		m_size = Vec2i(s32(m_image.getWidth()), s32(m_image.getHeight()));
		
		destroy();
		
		if(create()) {
			upload();
			restored = true;
		}
		
	}
	
	if(!getFileName().empty()) {
		m_image.reset();
	}
	
	return restored;
}
