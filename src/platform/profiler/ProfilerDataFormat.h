/*
 * Copyright 2014-2017 Arx Libertatis Team (see the AUTHORS file)
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

#pragma pack(push, 1)

static const char * profilerMagic = "arxprof";
struct SavedProfilerHeader {
	char magic[8];
	u32  version;
	char padding[4];
};

ARX_STATIC_ASSERT(sizeof(SavedProfilerHeader) == 16, "Header size mismatch");

enum ArxProfilerChunkType {
	ArxProfilerChunkType_None = 0,
	ArxProfilerChunkType_Strings = 1,
	ArxProfilerChunkType_Threads = 2,
	ArxProfilerChunkType_Samples = 3
};

struct SavedProfilerChunkHeader {
	u32 type;
	u64 size;
	char padding[52];
};

ARX_STATIC_ASSERT(sizeof(SavedProfilerChunkHeader) == 64, "Header size mismatch");

struct SavedProfilerThread {
	u32 stringIndex;
	u64 threadId;
	s64 startTime;
	s64 endTime;
};

struct SavedProfilerSample {
	u32 stringIndex;
	u64 threadId;
	s64 startTime;
	s64 endTime;
};

#pragma pack(pop)

#endif // ARX_PLATFORM_PROFILER_PROFILERDATAFORMAT_H
