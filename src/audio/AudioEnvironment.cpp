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
/* Based on:
===========================================================================
ARX FATALIS GPL Source Code
Copyright (C) 1999-2010 Arkane Studios SA, a ZeniMax Media company.

This file is part of the Arx Fatalis GPL Source Code ('Arx Fatalis Source Code'). 

Arx Fatalis Source Code is free software: you can redistribute it and/or modify it under the terms of the GNU General Public 
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Arx Fatalis Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Arx Fatalis Source Code.  If not, see 
<http://www.gnu.org/licenses/>.

In addition, the Arx Fatalis Source Code is also subject to certain additional terms. You should have received a copy of these 
additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Arx 
Fatalis Source Code. If not, please request a copy in writing from Arkane Studios at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing Arkane Studios, c/o 
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/

#include "audio/AudioEnvironment.h"

#include "audio/AudioResource.h"
#include "audio/AudioGlobal.h"
#include "io/resource/PakReader.h"

namespace audio {

Environment::Environment(const res::path & _name)
	: name(_name)
	, size(7.5f)
	, diffusion(1.f)       // High density echoes
	, absorption(0.05f)    // Air-like absorbtion
	, reflect_volume(0.8f)
	, reflect_delay(7.f)
	, reverb_volume(1.f)
	, reverb_delay(10.f)
	, reverb_decay(1500.f)
	, reverb_hf_decay(1200.f)
{ }

aalError Environment::load() {
	
	arx_assert(!environment_path.empty());
	
	PakFileHandle * file = g_resources->open(environment_path / name);
	
	if(!file) {
		return AAL_ERROR_FILEIO;
	}
	
	if(!file->read(&size, 4)
	   || !file->read(&diffusion, 4)
	   || !file->read(&absorption, 4)
	   || !file->read(&reflect_volume, 4)
	   || !file->read(&reflect_delay, 4)
	   || !file->read(&reverb_volume, 4)
	   || !file->read(&reverb_delay, 4)
	   || !file->read(&reverb_decay, 4)
	   || !file->read(&reverb_hf_decay, 4)) {
		delete file;
		return AAL_ERROR_FILEIO;
	}
	
	delete file;
	
	return AAL_OK;
}

} // namespace audio
