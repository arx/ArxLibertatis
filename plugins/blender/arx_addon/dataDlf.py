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
    c_char,
    c_uint32,
    c_int16,
    c_int32,
    c_float
)

from .dataCommon import (
    SavedVec3,
    SavedAnglef,
    PolyTypeFlag,
    SavedColor
)

class DANAE_LS_HEADER(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("version",         c_float),
        ("ident",           c_char * 16),
        ("lastuser",        c_char * 256),
        ("time",            c_int32),
        ("pos_edit",        SavedVec3),
        ("angle_edit",      SavedAnglef),
        ("nb_scn",          c_int32),
        ("nb_inter",        c_int32),
        ("nb_nodes",        c_int32),
        ("nb_nodeslinks",   c_int32),
        ("nb_zones",        c_int32),
        ("lighting",        c_int32),
        ("Bpad",            c_int32 * 256),
        ("nb_lights",       c_int32),
        ("nb_fogs",         c_int32),
        ("nb_bkgpolys",     c_int32),
        ("nb_ignoredpolys", c_int32),
        ("nb_childpolys",   c_int32),
        ("nb_paths",        c_int32),
        ("pad",             c_int32 * 250),
        ("offset",          SavedVec3),
        ("fpad",            c_float * 253),
        ("cpad",            c_char * 4096),
        ("bpad",            c_int32 * 256),
    ]

class DANAE_LS_SCENE(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("name", c_char * 512),
        ("pad",  c_int32 * 16),
        ("fpad", c_float * 16)
    ]

class DANAE_LS_INTER(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("name",     c_char * 512),
        ("pos",      SavedVec3),
        ("angle",    SavedAnglef),
        ("ident",    c_int32),
        ("flags",    c_int32),
        ("pad",      c_int32 * 14),
        ("fpad",     c_float * 16),
    ]

class DANAE_LS_LIGHTINGHEADER(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("nb_values", c_int32),
        ("ViewMode",  c_int32), # unused
        ("ModeLight", c_int32), # unused
        ("pad",       c_int32), # unused
    ]

# version 1.003f
# TODO probably unused
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
from ctypes import sizeof, create_string_buffer

class DlfSerializer(object):
    def __init__(self, ioLib):
        self.log = logging.getLogger('DlfSerializer')
        self.ioLib = ioLib
    
    def read(self, data, lsHeader):
        pos = 0
        result = {}
        
        scnHeader = DANAE_LS_SCENE.from_buffer_copy(data, pos)
        pos += sizeof(DANAE_LS_SCENE)
        self.log.info("DANAE_LS_SCENE name: %s" % scnHeader.name.decode('iso-8859-1'))
        
        EntitiesType = DANAE_LS_INTER * lsHeader.nb_inter
        entities = EntitiesType.from_buffer_copy(data, pos)
        pos += sizeof(EntitiesType)
        result["entities"] = entities
        
        if lsHeader.lighting != 0:
            lightingHeader = DANAE_LS_LIGHTINGHEADER.from_buffer_copy(data, pos)
            pos += sizeof(DANAE_LS_LIGHTINGHEADER)
            
            pos += lightingHeader.nb_values * 4 # Skip the data
        
        # Skip lights
        pos += sizeof(DANAE_LS_LIGHT) * lsHeader.nb_lights
        
        return result
        
    
    def readContainer(self, fileName):
        f = open(fileName, "rb")
        data = f.read()
        f.close()

        self.log.info("Loaded %i bytes from file %s" % (len(data), fileName))
        
        pos = 0
        lsHeader = DANAE_LS_HEADER.from_buffer_copy(data, pos)
        pos += sizeof(DANAE_LS_HEADER)
        self.log.info("DANAE_LS_HEADER version: %f" % lsHeader.version)
        
        if lsHeader.ident.decode('iso-8859-1') != "DANAE_FILE":
            raise Exception('Invalid ident')
        
        uncompressed = self.ioLib.unpack(data[pos:])
        
        return self.read(uncompressed, lsHeader)

