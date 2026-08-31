#!/usr/bin/env python3
"""Pair every polygon in an .fts with its colour in the matching .llf.

The engine walks tiles in Z rows then X columns and consumes vertex colours from
the .llf in exactly that order, so a lightmap can have the right values in the
wrong places and still have a perfectly valid vertex count. This prints the
pairing the engine will actually make, so a polygon that renders dark can be
looked up by where it is in the world.

    python3 plugins/blender/level_lighting_dump.py <data root> <level> [x0 z0 x1 z1]

The optional bounds restrict the listing to polygons whose centre falls inside
them, which is how you isolate a newly added object from the rest of a level.
"""
import importlib.util
import os
import sys
import types

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
dataLlf = _load('dataLlf')
sys.path.insert(0, ADDON)
from lib import ArxIO  # noqa: E402


def main():
    if len(sys.argv) < 3:
        print(__doc__)
        raise SystemExit(2)

    root, level = sys.argv[1], sys.argv[2]
    bounds = [float(v) for v in sys.argv[3:7]] if len(sys.argv) >= 7 else None

    io = ArxIO()
    fts = dataFts.FtsSerializer(io).read_fts_container(
        os.path.join(root, 'game', 'graph', 'levels', f'level{level}', 'fast.fts'))
    llf = dataLlf.LlfSerializer(io).read(
        os.path.join(root, 'graph', 'levels', f'level{level}', f'level{level}.llf'))
    colors = llf.levelLighting

    # Tiles in the engine's order: rows along Z, columns along X.
    expected = 0
    rows = []
    for z in range(160):
        for x in range(160):
            cell = fts.cells[z][x] if fts.cells[z] is not None else None
            for index, poly in enumerate(cell or []):
                count = 4 if poly.type.POLY_QUAD else 3
                rows.append((x, z, index, poly, expected, count))
                expected += count

    print(f"level {level}: {len(rows)} polygons need {expected} vertex colours, "
          f"the lightmap has {len(colors)}")
    if expected != len(colors):
        print("  MISMATCH: the engine discards the whole lightmap when these differ")

    shown = 0
    dark = 0
    for x, z, index, poly, offset, count in rows:
        verts = [(poly.v[i].ssx, poly.v[i].sy, poly.v[i].ssz) for i in range(count)]
        centre = tuple(sum(v[i] for v in verts) / count for i in range(3))

        if bounds and not (bounds[0] <= centre[0] <= bounds[2]
                           and bounds[1] <= centre[2] <= bounds[3]):
            continue

        if offset + count > len(colors):
            print(f"  tile ({x},{z}) #{index}: colour offset {offset} is past the end")
            continue

        rgb = [(colors[offset + i].r, colors[offset + i].g, colors[offset + i].b)
               for i in range(count)]
        mean = sum(sum(c) for c in rgb) / (3.0 * count)
        if mean <= 24:
            dark += 1
        if shown < 40:
            print(f"  tile ({x:3d},{z:3d}) #{index:3d} centre "
                  f"({centre[0]:8.1f},{centre[1]:8.1f},{centre[2]:8.1f}) "
                  f"room {poly.room:3d} area {poly.area:8.1f} mean {mean:6.1f} {rgb[0]}")
        shown += 1

    print(f"  listed {shown} polygons, {dark} of them sitting at the ambient floor")


if __name__ == '__main__':
    main()
