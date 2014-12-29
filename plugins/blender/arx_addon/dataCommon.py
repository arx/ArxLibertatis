# Copyright 2014 Arx Libertatis Team (see the AUTHORS file)
#
# This file is part of Arx Libertatis.
#
# Arx Libertatis is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Arx Libertatis is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Arx Libertatis. If not, see <http://www.gnu.org/licenses/>.

from ctypes import (
    LittleEndianStructure,
    Union,
    c_uint8,
    c_uint32,
    c_float
)


class SavedVec3(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("x", c_float),
        ("y", c_float),
        ("z", c_float)
    ]

class SavedAnglef(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("a", c_float),
        ("b", c_float),
        ("g", c_float)
    ]

class ArxQuat(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("w", c_float),
        ("x", c_float),
        ("y", c_float),
        ("z", c_float)
    ]

class SavedColor(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("r", c_float),
        ("g", c_float),
        ("b", c_float)
    ]

class PolyTypeFlag_bits(LittleEndianStructure):
     _fields_ = [
        ("POLY_NO_SHADOW",    c_uint8, 1),
        ("POLY_DOUBLESIDED",  c_uint8, 1),
        ("POLY_TRANS",        c_uint8, 1),
        ("POLY_WATER",        c_uint8, 1),
        ("POLY_GLOW",         c_uint8, 1),
        ("POLY_IGNORE",       c_uint8, 1),
        ("POLY_QUAD",         c_uint8, 1),
        ("POLY_TILED",        c_uint8, 1), # Unused
        ("POLY_METAL",        c_uint8, 1),
        ("POLY_HIDE",         c_uint8, 1),
        ("POLY_STONE",        c_uint8, 1),
        ("POLY_WOOD",         c_uint8, 1),
        ("POLY_GRAVEL",       c_uint8, 1),
        ("POLY_EARTH",        c_uint8, 1),
        ("POLY_NOCOL",        c_uint8, 1),
        ("POLY_LAVA",         c_uint8, 1),
        ("POLY_CLIMB",        c_uint8, 1),
        ("POLY_FALL",         c_uint8, 1),
        ("POLY_NOPATH",       c_uint8, 1),
        ("POLY_NODRAW",       c_uint8, 1),
        ("POLY_PRECISE_PATH", c_uint8, 1),
        ("POLY_NO_CLIMB",     c_uint8, 1), # Unused
        ("POLY_ANGULAR",      c_uint8, 1), # Unused
        ("POLY_ANGULAR_IDX0", c_uint8, 1), # Unused
        ("POLY_ANGULAR_IDX1", c_uint8, 1), # Unused
        ("POLY_ANGULAR_IDX2", c_uint8, 1), # Unused
        ("POLY_ANGULAR_IDX3", c_uint8, 1), # Unused
        ("POLY_LATE_MIP",     c_uint8, 1),
    ]

class PolyTypeFlag(Union):
    _fields_ = [
        ("b",      PolyTypeFlag_bits),
        ("asUInt", c_uint32),
    ]
    _anonymous_ = ("b")
