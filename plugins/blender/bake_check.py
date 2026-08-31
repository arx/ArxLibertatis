"""Headless check of the DANAE lightmap bake against a reference .llf.

Imports an area from a data root, runs the bake without writing anything, and
scores the result per vertex against the lightmap that shipped with the game.
Nothing is written, so it is safe to point at real game data.

    build/blender-link --background --factory-startup \
        --python plugins/blender/bake_check.py -- <data root> <reference .llf> [area]

The data root is an unpacked data.pak plus data2.pak. Getting one:

    build/arxunpak -o /tmp/arxdata "/path/to/Arx Fatalis/data.pak"
    build/arxunpak -o /tmp/arxdata "/path/to/Arx Fatalis/data2.pak"
    cp /tmp/arxdata/graph/levels/level1/level1.llf /tmp/reference.llf
"""
import sys

import addon_utils
import bpy
from mathutils import Vector

argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
if len(argv) < 2:
    print(__doc__)
    raise SystemExit(2)

DATA, REFERENCE = argv[0], argv[1]
AREA = int(argv[2]) if len(argv) > 2 else 1

addon_utils.enable("arx_addon", default_set=True, persistent=False)
bpy.context.preferences.addons["arx_addon"].preferences.arxAssetPath = DATA

from arx_addon.managers import getAddon
from arx_addon.arx_ui_area import importArea, CUSTOM_OT_arx_area_list_export_all as ExportOp


class Harness:
    """The export operator's methods on a plain object.

    bpy Operator subclasses cannot be instantiated from Python, so lift the
    functions out of the class dictionaries instead.
    """

    def __init__(self):
        self._scene_lights = []
        self._scene_offset = Vector((0, 0, 0))
        self._preserve_original_lighting = False
        self.converted_faces = []
        self._cell_grid = None
        self._ordered_polys = None

    def report(self, level, message):
        print(f"  [{'/'.join(level)}] {message}")


for _cls in reversed(ExportOp.__mro__):
    if _cls.__module__ != ExportOp.__module__:
        continue
    for _name, _value in vars(_cls).items():
        if callable(_value) and not _name.startswith('__') and _name != 'report':
            setattr(Harness, _name, _value)


importArea(bpy.context, lambda level, message: None, AREA)
scene = bpy.data.scenes[f"Area_{AREA:02d}"]
background = next(o for o in scene.objects
                  if o.type == 'MESH' and o.name.endswith('-background'))

addon = getAddon(bpy.context)
reference = [(c.r, c.g, c.b)
             for c in addon.sceneManager.llfSerializer.read(REFERENCE).levelLighting]

with bpy.context.temp_override(scene=scene):
    # Object matrices are lazily evaluated. Without this every matrix_world in a
    # --background run is still the identity, which parks all the lights on the
    # world origin and bakes a pitch black level.
    bpy.context.view_layer.update()

    fts_data = addon.sceneManager.ftsSerializer.read_fts_container(
        addon.arxFiles.levels.levels[AREA].fts)

    harness = Harness()
    harness._scene_offset = Vector(fts_data.sceneOffset)
    fts_data = harness.convertMeshToFtsCells(background, fts_data)
    ordered = harness._orderedPolygons(fts_data)
    colors = harness._calculateDanaeVertexLighting(ordered, scene)

print("\n" + "=" * 60)
print(f"AREA {AREA}: {len(ordered)} polygons, {len(colors)} vertices "
      f"(reference has {len(reference)})")
print("=" * 60)

if len(colors) != len(reference):
    print("FAIL: vertex count does not match the reference, so the engine would "
          "reject this lightmap outright")
    raise SystemExit(1)

deltas = [max(abs(x - y) for x, y in zip(p, q[:3])) for p, q in zip(reference, colors)]
n = len(deltas)
mean_reference = sum(sum(p) for p in reference) / (3.0 * n)
mean_candidate = sum(c[0] + c[1] + c[2] for c in colors) / (3.0 * n)

print(f"  identical:      {sum(1 for d in deltas if d == 0):8d} "
      f"({100.0 * sum(1 for d in deltas if d == 0) / n:5.1f}%)")
for limit in (2, 8, 32):
    within = sum(1 for d in deltas if d <= limit)
    print(f"  within {limit:2d}:      {within:8d} ({100.0 * within / n:5.1f}%)")
print(f"  mean delta:     {sum(deltas) / n:8.2f}")
print(f"  worst delta:    {max(deltas):8d}")
print(f"  mean channel:   reference {mean_reference:.2f}, ours {mean_candidate:.2f} "
      f"({100.0 * (mean_candidate - mean_reference) / mean_reference:+.1f}%)")
