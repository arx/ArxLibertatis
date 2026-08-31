# Copyright 2025 Arx Libertatis Team (see the AUTHORS file)
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.

"""Port of the level lightmap bake from the original DANAE editor.

This is the algorithm that produced the .llf files shipped with the game, not an
approximation of it with a modern renderer. The two functions it comes from are,
in the Arx Fatalis GPL source drop kept under src/DANAE_OLD:

    ARX_EERIE_LIGHT_Make   (DANAE/ARX_Paths.cpp)   per light, per vertex
    EERIE_LIGHT_Apply      (EERIE/EERIELight.cpp)  accumulate, clamp, write

Everything happens in Arx world units and in linear 0..1 intensity space. There
is no gamma step and no brightness scale anywhere in the original: the engine
multiplies the stored vertex colour straight into the texture, so a value here
of 1.0 means "this vertex is fully lit" and nothing brighter exists.

The module deliberately has no Blender imports so the algorithm can be exercised
on its own. Shadow casting is injected as a `visible(origin, target)` callable.
"""

import math

from collections import namedtuple

# Danae.cpp: GLOBAL_LIGHT_FACTOR = 0.85f
GLOBAL_LIGHT_FACTOR = 0.85

# Danae.cpp: ACTIVEBKG->ambient.r/g/b = 0.09f
DEFAULT_AMBIENT = (0.09, 0.09, 0.09)

# DANAE culls a light against a polygon using its centre plus this slack before
# testing individual vertices (EERIE_LIGHT_Apply).
LIGHT_CULL_MARGIN = 100.0

POLY_TRANS = 1 << 2
POLY_WATER = 1 << 3
POLY_IGNORE = 1 << 5
POLY_QUAD = 1 << 6

#: pos/rgb are 3-tuples, rgb in 0..1. `casts_shadow` is False for lights the
#: original flagged EXTRAS_NOCASTED, which skip the ray launch step.
DanaeLight = namedtuple('DanaeLight',
                        ['pos', 'rgb', 'intensity', 'fallstart', 'fallend', 'casts_shadow'])

#: vertices/normals are lists of 3-tuples in FTS order, centre is a 3-tuple,
#: `ignore` mirrors POLY_IGNORE, which DANAE skips entirely.
DanaePolygon = namedtuple('DanaePolygon', ['vertices', 'normals', 'center', 'ignore'])


# ARX_PrepareBackgroundNRMLs matches vertices whose coordinates agree to within
# this on every axis, and only merges normals whose tips are closer together than
# LittleAngularDiff allows, which works out as less than ninety degrees apart.
NORMAL_WELD_DISTANCE = 2.0
LITTLE_ANGULAR_DIFF = 1.41421


def _little_angular_diff(a, b):
    return ((a[0] - b[0]) ** 2 + (a[1] - b[1]) ** 2
            + (a[2] - b[2]) ** 2) < LITTLE_ANGULAR_DIFF ** 2


def prepare_vertex_normals(faces):
    """Recompute per vertex normals the way the DANAE editor did before baking.

    Port of ARX_PrepareBackgroundNRMLs in DANAE/ARX_Paths.cpp. Every vertex starts
    from its own polygon's face normal - the second normal for the fourth corner
    of a quad, the mean of the two for the middle corners - and then averages in
    the matching normal of every other polygon that shares that position, but only
    where the two point roughly the same way. That angle limit is what keeps a
    sharp edge sharp instead of smearing the two sides into each other.

    `faces` is a sequence of (vertices, norm, norm2) with vertices as 3 or 4
    position tuples. Returns one list of normals per face, in the same order.

    The result is deliberately left unnormalised, as in the original: the lighting
    kernel dots it against a unit vector without rescaling, so the averaging also
    dims vertices whose surrounding normals disagree.
    """

    def base_normal(norm, norm2, k, count_verts):
        if k == 3:
            return (norm2[0], norm2[1], norm2[2]), 1.0, 1.0
        if k > 0 and count_verts > 3:
            return (norm[0] + norm2[0], norm[1] + norm2[1], norm[2] + norm2[2]), 2.0, 0.5
        return (norm[0], norm[1], norm[2]), 1.0, 1.0

    # Bucket every vertex by position so the neighbour search is not quadratic.
    # DANAE swept a block of tiles around each polygon instead, which comes to the
    # same thing once the weld distance is only two units.
    buckets = {}
    for face_index, (vertices, _norm, _norm2) in enumerate(faces):
        for k, vertex in enumerate(vertices):
            key = (int(vertex[0] // NORMAL_WELD_DISTANCE),
                   int(vertex[1] // NORMAL_WELD_DISTANCE),
                   int(vertex[2] // NORMAL_WELD_DISTANCE))
            buckets.setdefault(key, []).append((face_index, k))

    neighbourhood = [(dx, dy, dz)
                     for dx in (-1, 0, 1) for dy in (-1, 0, 1) for dz in (-1, 0, 1)]

    result = []
    for face_index, (vertices, norm, norm2) in enumerate(faces):
        count_verts = len(vertices)
        normals = []

        for k, vertex in enumerate(vertices):
            accumulated, count, scale = base_normal(norm, norm2, k, count_verts)
            current = (accumulated[0] * scale, accumulated[1] * scale, accumulated[2] * scale)
            accumulated = list(accumulated)

            key = (int(vertex[0] // NORMAL_WELD_DISTANCE),
                   int(vertex[1] // NORMAL_WELD_DISTANCE),
                   int(vertex[2] // NORMAL_WELD_DISTANCE))

            for offset in neighbourhood:
                bucket = buckets.get((key[0] + offset[0], key[1] + offset[1],
                                      key[2] + offset[2]))
                if not bucket:
                    continue
                for other_index, k2 in bucket:
                    if other_index == face_index:
                        continue
                    other_vertices, other_norm, other_norm2 = faces[other_index]
                    other = other_vertices[k2]
                    if (abs(other[0] - vertex[0]) >= NORMAL_WELD_DISTANCE
                            or abs(other[1] - vertex[1]) >= NORMAL_WELD_DISTANCE
                            or abs(other[2] - vertex[2]) >= NORMAL_WELD_DISTANCE):
                        continue

                    if k2 == 3:
                        if _little_angular_diff(current, other_norm2):
                            for axis in range(3):
                                accumulated[axis] += other_norm2[axis] + current[axis]
                            count += 2.0
                    elif k2 > 0 and len(other_vertices) > 3:
                        merged = tuple((other_norm[axis] + other_norm2[axis]) * 0.5
                                       for axis in range(3))
                        if _little_angular_diff(current, merged):
                            for axis in range(3):
                                accumulated[axis] += merged[axis] * 2.0
                            count += 2.0
                    else:
                        if _little_angular_diff(current, other_norm):
                            for axis in range(3):
                                accumulated[axis] += other_norm[axis]
                            count += 1.0

            inverse = 1.0 / count
            normals.append((accumulated[0] * inverse, accumulated[1] * inverse,
                            accumulated[2] * inverse))

        result.append(normals)

    return result


def _distance(a, b):
    return math.sqrt((a[0] - b[0]) ** 2 + (a[1] - b[1]) ** 2 + (a[2] - b[2]) ** 2)


def accumulate_light(polygon, light, use_normals, visible, red, green, blue, index=None):
    """One light over one polygon, adding into the red/green/blue accumulators.

    Straight port of ARX_EERIE_LIGHT_Make.
    """

    if polygon.ignore:
        return

    falldiff = light.fallend - light.fallstart
    if falldiff <= 0.0:
        # Degenerate light. The original precomputes 1/falldiff unconditionally,
        # which would divide by zero here, so treat the falloff band as absent
        # and let every vertex inside fallend take the full intensity.
        falldiffmul = None
    else:
        falldiffmul = 1.0 / falldiff

    for i, vertex in enumerate(polygon.vertices):

        distance = _distance(light.pos, vertex)
        if distance >= light.fallend:
            continue

        res = 1.0

        if use_normals:
            # MODE_NORMALS: lambert term against the interpolated vertex normal.
            dx = light.pos[0] - vertex[0]
            dy = light.pos[1] - vertex[1]
            dz = light.pos[2] - vertex[2]
            length = math.sqrt(dx * dx + dy * dy + dz * dz)
            if length == 0.0:
                continue
            normal = polygon.normals[i]
            res = (dx * normal[0] + dy * normal[1] + dz * normal[2]) / length
            if res < 0.0:
                res = 0.0

        if visible is not None and light.casts_shadow:
            # MODE_RAYLAUNCH: shadow ray from the light to the vertex, exactly as
            # Visible() in EERIE/EERIEPoly.cpp does it. That function keeps the
            # nearest thing the ray meets over its full length and then forgives
            # it if it is the polygon being lit, which is what stops a surface
            # shadowing itself. The ray is never shortened and the target is never
            # nudged: a vertex sits on its own polygon, so the nearest hit is
            # normally that polygon and the vertex is lit.
            if res > 0.0 and not visible(light.pos, vertex, index):
                res = 0.0

        if res <= 0.0:
            continue

        contribution = light.intensity * res * GLOBAL_LIGHT_FACTOR

        if distance > light.fallstart and falldiffmul is not None:
            contribution *= (falldiff - (distance - light.fallstart)) * falldiffmul

        red[i] += light.rgb[0] * contribution
        green[i] += light.rgb[1] * contribution
        blue[i] += light.rgb[2] * contribution


def light_polygon(polygon, lights, ambient=DEFAULT_AMBIENT, use_normals=True, visible=None,
                  index=None):
    """All lights over one polygon, returning one (r, g, b, a) tuple per vertex.

    Straight port of EERIE_LIGHT_Apply: accumulate in linear space, clamp the
    top at 1.0, raise anything below the ambient floor up to it, scale to bytes.
    """

    count = len(polygon.vertices)
    red = [0.0] * count
    green = [0.0] * count
    blue = [0.0] * count

    if not polygon.ignore:
        for light in lights:
            if _distance(light.pos, polygon.center) < light.fallend + LIGHT_CULL_MARGIN:
                accumulate_light(polygon, light, use_normals, visible, red, green, blue,
                                 index)

    colors = []
    for i in range(count):
        channels = []
        for value, floor in ((red[i], ambient[0]), (green[i], ambient[1]), (blue[i], ambient[2])):
            if value > 1.0:
                value = 1.0
            elif value < floor:
                value = floor
            # Round rather than truncate. The shipped lightmaps bottom out at
            # exactly (23, 23, 23) and the ambient floor is 0.09, so 0.09 * 255 =
            # 22.95 has to land on 23. Truncating puts every value one step dark.
            channels.append(int(value * 255.0 + 0.5))
        colors.append((channels[0], channels[1], channels[2], 255))

    return colors


def compute_vertex_colors(polygons, lights, ambient=DEFAULT_AMBIENT, use_normals=True,
                          visible=None, progress=None):
    """Bake `polygons` against `lights`, flattened into one colour per vertex.

    The result is in the order the polygons are given, which is the order the
    .llf file stores and the engine reads back.
    """

    # ponytail: plain O(polygons x lights) scan with a per-polygon distance
    # cull, same as the original. Bucket the lights into the tile grid if bake
    # times on large levels start to hurt.
    colors = []
    for index, polygon in enumerate(polygons):
        colors.extend(light_polygon(polygon, lights, ambient, use_normals, visible, index))
        if progress is not None and index % 2000 == 0:
            progress(index, len(polygons))

    return colors
