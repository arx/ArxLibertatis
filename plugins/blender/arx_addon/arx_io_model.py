# Copyright 2019-2020 Arx Libertatis Team (see the AUTHORS file)
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

import os
import itertools
import logging
import math
import bpy
import bmesh
from mathutils import Vector

from .arx_io_material import createMaterial
from .arx_io_util import arx_pos_to_blender_for_model, blender_pos_to_arx, ArxException
from .files import splitPath
from .dataFtl import FtlMetadata, FtlVertex, FtlFace, FtlGroup, FtlSelection, FtlAction, FtlData, FtlSerializer

logging.basicConfig(level=logging.DEBUG)

#: Bone length in Blender units. Cosmetic only: Arx bones have no length,
#: just an origin, so this exists purely so they can be seen and picked.
BONE_DISPLAY_LENGTH = 4.0


class ArxObjectManager(object):
    def __init__(self, ioLib, dataPath):
        self.log = logging.getLogger(__name__)
        self.ioLib = ioLib
        self.dataPath = dataPath
        self.ftlSerializer = FtlSerializer()

    def validateAssetDirectory(self):
        if not os.path.isdir(self.dataPath):
            raise ArxException(f"Arx asset directory path [{self.dataPath}] is not a directory")
        if not os.path.exists(self.dataPath):
            raise ArxException(f"Arx asset directory path [{self.dataPath}] does not exist")

    def validateObjectName(self, name):
        if len(name.encode('utf-8')) > 63:
            raise ArxException(f"Name [{name}] too long to be used as blender object name")

    def analyzeFaceData(self, faceData):
        facesToDrop = []
        seenFaceVerts = {}
        for i, face in enumerate(faceData):
            for foo in itertools.permutations(face.vids):
                if foo in seenFaceVerts:
                    seen = seenFaceVerts[foo]
                    facesToDrop.append(seen)
                    break
            seenFaceVerts[face.vids] = i
        if facesToDrop:
            self.log.debug("Dropping faces: %s", facesToDrop)
        return facesToDrop

    def createBmesh(self, context, vertData, faceData) -> bmesh.types.BMesh:
        facesToDrop = self.analyzeFaceData(faceData)
        bm = bmesh.new()
        arxFaceType = bm.faces.layers.int.new('arx_facetype')
        arxTransVal = bm.faces.layers.float.new('arx_transval')
        uvData = bm.loops.layers.uv.verify()
        scale_factor = 0.1
        for i, v in enumerate(vertData):
            vertex = bm.verts.new(arx_pos_to_blender_for_model(v.xyz) * scale_factor)
            vertex.normal = v.n
            vertex.index = i
        bm.verts.ensure_lookup_table()
        for i, f in enumerate(faceData):
            if i in facesToDrop:
                continue
            faceVerts = [bm.verts[v] for v in f.vids]
            try:
                face = bm.faces.new(faceVerts)
                face.index = i
            except ValueError:
                self.log.debug("Extra face")
            face.normal = Vector(f.normal)
            face.material_index = max(f.texid, 0)
            face[arxFaceType] = f.facetype
            face[arxTransVal] = f.transval
            for j, loop in enumerate(face.loops):
                u, v = f.uvs[j]
                loop[uvData].uv = (u, 1.0 - v)
        bm.faces.ensure_lookup_table()
        bm.edges.index_update()
        bm.edges.ensure_lookup_table()
        return bm

    def createArmature(self, canonicalId, bm, groups) -> bpy.types.Object:
        amtname = "/".join(canonicalId) + "-amt"
        amt = bpy.data.armatures.new(amtname)
        amt.display_type = 'WIRE'
        amtobject = bpy.data.objects.new(amtname, amt)
        amtobject.location = (0, 0, 0)
        bpy.context.scene.collection.objects.link(amtobject)
        bpy.context.view_layer.objects.active = amtobject
        bpy.ops.object.mode_set(mode='EDIT')
        edit_bones = amt.edit_bones
        edit_bones_array = []
        scale_factor = 1
        for i, group in enumerate(groups):
            bGrpName = f"grp:{i:02d}:{group.name}"
            bone = edit_bones.new(bGrpName)
            if group.origin >= 0 and group.origin < len(bm.verts):
                bone.head = bm.verts[group.origin].co * scale_factor
            else:
                self.log.warning("Invalid origin index %d for group '%s', using (0,0,0)", group.origin, bGrpName)
                bone.head = Vector((0, 0, 0))
            # Bones point at their first child so the armature reads as a skeleton
            # and can actually be posed by hand.
            #
            # Arx bones have no orientation of their own - EERIE_CreateCedricData
            # gives a bone only a translation from its parent and treats the
            # animation quaternion as its entire local rotation - so this rest
            # orientation is ours, not the format's. The animation importer and
            # exporter cancel it out through the bone's rest matrix rather than
            # letting it rotate every pose, which is what arx_io_animation's
            # engine_transform_to_pose_basis and its inverse are for.
            tail_set = False
            children = [j for j, child_group in enumerate(groups) if child_group.parentIndex == i]
            if children:
                child_group = groups[children[0]]
                if child_group.origin >= 0 and child_group.origin < len(bm.verts):
                    candidate = bm.verts[child_group.origin].co * scale_factor
                    if (candidate - bone.head).length > 1e-4:
                        bone.tail = candidate
                        tail_set = True
            if not tail_set and group.parentIndex >= 0 and group.parentIndex < len(groups):
                parent_group = groups[group.parentIndex]
                if parent_group.origin >= 0 and parent_group.origin < len(bm.verts):
                    direction = bone.head - bm.verts[parent_group.origin].co * scale_factor
                    if direction.length > 1e-4:
                        bone.tail = bone.head + direction.normalized() * (
                            BONE_DISPLAY_LENGTH * scale_factor)
                        tail_set = True
            if not tail_set:
                bone.tail = bone.head + Vector((0.0, BONE_DISPLAY_LENGTH * scale_factor, 0.0))
            bone["OriginVertex"] = group.origin
            bone["ParentIndex"] = group.parentIndex
            if group.origin >= 0 and group.origin < len(bm.verts):
                orig_pos = bm.verts[group.origin].co.copy()
                bone["OriginVertexPos"] = [orig_pos.x, orig_pos.y, orig_pos.z]
            edit_bones_array.append(bone)
            self.log.debug("Created bone '%s' at head=%s, tail=%s", bGrpName, bone.head, bone.tail)
        for i, group in enumerate(groups):
            if group.parentIndex >= 0 and group.parentIndex < len(groups):
                bone = edit_bones_array[i]
                bone.parent = edit_bones_array[group.parentIndex]
                self.log.debug("Set parent of bone '%s' to '%s'", bone.name, edit_bones_array[group.parentIndex].name)
        bpy.ops.object.mode_set(mode='OBJECT')
        return amtobject

    def createObject(self, context, bm, data: FtlData, canonicalId, collection) -> bpy.types.Object:
        idString = "/".join(canonicalId)
        self.validateObjectName(idString)
        mesh = bpy.data.meshes.new(idString)
        bm.to_mesh(mesh)
        armatureObj = self.createArmature(canonicalId, bm, data.groups)
        bm.free()
        if bpy.context.active_object:
            bpy.ops.object.mode_set(mode='OBJECT')
        obj = bpy.data.objects.new(name=mesh.name, object_data=mesh)
        obj['arx.ftl.name'] = data.metadata.name
        obj['arx.ftl.org'] = data.metadata.org
        collection.objects.link(obj)
        vertex_to_group = {}
        for i in reversed(range(len(data.groups))):
            group = data.groups[i]
            grp = obj.vertex_groups.new(name=f"grp:{i:02d}:{group.name}")
            if not group.indices:
                self.log.warning("Vertex group '%s' is empty, no vertices assigned", grp.name)
                continue
            valid_indices = [v for v in group.indices if v >= 0 and v < len(mesh.vertices)]
            if len(valid_indices) < len(group.indices):
                self.log.warning("Group '%s' has %d invalid vertex indices", grp.name, len(group.indices) - len(valid_indices))
            for v in valid_indices:
                if v not in vertex_to_group:
                    vertex_to_group[v] = grp
                    self.log.debug("Assigned vertex %d to group '%s' with weight 1.0", v, grp.name)
        for v, grp in vertex_to_group.items():
            grp.add([v], 1.0, 'REPLACE')
        for i, s in enumerate(data.sels):
            grp = obj.vertex_groups.new(name=f"sel:{i:02d}:{s.name}")
            valid_indices = [v for v in s.indices if v >= 0 and v < len(mesh.vertices)]
            if valid_indices:
                grp.add(valid_indices, 1.0, 'REPLACE')
        for a in data.actions:
            action = bpy.data.objects.new(a.name, None)
            action.parent = obj
            action.parent_type = 'VERTEX'
            action.parent_vertices = [a.vidx, 0, 0]
            action.show_name = True
            collection.objects.link(action)
            if a.name.lower().startswith("hit_"):
                radius = int(a.name[4:])
                action.empty_display_type = 'SPHERE'
                action.empty_display_size = radius
                action.lock_rotation = [True, True, True]
                action.lock_scale = [True, True, True]
            else:
                action.scale = [3, 3, 3]
        for i, group in enumerate(data.groups):
            if group.origin >= 0 and group.origin < len(mesh.vertices):
                empty_name = f"origin:{i:02d}:{group.name}"
                empty = bpy.data.objects.new(empty_name, None)
                empty.empty_display_type = 'PLAIN_AXES'
                empty.empty_display_size = 0.05
                empty.parent = obj
                empty.parent_type = 'VERTEX'
                empty.parent_vertices = [group.origin, 0, 0]
                empty.show_name = True
                collection.objects.link(empty)
                self.log.debug("Created origin empty '%s' at vertex %d", empty_name, group.origin)
        armatureModifier = obj.modifiers.new(type='ARMATURE', name="Skeleton")
        armatureModifier.object = armatureObj
        for m in data.mats:
            mat = createMaterial(self.dataPath, m)
            obj.data.materials.append(mat)
        armatureObj.parent = obj
        collection.objects.link(armatureObj)
        bpy.context.view_layer.update()
        return obj

    def loadFile_data(self, filePath) -> FtlData:
        self.validateAssetDirectory()
        self.log.debug("Loading file: %s", filePath)
        with open(filePath, "rb") as f:
            data = f.read()
            if data[:3] == b"FTL":
                unpacked = data
                self.log.debug("Loaded %i unpacked bytes from file %s", len(data), filePath)
            else:
                unpacked = self.ioLib.unpack(data)
                with open(filePath + ".unpack", "wb") as f:
                    f.write(unpacked)
                    self.log.debug("Written unpacked ftl")
                self.log.debug("Loaded %i packed bytes from file %s", len(data), filePath)
        ftlData = self.ftlSerializer.read(unpacked)
        for i, group in enumerate(ftlData.groups):
            father = self.getFatherIndex(ftlData.groups, i)
            self.log.debug("group %d with father %d", i, father)
            group.parentIndex = father
        return ftlData

    def getFatherIndex(self, groups, index):
        return groups[index].parentIndex

    def loadFile(self, context, filePath, scene, import_tweaks: bool) -> bpy.types.Object:
        ftlData = self.loadFile_data(filePath)
        # Use os.path.join for cross-platform compatibility
        objPath = os.path.join(self.dataPath, "game", "graph", "obj3d", "interactive")
        relPath = os.path.relpath(filePath, objPath)
        split = splitPath(relPath)
        canonicalId = split[:-1]
        parent_collection = context.scene.collection
        idLen = len(canonicalId)
        for i in range(1, idLen + 1):
            prefix_str = "/".join(canonicalId[:i])
            if prefix_str in bpy.data.collections:
                parent_collection = bpy.data.collections[prefix_str]
            else:
                col = bpy.data.collections.new(prefix_str)
                parent_collection.children.link(col)
                parent_collection = col
        object_id_string = "/".join(canonicalId)
        self.log.debug("Canonical ID: %s", object_id_string)
        bm = self.createBmesh(context, ftlData.verts, ftlData.faces)
        obj = self.createObject(context, bm, ftlData, canonicalId, parent_collection)
        if import_tweaks:
            file_dir = os.path.dirname(filePath)
            tweak_dir = os.path.join(file_dir, 'tweaks')
            if os.path.isdir(tweak_dir):
                for rel_tweak_file in os.listdir(tweak_dir):
                    abs_tweak_file = os.path.join(tweak_dir, rel_tweak_file)
                    tweak_data = self.loadFile_data(abs_tweak_file)
                    tweak_mesh = self.createBmesh(context, tweak_data.verts, tweak_data.faces)
                    tweak_obj_id = canonicalId.copy()
                    tweak_obj_id.append(os.path.splitext(rel_tweak_file)[0])
                    tweak_obj = self.createObject(context, tweak_mesh, tweak_data, tweak_obj_id, parent_collection)
                    tweak_obj.parent = obj
        return obj

    def updateArmatureOrigins(self, obj, verts, scale_factor):
        canonicalId = obj.name.split("/")
        armature_name = "/".join(canonicalId) + "-amt"
        armature_obj = bpy.data.objects.get(armature_name)
        if not armature_obj or armature_obj.type != 'ARMATURE':
            return
        armature = armature_obj.data
        bones_to_update = []
        for bone in armature.bones:
            if "OriginVertex" in bone:
                origin_idx = bone["OriginVertex"]
                if 0 <= origin_idx < len(verts):
                    current_vert_pos = Vector(arx_pos_to_blender_for_model(verts[origin_idx].xyz)) * scale_factor
                    bone_head_pos = bone.head_local
                    if (current_vert_pos - bone_head_pos).length > 0.001:
                        bones_to_update.append((bone.name, current_vert_pos, origin_idx))
        if bones_to_update:
            original_active = bpy.context.view_layer.objects.active
            original_mode = bpy.context.mode
            try:
                bpy.context.view_layer.objects.active = armature_obj
                bpy.ops.object.mode_set(mode='EDIT')
                for bone_name, new_pos, vertex_idx in bones_to_update:
                    edit_bone = armature.edit_bones[bone_name]
                    old_head = edit_bone.head.copy()
                    old_tail = edit_bone.tail.copy()
                    edit_bone.head = new_pos
                    bone_vector = old_tail - old_head
                    edit_bone.tail = new_pos + bone_vector
                    self.log.info("Updated bone '%s' head: %s -> %s (vertex %d)", 
                                bone_name, old_head, new_pos, vertex_idx)
            finally:
                bpy.ops.object.mode_set(mode='OBJECT')
                if original_active:
                    bpy.context.view_layer.objects.active = original_active
                if original_mode != 'OBJECT':
                    try:
                        bpy.ops.object.mode_set(mode=original_mode)
                    except:
                        pass

    def validate_mesh(self, obj, bm):
        """Validate mesh properties and custom data layers."""
        if obj is None or obj.type != 'MESH':
            self.log.error("No valid mesh object provided")
            raise ArxException("No active mesh object to export")
        if not (math.isclose(obj.location[0], 0.0) and math.isclose(obj.location[1], 0.0) and math.isclose(obj.location[2], 0.0)):
            self.log.error("Object has non-zero location: %s", obj.location)
            raise ArxException("Object is moved, please apply the location to the vertex positions")
        if not (math.isclose(obj.rotation_euler[0], 0.0) and math.isclose(obj.rotation_euler[1], 0.0) and math.isclose(obj.rotation_euler[2], 0.0)):
            self.log.error("Object has non-zero rotation: %s", obj.rotation_euler)
            raise ArxException("Object is rotated, please apply the rotation to the vertex positions")
        if not (math.isclose(obj.scale[0], 1.0) and math.isclose(obj.scale[1], 1.0) and math.isclose(obj.scale[2], 1.0)):
            self.log.error("Object has non-unit scale: %s", obj.scale)
            raise ArxException("Object is scaled, please apply the scale to the vertex positions")
        arxFaceType = bm.faces.layers.int.get('arx_facetype')
        if not arxFaceType:
            self.log.error("Missing arx_facetype layer")
            raise ArxException("Mesh is missing arx specific data layer: arx_facetype")
        arxTransVal = bm.faces.layers.float.get('arx_transval')
        if not arxTransVal:
            self.log.error("Missing arx_transval layer")
            raise ArxException("Mesh is missing arx specific data layer: arx_transval")
        for f in bm.faces:
            if len(f.loops) > 3:
                self.log.error("Found face with %d vertices", len(f.loops))
                raise ArxException("Face with more than 3 vertices found")
        for o in bpy.data.objects:
            for ms in o.material_slots:
                if ms.material and not ms.material.name.endswith('-mat'):
                    self.log.error("Material %s does not end with -mat", ms.material.name)
                    raise ArxException("Material slot names must end with: -mat")

    def validate_child_empties(self, obj):
        """Validate child empty objects."""
        for child in bpy.data.objects:
            if child.type == 'EMPTY' and child.parent == obj:
                nameParts = child.name.split(".")
                name = nameParts[0].lower()
                if name not in ("hit_", "view_attach", "primary_attach", "left_attach", "weapon_attach", "secondary_attach", "shield_attach", "head2chest", "chest2leggings", "u_right", "v_right", "fire") and not name.startswith("origin:"):
                    self.log.warning("Unexpected child empty: %s", nameParts)

    def create_vertices(self, bm, scale_factor=0.1):
        """Create FtlVertex objects from BMesh vertices."""
        verts = []
        bmesh_to_verts = {}
        for i, v in enumerate(bm.verts):
            vertexNormals = [sc.normal for sc in bm.verts if (sc.co - v.co).length < 1e-6]
            normal = Vector((0.0, 0.0, 0.0))
            for n in vertexNormals:
                normal += n
            normal.normalize()
            scaled_co = [coord / scale_factor for coord in v.co[:]]
            verts.append(FtlVertex(
                xyz=blender_pos_to_arx(scaled_co),
                n=blender_pos_to_arx(normal)
            ))
            bmesh_to_verts[v] = i
        return verts, bmesh_to_verts

    def create_metadata(self, obj, verts):
        """Create FtlMetadata with origin vertex."""
        originVertexIndex = -1
        for index, vert in enumerate(verts):
            if math.isclose(vert.xyz[0], 0.0, abs_tol=1e-6) and \
               math.isclose(vert.xyz[1], 0.0, abs_tol=1e-6) and \
               math.isclose(vert.xyz[2], 0.0, abs_tol=1e-6):
                originVertexIndex = index
                self.log.debug("Found origin vertex at index %d", index)
        if originVertexIndex == -1:
            self.log.info("Origin vertex not found, adding new one")
            originVertexIndex = len(verts)
            verts.append(FtlVertex((0.0, 0.0, 0.0), (0.0, 0.0, 0.0)))
        return FtlMetadata(name=obj.get('arx.ftl.name', ''), org=originVertexIndex), verts

    def action_bone(self, obj, empty, mesh_verts):
        """Which bone drives an attach point.

        This is not cosmetic. AnimationRender.cpp positions a linked object at
        the attach vertex and then rotates it by that bone's animation
        quaternion, whole and entire - there is no rotation stored on the slot
        and no normal involved. Pick the wrong bone and the sword hangs at the
        wrong angle, which is exactly what "the back slot is messed up" looks
        like. getGroupForVertex scans groups in reverse, so of the groups
        holding the vertex the engine uses the highest numbered one.

        An 'arx_action_group' property on the empty names the bone outright, by
        group name or by index. Without it, fall back to the deepest group of the
        nearest vertex - which is right for anything held in a hand, and wrong
        for anything hanging off a bone that carries no geometry of its own.
        human_base's WEAPON_ATTACH is the case in point: it rides on
        secondary_weapon, a bone that exists for nothing else, and no amount of
        looking at nearby vertices will find it.
        """
        requested = empty.get('arx_action_group')
        if requested is not None:
            if isinstance(requested, str):
                for vertex_group in obj.vertex_groups:
                    parts = vertex_group.name.split(':', 2)
                    if parts[0] == 'grp' and requested in (parts[2], vertex_group.name):
                        return int(parts[1])
                self.log.warning("Action '%s' asks for unknown group '%s'", empty.name, requested)
            else:
                return int(requested)

        if not mesh_verts:
            return None
        nearest = min(range(len(mesh_verts)),
                      key=lambda i: (mesh_verts[i].co - empty.location).length_squared)
        deepest = None
        for group in mesh_verts[nearest].groups:
            parts = obj.vertex_groups[group.group].name.split(':', 2)
            if parts[0] == 'grp' and (deepest is None or int(parts[1]) > deepest):
                deepest = int(parts[1])
        return deepest

    def create_actions(self, obj, verts, scale_factor=0.1):
        """Create FtlAction objects from empty objects.

        An action is a named vertex: primary_attach is where a drawn weapon
        hangs, weapon_attach where a sheathed one does. linkObjects looks the
        vertex up and then asks getGroupForVertex which bone drives it, and
        gives up silently if the answer is none - the weapon is created, owned,
        and never appears. A vertex in no group is not animated either, so even
        without the link it would sit where the model was built.

        A VERTEX parented empty reuses an existing vertex and inherits its
        groups, which is what the importer writes and why a stock model survives
        a round trip. An OBJECT parented one, which is how a model gets authored
        from scratch, appends a new vertex that belongs to nothing - so note
        which vertex it sits on and let create_groups_and_selections put it in
        the same bone.
        """
        actions = []
        self.action_vertex_group = {}
        mesh_verts = obj.data.vertices
        for o in bpy.data.objects:
            if o.type == 'EMPTY' and o.parent == obj:
                nameParts = o.name.split(".")
                name = nameParts[0]
                if name.startswith("origin:"):
                    # Bone origins, not actions - they are rebuilt from the groups.
                    continue
                if o.parent_type == 'VERTEX':
                    actionVertexIndex = o.parent_vertices[0]
                    if 0 <= actionVertexIndex < len(obj.data.vertices):
                        actions.append(FtlAction(name, actionVertexIndex))
                    else:
                        self.log.warning("Invalid vertex index %d for empty '%s', skipping", actionVertexIndex, name)
                elif o.parent_type == 'OBJECT':
                    actionVertexIndex = len(verts)
                    scaled_location = [coord / scale_factor for coord in o.location]
                    verts.append(FtlVertex(blender_pos_to_arx(scaled_location), (0.0, 0.0, 0.0)))
                    actions.append(FtlAction(name, actionVertexIndex))
                    group = self.action_bone(obj, o, mesh_verts)
                    if group is not None:
                        self.action_vertex_group[actionVertexIndex] = group
                        self.log.debug("Action '%s' driven by group %d", name, group)
                    else:
                        self.log.warning("Action '%s' belongs to no group, so it will not "
                                         "animate and nothing can be linked to it", name)
                else:
                    self.log.warning("Unhandled empty parent type %s for '%s'", o.parent_type, name)
        return actions, verts

    def find_vertex_index(self, coord, allXYZ, tolerance=1e-6):
        """Find vertex index with floating-point tolerance."""
        for i, xyz in enumerate(allXYZ):
            if (math.isclose(coord[0], xyz[0], abs_tol=tolerance) and
                math.isclose(coord[1], xyz[1], abs_tol=tolerance) and
                math.isclose(coord[2], xyz[2], abs_tol=tolerance)):
                return i
        self.log.error("Vertex %s not found in vertex list", coord)
        raise ValueError(f"Vertex {coord} not found in vertex list")

    def create_faces(self, bm, verts, scale_factor=0.1):
        """Create FtlFace objects from BMesh faces."""
        uvData = bm.loops.layers.uv.verify()
        arxFaceType = bm.faces.layers.int['arx_facetype']
        arxTransVal = bm.faces.layers.float['arx_transval']
        allXYZ = [v.xyz for v in verts]
        faces = []
        for f in bm.faces:
            vertexIndices = []
            vertexUvs = []
            vertexNormals = []
            for c in f.loops:
                scaled_co = [coord / scale_factor for coord in c.vert.co[:]]
                vertexIndices.append(self.find_vertex_index(blender_pos_to_arx(scaled_co), allXYZ))
                vertexUvs.append((c[uvData].uv[0], 1 - c[uvData].uv[1]))
                vertexNormals.append((c.vert.normal[0], c.vert.normal[1], c.vert.normal[2]))
            faces.append(FtlFace(
                vids=tuple(vertexIndices),
                uvs=vertexUvs,
                texid=f.material_index,
                facetype=f[arxFaceType],
                transval=f[arxTransVal],
                normal=vertexNormals
            ))
        return faces


    def create_groups_and_selections(self, obj, bm, bmesh_to_verts):
        """Create FtlGroup and FtlSelection objects from vertex groups."""
        grps = []
        sels = []
        dvert_lay = bm.verts.layers.deform.active
        if not dvert_lay:
            self.log.warning("No deform layer found, groups will be empty")
            return grps, sels
        armature = None
        for modifier in obj.modifiers:
            if modifier.type == 'ARMATURE' and modifier.object:
                armature = modifier.object.data
                break
        vertex_to_groups = {}  # Map vertex index to list of group indices
        for vert in bm.verts:
            vertex_to_groups[bmesh_to_verts[vert]] = []
        for grp in obj.vertex_groups:
            v = []
            for vert in bm.verts:
                dvert = vert[dvert_lay]
                if grp.index in dvert:
                    v.append(bmesh_to_verts[vert])
                    vertex_to_groups[bmesh_to_verts[vert]].append(grp.index)
            # maxsplit, because a group name may itself contain a colon -
            # human_base really does ship one called "28: right_hand_fingers_root".
            s = grp.name.split(":", 2)
            if s[0] == "grp":
                group_index = int(s[1])
                group_name = s[2]
                origin_empty_name = f"origin:{group_index:02d}:{group_name}"
                origin_empty = bpy.data.objects.get(origin_empty_name)
                origin = 0
                if origin_empty and origin_empty.parent == obj and origin_empty.parent_type == 'VERTEX':
                    origin_vertex_index = origin_empty.parent_vertices[0]
                    if 0 <= origin_vertex_index < len(bm.verts):
                        origin = bmesh_to_verts[bm.verts[origin_vertex_index]]
                    else:
                        origin = v[0] if v else 0
                        self.log.warning("Origin empty for group '%s' points to invalid vertex %d, using %d", 
                                       grp.name, origin_vertex_index, origin)
                else:
                    # Choose origin from vertices that are also in the parent group
                    parentIndex = -1
                    if armature:
                        bone_name = f"grp:{group_index:02d}:{group_name}"
                        if bone_name in armature.bones:
                            bone = armature.bones[bone_name]
                            if bone.parent and bone.parent.name.startswith("grp:"):
                                parent_parts = bone.parent.name.split(":")
                                parent_index = int(parent_parts[1])
                                parentIndex = parent_index
                    if parentIndex != -1:
                        for vert in v:
                            vert_groups = vertex_to_groups[vert]
                            parent_group = [g for g in grps if g.sortIndex == parentIndex]
                            if parent_group and vert in parent_group[0].indices:
                                origin = vert
                                break
                        if origin == 0:
                            origin = v[0] if v else 0
                            self.log.warning("No suitable origin found for group '%s', using %d", group_name, origin)
                    else:
                        origin = v[0] if v else 0
                        self.log.warning("No origin empty or parent for group '%s', using %d", group_name, origin)
                parentIndex = -1
                if armature:
                    bone_name = f"grp:{group_index:02d}:{group_name}"
                    if bone_name in armature.bones:
                        bone = armature.bones[bone_name]
                        if bone.parent and bone.parent.name.startswith("grp:"):
                            parent_parts = bone.parent.name.split(":")
                            parent_index = int(parent_parts[1])
                            parentIndex = parent_index
                # Attach points assigned to this bone join its vertex list, which
                # is what makes getGroupForVertex answer with this bone and so what
                # decides the orientation of anything linked there.
                for action_vertex, target in getattr(self, 'action_vertex_group', {}).items():
                    if target == group_index and action_vertex not in v:
                        v.append(action_vertex)

                ftl_group = FtlGroup(s[2], origin, v, parentIndex)
                ftl_group.sortIndex = group_index
                grps.append(ftl_group)
                self.log.debug("Added FTL group '%s' (index %d): origin=%d, parent=%d, indices=%s", 
                             s[2], group_index, origin, parentIndex, v)
            else:
                sels.append(FtlSelection(s[2], v))
                self.log.debug("Added FTL selection '%s': indices=%s", s[2], v)
        
        # Propagate child origins to parent indices
        for group in grps:
            group_index = group.sortIndex
            bone_name = f"grp:{group_index:02d}:{group.name}"
            if armature and bone_name in armature.bones:
                bone = armature.bones[bone_name]
                # Find all child bones
                for child_bone in armature.bones:
                    if child_bone.parent == bone and child_bone.name.startswith("grp:"):
                        child_parts = child_bone.name.split(":")
                        child_index = int(child_parts[1])
                        child_group = [g for g in grps if g.sortIndex == child_index]
                        if child_group:
                            child_origin = child_group[0].origin
                            if child_origin not in group.indices:
                                group.indices.append(child_origin)
                                self.log.debug("Added child origin %d to group '%s' (index %d) for child '%s' (index %d)", 
                                             child_origin, group.name, group_index, child_group[0].name, child_index)
        
        return grps, sels

    def buildHierarchicalOrder(self, grps, armature):
        """Sort groups to match original FTL order and ensure getFatherIndex compatibility."""
        self.log.debug("Starting buildHierarchicalOrder with %d groups", len(grps))
        if not armature:
            self.log.warning("No armature found, sorting groups by sortIndex")
            return sorted(grps, key=lambda g: g.sortIndex)
        
        ordered_groups = sorted(grps, key=lambda g: g.sortIndex)
        
        self.log.debug("Ordered groups: %s", [g.name for g in ordered_groups])
        self.log.debug("Parent indices: %s", [g.parentIndex for g in ordered_groups])
        return ordered_groups

    def create_material_names(self):
        """Create material names for FtlData."""
        matNames = []
        for o in bpy.data.objects:
            for ms in o.material_slots:
                if ms.material:
                    matNames.append(f'GRAPH\\OBJ3D\\TEXTURES\\{ms.material.name[:-4]}.FOO')
        return matNames

    def toFtlData(self, obj) -> FtlData:
        """Convert Blender object to FtlData."""
        self.log.debug("Starting toFtlData for object: %s", obj.name)
        canonicalId = obj.name.split("/")
        self.log.debug("Exporting Canonical Id: %s", canonicalId)
        
        bm = bmesh.new()
        try:
            bm.from_mesh(obj.data)
            bm.normal_update()
            bm.verts.ensure_lookup_table()
            bm.edges.ensure_lookup_table()
            bm.faces.ensure_lookup_table()
            
            self.validate_mesh(obj, bm)
            self.validate_child_empties(obj)
            
            verts, bmesh_to_verts = self.create_vertices(bm)
            metadata, verts = self.create_metadata(obj, verts)
            actions, verts = self.create_actions(obj, verts)
            faces = self.create_faces(bm, verts)
            grps, sels = self.create_groups_and_selections(obj, bm, bmesh_to_verts)
            
            armature = None
            for modifier in obj.modifiers:
                if modifier.type == 'ARMATURE' and modifier.object:
                    armature = modifier.object.data
                    break
            grps = self.buildHierarchicalOrder(grps, armature)
            
            matNames = self.create_material_names()
            
            self.log.debug("Final FTL data: %d vertices, %d faces, %d groups, %d selections", 
                         len(verts), len(faces), len(grps), len(sels))
            self.log.debug("Group order: %s", [g.name for g in grps])
            self.log.debug("Parent indices: %s", [g.parentIndex for g in grps])
            
            return FtlData(
                metadata=metadata,
                verts=verts,
                faces=faces,
                mats=matNames,
                groups=grps,
                actions=actions,
                sels=sels
            )
        finally:
            bm.free()

    def saveFile(self, path):
        obj = bpy.context.active_object
        if obj is None or obj.type != 'MESH':
            raise ArxException("No active mesh object to export")
        data = self.toFtlData(obj)
        binData = self.ftlSerializer.write(data)
        with open(path, 'wb') as f:
            f.write(binData)
        self.log.debug("Written %i bytes to file %s", len(binData), path)
