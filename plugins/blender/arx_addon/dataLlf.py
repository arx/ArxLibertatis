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
    c_char,
    c_int32,
    c_float
)

from .dataCommon import (
    SavedVec3,
    SavedColor
)

class DANAE_LLF_HEADER(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("version",          c_float),
        ("ident",            c_char * 16),
        ("lastuser",         c_char * 256),
        ("time",             c_int32),
        ("nb_lights",        c_int32),
        ("nb_Shadow_Polys",  c_int32),
        ("nb_IGNORED_Polys", c_int32),
        ("nb_bkgpolys",      c_int32),
        ("pad",              c_int32 * 256),
        ("fpad",             c_float * 256),
        ("cpad",             c_char * 4096),
        ("bpad",             c_int32 * 256)
    ]

class DANAE_LS_LIGHT(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("pos",          SavedVec3),
        ("rgb",          SavedColor),
        ("fallstart",    c_float),
        ("fallend",      c_float),
        ("intensity",    c_float),
        ("i",            c_float),
        ("ex_flicker",   SavedColor),
        ("ex_radius",    c_float),
        ("ex_frequency", c_float),
        ("ex_size",      c_float),
        ("ex_speed",     c_float),
        ("ex_flaresize", c_float),
        ("fpadd",        c_float * 24),
        ("extras",       c_int32),
        ("lpadd",        c_int32 * 31)
    ]



import logging
from ctypes import sizeof

class LlfSerializer(object):
    def __init__(self, ioLib):
        self.log = logging.getLogger('LlfSerializer')
        self.ioLib = ioLib
    
    def read(self, fileName):
        f = open(fileName, "rb")
        compressedData = f.read()
        f.close()
        self.log.info("Loaded %i bytes from file %s" % (len(compressedData), fileName))
        
        data = self.ioLib.unpack(compressedData)
        
        result = {}
        pos = 0
        
        llfHeader = DANAE_LLF_HEADER.from_buffer_copy(data, pos)
        pos += sizeof(DANAE_LLF_HEADER)
        
        LightsList = DANAE_LS_LIGHT * llfHeader.nb_lights
        lights = LightsList.from_buffer_copy(data, pos)
        pos += sizeof(LightsList)
        result["lights"] = lights
        
        return result