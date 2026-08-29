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

"""Everything a mesh needs before it can be exported as an FTL, in one panel.

The FTL exporter refuses a model for half a dozen different reasons and does two
more things wrong in silence, and until now the only way to know which was to
export and read the error - or to not get an error and find out in the game. All
of it is checkable up front, so this lists the checks with a button next to each.

The armature is the odd one out and worth explaining. Arx does not store a bone
hierarchy: FTL keeps a vertex list per group and an origin vertex, and the engine
rebuilds the parenting in getFatherIndex by asking which earlier group holds this
group's origin. So the armature is ours - it is what an animator poses - but it
is also the only place the exporter can learn the parenting it needs to make that
rebuild work, by pushing each bone's origin into its parent's vertex list. Hence
a button to build one from the vertex groups, rather than importing some other
model to borrow its skeleton.
"""

import os

import bmesh
import bpy
from bpy.props import EnumProperty, StringProperty
from bpy.types import Operator, Panel
from bpy_extras.io_utils import ImportHelper
from mathutils import Vector

from .arx_io_util import ArxException

GROUP_PREFIX = 'grp:'
ORIGIN_PREFIX = 'origin:'
ACTION_BONE_PROPERTY = 'arx_action_group'
#: Cosmetic only. Arx bones have no length, just an origin.
BONE_LENGTH = 0.05


def group_index_of(name):
    """The FTL group number out of a 'grp:NN:whatever' or 'origin:NN:whatever'."""
    parts = name.split(':', 2)
    if len(parts) < 3:
        return None
    try:
        return int(parts[1])
    except ValueError:
        return None


def bone_groups(obj):
    """{group number: vertex group}, for the groups that name a bone."""
    found = {}
    for group in obj.vertex_groups:
        if group.name.startswith(GROUP_PREFIX):
            index = group_index_of(group.name)
            if index is not None:
                found[index] = group
    return found


def child_empties(obj):
    return [child for child in bpy.data.objects
            if child.type == 'EMPTY' and child.parent == obj]


def origin_empties(obj):
    """{group number: empty}, the empties that mark where each bone pivots."""
    found = {}
    for empty in child_empties(obj):
        if empty.name.startswith(ORIGIN_PREFIX):
            index = group_index_of(empty.name)
            if index is not None:
                found[index] = empty
    return found


def action_empties(obj):
    """The attach points - everything that is not a bone origin."""
    return [empty for empty in child_empties(obj)
            if not empty.name.startswith(ORIGIN_PREFIX)]


def group_members(obj):
    """{vertex group index: [vertex indices]}."""
    members = {group.index: [] for group in obj.vertex_groups}
    for vertex in obj.data.vertices:
        for entry in vertex.groups:
            members.setdefault(entry.group, []).append(vertex.index)
    return members


def armature_of(obj):
    for modifier in obj.modifiers:
        if modifier.type == 'ARMATURE' and modifier.object:
            return modifier.object
    return None


def empty_position(obj, empty):
    """Where an empty actually is, in the mesh's own space.

    Not empty.location: once an empty is vertex parented that is an offset from
    the parent vertex, usually near zero. Reading it as a position sends every
    search for "the nearest vertex" to the model's origin, so re-running any of
    this over an already set up model quietly rewires the rig.
    """
    return obj.matrix_world.inverted() @ empty.matrix_world.translation


def unresolved_textures(obj):
    """Material names whose texture is not in the data directory.

    The exporter turns a material called foo-mat into
    GRAPH\\OBJ3D\\TEXTURES\\foo.FOO by stripping the last four characters, and
    the engine drops the extension and looks the rest up. Nothing checks that the
    file is there - the model loads and renders untextured. Renaming a material
    to satisfy the -mat rule is exactly how you end up here, because the name it
    leaves behind is the mesh's old material name rather than a texture's.
    """
    from .managers import getAddon

    try:
        root = getAddon(bpy.context).objectManager.dataPath
    except Exception:
        return []
    if not root:
        return []

    textures = os.path.join(root, 'graph', 'obj3d', 'textures')
    try:
        available = {os.path.splitext(name)[0].lower() for name in os.listdir(textures)}
    except OSError:
        return []

    missing = []
    for slot in obj.material_slots:
        if not slot.material or not slot.material.name.endswith('-mat'):
            continue
        stem = slot.material.name[:-4]
        if stem.lower() not in available:
            missing.append(stem)
    return sorted(set(missing))


def setup_checks(obj):
    """Every reason the exporter would reject or quietly mangle this mesh.

    Returns (key, label, ok, detail) per check, in the order they are worth
    fixing - the cheap mechanical ones first, the ones needing a decision last.
    """
    checks = []

    moved = (obj.location.length > 1e-6
             or any(abs(angle) > 1e-6 for angle in obj.rotation_euler)
             or any(abs(axis - 1.0) > 1e-6 for axis in obj.scale))
    checks.append(('transform', "Transform applied", not moved,
                   "FTL has no object transform" if moved else ""))

    mesh = obj.data
    layers = [name for name in ('arx_facetype', 'arx_transval')
              if mesh.attributes.get(name) is None]
    checks.append(('facedata', "Face data layers", not layers,
                   "missing " + ", ".join(layers) if layers else ""))

    ngons = sum(1 for face in mesh.polygons if len(face.vertices) > 3)
    checks.append(('triangles', "Triangles only", ngons == 0,
                   f"{ngons} faces with more than 3 vertices" if ngons else ""))

    # validate_mesh walks every object in the file, not just this one.
    bad_materials = sorted({slot.material.name
                            for other in bpy.data.objects
                            for slot in other.material_slots
                            if slot.material and not slot.material.name.endswith('-mat')})
    checks.append(('materials', "Material names end in -mat", not bad_materials,
                   ", ".join(bad_materials[:3]) + ("..." if len(bad_materials) > 3 else "")))

    missing_textures = unresolved_textures(obj)
    checks.append(('textures', "Textures exist", not missing_textures,
                   ", ".join(missing_textures[:3])
                   + ("..." if len(missing_textures) > 3 else "")))

    groups = bone_groups(obj)
    checks.append(('groups', "Bone vertex groups", bool(groups),
                   f"{len(groups)} groups" if groups else "no grp:NN:name groups"))

    origins = origin_empties(obj)
    unparented = sorted(index for index, empty in origins.items()
                        if empty.parent_type != 'VERTEX')
    missing = sorted(set(groups) - set(origins))
    origins_ok = bool(origins) and not unparented and not missing
    detail = ""
    if missing:
        detail = f"{len(missing)} groups have no origin empty"
    elif unparented:
        detail = f"{len(unparented)} of {len(origins)} not vertex parented"
    elif not origins:
        detail = "no origin:NN:name empties"
    checks.append(('origins', "Bone origins on vertices", origins_ok, detail))

    # A rig is not just present or absent. Deriving the hierarchy from a mesh
    # whose groups do not nest leaves every bone a root, which exports as a flat
    # skeleton and animates as nonsense - so count the roots rather than let a
    # useless armature tick the box.
    armature = armature_of(obj)
    if armature is None:
        detail, armature_ok = "no hierarchy to export parenting from", False
    else:
        bones = [bone for bone in armature.data.bones
                 if group_index_of(bone.name) is not None]
        roots = [bone for bone in bones if bone.parent is None]
        armature_ok = len(bones) > 0 and len(roots) == 1
        detail = (f"{armature.name}, {len(bones)} bones" if armature_ok
                  else f"{armature.name}: {len(roots)} of {len(bones)} bones have no parent")
    checks.append(('armature', "Armature", armature_ok, detail))

    actions = action_empties(obj)
    unassigned = [empty for empty in actions if ACTION_BONE_PROPERTY not in empty]
    checks.append(('actions', "Attach points assigned", actions and not unassigned,
                   f"{len(unassigned)} of {len(actions)} unassigned" if unassigned
                   else ("no attach empties" if not actions else "")))

    return checks


def active_arx_mesh(context):
    obj = context.active_object
    return obj if obj is not None and obj.type == 'MESH' else None


class ARX_OT_model_apply_transform(Operator):
    bl_idname = "arx.model_apply_transform"
    bl_label = "Apply Transform"
    bl_description = ("Bake location, rotation and scale into the vertices. FTL stores "
                      "no object transform, so the exporter refuses a moved model")
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        obj = active_arx_mesh(context)
        if not obj:
            self.report({'ERROR'}, "No active mesh")
            return {'CANCELLED'}
        # transform_apply polls for object mode, and a rigged model is usually
        # being looked at in weight paint or edit mode when you think to export.
        if obj.mode != 'OBJECT':
            bpy.ops.object.mode_set(mode='OBJECT')
        bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
        return {'FINISHED'}


class ARX_OT_model_triangulate(Operator):
    bl_idname = "arx.model_triangulate"
    bl_label = "Triangulate"
    bl_description = "Split every face with more than three vertices, which FTL cannot store"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        obj = active_arx_mesh(context)
        if not obj:
            self.report({'ERROR'}, "No active mesh")
            return {'CANCELLED'}
        bm = bmesh.new()
        bm.from_mesh(obj.data)
        faces = [face for face in bm.faces if len(face.verts) > 3]
        if faces:
            bmesh.ops.triangulate(bm, faces=faces)
        bm.to_mesh(obj.data)
        bm.free()
        obj.data.update()
        self.report({'INFO'}, f"Triangulated {len(faces)} faces")
        return {'FINISHED'}


class ARX_OT_model_fix_materials(Operator):
    bl_idname = "arx.model_fix_materials"
    bl_label = "Fix Material Names"
    bl_description = ("Append -mat to material names that lack it. The exporter strips "
                      "those four characters to build the texture path")
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        renamed = 0
        for obj in bpy.data.objects:
            for slot in obj.material_slots:
                if slot.material and not slot.material.name.endswith('-mat'):
                    slot.material.name += '-mat'
                    renamed += 1
        self.report({'INFO'}, f"Renamed {renamed} materials")
        return {'FINISHED'}


class ARX_OT_model_snap_origins(Operator):
    bl_idname = "arx.model_snap_origins"
    bl_label = "Snap Origins To Vertices"
    bl_description = ("Vertex parent every origin:NN:name empty to the nearest vertex of "
                      "its group. An Arx bone is a vertex index and nothing else")
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        obj = active_arx_mesh(context)
        if not obj:
            self.report({'ERROR'}, "No active mesh")
            return {'CANCELLED'}

        groups = bone_groups(obj)
        origins = origin_empties(obj)
        members = group_members(obj)
        verts = obj.data.vertices
        armature = armature_of(obj)
        parents = armature_hierarchy(armature) if armature else {}

        # Which bones hold each vertex. Weight painting makes groups overlap, and
        # once they do the choice of origin is what decides the exported
        # hierarchy: getFatherIndex walks back from a bone and stops at the first
        # group holding its origin, so the origin must not be held by anything
        # numbered between the bone and its parent. The parent itself needs no
        # check - the exporter appends every child origin to its parent's list.
        holders = {}
        for index, group in groups.items():
            for vertex in members.get(group.index, []):
                holders.setdefault(vertex, set()).add(index)

        used = set()
        snapped = 0
        for index in sorted(groups):
            empty = origins.get(index)
            if empty is None:
                continue
            parent = parents.get(index, -1)

            def clear(vertex):
                if vertex in used:
                    return False
                return not any(parent < other < index
                               for other in holders.get(vertex, ()))

            # This bone's own geometry first; a bone with no weight of its own
            # borrows from anywhere that satisfies the same rule.
            candidates = [v for v in members.get(groups[index].index, []) if clear(v)]
            if not candidates:
                candidates = [v.index for v in verts if clear(v.index)]
            if not candidates:
                candidates = [v.index for v in verts if v.index not in used]
            if not candidates:
                continue

            target = empty_position(obj, empty)
            nearest = min(candidates,
                          key=lambda i: (verts[i].co - target).length_squared)
            used.add(nearest)
            empty.parent = obj
            empty.parent_type = 'VERTEX'
            empty.parent_vertices = [nearest, 0, 0]
            snapped += 1

        self.report({'INFO'}, f"Vertex parented {snapped} bone origins")
        return {'FINISHED'}


def armature_hierarchy(armature):
    """{group number: parent group number} out of an armature's bone parenting."""
    parents = {}
    for bone in armature.data.bones:
        index = group_index_of(bone.name)
        if index is None:
            continue
        parent = group_index_of(bone.parent.name) if bone.parent else None
        parents[index] = parent if parent is not None else -1
    return parents


class ARX_OT_model_build_armature(Operator, ImportHelper):
    bl_idname = "arx.model_build_armature"
    bl_label = "Build Armature From Groups"
    bl_description = ("Create an armature from the grp:NN:name vertex groups and bind it. "
                      "Optionally copy the bone parenting from an existing .ftl whose "
                      "groups match")
    bl_options = {'REGISTER', 'UNDO'}

    filename_ext = ".ftl"
    filter_glob: StringProperty(default="*.ftl", options={'HIDDEN'})

    hierarchy: EnumProperty(
        name="Hierarchy",
        items=[('DERIVE', "From this mesh",
                "Parent each bone to the deepest other group that already contains its "
                "origin vertex, the same rule the engine uses"),
               ('FTL', "From an .ftl file",
                "Copy the parenting of a model whose groups match, by group number")],
        default='DERIVE')

    def invoke(self, context, event):
        # The panel sets hierarchy on the button, so it is already decided by the
        # time this runs - which is the whole point. Opening the browser on a
        # property the user has not been shown yet would just run DERIVE.
        if self.hierarchy == 'FTL':
            return ImportHelper.invoke(self, context, event)
        return self.execute(context)

    def execute(self, context):
        obj = active_arx_mesh(context)
        if not obj:
            self.report({'ERROR'}, "No active mesh")
            return {'CANCELLED'}

        if self.hierarchy == 'FTL' and not self.filepath:
            self.report({'ERROR'}, "No reference .ftl chosen")
            return {'CANCELLED'}

        groups = bone_groups(obj)
        if not groups:
            self.report({'ERROR'}, "No grp:NN:name vertex groups to build bones from")
            return {'CANCELLED'}

        origins = origin_empties(obj)
        members = group_members(obj)

        attached = 0
        try:
            if self.hierarchy == 'FTL':
                parents, attach = self.rig_from_ftl(self.filepath, groups)
                attached = self.apply_attach_bones(obj, attach)
            else:
                parents = self.hierarchy_from_mesh(obj, groups, origins, members)
        except ArxException as error:
            self.report({'ERROR'}, str(error))
            return {'CANCELLED'}

        armature = self.create(context, obj, groups, origins, members, parents)
        rooted = sum(1 for index in groups if parents.get(index, -1) < 0)
        message = f"Built {len(groups)} bones in '{armature.name}', {rooted} without a parent"
        if attached:
            message += f", {attached} attach points assigned"
        self.report({'INFO'}, message)
        return {'FINISHED'}

    def rig_from_ftl(self, path, groups):
        """Bone parenting and attach point bones of a matching model.

        Both are read from the same file for the same reason: a model whose
        group list is a copy of another's wants that model's rig, and neither
        piece can be worked out from the mesh alone. The parenting is not stored
        in the FTL at all - FtlSerializer.read recovers it with getFatherIndex,
        exactly as the engine does - and an attach point's bone is whichever
        group holds its vertex, which is a decision the original author made.
        """
        from .dataFtl import FtlSerializer
        from .managers import getAddon

        data = open(path, 'rb').read()
        if data[:3] != b'FTL':
            data = getAddon(bpy.context).objectManager.ioLib.unpack(data)
        source = FtlSerializer().read(data)
        if len(source.groups) < len(groups):
            raise ArxException(f"{path} has {len(source.groups)} groups, "
                               f"this mesh has {len(groups)}")

        parents = {index: source.groups[index].parentIndex
                   for index in groups if index < len(source.groups)}

        owners = {}
        for index, group in enumerate(source.groups):
            for vertex in group.indices:
                owners.setdefault(vertex, []).append(index)
        # getGroupForVertex scans in reverse, so the engine uses the highest.
        attach = {action.name.upper(): max(owners[action.vidx])
                  for action in source.actions if owners.get(action.vidx)}

        return parents, attach

    def apply_attach_bones(self, obj, attach):
        assigned = 0
        for empty in action_empties(obj):
            bone = attach.get(empty.name.split('.')[0].upper())
            if bone is not None:
                empty[ACTION_BONE_PROPERTY] = bone
                assigned += 1
        return assigned

    def hierarchy_from_mesh(self, obj, groups, origins, members):
        """The engine's own rule: the deepest earlier group holding this origin.

        Works on a model whose groups nest, which is how the shipped ones are
        built. On a mesh whose groups are disjoint it finds nothing and every
        bone comes out a root - that is honest rather than a guess, and the
        parenting can be drawn in Edit Mode or copied from an .ftl instead.
        """
        owners = {}
        for index, group in groups.items():
            for vertex in members.get(group.index, []):
                owners.setdefault(vertex, []).append(index)

        parents = {}
        for index in groups:
            empty = origins.get(index)
            origin = (empty.parent_vertices[0]
                      if empty is not None and empty.parent_type == 'VERTEX' else None)
            holders = [other for other in owners.get(origin, []) if other < index]
            parents[index] = max(holders) if holders else -1
        return parents

    def create(self, context, obj, groups, origins, members, parents):
        verts = obj.data.vertices

        def head_of(index):
            empty = origins.get(index)
            if empty is not None:
                if empty.parent_type == 'VERTEX':
                    return verts[empty.parent_vertices[0]].co.copy()
                return empty_position(obj, empty)
            owned = members.get(groups[index].index, [])
            if owned:
                total = Vector((0.0, 0.0, 0.0))
                for vertex in owned:
                    total += verts[vertex].co
                return total / len(owned)
            return Vector((0.0, 0.0, 0.0))

        # Drop the old rig before naming the new one, or Blender hands out
        # cj-amt.001 and every rebuild leaves another orphan behind.
        existing = armature_of(obj)
        if existing:
            bpy.data.objects.remove(existing)

        data = bpy.data.armatures.new(obj.name + '-amt')
        armature = bpy.data.objects.new(obj.name + '-amt', data)
        context.scene.collection.objects.link(armature)

        previous = context.view_layer.objects.active
        context.view_layer.objects.active = armature
        bpy.ops.object.mode_set(mode='EDIT')
        bones = {}
        for index in sorted(groups):
            bone = data.edit_bones.new(groups[index].name)
            bone.head = head_of(index)
            bone.tail = bone.head + Vector((0.0, 0.0, BONE_LENGTH))
            bones[index] = bone
        for index, bone in bones.items():
            parent = parents.get(index, -1)
            if parent is not None and parent >= 0 and parent in bones and parent != index:
                bone.parent = bones[parent]
        bpy.ops.object.mode_set(mode='OBJECT')
        context.view_layer.objects.active = previous

        modifier = obj.modifiers.new(type='ARMATURE', name='Skeleton')
        modifier.object = armature
        return armature


class ARX_OT_model_assign_attach(Operator):
    bl_idname = "arx.model_assign_attach"
    bl_label = "Assign"
    bl_description = ("Set which bone drives this attach point. A linked object is "
                      "rotated by that bone's animation quaternion and nothing else, so "
                      "the wrong bone hangs a weapon at the wrong angle")
    bl_options = {'REGISTER', 'UNDO'}

    empty: StringProperty()

    def execute(self, context):
        obj = active_arx_mesh(context)
        empty = bpy.data.objects.get(self.empty)
        if not obj or not empty:
            self.report({'ERROR'}, "No attach point to assign")
            return {'CANCELLED'}

        verts = obj.data.vertices
        if not verts:
            self.report({'ERROR'}, "Mesh has no vertices")
            return {'CANCELLED'}

        # Nearest vertex is right for anything held in a hand and wrong for
        # anything hanging off a bone that carries no geometry, so this is a
        # starting value to correct rather than an answer.
        target = empty_position(obj, empty)
        nearest = min(range(len(verts)),
                      key=lambda i: (verts[i].co - target).length_squared)
        deepest = None
        for entry in verts[nearest].groups:
            index = group_index_of(obj.vertex_groups[entry.group].name)
            if index is not None and (deepest is None or index > deepest):
                deepest = index
        if deepest is None:
            self.report({'ERROR'}, "Nearest vertex belongs to no bone group")
            return {'CANCELLED'}

        empty[ACTION_BONE_PROPERTY] = deepest
        self.report({'INFO'}, f"{empty.name} driven by group {deepest}")
        return {'FINISHED'}


class ARX_PT_model_setup(Panel):
    bl_idname = "VIEW3D_PT_arx_model_setup"
    bl_label = "Arx Model Setup"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = "Arx"

    FIXES = {
        'transform': ("arx.model_apply_transform", "Apply Transform"),
        'facedata': ("arx.mesh_add_custom_properties", "Add Face Data"),
        'triangles': ("arx.model_triangulate", "Triangulate"),
        'materials': ("arx.model_fix_materials", "Fix Material Names"),
        'origins': ("arx.model_snap_origins", "Snap Origins To Vertices"),
    }

    def draw(self, context):
        layout = self.layout
        obj = active_arx_mesh(context)
        if not obj:
            layout.label(text="Select a mesh", icon='INFO')
            return

        layout.label(text=obj.name, icon='OUTLINER_OB_MESH')

        checks = setup_checks(obj)
        for key, label, ok, detail in checks:
            box = layout.box()
            row = box.row()
            row.label(text=label, icon='CHECKMARK' if ok else 'ERROR')
            if detail:
                row.label(text=detail)
            if key == 'armature':
                # Two buttons rather than one with a mode: an operator's
                # properties are fixed before invoke runs, so a single button
                # can only ever offer whichever mode is the default, and the
                # file browser for the other one would never open.
                row = box.row(align=True)
                row.operator("arx.model_build_armature",
                             text="Build From Groups").hierarchy = 'DERIVE'
                row.operator("arx.model_build_armature",
                             text="Copy Rig From .ftl").hierarchy = 'FTL'
                continue
            fix = self.FIXES.get(key)
            if fix and not ok:
                box.operator(fix[0], text=fix[1])

        actions = action_empties(obj)
        if actions:
            box = layout.box()
            box.label(text="Attach points", icon='EMPTY_ARROWS')
            groups = bone_groups(obj)
            for empty in sorted(actions, key=lambda item: item.name):
                row = box.row(align=True)
                row.label(text=empty.name)
                if ACTION_BONE_PROPERTY in empty:
                    index = empty[ACTION_BONE_PROPERTY]
                    group = groups.get(index)
                    row.prop(empty, f'["{ACTION_BONE_PROPERTY}"]', text="")
                    row.label(text=group.name.split(':', 2)[2] if group else "unknown")
                else:
                    row.operator("arx.model_assign_attach", text="Assign").empty = empty.name

        layout.separator()
        column = layout.column()
        column.enabled = all(ok for _key, _label, ok, _detail in checks)
        column.operator("arx.export_ftl", text="Export FTL", icon='EXPORT')


classes = (
    ARX_OT_model_apply_transform,
    ARX_OT_model_triangulate,
    ARX_OT_model_fix_materials,
    ARX_OT_model_snap_origins,
    ARX_OT_model_build_armature,
    ARX_OT_model_assign_attach,
    ARX_PT_model_setup,
)


def arx_ui_model_register():
    for cls in classes:
        bpy.utils.register_class(cls)


def arx_ui_model_unregister():
    for cls in reversed(classes):
        bpy.utils.unregister_class(cls)
