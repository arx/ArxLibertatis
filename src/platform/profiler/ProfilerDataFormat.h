/*
 * Copyright 2014 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_PLATFORM_PROFILER_PROFILERDATAFORMAT_H
#define ARX_PLATFORM_PROFILER_PROFILERDATAFORMAT_H

#include "platform/Platform.h"

#pragma pack(push,1)

struct SavedThreadInfoHeader {
	u64  size;
};

struct SavedThreadInfo {
	char threadName[32];
	u64  threadId;
	u64  startTime;
	u64  endTime;
};

struct SavedProfilePointHeader {
	u64  size;
};

struct SavedProfilePoint {
	char tag[32];
	u64  threadId;
	u64  startTime;
	u64  endTime;
};

#pragma pack(pop)

#endif // ARX_PLATFORM_PROFILER_PROFILERDATAFORMAT_H
