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

#ifndef ARX_GRAPHICS_TEXTURE_PACKEDTEXTURE_H
#define ARX_GRAPHICS_TEXTURE_PACKEDTEXTURE_H

#include <vector>
#include <stddef.h>

#include "graphics/image/Image.h"
#include "math/Rectangle.h"
#include "math/Vector.h"

class Texture;

class PackedTexture {
	
public:
	
	PackedTexture(size_t textureSize, Image::Format textureFormat);
	~PackedTexture();
	
	//! Reset the packed texture - remove all images
	void clear();
	
	//! Upload changed textures
	void upload();
	
	bool insertImage(const Image & image, size_t & textureIndex, Vec2i & offset);
	
	Texture & getTexture(size_t index);
	
	size_t getTextureSize() const { return m_textureSize; }
	size_t getTextureCount() const { return m_textures.size(); }
	
protected:
	
	class TextureTree {
		
	public:
		
		struct Node {
			
			Node();
			~Node();
			
			Node * insertImage(const Image & image);
			
			Node * children[2];
			Rect rect;
			bool used;
		};
		
		explicit TextureTree(size_t textureSize, Image::Format textureFormat);
		~TextureTree();
		
		Node * insertImage(const Image & image);
		
	private:
		
		Node root;
		
	public:
		
		Texture * texture;
		bool dirty;
	};
	
private:
	
	std::vector<TextureTree *> m_textures;
	
	const size_t m_textureSize;
	const Image::Format m_textureFormat;
	
};


#endif // ARX_GRAPHICS_TEXTURE_PACKEDTEXTURE_H
