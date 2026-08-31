#!/usr/bin/env python3
# Copyright 2014-2026 Arx Libertatis Team (see the AUTHORS file)
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

"""Check, and optionally repair, a fast.fts that has been round tripped.

Three things in an FTS are indexed by room id: the polygons, the portals, and the
per room structures that list which portals and polygons a room owns. An exporter
that renumbers rooms has to move all three. One that moves only the first two
leaves every room holding some other room's portal list, and a room whose list
comes out empty can never be opened by portal traversal - its frustum list stays
empty and ARX_SCENE_PORTAL_ClipIO hides every entity standing in it.

Both facts are recoverable from the portals themselves, which is what --fix does:
a room's portals are exactly the portals naming it, and the distance matrix is the
shortest route through that portal graph.

It also checks the anchors. An anchor's height is a cylinder height and so is
negative - the stock levels use -165. An exporter that lost the value and wrote a
positive default makes every anchor fail PathFinder's `height > m_height` test
against any creature, which leaves the level with a navmesh nothing can use.

  repair_fts.py <fast.fts> [--fix]

--fix writes the payload uncompressed, which FastSceneLoad accepts: it only calls
blast when the header's uncompressedsize is set. The original is kept as .bak.
"""

import ctypes
import importlib
import os
import shutil
import sys
import types
from ctypes import sizeof

_HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, _HERE)
sys.path.insert(0, os.path.join(_HERE, 'arx_addon'))
_pkg = types.ModuleType('arx_repair_pkg')
_pkg.__path__ = [os.path.join(_HERE, 'arx_addon')]
sys.modules['arx_repair_pkg'] = _pkg
F = importlib.import_module('arx_repair_pkg.dataFts')
from lib import ArxIO


def read_container(path):
    data = open(path, 'rb').read()
    pos = 0
    header = F.UNIQUE_HEADER.from_buffer_copy(data, pos)
    pos += sizeof(F.UNIQUE_HEADER)
    pos += sizeof(F.UNIQUE_HEADER3) * header.count
    payload = data[pos:]
    if header.uncompressedsize:
        payload = bytes(ArxIO().unpack(payload))
    return data[:pos], header, payload


def parse(raw):
    """Offsets and contents of everything from the portal section on."""
    pos = 0
    head = F.FAST_SCENE_HEADER.from_buffer_copy(raw, pos)
    pos += sizeof(F.FAST_SCENE_HEADER)
    pos += sizeof(F.FAST_TEXTURE_CONTAINER) * head.nb_textures
    for _ in range(head.sizex * head.sizez):
        info = F.FAST_SCENE_INFO.from_buffer_copy(raw, pos)
        pos += sizeof(F.FAST_SCENE_INFO)
        pos += sizeof(F.FAST_EERIEPOLY) * info.nbpoly
        pos += sizeof(ctypes.c_int32) * info.nbianchors
    anchors = []
    for _ in range(head.nb_anchors):
        anchor = F.FAST_ANCHOR_DATA.from_buffer_copy(raw, pos)
        anchors.append((pos, anchor))
        pos += sizeof(F.FAST_ANCHOR_DATA)
        pos += sizeof(ctypes.c_int32) * anchor.nb_linked

    portals = []
    for _ in range(head.nb_portals):
        portals.append(F.EERIE_SAVE_PORTALS.from_buffer_copy(raw, pos))
        pos += sizeof(F.EERIE_SAVE_PORTALS)

    rooms_start = pos
    rooms = []
    for _ in range(head.nb_rooms + 1):
        info = F.EERIE_SAVE_ROOM_DATA.from_buffer_copy(raw, pos)
        pos += sizeof(F.EERIE_SAVE_ROOM_DATA)
        indices = list((ctypes.c_int32 * info.nb_portals).from_buffer_copy(raw, pos))
        pos += 4 * info.nb_portals
        polys = raw[pos:pos + sizeof(F.FAST_EP_DATA) * info.nb_polys]
        pos += len(polys)
        rooms.append((info, indices, polys))

    pos += sizeof(F.ROOM_DIST_DATA_SAVE) * (head.nb_rooms + 1) ** 2
    if pos != len(raw):
        print(f"  warning: {len(raw) - pos} bytes left over after the room data")
    return head, portals, anchors, rooms_start, rooms


def check(head, portals, rooms):
    stale = []
    stranded = []
    for room, (info, indices, polys) in enumerate(rooms):
        for index in indices:
            if index >= len(portals):
                stale.append((room, index, 'out of range'))
            elif portals[index].room_1 != room and portals[index].room_2 != room:
                stale.append((room, index,
                              (portals[index].room_1, portals[index].room_2)))
        # Room 0 is the engine's unused first slot - RoomHandle 0 is the null
        # handle, so polygons parked there are simply not room culled.
        if room > 0 and info.nb_polys > 0 and not indices:
            stranded.append(room)
    return stale, stranded


def rebuild(head, portals, rooms):
    """Room section and distance matrix, both derived from the portals."""
    out = bytearray()
    for room, (info, _indices, polys) in enumerate(rooms):
        indices = [i for i, portal in enumerate(portals)
                   if portal.room_1 == room or portal.room_2 == room]
        info.nb_portals = len(indices)
        out.extend(bytes(info))
        for index in indices:
            out.extend(ctypes.c_int32(index))
        out.extend(polys)

    size = head.nb_rooms + 1
    routes = F.compute_room_distances(portals, head.nb_rooms)
    for i in range(size):
        for j in range(size):
            entry = F.ROOM_DIST_DATA_SAVE()
            route = routes.get((i, j))
            if route and i != j:
                entry.distance = route[0]
                entry.startpos.x, entry.startpos.y, entry.startpos.z = route[1]
                entry.endpos.x, entry.endpos.y, entry.endpos.z = route[2]
            # Otherwise zero, which the engine reads as "no route" and answers
            # with straight line distance.
            out.extend(bytes(entry))
    return bytes(out)


#: What DANAE's own anchors use, and what EERIE_COLLISION_Cylinder_Create clamps a
#: human sized entity to. Anything shallower admits nobody.
ANCHOR_HEIGHT = -165.0


def fix_anchors(raw, anchors):
    """Give back any anchor whose height is not a cylinder height."""
    broken = [(offset, anchor) for offset, anchor in anchors if anchor.height >= 0.0]
    if not broken:
        return raw, 0
    patched = bytearray(raw)
    field = F.FAST_ANCHOR_DATA.height.offset
    for offset, _anchor in broken:
        patched[offset + field:offset + field + 4] = bytes(ctypes.c_float(ANCHOR_HEIGHT))
    return bytes(patched), len(broken)


def main():
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    fix = '--fix' in sys.argv[1:]
    if len(args) != 1:
        print(__doc__)
        return 1
    path = args[0]

    prefix, header, raw = read_container(path)
    head, portals, anchors, rooms_start, rooms = parse(raw)
    print(f"{path}: {head.nb_rooms + 1} rooms, {len(portals)} portals, "
          f"{head.nb_polys} polygons")

    bad_anchors = [a for _, a in anchors if a.height >= 0.0]
    print(f"  anchors with a non negative height (unusable by any creature): "
          f"{len(bad_anchors)} of {len(anchors)}")

    stale, stranded = check(head, portals, rooms)
    print(f"  room portal lists naming a portal that does not touch the room: {len(stale)}")
    for entry in stale[:5]:
        print(f"    room {entry[0]} lists portal {entry[1]}, which joins rooms {entry[2]}")
    print(f"  rooms with geometry but no portals (entities in them are never drawn): "
          f"{stranded if stranded else 'none'}")

    if not stale and not stranded and not bad_anchors:
        print("  nothing to repair")
        return 0
    if not fix:
        print("  re-run with --fix to repair")
        return 1

    raw, fixed = fix_anchors(raw, anchors)
    if fixed:
        print(f"  reset {fixed} anchor heights to {ANCHOR_HEIGHT}")
    repaired = raw[:rooms_start]
    if stale or stranded:
        repaired += rebuild(head, portals, rooms)
    else:
        repaired += raw[rooms_start:]
    backup = path + '.bak'
    if not os.path.exists(backup):  # never clobber an earlier original
        shutil.copy2(path, backup)
    header.uncompressedsize = 0  # FastSceneLoad only calls blast when this is set
    with open(path, 'wb') as out:
        out.write(bytes(header))
        out.write(prefix[sizeof(F.UNIQUE_HEADER):])
        out.write(repaired)
    print(f"  rebuilt; original kept as {os.path.basename(path)}.bak")
    return 0


if __name__ == '__main__':
    sys.exit(main())
