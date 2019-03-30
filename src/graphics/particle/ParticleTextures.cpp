/*
 * Copyright 2019 Arx Libertatis Team (see the AUTHORS file)
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

#include "graphics/particle/ParticleTextures.h"

#include <boost/format.hpp>

#include "graphics/data/TextureContainer.h"

ParticleTextures g_particleTextures;

void ParticleTextures::init() {
	
	smokeparticle = TextureContainer::Load("graph/particles/smoke");
	
	// TODO bloodsplat and water_splat cannot use mipmapping because they need a constant color border pixel
	// this may also apply to other textures
	
	TextureContainer::TCFlags flags = TextureContainer::NoMipmap;
	flags |= TextureContainer::NoColorKey | TextureContainer::Intensity;
	bloodsplat[0] = TextureContainer::Load("graph/particles/new_blood", flags);
	bloodsplat[1] = TextureContainer::Load("graph/particles/new_blood_splat1", flags);
	bloodsplat[2] = TextureContainer::Load("graph/particles/new_blood_splat2", flags);
	bloodsplat[3] = TextureContainer::Load("graph/particles/new_blood_splat3", flags);
	bloodsplat[4] = TextureContainer::Load("graph/particles/new_blood_splat4", flags);
	bloodsplat[5] = TextureContainer::Load("graph/particles/new_blood_splat5", flags);
	blood_splat = TextureContainer::Load("graph/particles/new_blood2", flags);
	
	water_splat[0] = TextureContainer::Load("graph/particles/[fx]_water01", TextureContainer::NoMipmap);
	water_splat[1] = TextureContainer::Load("graph/particles/[fx]_water02", TextureContainer::NoMipmap);
	water_splat[2] = TextureContainer::Load("graph/particles/[fx]_water03", TextureContainer::NoMipmap);
	
	water_drop[0] = TextureContainer::Load("graph/particles/[fx]_water_drop01");
	water_drop[1] = TextureContainer::Load("graph/particles/[fx]_water_drop02");
	water_drop[2] = TextureContainer::Load("graph/particles/[fx]_water_drop03");
	healing = TextureContainer::Load("graph/particles/heal_0005");
	tzupouf = TextureContainer::Load("graph/obj3d/textures/(fx)_tsu_greypouf");
	fire2 = TextureContainer::Load("graph/particles/fire2");
	fire_hit = TextureContainer::Load("graph/particles/fire_hit");
	boom = TextureContainer::Load("graph/particles/boom");
	
	for(unsigned int i = 0; i < MAX_EXPLO; i++) {
		std::string texturePath = boost::str(boost::format("graph/particles/fireb_%02u") % (i + 1));
		TextureContainer * texture = TextureContainer::LoadUI(texturePath);
		arx_assert(texture);
		explo[i] = texture;
	}
}
