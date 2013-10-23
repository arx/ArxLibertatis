/*
 * Copyright 2011-2013 Arx Libertatis Team (see the AUTHORS file)
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

#include "graphics/texture/PackedTexture.h"

#include "graphics/Renderer.h"
#include "graphics/texture/Texture.h"
#include "io/log/Logger.h"

PackedTexture::PackedTexture(unsigned int pSize, Image::Format pFormat)
	: textureSize(pSize), textureFormat(pFormat) { }

PackedTexture::~PackedTexture() {
	clear();
}

void PackedTexture::clear() {
	textures.clear();
}

void PackedTexture::upload() {
	for(texture_iterator i = textures.begin(); i != textures.end(); ++i) {
		TextureTree * tree = *i;
		if(tree->dirty) {
			tree->texture->Upload();
			tree->dirty = false;
		}
	}
}

PackedTexture::TextureTree::TextureTree(unsigned int textureSize,
                                        Image::Format textureFormat) {
	
	root.rect = Rect(0, 0, textureSize - 1, textureSize - 1);
	
	texture = GRenderer->CreateTexture2D();
	if(!texture->Init(textureSize, textureSize, textureFormat)) {
		LogError << "Could not create texture for size " << textureSize
		         << " and format " << textureFormat;
		delete texture, texture = NULL;
		return;
	}
	texture->GetImage().Clear();
	dirty = true;
}

PackedTexture::TextureTree::~TextureTree() {
	delete texture;
}

PackedTexture::TextureTree::Node * PackedTexture::TextureTree::insertImage(const Image & img) {
	
	Node * node = root.insertImage(img);
	
	if(node != NULL) {
		texture->GetImage().Copy(img, node->rect.left, node->rect.top);
		dirty = true;
	}
	
	return node;
}

bool PackedTexture::insertImage(const Image & image, unsigned int & textureIndex,
                                Vec2i & offset) {
	
	// Validate image size
	if(image.GetWidth() > textureSize || image.GetHeight() > textureSize) {
		return false;
	}
	
	// Copy to one of the existing textures
	TextureTree::Node * node = NULL;
	unsigned int nodeTree = 0;
	
	for(size_t i = 0; i < textures.size(); i++) {
		node = textures[i]->insertImage(image);
		nodeTree = i;
	}
	
	// No space found, create a new texture
	if(!node) {
		TextureTree * newTree = new TextureTree(textureSize, textureFormat);
		if(!newTree->texture) {
			delete newTree;
			return false;
		}
		textures.push_back(newTree);
		node = textures[textures.size() - 1]->insertImage(image);
		nodeTree = textures.size() - 1;
	}
	
	// A node must have been found
	arx_assert(node);
	
	// Copy texture there
	if(node) {
		// Copy values back into info structure
		offset = node->rect.origin.toVec2();
		textureIndex = nodeTree;
	}
	
	return node != NULL;
}

Texture2D& PackedTexture::getTexture(unsigned int index) {
	arx_assert(index < textures.size());
	arx_assert(textures[index]->texture);
	return *textures[index]->texture;
}

PackedTexture::TextureTree::Node::Node() {
	children[0] = NULL;
	children[1] = NULL;
	used = 0;
}

PackedTexture::TextureTree::Node::~Node() {
	delete children[0];
	delete children[1];
}

PackedTexture::TextureTree::Node * PackedTexture::TextureTree::Node::insertImage(const Image & image) {
	
	Node * result = NULL;
	
	// We're in a full node/leaf, return immediately.
	if(used) {
		return NULL;
	}
	
	// If we're not a leaf, try inserting in childs
	if(children[0]) {
		
		result = children[0]->insertImage(image);
		
		if(!result) {
			result = children[1]->insertImage(image);
		}
		
		used = children[0]->used && children[1]->used;
		return result;
	}
	
	int diffW = (rect.width() + 1) - image.GetWidth();
	int diffH = (rect.height() + 1) - image.GetHeight();
	
	// If we're too small, return.
	if(diffW < 0 || diffH < 0) {
		return NULL;
	}
	
	// Perfect match !
	if(diffW == 0 && diffH == 0) {
		used = true;
		return this;
	}
	
	// Otherwise, gotta split this node and create some kids
	children[0] = new Node();
	children[1] = new Node();
	
	if(diffW > diffH) {
		children[0]->rect = Rect(rect.left, rect.top, rect.left + image.GetWidth() - 1, rect.bottom);
		children[1]->rect = Rect(rect.left + image.GetWidth(), rect.top, rect.right, rect.bottom);
	} else {
		children[0]->rect = Rect(rect.left, rect.top, rect.right, rect.top + image.GetHeight() - 1);
		children[1]->rect = Rect(rect.left, rect.top + image.GetHeight(), rect.right, rect.bottom);
	}
	
	// Insert into first child we created
	return children[0]->insertImage(image);
}
