/*
 * Copyright 2022 Arx Libertatis Team (see the AUTHORS file)
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

#include "util/Number.h"

#include "platform/PlatformConfig.h"

#if ARX_HAVE_CXX17_FROM_CHARS_INT || ARX_HAVE_CXX17_FROM_CHARS_FLOAT
#include <charconv>
#endif

#if !ARX_HAVE_CXX17_FROM_CHARS_INT
#include "boost/lexical_cast.hpp"
#endif

#if !ARX_HAVE_CXX17_FROM_CHARS_FLOAT
#include "fast_float/fast_float.h"
#endif

#include "util/String.h"


namespace util {

//! Convert the start of string to a float similar to \ref atoi
[[nodiscard]] std::optional<s32> toInt(std::string_view string, bool allowTrailingGarbage) noexcept {
	
	if(!string.empty() && string[0] == '+') {
		string.remove_prefix(1);
	}
	
	#if ARX_HAVE_CXX17_FROM_CHARS_INT
	s32 value = 0;
	auto result = std::from_chars(string.data(), string.data() + string.length(), value);
	if(result.ec == std::errc() && (allowTrailingGarbage || result.ptr == string.data() + string.length())) {
		return value;
	} else {
		return { };
	}
	#else
	if(allowTrailingGarbage) {
		size_t end = 0;
		if(end != string.length() && string[end] == '-') {
			end++;
		}
		if(end != string.length() && string[end] >= '0' && string[end] <= '9') {
			end++;
		}
		string = string.substr(0, end);
	}
	try {
		return boost::lexical_cast<s32>(string);
	} catch(...) {
		return { };
	}
	#endif
	
}

//! Convert the start of string to a float similar to \ref atof
[[nodiscard]] std::optional<float> toFloat(std::string_view string, bool allowTrailingGarbage) noexcept {
	
	if(!string.empty() && string[0] == '+') {
		string.remove_prefix(1);
	}
	
	float value = 0.f;
	
	#if ARX_HAVE_CXX17_FROM_CHARS_FLOAT
	auto result = std::from_chars(string.data(), string.data() + string.length(), value);
	#else
	auto result = fast_float::from_chars(string.data(), string.data() + string.length(), value);
	#endif
	
	if(result.ec == std::errc() && (allowTrailingGarbage || result.ptr == string.data() + string.length())) {
		return value;
	} else {
		return { };
	}
	
}

s32 parseInt(std::string_view string) noexcept {
	return toInt(trimLeft(string), true).value_or(0);
}

float parseFloat(std::string_view string) noexcept {
	return toFloat(trimLeft(string), true).value_or(0.f);
}

} // namespace util
