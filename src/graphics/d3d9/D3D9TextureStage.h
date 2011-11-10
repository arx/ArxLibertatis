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

#ifndef ARX_GRAPHICS_D3D9_D3D9TEXTURESTAGE_H
#define ARX_GRAPHICS_D3D9_D3D9TEXTURESTAGE_H

#include <d3d9.h>

#include "graphics/texture/TextureStage.h"

class D3D9TextureStage : public TextureStage {
	
public:
	
	explicit D3D9TextureStage(unsigned int textureStage);
	
	void SetTexture(Texture * pTexture);
	void ResetTexture();
	
	void SetColorOp(TextureOp textureOp, TextureArg texArg1, TextureArg texArg2);
	void SetColorOp(TextureOp textureOp);
	void SetAlphaOp(TextureOp textureOp, TextureArg texArg1, TextureArg texArg2);
	void SetAlphaOp(TextureOp textureOp);
	
	void SetWrapMode(WrapMode wrapMode);
	
	void SetMinFilter(FilterMode filterMode);
	void SetMagFilter(FilterMode filterMode);
	void SetMipFilter(FilterMode filterMode);
	
	void SetMipMapLODBias(float bias);
};

extern D3DTEXTUREFILTERTYPE ARXToDX9MagFilter[];
extern D3DTEXTUREFILTERTYPE ARXToDX9MinFilter[];
extern D3DTEXTUREFILTERTYPE ARXToDX9MipFilter[];

#endif // ARX_GRAPHICS_D3D9_D3D9TEXTURESTAGE_H
