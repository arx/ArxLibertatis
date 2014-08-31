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

#ifndef ARX_UTIL_HANDLETYPE_H
#define ARX_UTIL_HANDLETYPE_H

#define ARX_HANDLE_TYPEDEF(T, D, INVALID_VALUE)                 \
struct D                                                        \
{                                                               \
    enum SpecialHandles {                                       \
        Invalid = INVALID_VALUE                                 \
    };                                                          \
    T t;                                                        \
    explicit D(const T t_) : t(t_) {}                           \
    D() : t(Invalid) {}                                         \
    D(const D & t_) : t(t_.t){}                                 \
    /* implicit */ D(const SpecialHandles & value) : t(value){} \
    D & operator=(const D & rhs) { t = rhs.t; return *this;}    \
    operator const T & () const {return t; }                    \
    operator T & () { return t; }                               \
    bool operator==(const D & rhs) const { return t == rhs.t; } \
    bool operator==(const SpecialHandles & rhs) const { return t == rhs; } \
};

#endif // ARX_UTIL_HANDLETYPE_H
