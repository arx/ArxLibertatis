/*
 * Copyright 2018 Arx Libertatis Team (see the AUTHORS file)
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

#include "gui/debug/DebugHudAudio.h"

#include <boost/foreach.hpp>

#include "audio/Audio.h"
#include "gui/debug/DebugPanel.h"

void debugHud_Audio() {
	
	static std::vector<audio::SourceInfo> infos;
	infos.clear();
	audio::getSourceInfos(infos);
	
	
	DebugBox srcInfos = DebugBox(Vec2i(10, 10), "Audio Sources");
	srcInfos.add("src", "status", "smp", "sample path");
	
	BOOST_FOREACH(const audio::SourceInfo & si, infos) {
		srcInfos.add(si.source, si.status, si.sample, si.sampleName);
	}
	
	srcInfos.print();
}
