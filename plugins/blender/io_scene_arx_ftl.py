bl_info = {
    "name": "Arx Fatalis Model",
    "author": "nemyax, Eli2",
    "version": (0, 0, 20140116),
    "blender": (2, 6, 9),
    "location": "File > Import-Export",
    "description": "Import and export Arx Fatalis .ftl models",
    "warning": "",
    "wiki_url": "",
    "tracker_url": "",
    "category": "Import-Export"}

import bpy
import bmesh
import mathutils as mu
import platform
import os.path
import struct
from struct import pack, unpack
import math
import ctypes
from ctypes import *
from bpy.props import (
    StringProperty,
    FloatProperty,
    BoolProperty)
from bpy_extras.io_utils import (
    ExportHelper,
    ImportHelper,
    path_reference_mode)
from collections import namedtuple

import logging

logging.basicConfig(level=logging.INFO)
log = logging.getLogger('Arx Pipeline')


### 
### Arx Libertatis IO library interface by Eli2
### 

class ArxIO(object):
    messageBufferSize = 512

    def __init__(self, libraryPath):
        self.lib = cdll.LoadLibrary(libraryPath)
        self.lib.ArxIO_init()

    def getError(self):
        errorBuffer = create_string_buffer(self.messageBufferSize)
        self.lib.ArxIO_getError(errorBuffer, self.messageBufferSize)
        return RuntimeError(errorBuffer.value)
        
    def printLog(self):
        readMore = True
        while readMore:
            logBuffer = create_string_buffer(self.messageBufferSize)
            result = self.lib.ArxIO_getLogLine(logBuffer, self.messageBufferSize)
            log.info("Lib " + logBuffer.value)
            if result == 0:
                readMore = False;

    def getRawFtl(self, filePath):
        CfilePath = c_char_p(filePath)
        result = self.lib.ArxIO_ftlLoad(CfilePath)
        if result < 0:
            raise self.getError()
        dataBufferSize = self.lib.ArxIO_ftlGetRawDataSize()
        dataBuffer = create_string_buffer(dataBufferSize)
        self.lib.ArxIO_ftlGetRawData(dataBuffer, dataBufferSize)
        if result < 0:
            raise self.getError()
        self.lib.ArxIO_ftlRelease()
        return dataBuffer.raw

if platform.system() == "Windows":
    libPath = os.path.realpath(__file__ + "\..\ArxIO.dll")
else:
    libPath = os.path.realpath(__file__ + "/../ArxIO.so")
#print("libPath:", libPath)
try:
    ftlReader = ArxIO(libPath)
    print("Loaded library:", ftlReader, "(" + libPath + ")")
except OSError:
    print("Cannot find", libPath + ". Compressed FTL import disabled.")
    ftlReader = None

###
### Utilities
###

def strip_wires(bm):
    [bm.verts.remove(v) for v in bm.verts if v.is_wire]
    [bm.verts.remove(v) for v in bm.verts if not v.link_faces[:]]
    [bm.edges.remove(e) for e in bm.edges if not e.link_faces[:]]
    [bm.faces.remove(f) for f in bm.faces if len(f.edges) < 3]
    for seq in [bm.verts, bm.faces, bm.edges]: seq.index_update()
    return bm

###
### Import
###

class SavedVec3(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("x", c_float),
        ("y", c_float),
        ("z", c_float)
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

class EERIE_OLD_VERTEX(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("vert", SavedTextureVertex),
        ("v",    SavedVec3),
        ("norm", SavedVec3)
    ]

class PolyTypeFlag_bits(ctypes.LittleEndianStructure):
     _fields_ = [
        ("POLY_NO_SHADOW",    ctypes.c_uint8, 1),
        ("POLY_DOUBLESIDED",  ctypes.c_uint8, 1),
        ("POLY_TRANS",        ctypes.c_uint8, 1),
        ("POLY_WATER",        ctypes.c_uint8, 1),
        ("POLY_GLOW",         ctypes.c_uint8, 1),
        ("POLY_IGNORE",       ctypes.c_uint8, 1),
        ("POLY_QUAD",         ctypes.c_uint8, 1),
        ("POLY_TILED",        ctypes.c_uint8, 1), # Unused
        ("POLY_METAL",        ctypes.c_uint8, 1),
        ("POLY_HIDE",         ctypes.c_uint8, 1),
        ("POLY_STONE",        ctypes.c_uint8, 1),
        ("POLY_WOOD",         ctypes.c_uint8, 1),
        ("POLY_GRAVEL",       ctypes.c_uint8, 1),
        ("POLY_EARTH",        ctypes.c_uint8, 1),
        ("POLY_NOCOL",        ctypes.c_uint8, 1),
        ("POLY_LAVA",         ctypes.c_uint8, 1),
        ("POLY_CLIMB",        ctypes.c_uint8, 1),
        ("POLY_FALL",         ctypes.c_uint8, 1),
        ("POLY_NOPATH",       ctypes.c_uint8, 1),
        ("POLY_NODRAW",       ctypes.c_uint8, 1),
        ("POLY_PRECISE_PATH", ctypes.c_uint8, 1),
        ("POLY_NO_CLIMB",     ctypes.c_uint8, 1), # Unused
        ("POLY_ANGULAR",      ctypes.c_uint8, 1), # Unused
        ("POLY_ANGULAR_IDX0", ctypes.c_uint8, 1), # Unused
        ("POLY_ANGULAR_IDX1", ctypes.c_uint8, 1), # Unused
        ("POLY_ANGULAR_IDX2", ctypes.c_uint8, 1), # Unused
        ("POLY_ANGULAR_IDX3", ctypes.c_uint8, 1), # Unused
        ("POLY_LATE_MIP",     ctypes.c_uint8, 1),
    ]
     
class PolyTypeFlag(ctypes.Union):
    _fields_ = [
        ("b",      PolyTypeFlag_bits),
        ("asUInt", ctypes.c_uint32),
    ]
    _anonymous_ = ("b")

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


def parse_ftl(ftlBytes):
    pos = 0
    
    primaryHeader = ARX_FTL_PRIMARY_HEADER.from_buffer_copy(ftlBytes, pos)
    pos += sizeof(ARX_FTL_PRIMARY_HEADER)
    log.debug("Loading ftl file version: %f" % primaryHeader.version)
    
    secondaryHeader = ARX_FTL_SECONDARY_HEADER.from_buffer_copy(ftlBytes, pos)
    
    if secondaryHeader.offset_3Ddata == -1:
        log.error("Invalid offset to 3d data")
        return
    
    pos = secondaryHeader.offset_3Ddata
    
    chunkHeader = ARX_FTL_3D_DATA_HEADER.from_buffer_copy(ftlBytes, pos)
    pos += sizeof(ARX_FTL_3D_DATA_HEADER)
    
    verts = []
    for i in range(chunkHeader.nb_vertex):
        vert = EERIE_OLD_VERTEX.from_buffer_copy(ftlBytes, pos)
        pos += sizeof(EERIE_OLD_VERTEX)
        verts.append(list((vert.v.x, vert.v.y, vert.v.z)))
    
    faces = []
    for i in range(chunkHeader.nb_faces):
        face = EERIE_FACE_FTL.from_buffer_copy(ftlBytes, pos)
        pos += sizeof(EERIE_FACE_FTL)
        faces.append((face.vid, zip(face.u, face.v), face.texid, face.facetype.asUInt, face.transval))
    
    mats = []
    for i in range(chunkHeader.nb_maps):
        texture = Texture_Container_FTL.from_buffer_copy(ftlBytes, pos)
        pos += sizeof(Texture_Container_FTL)
        mats.append(texture.name.decode('iso-8859-1'))
    
    temp = []
    for i in range(chunkHeader.nb_groups):
        group = EERIE_GROUPLIST_FTL.from_buffer_copy(ftlBytes, pos)
        pos += sizeof(EERIE_GROUPLIST_FTL)
        temp.append((group.name.decode('iso-8859-1'), group.nb_index))
    
    groups = []
    for (name, count) in temp:
        vertArray = c_int32 * count
        vertsIndices = vertArray.from_buffer_copy(ftlBytes, pos)
        pos += sizeof(vertsIndices)
        groups.append((name, list(vertsIndices)))
    
    actions = []
    for i in range(chunkHeader.nb_action):
        action = EERIE_ACTIONLIST_FTL.from_buffer_copy(ftlBytes, pos)
        pos += sizeof(EERIE_ACTIONLIST_FTL)
        actions.append((action.name.decode('iso-8859-1'), verts[action.idx]))
    
    temp = []
    for i in range(chunkHeader.nb_selections):
        selection = EERIE_SELECTIONS_FTL.from_buffer_copy(ftlBytes, pos)
        pos += sizeof(EERIE_SELECTIONS_FTL)
        temp.append((selection.name.decode('iso-8859-1'), selection.nb_selected))
    
    sels = []
    for (name, count) in temp:
        vertArray = c_int32 * count
        vertsIndices = vertArray.from_buffer_copy(ftlBytes, pos)
        pos += sizeof(vertsIndices)
        sels.append((name, list(vertsIndices)))
    
    data = {}
    data['verts']   = verts
    data['faces']   = faces
    data['mats']    = mats
    data['groups']  = groups
    data['actions'] = actions
    data['sels']    = sels
    
    return (data, chunkHeader.name.decode('iso-8859-1'))

def build_initial_bmesh(vertData, faceData, correctionMatrix):
    bm = bmesh.new()
    
    arxFaceType = bm.faces.layers.int.new('arx_facetype')
    arxTransVal = bm.faces.layers.float.new('arx_transval')
    
    uvData = bm.loops.layers.uv.verify()
    for xyz in vertData:
        bm.verts.new(xyz)
        bm.verts.index_update()
    
    for (vertIndexes, uvs, mat, faceType, trans) in faceData:
        faceVerts = [bm.verts[v] for v in vertIndexes]
        evDict = {}
        try:
            f = bm.faces.new(faceVerts)
        except ValueError:
            extraVerts = []
            for v in faceVerts:
                ev = bm.verts.new(v.co)
                bm.verts.index_update()
                extraVerts.append(ev)
                evDict[v] = ev
            f = bm.faces.new(extraVerts)
        
        if mat >= 0:
            f.material_index = mat
        
        f[arxFaceType] = faceType
        f[arxTransVal] = trans
            
        bm.faces.index_update()
        for c in f.loops:
            u, v = next(uvs)
            c[uvData].uv = u, 1.0 - v
    bm.edges.index_update()
    bm.transform(correctionMatrix)
    return (bm,evDict)

def assign_groups(bm, groups, evDict):
    weightData = bm.verts.layers.deform.verify()
    gi = 0
    for (whatever,indexes) in groups:
        extra = []
        for i in indexes:
            if i in evDict.keys():
                extra.extend(evDict[i])
        indexes.extend(extra)
        for vi in indexes:
            bm.verts[vi][weightData][gi] = 1.0
        gi += 1
    return bm

def createMaterial(rootDirectory, textureName):
    relativePath, fileExtension = os.path.splitext(textureName.replace("\\", "/").lower())
    foo, fileName = os.path.split(relativePath)
    
    mat = bpy.data.materials.new(fileName + "-mat")
    mat.use_shadeless = True
    
    extensions = [".png", ".jpg", ".jpeg", ".bmp", ".tga"]
    for ext in extensions:
        fullPath = os.path.join(rootDirectory, relativePath) + ext
        if os.path.exists(fullPath):
            break
    
    if not os.path.exists(fullPath):
        log.warning("Texture not found: %s" % textureName)
        return mat
    
    if relativePath in bpy.data.images:
        img = bpy.data.images[relativePath]
    else:
        img = bpy.data.images.load(fullPath)
        img.name = relativePath
    
    
    tex = bpy.data.textures.new(fileName + "-tex", type = 'IMAGE')
    tex.image = img
    tex.use_alpha = True
    
    mtex = mat.texture_slots.add()
    mtex.texture = tex
    mtex.texture_coords = 'UV'
    mtex.use_map_color_diffuse = True
    return mat

def make_object(bm, data, evDict, name, correctionMatrix, assetsRootDirectory):
    groups = data['groups']
    groups.extend([("sel:"+n,i) for (n,i) in data['sels']])
    mesh = bpy.data.meshes.new(name)
    bm = assign_groups(bm, groups, evDict)
    bm = strip_wires(bm)
    bm.to_mesh(mesh)
    bm.free()
    if bpy.context.active_object:
        bpy.ops.object.mode_set(mode='OBJECT')
    obj = bpy.data.objects.new(name=mesh.name, object_data=mesh)
    for (name,whatever) in groups:
        obj.vertex_groups.new(name=name)
    bpy.context.scene.objects.link(obj)
    bpy.context.scene.objects.active = obj
    bpy.ops.object.mode_set(mode='EDIT') # initialises UVmap correctly
    mesh.uv_textures.new()
    for m in data['mats']:
        mat = createMaterial(assetsRootDirectory, m)
        obj.data.materials.append(mat)
    
    for (name,coords) in data['actions']:
        action = bpy.data.objects.new(name, None)
        bpy.context.scene.objects.link(action)
        action.parent = obj
        action.location = correctionMatrix * mu.Vector(coords)
    return {'FINISHED'}

def do_import(fileName, correctionMatrix, assetsRootDirectory):
    f = open(fileName, 'r+b')
    bs = f.read(-1)
    f.close()
    
    if bs[:4] != b'FTL\x00':
        try:
            bs = ftlReader.getRawFtl(bytes(fileName, encoding="utf-8"))
        except:
            return ("Cannot understand the file format.",{'CANCELLED'})
    
    data, name = parse_ftl(bs)
    
    bm, evDict = build_initial_bmesh(
        data['verts'],
        data['faces'],
        correctionMatrix)
    
    msg = "File \"" + fileName + "\" loaded successfully."
    result = make_object(
        bm,
        data,
        evDict,
        name,
        correctionMatrix,
        assetsRootDirectory)
    return (msg,result)

###
### Export
###

class Vertex(object):
    def __init__(self, xyz, n):
        self.xyz = xyz
        self.n = n

class Face(object):
    def __init__(self, verts, us, vs, ns, n, mat):
        self.verts = verts
        self.us = us
        self.vs = vs
        self.ns = ns
        self.n = n
        self.mat = mat

class Model(object):
    def __init__(self, bm, mats, groups, actions):
        uvData = bm.loops.layers.uv.verify()
        # verts = list(set(
            # [v.co[:] for v in bm.verts] +\
            # [a[1][:] for a in actions] +\
            # [(0.0,0.0,0.0)]))
        verts = []
        for v in bm.verts:
            ns = [sc.normal for sc in bm.verts if sc.co == v.co]
            normal = mu.Vector((0.0,0.0,0.0))
            for n in ns:
                normal += v.normal
            normal.normalize()
            verts.append(Vertex(v.co[:], normal))
        for a in actions:
            actionXYZ = a[1][:]
            if actionXYZ not in [v.xyz for v in verts]:
                verts.append(Vertex(actionXYZ, (0.0,0.0,0.0)))
        if (0.0,0.0,0.0) not in [v.xyz for v in verts]:
            verts.append(Vertex((0.0,0.0,0.0), (0.0,0.0,0.0)))
        verts = list(set(verts))
        allXYZ = [v.xyz for v in verts]
        faces = []
        for f in bm.faces:
            vis = []
            us = []
            vs = []
            ns = []
            for c in f.loops:
                vis.append(allXYZ.index(c.vert.co[:]))
                u, v = c[uvData].uv
                us.append(u)
                vs.append(v)
                ns.extend(c.vert.normal)
            mat = f.material_index
            faces.append(Face(vis, us, vs, ns, f.normal[:], mat))
        self.verts = verts
        self.faces = faces
        self.mats = mats
        self.actions = actions
        grpLookup = {}
        for g in groups:
            grpLookup[g] = set()
        grpData = bm.verts.layers.deform.verify()
        for v in bm.verts:
            membership = v[grpData].keys()
            for i in membership:
                grpLookup[groups[i]].add(allXYZ.index(v.co[:]))
        self.groups = [(k,grpLookup[k]) for k in grpLookup.keys()
            if not k.startswith("sel:")]
        self.sels = [(k,grpLookup[k]) for k in grpLookup.keys()
            if k.startswith("sel:")]
    def encode_verts(self):
        result = b''
        for v in self.verts:
            result = concat_bytes([
                result,
                bytes(32), # SavedTextureVertex
                encode_floats(v.xyz),
                encode_floats(v.n)])
        # print("verts in result:", len(result)/56)
        return result
    def encode_faces(self):
        result = b''
        for f in self.faces:
            result = concat_bytes([
                result,
                bytes(4), # face type: flat
                bytes(12), # rgb
                encode_ushorts(f.verts),
                pack('<h', f.mat),
                encode_floats(f.us),
                encode_floats(f.vs),
                bytes(6), # "ou" * 3
                bytes(6), # "ov" * 3
                bytes(4), # "transval"
                encode_floats(list(f.n) + f.ns),
                bytes(4)]) # "temp"
        return result
    def encode_mats(self):
        return concat_bytes(encode_names(self.mats, 256))
    def encode_groups(self):
# char name[256];
# s32 origin;
# s32 nb_index;
# s32 indexes;
# f32 siz;
        names = encode_names([n for (n,l) in self.groups], 256)
        sizes = [len(l) for (n,l) in self.groups]
        headers = b''
        for (name,size) in zip(names, sizes):
            headers += concat_bytes([
                name,
                encode_ints([0,size,0]),
                pack('<f', 1.0)])
        allMembers = []
        for (n,l) in self.groups:
            allMembers.extend(l)
        return concat_bytes([headers,encode_ints(allMembers)])
    def encode_actions(self):
        names = encode_names([n for (n,v) in self.actions], 256)
        allXYZ = [v.xyz for v in self.verts]
        indexes = [allXYZ.index(v) for (n,v) in self.actions]
        result = b''
        for (n,i) in zip(names, indexes):
            result = concat_bytes([result,n,encode_ints([i,-1,-1])])
        return result
    def encode_sels(self):
        names = encode_names([n[4:] for (n,l) in self.sels], 64)
        sizes = [len(l) for (n,l) in self.sels]
        headers = b''
        for (name,size) in zip(names, sizes):
            headers += concat_bytes([
                name,
                encode_ints([size,0])])
        allMembers = []
        for (n,l) in self.sels:
            allMembers.extend(l)
        return concat_bytes([headers,encode_ints(allMembers)])
# SavedVec3 pos;
# f32 rhw;
# u32 color;
# u32 specular;
# f32 tu;
# f32 tv;
# SavedVec3 v;
# SavedVec3 norm;

def encode_names(names, length):
    newNames = []
    counts = {}
    for n in names:
        new = ascii(n)[1:-1][:length]
        if new in counts.keys():
            counts[new] += 1
        else:
            counts[new] = 1
    for k in counts.keys():
        count = counts[k]
        if count == 1:
            newNames.append(k)
        else:
            suffixLen = len(str(count))
            for i in range(count):
                suffix = str(i).zfill(suffixLen)
                trunc = min([length-suffixLen,len(k)])
                newNames.append(k[:trunc] + suffix)
    binNames = []
    for nn in newNames:
        binName = bytes([ord(c) for c in nn])
        while len(binName) < length:
            binName += b'\x00'
        binNames.append(binName)
    return binNames

def triangulate(bm):
    while True:
        nonTris = [f for f in bm.faces if len(f.verts) > 3]
        if nonTris:
            nt = nonTris[0]
            pivotLoop = nt.loops[0]
            allVerts = nt.verts
            vert1 = pivotLoop.vert
            wrongVerts = [vert1,
                pivotLoop.link_loop_next.vert,
                pivotLoop.link_loop_prev.vert]
            bmesh.utils.face_split(nt, vert1, [v for v in allVerts
                if v not in wrongVerts][0])
            for seq in [bm.verts, bm.faces, bm.edges]: seq.index_update()
        else: break
    return bm

def concat_bytes(bytesList):
    result = b''
    while bytesList:
        result = bytesList.pop() + result
    return result

def encode(fmt, what):
    return concat_bytes([pack(fmt, i) for i in what])

def encode_floats(floats):
    return encode('<f', floats)

def encode_uints(uints):
    return encode('<I', uints)

def encode_ints(ints):
    return encode('<i', ints)

def encode_ushorts(ushorts):
    return encode('<H', ushorts)

def build_ftl(model):
    origin = 0
    for v in model.verts:
        if v.xyz == (0.0,0.0,0.0) and v.n == (0.0,0.0,0.0):
            break
        origin += 1
    geomHeader = encode_ints([
        len(model.verts),
        len(model.faces),
        len(model.mats),
        len(model.groups),
        len(model.actions),
        len(model.sels),
        origin]) + bytes(256)
    return concat_bytes([
        b'FTL\x00',
        pack('<f', 0.83257),
        bytes(512), # fake checksum
        pack('<i', 544), # 3d data header offset
        b'\xff' * 20, # 5 times 32-bit -1: skipped offsets
        geomHeader,
        model.encode_verts(),
        model.encode_faces(),
        model.encode_mats(),
        model.encode_groups(),
        model.encode_actions(),
        model.encode_sels()])

def prep_groups(objs):
    allGroups = []
    for o in objs:
        allGroups.extend([(g.name,o.name) for g in o.vertex_groups])
    result = []
    for (gn,on) in allGroups:
        if len([None for (gn2,on2) in allGroups if gn == gn2]) > 1:
            result.append(gn + "_" + on)
        else:
            result.append(gn)
    return result

def append_object(bm1, o, mats, groups):
    matSlotLookup = {}
    for i in range(len(o.material_slots)):
        maybeMat = o.material_slots[i].material
        if maybeMat:
            matSlotLookup[i] = mats.index(maybeMat)
    grpLookup = {}
    for i in range(len(o.vertex_groups)):
        gn = o.vertex_groups[i].name
        try:
            grpLookup[i] = groups.index(gn)
        except ValueError:
            grpLookup[i] = groups.index(gn + "_" + o.name)
    bm2 = bmesh.new()
    bm2.from_object(o, bpy.context.scene)
    bm2.transform(o.matrix_world)
    bm2 = triangulate(strip_wires(bm2))
    for f in bm2.faces:
        origMat = f.material_index
        if origMat in matSlotLookup.keys():
            f.material_index = matSlotLookup[origMat]
    uvData = bm1.loops.layers.uv.verify()
    uvDataOrig = bm2.loops.layers.uv.verify()
    grpData = bm1.verts.layers.deform.verify()
    grpDataOrig = bm2.verts.layers.deform.verify()
    vSoFar = len(bm1.verts)
    for v in bm2.verts:
        nv = bm1.verts.new(v.co)
        for gi in v[grpDataOrig].keys():
            nv[grpData][grpLookup[gi]] = 1.0
        bm1.verts.index_update()
    for f in bm2.faces:
        nf = bm1.faces.new(
            [bm1.verts[vSoFar+v.index] for v in f.verts])
        for i in range(len(f.loops)):
            origU, origV = f.loops[i][uvDataOrig].uv
            nf.loops[i][uvData].uv = origU, 1.0 - origV
            nf.material_index = f.material_index
        bm1.faces.index_update()
    bm2.free()
    return bm1

def do_export(fileName, correctionMatrix):
    mats = [m for m in bpy.data.materials
        if any([m in [ms.material for ms in o.material_slots]
            for o in bpy.data.objects])]
    objs = [o for o in bpy.data.objects
        if o.type == 'MESH' and not o.hide]
    if not objs:
        return ("Nothing to export.",{'CANCELLED'})
    groups = prep_groups(objs)
    actions = [(
        o.name,
        (correctionMatrix * o.matrix_world.to_translation())[:])
        for o in bpy.data.objects
        if o.type == 'EMPTY'
        and o.parent in objs]
    bm = bmesh.new()
    for o in objs:
        bm = append_object(bm, o, mats, groups)
    bm.transform(correctionMatrix)
    bm.normal_update()
    model = Model(bm, [m.name for m in mats], groups, actions)
    bm.free()
    f = open(fileName, 'w+b')
    f.write(build_ftl(model))
    f.close()
    msg = "File \"" + fileName + "\" written successfully."
    result = {'FINISHED'}
    return (msg,result)

###
### UI
###

class ImportFTL(bpy.types.Operator, ImportHelper):
    '''Load an Arx Fatalis Model File'''
    bl_idname = "import_scene.ftl"
    bl_label = 'Import FTL'
    bl_options = {'PRESET'}
    filename_ext = ".ftl"
    filter_glob = StringProperty(
        default="*.ftl",
        options={'HIDDEN'})
    path_mode = path_reference_mode
    check_extension = True
    path_mode = path_reference_mode
    reorient = BoolProperty(
            name="Reorient",
            description="Make the model Z-up",
            default=True)
    scaleFactor = FloatProperty(
            name="Scale",
            description="Scale all data",
            min=0.01, max=1000.0,
            soft_min=0.01,
            soft_max=1000.0,
            default=0.1)
    assetsRootDirectory = StringProperty(name="Assets directory", subtype='DIR_PATH')
    
    def execute(self, context):
        maybeTurn = float(self.reorient)
        correctionMatrix = \
            mu.Matrix.Rotation(math.radians(180 * maybeTurn), 4, 'Z') *\
            mu.Matrix.Rotation(math.radians(-90 * maybeTurn), 4, 'X') *\
            mu.Matrix.Scale(self.scaleFactor, 4)
        msg, result = do_import(self.filepath, correctionMatrix, self.assetsRootDirectory)
        print(msg)
        if result == {'CANCELLED'}:
            self.report({'ERROR'}, msg)
        return result

class ExportFTL(bpy.types.Operator, ExportHelper):
    '''Save a raw uncompressed Arx Fatalis Model File'''
    bl_idname = "export_scene.ftl"
    bl_label = 'Export FTL'
    bl_options = {'PRESET'}
    filename_ext = ".ftl"
    filter_glob = StringProperty(
        default="*.ftl",
        options={'HIDDEN'})
    path_mode = path_reference_mode
    check_extension = True
    path_mode = path_reference_mode
    reorient = BoolProperty(
            name="Reorient",
            description="Make the model Z-up",
            default=True)
    scaleFactor = FloatProperty(
            name="Scale",
            description="Scale all data",
            min=0.01, max=1000.0,
            soft_min=0.01,
            soft_max=1000.0,
            default=10.0)
    def execute(self, context):
        maybeTurn = float(self.reorient)
        correctionMatrix = \
            mu.Matrix.Rotation(math.radians(180 * maybeTurn), 4, 'Z') *\
            mu.Matrix.Rotation(math.radians(-90 * maybeTurn), 4, 'X') *\
            mu.Matrix.Scale(self.scaleFactor, 4)
        msg, result = do_export(self.filepath, correctionMatrix)
        if result == {'CANCELLED'}:
            self.report({'ERROR'}, msg)
        print(msg)
        return result

def menu_func_import_ftl(self, context):
    self.layout.operator(
        ImportFTL.bl_idname, text="Arx Fatalis Model (.ftl)")

def menu_func_export_ftl(self, context):
    self.layout.operator(
        ExportFTL.bl_idname, text="Arx Fatalis Model (.ftl)")


class ArxFacePanel(bpy.types.Panel):
    bl_label = "Arx Face Properties"
    bl_space_type = 'VIEW_3D'
    bl_region_type = "UI"
    
    def draw(self, context):
        ob = bpy.context.object
        
        if ob is None:
            return
        
        if ob.type != 'MESH':
            raise TypeError("Active object is not a Mesh")
        
        me = ob.data
        
        if me.is_editmode:
            # Gain direct access to the mesh
            bm = bmesh.from_edit_mesh(me)
        else:
            # Create a bmesh from mesh
            # (won't affect mesh, unless explicitly written back)
            bm = bmesh.new()
            bm.from_mesh(me)
        
        # Get active face
        face = bm.faces.active
        
        if face is None:
            return
        
        arxFaceType = bm.faces.layers.int.get('arx_facetype')
        arxTransVal = bm.faces.layers.float.get('arx_transval')
        
        faceType = PolyTypeFlag()
        faceType.asUInt = face[arxFaceType]
        
        transval = face[arxTransVal]
        
        obj = bpy.context.active_object
        
        layout = self.layout
        
        layout.label(text="transval: " + str(transval))
        layout.label(text="POLY_NO_SHADOW: " + str(faceType.POLY_NO_SHADOW))
        layout.label(text="POLY_DOUBLESIDED: " + str(faceType.POLY_DOUBLESIDED))
        layout.label(text="POLY_TRANS: " + str(faceType.POLY_TRANS))
        layout.label(text="POLY_WATER: " + str(faceType.POLY_WATER))
        layout.label(text="POLY_GLOW: " + str(faceType.POLY_GLOW))
        layout.label(text="POLY_IGNORE: " + str(faceType.POLY_IGNORE))
        layout.label(text="POLY_QUAD: " + str(faceType.POLY_QUAD))
        layout.label(text="POLY_METAL: " + str(faceType.POLY_METAL))
        layout.label(text="POLY_HIDE: " + str(faceType.POLY_HIDE))
        layout.label(text="POLY_STONE: " + str(faceType.POLY_STONE))
        layout.label(text="POLY_WOOD: " + str(faceType.POLY_WOOD))
        layout.label(text="POLY_GRAVEL: " + str(faceType.POLY_GRAVEL))
        layout.label(text="POLY_EARTH: " + str(faceType.POLY_EARTH))
        layout.label(text="POLY_NOCOL: " + str(faceType.POLY_NOCOL))
        layout.label(text="POLY_LAVA: " + str(faceType.POLY_LAVA))
        layout.label(text="POLY_CLIMB: " + str(faceType.POLY_CLIMB))
        layout.label(text="POLY_FALL: " + str(faceType.POLY_FALL))
        layout.label(text="POLY_NOPATH: " + str(faceType.POLY_NOPATH))
        layout.label(text="POLY_NODRAW: " + str(faceType.POLY_NODRAW))
        layout.label(text="POLY_PRECISE_PATH: " + str(faceType.POLY_PRECISE_PATH))
        layout.label(text="POLY_LATE_MIP: " + str(faceType.POLY_LATE_MIP))



def register():
    bpy.utils.register_module(__name__)
    bpy.types.INFO_MT_file_import.append(menu_func_import_ftl)
    bpy.types.INFO_MT_file_export.append(menu_func_export_ftl)

def unregister():
    bpy.utils.unregister_module(__name__)
    bpy.types.INFO_MT_file_import.remove(menu_func_import_ftl)
    bpy.types.INFO_MT_file_export.remove(menu_func_export_ftl)
