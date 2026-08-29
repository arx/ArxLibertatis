#!/usr/bin/env python3
"""Self-check for FTS room section sizing.

The engine reads exactly nb_rooms + 1 room structures followed by an
(nb_rooms + 1)^2 room distance matrix. nb_rooms is derived from the room ids
referenced by polygons *and* portals, so a portal pointing at a room id that no
polygon uses must still produce a room entry, otherwise every later read in the
file is misaligned.

Run with: python3 plugins/blender/test_fts_rooms.py
"""

import importlib.util
import os
import sys
import types

# Load the two serializer modules on their own. Importing the arx_addon package
# pulls in bpy, which only exists inside Blender.
_ADDON_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'arx_addon')
_package = types.ModuleType('arx_addon_standalone')
_package.__path__ = [_ADDON_DIR]
sys.modules['arx_addon_standalone'] = _package


def _load(name):
    spec = importlib.util.spec_from_file_location(
        'arx_addon_standalone.' + name, os.path.join(_ADDON_DIR, name + '.py'))
    module = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    return module


_load('dataCommon')
dataFts = _load('dataFts')

EERIE_SAVE_PORTALS = dataFts.EERIE_SAVE_PORTALS
FtsData = dataFts.FtsData
FtsSerializer = dataFts.FtsSerializer
ROOM_DIST_DATA_SAVE = dataFts.ROOM_DIST_DATA_SAVE


def make_poly(room):
    vertex = {'ssx': 0.0, 'sy': 0.0, 'ssz': 0.0, 'stu': 0.0, 'stv': 0.0}
    normal = {'x': 0.0, 'y': 1.0, 'z': 0.0}
    return {
        'vertices': [dict(vertex) for _ in range(4)],
        'vertex_normals': [dict(normal) for _ in range(4)],
        'tex': 0,
        'transval': 0.0,
        'area': 1.0,
        'room': room,
        'norm': dict(normal),
        'norm2': dict(normal),
        'poly_type': 64,
        'is_quad': True,
    }


def make_portal(room_1, room_2, centre=(10000.0, -125.0, 8900.0)):
    portal = EERIE_SAVE_PORTALS()
    portal.room_1 = room_1
    portal.room_2 = room_2
    portal.useportal = 1
    # A doorway sized quad around `centre`, so its centre is meaningful to the
    # room distance computation.
    for i, (dy, dz) in enumerate(((0.0, -100.0), (0.0, 100.0),
                                  (-125.0, -100.0), (-125.0, 100.0))):
        portal.poly.v[i].pos.x = centre[0]
        portal.poly.v[i].pos.y = centre[1] + dy
        portal.poly.v[i].pos.z = centre[2] + dz
    return bytes(portal)


def build(poly_rooms, portals, room_data_list, room_distances):
    cells = [[None for _ in range(160)] for _ in range(160)]
    cells[0][0] = [make_poly(room) for room in poly_rooms]
    fts_data = FtsData(
        sceneOffset=(0.0, 0.0, 0.0),
        textures=[],
        cells=cells,
        cell_anchors=None,
        anchors=[],
        portals=portals,
        room_data=(room_data_list, room_distances),
    )
    return fts_data, cells


def empty_room():
    return ({'nb_portals': 0, 'nb_polys': 0, 'padd': [0] * 6}, [], [])


def check(name, poly_rooms, portals, room_data_list, expected_rooms):
    """Round trip one level and assert the room section came back the right size."""
    serializer = FtsSerializer(None)  # ioLib is only used by the container read path
    fts_data, cells = build(poly_rooms, portals, room_data_list, [])
    data = serializer.write_fts(fts_data, cells)

    # Raises if the room section is sized differently than the header claims,
    # because the distance matrix read then runs off the end of the buffer.
    parsed_header, parsed = serializer.read_fts(data)
    parsed_rooms, parsed_distances = parsed.room_data

    assert parsed_header.nb_rooms + 1 == expected_rooms, \
        f"{name}: header says nb_rooms={parsed_header.nb_rooms}, expected {expected_rooms - 1}"

    assert len(parsed_rooms) == expected_rooms, \
        f"{name}: read back {len(parsed_rooms)} rooms, expected {expected_rooms}"
    assert len(parsed_distances) == expected_rooms, \
        f"{name}: distance matrix has {len(parsed_distances)} rows, expected {expected_rooms}"
    for row in parsed_distances:
        assert len(row) == expected_rooms, \
            f"{name}: distance row has {len(row)} entries, expected {expected_rooms}"

    # Polygon room ids must survive untouched, negatives included: a negative id
    # means "in no room" and the engine reads it as an empty RoomHandle.
    read_back = [poly.room for poly in parsed.cells[0][0]]
    assert read_back == list(poly_rooms), \
        f"{name}: polygon rooms came back as {read_back}, expected {list(poly_rooms)}"

    print(f"  ok: {name} ({expected_rooms} rooms)")


def check_distances(name, portals, room_data_list, supplied, expected):
    """Write a level and read back the routes in its room distance matrix.

    SP_GetRoomDist answers fdist(from, startpos) + distance + fdist(endpos, to),
    so an entry whose waypoints sit on the world origin reports a distance roughly
    equal to the level's distance from the origin. That silently drops entities
    out of the treat zone, which freezes them until the player shares their room.
    """
    serializer = FtsSerializer(None)
    fts_data, cells = build([0, 1], portals, room_data_list, supplied)
    parsed_header, parsed = serializer.read_fts(serializer.write_fts(fts_data, cells))
    _rooms, distances = parsed.room_data

    routes = {}
    for i, row in enumerate(distances):
        for j, cell in enumerate(row):
            entry = ROOM_DIST_DATA_SAVE.from_buffer_copy(cell) if isinstance(cell, bytes) else cell
            if entry.distance > 0.0:
                routes[(i, j)] = (round(entry.distance, 1),
                                  (entry.startpos.x, entry.startpos.z),
                                  (entry.endpos.x, entry.endpos.z))

    assert routes == expected, f"{name}: got {routes}, expected {expected}"
    print(f"  ok: {name}")


def main():
    print("FTS room section sizing:")

    # Portal references room 5 while no polygon goes above room 1. The room list
    # supplied by the caller only covers rooms 0..1 and must get padded to 0..5.
    check(
        "portal room id above every polygon room id",
        poly_rooms=[0, 1],
        portals=[make_portal(1, 5)],
        room_data_list=[empty_room(), empty_room()],
        expected_rooms=6,
    )

    # Caller supplies more rooms than anything references; the extras have to go
    # so the reader does not stop short of the distance matrix.
    check(
        "room list longer than the highest referenced room id",
        poly_rooms=[0, 1],
        portals=[],
        room_data_list=[empty_room() for _ in range(9)],
        expected_rooms=2,
    )

    # Nothing to reconcile.
    check(
        "room list already matching",
        poly_rooms=[0, 1, 2],
        portals=[make_portal(1, 2)],
        room_data_list=[empty_room() for _ in range(3)],
        expected_rooms=3,
    )

    # Polygons in no room carry a negative id. It has to round trip, and it must
    # not inflate the room count the way clamping it to 0 used to merge those
    # polygons into room 0.
    check(
        "polygons with no room keep their negative id",
        poly_rooms=[-1, 0, 1, -1],
        portals=[],
        room_data_list=[empty_room(), empty_room()],
        expected_rooms=2,
    )

    print("\nRoom distance routes:")

    # A room built in Blender arrives with no usable matrix at all.
    check_distances(
        "routes derived from the portal graph when none are supplied",
        portals=[make_portal(1, 2)],
        room_data_list=[empty_room(), empty_room(), empty_room()],
        supplied=[],
        expected={(1, 2): (1.0, (10000.0, 8900.0), (10000.0, 8900.0)),
                  (2, 1): (1.0, (10000.0, 8900.0), (10000.0, 8900.0))},
    )

    # Two hops: the run between the two doorways becomes the stored distance.
    check_distances(
        "multi room routes measure the run between doorways",
        portals=[make_portal(1, 2, (10000.0, -125.0, 8900.0)),
                 make_portal(2, 3, (10800.0, -125.0, 8900.0))],
        room_data_list=[empty_room() for _ in range(4)],
        supplied=[],
        expected={(1, 2): (1.0, (10000.0, 8900.0), (10000.0, 8900.0)),
                  (2, 1): (1.0, (10000.0, 8900.0), (10000.0, 8900.0)),
                  (2, 3): (1.0, (10800.0, 8900.0), (10800.0, 8900.0)),
                  (3, 2): (1.0, (10800.0, 8900.0), (10800.0, 8900.0)),
                  (1, 3): (800.0, (10000.0, 8900.0), (10800.0, 8900.0)),
                  (3, 1): (800.0, (10800.0, 8900.0), (10000.0, 8900.0))},
    )

    # What _rebuildRoomPolygonReferences hands over for a room added in Blender:
    # a full sized matrix carrying the 999999 sentinel and no waypoints at all.
    def sentinel_matrix(size):
        rows = []
        for i in range(size):
            row = []
            for j in range(size):
                entry = ROOM_DIST_DATA_SAVE()
                entry.distance = 0.0 if i == j else 999999.0
                row.append(bytes(entry))
            rows.append(row)
        return rows

    check_distances(
        "a supplied matrix of sentinels is replaced, not trusted",
        portals=[make_portal(1, 2)],
        room_data_list=[empty_room(), empty_room(), empty_room()],
        supplied=sentinel_matrix(3),
        expected={(1, 2): (1.0, (10000.0, 8900.0), (10000.0, 8900.0)),
                  (2, 1): (1.0, (10000.0, 8900.0), (10000.0, 8900.0))},
    )

    print("all ok")


if __name__ == '__main__':
    main()
