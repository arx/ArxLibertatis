# Copyright 2015-2020 Arx Libertatis Team (see the AUTHORS file)
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

"""Build a level's navigation mesh from its geometry.

Anchors are the pathfinding graph. An NPC under any moving behaviour asks
AnchorData_GetNearest (src/game/NPC.cpp) for a node to start from, and a level
with none has NPCs that stand still and log nothing about it.

DANAE generated them by dropping a cylinder on nine points of every cell and
growing its radius until it touched something, then linking the pairs it could
sweep a cylinder between - AnchorData_Create_Original_Method and
AnchorData_Create_Links_Original_Method in src/DANAE_OLD/EERIE/EERIEAnchors.cpp.
That ran inside the editor with the collision code to hand. This does the same
job against the polygon soup alone: sample the floor on a grid, drop the samples
that have no room for a person, and link the ones a person could walk between.

Everything here is in Arx world coordinates, where y points *down*: a floor's
normal has negative y, a ceiling is at a more negative y than the floor beneath
it, and a cylinder's height is negative.

The module deliberately has no Blender imports, so the same code serves the
addon's export and the standalone level generator, and so it can be exercised
without starting Blender - run it directly for a self check.
"""

import math

#: The engine will not use an anchor unless its height is at or below the NPC's
#: cylinder and its radius at or above it (AnchorData_GetNearest), and
#: EERIE_COLLISION_Cylinder_Create clamps a human-sized entity to exactly height
#: -165 and radius 40 or less. DANAE's own anchors came out at -165 and 40 to 50.
#: An anchor that is shallower or narrower than this admits nobody, silently.
ANCHOR_RADIUS = 45.0
ANCHOR_HEIGHT = -165.0
#: One sample per background cell, which is the density DANAE ended up at.
ANCHOR_SPACING = 100.0
#: DANAE's link cutoff. 200 reaches an orthogonal neighbour, a diagonal one and
#: the cell beyond it, so the graph stays connected when samples drop out.
ANCHOR_LINK_RANGE = 200.0
#: How far a walker can step up. Floor within this of a sample is the ground it
#: stands on rather than something blocking its head.
STEP_HEIGHT = 30.0
#: A polygon counts as floor when its normal is more up than sideways.
FLOOR_NORMAL_Y = -0.5
#: Cell size of the background grid, and so of the lookup used here.
CELL_SIZE = 100.0


def _polygon_faces(polygon):
    """Accept the shapes the callers already have: a dict with 'v' and 'norm',
    or anything exposing the same two attributes."""
    if isinstance(polygon, dict):
        return polygon['v'], polygon.get('norm'), bool(polygon.get('nopath', False))
    return polygon.v, getattr(polygon, 'norm', None), bool(getattr(polygon, 'nopath', False))


class _Poly:
    """A polygon reduced to what the sampling needs."""

    __slots__ = ('verts', 'floor', 'nopath', 'ymin', 'ymax', 'x0', 'x1', 'z0', 'z1',
                 'plane')

    def __init__(self, verts, normal, nopath):
        self.verts = verts
        self.nopath = nopath
        ys = [v[1] for v in verts]
        xs = [v[0] for v in verts]
        zs = [v[2] for v in verts]
        # ymin is the highest point, because y grows downwards.
        self.ymin, self.ymax = min(ys), max(ys)
        self.x0, self.x1 = min(xs), max(xs)
        self.z0, self.z1 = min(zs), max(zs)
        if normal is None:
            normal = _newell_normal(verts)
        self.floor = normal[1] <= FLOOR_NORMAL_Y
        # Plane through the first vertex, used to read a height off a floor.
        self.plane = (normal, normal[0] * verts[0][0] + normal[1] * verts[0][1]
                      + normal[2] * verts[0][2])


def _newell_normal(verts):
    nx = ny = nz = 0.0
    for i, a in enumerate(verts):
        b = verts[(i + 1) % len(verts)]
        nx += (a[1] - b[1]) * (a[2] + b[2])
        ny += (a[2] - b[2]) * (a[0] + b[0])
        nz += (a[0] - b[0]) * (a[1] + b[1])
    length = math.sqrt(nx * nx + ny * ny + nz * nz)
    if length < 1e-9:
        return (0.0, -1.0, 0.0)
    return (nx / length, ny / length, nz / length)


def _contains_xz(verts, x, z):
    """Ray crossing test in the XZ plane."""
    inside = False
    count = len(verts)
    for i in range(count):
        ax, _, az = verts[i]
        bx, _, bz = verts[(i + 1) % count]
        if (az > z) != (bz > z):
            t = (z - az) / (bz - az)
            if x < ax + t * (bx - ax):
                inside = not inside
    return inside


def _distance_xz(verts, x, z):
    """Distance from a point to a polygon, both flattened onto XZ."""
    if _contains_xz(verts, x, z):
        return 0.0
    best = float('inf')
    count = len(verts)
    for i in range(count):
        ax, _, az = verts[i]
        bx, _, bz = verts[(i + 1) % count]
        dx, dz = bx - ax, bz - az
        length = dx * dx + dz * dz
        if length < 1e-9:
            best = min(best, math.hypot(x - ax, z - az))
            continue
        t = max(0.0, min(1.0, ((x - ax) * dx + (z - az) * dz) / length))
        best = min(best, math.hypot(x - (ax + t * dx), z - (az + t * dz)))
    return best


def _height_on(poly, x, z):
    """Where the plane of a floor polygon sits above (x, z)."""
    (nx, ny, nz), d = poly.plane
    if abs(ny) < 1e-6:
        return None
    return (d - nx * x - nz * z) / ny


class _Grid:
    """Polygons bucketed by background cell, so a lookup touches a handful."""

    def __init__(self, polys):
        self.cells = {}
        self.floors = {}
        for poly in polys:
            for key in self._keys(poly):
                self.cells.setdefault(key, []).append(poly)
                if poly.floor:
                    self.floors.setdefault(key, []).append(poly)

    @staticmethod
    def _keys(poly):
        for cx in range(int(math.floor(poly.x0 / CELL_SIZE)),
                        int(math.floor(poly.x1 / CELL_SIZE)) + 1):
            for cz in range(int(math.floor(poly.z0 / CELL_SIZE)),
                            int(math.floor(poly.z1 / CELL_SIZE)) + 1):
                yield cx, cz

    def near(self, x, z, reach):
        """Every polygon whose cell is within reach of (x, z), without repeats."""
        seen = set()
        out = []
        for cx in range(int(math.floor((x - reach) / CELL_SIZE)),
                        int(math.floor((x + reach) / CELL_SIZE)) + 1):
            for cz in range(int(math.floor((z - reach) / CELL_SIZE)),
                            int(math.floor((z + reach) / CELL_SIZE)) + 1):
                for poly in self.cells.get((cx, cz), ()):
                    if id(poly) not in seen:
                        seen.add(id(poly))
                        out.append(poly)
        return out

    def floors_at(self, x, z):
        return self.floors.get((int(math.floor(x / CELL_SIZE)),
                                int(math.floor(z / CELL_SIZE))), ())


def _fits(grid, x, y, z, radius, height):
    """Whether a cylinder standing at (x, y, z) has room.

    Blocked by anything crossing the space the body occupies - the open span
    between the feet and the head. The ground itself is excluded by the step
    allowance, or a walker would be blocked by the floor it is standing on and
    no ramp would ever be walkable.
    """
    head = y + height  # height is negative, so this is above the feet
    for poly in grid.near(x, z, radius):
        if poly.ymax <= head or poly.ymin >= y:
            continue
        if poly.floor and poly.ymax > y - STEP_HEIGHT:
            continue
        if _distance_xz(poly.verts, x, z) < radius:
            return False
    return True


def build_anchors(polygons, spacing=ANCHOR_SPACING, radius=ANCHOR_RADIUS,
                  height=ANCHOR_HEIGHT, link_range=ANCHOR_LINK_RANGE,
                  progress=None):
    """Return the anchor graph for a level's polygons.

    `polygons` are in Arx world coordinates: either dicts with 'v' and 'norm'
    keys, or objects with matching attributes, optionally carrying 'nopath' for
    POLY_NOPATH. Each anchor comes back as a dict with 'pos', 'links', 'radius',
    'height' and 'flags', ready for FAST_ANCHOR_DATA.
    """
    polys = []
    for polygon in polygons:
        verts, normal, nopath = _polygon_faces(polygon)
        verts = [tuple(float(c) for c in v[:3]) for v in verts]
        if len(verts) >= 3:
            polys.append(_Poly(verts, normal, nopath))
    if not polys:
        return []

    grid = _Grid(polys)

    x0 = min(p.x0 for p in polys)
    x1 = max(p.x1 for p in polys)
    z0 = min(p.z0 for p in polys)
    z1 = max(p.z1 for p in polys)

    # Sample cell centres rather than corners: a corner sits on the seam between
    # four polygons, where the containment test is a coin toss.
    positions = []
    steps_x = max(1, int(math.ceil((x1 - x0) / spacing)))
    steps_z = max(1, int(math.ceil((z1 - z0) / spacing)))
    for ix in range(steps_x):
        x = x0 + (ix + 0.5) * spacing
        for iz in range(steps_z):
            z = z0 + (iz + 0.5) * spacing
            # A column can cross several floors - a walkway over a room - so keep
            # one candidate per distinct level rather than only the lowest.
            levels = []
            for poly in grid.floors_at(x, z):
                if poly.nopath or not _contains_xz(poly.verts, x, z):
                    continue
                y = _height_on(poly, x, z)
                if y is None or any(abs(y - other) < 1.0 for other in levels):
                    continue
                levels.append(y)
            for y in levels:
                if _fits(grid, x, y, z, radius, height):
                    positions.append((x, y, z))
        if progress:
            progress(ix + 1, steps_x)

    # Links. DANAE required the pair to be within range, not too steep, and a
    # cylinder to survive the sweep between them; sampling the segment is the
    # same test without the physics.
    buckets = {}
    for index, (x, _y, z) in enumerate(positions):
        buckets.setdefault((int(math.floor(x / link_range)),
                            int(math.floor(z / link_range))), []).append(index)

    links = [[] for _ in positions]
    for i, (ax, ay, az) in enumerate(positions):
        bx_cell, bz_cell = int(math.floor(ax / link_range)), int(math.floor(az / link_range))
        for cx in (bx_cell - 1, bx_cell, bx_cell + 1):
            for cz in (bz_cell - 1, bz_cell, bz_cell + 1):
                for j in buckets.get((cx, cz), ()):
                    if j <= i:
                        continue
                    bx, by, bz = positions[j]
                    flat = math.hypot(bx - ax, bz - az)
                    if flat < 5.0 or flat > link_range:
                        continue
                    # DANAE's slope limit: a link may not climb faster than it runs.
                    if abs(by - ay) > flat * 0.9:
                        continue
                    if _walkable(grid, (ax, ay, az), (bx, by, bz), radius, height):
                        links[i].append(j)
                        links[j].append(i)

    # AnchorData_GetNearest skips an anchor with no links, so an isolated one is
    # only a lie in the header count. Links are mutual, so dropping them never
    # dangles a reference - but the survivors have to be renumbered, because a
    # link is an index into the array that gets written.
    keep = [i for i, link in enumerate(links) if link]
    renumber = {old: new for new, old in enumerate(keep)}
    return [{'pos': positions[i], 'links': [renumber[j] for j in links[i]],
             'radius': radius, 'height': height, 'flags': 0}
            for i in keep]


def _walkable(grid, a, b, radius, height):
    """Sample the segment between two anchors, the way DANAE swept a cylinder."""
    flat = math.hypot(b[0] - a[0], b[2] - a[2])
    steps = max(2, int(math.ceil(flat / (radius * 0.5))))
    for step in range(1, steps):
        t = step / steps
        if not _fits(grid, a[0] + (b[0] - a[0]) * t, a[1] + (b[1] - a[1]) * t,
                     a[2] + (b[2] - a[2]) * t, radius, height):
            return False
    return True


def _demo():
    """Two rooms joined by a doorway, with a pillar in one of them.

    Checks the three things that actually matter: anchors only appear on the
    floor, they do not appear inside solid geometry, and the graph crosses the
    doorway - a navmesh that stops at the door looks fine until an NPC has to
    walk through it.
    """
    def quad(a, b, c, d, normal):
        return {'v': [a, b, c, d], 'norm': normal}

    polygons = []
    # 800 x 400 floor and ceiling.
    polygons.append(quad((0, 0, 0), (800, 0, 0), (800, 0, 400), (0, 0, 400),
                         (0.0, -1.0, 0.0)))
    polygons.append(quad((0, -300, 0), (800, -300, 0), (800, -300, 400), (0, -300, 400),
                         (0.0, 1.0, 0.0)))
    # Outer walls.
    for z in (0, 400):
        polygons.append(quad((0, 0, z), (800, 0, z), (800, -300, z), (0, -300, z),
                             (0.0, 0.0, 1.0)))
    for x in (0, 800):
        polygons.append(quad((x, 0, 0), (x, 0, 400), (x, -300, 400), (x, -300, 0),
                             (1.0, 0.0, 0.0)))
    # A dividing wall at x=400 with a 200 wide doorway around z=200.
    for z0, z1 in ((0, 100), (300, 400)):
        polygons.append(quad((400, 0, z0), (400, 0, z1), (400, -300, z1), (400, -300, z0),
                             (1.0, 0.0, 0.0)))
    # A solid pillar in the left room, 100 x 100 around (150, 150).
    for a, b in (((100, 100), (200, 100)), ((200, 100), (200, 200)),
                 ((200, 200), (100, 200)), ((100, 200), (100, 100))):
        polygons.append(quad((a[0], 0, a[1]), (b[0], 0, b[1]),
                             (b[0], -300, b[1]), (a[0], -300, a[1]), (1.0, 0.0, 0.0)))

    anchors = build_anchors(polygons)
    assert anchors, "no anchors on an open floor"

    for anchor in anchors:
        x, y, z = anchor['pos']
        assert abs(y) < 1e-6, f"anchor off the floor at y={y}"
        assert anchor['height'] == ANCHOR_HEIGHT and anchor['radius'] == ANCHOR_RADIUS
        assert not (100 <= x <= 200 and 100 <= z <= 200), \
            f"anchor {anchor['pos']} inside the pillar"
        for link in anchor['links']:
            assert 0 <= link < len(anchors), "link out of range"

    # Reachability: everything must be one graph, or the doorway is not linked.
    seen, stack = {0}, [0]
    while stack:
        for link in anchors[stack.pop()]['links']:
            if link not in seen:
                seen.add(link)
                stack.append(link)
    assert len(seen) == len(anchors), \
        f"graph is split: {len(seen)} of {len(anchors)} reachable"
    assert any(a['pos'][0] < 400 for a in anchors) and any(a['pos'][0] > 400 for a in anchors), \
        "only one room got anchors"

    print(f"ok: {len(anchors)} anchors, "
          f"{sum(len(a['links']) for a in anchors) // 2} links, one connected graph")


if __name__ == '__main__':
    _demo()
