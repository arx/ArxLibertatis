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
// Code: Cyril Meynier
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved

#ifndef ARX_CORE_GAMETIME_H
#define ARX_CORE_GAMETIME_H

#include "graphics/Math.h"

#include "platform/Time.h"

namespace arx
{
	class time
	{
	public:

		time();
		~time() {}

		void init();

		void pause();
		void resume();

		void force_time_restore(const float &time);

		// TODO probably the source of the need to clip frame_delay
		inline void force_frame_time_restore(const float &v) {
			frame_time_us = v * 1000;
			last_frame_time_us = v * 1000;
		}

		inline bool operator>(const float &v) const { 
			return delta_time_us > (v * 1000); 
		}

		inline operator float() const {
			return delta_time_us / 1000.0f;
		}

		inline operator long() const {
			//return static_cast<long>(delta_time);
			return checked_range_cast<long>(delta_time_us / 1000);
		}

		inline operator unsigned long() const {
			return checked_range_cast<unsigned long>(delta_time_us / 1000);
		}

		inline void setMs(const float &v) {
			delta_time_us = v * 1000;
		}

		inline void update(const bool & use_pause = true) {

			if (is_paused() && use_pause) {
				delta_time_us = platform::getElapsedUs(start_time, pause_time);
			} else {
				delta_time_us = platform::getElapsedUs(start_time);
			}
		}

		inline float get_updated(const bool & use_pause = true) {
			
			update(use_pause);
			return delta_time_us / 1000.0f;
		}

		inline unsigned long get_updated_ul(const bool & use_pause = true) {

			update(use_pause);
			return checked_range_cast<unsigned long>(delta_time_us / 1000);
		}

		inline bool is_paused() const { 
			return paused; 
		}

		// used only for "slow time" spell
		inline void increment_start_time(const u64 & inc) {
			start_time += inc;
		}

		inline float get_frame_time() const { 
			return frame_time_us / 1000.0f; 
		}

		inline float get_last_frame_time() const {
			return last_frame_time_us / 1000.0f;
		}

		inline float get_frame_delay() const {
			return frame_delay_ms;
		}

		inline void update_frame_time() {
			update();
			frame_time_us = delta_time_us;
			frame_delay_ms = (frame_time_us - last_frame_time_us) / 1000.0f;
		}

		inline void update_last_frame_time() {
			last_frame_time_us = frame_time_us;
		}

	private:
		bool paused;

		// these values are expected to wrap
		u64 pause_time;
		u64 start_time;

		// TODO this sometimes respects pause and sometimes not! [adejr]: is this TODO still valid?
		// TODO an absolute time value stored in a floating point number will lose precision 
		// when large numbers are stored.
		u64 delta_time_us;
		
		u64 last_frame_time_us;
		u64 frame_time_us;
		float frame_delay_ms;

		/* TODO RFC (adejr: safe to allow varied precision?)
		** since these values and their accessors are isolated in this class, 
		** last_frame_time and frame_time could be replaced with u64 values 
		** and stored as int before cast to float. 
		**
		** this would eliminate the need for casting to int from the float value 
		** in the operator "int" accessors and also increase precision.
		** assuming that would be safe while the float value is still used.
		** as frame_delay is a delta the issue of precision is less critical.
		*/
	};
};

extern arx::time arxtime;

#endif // ARX_CORE_GAMETIME_H
