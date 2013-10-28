/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
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
#include <stddef.h>

#include "graphics/image/Image.h"
#include "math/Rectangle.h"
#include "math/Vector.h"

class Texture2D;

class PackedTexture {
	
public:
	
	PackedTexture(unsigned int textureSize, Image::Format pFormat);
	~PackedTexture();
	
	//! Reset the packed texture - remove all images
	void clear();
	
	//! Upload changed textures
	void upload();
	
	bool insertImage(const Image & image, unsigned int & textureIndex, Vec2i & offset);
	
	Texture2D & getTexture(unsigned int index);
	
	unsigned int getTextureSize() const { return textureSize; }
	size_t getTextureCount() const { return textures.size(); }
	
protected:
	
	class TextureTree {
		
	public:
		
		struct Node {
			
			Node();
			~Node();
			
			Node * insertImage(const Image & pImg);
			
			Node * children[2];
			Rect rect;
			bool used;
		};
		
		explicit TextureTree(unsigned int textureSize, Image::Format textureFormat);
		~TextureTree();
		
		Node * insertImage(const Image & pImg);
		
	private:
		
		Node root;
		
	public:
		
		Texture2D * texture;
		bool dirty;
	};
	
private:
	
	std::vector<TextureTree *> textures;
	typedef std::vector<TextureTree *>::iterator texture_iterator;
	
	const unsigned int textureSize;
	Image::Format textureFormat;
	
};


#endif // ARX_GRAPHICS_TEXTURE_PACKEDTEXTURE_H
