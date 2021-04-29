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

bool Texture2D::Init(const res::path & strFileName, TextureFlags newFlags) {
	
	mFileName = strFileName;
	flags = newFlags;
	return Restore();
}

bool Texture2D::Init(const Image & pImage, TextureFlags newFlags) {
	
	mFileName.clear();
	mImage = pImage;
	flags = newFlags;
	return Restore();
}

bool Texture2D::Init(unsigned int pWidth, unsigned int pHeight, Image::Format pFormat) {
	
	mFileName.clear();
	
	size = Vec2i(pWidth, pHeight);
	mImage.Create(pWidth, pHeight, pFormat);
	mFormat = pFormat;
	flags = 0;
	
	return Create();
}

bool Texture2D::Restore() {
	
	bool bRestored = false;

	if(!mFileName.empty()) {
		
		mImage.LoadFromFile(mFileName);
		
		if((flags & HasColorKey) && !mImage.HasAlpha()) {
			mImage.ApplyColorKeyToAlpha(Color::black, config.video.colorkeyAntialiasing);
			if(!mImage.HasAlpha()) {
				flags &= ~HasColorKey;
			}
		}
		
		if(flags & Intensity) {
			mImage.ToGrayscale();
		}
		
	}

	if(mImage.IsValid()) {
		mFormat = mImage.GetFormat();
		size = Vec2i(mImage.GetWidth(), mImage.GetHeight());

		Destroy();
		bool bCreated = Create();
		if(!bCreated) {
			return false;
		}

		Upload();

		bRestored = true;
	} 

	if(!mFileName.empty()) {
		mImage.Reset();
	}
	
	return bRestored;
}
