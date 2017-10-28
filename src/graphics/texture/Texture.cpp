/*
 * Copyright 2011-2015 Arx Libertatis Team (see the AUTHORS file)
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

bool Texture::Init(const res::path & strFileName, TextureFlags newFlags) {
	
	m_filename = strFileName;
	flags = newFlags;
	
	return Restore();
}

bool Texture::Init(const Image & pImage, TextureFlags newFlags) {
	
	m_filename.clear();
	m_image = pImage;
	flags = newFlags;
	
	return Restore();
}

bool Texture::Init(unsigned int pWidth, unsigned int pHeight, Image::Format pFormat) {
	
	m_filename.clear();
	
	m_size = Vec2i(pWidth, pHeight);
	m_image.create(pWidth, pHeight, pFormat);
	m_format = pFormat;
	flags = 0;
	
	return Create();
}

bool Texture::Restore() {
	
	bool bRestored = false;
	
	if(!getFileName().empty()) {
		
		m_image.load(getFileName());
		
		if((flags & ApplyColorKey) && !m_image.hasAlpha()) {
			m_image.applyColorKeyToAlpha(Color::black, config.video.colorkeyAntialiasing);
		}
		
		if(isIntensity()) {
			m_image.toGrayscale();
		}
		
	}
	
	if(m_image.isValid()) {
		
		m_format = m_image.getFormat();
		m_size = Vec2i(m_image.getWidth(), m_image.getHeight());
		
		Destroy();
		bool bCreated = Create();
		if(!bCreated) {
			return false;
		}
		
		Upload();
		
		bRestored = true;
	}
	
	if(!getFileName().empty()) {
		m_image.reset();
	}
	
	return bRestored;
}
