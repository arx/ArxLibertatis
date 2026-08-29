#!/usr/bin/env python3
"""Build a minimal Arx level from scratch, for testing the exporter and the bake.

One box room, thirty quads, one light, one room, no portals and no entities.
Small enough to read byte by byte, and it loads in the real engine:

    python3 plugins/blender/make_test_level.py /tmp/arxtest
    build/arx -d /tmp/arxtest --loadlevel 9 --skiplogo

It defaults to level 9 because the game ships nothing at all under that id: no
scene, no scripts, no map and no loading screen. Overwriting a real level instead
would leave the engine running that level's entity scripts against geometry that
has none of the markers they expect.

Nothing here is compressed. The FTS sets uncompressedsize to 0, which makes the
engine read the payload as is, and the DLF declares version 1.43, below the 1.44
cutoff at which the engine expects both the DLF body and the LLF to be imploded.
So a hexdump of the output is the actual data.

Lighting is baked with the same danae_lighting module the Blender addon uses, so
a level that comes out wrong in game narrows the problem to the file writing or
the engine rather than the bake.
"""
import importlib.util
import math
import os
import struct
import sys
import time
import types
from ctypes import sizeof

ADDON = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'arx_addon')
_package = types.ModuleType('arx_addon_standalone')
_package.__path__ = [ADDON]
sys.modules['arx_addon_standalone'] = _package


def _load(name):
    spec = importlib.util.spec_from_file_location(
        'arx_addon_standalone.' + name, os.path.join(ADDON, name + '.py'))
    module = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    return module


_load('dataCommon')
dataFts = _load('dataFts')
dataDlf = _load('dataDlf')
dataLlf = _load('dataLlf')
dataFtl = _load('dataFtl')
danae = _load('danae_lighting')
anchors_module = _load('anchor_generation')

# Arx world axes: x east, z north, y DOWN. The corridor runs east along x and is
# laid out on 100 unit tile boundaries so every wall and floor quad lands in a
# tile of its own.
TILE = 100.0
ORIGIN_X, ORIGIN_Z = 8800.0, 8600.0
CHAMBERS = 4
CHAMBER_TILES = 6                       # 600 units square, one room each
LENGTH_TILES, WIDTH_TILES = CHAMBERS * CHAMBER_TILES, CHAMBER_TILES
FLOOR_Y = 0.0
CEILING_Y = -300.0
# Vertex lighting only has values at corners, so a wall built from one quad per
# tile gets four samples over ten thousand square units and a light next to it
# reads as a flat wash. Splitting the shell finer is what lets the falloff show.
SHELL_STEP = 50.0

WALL_TEXTURE = 'GRAPH\\OBJ3D\\TEXTURES\\L1_DRAGON_[STONE]_GROUND02.BMP'

POLY_DOUBLESIDED = 1 << 1
POLY_QUAD = 1 << 6
# Single sided, like the shipped levels, where only 3.9% of polygons are double
# sided. A double sided polygon has one normal, so it is lit from one side and
# shows that same lighting on the other - which is why a shared wall between two
# rooms has to be two polygons, one per room, rather than one drawn both ways.
SHELL_TYPE = POLY_QUAD

# Walls with doorways split the corridor into rooms, so portal culling has
# something to do. Room 0 is left empty: the engine allocates nb_rooms + 1 and the
# original data never puts geometry in the first slot either, so rooms count from 1.
DIVIDERS = [ORIGIN_X + i * CHAMBER_TILES * TILE for i in range(1, CHAMBERS)]
NB_ROOMS = len(DIVIDERS) + 1

# The player is a cylinder of radius 52, so a doorway has to clear 104 units just
# to admit them, and more than that to walk through without snagging.
DOOR_WIDTH = 200.0
DOOR_HEIGHT = 250.0


def room_at(x):
    room = 1
    for divider in DIVIDERS:
        if x >= divider:
            room += 1
    return room


def door_bounds(centre_z):
    return (centre_z - DOOR_WIDTH * 0.5, centre_z + DOOR_WIDTH * 0.5,
            FLOOR_Y - DOOR_HEIGHT, FLOOR_Y)

# A person is about 180 Arx units tall.
MODEL_HEIGHT = 180.0
# The entity copy is smaller so it reads as something to pick up rather than
# another piece of scenery, and so its physics box is a sane size to throw.
ITEM_SCALE = 0.45


def polygon(vertices, uvs, normal, texture, quad, room=None):
    if room is None:
        room = room_at(sum(v[0] for v in vertices) / len(vertices))
    return {'v': vertices, 'uv': uvs, 'norm': normal, 'tex': texture, 'quad': quad,
            'room': room}


def face_normal(vertices):
    a, b, c = vertices[0], vertices[1], vertices[2]
    u = (b[0] - a[0], b[1] - a[1], b[2] - a[2])
    v = (c[0] - a[0], c[1] - a[1], c[2] - a[2])
    n = (u[1] * v[2] - u[2] * v[1],
         u[2] * v[0] - u[0] * v[2],
         u[0] * v[1] - u[1] * v[0])
    length = math.sqrt(n[0] ** 2 + n[1] ** 2 + n[2] ** 2)
    if length < 1e-9:
        return (0.0, -1.0, 0.0)
    return (n[0] / length, n[1] / length, n[2] / length)


def quad_z_order(a, b, c, d, uvs, texture, desired=None, room=None):
    """A quad in FTS Z order: v0 v1 along one edge, v2 v3 along the opposite one,
    so the perimeter runs 0, 1, 3, 2.

    `desired` is the direction the face should look. Opposite walls of a corridor
    are described by the same sweep of coordinates, so half of them come out wound
    backwards; swapping v1 and v2 reverses the winding while keeping Z order. This
    matters well beyond backface culling - both the baked lightmap and the engine's
    runtime lighting dot against this normal, so a face pointing out of the room
    is simply never lit.
    """
    vertices = [a, b, c, d]
    uvs = list(uvs)
    normal = face_normal([vertices[0], vertices[1], vertices[3]])
    if desired is not None:
        if sum(normal[i] * desired[i] for i in range(3)) < 0.0:
            vertices[1], vertices[2] = vertices[2], vertices[1]
            uvs[1], uvs[2] = uvs[2], uvs[1]
            normal = face_normal([vertices[0], vertices[1], vertices[3]])
    return polygon(vertices, uvs, normal, texture, True, room)


QUAD_UVS = [(0.0, 0.0), (1.0, 0.0), (0.0, 1.0), (1.0, 1.0)]

# Directions a corridor surface should look, in Arx axes where y points down.
UP, DOWN = (0.0, -1.0, 0.0), (0.0, 1.0, 0.0)
NORTH, SOUTH = (0.0, 0.0, 1.0), (0.0, 0.0, -1.0)
EAST, WEST = (1.0, 0.0, 0.0), (-1.0, 0.0, 0.0)


def build_corridor():
    """Floor, ceiling, side walls and two end caps, subdivided into SHELL_STEP quads."""
    polygons = []
    x0, z0 = ORIGIN_X, ORIGIN_Z
    x1 = x0 + LENGTH_TILES * TILE
    z1 = z0 + WIDTH_TILES * TILE
    step = SHELL_STEP

    def span(lo, hi):
        count = int(round((hi - lo) / step))
        return [(lo + i * step, lo + (i + 1) * step) for i in range(count)]

    for xa, xb in span(x0, x1):
        for za, zb in span(z0, z1):
            # Arx y grows downwards, so the floor looks along -y and the ceiling +y.
            polygons.append(quad_z_order((xa, FLOOR_Y, za), (xb, FLOOR_Y, za),
                                         (xa, FLOOR_Y, zb), (xb, FLOOR_Y, zb),
                                         QUAD_UVS, 1, UP))
            polygons.append(quad_z_order((xa, CEILING_Y, za), (xb, CEILING_Y, za),
                                         (xa, CEILING_Y, zb), (xb, CEILING_Y, zb),
                                         QUAD_UVS, 1, DOWN))

    for ya, yb in span(CEILING_Y, FLOOR_Y):
        for xa, xb in span(x0, x1):
            polygons.append(quad_z_order((xa, ya, z0), (xb, ya, z0),
                                         (xa, yb, z0), (xb, yb, z0), QUAD_UVS, 1, NORTH))
            polygons.append(quad_z_order((xa, ya, z1), (xb, ya, z1),
                                         (xa, yb, z1), (xb, yb, z1), QUAD_UVS, 1, SOUTH))
        for za, zb in span(z0, z1):
            polygons.append(quad_z_order((x0, ya, za), (x0, ya, zb),
                                         (x0, yb, za), (x0, yb, zb), QUAD_UVS, 1, EAST))
            polygons.append(quad_z_order((x1, ya, za), (x1, ya, zb),
                                         (x1, yb, za), (x1, yb, zb), QUAD_UVS, 1, WEST))

    # The walls that split the rooms, each with a doorway punched through it. Each
    # one is built twice, once facing into the room on either side, and the two
    # copies belong to their own rooms. That is what the shipped levels do, and it
    # is the only way each side can be lit by its own room's lights: a single
    # double sided quad carries one normal, so the far side would show the near
    # side's lighting, and it would only be drawn when the near room is drawn.
    centre_z = (z0 + z1) * 0.5
    dz0, dz1, dy0, dy1 = door_bounds(centre_z)
    for divider in DIVIDERS:
        near_room, far_room = room_at(divider - 1.0), room_at(divider + 1.0)
        for ya, yb in span(CEILING_Y, FLOOR_Y):
            for za, zb in span(z0, z1):
                if za >= dz0 and zb <= dz1 and ya >= dy0 and yb <= dy1:
                    continue
                corners = ((divider, ya, za), (divider, ya, zb),
                           (divider, yb, za), (divider, yb, zb))
                polygons.append(quad_z_order(*corners, QUAD_UVS, 1, WEST, room=near_room))
                polygons.append(quad_z_order(*corners, QUAD_UVS, 1, EAST, room=far_room))

    return polygons


def make_portals(centre_z):
    return [make_portal(divider, centre_z) for divider in DIVIDERS]


def make_portal(divider, centre_z):
    """The doorway, as a portal quad joining the two rooms.

    The engine takes the plane from the first three corners and then decides which
    way to travel by comparing it against the camera: from room_1 when the camera
    sits on the side the normal points into, from room_2 otherwise. So room_1 has
    to be the room in front of the normal. The bounding radius matters just as
    much - it is read straight out of v[0].rhw and never recomputed, and the
    portal is rejected outright if it is too small.
    """
    dz0, dz1, dy0, dy1 = door_bounds(centre_z)
    near_room, far_room = room_at(divider - 1.0), room_at(divider + 1.0)
    corners = [(divider, dy1, dz0), (divider, dy1, dz1),
               (divider, dy0, dz0), (divider, dy0, dz1)]

    # Aim the normal back into the near room, so room_1 is the near one.
    normal = face_normal([corners[0], corners[1], corners[2]])
    if normal[0] > 0.0:
        corners[1], corners[2] = corners[2], corners[1]
        normal = face_normal([corners[0], corners[1], corners[2]])

    centre = tuple(sum(c[i] for c in corners) / 4.0 for i in range(3))
    radius = max(math.sqrt(sum((c[i] - centre[i]) ** 2 for i in range(3)))
                 for c in corners)

    poly = dataFts.SAVE_EERIEPOLY()
    poly.type = 64  # the engine accepts only 64 or 0 here
    for i, corner in enumerate(corners):
        poly.v[i].pos.x, poly.v[i].pos.y, poly.v[i].pos.z = corner
        poly.v[i].color = 0xFFFFFFFF
        poly.tv[i] = poly.v[i]
        poly.nrml[i].x, poly.nrml[i].y, poly.nrml[i].z = normal
    poly.v[0].rhw = radius
    poly.norm.x, poly.norm.y, poly.norm.z = normal
    poly.norm2.x, poly.norm2.y, poly.norm2.z = normal
    poly.min.x = poly.min.y = poly.min.z = 0.0
    for axis, name in enumerate('xyz'):
        setattr(poly.min, name, min(c[axis] for c in corners))
        setattr(poly.max, name, max(c[axis] for c in corners))
        setattr(poly.center, name, centre[axis])
    poly.tex = -1
    poly.transval = 0.0
    poly.area = 1.0
    poly.room = near_room
    poly.misc = 0

    portal = dataFts.EERIE_SAVE_PORTALS()
    portal.poly = poly
    portal.room_1 = near_room
    portal.room_2 = far_room
    portal.useportal = 1
    portal.paddy = 0

    print(f"  portal at x={divider:.0f}, {DOOR_WIDTH:.0f} x {DOOR_HEIGHT:.0f} opening, "
          f"normal {tuple(round(v, 2) for v in normal)}, radius {radius:.1f}, "
          f"room_1={near_room} room_2={far_room}")
    return portal


def load_obj(path):
    """Read a Wavefront OBJ into triangles grouped by material name.

    Only what a static prop needs: positions, texture coordinates, faces and
    usemtl. Faces with more than three corners are fanned into triangles.
    """
    positions, texcoords = [], []
    faces = []
    material = None

    with open(path, encoding='latin-1') as handle:
        for line in handle:
            parts = line.split()
            if not parts:
                continue
            if parts[0] == 'v':
                positions.append(tuple(float(v) for v in parts[1:4]))
            elif parts[0] == 'vt':
                texcoords.append((float(parts[1]), float(parts[2])))
            elif parts[0] == 'usemtl':
                material = parts[1]
            elif parts[0] == 'f':
                corners = []
                for chunk in parts[1:]:
                    bits = chunk.split('/')
                    vertex = int(bits[0]) - 1
                    uv = int(bits[1]) - 1 if len(bits) > 1 and bits[1] else None
                    corners.append((vertex, uv))
                for k in range(1, len(corners) - 1):
                    faces.append((material, [corners[0], corners[k], corners[k + 1]]))

    return positions, texcoords, faces


def place_model(obj, origin, scale, textures, flip=True):
    """Convert a loaded OBJ into Arx triangles standing at `origin`.

    OBJ is y up and Arx is y down, so the vertical axis is negated. That mirrors
    the coordinate system, which reverses the winding, so the corners are emitted
    in the opposite order to keep the face normals pointing out of the model.
    """
    positions, texcoords, faces = obj
    polygons = []

    for material, corners in faces:
        if flip:
            corners = list(reversed(corners))
        vertices, uvs = [], []
        for vertex, uv in corners:
            px, py, pz = positions[vertex]
            vertices.append((origin[0] + px * scale,
                             origin[1] - py * scale,
                             origin[2] + pz * scale))
            if uv is not None and uv < len(texcoords):
                u, v = texcoords[uv]
                uvs.append((u, 1.0 - v))
            else:
                uvs.append((0.0, 0.0))
        vertices.append(vertices[-1])
        uvs.append(uvs[-1])
        polygons.append(polygon(vertices[:3] + [vertices[2]], uvs[:3] + [uvs[2]],
                                face_normal(vertices), textures[material], False))

    return polygons


def tile_of(polygon):
    centre_x = sum(v[0] for v in polygon['v']) / 4.0
    centre_z = sum(v[2] for v in polygon['v']) / 4.0
    return int(centre_x / 100.0), int(centre_z / 100.0)


def triangle_area(a, b, c):
    u = (b[0] - a[0], b[1] - a[1], b[2] - a[2])
    v = (c[0] - a[0], c[1] - a[1], c[2] - a[2])
    n = (u[1] * v[2] - u[2] * v[1], u[2] * v[0] - u[0] * v[2], u[0] * v[1] - u[1] * v[0])
    return 0.5 * math.sqrt(n[0] ** 2 + n[1] ** 2 + n[2] ** 2)


def polygon_area(poly):
    v = poly['v']
    if poly['quad']:
        # Stored in Z order, so the two triangles are 0,1,3 and 0,3,2.
        return triangle_area(v[0], v[1], v[3]) + triangle_area(v[0], v[3], v[2])
    return triangle_area(v[0], v[1], v[2])


def fast_poly(poly):
    out = dataFts.FAST_EERIEPOLY()
    for i in range(4):
        position = poly['v'][i] if i < len(poly['v']) else poly['v'][-1]
        uv = poly['uv'][i] if i < len(poly['uv']) else poly['uv'][-1]
        out.v[i].ssx, out.v[i].sy, out.v[i].ssz = position
        out.v[i].stu, out.v[i].stv = uv
    normal = poly['norm']
    out.norm.x, out.norm.y, out.norm.z = normal
    out.norm2.x, out.norm2.y, out.norm2.z = normal
    for i in range(4):
        out.nrml[i].x, out.nrml[i].y, out.nrml[i].z = normal
    out.tex = poly['tex']
    out.transval = 0.0
    out.area = polygon_area(poly)
    out.type.asUInt = SHELL_TYPE if poly['quad'] else 0
    out.room = poly['room']
    out.paddy = 0
    return out


def write_fts(path, polygons, textures, portals=(), anchors=()):
    """FAST_SCENE_HEADER, textures, 160x160 cells, anchors, portals, rooms, distances."""
    cells = {}
    for poly in polygons:
        cells.setdefault(tile_of(poly), []).append(poly)

    header = dataFts.FAST_SCENE_HEADER()
    header.version = 0.141
    header.sizex = 160
    header.sizez = 160
    header.nb_textures = len(textures)
    header.nb_polys = len(polygons)
    header.nb_anchors = len(anchors)
    header.nb_portals = len(portals)
    header.nb_rooms = NB_ROOMS
    # Vertices are absolute, so leave the scene offset at zero and let the player
    # position in the DLF be absolute too.
    for field in (header.playerpos, header.Mscenepos):
        field.x = field.y = field.z = 0.0

    payload = bytearray(bytes(header))

    for index, name in textures:
        texture = dataFts.FAST_TEXTURE_CONTAINER()
        texture.tc = index
        texture.temp = 0
        texture.fic = name.encode('latin-1')
        payload += bytes(texture)

    # Which cell each anchor sits in. The engine reads these indices and throws
    # them away (Mesh.cpp only skips over them), but DANAE wrote them and the
    # Blender importer reads them back, so they are worth getting right.
    cell_anchors = {}
    for index, anchor in enumerate(anchors):
        key = (int(anchor['pos'][0] / TILE), int(anchor['pos'][2] / TILE))
        cell_anchors.setdefault(key, []).append(index)

    # Cells in the order the engine reads them: rows along Z, columns along X.
    ordered = []
    for z in range(160):
        for x in range(160):
            here = cells.get((x, z), [])
            mine = cell_anchors.get((x, z), [])
            info = dataFts.FAST_SCENE_INFO()
            info.nbpoly = len(here)
            info.nbianchors = len(mine)
            payload += bytes(info)
            for poly in here:
                payload += bytes(fast_poly(poly))
                ordered.append(poly)
            for index in mine:
                payload += struct.pack('<i', index)

    for anchor in anchors:
        record = dataFts.FAST_ANCHOR_DATA()
        record.pos.x, record.pos.y, record.pos.z = anchor['pos']
        record.radius = anchor['radius']
        record.height = anchor['height']
        record.nb_linked = len(anchor['links'])
        record.flags = anchor['flags']
        payload += bytes(record)
        for linked in anchor['links']:
            payload += struct.pack('<i', linked)

    for portal in portals:
        payload += bytes(portal)

    # EP_DATA addresses a polygon as (cell x, cell z, index within that cell), so
    # the indices have to come from the very walk that just wrote the cells.
    room_polygons = {}
    room_portals = {}
    for index, portal in enumerate(portals):
        room_portals.setdefault(portal.room_1, []).append(index)
        room_portals.setdefault(portal.room_2, []).append(index)

    index_in_cell = {}
    for poly in ordered:
        tile = tile_of(poly)
        idx = index_in_cell.get(tile, 0)
        index_in_cell[tile] = idx + 1
        room_polygons.setdefault(poly['room'], []).append((tile[0], tile[1], idx))

    for room in range(NB_ROOMS + 1):
        entries = room_polygons.get(room, [])
        linked = room_portals.get(room, [])
        data = dataFts.EERIE_SAVE_ROOM_DATA()
        data.nb_portals = len(linked)
        data.nb_polys = len(entries)
        payload += bytes(data)
        # The engine reads these and throws them away, rebuilding the links from
        # the portals themselves, but it still steps over exactly this many.
        for portal_index in linked:
            payload += struct.pack('<i', portal_index)
        for cell_x, cell_z, idx in entries:
            ep = dataFts.FAST_EP_DATA()
            ep.px, ep.py, ep.idx, ep.padd = cell_x, cell_z, idx, 0
            payload += bytes(ep)
        print(f"  room {room}: {len(entries)} polygons, {len(linked)} portals")

    # Written start room major, which is the order the loader reads it back in.
    routes = dataFts.compute_room_distances(portals, NB_ROOMS)
    for start in range(NB_ROOMS + 1):
        for end in range(NB_ROOMS + 1):
            entry = dataFts.ROOM_DIST_DATA_SAVE()
            route = routes.get((start, end))
            if route:
                entry.distance = route[0]
                entry.startpos.x, entry.startpos.y, entry.startpos.z = route[1]
                entry.endpos.x, entry.endpos.y, entry.endpos.z = route[2]
            else:
                entry.distance = 0.0
            payload += bytes(entry)
    for (start, end), route in sorted(routes.items()):
        print(f"  route room {start} -> {end}: distance {route[0]:.1f} "
              f"via ({route[1][0]:.0f},{route[1][2]:.0f}) .. "
              f"({route[2][0]:.0f},{route[2][2]:.0f})")

    container = dataFts.UNIQUE_HEADER()
    container.path = b'Level\\FTS'
    container.count = 1
    container.version = 0.141
    container.uncompressedsize = 0  # 0 means the payload is not imploded
    inner = dataFts.UNIQUE_HEADER3()
    inner.path = b'fast.fts'
    inner.check = b'DANAE_FILE'

    with open(path, 'wb') as handle:
        handle.write(bytes(container))
        handle.write(bytes(inner))
        handle.write(payload)

    return ordered


# Hung below the ceiling rather than against it: a light sitting in the ceiling
# plane lights nothing above it and grazes everything below, which reads as flat.
LIGHT_Y = CEILING_Y + 130.0


def make_light(position, rgb, intensity=2.4, fallstart=30.0, fallend=300.0):
    light = dataLlf.DANAE_LS_LIGHT()
    light.pos.x, light.pos.y, light.pos.z = position
    light.rgb.r, light.rgb.g, light.rgb.b = rgb
    light.fallstart = fallstart
    light.fallend = fallend
    light.intensity = intensity
    light.extras = 0
    return light


def write_llf(path, ordered, lights):
    """Header, lights, then one BGRA colour per polygon vertex in cell order."""
    bake = [danae.DanaeLight(
        pos=(light.pos.x, light.pos.y, light.pos.z),
        rgb=(light.rgb.r, light.rgb.g, light.rgb.b),
        intensity=light.intensity, fallstart=light.fallstart,
        fallend=light.fallend, casts_shadow=True) for light in lights]

    faces = []
    for poly in ordered:
        count = 4 if poly['quad'] else 3
        faces.append((poly['v'][:count], poly['norm'], poly['norm']))

    # Same normal pass the editor ran before every bake.
    normals = danae.prepare_vertex_normals(faces)

    polygons = []
    for (vertices, _n, _n2), vertex_normals in zip(faces, normals):
        count = len(vertices)
        centre = tuple(sum(v[i] for v in vertices) / count for i in range(3))
        polygons.append(danae.DanaePolygon(vertices=vertices, normals=vertex_normals,
                                           center=centre, ignore=False))

    # No shadow rays: casting them needs a bounding volume, which lives on the
    # Blender side. Everything here is lit by distance and orientation only.
    colors = danae.compute_vertex_colors(polygons, bake, visible=None)

    header = dataLlf.DANAE_LLF_HEADER()
    header.version = 1.43
    header.ident = b'DANAE_LLH_FILE'
    header.lastuser = b'make_test_level'
    header.time = int(time.time())
    header.nb_lights = len(lights)
    header.nb_bkgpolys = len(ordered)
    header.nb_Shadow_Polys = 0
    header.nb_IGNORED_Polys = 0

    lighting = dataLlf.DANAE_LS_LIGHTINGHEADER()
    lighting.nb_values = len(colors)
    lighting.ViewMode = 0
    lighting.ModeLight = 63

    payload = bytearray(bytes(header))
    for light in lights:
        payload += bytes(light)
    payload += bytes(lighting)
    for r, g, b, a in colors:
        colour = dataLlf.SavedColorBGRA()
        colour.b, colour.g, colour.r, colour.a = b, g, r, a
        payload += bytes(colour)

    with open(path, 'wb') as handle:
        handle.write(payload)

    return colors


# src/ai/Paths.h
PATH_AMBIANCE = 1 << 1
PATH_RGB = 1 << 2
PATH_FARCLIP = 1 << 3

# One flavour per chamber, so it is obvious which zone you are standing in.
CHAMBER_AMBIANCE = ['ambient_cave_frozen', 'ambient_crypt_a',
                    'ambient_cave_greu', 'ambient_crypt_d']
CHAMBER_TINT = [(0.35, 0.55, 1.0), (1.0, 0.45, 0.25),
                (0.45, 1.0, 0.55), (0.85, 0.5, 1.0)]

MARKER_CLASS = 'GRAPH\\OBJ3D\\INTERACTIVE\\SYSTEM\\MARKER\\MARKER.TEO'


def chamber_bounds(index):
    """The x range of one chamber, and the centre of its footprint."""
    x0 = ORIGIN_X + index * CHAMBER_TILES * TILE
    x1 = x0 + CHAMBER_TILES * TILE
    z0, z1 = ORIGIN_Z, ORIGIN_Z + WIDTH_TILES * TILE
    return x0, x1, ((x0 + x1) * 0.5, FLOOR_Y, (z0 + z1) * 0.5)


def make_zone(name, index, inset=60.0):
    """A zone covering one chamber's footprint.

    The engine treats a zone as a polygon in XZ that it point-in-polygon tests,
    with height bounding it vertically: above zero spans pos.y - height to pos.y,
    zero or less leaves it unbounded (ARX_PATH_ComputeBB). The pathway positions
    are relative to the zone's own position.
    """
    x0, x1, centre = chamber_bounds(index)
    z0, z1 = ORIGIN_Z + inset, ORIGIN_Z + WIDTH_TILES * TILE - inset
    x0, x1 = x0 + inset, x1 - inset

    zone = dataDlf.DANAE_LS_PATH()
    encoded = name.encode('iso-8859-1')[:63]
    zone.name = encoded + b'\x00' * (64 - len(encoded))
    zone.idx = index
    zone.flags = PATH_AMBIANCE | PATH_RGB
    zone.pos.x, zone.pos.y, zone.pos.z = centre
    zone.initpos.x, zone.initpos.y, zone.initpos.z = centre
    # Positive height, measured upwards from the zone position.
    zone.height = 280
    ambiance = CHAMBER_AMBIANCE[index % len(CHAMBER_AMBIANCE)].encode('iso-8859-1')
    zone.ambiance = ambiance + b'\x00' * (128 - len(ambiance))
    zone.amb_max_vol = 100.0
    zone.rgb.r, zone.rgb.g, zone.rgb.b = CHAMBER_TINT[index % len(CHAMBER_TINT)]
    zone.farclip = 0.0
    zone.reverb = 0.0

    corners = [(x0, z0), (x1, z0), (x1, z1), (x0, z1)]
    pathways = []
    for cx, cz in corners:
        way = dataDlf.DANAE_LS_PATHWAYS()
        way.rpos.x = cx - centre[0]
        way.rpos.y = 0.0
        way.rpos.z = cz - centre[2]
        way.flag = 0
        way.time = 0
        pathways.append(way)
    zone.nb_pathways = len(pathways)
    return zone, pathways


def make_patrol_path(name='CJ_PATROL'):
    """A path, which is the same record with height left at zero."""
    _x0, _x1, first = chamber_bounds(0)
    path = dataDlf.DANAE_LS_PATH()
    encoded = name.encode('iso-8859-1')[:63]
    path.name = encoded + b'\x00' * (64 - len(encoded))
    path.idx = 100
    path.flags = 0
    path.height = 0  # zero is what makes it a path rather than a zone
    path.pos.x, path.pos.y, path.pos.z = first
    path.initpos.x, path.initpos.y, path.initpos.z = first
    path.ambiance = b'\x00' * 128

    pathways = []
    for index in range(CHAMBERS):
        _a, _b, centre = chamber_bounds(index)
        way = dataDlf.DANAE_LS_PATHWAYS()
        way.rpos.x = centre[0] - first[0]
        way.rpos.y = -60.0
        way.rpos.z = centre[2] - first[2]
        way.flag = 0
        way.time = 2000
        pathways.append(way)
    path.nb_pathways = len(pathways)
    return path, pathways


def write_ftl(path, obj, scale, textures, name):
    """Turn a loaded OBJ into an Arx entity mesh.

    Entity meshes live in object space with the same axes as the world, so the
    vertical axis is negated and the winding reversed exactly as for baked
    geometry. The model is shifted so that the centre of its base sits on the
    origin, and an unreferenced vertex is placed there for the header to point
    at, because the engine expects the origin vertex to be at (0, 0, 0) and warns
    loudly when it is not.

    The header carries the FTL magic, so the file needs no compression: the
    loader only reaches for blast when those three bytes are missing.
    """
    positions, texcoords, faces = obj

    lo = [min(p[i] for p in positions) for i in range(3)]
    hi = [max(p[i] for p in positions) for i in range(3)]
    centre_x = (lo[0] + hi[0]) * 0.5
    centre_z = (lo[2] + hi[2]) * 0.5
    base_y = lo[1]

    def convert(p):
        return ((p[0] - centre_x) * scale,
                -(p[1] - base_y) * scale,
                (p[2] - centre_z) * scale)

    verts = [convert(p) for p in positions]

    materials = sorted({material for material, _ in faces})
    material_index = {name: i for i, name in enumerate(materials)}

    ftl_faces = []
    normals = [[0.0, 0.0, 0.0] for _ in verts]
    for material, corners in faces:
        corners = list(reversed(corners))
        vids = [c[0] for c in corners]
        uvs = []
        for _vertex, uv in corners:
            if uv is not None and uv < len(texcoords):
                u, v = texcoords[uv]
                uvs.append((u, 1.0 - v))
            else:
                uvs.append((0.0, 0.0))
        normal = face_normal([verts[i] for i in vids])
        for i in vids:
            for axis in range(3):
                normals[i][axis] += normal[axis]
        ftl_faces.append(dataFtl.FtlFace(vids=vids, uvs=uvs,
                                         texid=material_index[material],
                                         facetype=1, transval=0.0, normal=normal))

    def normalised(n):
        length = math.sqrt(n[0] ** 2 + n[1] ** 2 + n[2] ** 2)
        return (0.0, -1.0, 0.0) if length < 1e-9 else (n[0] / length, n[1] / length,
                                                       n[2] / length)

    ftl_verts = [dataFtl.FtlVertex(xyz=v, n=normalised(n)) for v, n in zip(verts, normals)]

    origin = len(ftl_verts)
    ftl_verts.append(dataFtl.FtlVertex(xyz=(0.0, 0.0, 0.0), n=(0.0, -1.0, 0.0)))

    group = dataFtl.FtlGroup(name='all', origin=origin,
                             indices=list(range(len(ftl_verts))), parentIndex=-1)
    # Lets the entity be held; without it the engine has nowhere to attach a hand.
    action = dataFtl.FtlAction(name='HAND_ATTACH', vidx=origin)

    data = dataFtl.FtlData(
        metadata=dataFtl.FtlMetadata(name=name, org=origin),
        verts=ftl_verts,
        faces=ftl_faces,
        mats=[textures[m] for m in materials],
        groups=[group],
        actions=[action],
        sels=[],
    )

    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, 'wb') as handle:
        handle.write(bytes(dataFtl.FtlSerializer().write(data)))

    height = (hi[1] - lo[1]) * scale
    print(f"  wrote {os.path.basename(path)}: {len(ftl_verts)} vertices, "
          f"{len(ftl_faces)} faces, {len(materials)} materials, {height:.0f} units tall")


CJ_ITEM_SCRIPT = """ON INIT {
 SETNAME "CJ"
 SET_MATERIAL FLESH
 SET_GROUP PROVISIONS
 SET_DURABILITY 20
 SET_PRICE 5
 SET_WEIGHT 10
 SET_STEAL 50
 ACCEPT
}
"""

# One marker controls every zone in the level and switches on which one was
# entered, which is why the class script can be shared. CONTROLLEDZONE_ENTER
# hands it the entering entity in PARAM1 and the zone name in PARAM2
# (ARX_PATH_ComputeAllZoneEvents in ai/Paths.cpp).
#
# Both arrive lowercased - the level loader runs zone names and entity ids through
# toLowercase - and the == operator compares text exactly (ScriptedLang.cpp), so
# the literals below have to be lowercase or no branch ever fires.
CJ_NPC_SCRIPT = """// CJ as a walking NPC, the counterpart to the item.
//
// The two are different entities and neither replaces the other: the item is a
// pickup with an inventory icon and a physics box, this one has _npcdata,
// animation layers and a collision cylinder. AddInteractive picks between them
// on the class path alone and tests "items" before "npc", so anything with
// "items" anywhere in its path is an item whatever else the path says.
//
// Every animation below is the stock human set. The mesh's groups are a copy of
// human_base's, group for group and in the same order, and TEA addresses bones
// by index, so no new .tea file is needed to make this walk.

ON INIT {
 SETGORE OFF
 SETNAME "CJ"
 PHYSICAL RADIUS 30
 SET_MATERIAL FLESH
 SET_ARMOR_MATERIAL LEATHER
 SET_STEP_MATERIAL Foot_shoe
 SETDETECT 40
 SETSTAREFACTOR 0.4
 SET_NPC_STAT life 32
 SET_NPC_STAT armor_class 8
 SET_NPC_STAT absorb 10
 SET_NPC_STAT damages 5
 SET_NPC_STAT tohit 30
 SET_NPC_STAT aimtime 1000
 SET_NPC_STAT reach 20
 SET_NPC_STAT resistmagic 10
 SET_NPC_STAT resistpoison 10
 SET_NPC_STAT resistfire 10
 SET_XP_VALUE 40
 SETIRCOLOR 0.8 0.0 0.0
// Neutral until hit. No target and no fight behaviour is what neutral *is* -
// there is no separate flag for it.
 SET §angry 0
 ACCEPT
}

ON INITEND {
// Copied from human_base's INITEND. BARE_READY is spelled BAE_READY there, a
// typo the engine carries a suppression for; there is no reason to inherit it.
 LOADANIM WALK_SNEAK                 "human_walk_sneak"    //*****************  NEW
 LOADANIM CAST_START                 "human_npc_cast_start"
 LOADANIM CAST_CYCLE                 "human_npc_cast_cycle"
 LOADANIM CAST                       "human_npc_cast_cast"
 LOADANIM CAST_END                   "human_npc_cast_end"
//LOADANIM CAST_HOLD                 "human_npc_cast_hold"
 LOADANIM WALK                       "human_normal_walk_guard"
 LOADANIM RUN                        "Human_normal_run"
 LOADANIM WAIT                       "Human_normal_wait"
 LOADANIM HIT                        "Human_fight_receive_damage"
 LOADANIM HIT_SHORT                  "human_hit_short"
 LOADANIM DIE                        "Human_death"
 LOADANIM TALK_NEUTRAL               "human_talk_neutral_headonly"
 LOADANIM TALK_ANGRY                 "human_talk_angry_headonly"
 LOADANIM TALK_HAPPY                 "human_talk_happy_headonly"
 LOADANIM GRUNT                      "human_fight_grunt"
 loadanim FIGHT_WAIT                 "human_fight_wait"
 loadanim FIGHT_WALK_FORWARD         "human_fight_walk"
 loadanim FIGHT_WALK_BACKWARD        "human_fight_walk_backward"
 loadanim FIGHT_STRAFE_RIGHT         "human_fight_strafe_right"
 loadanim FIGHT_STRAFE_LEFT          "human_fight_strafe_left"
 loadanim BARE_READY                 "human_fight_ready_noweap"
 loadanim BARE_UNREADY               "human_fight_unready_noweap"
 loadanim BARE_WAIT                  "human_fight_wait_noweap"
 loadanim BARE_STRIKE_LEFT_START     "human_fight_attack_noweap_left_start"
 loadanim BARE_STRIKE_LEFT_CYCLE     "human_fight_attack_noweap_left_cycle"
 loadanim BARE_STRIKE_LEFT           "human_fight_attack_noweap_left_strike"
 loadanim BARE_STRIKE_RIGHT_START    "human_fight_attack_noweap_right_start"
 loadanim BARE_STRIKE_RIGHT_CYCLE    "human_fight_attack_noweap_right_cycle"
 loadanim BARE_STRIKE_RIGHT          "human_fight_attack_noweap_right_strike"
 loadanim BARE_STRIKE_TOP_START      "human_fight_attack_noweap_top_start"
 loadanim BARE_STRIKE_TOP_CYCLE      "human_fight_attack_noweap_top_cycle"
 loadanim BARE_STRIKE_TOP            "human_fight_attack_noweap_top_strike"
 loadanim BARE_STRIKE_BOTTOM_START   "human_fight_attack_noweap_bottom_start"
 loadanim BARE_STRIKE_BOTTOM_CYCLE   "human_fight_attack_noweap_bottom_cycle"
 loadanim BARE_STRIKE_BOTTOM         "human_fight_attack_noweap_bottom_strike"
 loadanim 1H_WAIT                    "human_fight_wait_1handed"
 loadanim 1H_READY_PART_1            "human_fight_ready_1handed_start"
 loadanim 1H_READY_PART_2            "human_fight_ready_1handed_end"
 loadanim 1H_UNREADY_PART_1          "human_fight_unready_1handed_start"
 loadanim 1H_UNREADY_PART_2          "human_fight_unready_1handed_end"
 loadanim 1H_STRIKE_LEFT_START       "human_fight_attack_1handed_left_start"
 loadanim 1H_STRIKE_LEFT_CYCLE       "human_fight_attack_1handed_left_cycle"
 loadanim 1H_STRIKE_LEFT             "human_fight_attack_1handed_left_strike"
 loadanim 1H_STRIKE_RIGHT_START      "human_fight_attack_1handed_right_start"
 loadanim 1H_STRIKE_RIGHT_CYCLE      "human_fight_attack_1handed_right_cycle"
 loadanim 1H_STRIKE_RIGHT            "human_fight_attack_1handed_right_strike"
 loadanim 1H_STRIKE_TOP_START        "human_fight_attack_1handed_top_start"
 loadanim 1H_STRIKE_TOP_CYCLE        "human_fight_attack_1handed_top_cycle"
 loadanim 1H_STRIKE_TOP              "human_fight_attack_1handed_top_strike"
 loadanim 1H_STRIKE_BOTTOM_START     "human_fight_attack_1handed_bottom_start"
 loadanim 1H_STRIKE_BOTTOM_CYCLE     "human_fight_attack_1handed_bottom_cycle"
 loadanim 1H_STRIKE_BOTTOM           "human_fight_attack_1handed_bottom_strike"
 loadanim 2H_READY_PART_1            "human_fight_ready_2handed_start"
 loadanim 2H_READY_PART_2            "human_fight_ready_2handed_end"
 loadanim 2H_UNREADY_PART_1          "human_fight_unready_2handed_start"
 loadanim 2H_UNREADY_PART_2          "human_fight_unready_2handed_end"
 loadanim 2H_WAIT                    "human_fight_wait_2handed"
 loadanim 2H_STRIKE_LEFT_START       "human_fight_attack_2handed_left_start"
 loadanim 2H_STRIKE_LEFT_CYCLE       "human_fight_attack_2handed_left_cycle"
 loadanim 2H_STRIKE_LEFT             "human_fight_attack_2handed_left_strike"
 loadanim 2H_STRIKE_RIGHT_START      "human_fight_attack_2handed_right_start"
 loadanim 2H_STRIKE_RIGHT_CYCLE      "human_fight_attack_2handed_right_cycle"
 loadanim 2H_STRIKE_RIGHT            "human_fight_attack_2handed_right_strike"
 loadanim 2H_STRIKE_TOP_START        "human_fight_attack_2handed_top_start"
 loadanim 2H_STRIKE_TOP_CYCLE        "human_fight_attack_2handed_top_cycle"
 loadanim 2H_STRIKE_TOP              "human_fight_attack_2handed_top_strike"
 loadanim 2H_STRIKE_BOTTOM_START     "human_fight_attack_2handed_bottom_start"
 loadanim 2H_STRIKE_BOTTOM_CYCLE     "human_fight_attack_2handed_bottom_cycle"
 loadanim 2H_STRIKE_BOTTOM           "human_fight_attack_2handed_bottom_strike"
 loadanim DAGGER_READY_PART_1        "human_fight_ready_dagger_start"
 loadanim DAGGER_READY_PART_2        "human_fight_ready_dagger_end"
 loadanim DAGGER_UNREADY_PART_1      "human_fight_unready_dagger_start"
 loadanim DAGGER_UNREADY_PART_2      "human_fight_unready_dagger_end"
 loadanim DAGGER_WAIT                "human_fight_attack_dagger_wait"
 loadanim DAGGER_STRIKE_LEFT_START   "human_fight_attack_dagger_left_start"
 loadanim DAGGER_STRIKE_LEFT_CYCLE   "human_fight_attack_dagger_left_cycle"
 loadanim DAGGER_STRIKE_LEFT         "human_fight_attack_dagger_left_strike"
 loadanim DAGGER_STRIKE_RIGHT_START  "human_fight_attack_dagger_right_start"
 loadanim DAGGER_STRIKE_RIGHT_CYCLE  "human_fight_attack_dagger_right_cycle"
 loadanim DAGGER_STRIKE_RIGHT        "human_fight_attack_dagger_right_strike"
 loadanim DAGGER_STRIKE_TOP_START    "human_fight_attack_dagger_top_start"
 loadanim DAGGER_STRIKE_TOP_CYCLE    "human_fight_attack_dagger_top_cycle"
 loadanim DAGGER_STRIKE_TOP          "human_fight_attack_dagger_top_strike"
 loadanim DAGGER_STRIKE_BOTTOM_START "human_fight_attack_dagger_bottom_start"
 loadanim DAGGER_STRIKE_BOTTOM_CYCLE "human_fight_attack_dagger_bottom_cycle"
 loadanim DAGGER_STRIKE_BOTTOM       "human_fight_attack_dagger_bottom_strike"
 loadanim MISSILE_READY_PART_1       "human_fight_ready_bow_start"
 loadanim MISSILE_READY_PART_2       "human_fight_ready_bow_end"
 loadanim MISSILE_UNREADY_PART_1     "human_fight_unready_bow_start"
 loadanim MISSILE_UNREADY_PART_2     "human_fight_unready_bow_end"
 loadanim MISSILE_WAIT               "human_fight_wait_bow"
 loadanim MISSILE_STRIKE_PART_1      "human_fight_attack_bow_start_part1"
 loadanim MISSILE_STRIKE_PART_2      "human_fight_attack_bow_start_part2"
 loadanim MISSILE_STRIKE_CYCLE       "human_fight_attack_bow_cycle"
 loadanim MISSILE_STRIKE             "human_fight_attack_bow_strike"
 loadanim ACTION1                    "human_misc_kick_rat"
// A weapon, which is the point of the attachment slots: Prepare_SetWeapon loads
// graph/obj3d/interactive/items/weapons/long_sword/long_sword as a second entity
// and hangs it off this one's weapon_attach, and drawing it moves it to
// primary_attach. Both actions came out of cj.blend with the rest, so if either
// is missing or ungrouped it shows here.
//
// Set in INITEND rather than INIT because Prepare_SetWeapon asserts on io->obj,
// and the mesh is not attached yet when INIT runs during level load.
 SETWEAPON LONG_SWORD
 ACCEPT
}

ON MAIN {
// Once angry the fight behaviour owns this NPC. MAIN keeps firing, so without
// this guard the next tick would wander him away mid swing.
 IF (§angry == 1) ACCEPT
 BEHAVIOR WANDER_AROUND 300
 ACCEPT
}

// Talking. The variant is picked by the engine, not here: with no subtitle on
// screen ARX_SPEECH_AddSpeech counts the strings under [cj_talk] in the
// localisation and appends a number to the sample name, avoiding whatever it
// said last. One key, six lines, no bookkeeping.
ON CHAT {
 IF (§angry == 1) GOTO ANGRY_REPLY
 SPEAK [cj_talk] NOP
 ACCEPT
>>ANGRY_REPLY
 SPEAK -a [cj_fight] NOP
 ACCEPT
}

// Neutral means neutral until provoked. AGGRESSION arrives on a swing, HIT and
// OUCH on damage that lands. human_base and goblin_base funnel all three into
// their fight code and so does this.
ON AGGRESSION {
 GOSUB ENRAGE
 ACCEPT
}

ON HIT {
 GOSUB ENRAGE
 ACCEPT
}

ON OUCH {
 IF (^PLAYERCASTING == 0) FORCEANIM HIT_SHORT
 GOSUB ENRAGE
 ACCEPT
}

ON DIE {
 SET §angry 0
 ACCEPT
}

// Four commands, in this order, copied from what human_base and goblin_base
// actually do rather than from what looks reasonable.
//
// WEAPON ON is the one that is easy to leave out and fatal to leave out. It
// sets weaponinhand to -1, which is what makes ARX_NPC_Manage_Anims play the
// 1H_READY animations and call SetWeapon_On to move the sword from the back to
// the hand. Without it weaponinhand stays 0, and the melee branch of that
// function only handles -1, 1 and 2 - so an armed NPC that never drew has no
// attack animation at all. It closes on the player and then just stands there,
// which reads as broken AI rather than an undrawn weapon.
//
// SETTARGET needs -a for PATHFIND_ALWAYS. Without it the path to the player is
// computed once and he simply walks out of it.
//
// BEHAVIOR sets movemode to WALK itself, so SETMOVEMODE has to come after it.
>>ENRAGE
 IF (§angry == 1) RETURN
 SET §angry 1
 SPEAK -a [cj_fight] NOP
 BEHAVIOR -f MOVE_TO
 SETTARGET -a PLAYER
 WEAPON ON
 SETMOVEMODE RUN
 RETURN
"""

TRIGGER_SCRIPT = """ON INIT {
 SETCONTROLLEDZONE CHAMBER_1
 SETCONTROLLEDZONE CHAMBER_2
 SETCONTROLLEDZONE CHAMBER_3
 SETCONTROLLEDZONE CHAMBER_4
 ACCEPT
}

ON CONTROLLEDZONE_ENTER {
 IF (^$PARAM1 != "player") {
  ACCEPT
 }

 IF (^$PARAM2 == "chamber_1") {
  HEROSAY "chamber one: the intro camera takes over"
  CINEMASCOPE ON
  CAMERAACTIVATE intro_0001
  SENDEVENT START intro_0001 ""
 }

 IF (^$PARAM2 == "chamber_2") {
  HEROSAY "chamber two: the ground shakes"
  QUAKE 40 3000 300
 }

 IF (^$PARAM2 == "chamber_3") {
  HEROSAY "chamber three: the world fades red"
  WORLDFADE OUT 500 255 0 0
  TIMERfadein -m 1 600 WORLDFADE IN 800
 }

 IF (^$PARAM2 == "chamber_4") {
  HEROSAY "chamber four: the last room"
 }

 ACCEPT
}

ON CONTROLLEDZONE_LEAVE {
 IF (^$PARAM2 == "chamber_1") {
  CAMERAACTIVATE none
  CINEMASCOPE OFF
 }
 ACCEPT
}
"""

# The camera walks the patrol path when the first zone tells it to, then hands
# control back. setpath is what attaches an entity to a named path.
CAMERA_SCRIPT = """ON INIT {
 CAMERAFOCAL 60
 CAMERASMOOTHING 300
 ACCEPT
}

ON START {
 SETPATH CJ_PATROL
 TIMERdone -m 1 12000 SENDEVENT DONE SELF ""
 ACCEPT
}

ON DONE {
 SETPATH NONE
 CAMERAACTIVATE none
 CINEMASCOPE OFF
 ACCEPT
}
"""


def write_dlf(path, level, player_start, entities=(), routes=()):
    """Version 1.43 keeps the body uncompressed and the .llf uncompressed too.

    `entities` is a sequence of (class path, position, ident). The engine strips
    everything before "graph" from that name, lowercases it and drops the
    extension to get the class path, then dispatches on whether the result
    contains items, npc, fix, camera or marker.
    """
    header = dataDlf.DANAE_LS_HEADER()
    header.version = 1.43
    header.ident = b'DANAE_FILE'
    header.lastuser = b'make_test_level'
    header.time = int(time.time())
    header.pos_edit.x, header.pos_edit.y, header.pos_edit.z = player_start
    header.angle_edit.a = header.angle_edit.b = header.angle_edit.g = 0.0
    header.nb_scn = 1
    header.nb_inter = len(entities)
    header.nb_nodes = 0
    header.nb_nodeslinks = 0
    header.nb_zones = 0
    header.lighting = 0  # lighting lives in the .llf
    header.nb_lights = 0
    header.nb_fogs = 0
    header.nb_bkgpolys = 0
    header.nb_ignoredpolys = 0
    header.nb_childpolys = 0
    # Zones and paths are the same record; a non zero height is what makes one a
    # zone. The engine reads them from a single list after the fogs and nodes.
    header.nb_paths = len(routes)
    header.nb_zones = sum(1 for route, _ways in routes if route.height != 0)

    scene = dataDlf.DANAE_LS_SCENE()
    scene.name = f'GRAPH\\LEVELS\\LEVEL{level}'.encode()

    with open(path, 'wb') as handle:
        handle.write(bytes(header))
        handle.write(bytes(scene))
        for class_path, position, ident in entities:
            inter = dataDlf.DANAE_LS_INTER()
            inter.name = class_path.encode('latin-1')
            inter.pos.x, inter.pos.y, inter.pos.z = position
            inter.angle.a = inter.angle.b = inter.angle.g = 0.0
            inter.ident = ident
            inter.flags = 0
            handle.write(bytes(inter))

        # No fogs and no nodes, so the paths follow straight on.
        for route, pathways in routes:
            handle.write(bytes(route))
            for way in pathways:
                handle.write(bytes(way))


MODEL_TEXTURE_DIR = os.path.join('graph', 'obj3d', 'textures')


def install_model_textures(root, source_dir, materials, prefix):
    """Copy a model's bitmaps into the level texture tree and name them for the FTS.

    The engine resolves a polygon's texture through the path stored in the scene,
    under the shared graph/obj3d/textures tree, so a model's own bitmaps have to
    be published there. The prefix keeps them from colliding with the game's.
    """
    import shutil

    target = os.path.join(root, MODEL_TEXTURE_DIR)
    os.makedirs(target, exist_ok=True)

    textures = {}
    for index, (material, bitmap) in enumerate(materials.items(), start=2):
        name = f'{prefix}_{material}.bmp'
        source = os.path.join(source_dir, bitmap)
        if os.path.exists(source):
            shutil.copyfile(source, os.path.join(target, name))
        else:
            print(f"  WARNING: {source} is missing, polygons using it will be untextured")
        textures[material] = (index, 'GRAPH\\OBJ3D\\TEXTURES\\' + name.upper())

    return textures


# CJ's voice lines, in the order they become sample variants. The first entry of
# each list is the unnumbered sample: [cj_talk] plays cj_talk.wav, cj_talk2.wav
# and so on. Text is the subtitle, and it is also what makes the random pick
# work - see install_voice_lines.
CJ_VOICE_TALK = [
    ('ain-t-this-fun', "Ain't this fun?"),
    ('what-you-smoking', "What you smoking?"),
    ('like-i-give-a-fuck-what-you-think', "Like I give a fuck what you think."),
    ('that-s-complete-and-utter-bullshit', "That's complete and utter bullshit."),
    ('just-cause-i-look-fine-i-ain-t-a-bitch', "Just cause I look fine, I ain't a bitch."),
    ('go-do-your-community-work-somewhere-else', "Go do your community work somewhere else."),
]
CJ_VOICE_FIGHT = [
    ('come-on-then-asshole-fight', "Come on then, asshole. Fight!"),
    ('you-wanna-get-slapped-fool', "You wanna get slapped, fool?"),
    ('fuck-you', "Fuck you!"),
    ('asshole', "Asshole!"),
]
CJ_VOICE_KEYS = (('cj_talk', CJ_VOICE_TALK), ('cj_fight', CJ_VOICE_FIGHT))
#: The shipped speech samples are 22050 Hz mono, and WAV.cpp reads plain PCM as
#: happily as the MS ADPCM the originals use.
VOICE_RATE = 22050


def install_voice_lines(root, source_dir):
    """Convert CJ's mp3s into speech samples, and the localisation that indexes them.

    The random line is not something the script does. ARX_SPEECH_AddSpeech picks
    the variant itself: with no subtitle on screen - which is every line outside a
    cinematic - it asks getLocalisedKeyCount how many strings the key has and
    appends the number to the sample name, never repeating the last one. So
    [cj_talk] backed by six strings plays one of cj_talk.wav, cj_talk2.wav ..
    cj_talk6.wav. Ship the wavs without the localisation entries and the count is
    1, and CJ says the same line forever.

    Samples go in speech/<language>/, which is chosen by the audio language rather
    than the text one, so every language present gets a copy. The entries go in
    localisation/xtext_default_002_cj.ini: loadLocalisations reads every
    xtext_default_* file whatever the language is set to, which keeps this out of
    the shipped utext_*.ini.
    """
    import shutil
    import subprocess

    if not shutil.which('ffmpeg'):
        print("  WARNING: no ffmpeg, skipping voice lines. CJ will be mute and "
              "every SPEAK will log a missing sample.")
        return 0

    speech_root = os.path.join(root, 'speech')
    languages = [name for name in sorted(os.listdir(speech_root))
                 if os.path.isdir(os.path.join(speech_root, name))] \
        if os.path.isdir(speech_root) else []
    if not languages:
        languages = ['english']

    written = 0
    for key, lines in CJ_VOICE_KEYS:
        for index, (source_name, _text) in enumerate(lines):
            source = os.path.join(source_dir, source_name + '.mp3')
            if not os.path.exists(source):
                print(f"  WARNING: missing voice line {source_name}.mp3")
                continue
            sample = key if index == 0 else f'{key}{index + 1}'
            for language in languages:
                target = os.path.join(speech_root, language, sample + '.wav')
                os.makedirs(os.path.dirname(target), exist_ok=True)
                # -vn and -map_metadata drop the cover art and the tags: the
                # engine's WAV reader skips unknown chunks, but there is no
                # reason to ship a LIST/INFO block in a sound effect.
                subprocess.run(['ffmpeg', '-y', '-loglevel', 'error', '-i', source,
                                '-vn', '-map_metadata', '-1', '-ac', '1',
                                '-ar', str(VOICE_RATE), '-acodec', 'pcm_s16le',
                                target], check=True)
                written += 1

    ini = os.path.join(root, 'localisation', 'xtext_default_002_cj.ini')
    os.makedirs(os.path.dirname(ini), exist_ok=True)
    with open(ini, 'w', encoding='utf-8') as handle:
        handle.write("// CJ's voice lines. The number of strings in a section is\n"
                     "// what tells the engine how many sample variants exist.\n")
        for key, lines in CJ_VOICE_KEYS:
            handle.write(f'\n[{key}]\n')
            for index, (_source, text) in enumerate(lines):
                name = 'string' if index == 0 else f'string{index + 1}'
                handle.write(f'{name}="{text}"\n')

    print(f"  {written} voice samples across {len(languages)} language(s), "
          f"{sum(len(lines) for _key, lines in CJ_VOICE_KEYS)} lines")
    return written


CJ_MATERIALS = {'head': 'head.bmp', 'torso': 'torso.bmp',
                'legs': 'jeans.bmp', 'shoes': 'sneaker.bmp'}


def main():
    root = sys.argv[1] if len(sys.argv) > 1 else '/tmp/arxtest'
    level = int(sys.argv[2]) if len(sys.argv) > 2 else 9
    model_dir = sys.argv[3] if len(sys.argv) > 3 else None

    fts_dir = os.path.join(root, 'game', 'graph', 'levels', f'level{level}')
    dlf_dir = os.path.join(root, 'graph', 'levels', f'level{level}')
    os.makedirs(fts_dir, exist_ok=True)
    os.makedirs(dlf_dir, exist_ok=True)

    textures = [(1, WALL_TEXTURE)]
    polygons = build_corridor()
    shell_count = len(polygons)
    placements = []

    if model_dir:
        obj_path = os.path.join(model_dir, 'cj.obj')
        bitmap_dir = os.path.join(model_dir, 'BMP Textures')
        published = install_model_textures(root, bitmap_dir, CJ_MATERIALS, 'cj')
        install_voice_lines(root, model_dir)
        textures.extend(sorted(published.values()))
        lookup = {material: index for material, (index, _name) in published.items()}

        model_textures = {m: name for m, (_i, name) in published.items()}
        obj = load_obj(obj_path)
        height = max(p[1] for p in obj[0]) - min(p[1] for p in obj[0])
        scale = MODEL_HEIGHT / height
        print(f"  model is {height:.1f} units tall, scaling by {scale:.3f} "
              f"to {MODEL_HEIGHT:.0f}")

        centre_z = ORIGIN_Z + WIDTH_TILES * TILE / 2.0
        # A pair of baked CJs in every chamber.
        for chamber in range(CHAMBERS):
            _x0, _x1, chamber_centre = chamber_bounds(chamber)
            placements.append((chamber_centre[0] - 150.0, chamber_centre[2] - 180.0))
            placements.append((chamber_centre[0] + 150.0, chamber_centre[2] + 180.0))
        for x, z in placements:
            polygons.extend(place_model(obj, (x, FLOOR_Y, z), scale, lookup))

    # Lights down the middle of the corridor, near the ceiling, in different
    # colours so a texturing problem cannot be mistaken for a lighting one.
    centre_z = ORIGIN_Z + WIDTH_TILES * TILE / 2.0
    palette = [(1.0, 0.95, 0.85), (0.6, 0.75, 1.0), (1.0, 0.7, 0.5), (0.75, 1.0, 0.8)]
    # The first room is lit, the middle one is left on the ambient floor, and the
    # last gets a single light in a colour used nowhere else. Three rooms that look
    # obviously different is what makes a culling mistake visible rather than
    # subtle: if the far room is being drawn when it should not be, its colour
    # shows up through the doorway.
    # Two lights per chamber, in that chamber's own colour, so the lighting and the
    # zone tint agree and you can tell at a glance which room you are standing in.
    lights = []
    for chamber in range(CHAMBERS):
        x0, x1, chamber_centre = chamber_bounds(chamber)
        tint = CHAMBER_TINT[chamber % len(CHAMBER_TINT)]
        for offset in (-160.0, 160.0):
            lights.append(make_light((chamber_centre[0] + offset, LIGHT_Y,
                                      chamber_centre[2]), tint))

    player_start = (ORIGIN_X + 60.0, FLOOR_Y - 180.0, centre_z)

    # Entities. Everything the engine needs is derived from the class path in the
    # DLF, so both of these can live inside the level's own folders - the only
    # rule is that the path contains one of the dispatch keywords.
    entities = []
    routes = []
    if model_dir:
        item_class = f'graph/levels/level{level}/items/cj/cj'
        write_ftl(os.path.join(root, 'game', item_class + '.ftl'),
                  obj, MODEL_HEIGHT * ITEM_SCALE / height, model_textures, 'cj')

        script_path = os.path.join(root, *item_class.split('/')) + '.asl'
        os.makedirs(os.path.dirname(script_path), exist_ok=True)
        with open(script_path, 'w', encoding='latin-1') as handle:
            handle.write(CJ_ITEM_SCRIPT)

        # Every item needs an inventory picture at <class>[icon].bmp, and the
        # engine logs an error and shows nothing without one.
        import shutil
        shutil.copyfile(os.path.join(bitmap_dir, 'head.bmp'),
                        os.path.join(root, *item_class.split('/')) + '[icon].bmp')

        # One throwable CJ per chamber.
        for chamber in range(CHAMBERS):
            _x0, _x1, chamber_centre = chamber_bounds(chamber)
            entities.append((item_class.upper().replace('/', '\\') + '.TEO',
                             (chamber_centre[0], FLOOR_Y - 20.0, chamber_centre[2]),
                             chamber + 1))

        # The NPC is a separate class from the item, not a replacement for it:
        # AddInteractive tests "items" before "npc", so a path under items/ can
        # never be an NPC however it is scripted.
        # Not npc/cj/cj: EntityId is the class path's *filename* plus the
        # instance number (game/EntityId.cpp), so an NPC called cj would share
        # ids cj_0001.. with the item above. LoadInter_Ex looks an id up before
        # it creates anything, finds the item already loaded and returns it, and
        # the NPC is never built - no warning, no entity, nothing in the log.
        npc_class = f'graph/levels/level{level}/npc/cj_npc/cj_npc'
        script_path = os.path.join(root, *npc_class.split('/')) + '.asl'
        os.makedirs(os.path.dirname(script_path), exist_ok=True)
        with open(script_path, 'w', encoding='latin-1') as handle:
            handle.write(CJ_NPC_SCRIPT)

        # The mesh this script wants is the rigged one out of cj.blend, whose
        # vertex groups match human_base so the stock animations drive it. This
        # generator only has the .obj, which has no groups at all, so it writes a
        # stand-in and steps aside as soon as a real export exists - otherwise
        # every run would silently replace a rigged NPC with a static one.
        npc_mesh = os.path.join(root, 'game', npc_class + '.ftl')
        if os.path.exists(npc_mesh):
            print(f"  keeping existing rigged {os.path.relpath(npc_mesh, root)}")
        else:
            write_ftl(npc_mesh, obj, scale, model_textures, 'cj')
            print("  WARNING: that mesh has no bone groups, so the NPC will walk "
                  "without animating. Export cj.blend over it.")

        # One walker per chamber, off the corridor's centre line. Two entities
        # that start inside each other's collision cylinders are both stuck, and
        # the player spawns on that line at the west end - which is exactly what
        # putting the first CJ at the chamber centre did.
        for chamber in range(CHAMBERS):
            _x0, _x1, chamber_centre = chamber_bounds(chamber)
            entities.append((npc_class.upper().replace('/', '\\') + '.TEO',
                             (chamber_centre[0], FLOOR_Y - 20.0,
                              chamber_centre[2] - 250.0),
                             chamber + 1))

    # The game's own torch, so there is a known good entity beside the new ones.
    _a, _b, first_centre = chamber_bounds(0)
    entities.append(('GRAPH\\OBJ3D\\INTERACTIVE\\ITEMS\\PROVISIONS\\TORCH\\TORCH.TEO',
                     (first_centre[0] - 120.0, FLOOR_Y - 20.0, first_centre[2]), 1))

    # Script only classes: the engine is happy with an entity that has nothing but
    # an .asl (AddMarker and AddCamera check for ftl, teo or asl), so these live in
    # the level folder like everything else. The class path has to name "marker" or
    # "camera" for AddInteractive to route them.
    for class_path, script, position, ident in (
            (f'graph/levels/level{level}/marker/trigger/trigger', TRIGGER_SCRIPT,
             (first_centre[0], FLOOR_Y - 10.0, first_centre[2]), 1),
            (f'graph/levels/level{level}/camera/intro/intro', CAMERA_SCRIPT,
             (first_centre[0], FLOOR_Y - 160.0, first_centre[2]), 1)):
        script_file = os.path.join(root, *class_path.split('/')) + '.asl'
        os.makedirs(os.path.dirname(script_file), exist_ok=True)
        with open(script_file, 'w', encoding='iso-8859-15') as handle:
            handle.write(script)
        entities.append((class_path.upper().replace('/', '\\') + '.TEO', position, ident))

    # A zone per chamber, each with its own ambient track and tint, plus a path
    # running through the middle of them for the camera to follow.
    routes = [make_zone(f'CHAMBER_{i + 1}', i) for i in range(CHAMBERS)]
    routes.append(make_patrol_path())

    # Same navmesh builder the Blender exporter runs, fed the polygons that are
    # about to be written. Nothing here knows the corridor's layout: the graph
    # comes out of the geometry, statues included.
    anchors = anchors_module.build_anchors(polygons)
    ordered = write_fts(os.path.join(fts_dir, 'fast.fts'), polygons, textures,
                        make_portals(centre_z), anchors)
    colors = write_llf(os.path.join(dlf_dir, f'level{level}.llf'), ordered, lights)
    write_dlf(os.path.join(dlf_dir, f'level{level}.dlf'), level, player_start,
              entities, routes)

    floor = tuple(int(c * 255.0 + 0.5) for c in danae.DEFAULT_AMBIENT)
    lit = sum(1 for c in colors if c[:3] != floor)
    print(f"wrote level {level} to {root}")
    print(f"  corridor {LENGTH_TILES * TILE:.0f} x {WIDTH_TILES * TILE:.0f} units, "
          f"{shell_count} shell polygons, {len(polygons) - shell_count} model polygons")
    print(f"  {len(textures)} textures, {len(lights)} lights, "
          f"{len(colors)} vertex colours, {lit} above the ambient floor "
          f"({100.0 * lit / len(colors):.0f}%)")
    print(f"  brightest {max(colors, key=lambda c: sum(c[:3]))[:3]}")
    link_count = sum(len(a['links']) for a in anchors)
    print(f"  {len(anchors)} anchors, {link_count // 2} links "
          f"(radius {anchors_module.ANCHOR_RADIUS:.0f}, "
          f"height {anchors_module.ANCHOR_HEIGHT:.0f})")
    zones = [r for r, _w in routes if r.height != 0]
    print(f"  {len(entities)} entities placed, {len(zones)} zones, "
          f"{len(routes) - len(zones)} paths")
    for route, ways in routes:
        name = route.name.decode('iso-8859-1').strip('\x00')
        kind = 'zone' if route.height else 'path'
        ambiance = route.ambiance.decode('iso-8859-1').strip('\x00')
        print(f"    {kind} {name}: {len(ways)} points"
              + (f", ambiance {ambiance}" if ambiance else ""))
    print(f"  player starts at ({player_start[0]:.0f}, {player_start[1]:.0f}, "
          f"{player_start[2]:.0f})")


if __name__ == '__main__':
    main()
