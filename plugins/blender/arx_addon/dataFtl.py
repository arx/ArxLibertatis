# Copyright 2014-2020 Arx Libertatis Team (see the AUTHORS file)
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
        ("rgb",      c_uint32 * 3), # always zero
        ("vid",      c_uint16 * 3),
        ("texid",    c_int16),
        ("u",        c_float * 3),
        ("v",        c_float * 3),
        ("ou",       c_uint16 * 3),
        ("ov",       c_uint16 * 3),
        ("transval", c_float),
        ("norm",     SavedVec3),
        ("nrmls",    SavedVec3 * 3), # always zero
        ("temp",     c_float),
    ]

    def toTuple(self):
        return (self.facetype.asUInt,
                (self.vid[0], self.vid[1], self.vid[2]),
                self.texid,
                (self.u[0], self.u[1], self.u[2]),
                (self.v[0], self.v[1], self.v[2]),
                (self.ou[0], self.ou[1], self.ou[2]),
                (self.ov[0], self.ov[1], self.ov[2]),
                self.transval,
                (self.norm.x, self.norm.y, self.norm.z),
                self.temp)


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


def comp(a, b):
    for fld in a._fields_:
        attra = getattr(a, fld[0])
        attrb = getattr(b, fld[0])
        print("foo")

    return True


from collections import namedtuple
from dataclasses import dataclass

from ctypes import sizeof
import itertools

import logging
logging.basicConfig(level=logging.INFO)

from typing import Any, List


@dataclass(frozen=True)
class FtlMetadata():
    name: Any
    org: Any

@dataclass(frozen=True)
class FtlVertex():
    xyz: Any
    n: Any

@dataclass(frozen=True)
class FtlFace():
    vids: Any
    uvs: Any
    texid: Any
    facetype: Any
    transval: Any
    normal: Any

@dataclass
class FtlGroup():
    name: Any
    origin: Any
    indices: Any
    parentIndex: Any

@dataclass(frozen=True)
class FtlSelection():
    name: Any
    indices: Any

@dataclass(frozen=True)
class FtlAction():
    name: Any
    vidx: Any

@dataclass(frozen=True)
class FtlData():
    metadata: FtlMetadata
    verts: List[FtlVertex]
    faces: List[FtlFace]
    mats: List[str]
    groups: List[FtlGroup]
    actions: List[FtlAction]
    sels: List[FtlSelection]


def getFatherIndex(groups, childIndex):
    child = groups[childIndex]
    i = childIndex-1
    while i >= 0:
        group = groups[i]
        for vertexIndex in group.indices:
            if vertexIndex == child.origin:
                return i
        i-=1

    return -1

class FtlSerializer(object):
    def __init__(self):
        self.log = logging.getLogger('FtlSerializer')

    def read(self, data) -> FtlData:
        pos = 0

        primaryHeader = ARX_FTL_PRIMARY_HEADER.from_buffer_copy(data, pos)
        pos += sizeof(ARX_FTL_PRIMARY_HEADER)

        if primaryHeader.version != 0.8325700163841248:
            self.log.debug("Unexpected ftl version: %f" % primaryHeader.version)

        secondaryHeader = ARX_FTL_SECONDARY_HEADER.from_buffer_copy(data, pos)
        pos += sizeof(ARX_FTL_SECONDARY_HEADER)

        if secondaryHeader.offset_3Ddata == -1:
            self.log.error("Invalid offset to 3d data")
            return

        assert (secondaryHeader.offset_3Ddata == pos), "3D Data should follow secondary header"
        assert (secondaryHeader.offset_cylinder <= 0), "Cylinder data should be unused"
        assert (secondaryHeader.offset_progressive_data <= 0), "Progressive data should be unused"
        assert (secondaryHeader.offset_physics_box <= 0), "Physics data should be unused"

        if (secondaryHeader.offset_clothes_data > 0):
            self.log.debug("Found unused clothes data")

        if (secondaryHeader.offset_collision_spheres > 0):
            self.log.debug("Found unused collision_spheres data")

        pos = secondaryHeader.offset_3Ddata

        chunkHeader = ARX_FTL_3D_DATA_HEADER.from_buffer_copy(data, pos)
        pos += sizeof(ARX_FTL_3D_DATA_HEADER)

        metadata = FtlMetadata(name=chunkHeader.name.decode('iso-8859-1'), org=chunkHeader.origin)

        # self.log.debug("Name %s" % chunkHeader.name.decode('iso-8859-1'))  # This might be empty

        verts = []
        for i in range(chunkHeader.nb_vertex):
            vert = EERIE_OLD_VERTEX.from_buffer_copy(data, pos)
            pos += sizeof(EERIE_OLD_VERTEX)
            verts.append(FtlVertex(xyz=(vert.v.x, vert.v.y, vert.v.z), n=(vert.norm.x, vert.norm.y, vert.norm.z)))

        # debugFaces = []
        faces = []
        for i in range(chunkHeader.nb_faces):
            face = EERIE_FACE_FTL.from_buffer_copy(data, pos)
            pos += sizeof(EERIE_FACE_FTL)

            assert (face.rgb[0] == 0 and face.rgb[1] == 0 and face.rgb[2] == 0)
            assert (face.nrmls[0].x == 0 and face.nrmls[1].x == 0 and face.nrmls[2].x == 0)
            assert (face.nrmls[0].y == 0 and face.nrmls[1].y == 0 and face.nrmls[2].y == 0)
            assert (face.nrmls[0].z == 0 and face.nrmls[1].z == 0 and face.nrmls[2].z == 0)

            if face.texid < 0:
                self.log.debug("Face negaive texture id %i" % i)

            oface = FtlFace((face.vid[0], face.vid[1], face.vid[2]),
                            list(zip(face.u, face.v)),
                            face.texid,
                            face.facetype.asUInt,
                            face.transval,
                            (face.norm.x, face.norm.y, face.norm.z))
            faces.append(oface)
            """
            debugFaceVids = (face.vid[0], face.vid[1], face.vid[2])
            for j, df in debugFaces:
                myVids = (df.vid[0], df.vid[1], df.vid[2])
                for p in itertools.permutations(myVids):
                    if debugFaceVids == p:
                        print("face %i %s" % (i, str(face.toTuple())))
                        print("dupl %i %s" % (j, str(df.toTuple())))
            debugFaces.append((i, face))
            """

        mats = []
        for i in range(chunkHeader.nb_maps):
            texture = Texture_Container_FTL.from_buffer_copy(data, pos)
            pos += sizeof(Texture_Container_FTL)
            mats.append(texture.name.decode('iso-8859-1'))

        temp = []
        for i in range(chunkHeader.nb_groups):
            group = EERIE_GROUPLIST_FTL.from_buffer_copy(data, pos)
            pos += sizeof(EERIE_GROUPLIST_FTL)

            # print(str(group.indexes) + ":" + str(group.nb_index) + ":" + str(group.siz))
            # assert (group.indexes == group.nb_index)
            # assert (group.siz == 0)

            temp.append((group.name.decode('iso-8859-1'), group.origin, group.nb_index))

        groups = []
        for (name, origin, count) in temp:
            vertArray = c_int32 * count
            vertsIndices = vertArray.from_buffer_copy(data, pos)
            pos += sizeof(vertsIndices)
            # 0 temporarily for parentIndex.
            groups.append(FtlGroup(name, origin, list(vertsIndices), 0))

        # parenting
        for i, group in enumerate(groups):
            father = getFatherIndex(groups, i)
            self.log.debug("group %d with father %d" % (i,father))
            group.parentIndex = father

        actions = []
        for i in range(chunkHeader.nb_action):
            action = EERIE_ACTIONLIST_FTL.from_buffer_copy(data, pos)
            pos += sizeof(EERIE_ACTIONLIST_FTL)
            actions.append(FtlAction(action.name.decode('iso-8859-1'), action.idx))

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
            sels.append(FtlSelection(name, list(vertsIndices)))

        return FtlData(
            metadata=metadata,
            verts=verts,
            faces=faces,
            mats=mats,
            groups=groups,
            actions=actions,
            sels=sels
        )

    def write(self, data: FtlData) -> bytearray:

        result = bytearray()

        primaryHeader = ARX_FTL_PRIMARY_HEADER()
        primaryHeader.ident = b'FTL'
        primaryHeader.version = 0.8325700163841248
        result += bytearray(primaryHeader)

        secondaryHeader = ARX_FTL_SECONDARY_HEADER()
        secondaryHeader.offset_3Ddata = sizeof(ARX_FTL_PRIMARY_HEADER) + sizeof(ARX_FTL_SECONDARY_HEADER)

        secondaryHeader.offset_cylinder = -1
        secondaryHeader.offset_progressive_data = -1
        secondaryHeader.offset_clothes_data = -1
        secondaryHeader.offset_collision_spheres = -1
        secondaryHeader.offset_physics_box = -1
        result += bytearray(secondaryHeader)

        chunkHeader = ARX_FTL_3D_DATA_HEADER()
        chunkHeader.nb_vertex = len(data.verts)
        chunkHeader.nb_faces = len(data.faces)
        chunkHeader.nb_maps = len(data.mats)
        chunkHeader.nb_groups = len(data.groups)
        chunkHeader.nb_action = len(data.actions)
        chunkHeader.nb_selections = len(data.sels)

        chunkHeader.origin = data.metadata.org
        chunkHeader.name = data.metadata.name.encode('iso-8859-1')
        result += bytearray(chunkHeader)

        self.log.debug("Verts start: %s" % hex(len(result)))

        for v in data.verts:
            vertex = EERIE_OLD_VERTEX()
            vertex.v = SavedVec3(v.xyz[0], v.xyz[1], v.xyz[2])
            vertex.norm = SavedVec3(v.n[0], v.n[1], v.n[2])
            result += bytearray(vertex)

        self.log.debug("Faces start: %s" % hex(len(result)))

        for f in data.faces:
            face = EERIE_FACE_FTL()
            face.vid = (c_uint16 * 3)(*f.vids)
            face.u, face.v = zip(*f.uvs)
            
            if f[2] < len(data.mats):
                face.texid = f[2]
            else:
                face.texid = -1
            
            #face.texid = f[2]
            face.facetype.asUInt = f[3]
            face.transval = f[4]
            # face.norm.x = f.normal[0]
            # face.norm.y = f.normal[1]
            # face.norm.z = f.normal[2]
            result += bytearray(face)

        self.log.debug("Mats start: %s" % hex(len(result)))

        for m in data.mats:
            texture = Texture_Container_FTL()
            texture.name = m.encode('iso-8859-1')
            result += bytearray(texture)

        self.log.debug("Groups start: %s" % hex(len(result)))

        for g in data.groups:
            group = EERIE_GROUPLIST_FTL()
            group.name = g[0].encode('iso-8859-1')
            group.origin = g[1]
            group.nb_index = len(g[2])
            result += bytearray(group)

        for g in data.groups:
            VertArray = c_int32 * len(g[2])
            array = (VertArray)(*g[2])
            result += bytearray(array)

        self.log.debug("Actions start: %s" % hex(len(result)))

        for a in data.actions:
            action = EERIE_ACTIONLIST_FTL()
            action.name = a[0].encode('iso-8859-1')
            action.idx = a[1]
            result += bytearray(action)

        self.log.debug("Sels start: %s" % hex(len(result)))

        for s in data.sels:
            selection = EERIE_SELECTIONS_FTL()
            selection.name = s[0].encode('iso-8859-1')
            selection.nb_selected = len(s[1])
            result += bytearray(selection)

        for s in data.sels:
            VertArray = c_int32 * len(s[1])
            array = (VertArray)(*s[1])
            result += bytearray(array)

        return result
