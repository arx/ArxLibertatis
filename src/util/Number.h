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

#ifndef ARX_UTIL_NUMBER_H
#define ARX_UTIL_NUMBER_H

#include <optional>
#include <string_view>

#include "platform/Platform.h"

namespace util {

//! Convert the start of string to a float similar to \ref atoi but don't allow leading whitespace
[[nodiscard]] std::optional<s32> toInt(std::string_view string, bool allowTrailingGarbage = false) noexcept;

//! Convert the start of string to a float similar to \ref atof but don't allow leading whitespace
[[nodiscard]] std::optional<float> toFloat(std::string_view string, bool allowTrailingGarbage = false) noexcept;

//! Convert the start of string to a float similar to \ref atoi
[[nodiscard]] s32 parseInt(std::string_view string) noexcept;

//! Convert the start of string to a float similar to \ref atof
[[nodiscard]] float parseFloat(std::string_view string) noexcept;

} // namespace util

#endif // ARX_UTIL_NUMBER_H
