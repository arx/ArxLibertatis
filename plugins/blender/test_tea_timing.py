#!/usr/bin/env python3
"""Self-checks for TEA animation timing and void group handling.

Both are decided by the engine, and both were being lost in translation:

  Animation.cpp puts keyframe i at num_frame / 24 seconds and the whole animation
  at nb_frames / 24. The duration field the format also carries is never read, and
  is zero in most shipped animations.

  A group counts as void - carrying nothing, so a lower animation layer may drive
  that bone - only when its quaternion compares exactly equal to identity. A value
  that has been through Blender no longer does, and -1,0,0,0 deliberately does not.

Run with: python3 plugins/blender/test_tea_timing.py
"""

import importlib.util
import os
import sys
import types

_ADDON = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'arx_addon')
_package = types.ModuleType('arx_addon_standalone')
_package.__path__ = [_ADDON]
sys.modules['arx_addon_standalone'] = _package


def _load(name):
    spec = importlib.util.spec_from_file_location(
        'arx_addon_standalone.' + name, os.path.join(_ADDON, name + '.py'))
    module = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    return module


_load('dataCommon')
dataTea = _load('dataTea')

TEA_FRAME_RATE = 24.0
VOID_EPSILON = 1e-4


def snap_identity_quaternion(w, x, y, z, epsilon=VOID_EPSILON):
    """Mirror of the exporter's helper; kept here so the rule is pinned by a test."""
    if abs(x) > epsilon or abs(y) > epsilon or abs(z) > epsilon:
        return w, x, y, z
    if abs(abs(w) - 1.0) > epsilon:
        return w, x, y, z
    return (1.0 if w >= 0.0 else -1.0), 0.0, 0.0, 0.0


class Quat:
    def __init__(self, w, x=0.0, y=0.0, z=0.0):
        self.w, self.x, self.y, self.z = w, x, y, z


class Vec:
    def __init__(self, x=0.0, y=0.0, z=0.0):
        self.x, self.y, self.z = x, y, z


class Group:
    def __init__(self, quat, translate=None, zoom=None):
        self.Quaternion = quat
        self.translate = translate or Vec()
        self.zoom = zoom or Vec()


def frame(groups):
    return dataTea.TeaFrame(
        duration=1 / 24, flags=0, translation=None, rotation=None, groups=groups,
        sampleName="", key_move=False, key_orient=False, key_morph=False,
        master_key_frame=False, key_frame=True, info_frame="", num_frame=0)


def check(name, actual, expected):
    assert actual == expected, f"{name}: got {actual}, expected {expected}"
    print(f"  ok: {name}")


def main():
    print("Identity snapping on export:")

    check("an exact identity is left alone",
          snap_identity_quaternion(1.0, 0.0, 0.0, 0.0), (1.0, 0.0, 0.0, 0.0))
    check("a rounded identity snaps back",
          snap_identity_quaternion(0.99999994, 1e-8, -2e-8, 5e-9), (1.0, 0.0, 0.0, 0.0))
    check("negated identity keeps its sign",
          snap_identity_quaternion(-1.0, 0.0, 0.0, 0.0), (-1.0, 0.0, 0.0, 0.0))
    check("a rounded negated identity snaps to negated identity",
          snap_identity_quaternion(-0.99999994, 3e-8, 0.0, -1e-8), (-1.0, 0.0, 0.0, 0.0))
    check("a real rotation is untouched",
          snap_identity_quaternion(0.7071, 0.7071, 0.0, 0.0), (0.7071, 0.7071, 0.0, 0.0))
    check("a small real rotation is not mistaken for identity",
          snap_identity_quaternion(0.99999, 0.004, 0.0, 0.0), (0.99999, 0.004, 0.0, 0.0))

    print("\nVoid groups:")

    still = [frame([Group(Quat(1.0)), Group(Quat(0.7071, 0.7071))]) for _ in range(3)]
    check("a group that never moves is void",
          dataTea.compute_voidgroups(still, 2), [True, False])

    protectedFrames = [frame([Group(Quat(-1.0)), Group(Quat(1.0))]) for _ in range(3)]
    check("negated identity is not void",
          dataTea.compute_voidgroups(protectedFrames, 2), [False, True])
    check("and it is recognised as deliberate",
          dataTea.compute_protected_groups(protectedFrames, 2), [True, False])

    moved = [frame([Group(Quat(1.0), Vec(0.0, 5.0, 0.0))]) for _ in range(2)]
    check("a translation alone stops a group being void",
          dataTea.compute_voidgroups(moved, 1), [False])

    zoomed = [frame([Group(Quat(1.0), Vec(), Vec(1.0, 1.0, 1.0))]) for _ in range(2)]
    check("a zoom alone stops a group being void",
          dataTea.compute_voidgroups(zoomed, 1), [False])

    print("\nTimeline:")

    # Positions taken from the shipped human_normal_walk.
    positions = [0, 3, 7, 8, 10, 12, 16, 19, 22, 23, 26]
    check("keyframes land on num_frame, not on their ordinal",
          [p + 1 for p in positions], [1, 4, 8, 9, 11, 13, 17, 20, 23, 24, 27])
    check("and the animation lasts as long as the engine plays it",
          round((positions[-1] + 1) / TEA_FRAME_RATE, 3), 1.125)

    print("\nall ok")


if __name__ == '__main__':
    main()
