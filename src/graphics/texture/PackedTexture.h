/*
 * Copyright 2011 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GRAPHICS_TEXTURE_PACKEDTEXTURE_H
#define ARX_GRAPHICS_TEXTURE_PACKEDTEXTURE_H

#include <vector>

#include "graphics/image/Image.h"
#include "math/Rectangle.h"

class Texture2D;

class PackedTexture {
	
public:
	
	PackedTexture(unsigned int pSize, Image::Format pFormat);
	~PackedTexture();
	
	void ClearAll();
	
	void BeginPacking();
	void EndPacking();
	
	bool InsertImage(const Image& pImg, int & pOffsetU, int & pOffsetV, unsigned int & pTextureIndex);
	
	Texture2D & GetTexture(unsigned int pTexture);
	
	unsigned int GetTextureSize() const;
	unsigned int GetTextureCount() const;
	
protected:
	
	class TextureTree {
		
	public:
		
		struct Node {
			
			Node();
			~Node();
			
			Node * InsertImage(const Image & pImg);
			
			Node * mChilds[2];
			Rect mRect;
			bool mInUse;
		};
		
		explicit TextureTree(unsigned int pSize);
		Node * InsertImage(const Image & pImg);
		
	private:
		
		Node mRoot;
		
	};
	
private:
	
	std::vector<Image *> mImages;
	std::vector<Texture2D *> mTextures;
	std::vector<TextureTree *> mTexTrees;
	
	unsigned int mTexSize;
	Image::Format mTexFormat;
	
};


#endif // ARX_GRAPHICS_TEXTURE_PACKEDTEXTURE_H
