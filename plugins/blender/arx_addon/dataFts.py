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

from .dataCommon import SavedVec3, PolyTypeFlag

class UNIQUE_HEADER(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("path",             c_char * 256),
        ("count",            c_int32),
        ("version",          c_float),
        ("uncompressedsize", c_int32),
        ("pad",              c_int32 * 3)
    ]

class UNIQUE_HEADER3(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("path",  c_char * 256), # In the c code this is currently in a separate struct
        ("check", c_char * 512)
    ]

class FAST_SCENE_HEADER(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("version",     c_float),
        ("sizex",       c_int32),
        ("sizez",       c_int32),
        ("nb_textures", c_int32),
        ("nb_polys",    c_int32),
        ("nb_anchors",  c_int32),
        ("playerpos",   SavedVec3),
        ("Mscenepos",   SavedVec3),
        ("nb_portals",  c_int32),
        ("nb_rooms",    c_int32)
    ]

class FAST_TEXTURE_CONTAINER(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("tc",   c_int32),
        ("temp", c_int32),
        ("fic",  c_char * 256)
    ]

class FAST_SCENE_INFO(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("nbpoly",     c_int32),
        ("nbianchors", c_int32),
    ]

class FAST_VERTEX(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("sy",  c_float),
        ("ssx", c_float),
        ("ssz", c_float),
        ("stu", c_float),
        ("stv", c_float)
    ]

class FAST_EERIEPOLY(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("v",        FAST_VERTEX * 4),
        ("tex",      c_int32),
        ("norm",     SavedVec3),
        ("norm2",    SavedVec3),
        ("nrml",     SavedVec3 * 4),
        ("transval", c_float),
        ("area",     c_float),
        ("type",     PolyTypeFlag),
        ("room",     c_int16),
        ("paddy",    c_int16)
    ]

class FAST_ANCHOR_DATA(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("pos",       SavedVec3),
        ("radius",    c_float),
        ("height",    c_float),
        ("nb_linked", c_int16),
        ("flags",     c_int16)
    ]

class SavedTextureVertex(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("pos",      SavedVec3),
        ("rhw",      c_float),
        ("color",    c_uint32),
        ("specular", c_uint32),
        ("tu",       c_float),
        ("tv",       c_float)
    ]

class SAVE_EERIEPOLY(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("type",     c_int32),
        ("min",      SavedVec3),
        ("max",      SavedVec3),
        ("norm",     SavedVec3),
        ("norm2",    SavedVec3),
        ("v",        SavedTextureVertex * 4),
        ("tv",       SavedTextureVertex * 4),
        ("nrml",     SavedVec3 * 4),
        ("tex",      c_int32),
        ("center",   SavedVec3),
        ("transval", c_float),
        ("area",     c_float),
        ("room",     c_int16),
        ("misc",     c_int16)
    ]

class EERIE_SAVE_PORTALS(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("poly",      SAVE_EERIEPOLY),
        ("room_1",    c_int32),
        ("room_2",    c_int32),
        ("useportal", c_int16),
        ("paddy",     c_int16)
    ]
    
class EERIE_SAVE_ROOM_DATA(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("nb_portals", c_int32),
        ("nb_polys",   c_int32),
        ("padd",       c_int32 * 6)
    ]

class FAST_EP_DATA(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("px",   c_int16),
        ("py",   c_int16),
        ("idx",  c_int16),
        ("padd", c_int16)
    ]
    
class ROOM_DIST_DATA_SAVE(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("distance", c_float),
        ("startpos", SavedVec3),
        ("endpos",   SavedVec3),
    ]


import logging

from ctypes import sizeof
from .dataFts import *
from .lib import ArxIO

class FtsSerializer(object):
    def __init__(self, ioLib):
        self.log = logging.getLogger('FtsSerializer')
        self.ioLib = ioLib

    def read_fts(self, data):
        result = {}
        
        pos = 0
        ftsHeader = FAST_SCENE_HEADER.from_buffer_copy(data, pos)
        pos += sizeof(FAST_SCENE_HEADER)
        self.log.info("Fts Header version: %f" % ftsHeader.version)
        self.log.info("Fts Header size x,z: %i,%i" % (ftsHeader.sizex, ftsHeader.sizez))
        self.log.info("Fts Header playerpos: %f,%f,%f" % (ftsHeader.playerpos.x, ftsHeader.playerpos.y, ftsHeader.playerpos.z))
        self.log.info("Fts Header Mscenepos: %f,%f,%f" % (ftsHeader.Mscenepos.x, ftsHeader.Mscenepos.y, ftsHeader.Mscenepos.z))
        result["sceneOffset"] = (ftsHeader.Mscenepos.x, ftsHeader.Mscenepos.y, ftsHeader.Mscenepos.z)
        
        texturesType = FAST_TEXTURE_CONTAINER * ftsHeader.nb_textures
        textures = texturesType.from_buffer_copy(data, pos)
        pos += sizeof(texturesType)
        result["textures"] = textures
        self.log.info("Loaded %i textures" % len(textures))

        #for i in textures:
        #    log.info(i.fic.decode('iso-8859-1'))
        
        cells = [[None for x in range(ftsHeader.sizex)] for x in range(ftsHeader.sizez)]
        for z in range(ftsHeader.sizez):
            for x in range(ftsHeader.sizex):
                cellHeader = FAST_SCENE_INFO.from_buffer_copy(data, pos)
                pos += sizeof(FAST_SCENE_INFO)

                try:
                    if cellHeader.nbpoly <= 0:
                        cells[x][z] = None
                    else:
                        polysType = FAST_EERIEPOLY * cellHeader.nbpoly
                        polys = polysType.from_buffer_copy(data, pos)
                        pos += sizeof(polysType)

                        cells[x][z] = polys
                except ValueError as e:
                    print("Failed reading cell data, x:%i z:%i polys:%i" % (x, z, cellHeader.nbpoly))
                    raise e

                
                if cellHeader.nbianchors > 0:
                    AnchorsArrayType = c_int32 * cellHeader.nbianchors
                    anchors = AnchorsArrayType.from_buffer_copy(data, pos)
                    pos += sizeof(AnchorsArrayType)
                    
        result["cells"] = cells

        for i in range(ftsHeader.nb_anchors):
            anchor = FAST_ANCHOR_DATA.from_buffer_copy(data, pos)
            pos += sizeof(FAST_ANCHOR_DATA)
            
            if anchor.nb_linked > 0:
                LinkedAnchorsArrayType = c_int32 * anchor.nb_linked
                linked = LinkedAnchorsArrayType.from_buffer_copy(data, pos)
                pos += sizeof(LinkedAnchorsArrayType)
        
        portals = []
        for i in range(ftsHeader.nb_portals):
            portal = EERIE_SAVE_PORTALS.from_buffer_copy(data, pos)
            pos += sizeof(EERIE_SAVE_PORTALS)
            portals.append(portal)
        result["portals"] = portals
            
        for i in range(ftsHeader.nb_rooms + 1): # Off by one in data
            room = EERIE_SAVE_ROOM_DATA.from_buffer_copy(data, pos)
            pos += sizeof(EERIE_SAVE_ROOM_DATA)
            
            if room.nb_portals > 0:
                PortalsArrayType = c_int32 * room.nb_portals
                portals = PortalsArrayType.from_buffer_copy(data, pos)
                pos += sizeof(PortalsArrayType)
                
            if room.nb_polys > 0:
                PolysArrayType = FAST_EP_DATA * room.nb_polys
                portals = PolysArrayType.from_buffer_copy(data, pos)
                pos += sizeof(PolysArrayType)
        
        for i in range(ftsHeader.nb_rooms):
            for j in range(ftsHeader.nb_rooms):
                dist = ROOM_DIST_DATA_SAVE.from_buffer_copy(data, pos)
                pos += sizeof(ROOM_DIST_DATA_SAVE)
                
        self.log.info("Loaded %i bytes of %i" % (pos, len(data)))
        return result

    def read_fts_container(self, filepath):
        f = open(filepath, "rb")
        data = f.read()
        f.close()

        self.log.info("Loaded %i bytes from file %s" % (len(data), filepath))
        
        pos = 0
        
        primaryHeader = UNIQUE_HEADER.from_buffer_copy(data, pos)
        pos += sizeof(UNIQUE_HEADER)
        self.log.info("Header path: %s" % primaryHeader.path.decode('iso-8859-1'))
        self.log.info("Header count: %i" % primaryHeader.count)
        self.log.info("Header version: %f" % primaryHeader.version)
        self.log.info("Header uncompressedsize: %i" % primaryHeader.uncompressedsize)
            
        secondaryHeadersType = UNIQUE_HEADER3 * primaryHeader.count
        secondaryHeaders = secondaryHeadersType.from_buffer_copy(data, pos)
        pos += sizeof(secondaryHeadersType)
        
        for h in secondaryHeaders:
            self.log.info("Header2 path: %s" % h.path.decode('iso-8859-1'))
        
        uncompressed = self.ioLib.unpack(data[pos:])
        
        if primaryHeader.uncompressedsize != len(uncompressed):
            self.log.warn("Uncompressed size mismatch, expected %i actual %i" % (primaryHeader.uncompressedsize, len(uncompressed)))
        
        return self.read_fts(uncompressed)
