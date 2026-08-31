#!/usr/bin/env python3
"""Self-check for the DANAE lightmap bake port.

Pins the numbers the original produced: linear accumulation of
intensity * GLOBAL_LIGHT_FACTOR, a linear falloff band between fallstart and
fallend, an ambient floor, a hard clamp at 1.0, and no colour space step
anywhere. See arx_addon/danae_lighting.py for the source functions.

Run with: python3 plugins/blender/test_danae_lighting.py
"""

import importlib.util
import os
import sys
import types

_ADDON_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'arx_addon')
_package = types.ModuleType('arx_addon_standalone')
_package.__path__ = [_ADDON_DIR]
sys.modules['arx_addon_standalone'] = _package

_spec = importlib.util.spec_from_file_location(
    'arx_addon_standalone.danae_lighting', os.path.join(_ADDON_DIR, 'danae_lighting.py'))
danae_lighting = importlib.util.module_from_spec(_spec)
sys.modules[_spec.name] = danae_lighting
_spec.loader.exec_module(danae_lighting)

DanaeLight = danae_lighting.DanaeLight
DanaePolygon = danae_lighting.DanaePolygon
light_polygon = danae_lighting.light_polygon
compute_vertex_colors = danae_lighting.compute_vertex_colors

AMBIENT = danae_lighting.DEFAULT_AMBIENT

# The lightmaps shipped with the game bottom out at exactly this value, which is
# 0.09 * 255 rounded rather than truncated. Hard coded on purpose: it is measured
# from the original data, not derived from the code under test.
AMBIENT_BYTE = 23


def white_light(height, intensity=1.0, fallstart=100.0, fallend=200.0, casts_shadow=True):
    return DanaeLight(pos=(0.0, height, 0.0), rgb=(1.0, 1.0, 1.0), intensity=intensity,
                      fallstart=fallstart, fallend=fallend, casts_shadow=casts_shadow)


def flat_quad(normal=(0.0, 1.0, 0.0), ignore=False):
    """One unit quad at the origin, every vertex facing `normal`."""
    vertices = [(0.0, 0.0, 0.0), (1.0, 0.0, 0.0), (0.0, 0.0, 1.0), (1.0, 0.0, 1.0)]
    return DanaePolygon(vertices=vertices, normals=[normal] * 4,
                        center=(0.5, 0.0, 0.5), ignore=ignore)


def check(name, actual, expected):
    assert actual == expected, f"{name}: got {actual}, expected {expected}"
    print(f"  ok: {name}")


def main():
    print("DANAE lightmap bake:")

    # The ambient floor has to reproduce the value in the shipped lightmaps.
    colors = light_polygon(flat_quad(), [], AMBIENT, True, None)
    check("ambient floor matches the shipped lightmaps", colors[0][:3],
          (AMBIENT_BYTE, AMBIENT_BYTE, AMBIENT_BYTE))

    # Inside fallstart: full intensity * GLOBAL_LIGHT_FACTOR, nothing else.
    colors = light_polygon(flat_quad(), [white_light(50.0)], AMBIENT, True, None)
    full = round(1.0 * 0.85 * 255)
    check("inside fallstart is intensity * 0.85", colors[0][:3], (full, full, full))
    check("one colour per vertex", len(colors), 4)

    # Halfway down the falloff band: (fallend - d) / (fallend - fallstart) = 0.5.
    colors = light_polygon(flat_quad(), [white_light(150.0)], AMBIENT, True, None)
    half = round(0.85 * 0.5 * 255)
    check("falloff band is linear", colors[0][:3], (half, half, half))

    # Past fallend the light contributes nothing and the ambient floor shows.
    colors = light_polygon(flat_quad(), [white_light(250.0)], AMBIENT, True, None)
    check("beyond fallend falls back to ambient", colors[0][:3],
          (AMBIENT_BYTE, AMBIENT_BYTE, AMBIENT_BYTE))

    # A surface facing away gets nothing from MODE_NORMALS.
    colors = light_polygon(flat_quad(normal=(0.0, -1.0, 0.0)), [white_light(50.0)],
                           AMBIENT, True, None)
    check("back facing vertex gets no light", colors[0][:3],
          (AMBIENT_BYTE, AMBIENT_BYTE, AMBIENT_BYTE))

    # With MODE_NORMALS off the same vertex is lit, as in the original.
    colors = light_polygon(flat_quad(normal=(0.0, -1.0, 0.0)), [white_light(50.0)],
                           AMBIENT, False, None)
    check("without MODE_NORMALS orientation is ignored", colors[0][:3], (full, full, full))

    # A hit on the polygon being lit is not an obstruction: every vertex sits on
    # its own polygon, so Visible() forgives exactly that one case.
    colors = light_polygon(flat_quad(), [white_light(50.0)], AMBIENT, True,
                           lambda origin, target, index=None: index == 7, 7)
    check("a hit on the polygon being lit is not a shadow", colors[0][:3], (full, full, full))

    # A blocked shadow ray kills the contribution entirely.
    colors = light_polygon(flat_quad(), [white_light(50.0)], AMBIENT, True,
                           lambda origin, target, index=None: False)
    check("occluded vertex falls back to ambient", colors[0][:3],
          (AMBIENT_BYTE, AMBIENT_BYTE, AMBIENT_BYTE))

    # EXTRAS_NOCASTED lights skip the ray launch pass, so they light through walls.
    colors = light_polygon(flat_quad(), [white_light(50.0, casts_shadow=False)], AMBIENT,
                           True, lambda origin, target, index=None: False)
    check("non casting light ignores occlusion", colors[0][:3], (full, full, full))

    # Accumulation is linear and clamps at 1.0, never above.
    colors = light_polygon(flat_quad(), [white_light(50.0), white_light(50.0)],
                           AMBIENT, True, None)
    check("accumulation clamps at full white", colors[0][:3], (255, 255, 255))

    # POLY_IGNORE polygons are skipped by the bake but still need their vertices.
    colors = light_polygon(flat_quad(ignore=True), [white_light(50.0)], AMBIENT, True, None)
    check("ignored polygon still emits ambient vertices", colors[0][:3],
          (AMBIENT_BYTE, AMBIENT_BYTE, AMBIENT_BYTE))

    # The flat result must follow polygon order, three values for a triangle and
    # four for a quad, because that is how the .llf file is read back.
    triangle = DanaePolygon(vertices=[(0.0, 0.0, 0.0), (1.0, 0.0, 0.0), (0.0, 0.0, 1.0)],
                            normals=[(0.0, 1.0, 0.0)] * 3, center=(0.3, 0.0, 0.3), ignore=False)
    flat = compute_vertex_colors([triangle, flat_quad()], [white_light(50.0)], AMBIENT, True, None)
    check("flat output is 3 + 4 values", len(flat), 7)

    print("all ok")


if __name__ == '__main__':
    main()
