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
    c_uint16,
    c_uint32,
    c_int16,
    c_int32,
    c_float
)

from .dataCommon import SavedVec3, PolyTypeFlag

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

class EERIE_OLD_VERTEX(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("vert", SavedTextureVertex),
        ("v",    SavedVec3),
        ("norm", SavedVec3)
    ]

class EERIE_FACE_FTL(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("facetype", PolyTypeFlag),
        ("rgb",      c_uint32 * 3),
        ("vid",      c_uint16 * 3),
        ("texid",    c_int16),
        ("u",        c_float * 3),
        ("v",        c_float * 3),
        ("ou",       c_uint16 * 3),
        ("ov",       c_uint16 * 3),
        ("transval", c_float),
        ("norm",     SavedVec3),
        ("nrmls",    SavedVec3 * 3),
        ("temp",     c_float),
    ]

class Texture_Container_FTL(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("name", c_char * 256)
    ]

class EERIE_GROUPLIST_FTL(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("name",     c_char * 256),
        ("origin",   c_int32),
        ("nb_index", c_int32),
        ("indexes",  c_int32),
        ("siz",      c_float)
    ]

class EERIE_ACTIONLIST_FTL(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("name",   c_char * 256),
        ("idx",    c_int32),
        ("action", c_int32),
        ("sfx",    c_int32),
    ]

class EERIE_SELECTIONS_FTL(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("name",        c_char * 64),
        ("nb_selected", c_int32),
        ("selected",    c_int32),
    ]


class ARX_FTL_PRIMARY_HEADER(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("ident",    c_char * 4),
        ("version",  c_float),
        ("checksum", c_char * 512) # In the c code this is currently not part of the header
    ]

class ARX_FTL_SECONDARY_HEADER(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("offset_3Ddata",            c_int32),
        ("offset_cylinder",          c_int32),
        ("offset_progressive_data",  c_int32),
        ("offset_clothes_data",      c_int32),
        ("offset_collision_spheres", c_int32),
        ("offset_physics_box",       c_int32)
    ]
    
class ARX_FTL_3D_DATA_HEADER(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("nb_vertex",     c_int32),
        ("nb_faces",      c_int32),
        ("nb_maps",       c_int32),
        ("nb_groups",     c_int32),
        ("nb_action",     c_int32),
        ("nb_selections", c_int32),
        ("origin",        c_int32),
        ("name",          c_char * 256)
    ]


import logging

from ctypes import sizeof

class FtlSerializer(object):
    def __init__(self, ioLib):
        self.log = logging.getLogger('FtlSerializer')
        self.ioLib = ioLib

    def readFile(self, fileName):
        f = open(fileName, "rb")
        data = f.read()
        f.close()
        self.log.debug("Loaded %i bytes from file %s" % (len(data), fileName))
        
        return self.read(self.ioLib.unpack(data))

    def read(self, data):
        pos = 0
        
        primaryHeader = ARX_FTL_PRIMARY_HEADER.from_buffer_copy(data, pos)
        pos += sizeof(ARX_FTL_PRIMARY_HEADER)
        self.log.debug("Loading ftl file version: %f" % primaryHeader.version)
        
        secondaryHeader = ARX_FTL_SECONDARY_HEADER.from_buffer_copy(data, pos)
        
        if secondaryHeader.offset_3Ddata == -1:
            self.log.error("Invalid offset to 3d data")
            return
        
        pos = secondaryHeader.offset_3Ddata
        
        chunkHeader = ARX_FTL_3D_DATA_HEADER.from_buffer_copy(data, pos)
        pos += sizeof(ARX_FTL_3D_DATA_HEADER)
        
        self.log.debug("Name %s" % chunkHeader.name.decode('iso-8859-1')) # This might be empty
        
        verts = []
        for i in range(chunkHeader.nb_vertex):
            vert = EERIE_OLD_VERTEX.from_buffer_copy(data, pos)
            pos += sizeof(EERIE_OLD_VERTEX)
            verts.append(list((vert.v.x, vert.v.y, vert.v.z)))
        
        faces = []
        for i in range(chunkHeader.nb_faces):
            face = EERIE_FACE_FTL.from_buffer_copy(data, pos)
            pos += sizeof(EERIE_FACE_FTL)
            faces.append((face.vid, zip(face.u, face.v), face.texid, face.facetype.asUInt, face.transval))
        
        mats = []
        for i in range(chunkHeader.nb_maps):
            texture = Texture_Container_FTL.from_buffer_copy(data, pos)
            pos += sizeof(Texture_Container_FTL)
            mats.append(texture.name.decode('iso-8859-1'))
        
        temp = []
        for i in range(chunkHeader.nb_groups):
            group = EERIE_GROUPLIST_FTL.from_buffer_copy(data, pos)
            pos += sizeof(EERIE_GROUPLIST_FTL)
            temp.append((group.name.decode('iso-8859-1'), group.nb_index))
        
        groups = []
        for (name, count) in temp:
            vertArray = c_int32 * count
            vertsIndices = vertArray.from_buffer_copy(data, pos)
            pos += sizeof(vertsIndices)
            groups.append((name, list(vertsIndices)))
        
        actions = []
        for i in range(chunkHeader.nb_action):
            action = EERIE_ACTIONLIST_FTL.from_buffer_copy(data, pos)
            pos += sizeof(EERIE_ACTIONLIST_FTL)
            actions.append((action.name.decode('iso-8859-1'), verts[action.idx]))
        
        temp = []
        for i in range(chunkHeader.nb_selections):
            selection = EERIE_SELECTIONS_FTL.from_buffer_copy(data, pos)
            pos += sizeof(EERIE_SELECTIONS_FTL)
            temp.append((selection.name.decode('iso-8859-1'), selection.nb_selected))
        
        sels = []
        for (name, count) in temp:
            vertArray = c_int32 * count
            vertsIndices = vertArray.from_buffer_copy(data, pos)
            pos += sizeof(vertsIndices)
            sels.append((name, list(vertsIndices)))
        
        result = {}
        result['verts']   = verts
        result['faces']   = faces
        result['mats']    = mats
        result['groups']  = groups
        result['actions'] = actions
        result['sels']    = sels
        
        return result
