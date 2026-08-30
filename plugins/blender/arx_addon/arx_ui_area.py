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

import bpy
import os
import struct
from bpy.props import IntProperty, BoolProperty, StringProperty, CollectionProperty, PointerProperty, EnumProperty, FloatProperty, FloatVectorProperty
from bpy.types import Operator, Panel, PropertyGroup, UIList
from mathutils import Matrix, Vector, Quaternion
from .arx_io_util import ArxException, arx_pos_to_blender_for_model, arx_transform_to_blender, blender_pos_to_arx
from .managers import getAddon
from .arx_asl_reader import ASLReader
from .arx_asl_syntax import ASLSyntaxHighlighter, ASLNavigator
from . import anchor_generation
import math

# Global ASL navigator instance
g_asl_navigator = None

def get_asl_text_name(entity_ident, object_id=None):
    """Generate consistent text block name for ASL files"""
    if object_id:
        safe_object_id = object_id.replace('/', '_')
        return f"ASL_{safe_object_id}_{entity_ident:04d}"
    else:
        return f"ASL_{entity_ident:04d}"

def parse_asl_text_name(text_name):
    """Parse entity_ident and object_id from ASL text block name"""
    if not text_name.startswith('ASL_'):
        return None, None
    
    parts = text_name[4:].split('_')  # Remove 'ASL_' prefix
    
    try:
        if len(parts) == 1:
            # Format: ASL_0001
            entity_ident = int(parts[0])
            return entity_ident, None
        else:
            # Format: ASL_object_id_0001
            entity_ident = int(parts[-1])  # Last part is always entity_ident
            object_id = '_'.join(parts[:-1]).replace('_', '/')  # Restore original object_id
            return entity_ident, object_id
    except ValueError:
        return None, None

#: Level file version the exporter writes. The engine implodes both the .dlf body
#: and the .llf from 1.44 onwards (DanaeLoadLevel), and _writeDlfFile always
#: implodes, so this is not a free choice.
DLF_WRITE_VERSION = 1.44

g_areaToLevel = {
    0:0, 8:0, 11:0, 12:0,
    1:1, 13:1, 14:1,
    2:2, 15:2,
    3:3, 16:3, 17:3,
    4:4, 18:4, 19:4,
    5:5, 21:5,
    6:6, 22:6,
    7:7, 23:7
}

def importArea(context, report, area_id):
    scene_name = f"Area_{area_id:02d}"
    scene = bpy.data.scenes.get(scene_name)
    if scene:
        report({'INFO'}, f"Area Scene named [{scene_name}] already exists.")
        return
    report({'INFO'}, f"Creating new Area Scene [{scene_name}]")
    scene = bpy.data.scenes.new(name=scene_name)
    scene.unit_settings.system = 'METRIC'
    scene.unit_settings.scale_length = 0.01
    getAddon(context).sceneManager.importScene(context, scene, area_id)

class CUSTOM_OT_arx_area_list_reload(Operator):
    bl_idname = "arx.arx_area_list_reload"
    bl_label = "Reload Area List"
    def invoke(self, context, event):
        area_list = context.window_manager.arx_areas_col
        area_list.clear()
        for area_id, value in getAddon(context).arxFiles.levels.levels.items():
            item = area_list.add()
            item.name = f'Area {area_id}'
            item.area_id = area_id
            item.level_id = g_areaToLevel.get(area_id, -1)
        return {"FINISHED"}

class ARX_area_properties(PropertyGroup):
    area_id: IntProperty(name="Arx Area ID", min=0)
    level_id: IntProperty(name="Arx Level ID", description="Levels consist of areas", min=-1)

class ARX_lighting_properties(PropertyGroup):
    """Properties for controlling vertex lighting generation"""
    
    # Lighting workflow options
    import_original_lighting: BoolProperty(
        name="Import Original Lighting",
        description="Import and use original LLF lighting data",
        default=True
    )
    
    regenerate_lighting: BoolProperty(
        name="Regenerate Lighting",
        description="Calculate new vertex lighting when exporting",
        default=True  # Default to True since this is a full level editor
    )
    
    lighting_method: EnumProperty(
        name="Lighting Method",
        description="Method to use for lighting calculation",
        items=[
            ('DANAE', 'DANAE (original)', 'Bake exactly the way the original DANAE editor did'),
            ('CYCLES', 'Cycles Renderer', 'Use Blender Cycles for realistic lighting'),
            ('SIMPLE', 'Simple Calculation', 'Fast basic lighting calculation'),
            ('PRESERVE', 'Preserve Original', 'Keep existing vertex colors from Blender'),
            ('SKIP', 'Skip Lighting', 'Skip lighting update entirely (fast export)')
        ],
        default='DANAE'
    )

    # DANAE bake settings. The defaults reproduce the original editor; they are
    # exposed because real levels were tuned by eye against this bake.
    danae_use_normals: BoolProperty(
        name="Use Vertex Normals",
        description="MODE_NORMALS: modulate each vertex by the angle to the light",
        default=True
    )

    danae_raylaunch: BoolProperty(
        name="Cast Shadows",
        description="MODE_RAYLAUNCH: trace a shadow ray from each light to each vertex",
        default=True
    )

    danae_ambient: FloatProperty(
        name="Ambient Floor",
        description="Minimum value per channel, 0.09 in the original DANAE",
        default=0.09,
        min=0.0,
        max=1.0
    )

    
    # Cycles settings
    cycles_samples: IntProperty(
        name="Cycles Samples",
        description="Number of samples for Cycles lighting calculation",
        default=64,
        min=1,
        max=4096
    )

    cycles_use_denoising: BoolProperty(
        name="Use Denoising",
        description="Enable denoising for cleaner results",
        default=True
    )

    cycles_bake_type: EnumProperty(
        name="Bake Type",
        description="Type of lighting to bake",
        items=[
            ('DIFFUSE', 'Diffuse', 'Diffuse lighting only'),
            ('COMBINED', 'Combined', 'All lighting effects combined'),
            ('AO', 'Ambient Occlusion', 'Ambient occlusion only'),
        ],
        default='DIFFUSE'
    )

    cycles_use_direct_light: BoolProperty(
        name="Direct Lighting",
        description="Include direct lighting from light sources",
        default=True
    )

    cycles_use_indirect_light: BoolProperty(
        name="Indirect Lighting",
        description="Include indirect (bounced) lighting",
        default=True
    )

    cycles_use_color: BoolProperty(
        name="Include Material Color",
        description="Multiply by material diffuse color",
        default=False
    )

    cycles_ao_distance: FloatProperty(
        name="AO Distance",
        description="Distance for ambient occlusion rays",
        default=1.0,
        min=0.0,
        max=100.0
    )

    # Export settings
    export_intensity_multiplier: FloatProperty(
        name="Export Intensity Multiplier",
        description="Multiplier for lightmap brightness when exporting to LLF. Only "
                    "applies to the renderer based methods; the DANAE bake needs no "
                    "correction and ignores this",
        default=1.0,
        min=0.0,
        max=2.0,
        soft_min=0.1,
        soft_max=1.0
    )

    # Simple lighting parameters
    ambient_strength: FloatProperty(
        name="Ambient Strength",
        description="Strength of ambient lighting",
        default=0.2,
        min=0.0,
        max=1.0
    )
    
    light_falloff_power: FloatProperty(
        name="Light Falloff Power",
        description="Power curve for light distance falloff",
        default=1.5,
        min=0.1,
        max=3.0
    )
    
    max_light_contribution: FloatProperty(
        name="Max Light Contribution",
        description="Maximum brightness from lights",
        default=200.0,
        min=0.0,
        max=255.0
    )

class SCENE_UL_arx_area_list(UIList):
    def draw_item(self, context, layout, data, item, icon, active_data, active_propname, index):
        split = layout.split(factor=0.3)
        split.label(text=item.name)
        split.label(text=str(item.area_id))
        split.label(text=str(item.level_id))
    def invoke(self, context, event):
        pass

class CUSTOM_OT_arx_area_list_import_selected(Operator):
    bl_idname = "arx.area_list_import_selected"
    bl_label = "Import Selected Area"
    def invoke(self, context, event):
        area_list = context.window_manager.arx_areas_col
        area = area_list[context.window_manager.arx_areas_idx]
        try:
            importArea(context, self.report, area.area_id)
        except ArxException as e:
            self.report({'ERROR'}, str(e))
            return {'CANCELLED'}
        return {'FINISHED'}

class ArxOperatorImportAllLevels(Operator):
    bl_idname = "arx.operator_import_all_levels"
    bl_label = "Import All Levels"
    def execute(self, context):
        for area_id, value in getAddon(context).arxFiles.levels.levels.items():
            try:
                importArea(context, self.report, area_id)
            except ArxException as e:
                self.report({'ERROR'}, str(e))
                return {'CANCELLED'}
        return {'FINISHED'}

def gather_danae_lights(scene):
    """Collect the Arx lights to bake with, in Arx world coordinates.

    Light objects created by the importer carry the original Arx parameters as
    custom properties, so moving or retuning a light in Blender is picked up here.
    Anything without those properties is not an Arx light and is ignored - Blender
    wattage has no meaning in this model.
    """
    from .danae_lighting import DanaeLight

    lights = []
    # EERIE_LIGHT_Apply only accumulates lights that exist, are switched on and are
    # not semi-dynamic - the semi-dynamic ones are recomputed by the engine at run
    # time, and baking them in as well is double lighting. src/scene/Light.h has the
    # flag values.
    EXTRAS_SEMIDYNAMIC = 0x00000001
    EXTRAS_STARTEXTINGUISHED = 0x00000004
    EXTRAS_OFF = 0x00000020
    EXCLUDED_FROM_BAKE = EXTRAS_SEMIDYNAMIC | EXTRAS_STARTEXTINGUISHED | EXTRAS_OFF

    skipped = 0
    excluded = 0
    for obj in scene.objects:
        if obj.type != 'LIGHT':
            continue
        if 'arx_intensity' not in obj:
            skipped += 1
            continue
        if int(obj.get('arx_extras', 0)) & EXCLUDED_FROM_BAKE:
            excluded += 1
            continue

        arx_pos = Vector(blender_pos_to_arx(obj.matrix_world.translation)) * 10.0
        color = obj.data.color

        lights.append(DanaeLight(
            pos=(arx_pos.x, arx_pos.y, arx_pos.z),
            rgb=(color[0], color[1], color[2]),
            intensity=obj['arx_intensity'],
            fallstart=obj['arx_fallstart'],
            fallend=obj['arx_fallend'],
            # EXTRAS_NOCASTED lights are skipped by the ray launch pass.
            casts_shadow=not bool(obj.get('arx_nocasted', False)),
        ))

    if skipped:
        print(f"DEBUG: Ignored {skipped} light objects without Arx light properties")
    if excluded:
        print(f"DEBUG: Excluded {excluded} semi-dynamic or switched off lights from the bake")
    print(f"DEBUG: Baking with {len(lights)} Arx lights")
    return lights


# ComputePortalVertexBuffer gives up on a scene with more than 255 rooms and
# builds no vertex buffers at all, which renders the whole level black.
MAX_ROOMS = 255


def portal_plane_normal(corners):
    """The plane normal the engine will derive from a portal's stored corners.

    createNormalizedPlane in Math.cpp uses cross(p1 - p0, p2 - p0) over the first
    three, so anything that wants to reason about which way a portal faces has to
    use those same three and in that order.
    """
    if len(corners) < 3:
        return Vector((0.0, 0.0, 1.0))
    normal = (corners[1] - corners[0]).cross(corners[2] - corners[0])
    return normal.normalized() if normal.length > 1e-9 else Vector((0.0, 0.0, 1.0))


def build_danae_occluder(polygons):
    """BVH over the given polygons, in Arx coordinates, for the shadow rays.

    `polygons` is a sequence of 3 or 4 vertex position tuples in perimeter order,
    indexed to match the polygon list handed to the bake. Returns the
    `visible(origin, target, index)` callable, or None when there is nothing to
    cast shadows onto.

    This follows Visible() in src/DANAE_OLD/EERIE/EERIEPoly.cpp: keep the nearest
    thing the ray meets over its whole length, and treat it as no obstruction when
    it is the polygon being lit. That single exception is what lets a surface be
    lit at all, since every vertex lies on its own polygon. Nothing is filtered
    out of the tree - the original walks every polygon in the tiles it crosses,
    water and transparent ones included - so face i is polygon i.
    """
    from mathutils.bvhtree import BVHTree

    verts = []
    faces = []
    for vertices in polygons:
        base = len(verts)
        verts.extend(vertices)
        faces.append(tuple(range(base, base + len(vertices))))

    if not faces:
        return None

    tree = BVHTree.FromPolygons(verts, faces, all_triangles=False, epsilon=0.0)

    def visible(origin, target, index=None):
        dx = target[0] - origin[0]
        dy = target[1] - origin[1]
        dz = target[2] - origin[2]
        distance = math.sqrt(dx * dx + dy * dy + dz * dz)
        if distance <= 0.0:
            return True
        direction = (dx / distance, dy / distance, dz / distance)
        hit = tree.ray_cast(origin, direction, distance)
        if hit[0] is None:
            return True
        return hit[2] == index

    return visible



class ArxAreaExportHelper:
    """Shared utility methods for area export operations"""
    
    def __init__(self):
        self._scene_lights = []
        self._scene_offset = Vector((0, 0, 0))
        self._preserve_original_lighting = False
        self.converted_faces = []
    
    # Shared methods will be moved here from the main operator class

class CUSTOM_OT_arx_area_list_export_all(Operator, ArxAreaExportHelper):
    bl_idname = "arx.area_list_export_all"
    bl_label = "Export All Area Data"
    bl_description = "Export complete area data (FTS + LLF + DLF)"
    
    export_fts: BoolProperty(default=True)
    export_llf: BoolProperty(default=True)
    export_dlf: BoolProperty(default=True)
    
    def invoke(self, context, event):
        # Initialize helper state
        self._scene_lights = []
        self._scene_offset = Vector((0, 0, 0))
        self._preserve_original_lighting = False
        self.converted_faces = []
        
        area_list = context.window_manager.arx_areas_col
        if not area_list:
            self.report({'ERROR'}, "No area list loaded")
            return {'CANCELLED'}
            
        area = area_list[context.window_manager.arx_areas_idx]
        scene_name = f"Area_{area.area_id:02d}"
        scene = bpy.data.scenes.get(scene_name)
        
        if not scene:
            self.report({'ERROR'}, f"Scene '{scene_name}' not found. Import the area first.")
            return {'CANCELLED'}
        
        try:
            self.exportArea(context, scene, area.area_id, self.export_fts, self.export_llf, self.export_dlf)
            self.report({'INFO'}, f"Exported Area {area.area_id}")
        except ArxException as e:
            self.report({'ERROR'}, str(e))
            return {'CANCELLED'}
        except Exception as e:
            self.report({'ERROR'}, f"Export failed: {str(e)}")
            return {'CANCELLED'}
            
        return {'FINISHED'}
    
    def _map_room_id_to_index(self, room_id):
        """Room id as the engine wants it: a dense index, not the author's label."""
        if room_id < 0:
            # Negative means the polygon belongs to no room at all.
            return room_id
        return getattr(self, '_room_map', {}).get(room_id, room_id)

    def _buildRoomMap(self, fts_data):
        """Compact the room ids in use down to a contiguous range.

        The engine sizes its room array from the highest id it sees and indexes
        straight into it, so ids are positions, not names. A single room numbered
        420 therefore costs 421 room entries and a 421 by 421 distance matrix, and
        past 255 rooms ComputePortalVertexBuffer bails out before building a single
        vertex buffer, leaving the whole level black. Renumbering keeps whatever
        ids an author finds convenient while giving the engine what it needs.
        """
        from .dataFts import EERIE_SAVE_PORTALS

        used = set()
        for face in self.converted_faces:
            room = face.get('room', 0)
            if room > 0:
                used.add(room)
        for portal in fts_data.portals:
            data = (EERIE_SAVE_PORTALS.from_buffer_copy(portal)
                    if isinstance(portal, bytes) else portal)
            for room in (data.room_1, data.room_2):
                if room > 0:
                    used.add(room)

        # Room 0 is the engine's unused first slot and always maps to itself.
        self._room_map = {0: 0}
        for index, room in enumerate(sorted(used), start=1):
            self._room_map[room] = index

        renumbered = sum(1 for room, index in self._room_map.items() if room != index)
        if renumbered:
            highest = max(self._room_map)
            print(f"DEBUG: Compacted {len(used)} rooms into ids 1-{len(used)} "
                  f"(highest author id was {highest}, {renumbered} renumbered)")

        if len(used) + 1 > MAX_ROOMS:
            self.report({'ERROR'},
                        f"{len(used)} rooms is more than the engine's limit of "
                        f"{MAX_ROOMS - 1}; it will refuse to build any room "
                        f"geometry and the level will render black")

        # Portal room ids have to move with the polygons.
        portals = []
        for portal in fts_data.portals:
            data = (EERIE_SAVE_PORTALS.from_buffer_copy(portal)
                    if isinstance(portal, bytes) else portal)
            data.room_1 = self._map_room_id_to_index(data.room_1)
            data.room_2 = self._map_room_id_to_index(data.room_2)
            portals.append(bytes(data))

        # So do the room structures and the distance matrix, which are indexed by
        # room id as well. Leaving them behind is what made a renumbered level
        # cull entities out of existence: each room kept the portal list of
        # whichever room used to hold that index, so rooms that replaced an empty
        # one ended up with no portals at all. A room portal traversal can never
        # open has an empty frustum list, and ARX_SCENE_PORTAL_ClipIO answers that
        # by hiding everything standing in it.
        #
        # The portal index lists themselves need no adjustment: they point into
        # the portal array, which keeps its order.
        room_data = self._remapRoomData(fts_data.room_data, len(used) + 1)

        return fts_data._replace(portals=portals, room_data=room_data)

    def _remapRoomData(self, room_data, size):
        """Move preserved room structures and room distances onto the new ids."""
        from .dataFts import ROOM_DIST_DATA_SAVE

        if not room_data:
            return room_data
        old_rooms, old_distances = room_data
        pairs = [(old, new) for old, new in self._room_map.items() if new < size]

        new_rooms = [({'nb_portals': 0, 'nb_polys': 0, 'padd': [0] * 6}, [], [])
                     for _ in range(size)]
        for old, new in pairs:
            if old < len(old_rooms):
                new_rooms[new] = old_rooms[old]

        empty = bytes(ROOM_DIST_DATA_SAVE())
        new_distances = [[empty] * size for _ in range(size)]
        for old_i, new_i in pairs:
            if old_i >= len(old_distances):
                continue
            row = old_distances[old_i]
            for old_j, new_j in pairs:
                if old_j < len(row):
                    new_distances[new_i][new_j] = row[old_j]

        return (new_rooms, new_distances)
    
    def exportArea(self, context, scene, area_id, export_fts=True, export_llf=True, export_dlf=True):
        """Export area data based on flags"""
        print(f"DEBUG: exportArea called with export_fts={export_fts}, export_llf={export_llf}, export_dlf={export_dlf}")
        addon = getAddon(context)
        area_files = addon.arxFiles.levels.levels[area_id]
        
        # DLF-only export: Skip all the expensive FTS processing
        if export_dlf and not export_fts and not export_llf:
            if area_files.dlf:
                try:
                    self.updateDlfFile(area_files.dlf, scene, area_id)
                    self.report({'INFO'}, f"Successfully updated DLF entity data")
                except Exception as e:
                    self.report({'ERROR'}, f"DLF update failed: {str(e)}")
            return
        
        # For FTS/LLF exports, we need the expensive processing
        if area_files.fts is None:
            raise ArxException(f"Original FTS file not found for area {area_id}")
        
        # Find the background mesh
        background_obj = None
        for obj in scene.objects:
            if obj.name.endswith("-background") and obj.type == 'MESH':
                background_obj = obj
                break
                
        if not background_obj:
            raise ArxException(f"No background geometry found in scene {scene.name}")
        
        # Load lighting data for vertex lighting calculations (only if needed for LLF export)
        if export_llf:
            if area_files.llf:
                llfData = addon.sceneManager.llfSerializer.read(area_files.llf)
                self._storeLightsForLighting(llfData)
            else:
                print("WARNING: No LLF file found - lighting calculations will use defaults")
                self._storeLightsForLighting(None)
        elif export_fts:
            # For FTS-only export, skip lighting data loading for performance
            print("DEBUG: FTS-only export - skipping lighting data loading")
            self._storeLightsForLighting(None)
        
        # Try to restore complete FTS data from scene properties first
        current_scene = background_obj.users_scene[0]
        if ("arx_texture_data" in current_scene and 
            "arx_anchor_data" in current_scene and 
            "arx_portal_data" in current_scene):
            print("DEBUG: Using preserved FTS data from scene properties")
            # Use minimal base structure and restore from scene properties
            base_fts_data = addon.sceneManager.ftsSerializer.read_fts_container(area_files.fts)
            fts_data = self._restoreOriginalFtsDataFromScene(current_scene, base_fts_data)
        else:
            print("DEBUG: No preserved data found, reading fresh from FTS file and storing")
            # Read original FTS data and store it for future use
            fts_data = addon.sceneManager.ftsSerializer.read_fts_container(area_files.fts)
            self._storeOriginalFtsDataInScene(current_scene, fts_data)
        
        # Store scene offset for lighting calculations
        self._scene_offset = Vector(fts_data.sceneOffset) if hasattr(fts_data, 'sceneOffset') else Vector((0, 0, 0))
        
        # Set lighting recalculation mode based on export type
        if export_llf:
            # For LLF export, recalculate lighting for modified geometry to fix lightmap issues
            self._preserve_original_lighting = False
            print("DEBUG: LLF export - will recalculate vertex lighting")
        else:
            # For FTS-only export, preserve existing lighting to avoid expensive calculations
            # BUT regenerate if geometry has changed to prevent vector bounds errors
            self._preserve_original_lighting = True
            print("DEBUG: FTS-only export - preserving existing vertex lighting for performance")
        
        # Convert Blender mesh back to FTS cells with current material assignments
        fts_data = self.convertMeshToFtsCells(background_obj, fts_data)

        # Faces just changed, so the shared cell grid from any previous export is stale.
        self._cell_grid = None
        self._ordered_polys = None
        self._normals_prepared = False
        self._room_map = {}
        
        # Detect if geometry has been modified and rebuild portal/room system completely
        original_face_count = current_scene.get("arx_original_face_count", len(self.converted_faces))
        geometry_modified = len(self.converted_faces) != original_face_count
        
        # Read user-placed portals from Blender scene and update FTS data (skip if no portals)
        portal_count = 0
        for collection in current_scene.collection.children:
            if 'portals' in collection.name.lower():
                portal_count = len([obj for obj in collection.objects if obj.type == 'MESH'])
                break
        
        if portal_count > 0:
            print(f"DEBUG: Reading {portal_count} portals from Blender scene")
            fts_data = self._rebuildPortalSystemFromBlender(fts_data, current_scene)
        else:
            print("DEBUG: No portals found - keeping original portal data")
        
        # Read user-modified anchor network from Blender scene (skip if no anchors)
        anchor_count = 0
        for collection in current_scene.collection.children:
            if 'anchors' in collection.name.lower():
                anchor_count = len([obj for obj in collection.objects if obj.type == 'MESH'])
                break
        if anchor_count == 0:
            anchor_count = len([obj for obj in current_scene.collection.objects if 'anchor' in obj.name.lower() and obj.type == 'MESH'])
        
        if anchor_count > 0:
            print(f"DEBUG: Reading {anchor_count} anchor objects from Blender scene")
            fts_data = self._rebuildAnchorNetworkFromBlender(fts_data, current_scene)
        else:
            print("DEBUG: No anchor objects found - keeping original anchor data")

        # A level with no anchors has no pathfinding at all and says nothing about
        # it, so build one rather than ship that. Only when there is nothing to
        # keep: an existing graph, hand edited or not, is the author's.
        if not fts_data.anchors:
            print("DEBUG: No anchors anywhere - generating a navmesh from the geometry")
            generated = generate_anchors_for_scene(current_scene, self.report)
            if generated:
                fts_data = self._rebuildAnchorNetworkFromBlender(fts_data, current_scene)
        
        # Renumber rooms before anything is written, so polygons, portals and the
        # room references all agree on the new ids.
        fts_data = self._buildRoomMap(fts_data)

        # Only rebuild room polygon references if geometry was modified or new portals added
        if geometry_modified or portal_count > 0:
            if geometry_modified:
                print(f"DEBUG: Geometry modified ({len(self.converted_faces)} vs {original_face_count} faces) - user must assign room IDs manually")
                # Force lighting regeneration when geometry changes to prevent vector bounds errors
                if self._preserve_original_lighting:
                    print("DEBUG: Forcing lighting regeneration due to geometry changes")
                    self._preserve_original_lighting = False
            print("DEBUG: Rebuilding room polygon references due to changes")
            fts_data = self._rebuildRoomPolygonReferences(fts_data) or fts_data
        else:
            print("DEBUG: No geometry/portal changes - keeping original room references")
        
        # Write back to original FTS file
        if export_fts:
            try:
                self.writeFtsFile(area_files.fts, fts_data, self.converted_faces)
                self.report({'INFO'}, f"Successfully exported FTS with {len(self.converted_faces)} faces")
            except Exception as e:
                self.report({'ERROR'}, f"FTS write failed: {str(e)}")
                raise ArxException(f"Export failed: {str(e)}")
        else:
            self.report({'INFO'}, "Skipped FTS export")
        
        # Update LLF file with new vertex lighting data
        scene = bpy.context.scene
        lighting_props = scene.arx_lighting

        # Generate LLF path if it doesn't exist
        llf_path = area_files.llf
        if export_llf and not llf_path:
            # Create LLF path in the correct directory (graph/levels/levelX/levelX.llf)
            import os
            # Build the correct path based on area_id
            addon_path = addon.sceneManager.dataPath
            llf_path = os.path.join(addon_path, "graph", "levels", f"level{area_id}", f"level{area_id}.llf")
            print(f"INFO: Generated LLF path for area {area_id}: {llf_path}")

        if export_llf and llf_path and lighting_props.regenerate_lighting and lighting_props.lighting_method != 'SKIP':
            try:
                self.updateLlfFile(llf_path, self.converted_faces, fts_data)
                self.report({'INFO'}, f"Successfully updated LLF lighting data using {lighting_props.lighting_method}")
            except Exception as e:
                self.report({'ERROR'}, f"LLF update failed: {str(e)}")
                # Don't fail the entire export if LLF update fails
        elif lighting_props.lighting_method == 'SKIP':
            self.report({'INFO'}, "Skipped LLF lighting update (fast export mode)")
        else:
            # Debug why lighting wasn't exported
            reasons = []
            if not export_llf:
                reasons.append("export_llf=False")
            if not llf_path:
                reasons.append("no LLF path")
            if not lighting_props.regenerate_lighting:
                reasons.append("regenerate_lighting=False")
            if lighting_props.lighting_method == 'SKIP':
                reasons.append("method=SKIP")
            self.report({'INFO'}, f"LLF lighting update disabled ({', '.join(reasons)})")
        
        # Update DLF file with entity data
        if export_dlf and area_files.dlf:
            try:
                self.updateDlfFile(area_files.dlf, scene, area_id)
                self.report({'INFO'}, f"Successfully updated DLF entity data")
            except Exception as e:
                self.report({'ERROR'}, f"DLF update failed: {str(e)}")
                # Don't fail the entire export if DLF update fails
    
    def convertMeshToFtsCells(self, mesh_obj, fts_data):
        """Convert Blender mesh back to FTS cell format"""
        import bmesh
        
        # Create bmesh from mesh
        bm = bmesh.new()
        bm.from_mesh(mesh_obj.data)
        bm.faces.ensure_lookup_table()
        
        # Build material index mapping from Blender to FTS
        material_mapping, fts_data = self._buildMaterialMapping(mesh_obj, fts_data)
        
        # Get UV and vertex color layers
        uv_layer = bm.loops.layers.uv.active
        color_layer = bm.loops.layers.color.active
        
        # Get FTS polygon property layers
        transval_layer = bm.faces.layers.float.get('arx_transval')
        area_layer = bm.faces.layers.float.get('arx_area')
        room_layer = bm.faces.layers.int.get('arx_room')
        polytype_layer = bm.faces.layers.int.get('arx_polytype')
        
        # Get preserved geometric data layers
        norm_layer = bm.faces.layers.float_vector.get('arx_norm')
        norm2_layer = bm.faces.layers.float_vector.get('arx_norm2')
        vertex_norms_layer = bm.faces.layers.string.get('arx_vertex_normals')
        tex_index_layer = bm.faces.layers.int.get('arx_tex_index')
        
        # Get preserved cell coordinate layers for exact round-trip
        cell_x_layer = bm.faces.layers.int.get('arx_cell_x')
        cell_z_layer = bm.faces.layers.int.get('arx_cell_z')
        cell_valid_layer = bm.faces.layers.int.get('arx_cell_valid')

        # DO NOT remove cell coordinate layers - they contain critical preserved data!
        # These coordinates are essential for maintaining the original FTS structure
        
        if not uv_layer:
            raise ArxException("Background mesh missing UV coordinates")
        
        # Check for preserved FTS data - warn but don't fail if missing
        has_preserved_data = bool(transval_layer and cell_x_layer and cell_z_layer)
        if not cell_valid_layer:
            print("WARNING: Mesh has no arx_cell_valid layer, so no cell coordinate is "
                  "trusted and every polygon is binned from its centre. Reimport the "
                  "level to restore exact cell placement.")

        if not has_preserved_data:
            print(f"WARNING: Mesh missing FTS polygon properties - will use defaults for new/modified faces")
            print(f"  transval_layer: {transval_layer is not None}")
            print(f"  cell_x_layer: {cell_x_layer is not None}") 
            print(f"  cell_z_layer: {cell_z_layer is not None}")
        
        # Convert faces back to Arx format
        converted_faces = []
        quad_count = 0
        triangle_count = 0
        fallback_normal = 0
        fallback_vertex_normals = 0
        fallback_area = 0
        fallback_cell = 0
        tiny_area = 0
        for face in bm.faces:
            # Validate face geometry
            if len(face.verts) < 3:
                print(f"WARNING: Skipping degenerate face with {len(face.verts)} vertices")
                continue
            if len(face.verts) > 4:
                print(f"WARNING: Face has {len(face.verts)} vertices, only quads and triangles supported")
                continue
                
            # Convert face vertices back to Arx coordinates
            arx_vertices = []
            loop_normals = []
            for loop in face.loops:
                # Convert position back to Arx coordinates (reverse the 0.1 scaling and coordinate transform)
                blender_pos = loop.vert.co
                arx_pos_tuple = blender_pos_to_arx(blender_pos)
                arx_pos = Vector(arx_pos_tuple) * 10.0  # Reverse 0.1 scale factor
                
                
                # Get UV coordinates (flip V coordinate back)
                uv = loop[uv_layer].uv if uv_layer else (0.0, 0.0)
                arx_uv = (uv[0], 1.0 - uv[1])
                
                # Calculate vertex lighting - either use preserved lightmap or recalculate
                if hasattr(self, '_preserve_original_lighting') and self._preserve_original_lighting and color_layer:
                    # Use preserved lighting for unmodified faces
                    color = loop[color_layer]
                    arx_color = (int(color[0] * 255), int(color[1] * 255), int(color[2] * 255), int(color[3] * 255))
                else:
                    # For LLF export, skip lighting calculation here - it will be done via Cycles baking
                    # Use neutral gray as placeholder (will be replaced by Cycles baking)
                    arx_color = (128, 128, 128, 255)

                    # Debug: Only show message once
                    if not hasattr(self, '_skip_lighting_message_shown'):
                        print("DEBUG: Skipping manual lighting calculation during conversion (will use Cycles baking for LLF)")
                        self._skip_lighting_message_shown = True
                
                arx_vertices.append({
                    'pos': arx_pos,
                    'uv': arx_uv,
                    'color': arx_color
                })
                # Blender's own vertex normal, in Arx space, kept in loop order.
                # Used for faces that were modelled here rather than imported.
                loop_normals.append(Vector(blender_pos_to_arx(loop.vert.normal)))

            # Preserved geometry data only exists on faces that came from an FTS
            # file. A face modelled in Blender still has the layers - joining an
            # object into the background mesh gives it every existing attribute -
            # but they read back as zero, so test the value, not the layer.
            # A zero normal is not cosmetic: the engine plane-tests against it in
            # IntersectLinePlane, so such a face is unlit AND has no collision.
            preserved_normal = Vector(face[norm_layer]) if norm_layer else Vector((0.0, 0.0, 0.0))
            preserved_normal2 = Vector(face[norm2_layer]) if norm2_layer else Vector((0.0, 0.0, 0.0))

            if preserved_normal.length > 1e-6:
                # Use preserved original normals
                arx_normal = preserved_normal
                arx_normal2 = preserved_normal2 if preserved_normal2.length > 1e-6 else preserved_normal
            else:
                fallback_normal += 1
                # For new geometry, calculate normal from ORIGINAL vertex order (before swap)
                # The engine expects normals calculated from the un-swapped vertex order
                if len(arx_vertices) >= 3:
                    v0 = Vector(arx_vertices[0]['pos'])
                    v1 = Vector(arx_vertices[1]['pos'])
                    # For quads, we need to use the ORIGINAL vertex order for normal calculation
                    # Since we're about to swap vertices 2 and 3, we calculate before the swap
                    if len(face.verts) == 4 and len(arx_vertices) == 4:
                        # Use original vertex 3 (which will become vertex 2 after swap)
                        v2 = Vector(arx_vertices[3]['pos'])
                    else:
                        v2 = Vector(arx_vertices[2]['pos'])

                    edge1 = v1 - v0
                    edge2 = v2 - v0
                    calculated_normal = edge1.cross(edge2).normalized()
                    arx_normal = calculated_normal

                    if len(converted_faces) < 3:
                        print(f"DEBUG: Face {len(converted_faces)} normal calculated from original vertex order: {calculated_normal}")
                else:
                    # Fallback to Blender normal
                    blender_normal = face.normal
                    arx_normal = Vector(blender_pos_to_arx(blender_normal))

                arx_normal2 = arx_normal

            # Reverse the vertex order swap that was done during import for quads
            # During import, FTS vertices [0,1,2,3] were swapped to Blender [0,1,3,2]
            # Now we need to swap back: Blender [0,1,3,2] -> FTS [0,1,2,3]
            vertex_order_swapped = False
            if len(face.verts) == 4 and len(arx_vertices) == 4:
                # Swap vertices 2 and 3 to restore FTS order
                arx_vertices[2], arx_vertices[3] = arx_vertices[3], arx_vertices[2]
                loop_normals[2], loop_normals[3] = loop_normals[3], loop_normals[2]
                vertex_order_swapped = True
            
            # Get preserved vertex normals
            vertex_normals = []
            if vertex_norms_layer:
                import struct
                vertex_norm_data = face[vertex_norms_layer]
                if len(vertex_norm_data) >= 48:  # 4 normals × 3 floats × 4 bytes
                    for i in range(4):
                        offset = i * 12  # 3 floats × 4 bytes
                        x, y, z = struct.unpack('<fff', vertex_norm_data[offset:offset+12])
                        vertex_normals.append(Vector((x, y, z)))

            # Nothing preserved, so fall back to Blender's vertex normals, which
            # are the closest thing to what DANAE stored per vertex. They are
            # already in FTS order because they followed the swap above.
            if not vertex_normals:
                vertex_normals = list(loop_normals)
                fallback_vertex_normals += 1

            # Pad any remaining slot with the face normal
            while len(vertex_normals) < 4:
                vertex_normals.append(arx_normal)
            
            # Get stored FTS properties or calculate from geometry
            transval = face[transval_layer] if transval_layer else 0.0
            # Same trap as the normals: the layer exists on faces modelled in Blender
            # but reads back as 0. Area is not decoration - the collision walker
            # rejects any polygon under 100 square units outright
            # (Collisions.cpp, IsPolyInSphere and the CFLAG_EXTRA_PRECISION gate),
            # so a zero here means the face is drawn but cannot be walked into.
            stored_area = face[area_layer] if area_layer else 0.0
            if stored_area <= 0.0:
                # Calculate area in Arx units (Blender area × scale factor²)
                blender_area = face.calc_area()
                stored_area = blender_area * (10.0 * 10.0)  # Scale factor is 10.0, area scales by square
                fallback_area += 1
                if stored_area < 100.0:
                    tiny_area += 1
            # A negative room id means "belongs to no room" and is what the original
            # data uses for polygons outside the portal system. The engine reads it as
            # an empty RoomHandle (Mesh.cpp, loadFastScene) and only rejects ids at or
            # above nb_rooms + 1, so it must be preserved: forcing it to 0 moves those
            # polygons into room 0 and corrupts that room's contents.
            room_id = face[room_layer] if room_layer else 0
            # Use current material assignment instead of preserved texture index
            # This ensures texture changes in Blender are reflected in the export
            blender_mat_index = face.material_index
            fts_texture_id = material_mapping.get(blender_mat_index, 0)
            if blender_mat_index != 0 and fts_texture_id == 0:
                print(f"WARNING: Face {face.index} has material index {blender_mat_index} but no FTS texture mapping found")  # Default to texture 0 if not found
            
            # Calculate polygon type from actual geometry
            is_quad = len(face.verts) == 4
            if polytype_layer:
                poly_type = face[polytype_layer]
            else:
                # Default polygon type - calculate flag value directly to avoid ctypes
                # POLY_QUAD flag is bit 6 (value 64), POLY_DOUBLESIDED is bit 1 (value 2)
                # If faces have backface culling issues, set POLY_DOUBLESIDED flag (poly_type |= 2)
                poly_type = 64 if is_quad else 0
            
            # Get preserved cell coordinates. arx_cell_valid marks the faces that
            # actually came out of an FTS cell; a face modelled in Blender inherits
            # the coordinate layers but reads 0 from them, which is a real cell, so
            # without the flag all new geometry piles into cell (0, 0).
            cell_x = None
            cell_z = None
            if cell_x_layer and cell_z_layer:
                # Try to get preserved coordinates
                try:
                    if cell_valid_layer and face[cell_valid_layer]:
                        cell_x = face[cell_x_layer]
                        cell_z = face[cell_z_layer]
                except:
                    # Layer exists but face doesn't have value
                    pass

            has_preserved_cell_coords = (cell_x is not None and cell_z is not None)
            if not has_preserved_cell_coords:
                fallback_cell += 1

            if not has_preserved_cell_coords and len(converted_faces) < 5:
                print(f"DEBUG: No preserved cell coordinates for face {len(converted_faces)} - will calculate during grid generation")
            
            # Debug: log room values from Blender face data
            if len(converted_faces) < 5:
                print(f"DEBUG: Blender face {len(converted_faces)}: room_id={room_id}")
            
            # Count quad vs triangle faces
            if is_quad:
                quad_count += 1
            else:
                triangle_count += 1
            
            # Build complete FTS polygon data structure using preserved geometric data
            fts_polygon = {
                'vertices': arx_vertices,
                'material_index': face.material_index,
                # Faces with an unsupported vertex count are skipped above, so the
                # position in converted_faces is not the bmesh face index. Record the
                # real one for anything that has to read back off the mesh.
                'bmesh_index': face.index,
                'is_quad': is_quad,
                # FTS-specific polygon properties (preserved from original)
                'transval': transval,
                'area': stored_area,  # Use preserved area value
                'room': room_id,
                'poly_type': poly_type,
                'norm': arx_normal,
                'norm2': arx_normal2,  # Use preserved secondary normal
                'vertex_normals': vertex_normals[:4],  # Use preserved per-vertex normals
                'tex': fts_texture_id,  # Use current material assignment
            }
            
            # Only add cell coordinates if they were preserved from mesh data
            if has_preserved_cell_coords:
                fts_polygon['cell_x'] = cell_x
                fts_polygon['cell_z'] = cell_z
            
            converted_faces.append(fts_polygon)
        
        # Store converted data for potential FTS writing
        # NOTE: This doesn't actually update fts_data.cells yet - that requires 
        # implementing the full FTS write functionality
        self.converted_faces = converted_faces
        
        print(f"QUAD/TRIANGLE COUNT: {quad_count} quads, {triangle_count} triangles, {len(converted_faces)} total faces")
        print(f"FALLBACKS: {fallback_normal} face normals, {fallback_vertex_normals} vertex normal sets, "
              f"{fallback_area} areas, {fallback_cell} cell coordinates computed from geometry")
        if tiny_area:
            print(f"WARNING: {tiny_area} faces are under 100 square Arx units. The engine's "
                  f"collision walker rejects those outright, so they will be drawn but not "
                  f"solid. Arx level polygons are around 100 units across - scale imported "
                  f"models up, or expect to walk through them.")
        self.report({'INFO'}, f"Converted {len(converted_faces)} faces from Blender mesh ({quad_count} quads, {triangle_count} triangles)")
        
        bm.free()
        return fts_data
    
    def _buildMaterialMapping(self, mesh_obj, fts_data):
        """Build mapping from Blender material indices to FTS texture indices"""
        material_mapping = {}
        
        # Get mesh materials
        mesh_materials = mesh_obj.data.materials
        
        if not mesh_materials:
            return {0: 0}, fts_data  # Default mapping
        
        
        for blender_idx, material in enumerate(mesh_materials):
            if material is None:
                # Empty material slot
                material_mapping[blender_idx] = 0
                continue
                
            material_name = material.name
            
            # Extract image path from Blender material if available
            image_path = self._extractImagePathFromMaterial(material)
            
            # Try to find matching FTS texture by name
            fts_tex_index = None
            fts_texture_to_update = None
            
            for fts_idx, fts_texture in enumerate(fts_data.textures):
                if isinstance(fts_texture, dict):
                    fts_name = fts_texture['fic'].decode('iso-8859-1').rstrip('\x00')
                else:
                    fts_name = fts_texture.fic.decode('iso-8859-1').rstrip('\x00')
                
                # Improved matching logic - extract base name from both sides
                material_base = material_name.replace('-mat', '').lower()
                fts_base = fts_name.replace('\\', '/').split('/')[-1].lower()  # Get filename only
                fts_base = fts_base.replace('.jpg', '').replace('.tga', '').replace('.bmp', '')
                
                if material_base == fts_base or material_base in fts_base or fts_base in material_base:
                    if isinstance(fts_texture, dict):
                        fts_tex_index = fts_texture['tc']  # Use texture container ID, not array index
                        fts_tc = fts_texture['tc']
                    else:
                        fts_tex_index = fts_texture.tc
                        fts_tc = fts_texture.tc
                    fts_texture_to_update = fts_texture
                    break
            
            # Update FTS texture path only if user actually changed the image path
            if fts_texture_to_update and image_path:
                # Extract current FTS filename for comparison
                if isinstance(fts_texture_to_update, dict):
                    current_fts_path = fts_texture_to_update['fic'].decode('iso-8859-1').rstrip('\x00')
                else:
                    current_fts_path = fts_texture_to_update.fic.decode('iso-8859-1').rstrip('\x00')
                current_filename = current_fts_path.replace('\\', '/').split('/')[-1]
                current_base = current_filename.replace('.jpg', '').replace('.tga', '').replace('.bmp', '').lower()
                image_base = image_path.lower()
                
                # Only update if the user actually changed the texture
                if current_base != image_base:
                    # Reconstruct proper FTS path format
                    new_fts_path = f"GRAPH\\OBJ3D\\TEXTURES\\{image_path.upper()}.BMP"
                    new_texture = self._updateFtsTexturePath(fts_texture_to_update, new_fts_path)
                    if new_texture:
                        # Create new texture list (can't modify namedtuple directly)
                        new_textures = list(fts_data.textures)
                        for i, tex in enumerate(new_textures):
                            tex_tc = tex['tc'] if isinstance(tex, dict) else tex.tc
                            fts_to_update_tc = fts_texture_to_update['tc'] if isinstance(fts_texture_to_update, dict) else fts_texture_to_update.tc
                            if tex_tc == fts_to_update_tc:
                                new_textures[i] = new_texture
                                break
                        # Replace the entire FTS data with updated textures (namedtuple is immutable)
                        fts_data = fts_data._replace(textures=new_textures)
                        pass
            
            if fts_tex_index is None:
                # Add new texture to FTS texture list
                fts_tex_index, fts_data = self._addNewTexture(fts_data, material_name, image_path)
                
            material_mapping[blender_idx] = fts_tex_index
        
        return material_mapping, fts_data
    
    def _extractImagePathFromMaterial(self, material):
        """Extract image file path from Blender material node tree"""
        import os
        
        if not material or not material.use_nodes or not material.node_tree:
            return None
            
        # Look for Image Texture nodes in the material
        for node in material.node_tree.nodes:
            if node.bl_idname == 'ShaderNodeTexImage' and node.image:
                # Get the image file path
                image_path = node.image.filepath
                if image_path:
                    # Extract just the filename for FTS format matching
                    # Don't try to preserve paths - just get the base name
                    filename = os.path.basename(image_path)
                    
                    # Remove extension as FTS format doesn't include it
                    base_name = os.path.splitext(filename)[0]
                    return base_name
        
        return None
    
    def _updateFtsTexturePath(self, fts_texture, new_path):
        """Update FTS texture container with new image path"""
        if not new_path:
            return
            
        # Encode new path as ISO-8859-1 and ensure it fits in 256 bytes
        encoded_path = new_path.encode('iso-8859-1', errors='replace')
        if len(encoded_path) >= 256:
            encoded_path = encoded_path[:255]  # Leave room for null terminator
        
        # Create new texture as Python dict to avoid ctypes read-only issues
        path_bytes = encoded_path + b'\x00' * (256 - len(encoded_path))
        
        if isinstance(fts_texture, dict):
            new_texture = {
                'tc': fts_texture['tc'],
                'temp': fts_texture['temp'],
                'fic': path_bytes
            }
        else:
            new_texture = {
                'tc': fts_texture.tc,
                'temp': fts_texture.temp,
                'fic': path_bytes
            }
        
        # Return the new texture dict to replace the old one
        return new_texture
    
    def _storeOriginalFtsDataInScene(self, scene, fts_data):
        """Store complete original FTS data in scene custom properties for persistence across save/load"""
        import pickle
        print("DEBUG: Storing complete original FTS data in scene properties")
        
        # Store critical non-geometry data that must be preserved exactly
        try:
            # Store scene offset
            scene["arx_scene_offset"] = fts_data.sceneOffset
            print(f"DEBUG: Storing scene offset: {fts_data.sceneOffset}")
            
            # Store textures as serialized data  
            texture_data = []
            for i, tex in enumerate(fts_data.textures):
                try:
                    if isinstance(tex, dict):
                        # Validate dict format
                        if 'tc' in tex and 'temp' in tex and 'fic' in tex:
                            texture_data.append(tex)  # Already in correct format
                        else:
                            print(f"WARNING: Invalid texture dict at index {i}: {tex}")
                            texture_data.append({'tc': 0, 'temp': 0, 'fic': b'default.bmp' + b'\x00' * 245})
                    else:
                        # Convert ctypes to dict with proper type conversion
                        texture_data.append({
                            'tc': int(tex.tc) if hasattr(tex, 'tc') else 0,
                            'temp': int(tex.temp) if hasattr(tex, 'temp') else 0,
                            'fic': bytes(tex.fic) if hasattr(tex, 'fic') else b'default.bmp' + b'\x00' * 245
                        })
                except Exception as e:
                    print(f"WARNING: Failed to process texture {i}: {e}")
                    # Add fallback texture
                    texture_data.append({'tc': i, 'temp': 0, 'fic': b'default.bmp' + b'\x00' * 245})
            
            scene["arx_texture_data"] = pickle.dumps(texture_data)
            print(f"DEBUG: Stored {len(texture_data)} textures")
            
            # Store anchors - convert ALL ctypes to Python types
            anchor_data = []
            print(f"DEBUG: Processing {len(fts_data.anchors)} anchors")
            for i, anchor in enumerate(fts_data.anchors):
                if i < 3:  # Debug first few anchors
                    print(f"DEBUG: Anchor {i}: {type(anchor)}, length={len(anchor)}, content={anchor}")
                if len(anchor) >= 5:  # New format with preserved data
                    anchor_pos, anchor_links, radius, height, flags = anchor
                    # Convert all ctypes to Python types
                    if hasattr(anchor_pos, 'x'):  # SavedVec3 structure
                        pos_tuple = (float(anchor_pos.x), float(anchor_pos.y), float(anchor_pos.z))
                    else:
                        pos_tuple = tuple(float(x) for x in anchor_pos)  # Convert to tuple of floats
                    
                    link_list = list(anchor_links) if hasattr(anchor_links, '__iter__') else [anchor_links]
                    anchor_data.append((pos_tuple, link_list, float(radius), float(height), int(flags)))
                else:  # Old format fallback
                    anchor_pos, anchor_links = anchor[:2]
                    if hasattr(anchor_pos, 'x'):  # SavedVec3 structure
                        pos_tuple = (float(anchor_pos.x), float(anchor_pos.y), float(anchor_pos.z))
                    else:
                        pos_tuple = tuple(float(x) for x in anchor_pos)
                    
                    link_list = list(anchor_links) if hasattr(anchor_links, '__iter__') else [anchor_links]
                    anchor_data.append((pos_tuple, link_list))
            scene["arx_anchor_data"] = pickle.dumps(anchor_data)
            print(f"DEBUG: Stored {len(anchor_data)} anchors")
            
            # Store cell anchors - convert any ctypes arrays to lists
            cell_anchor_data = []
            for z_row in fts_data.cell_anchors:
                z_row_data = []
                for cell_anchors in z_row:
                    if cell_anchors is not None:
                        z_row_data.append(list(cell_anchors) if hasattr(cell_anchors, '__iter__') else cell_anchors)
                    else:
                        z_row_data.append(None)
                cell_anchor_data.append(z_row_data)
            scene["arx_cell_anchor_data"] = pickle.dumps(cell_anchor_data)
            print(f"DEBUG: Stored cell anchor data")
            
            # Store portals as binary data
            portal_data = []
            for portal in fts_data.portals:
                portal_data.append(bytes(portal))  # Serialize entire portal structure
            scene["arx_portal_data"] = pickle.dumps(portal_data)
            print(f"DEBUG: Stored {len(portal_data)} portals")
            
            # Store room data - handle ctypes arrays carefully
            if hasattr(fts_data, 'room_data') and fts_data.room_data:
                print(f"DEBUG: Processing room data")
                room_data_list, room_distances = fts_data.room_data
                
                # Serialize room structures as binary
                serialized_rooms = []
                for room_info, room_portal_indices, room_poly_refs in room_data_list:
                    # Convert ctypes arrays to lists for pickling
                    portal_indices_list = list(room_portal_indices) if room_portal_indices else []
                    
                    # Handle room_info - convert ctypes to dict if needed to avoid pickle issues
                    if isinstance(room_info, dict):
                        # Already a dict, serialize as bytes
                        room_info_bytes = pickle.dumps(room_info)
                    else:
                        # Convert ctypes structure to dict to avoid c_int_Array issues
                        room_info_dict = {
                            'nb_portals': room_info.nb_portals,
                            'nb_polys': room_info.nb_polys,
                            'padd': [room_info.padd[j] for j in range(6)]  # Convert ctypes array to list
                        }
                        room_info_bytes = pickle.dumps(room_info_dict)
                    
                    serialized_rooms.append({
                        'room_info_bytes': room_info_bytes,
                        'portal_indices': portal_indices_list,
                        'poly_refs': [bytes(ref) for ref in room_poly_refs]  # Serialize polygon references
                    })
                
                print(f"DEBUG: Processed {len(serialized_rooms)} room structures")
                
                # Serialize distance matrix - handle ROOM_DIST_DATA_SAVE structures
                serialized_distances = []
                for row in room_distances:
                    serialized_row = []
                    for dist in row:
                        try:
                            # Convert to bytes (should work for ctypes structures)
                            dist_bytes = bytes(dist)
                            serialized_row.append(dist_bytes)
                        except Exception as e:
                            print(f"WARNING: Failed to serialize distance data: {e}")
                            # Create a simple fallback distance structure
                            fallback_bytes = b'\x00' * 28  # Size of ROOM_DIST_DATA_SAVE (1 float + 2 Vec3s = 4 + 12 + 12 = 28 bytes)
                            serialized_row.append(fallback_bytes)
                    serialized_distances.append(serialized_row)
                
                scene["arx_room_data"] = pickle.dumps((serialized_rooms, serialized_distances))
                print(f"DEBUG: Stored room data: {len(serialized_rooms)} rooms")
            
            print(f"DEBUG: Stored FTS data: {len(fts_data.textures)} textures, {len(fts_data.portals)} portals")
            
        except Exception as e:
            print(f"WARNING: Failed to store FTS data in scene properties: {e}")
    
    def _restoreOriginalFtsDataFromScene(self, scene, base_fts_data):
        """Restore complete original FTS data from scene custom properties using pure Python structures"""
        import pickle
        print("DEBUG: Restoring original FTS data from scene properties")
        
        try:
            # Restore textures as pure Python dicts (not ctypes)
            if "arx_texture_data" in scene:
                texture_data = pickle.loads(scene["arx_texture_data"])
                # Keep as pure Python dicts - don't create ctypes here
                base_fts_data = base_fts_data._replace(textures=texture_data)
            
            # Restore anchors (already Python tuples/lists)
            if "arx_anchor_data" in scene:
                anchors = pickle.loads(scene["arx_anchor_data"])
                base_fts_data = base_fts_data._replace(anchors=anchors)
            
            # Restore cell anchors (already Python lists)
            if "arx_cell_anchor_data" in scene:
                cell_anchors = pickle.loads(scene["arx_cell_anchor_data"])
                base_fts_data = base_fts_data._replace(cell_anchors=cell_anchors)
            
            # Restore scene offset
            if "arx_scene_offset" in scene:
                scene_offset = scene["arx_scene_offset"]
                base_fts_data = base_fts_data._replace(sceneOffset=scene_offset)
            
            # Restore portals as binary data (don't convert to ctypes yet)
            if "arx_portal_data" in scene:
                portal_bytes_list = pickle.loads(scene["arx_portal_data"])
                # Store as binary data, convert to ctypes only during final serialization
                base_fts_data = base_fts_data._replace(portals=portal_bytes_list)
            
            # Restore room data as pure Python structures
            if "arx_room_data" in scene:
                room_data = pickle.loads(scene["arx_room_data"])
                serialized_rooms, serialized_distances = room_data
                
                # Keep as pure Python structures - don't create ctypes here
                restored_room_list = []
                for room_data_dict in serialized_rooms:
                    # Convert bytes back to dict but keep as Python data
                    room_info_dict = {
                        'nb_portals': len(room_data_dict['portal_indices']),
                        'nb_polys': 0,  # Will be rebuilt by room reconstruction
                        'padd': [0] * 6
                    }
                    portal_indices = room_data_dict['portal_indices']
                    
                    # Keep polygon references empty - will be rebuilt
                    restored_room_list.append((room_info_dict, portal_indices, []))
                
                # Keep distance matrix as binary data
                base_fts_data = base_fts_data._replace(room_data=(restored_room_list, serialized_distances))
            
            print("DEBUG: Successfully restored FTS data as pure Python structures")
            return base_fts_data
            
        except Exception as e:
            print(f"WARNING: Failed to restore FTS data from scene properties: {e}")
            return base_fts_data
    
    def _addNewTexture(self, fts_data, material_name, image_path=None):
        """Create new FTS texture entry for new material"""
        print(f"WARNING: Material '{material_name}' not found in FTS textures")
        
        if image_path:
            print(f"DEBUG: Creating new FTS texture for image path '{image_path}'")
            
            # Generate new texture container ID - use highest existing tc + 1
            max_tc = max(((tex['tc'] if isinstance(tex, dict) else tex.tc) for tex in fts_data.textures), default=0)
            new_tc = max_tc + 1
            
            # Set texture path in proper FTS format
            fts_path = f"GRAPH\\OBJ3D\\TEXTURES\\{image_path.upper()}.BMP"
            encoded_path = fts_path.encode('iso-8859-1', errors='replace')
            if len(encoded_path) >= 256:
                encoded_path = encoded_path[:255]  # Leave room for null terminator
            
            # Create new texture as Python dict to avoid ctypes issues
            path_bytes = encoded_path + b'\x00' * (256 - len(encoded_path))
            new_texture = {
                'tc': new_tc,
                'temp': 0,  # Standard temp value
                'fic': path_bytes
            }
            
            # Create new texture list and update fts_data (namedtuple is immutable)
            new_textures = list(fts_data.textures)
            new_textures.append(new_texture)
            fts_data = fts_data._replace(textures=new_textures)
            
            print(f"DEBUG: Created new FTS texture tc={new_tc} for path '{fts_path}'")
            return new_tc, fts_data
        else:
            print(f"DEBUG: No image path provided, using fallback texture")
            
            # Try to find a reasonable fallback texture
            if fts_data.textures:
                # Use first texture as fallback (index 0 in array, but use its tc value)
                if isinstance(fts_data.textures[0], dict):
                    fallback_tc = fts_data.textures[0]['tc']
                    fallback_name = fts_data.textures[0]['fic'].decode('iso-8859-1').rstrip('\x00')
                else:
                    fallback_tc = fts_data.textures[0].tc
                    fallback_name = fts_data.textures[0].fic.decode('iso-8859-1').rstrip('\x00')
                print(f"DEBUG: Using fallback texture tc={fallback_tc} ('{fallback_name}') for material '{material_name}'")
                return fallback_tc, fts_data
            else:
                # No textures available, use tc=0 as absolute fallback
                print(f"DEBUG: No textures in FTS, using tc=0 for material '{material_name}'")
                return 0, fts_data
    
    def _rebuildRoomPolygonReferences(self, fts_data):
        """Rebuild room polygon references (EP_DATA) efficiently to fix topology changes"""
        print("DEBUG: Rebuilding room polygon references to fix topology changes")

        if not hasattr(fts_data, 'room_data') or not fts_data.room_data:
            print("DEBUG: No room data to rebuild - disabling room system entirely")
            # Can't modify namedtuple directly - return without room data
            return

        room_data_list, room_distances = fts_data.room_data

        # EP_DATA addresses a polygon as (cell x, cell z, index within that cell), so
        # it has to be derived from the very grid the FTS writer emits. Building a
        # second grid here drifted from the real one, because that one drops faces
        # that fall outside the 160x160 world and this one used to clamp them in.
        cell_grid = self._reconstructCellGrid(self.converted_faces, fts_data)

        # Now build room polygon references with correct indices
        room_polygon_refs = {}  # room_id -> [(cell_x, cell_z, poly_idx), ...]

        for cell_z in range(160):
            for cell_x in range(160):
                if cell_grid[cell_z][cell_x] is not None:
                    # For each polygon in this cell
                    for poly_idx_in_cell, poly in enumerate(cell_grid[cell_z][cell_x]):
                        room_id = poly.get('room', 0)
                        if room_id < 0:
                            # No room, so it gets no EP_DATA entry anywhere.
                            continue
                        if room_id not in room_polygon_refs:
                            room_polygon_refs[room_id] = []
                        # poly_idx_in_cell is the actual index within the cell's polygon list
                        room_polygon_refs[room_id].append((cell_x, cell_z, poly_idx_in_cell))
        
        # Find the maximum room ID actually used
        max_room_id = max(room_polygon_refs.keys()) if room_polygon_refs else 0
        max_room_id = max(max_room_id, len(room_data_list) - 1)  # Ensure we don't shrink existing rooms
        
        print(f"DEBUG: Expanding room data to support room IDs 0-{max_room_id}")
        
        # Rebuild room structures with simple Python data (not ctypes) for ALL room IDs
        new_room_data_list = []
        for room_idx in range(max_room_id + 1):
            
            # Get portal indices for this room (if it exists in old data)
            room_portal_indices = []
            if room_idx < len(room_data_list):
                _, room_portal_indices, _ = room_data_list[room_idx]
            
            # Create new room info as simple dict (not ctypes)
            new_room_info = {
                'nb_portals': len(room_portal_indices) if room_portal_indices else 0,
                'nb_polys': 0,
                'padd': [0] * 6  # Simple Python list instead of ctypes array
            }
            
            # Build new polygon references as simple dicts (not ctypes)
            new_poly_refs = []
            
            if room_idx in room_polygon_refs:
                for cell_x, cell_z, poly_idx in room_polygon_refs[room_idx]:
                    ep_data = {
                        'px': cell_x,
                        'py': cell_z,
                        'idx': poly_idx,
                        'padd': 0
                    }
                    new_poly_refs.append(ep_data)
                
                new_room_info['nb_polys'] = len(new_poly_refs)
                
            new_room_data_list.append((new_room_info, room_portal_indices, new_poly_refs))
            print(f"DEBUG: Room {room_idx}: {new_room_info['nb_polys']} polygons")
        
        # Check portal-room connectivity for debugging
        portal_room_errors = 0
        if hasattr(fts_data, 'portals') and fts_data.portals:
            for portal_idx, portal_data in enumerate(fts_data.portals):
                # Extract room connections from portal (assuming portal structure has room_1, room_2)
                # This would need to be adapted based on actual portal structure
                if portal_idx < 5:  # Debug first few portals
                    print(f"DEBUG: Portal {portal_idx} connects rooms (data available but structure analysis needed)")
        
        # Expand room distance matrix if needed
        current_matrix_size = len(room_distances) if room_distances else 0
        required_matrix_size = max_room_id + 1
        
        if required_matrix_size > current_matrix_size:
            print(f"DEBUG: Expanding room distance matrix from {current_matrix_size}x{current_matrix_size} to {required_matrix_size}x{required_matrix_size}")
            
            # Create new larger distance matrix
            new_room_distances = []
            for i in range(required_matrix_size):
                row = []
                for j in range(required_matrix_size):
                    if i < current_matrix_size and j < current_matrix_size and room_distances:
                        # Copy existing distance data
                        row.append(room_distances[i][j])
                    else:
                        # No route known for a room that was added here. Leave the
                        # whole entry at zero: the writer fills those in from the
                        # portal graph, and if it cannot, the engine reads zero as
                        # "no route" and falls back to straight line distance. A
                        # large distance with the waypoints on the world origin
                        # makes SP_GetRoomDist return about a million instead, which
                        # puts every entity in that room outside the treat zone -
                        # frozen and invisible until the player walks in.
                        from .dataFts import ROOM_DIST_DATA_SAVE
                        row.append(bytes(ROOM_DIST_DATA_SAVE()))
                new_room_distances.append(row)
            room_distances = new_room_distances
        
        # Update the FTS data with rebuilt room references using namedtuple _replace
        # Note: This function needs to return the updated fts_data since namedtuples are immutable
        print(f"DEBUG: Rebuilt room references for {len(new_room_data_list)} rooms (max room ID: {max_room_id})")
        return fts_data._replace(room_data=(new_room_data_list, room_distances))
    
    def _disablePortalSystem(self, fts_data):
        """Disable portal/room system to prevent engine fullbright fallback when geometry is modified"""
        print("DEBUG: Disabling portal/room system due to geometry modifications")
        
        # Clear room data to disable room-based rendering
        empty_room_data = ([], [])  # Empty room list and distance matrix
        
        # Clear portals to disable portal culling
        empty_portals = []
        
        # Update FTS data to disable portal system
        fts_data = fts_data._replace(
            room_data=empty_room_data,
            portals=empty_portals
        )
        
        print("DEBUG: Portal system disabled - engine should use basic rendering with vertex lighting")
        return fts_data
    
    def _rebuildPortalSystemFromBlender(self, fts_data, scene):
        """Read portal objects from Blender scene and rebuild portal system"""
        from .dataFts import EERIE_SAVE_PORTALS, SAVE_EERIEPOLY
        from mathutils import Vector
        
        # Find portals collection
        portals_collection = None
        for collection in scene.collection.children:
            if 'portals' in collection.name.lower():
                portals_collection = collection
                break
        
        if not portals_collection:
            print("DEBUG: No portals collection found - keeping original portals")
            return fts_data
        
        print(f"DEBUG: Found portals collection '{portals_collection.name}' with {len(portals_collection.objects)} objects")
        
        # Convert Blender portal objects back to FTS format
        new_portals = []
        for portal_obj in portals_collection.objects:
            if portal_obj.type != 'MESH' or not portal_obj.data.polygons:
                continue
                
            # Get the first face from the portal mesh
            mesh = portal_obj.data
            if len(mesh.polygons) == 0:
                continue
                
            face = mesh.polygons[0]
            if len(face.vertices) < 3:
                continue
            
            # Get world-space vertex positions
            portal_vertices = []
            for i in range(4):  # Portals are quads
                if i < len(face.vertices):
                    vert_idx = face.vertices[i]
                    local_pos = mesh.vertices[vert_idx].co
                    world_pos = portal_obj.matrix_world @ local_pos
                    # Convert back to Arx coordinates
                    arx_pos = blender_pos_to_arx(world_pos)
                    arx_pos = Vector(arx_pos) * 10.0  # Reverse 0.1 scale
                    portal_vertices.append(arx_pos)
                else:
                    # Pad with last vertex for triangles
                    portal_vertices.append(portal_vertices[-1] if portal_vertices else Vector((0,0,0)))
            
            # Reverse vertex order to match original import order swap
            portal_vertices[2], portal_vertices[3] = portal_vertices[3], portal_vertices[2]

            # Read room connections from custom properties
            room_1 = portal_obj.get('arx_room_1', 0)
            room_2 = portal_obj.get('arx_room_2', 1)

            # The plane the engine uses comes from the first three stored corners,
            # not from anything written into norm, so compute it the same way.
            normal = portal_plane_normal(portal_vertices)

            # ARX_PORTALS_Frustrum_ComputeRoom travels from room_1 when the camera
            # is on the side the normal points into, and from room_2 otherwise, so
            # a portal whose winding disagrees with its room ids connects
            # backwards. Reversing a Z order quad is a swap of the middle two
            # corners, which keeps it a valid quad.
            towards = self._roomDirection(room_1, portal_vertices)
            if towards is not None and normal.dot(towards) < 0.0:
                portal_vertices[1], portal_vertices[2] = portal_vertices[2], portal_vertices[1]
                normal = portal_plane_normal(portal_vertices)
                print(f"DEBUG: Flipped portal winding so its normal faces room {room_1}")
            useportal = portal_obj.get('arx_useportal', 1)

            print(f"DEBUG: Portal {len(new_portals)}: connects room {room_1} ↔ room {room_2}")

            # Create portal data as dictionary (compatible with FTS serializer)

            # Bounding sphere for the portal. The engine reads this straight out of
            # v[0].rhw (Mesh.cpp, portal.bounds.radius) and never recomputes it, unlike
            # background polys. It is used for the portal visibility test in
            # ARX_PORTALS_Frustrum_ComputeRoom, so a wrong value hides the portal and
            # the room behind it never gets drawn.
            portal_center = Vector((0.0, 0.0, 0.0))
            for pv in portal_vertices:
                portal_center += pv
            portal_center /= len(portal_vertices)
            portal_radius = max((pv - portal_center).length for pv in portal_vertices)

            # Build vertex data
            vertices = []
            vertex_normals = []
            for i in range(4):
                if i < len(portal_vertices):
                    pos = portal_vertices[i]
                    vertices.append({
                        'x': pos.x, 'y': pos.y, 'z': pos.z,
                        'rhw': portal_radius, 'color': 0xFFFFFFFF, 'specular': 0,
                        'tu': 0.0 if i % 2 == 0 else 1.0,
                        'tv': 0.0 if i < 2 else 1.0
                    })
                    vertex_normals.append({'x': normal.x, 'y': normal.y, 'z': normal.z})
                else:
                    # Duplicate last vertex for triangles
                    vertices.append(vertices[-1])
                    vertex_normals.append(vertex_normals[-1])
            
            # Calculate bounding box
            min_x = min(v.x for v in portal_vertices) if portal_vertices else 0
            max_x = max(v.x for v in portal_vertices) if portal_vertices else 0
            min_y = min(v.y for v in portal_vertices) if portal_vertices else 0
            max_y = max(v.y for v in portal_vertices) if portal_vertices else 0
            min_z = min(v.z for v in portal_vertices) if portal_vertices else 0
            max_z = max(v.z for v in portal_vertices) if portal_vertices else 0
            
            # Create portal polygon dictionary with proper PolyTypeFlag values
            # POLY_QUAD is bit 6, so value is 64 (0x40) for quads, 0 for triangles
            poly_type_value = 64 if len(face.vertices) == 4 else 0  # POLY_QUAD bit
            portal_poly = {
                'vertices': vertices,
                'vertex_normals': vertex_normals,
                'poly_type': poly_type_value,  # PolyTypeFlag bit values
                'is_quad': len(face.vertices) == 4,
                'norm': {'x': normal.x, 'y': normal.y, 'z': normal.z},
                'norm2': {'x': normal.x, 'y': normal.y, 'z': normal.z},
                'min': {'x': min_x, 'y': min_y, 'z': min_z},
                'max': {'x': max_x, 'y': max_y, 'z': max_z},
                'center': {'x': (min_x + max_x) / 2, 'y': (min_y + max_y) / 2, 'z': (min_z + max_z) / 2},
                'tex': -1,  # No texture for portals
                'transval': 0.0,
                'area': 1.0,
                'room': room_1,
                'misc': 0
            }
            
            # Create complete portal structure as ctypes (for FTS serialization)
            from .dataFts import EERIE_SAVE_PORTALS, SAVE_EERIEPOLY
            
            # Create the ctypes structures
            portal_poly_struct = SAVE_EERIEPOLY()
            portal_struct = EERIE_SAVE_PORTALS()
            
            # Fill the polygon structure from dictionary data
            for i, vertex in enumerate(portal_poly['vertices']):
                portal_poly_struct.v[i].pos.x = vertex['x']
                portal_poly_struct.v[i].pos.y = vertex['y'] 
                portal_poly_struct.v[i].pos.z = vertex['z']
                portal_poly_struct.v[i].rhw = vertex['rhw']
                portal_poly_struct.v[i].color = vertex['color']
                portal_poly_struct.v[i].specular = vertex['specular']
                portal_poly_struct.v[i].tu = vertex['tu']
                portal_poly_struct.v[i].tv = vertex['tv']
                portal_poly_struct.tv[i] = portal_poly_struct.v[i]
                
                # Set vertex normal
                vnorm = portal_poly['vertex_normals'][i]
                portal_poly_struct.nrml[i].x = vnorm['x']
                portal_poly_struct.nrml[i].y = vnorm['y']
                portal_poly_struct.nrml[i].z = vnorm['z']
            
            # Set polygon properties (type is c_int32, not PolyTypeFlag)
            portal_poly_struct.type = portal_poly['poly_type']
            portal_poly_struct.norm.x = portal_poly['norm']['x']
            portal_poly_struct.norm.y = portal_poly['norm']['y']
            portal_poly_struct.norm.z = portal_poly['norm']['z']
            portal_poly_struct.norm2 = portal_poly_struct.norm
            portal_poly_struct.min.x = portal_poly['min']['x']
            portal_poly_struct.min.y = portal_poly['min']['y']
            portal_poly_struct.min.z = portal_poly['min']['z']
            portal_poly_struct.max.x = portal_poly['max']['x']
            portal_poly_struct.max.y = portal_poly['max']['y']
            portal_poly_struct.max.z = portal_poly['max']['z']
            portal_poly_struct.center.x = portal_poly['center']['x']
            portal_poly_struct.center.y = portal_poly['center']['y']
            portal_poly_struct.center.z = portal_poly['center']['z']
            portal_poly_struct.tex = portal_poly['tex']
            portal_poly_struct.transval = portal_poly['transval']
            portal_poly_struct.area = portal_poly['area']
            portal_poly_struct.room = portal_poly['room']
            portal_poly_struct.misc = portal_poly['misc']
            
            # Set portal structure
            portal_struct.poly = portal_poly_struct
            portal_struct.room_1 = room_1
            portal_struct.room_2 = room_2
            portal_struct.useportal = useportal
            portal_struct.paddy = 0
            
            # Store as bytes for FTS serialization
            new_portals.append(bytes(portal_struct))
        
        print(f"DEBUG: Rebuilt {len(new_portals)} portals from Blender scene")
        self._validatePortals(new_portals)

        # Update FTS data with new portals
        return fts_data._replace(portals=new_portals)

    def _validatePortals(self, portals):
        """Report portals whose room links cannot be right.

        Rooms and portals are authored by hand, so nothing here changes the data.
        The point is that a mistake shows up at export instead of as an unlit hole
        in the level: duplicating a portal in Blender copies its room ids along
        with everything else, which silently leaves the room it was moved to
        unreachable and therefore never drawn.
        """
        from .dataFts import EERIE_SAVE_PORTALS

        rooms_in_use = {face.get('room', 0) for face in self.converted_faces
                        if face.get('room', 0) > 0}

        linked = set()
        seen = {}
        problems = []

        for index, portal in enumerate(portals):
            data = (EERIE_SAVE_PORTALS.from_buffer_copy(portal)
                    if isinstance(portal, bytes) else portal)
            room_1, room_2 = data.room_1, data.room_2
            linked.update((room_1, room_2))

            centre = tuple(sum(data.poly.v[k].pos.__getattribute__(axis)
                               for k in range(4)) / 4.0 for axis in ('x', 'y', 'z'))
            where = f"({centre[0]:.0f}, {centre[1]:.0f}, {centre[2]:.0f})"

            if room_1 == room_2:
                problems.append(f"portal {index} at {where} joins room {room_1} to itself")
            for room in (room_1, room_2):
                if room > 0 and room not in rooms_in_use:
                    problems.append(f"portal {index} at {where} names room {room}, "
                                    f"which no polygon belongs to")

            # Two portals joining the same pair of rooms is perfectly normal - a
            # room can have several doorways to its neighbour, and level 1 has six
            # such pairs. What gives a copy away is that it is the same size to the
            # last decimal while sitting somewhere else entirely.
            key = (min(room_1, room_2), max(room_1, room_2))
            radius = data.poly.v[0].rhw
            for other, other_centre, other_radius in seen.get(key, []):
                gap = math.sqrt(sum((centre[i] - other_centre[i]) ** 2 for i in range(3)))
                if gap > 1.0 and abs(radius - other_radius) < 1e-3:
                    problems.append(f"portals {other} and {index} join rooms {key[0]} "
                                    f"and {key[1]}, are exactly the same size and sit "
                                    f"{gap:.0f} units apart; one looks like a copy that "
                                    f"kept the original's rooms")
            seen.setdefault(key, []).append((index, centre, radius))

        unreachable = sorted(rooms_in_use - linked)
        if unreachable:
            problems.append(f"rooms {unreachable} have no portal, so nothing can open "
                            f"them and they will never be drawn")

        for problem in problems:
            print(f"WARNING: {problem}")
        if problems:
            self.report({'WARNING'}, f"{len(problems)} portal problem(s), see the console")
    
    def _roomDirection(self, room, portal_vertices):
        """Direction from a portal towards the middle of one of its rooms.

        Returns None when that room has no geometry to average, in which case the
        caller has nothing to check the portal's facing against and should leave
        the winding as the author built it.
        """
        total = Vector((0.0, 0.0, 0.0))
        count = 0
        for face in self.converted_faces:
            if face.get('room', 0) != room:
                continue
            for vertex in face['vertices']:
                total += Vector(vertex['pos'])
                count += 1

        if not count:
            return None

        centre = Vector((0.0, 0.0, 0.0))
        for corner in portal_vertices:
            centre += corner
        centre /= len(portal_vertices)

        direction = (total / count) - centre
        return direction.normalized() if direction.length > 1e-9 else None

    def _rebuildAnchorNetworkFromBlender(self, fts_data, scene):
        """Read anchor network from Blender anchor mesh and rebuild anchor system"""
        from mathutils import Vector
        
        # Find anchors collection or objects
        anchors_collection = None
        anchor_objects = []
        
        # Look for anchors collection first
        for collection in scene.collection.children:
            if 'anchors' in collection.name.lower():
                anchors_collection = collection
                break
        
        if anchors_collection:
            anchor_objects = [obj for obj in anchors_collection.objects if obj.type == 'MESH']
            print(f"DEBUG: Found anchors collection '{anchors_collection.name}' with {len(anchor_objects)} anchor objects")
        else:
            # Look for individual anchor objects in main collection
            anchor_objects = [obj for obj in scene.collection.objects if 'anchor' in obj.name.lower() and obj.type == 'MESH']
            if anchor_objects:
                print(f"DEBUG: Found {len(anchor_objects)} anchor objects in main collection")
        
        if not anchor_objects:
            print("DEBUG: No anchor objects found - keeping original anchors")
            return fts_data
        
        # Read anchor positions and connections from mesh
        new_anchors = []
        new_cell_anchors = [[[] for _ in range(160)] for _ in range(160)]
        
        # Process anchor mesh to extract anchor network
        for anchor_obj in anchor_objects:
            mesh = anchor_obj.data
            if not mesh.vertices:
                continue
            
            cylinders = read_anchor_attributes(mesh)

            # Get anchor positions from vertices
            anchor_positions = []
            for vertex in mesh.vertices:
                world_pos = anchor_obj.matrix_world @ vertex.co
                # Convert back to Arx coordinates
                arx_pos = blender_pos_to_arx(world_pos)
                arx_pos = Vector(arx_pos) * 10.0  # Reverse 0.1 scale
                anchor_positions.append((arx_pos.x, arx_pos.y, arx_pos.z))
            
            # Build anchor connectivity from mesh edges
            anchor_links = [[] for _ in range(len(anchor_positions))]
            for edge in mesh.edges:
                v1, v2 = edge.vertices
                if v1 < len(anchor_links) and v2 < len(anchor_links):
                    anchor_links[v1].append(v2)
                    anchor_links[v2].append(v1)  # Bidirectional links
            
            # Create anchor data with preserved properties or defaults
            base_anchor_index = len(new_anchors)
            for i, pos in enumerate(anchor_positions):
                global_index = base_anchor_index + i
                
                # Each anchor's own cylinder, as measured when it was generated.
                radius = cylinders['radius'][i]
                height = cylinders['height'][i]
                flags = int(cylinders['flags'][i])
                
                # Convert local links to global indices
                global_links = [base_anchor_index + link for link in anchor_links[i]]
                
                anchor_data = (pos, global_links, radius, height, flags)
                new_anchors.append(anchor_data)
                
                # Add anchor to appropriate cell for spatial indexing
                # Convert position to cell coordinates
                scene_offset = fts_data.sceneOffset if hasattr(fts_data, 'sceneOffset') else (0, 0, 0)
                relative_x = pos[0] - scene_offset[0]
                relative_z = pos[2] - scene_offset[2]
                cell_x = max(0, min(159, int(relative_x / 100)))
                cell_z = max(0, min(159, int(relative_z / 100)))
                
                new_cell_anchors[cell_z][cell_x].append(global_index)
        
        # Convert cell anchor lists to proper format (None for empty cells)
        for z in range(160):
            for x in range(160):
                if not new_cell_anchors[z][x]:
                    new_cell_anchors[z][x] = None
        
        print(f"DEBUG: Rebuilt {len(new_anchors)} anchors from Blender anchor mesh")
        
        # Update FTS data with new anchor network
        return fts_data._replace(anchors=new_anchors, cell_anchors=new_cell_anchors)
    
    def _assignRoomsToNewGeometry(self, fts_data):
        """Assign room IDs to new faces based on spatial connectivity"""
        print("DEBUG: Assigning room IDs to new geometry based on spatial analysis")
        
        # This is a simplified approach - would need sophisticated spatial analysis
        # For now, assign room IDs based on proximity to existing geometry
        
        faces_updated = 0
        for face_data in self.converted_faces:
            # If face has no room assignment (new geometry)
            if face_data.get('room', 0) == 0:
                # Simple heuristic: find nearest existing face with room assignment
                new_room = self._findNearestRoom(face_data)
                face_data['room'] = new_room
                faces_updated += 1
        
        if faces_updated > 0:
            print(f"DEBUG: Assigned room IDs to {faces_updated} new faces")
        
        return fts_data
    
    def _findNearestRoom(self, new_face):
        """Find the most appropriate room ID for a new face"""
        # Get center position of new face
        vertices = new_face.get('vertices', [])
        if not vertices:
            return 1  # Default room
            
        # Calculate face center
        center_x = sum(v['pos'][0] for v in vertices) / len(vertices)
        center_y = sum(v['pos'][1] for v in vertices) / len(vertices) 
        center_z = sum(v['pos'][2] for v in vertices) / len(vertices)
        new_center = Vector((center_x, center_y, center_z))
        
        # Find closest existing face with room assignment
        closest_room = 1
        min_distance = float('inf')
        
        for existing_face in self.converted_faces:
            existing_room = existing_face.get('room', 0)
            if existing_room <= 0:
                continue
                
            # Calculate distance to existing face
            existing_vertices = existing_face.get('vertices', [])
            if existing_vertices:
                existing_center_x = sum(v['pos'][0] for v in existing_vertices) / len(existing_vertices)
                existing_center_y = sum(v['pos'][1] for v in existing_vertices) / len(existing_vertices)
                existing_center_z = sum(v['pos'][2] for v in existing_vertices) / len(existing_vertices)
                existing_center = Vector((existing_center_x, existing_center_y, existing_center_z))
                
                distance = (new_center - existing_center).length
                if distance < min_distance:
                    min_distance = distance
                    closest_room = existing_room
        
        return closest_room
    
    def updateLlfFile(self, llf_path, converted_faces, fts_data):
        """Update LLF file with new vertex lighting data using Cycles renderer"""
        from .dataLlf import DANAE_LLF_HEADER, DANAE_LS_LIGHTINGHEADER, SavedColorBGRA
        from ctypes import sizeof
        import struct
        
        print(f"DEBUG: Updating LLF file with Cycles lighting for {len(converted_faces)} faces")

        # Try to read original LLF file, or create default if it doesn't exist
        addon = getAddon(bpy.context)
        try:
            original_llf_data = addon.sceneManager.llfSerializer.read(llf_path)
        except FileNotFoundError:
            print(f"INFO: LLF file doesn't exist, creating new one: {llf_path}")
            # Create minimal default LLF data structure
            from .dataLlf import LlfData
            from ctypes import c_ubyte

            # LlfData only has two fields: lights and levelLighting
            # Create empty arrays for now - they'll be replaced with calculated colors
            original_llf_data = LlfData(
                lights=[],           # No lights to preserve
                levelLighting=[]     # Will be replaced with our calculated vertex colors
            )
        
        # Get the background mesh object for Cycles lighting calculation
        scene = bpy.context.scene
        background_obj = None
        for obj in scene.objects:
            if obj.type == 'MESH' and obj.name.endswith('-background'):
                background_obj = obj
                break
        
        if not background_obj:
            print("ERROR: Could not find background mesh for Cycles lighting")
            self.report({'ERROR'}, "Cannot calculate lighting without background mesh")
            return
        
        print(f"DEBUG: Using {scene.arx_lighting.lighting_method} lighting calculation on mesh: {background_obj.name}")

        # Every method has to emit colours in FTS write order, because that is the
        # order the engine reads them back in RestoreLastLoadedLightning. Walking
        # converted_faces instead produces a scrambled lightmap and, once any face
        # falls outside the grid, the wrong number of values as well.
        ordered_polys = self._orderedPolygons(fts_data)

        # Calculate vertex lighting based on selected method
        try:
            if scene.arx_lighting.lighting_method == 'DANAE':
                vertex_lighting_colors = self._calculateDanaeVertexLighting(ordered_polys, scene)
                print(f"DEBUG: DANAE lighting calculated {len(vertex_lighting_colors)} vertex colors")
            elif scene.arx_lighting.lighting_method == 'CYCLES':
                vertex_lighting_colors = self._calculateCyclesVertexLighting(converted_faces, background_obj, scene, fts_data)
                print(f"DEBUG: Cycles calculated {len(vertex_lighting_colors)} vertex colors")
            elif scene.arx_lighting.lighting_method == 'SIMPLE':
                # Simple lighting calculation
                vertex_lighting_colors = []
                for poly in ordered_polys:
                    vertex_count = 4 if poly.get('is_quad', False) else 3

                    # Simple top-down lighting
                    # Only output the actual number of vertices (3 for triangles, 4 for quads)
                    for i in range(vertex_count):
                        brightness = int(scene.arx_lighting.ambient_strength * 255)
                        vertex_lighting_colors.append((brightness, brightness, brightness, 255))
                print(f"DEBUG: Simple lighting applied to {len(vertex_lighting_colors)} vertices")
            elif scene.arx_lighting.lighting_method == 'PRESERVE':
                # Preserve existing vertex colors from mesh
                vertex_lighting_colors = []
                mesh = background_obj.data
                vcol_layer = None
                for vcol in mesh.vertex_colors:
                    if vcol.name == "light-color":
                        vcol_layer = vcol
                        break

                if not vcol_layer:
                    raise Exception("No light-color vertex color layer found for PRESERVE mode")

                # Read existing colors in face order
                import bmesh
                bm = bmesh.new()
                bm.from_mesh(mesh)
                bm.faces.ensure_lookup_table()

                color_layer = None
                for layer in bm.loops.layers.color:
                    if layer.name == "light-color":
                        color_layer = layer
                        break

                for poly in ordered_polys:
                    is_quad = poly.get('is_quad', False)
                    vertex_count = 4 if is_quad else 3
                    face_idx = poly.get('bmesh_index', -1)

                    if 0 <= face_idx < len(bm.faces):
                        blender_face = bm.faces[face_idx]
                        # Read colors in standard order for triangles
                        loop_indices = [0, 1, 2] if not is_quad else [0, 1, 3, 2]

                        colors_for_face = []
                        for idx in loop_indices[:vertex_count]:
                            if idx < len(blender_face.loops) and color_layer:
                                loop = blender_face.loops[idx]
                                color = loop[color_layer]
                                colors_for_face.append((
                                    int(color[0] * 255),
                                    int(color[1] * 255),
                                    int(color[2] * 255),
                                    255
                                ))
                            else:
                                colors_for_face.append((128, 128, 128, 255))

                        # Don't duplicate colors for triangles - only output actual vertex count
                        vertex_lighting_colors.extend(colors_for_face)
                    else:
                        # Output correct number of colors based on face type
                        vertex_count = 4 if is_quad else 3
                        for i in range(vertex_count):
                            vertex_lighting_colors.append((128, 128, 128, 255))

                bm.free()
                print(f"DEBUG: Preserved {len(vertex_lighting_colors)} existing vertex colors")
            else:  # SKIP
                raise Exception("SKIP mode should not reach updateLlfFile")
            
            # Convert to SavedColorBGRA format for LLF with intensity multiplier.
            # The DANAE bake already produces exactly what the engine expects, so it
            # is written through untouched; the multiplier only trims the renderer
            # based methods, whose output is in a different scale to begin with.
            lighting_props = scene.arx_lighting
            if scene.arx_lighting.lighting_method == 'DANAE':
                intensity_mult = 1.0
            else:
                intensity_mult = lighting_props.export_intensity_multiplier
                print(f"DEBUG: Applying export intensity multiplier: {intensity_mult}")

            new_lighting_data = []
            for color in vertex_lighting_colors:
                bgra_color = SavedColorBGRA()
                # Apply the multiplier to RGB values (not alpha)
                bgra_color.r = min(255, int(color[0] * intensity_mult))
                bgra_color.g = min(255, int(color[1] * intensity_mult))
                bgra_color.b = min(255, int(color[2] * intensity_mult))
                bgra_color.a = color[3]  # Keep alpha unchanged
                new_lighting_data.append(bgra_color)
            
        except Exception as e:
            print(f"ERROR: {scene.arx_lighting.lighting_method} lighting failed: {e}")
            import traceback
            traceback.print_exc()
            self.report({'ERROR'}, f"Lighting calculation failed: {str(e)}")
            return
        
        # Write updated LLF file
        self._writeLlfFile(llf_path, original_llf_data.lights, new_lighting_data)
        print(f"DEBUG: Updated LLF with {len(new_lighting_data)} vertex colors")
    
    def updateDlfFile(self, dlf_path, scene, area_id):
        """Update DLF file with entity data from Blender scene"""
        from .dataDlf import DANAE_LS_HEADER, DANAE_LS_INTER
        from .arx_io_util import blender_pos_to_arx
        from mathutils import Vector, Euler
        import time
        
        print(f"DEBUG: Updating DLF file for area {area_id}")
        
        # Read original DLF file
        addon = getAddon(bpy.context)
        original_dlf_data = addon.sceneManager.dlfSerializer.readContainer(dlf_path)
        
        # Get scene offset from scene properties
        scene_offset = scene.get("arx_scene_offset", [0, 0, 0])
        
        # Find entity collection in scene
        entities_collection = None
        for collection in scene.collection.children:
            if collection.name.endswith('-entities'):
                entities_collection = collection
                break
        
        if not entities_collection:
            print("WARNING: No entities collection found in scene")
            return
        
        # Convert Blender objects to DLF format
        new_entities = []
        new_lights = []
        new_fogs = []
        new_paths = []
        new_zones = []
        
        # Process all objects in scene to gather different DLF components
        for collection in scene.collection.children:
            if not collection.objects:
                continue
                
            for obj in collection.objects:
                if obj.name.startswith('e:'):
                    # Regular entity objects
                    entity_name = obj.get("arx_entity_name", "")
                    entity_ident = obj.get("arx_entity_ident", 0)
                    entity_flags = obj.get("arx_entity_flags", 0)
                    
                    # Skip entities with invalid or empty names
                    if not entity_name or not entity_name.strip():
                        print(f"DEBUG: Skipping entity {obj.name} with empty name")
                        continue
                    
                    entity = DANAE_LS_INTER()
                    # Ensure entity name is properly null-terminated
                    name_bytes = entity_name.encode('iso-8859-1', errors='replace')[:511]
                    entity.name = name_bytes + b'\x00' * (512 - len(name_bytes))
                    
                    # Properly reverse the import transformation:
                    # Import: proxyObject.location = arx_pos_to_blender_for_model(sceneOffset + arx_pos) * 0.1
                    # Export: arx_pos = (blender_pos / 0.1) reverse_transform - sceneOffset
                    blender_pos = obj.location
                    arx_pos = Vector(blender_pos_to_arx(blender_pos / 0.1)) - Vector(scene_offset)
                    entity.pos.x = arx_pos.x
                    entity.pos.y = arx_pos.y 
                    entity.pos.z = arx_pos.z
                    
                    # Properly reverse the rotation transformation using the correct inverse
                    if obj.rotation_mode == 'QUATERNION':
                        blender_quat = obj.rotation_quaternion.copy()
                    else:
                        blender_quat = obj.rotation_euler.to_quaternion()
                    
                    # First, reverse the Z correction applied during import
                    z_correction_inverse = Quaternion((0, 0, 1), -math.radians(90))  # -90 degrees around Z
                    corrected_quat = z_correction_inverse @ blender_quat
                    
                    # Now use blender_to_arx_transform to properly reverse the transformation
                    from .arx_io_animation import blender_to_arx_transform
                    _, arx_rot, _ = blender_to_arx_transform(
                        Vector((0, 0, 0)), corrected_quat, Vector((1, 1, 1)), 0.1,
                        flip_w=True, flip_x=False, flip_y=True, flip_z=False
                    )
                    
                    # Convert quaternion back to Arx Euler angles (a=pitch, b=yaw, g=roll)
                    euler = arx_rot.to_euler('XYZ')
                    entity.angle.a = math.degrees(euler.x)  # pitch
                    entity.angle.b = math.degrees(euler.y)  # yaw  
                    entity.angle.g = math.degrees(euler.z)  # roll
                    entity.ident = entity_ident
                    entity.flags = entity_flags
                    
                    new_entities.append(entity)
                    
                elif obj.name.startswith('path:'):
                    # Convert path objects to DANAE_LS_PATH + DANAE_LS_PATHWAYS
                    from .dataDlf import DANAE_LS_PATH, DANAE_LS_PATHWAYS
                    
                    path = DANAE_LS_PATH()
                    path_name = obj.name[5:]  # Remove 'path:' prefix
                    name_bytes = path_name.encode('iso-8859-1', errors='replace')[:63]
                    path.name = name_bytes + b'\x00' * (64 - len(name_bytes))
                    
                    path.idx = obj.get("arx_path_idx", 0)
                    path.flags = obj.get("arx_path_flags", 0)
                    path.height = obj.get("arx_path_height", 0)
                    
                    ambiance = obj.get("arx_path_ambiance", "")
                    ambiance_bytes = ambiance.encode('iso-8859-1', errors='replace')[:127]
                    path.ambiance = ambiance_bytes + b'\x00' * (128 - len(ambiance_bytes))
                    
                    path.reverb = obj.get("arx_path_reverb", 0.0)
                    path.farclip = obj.get("arx_path_farclip", 0.0)
                    path.amb_max_vol = obj.get("arx_path_amb_max_vol", 0.0)
                    
                    # Convert path object position (same transformation as entities)
                    blender_pos = obj.location
                    arx_pos = Vector(blender_pos_to_arx(blender_pos / 0.1)) - Vector(scene_offset)
                    print(f"DEBUG: Path '{path_name}' export: blender_pos={blender_pos}, arx_pos={arx_pos}")
                    path.pos.x = arx_pos.x
                    path.pos.y = arx_pos.y
                    path.pos.z = arx_pos.z
                    path.initpos.x = arx_pos.x  # Same as pos
                    path.initpos.y = arx_pos.y
                    path.initpos.z = arx_pos.z
                    
                    # Find child waypoint objects and convert to pathways
                    pathways = []
                    waypoint_objects = []
                    
                    # Collect waypoint children
                    for child in obj.children:
                        if child.name.startswith('waypoint:'):
                            waypoint_objects.append(child)
                    
                    # Sort by pathway index to maintain order
                    waypoint_objects.sort(key=lambda w: w.get("arx_pathway_index", 0))
                    
                    for waypoint_obj in waypoint_objects:
                        pathway = DANAE_LS_PATHWAYS()
                        
                        # Convert waypoint position to relative Arx coordinates
                        # Since waypoint_obj is a child of path obj, waypoint_obj.location is already in local coordinates
                        # Just convert the local position to Arx coordinates
                        arx_relative_pos = Vector(blender_pos_to_arx(waypoint_obj.location / 0.1))
                        print(f"DEBUG: Waypoint '{waypoint_obj.name}' export: blender_local={waypoint_obj.location}, arx_relative={arx_relative_pos}")
                        
                        pathway.rpos.x = arx_relative_pos.x
                        pathway.rpos.y = arx_relative_pos.y
                        pathway.rpos.z = arx_relative_pos.z
                        
                        # Get pathway properties from waypoint object
                        pathway.flag = waypoint_obj.get("arx_pathway_flag", 0)
                        pathway.time = waypoint_obj.get("arx_pathway_time", 0)
                        
                        pathways.append(pathway)
                    
                    path.nb_pathways = len(pathways)
                    
                    if path.height != 0:
                        new_zones.append((path, pathways))
                    else:
                        new_paths.append((path, pathways))
                    
                elif obj.name.startswith('zone:'):
                    # Convert zone objects to DANAE_LS_PATH (with height != 0)
                    from .dataDlf import DANAE_LS_PATH, DANAE_LS_PATHWAYS
                    
                    zone = DANAE_LS_PATH()
                    zone_name = obj.name[5:]  # Remove 'zone:' prefix
                    name_bytes = zone_name.encode('iso-8859-1', errors='replace')[:63]
                    zone.name = name_bytes + b'\x00' * (64 - len(name_bytes))
                    
                    zone.idx = obj.get("arx_zone_idx", 0)
                    zone.flags = obj.get("arx_zone_flags", 0)
                    zone.height = obj.get("arx_zone_height", 1)  # Zones must have height != 0
                    
                    ambiance = obj.get("arx_zone_ambiance", "")
                    ambiance_bytes = ambiance.encode('iso-8859-1', errors='replace')[:127]
                    zone.ambiance = ambiance_bytes + b'\x00' * (128 - len(ambiance_bytes))
                    print(f"DEBUG: Zone '{zone_name}' ambiance: '{ambiance}'")
                    
                    zone.reverb = obj.get("arx_zone_reverb", 0.0)
                    zone.farclip = obj.get("arx_zone_farclip", 0.0)
                    zone.amb_max_vol = obj.get("arx_zone_amb_max_vol", 0.0)

                    rgb = obj.get("arx_zone_rgb", (0.0, 0.0, 0.0))
                    zone.rgb.r, zone.rgb.g, zone.rgb.b = rgb[0], rgb[1], rgb[2]
                    
                    # Convert position (same transformation as entities)
                    blender_pos = obj.location
                    arx_pos = Vector(blender_pos_to_arx(blender_pos / 0.1)) - Vector(scene_offset)
                    zone.pos.x = arx_pos.x
                    zone.pos.y = arx_pos.y
                    zone.pos.z = arx_pos.z
                    zone.initpos.x = arx_pos.x  # Same as pos
                    zone.initpos.y = arx_pos.y
                    zone.initpos.z = arx_pos.z
                    
                    # The outline mesh is the current shape of the zone; the older
                    # scatter of child empties is still read so scenes built before
                    # the mesh outline existed keep working.
                    zone_pathways = []
                    outline = next((child for child in obj.children
                                    if child.name.startswith('zone_outline:')
                                    and child.type == 'MESH'), None)

                    if outline is not None:
                        mesh = outline.data
                        flags = mesh.attributes.get("arx_pathway_flag")
                        times = mesh.attributes.get("arx_pathway_time")
                        # Vertex order is the polygon winding, so it has to be kept.
                        for index, vertex in enumerate(mesh.vertices):
                            pathway = DANAE_LS_PATHWAYS()
                            local = outline.matrix_local @ vertex.co
                            arx_relative_pos = Vector(blender_pos_to_arx(local / 0.1))
                            pathway.rpos.x = arx_relative_pos.x
                            pathway.rpos.y = arx_relative_pos.y
                            pathway.rpos.z = arx_relative_pos.z
                            pathway.flag = flags.data[index].value if flags else 0
                            pathway.time = times.data[index].value if times else 0
                            zone_pathways.append(pathway)
                    else:
                        waypoints = [child for child in obj.children
                                     if child.name.startswith('zone_waypoint:')]
                        waypoints.sort(key=lambda w: w.get("arx_pathway_index", 0))
                        for waypoint_obj in waypoints:
                            pathway = DANAE_LS_PATHWAYS()
                            arx_relative_pos = Vector(blender_pos_to_arx(waypoint_obj.location / 0.1))
                            pathway.rpos.x = arx_relative_pos.x
                            pathway.rpos.y = arx_relative_pos.y
                            pathway.rpos.z = arx_relative_pos.z
                            pathway.flag = waypoint_obj.get("arx_pathway_flag", 0)
                            pathway.time = waypoint_obj.get("arx_pathway_time", 0)
                            zone_pathways.append(pathway)

                    if len(zone_pathways) < 3:
                        print(f"WARNING: Zone '{zone_name}' has {len(zone_pathways)} "
                              f"points; a zone needs at least three to enclose anything")

                    zone.nb_pathways = len(zone_pathways)
                    new_zones.append((zone, zone_pathways))
                    
                elif obj.name.startswith('fog:'):
                    # Convert fog objects to DANAE_LS_FOG
                    from .dataDlf import DANAE_LS_FOG
                    
                    fog = DANAE_LS_FOG()
                    
                    # Convert position (same transformation as entities)
                    blender_pos = obj.location
                    arx_pos = Vector(blender_pos_to_arx(blender_pos / 0.1)) - Vector(scene_offset)
                    fog.pos.x = arx_pos.x
                    fog.pos.y = arx_pos.y
                    fog.pos.z = arx_pos.z
                    
                    # Convert rotation
                    if obj.rotation_mode == 'QUATERNION':
                        euler = obj.rotation_quaternion.to_euler('XYZ')
                    else:
                        euler = obj.rotation_euler
                    
                    fog.angle.a = math.degrees(euler.x)
                    fog.angle.b = math.degrees(euler.z)
                    fog.angle.g = math.degrees(euler.y)
                    
                    # Convert properties from custom properties
                    fog.size = obj.get("arx_fog_size", 100.0)
                    fog.special = obj.get("arx_fog_special", 0)
                    fog.scale = obj.get("arx_fog_scale", 1.0)
                    fog.speed = obj.get("arx_fog_speed", 0.0)
                    fog.rotatespeed = obj.get("arx_fog_rotatespeed", 0.0)
                    fog.tolive = obj.get("arx_fog_tolive", 0)
                    fog.blend = obj.get("arx_fog_blend", 0)
                    fog.frequency = obj.get("arx_fog_frequency", 0.0)
                    
                    # Convert color from object color
                    if hasattr(obj, 'color'):
                        fog.rgb.r = int(obj.color[0] * 255)
                        fog.rgb.g = int(obj.color[1] * 255)
                        fog.rgb.b = int(obj.color[2] * 255)
                    else:
                        fog.rgb.r = 255
                        fog.rgb.g = 255
                        fog.rgb.b = 255
                    
                    new_fogs.append(fog)
                    
                elif obj.name.startswith('light:') and obj.type == 'LIGHT':
                    # Convert DLF light objects to DANAE_LS_LIGHT
                    from .dataDlf import DANAE_LS_LIGHT
                    
                    light = DANAE_LS_LIGHT()
                    
                    # Convert position (same transformation as entities)
                    blender_pos = obj.location
                    arx_pos = Vector(blender_pos_to_arx(blender_pos / 0.1)) - Vector(scene_offset)
                    light.pos.x = arx_pos.x
                    light.pos.y = arx_pos.y
                    light.pos.z = arx_pos.z
                    
                    # Convert light properties
                    if obj.data:
                        light.intensity = obj.data.energy / 100.0
                        light.fallend = obj.data.distance / 0.1 if obj.data.distance > 0 else 1000.0
                        
                        # Convert color
                        color = obj.data.color
                        light.rgb.r = int(color[0] * 255)
                        light.rgb.g = int(color[1] * 255) 
                        light.rgb.b = int(color[2] * 255)
                    
                    # Get additional properties from custom properties
                    light.fallstart = obj.get("arx_light_fallstart", 10.0)
                    light.fallend = obj.get("arx_light_fallend", light.fallend)
                    light.intensity = obj.get("arx_light_intensity", light.intensity)
                    light.extras = obj.get("arx_light_extras", 0)
                    
                    new_lights.append(light)
        
        print(f"DEBUG: Converted {len(new_entities)} entities, {len(new_lights)} lights, {len(new_fogs)} fogs, {len(new_paths)} paths, {len(new_zones)} zones from Blender scene")
        
        # Write updated DLF file
        self._writeDlfFile(dlf_path, original_dlf_data, new_entities, new_lights, new_fogs, new_paths, new_zones)
    
    def _writeDlfFile(self, dlf_path, original_dlf_data, new_entities, new_lights, new_fogs, new_paths, new_zones):
        """Write DLF file with updated scene data"""
        from .dataDlf import DANAE_LS_HEADER, DANAE_LS_LIGHTINGHEADER
        import time
        
        # Create DLF header
        header = DANAE_LS_HEADER()
        header.version = DLF_WRITE_VERSION
        header.ident = b"DANAE_FILE\x00\x00\x00\x00\x00\x00"
        header.lastuser = b"Blender Export\x00" + b"\x00" * (256 - 15)
        header.time = int(time.time())
        
        # Copy most settings from original, then update counts to match what we're actually writing
        if hasattr(original_dlf_data, 'header') and original_dlf_data.header:
            original_header = original_dlf_data.header
            header.pos_edit = original_header.pos_edit
            header.angle_edit = original_header.angle_edit
            header.nb_nodes = original_header.nb_nodes
            header.nb_nodeslinks = original_header.nb_nodeslinks
            header.nb_zones = original_header.nb_zones
            header.lighting = original_header.lighting
            header.nb_bkgpolys = original_header.nb_bkgpolys
            header.nb_ignoredpolys = original_header.nb_ignoredpolys
            header.nb_childpolys = original_header.nb_childpolys
            header.offset = original_header.offset
        
        # Set counts to match what we're actually writing
        header.nb_scn = 1  # Always exactly 1 scene entry
        header.nb_inter = len(new_entities)
        header.nb_lights = len(new_lights) 
        header.nb_fogs = len(new_fogs)
        header.nb_paths = len(new_paths) + len(new_zones)  # Zones are stored as paths with height != 0
        
        # Build DLF payload data (everything except the main header)
        payload_data = bytearray()
        
        # Add scene data (required by DLF format) - this is the level directory path
        if hasattr(original_dlf_data, 'scene') and original_dlf_data.scene:
            payload_data.extend(bytes(original_dlf_data.scene))
            scene_dir = original_dlf_data.scene.name.decode('iso-8859-1').strip('\x00')
            print(f"DEBUG: Added preserved scene directory: '{scene_dir}'")
        else:
            # Create scene data with correct level directory path
            from .dataDlf import DANAE_LS_SCENE
            scene_data = DANAE_LS_SCENE()
            
            # Use the same directory format as original: Graph\Levels\Level1\
            scene_dir = b"Graph\\Levels\\Level1\\"
            # Pad to 512 bytes with null bytes
            scene_data.name = scene_dir + b'\x00' * (512 - len(scene_dir))
            
            payload_data.extend(bytes(scene_data))
            print(f"DEBUG: Added level directory: 'Graph\\Levels\\Level1\\'")
        
        # Add entity data from Blender scene
        for entity in new_entities:
            payload_data.extend(bytes(entity))
        
        # Add lighting data (copy from original if available)
        if hasattr(original_dlf_data, 'lighting_data') and original_dlf_data.lighting_data:
            payload_data.extend(original_dlf_data.lighting_data)
            print(f"DEBUG: Added {len(original_dlf_data.lighting_data)} bytes of lighting data")
        
        # Add lights from Blender scene 
        for light in new_lights:
            payload_data.extend(bytes(light))
        
        # Add fogs from Blender scene
        for fog in new_fogs:
            payload_data.extend(bytes(fog))
        
        # Add nodes data (preserve original for now since zones are handled as paths)
        if hasattr(original_dlf_data, 'nodes_data') and original_dlf_data.nodes_data:
            payload_data.extend(original_dlf_data.nodes_data)
        
        # Add paths and zones from Blender scene (zones are paths with height != 0)
        for path_data, pathways in new_paths:
            payload_data.extend(bytes(path_data))
            for pathway in pathways:
                payload_data.extend(bytes(pathway))
        
        for zone_data, pathways in new_zones:
            payload_data.extend(bytes(zone_data))
            for pathway in pathways:
                payload_data.extend(bytes(pathway))
        
        # Compress payload data using PKWare format
        compressed_data = self._encode_pkware_dlf(payload_data)
        
        # Write DLF file: header + compressed payload
        with open(dlf_path, 'wb') as f:
            f.write(bytes(header))  # Header is uncompressed
            f.write(compressed_data)  # Payload is compressed
        
        print(f"DEBUG: Wrote DLF file with {len(new_entities)} entities, {len(new_lights)} lights, {len(new_fogs)} fogs, {len(new_paths)} paths, {len(new_zones)} zones to {dlf_path}")
    
    
    def _writeLlfFile(self, llf_path, lights, vertex_lighting):
        """Write LLF file with updated lighting data using PKWare compression"""
        from .dataLlf import DANAE_LLF_HEADER, DANAE_LS_LIGHTINGHEADER, SavedColorBGRA
        from ctypes import sizeof
        import time
        
        # The engine decides whether to decompress the .llf from the version in the
        # matching .dlf, not from anything in the .llf itself (DanaeLoadLevel), so
        # the two have to agree. Reading the version off the .dlf on disk does not
        # give that: exportArea writes the .llf first, so on a 1.43 level it read
        # 1.43, wrote the lightmap raw, and _writeDlfFile then replaced the .dlf
        # with a 1.44 one - and the engine blasted the raw lightmap and lost every
        # vertex colour. _writeDlfFile always implodes its payload and so can only
        # ever declare 1.44; the lightmap beside it follows that unconditionally.
        dlf_version = DLF_WRITE_VERSION
        compress = True
        print(f"DEBUG: writing compressed llf for dlf version {dlf_version:.2f}")

        # Create LLF header
        header = DANAE_LLF_HEADER()
        header.version = dlf_version
        header.ident = b"DANAE_LLH_FILE\x00\x00"  # Correct identifier from original file
        header.lastuser = b"Blender Export\x00" + b"\x00" * (256 - 15)
        header.time = int(time.time())
        header.nb_lights = len(lights)
        header.nb_Shadow_Polys = 0
        header.nb_IGNORED_Polys = 0
        header.nb_bkgpolys = len(vertex_lighting) // 4  # Rough estimate
        
        # Create lighting header
        lighting_header = DANAE_LS_LIGHTINGHEADER()
        lighting_header.nb_values = len(vertex_lighting)
        lighting_header.ViewMode = 0
        lighting_header.ModeLight = 63  # Match original file's ModeLight value
        lighting_header.pad = 0
        
        # Build uncompressed binary data
        uncompressed_data = bytearray()
        uncompressed_data.extend(bytes(header))
        
        # Add lights data
        for light in lights:
            uncompressed_data.extend(bytes(light))
        
        # Add lighting header
        uncompressed_data.extend(bytes(lighting_header))
        
        # Add vertex lighting data in BGRA format
        for vertex_color in vertex_lighting:
            # Convert (r,g,b,a) tuple to SavedColorBGRA structure
            if isinstance(vertex_color, (tuple, list)) and len(vertex_color) >= 3:
                r, g, b = vertex_color[:3]
                a = vertex_color[3] if len(vertex_color) > 3 else 255
                
                # Create BGRA color structure
                bgra = SavedColorBGRA()
                bgra.b = max(0, min(255, int(b)))
                bgra.g = max(0, min(255, int(g)))
                bgra.r = max(0, min(255, int(r)))
                bgra.a = max(0, min(255, int(a)))
                uncompressed_data.extend(bytes(bgra))
            else:
                # Fallback: use existing vertex_color as-is
                uncompressed_data.extend(bytes(vertex_color))
        
        if compress:
            # Compress using PKWare format (same as FTS compression)
            output = self._encode_pkware_llf(uncompressed_data)
            print(f"DEBUG: Compression: {len(uncompressed_data)} -> {len(output)} bytes")
        else:
            output = bytes(uncompressed_data)

        with open(llf_path, 'wb') as f:
            f.write(output)

        print(f"DEBUG: Wrote LLF file (v{dlf_version:.2f}, "
              f"{'compressed' if compress else 'uncompressed'}) with "
              f"{len(vertex_lighting)} vertex colors to {llf_path}")
    
    def _encode_pkware_llf(self, data):
        """PKWare encoding for LLF files using proper header format"""
        # Based on Implode.cpp lines 194-195: header is literal bytes, not bitstream
        result = bytearray()
        
        # Write PKWare header as literal bytes (NOT bitstream)
        result.append(0)  # nLitSize: 0 = IMPLODE_LITERAL_FIXED (for binary data)
        result.append(6)  # nDictSizeByte: 6 (matches original LLF file)
        
        # Create bitstream encoder for the actual data
        encoder = self._PKWareEncoder()
        
        # Encode all input bytes as uncoded literals (no separate header bits)
        for byte_val in data:
            encoder.write_literal(byte_val)
        
        # Write end-of-stream marker (length 519)
        encoder.write_end_of_stream()
        
        # Combine literal header bytes + compressed bitstream
        result.extend(encoder.get_bytes())
        return bytes(result)
    
    def _encode_pkware_dlf(self, data):
        """PKWare encoding for DLF files using proper header format"""
        # Based on Implode.cpp lines 194-195: header is literal bytes, not bitstream
        result = bytearray()
        
        # Write PKWare header as literal bytes (NOT bitstream)
        result.append(0)  # nLitSize: 0 = IMPLODE_LITERAL_FIXED (for binary data)
        result.append(6)  # nDictSizeByte: 6 (matches original DLF file)
        
        # Create bitstream encoder for the actual data
        encoder = self._PKWareEncoder()
        
        # Encode all input bytes as uncoded literals (no separate header bits)
        for byte_val in data:
            encoder.write_literal(byte_val)
        
        # Write end-of-stream marker (length 519)
        encoder.write_end_of_stream()
        
        # Combine literal header bytes + compressed bitstream
        result.extend(encoder.get_bytes())
        return bytes(result)
    
    class _PKWareEncoder:
        """Clean PKWare encoder implementation based on ArxLibertatis blast.cpp"""
        
        def __init__(self):
            self.bits = []
            
            # Constants from ArxLibertatis/src/io/Blast.cpp lines 343-347
            self.BASE = [3, 2, 4, 5, 6, 7, 8, 9, 10, 12, 16, 24, 40, 72, 136, 264]
            self.EXTRA = [0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8]
            self.LENLEN = [2, 35, 36, 53, 38, 23]  # Line 340
            
            # Derived constants from C++ arrays
            self.MAX_LENGTH_SYMBOLS = len(self.BASE)  # 16 symbols (0-15)
            self.END_SYMBOL = self.MAX_LENGTH_SYMBOLS - 1  # Symbol 15 for end-of-stream
            self.END_LENGTH = self.BASE[self.END_SYMBOL] + ((1 << self.EXTRA[self.END_SYMBOL]) - 1)  # 519
            self.BYTE_BITS = 8
            
            # Build length code Huffman table
            self.length_codes = self._build_length_table()
        
        def _build_length_table(self):
            """Build Huffman table for length codes using C++ construct() algorithm"""
            # Decode compact lenlen format: each byte = (length & 15) | (count-1)<<4
            # From C++ construct() lines 224-233
            code_lengths = []
            for packed_val in self.LENLEN:
                length = (packed_val & 15) + 2  # Bottom 4 bits + 2
                count = (packed_val >> 4) + 1   # Top 4 bits + 1
                code_lengths.extend([length] * count)
            
            # Pad to MAX_LENGTH_SYMBOLS
            while len(code_lengths) < self.MAX_LENGTH_SYMBOLS:
                code_lengths.append(0)
            
            # Generate canonical Huffman codes with bit reversal
            # From C++ decode() lines 134-145: bits are inverted
            codes = [0] * self.MAX_LENGTH_SYMBOLS
            code = 0
            max_code_length = max(code_lengths) if code_lengths else 0
            
            for bit_length in range(1, max_code_length + 1):
                for symbol in range(self.MAX_LENGTH_SYMBOLS):
                    if code_lengths[symbol] == bit_length:
                        # Store bit-reversed code for PKWare format (blast.cpp line 165)
                        codes[symbol] = self._reverse_bits(code, bit_length)
                        code += 1
                code <<= 1
            
            return [(codes[i], code_lengths[i]) for i in range(self.MAX_LENGTH_SYMBOLS)]
        
        def _reverse_bits(self, value, num_bits):
            """Reverse bit order for PKWare compatibility (blast.cpp line 165)"""
            result = 0
            for i in range(num_bits):
                if value & (1 << i):
                    result |= (1 << (num_bits - 1 - i))
            return result
        
        def write_header(self, lit_flag, dict_size):
            """Write PKWare header as part of bitstream (blast.cpp lines 359-366)"""
            # NOTE: For LLF files, header is written as literal bytes, not bitstream
            # This method is kept for FTS compatibility but not used for LLF
            # Write lit flag (8 bits) - blast.cpp line 359: lit = bits(s, 8)
            for i in range(self.BYTE_BITS):
                self.bits.append((lit_flag >> i) & 1)
            
            # Write dict size (8 bits) - blast.cpp line 363: dict = bits(s, 8)  
            for i in range(self.BYTE_BITS):
                self.bits.append((dict_size >> i) & 1)
        
        def write_literal(self, byte_val):
            """Write uncoded literal: 0 prefix + 8 bits (blast.cpp line 292)"""
            # From blast.cpp line 292: "0 for literals"
            self.bits.append(0)
            
            # From blast.cpp line 294-297: "no bit-reversal is needed" for uncoded literals
            # Write in LSB-first order within byte
            for i in range(self.BYTE_BITS):
                self.bits.append((byte_val >> i) & 1)
        
        def write_end_of_stream(self):
            """Write simple end-of-stream marker like working FTS implementation"""
            # From working FTS code: much simpler EOS than complex Huffman
            # Use the pattern that works in the FTS encoder
            self.bits.append(1)    # EOS marker bit
            # Simple EOS pattern: 7 zeros + 8 ones (from working FTS implementation)
            for i in range(7):
                self.bits.append(0)
            for i in range(8):
                self.bits.append(1)
        
        def get_bytes(self):
            """Convert bit array to bytes like working FTS implementation"""            
            result = bytearray()
            
            # Process complete bytes first
            complete_bytes = len(self.bits) // self.BYTE_BITS
            for i in range(complete_bytes):
                byte_val = 0
                for j in range(self.BYTE_BITS):
                    if self.bits[i * self.BYTE_BITS + j]:
                        byte_val |= (1 << j)
                result.append(byte_val)
            
            # Handle final partial byte (like working FTS GetBytePadded)
            remaining_bits = len(self.bits) % self.BYTE_BITS
            if remaining_bits > 0:
                byte_val = 0
                for j in range(remaining_bits):
                    bit_index = complete_bytes * self.BYTE_BITS + j
                    if self.bits[bit_index]:
                        byte_val |= (1 << j)
                result.append(byte_val)
            
            return bytes(result)
    
    def writeFtsFile(self, fts_path, fts_data, converted_faces):
        """Write FTS file with updated background geometry"""
        if len(converted_faces) == 0:
            raise ArxException("No faces to export")
        
        # Validate FTS properties
        self._validateFtsProperties(converted_faces)
        
        # Convert faces back to FTS polygon structures. Go through _orderedPolygons
        # so the DANAE normal pass has run before the polygons are serialised.
        self._orderedPolygons(fts_data)
        updated_cells = self._cell_grid
        
        # Write FTS file using the serializer  
        import bpy
        addon = getAddon(bpy.context)
        fts_serializer = addon.sceneManager.ftsSerializer
        
        try:
            fts_serializer.write_fts_container(fts_path, fts_data, updated_cells)
            self.report({'INFO'}, f"Successfully wrote FTS file with {len(converted_faces)} faces")
        except Exception as e:
            raise ArxException(f"FTS write failed: {str(e)}")
    
    def _validateFtsProperties(self, converted_faces):
        """Validate that converted faces have required FTS properties"""
        required_props = ['transval', 'area', 'room', 'poly_type', 'vertices']
        missing_props = []
        
        for i, face in enumerate(converted_faces[:5]):  # Check first 5 faces
            for prop in required_props:
                if prop not in face:
                    missing_props.append(prop)
        
        if missing_props:
            raise ArxException(f"Missing required FTS properties: {set(missing_props)}")
    
    def _reconstructCellGrid(self, converted_faces, fts_data):
        """Reconstruct FTS cell grid from converted face data with spatial partitioning.

        The result is cached for the duration of one export. Everything that has to
        agree on polygon order - the FTS writer, the room EP_DATA references and the
        LLF vertex colours - reads this one grid, because the engine addresses
        polygons by (cell, index within cell) and reads lighting in cell order. Two
        independently built grids drift apart as soon as a single face is skipped.
        """
        if getattr(self, '_cell_grid', None) is not None:
            return self._cell_grid

        import math
        
        # Get scene offset for proper cell grid alignment
        scene_offset = fts_data.sceneOffset
        print(f"DEBUG: Using scene offset: {scene_offset}")
        
        # Debug: Check if scene offset seems reasonable
        if abs(scene_offset[0]) > 50000 or abs(scene_offset[2]) > 50000:
            print(f"WARNING: Scene offset seems very large, this may cause grid issues")
            print(f"WARNING: First few face centers will be around ({converted_faces[0]['vertices'][0]['pos'][0]:.1f}, {converted_faces[0]['vertices'][0]['pos'][2]:.1f}) if available")
        
        # Create Python dict structures instead of ctypes to avoid read-only issues
        fts_polygons = []
        degenerate_faces = 0
        
        for face_data in converted_faces:
            # Create polygon as Python dict instead of ctypes structure
            vertices = face_data['vertices']
            num_verts = len(vertices)
            
            # Check for degenerate geometry
            is_degenerate = False
            if num_verts >= 3:
                # Check if any vertices are identical (would create degenerate face)
                for i in range(num_verts):
                    for j in range(i + 1, num_verts):
                        v1 = vertices[i]['pos']
                        v2 = vertices[j]['pos']
                        # Check if positions are nearly identical
                        if abs(v1[0] - v2[0]) < 0.001 and abs(v1[1] - v2[1]) < 0.001 and abs(v1[2] - v2[2]) < 0.001:
                            is_degenerate = True
                            break
                    if is_degenerate:
                        break
            
            if is_degenerate:
                degenerate_faces += 1
                if degenerate_faces <= 5:
                    print(f"DEBUG: Degenerate face {len(fts_polygons)}: identical vertices detected")
            
            # Build vertices array as Python dicts
            poly_vertices = []
            # Only store the actual vertices - don't create degenerate quads
            for i in range(num_verts):
                vert = vertices[i]
                poly_vertices.append({
                    'ssx': vert['pos'][0],
                    'sy': vert['pos'][1],
                    'ssz': vert['pos'][2],
                    'stu': vert['uv'][0],
                    'stv': vert['uv'][1]
                })

            # Pad with zero vertices if needed for the FTS format (which expects 4 vertex slots)
            # But mark it properly as a triangle via poly_type flag
            while len(poly_vertices) < 4:
                # Add padding vertices for FTS format compatibility
                poly_vertices.append({
                    'ssx': 0.0, 'sy': 0.0, 'ssz': 0.0,
                    'stu': 0.0, 'stv': 0.0
                })

            # Create polygon as Python dict
            room_id = face_data.get('room', 0)
            mapped_room = self._map_room_id_to_index(room_id)

            poly_dict = {
                'vertices': poly_vertices,
                'bmesh_index': face_data.get('bmesh_index', -1),
                'tex': face_data.get('tex', 0),
                'transval': face_data.get('transval', 0.0),
                'area': face_data.get('area', 1.0),
                'room': mapped_room
            }
            
            # Add normals to polygon dict
            norm = face_data.get('norm', [0, 1, 0])
            poly_dict['norm'] = {'x': norm[0], 'y': norm[1], 'z': norm[2]}
            poly_dict['norm2'] = {'x': norm[0], 'y': norm[1], 'z': norm[2]}
            
            # Add vertex normals 
            vertex_norms = face_data.get('vertex_normals', [norm] * 4)
            poly_dict['vertex_normals'] = []
            for i in range(4):
                if i < len(vertex_norms):
                    vnorm = vertex_norms[i]
                    poly_dict['vertex_normals'].append({'x': vnorm[0], 'y': vnorm[1], 'z': vnorm[2]})
                else:
                    poly_dict['vertex_normals'].append({'x': norm[0], 'y': norm[1], 'z': norm[2]})
            
            # Set poly type flags - ensure POLY_QUAD flag matches actual vertex count
            poly_type = face_data.get('poly_type', 0)
            # Clear POLY_QUAD flag if this is a triangle
            if num_verts == 3:
                poly_type = poly_type & ~64  # Remove POLY_QUAD flag (bit 6)
            elif num_verts == 4:
                poly_type = poly_type | 64   # Ensure POLY_QUAD flag is set
            poly_dict['poly_type'] = poly_type
            poly_dict['is_quad'] = (num_verts == 4)
            
            # Debug: log room mapping for first few faces
            if len(fts_polygons) < 5:
                print(f"DEBUG: Face {len(fts_polygons)}: room_id={room_id} → mapped_room={mapped_room}")
                print(f"DEBUG: FTS polygon {len(fts_polygons)}: {num_verts} vertices, is_quad={poly_dict['is_quad']}")
            
            # Store the original face index in poly_dict for later matching
            poly_dict['original_face_index'] = len(fts_polygons)
            fts_polygons.append((poly_dict, face_data))
        
        if degenerate_faces > 0:
            print(f"DEBUG: Found {degenerate_faces} degenerate faces out of {len(fts_polygons)} total")
        
        # Initialize 160x160 cell grid
        updated_cells = [[None for _ in range(160)] for _ in range(160)]
        
        # Place polygons in cell grid using preserved coordinates or spatial calculation
        faces_processed = 0
        faces_placed = 0
        faces_calculated = 0
        
        for poly, face_data in fts_polygons:
            faces_processed += 1
            
            # Try to use preserved cell coordinates first
            cell_x = face_data.get('cell_x', None)
            cell_z = face_data.get('cell_z', None)
            
            # Only calculate if we don't have preserved coordinates
            if cell_x is None or cell_z is None:
                vertices = face_data.get('vertices', [])
                if vertices:
                    center_x = sum(v['pos'][0] for v in vertices) / len(vertices)
                    center_z = sum(v['pos'][2] for v in vertices) / len(vertices)

                    # Convert to cell coordinates (160x160 grid, each cell is 100 units)
                    # Engine formula: cell = int(pos / 100)
                    # No offset applied - grid is in world coordinates

                    cell_x = int(center_x / 100)
                    cell_z = int(center_z / 100)
                    faces_calculated += 1
                else:
                    # No vertices to calculate from
                    continue

            # Validate bounds - if out of bounds, skip the face
            if cell_x is not None and cell_z is not None:
                if cell_x < 0 or cell_x >= 160 or cell_z < 0 or cell_z >= 160:
                    if faces_processed < 10:  # Log first few out-of-bounds faces
                        # Calculate center for debug message if needed
                        vertices = face_data.get('vertices', [])
                        if vertices:
                            debug_center_x = sum(v['pos'][0] for v in vertices) / len(vertices)
                            debug_center_z = sum(v['pos'][2] for v in vertices) / len(vertices)
                        else:
                            debug_center_x = 0
                            debug_center_z = 0
                        print(f"DEBUG: Skipping out-of-bounds face at cell ({cell_x}, {cell_z}) - center: ({debug_center_x:.1f}, {debug_center_z:.1f})")
                    continue

                if faces_processed <= 5:
                    # Calculate center for debug message if needed
                    vertices = face_data.get('vertices', [])
                    if vertices:
                        debug_center_x = sum(v['pos'][0] for v in vertices) / len(vertices)
                        debug_center_z = sum(v['pos'][2] for v in vertices) / len(vertices)
                        print(f"DEBUG: Face {faces_processed}: cell=({cell_x}, {cell_z}) from center=({debug_center_x:.1f}, {debug_center_z:.1f})")
            else:
                # Fallback to center cell if no vertices
                cell_x, cell_z = 80, 80
                print(f"WARNING: Face {faces_processed} has no vertices, placing in center cell (80, 80)")
            
            # Validate cell coordinates
            if 0 <= cell_x < 160 and 0 <= cell_z < 160:
                # Add polygon to its cell
                if updated_cells[cell_z][cell_x] is None:
                    updated_cells[cell_z][cell_x] = []
                updated_cells[cell_z][cell_x].append(poly)
                faces_placed += 1
            else:
                print(f"ERROR: Invalid cell coordinates ({cell_x}, {cell_z}) for face {faces_processed}, placing in center")
                # Fallback to center cell
                if updated_cells[80][80] is None:
                    updated_cells[80][80] = []
                updated_cells[80][80].append(poly)
                faces_placed += 1
        
        # Count populated cells
        populated_cells = sum(1 for z in range(160) for x in range(160) if updated_cells[z][x] is not None)
        total_polys = sum(len(updated_cells[z][x]) for z in range(160) for x in range(160) if updated_cells[z][x] is not None)
        
        print(f"DEBUG: Processed {faces_processed} faces, {faces_placed} placed in cells ({faces_calculated} calculated, {faces_processed - faces_calculated} preserved), {total_polys} total in grid")
        self.report({'INFO'}, f"Reconstructed cell grid: {total_polys} polygons in {populated_cells} cells ({faces_calculated} new coordinates calculated)")

        # Flat list in exactly the order write_fts emits polygons and the engine
        # reads them back: cell rows along Z, cells along X, polygons within a cell.
        self._cell_grid = updated_cells
        self._ordered_polys = []
        for z in range(160):
            for x in range(160):
                if updated_cells[z][x] is not None:
                    self._ordered_polys.extend(updated_cells[z][x])

        return updated_cells

    def _orderedPolygons(self, fts_data):
        """Polygons in FTS write order, building the shared cell grid if needed."""
        if self._reconstructCellGrid(self.converted_faces, fts_data) is not None \
                and not getattr(self, '_normals_prepared', False):
            self._prepareVertexNormals()
            self._normals_prepared = True
        return self._ordered_polys

    def _prepareVertexNormals(self):
        """Rebuild every per vertex normal the way DANAE did before baking.

        The editor never trusted the normals it had loaded: ARX_PrepareBackgroundNRMLs
        recomputed the lot from the face normals, welding across polygons that share
        a position. Doing the same here means geometry modelled in Blender gets the
        normals the engine expects instead of whatever the mesh attributes happened
        to hold, and it fixes runtime lighting too, since ApplyTileLights dots
        against these same values every frame.
        """
        from .danae_lighting import prepare_vertex_normals

        faces = []
        for poly in self._ordered_polys:
            count = 4 if poly.get('is_quad', False) else 3
            vertices = [(v['ssx'], v['sy'], v['ssz']) for v in poly['vertices'][:count]]
            norm = poly['norm']
            norm2 = poly.get('norm2', norm)
            faces.append((vertices,
                          (norm['x'], norm['y'], norm['z']),
                          (norm2['x'], norm2['y'], norm2['z'])))

        computed = prepare_vertex_normals(faces)

        for poly, normals in zip(self._ordered_polys, computed):
            padded = list(normals)
            while len(padded) < 4:
                padded.append(normals[-1])
            poly['vertex_normals'] = [{'x': n[0], 'y': n[1], 'z': n[2]} for n in padded]

        print(f"DEBUG: Recomputed vertex normals for {len(computed)} polygons "
              f"(ARX_PrepareBackgroundNRMLs)")
    
    def _calculateVertexLighting(self, vertex_pos, vertex_normal):
        """Calculate vertex lighting from scene lights with tunable parameters"""
        # Lighting parameters - these could be made user-configurable
        ambient_color = (48, 48, 64)  # Slightly blue-tinted ambient
        ambient_intensity = 0.3
        light_falloff_power = 1.5     # Moderate falloff 
        max_light_contribution = 200.0  # Allow brighter lighting
        
        # Start with ambient lighting
        final_r = ambient_color[0] * ambient_intensity
        final_g = ambient_color[1] * ambient_intensity  
        final_b = ambient_color[2] * ambient_intensity
        
        # Get lights from scene data if available
        lights = getattr(self, '_scene_lights', [])
        scene_offset = getattr(self, '_scene_offset', Vector((0, 0, 0)))
        
        lights_affecting_vertex = 0
        for light in lights:
            # Apply scene offset to light position to align with geometry
            light_pos = Vector([light.pos.x, light.pos.y, light.pos.z]) + scene_offset
            distance = (vertex_pos - light_pos).length
            
            # Skip if beyond light radius
            if distance > light.fallend:
                continue
                
            lights_affecting_vertex += 1
                
            # Calculate falloff based on distance
            if distance < light.fallstart:
                # Full intensity within fallstart radius
                falloff = 1.0
            else:
                # Linear falloff from fallstart to fallend
                falloff_range = light.fallend - light.fallstart
                if falloff_range > 0:
                    distance_in_falloff = distance - light.fallstart
                    falloff = 1.0 - (distance_in_falloff / falloff_range)
                    falloff = max(0.0, falloff)
                    
                    # Apply power curve for more realistic falloff
                    falloff = pow(falloff, light_falloff_power)
                else:
                    falloff = 1.0 if distance <= light.fallend else 0.0
            
            # Calculate light direction and basic lambert lighting
            if distance > 0.01:  # Avoid division by zero
                light_dir = (light_pos - vertex_pos).normalized()
                lambert = max(0.0, vertex_normal.dot(light_dir))
            else:
                lambert = 1.0  # Point is at light source
            
            # Combine intensity, falloff, and lambert
            light_contribution = light.intensity * falloff * lambert * max_light_contribution
            
            # Add light color contribution
            final_r += light.rgb.r * light_contribution
            final_g += light.rgb.g * light_contribution  
            final_b += light.rgb.b * light_contribution
        
        # Clamp to valid color range
        final_r = max(0, min(255, int(final_r)))
        final_g = max(0, min(255, int(final_g)))
        final_b = max(0, min(255, int(final_b)))
        
        # Debug for first calculation
        if not hasattr(self, '_lighting_debug_done'):
            print(f"DEBUG: Lighting calc - {lights_affecting_vertex} lights affecting vertex, ambient={ambient_color}, final=({final_r},{final_g},{final_b})")
            self._lighting_debug_done = True
        
        return (final_r, final_g, final_b, 255)
    
    def _bakeLightingToCyclesVertexColors(self, mesh_obj, scene, lighting_props):
        """Core Cycles baking function used by both export and regenerate
        Returns True if successful, False otherwise
        This modifies the vertex colors in place on the mesh"""
        import bpy
        from mathutils import Vector

        mesh = mesh_obj.data

        # Ensure we have the light-color layer
        vcol_layer = None
        for vcol in mesh.vertex_colors:
            if vcol.name == "light-color":
                vcol_layer = vcol
                break

        if not vcol_layer:
            print("ERROR: No 'light-color' vertex color layer found!")
            return False

        mesh.vertex_colors.active = vcol_layer
        print("DEBUG: Using 'light-color' vertex color layer for baking")

        # Clear to black before baking
        print(f"DEBUG: Initializing {len(mesh.polygons)} polygons with black vertex colors...")
        for poly in mesh.polygons:
            for loop_idx in poly.loop_indices:
                vcol_layer.data[loop_idx].color = (0, 0, 0, 1)

        mesh.update()

        # Ensure UVs exist (required for baking)
        if len(mesh.uv_layers) == 0:
            print("WARNING: Mesh has no UV layers, creating one for baking")
            mesh.uv_layers.new(name="BakeUV")

        # CRITICAL: Disconnect vertex colors in materials to prevent circular dependency
        # The materials multiply texture * vertex_color, so during baking we need to disconnect
        # the vertex colors to bake pure lighting only
        disconnected_links = []
        for mat in mesh.materials:
            if mat and mat.use_nodes:
                # Find the Arx Material node group
                for node in mat.node_tree.nodes:
                    if node.type == 'GROUP' and node.node_tree:
                        # Check inside the node group for vertex color nodes
                        for group_node in node.node_tree.nodes:
                            if group_node.type == 'VERTEX_COLOR' and group_node.layer_name == 'light-color':
                                # Disconnect the Color output
                                for link in node.node_tree.links:
                                    if link.from_node == group_node and link.from_socket.name == 'Color':
                                        to_node = link.to_node
                                        to_socket = link.to_socket
                                        from_socket = link.from_socket

                                        # Set multiply node to white so we get texture only
                                        if to_node.type == 'MIX_RGB':
                                            to_socket.default_value = (1, 1, 1, 1)

                                        disconnected_links.append((node.node_tree, from_socket, to_socket))
                                        node.node_tree.links.remove(link)
                                        print(f"DEBUG: Disconnected vertex color in {mat.name}")
                                        break

        # Configure bake settings
        scene.render.bake.target = 'VERTEX_COLORS'
        scene.render.bake.use_clear = True
        scene.render.bake.margin = 0
        scene.render.bake.margin_type = 'EXTEND'
        scene.render.bake.use_cage = False
        scene.render.bake.cage_extrusion = 0.0
        scene.render.bake.max_ray_distance = 0.01

        # Use user-defined bake type and settings
        bake_type = lighting_props.cycles_bake_type

        # Configure passes based on user settings
        if bake_type == 'DIFFUSE' and hasattr(scene.render.bake, 'use_pass_direct'):
            scene.render.bake.use_pass_direct = lighting_props.cycles_use_direct_light
            scene.render.bake.use_pass_indirect = lighting_props.cycles_use_indirect_light
            scene.render.bake.use_pass_color = lighting_props.cycles_use_color
        elif bake_type == 'AO':
            if hasattr(scene.world.light_settings, 'distance'):
                scene.world.light_settings.distance = lighting_props.cycles_ao_distance

        print(f"DEBUG: Starting Cycles bake operation...")
        print(f"  - Bake type: {bake_type}")
        print(f"  - Mesh: {mesh_obj.name}")
        print(f"  - Materials: {len(mesh.materials)}")

        # Perform the actual bake
        try:
            override = bpy.context.copy()
            override['object'] = mesh_obj
            override['active_object'] = mesh_obj
            override['selected_objects'] = [mesh_obj]

            with bpy.context.temp_override(**override):
                result = bpy.ops.object.bake(type=bake_type)
        except Exception as e:
            print(f"ERROR: Bake with override failed: {e}")
            try:
                result = bpy.ops.object.bake(type=bake_type)
            except Exception as e2:
                print(f"ERROR: Regular bake also failed: {e2}")
                result = {'CANCELLED'}

        print(f"DEBUG: Cycles bake complete! Result: {result}")

        # Restore vertex color connections in materials
        for node_tree, from_socket, to_socket in disconnected_links:
            node_tree.links.new(from_socket, to_socket)
            print(f"DEBUG: Restored vertex color connection")

        # Check if baking actually worked
        has_color = False
        for poly in mesh.polygons[:10]:
            for loop_idx in poly.loop_indices:
                color = vcol_layer.data[loop_idx].color
                if color[0] > 0.01 or color[1] > 0.01 or color[2] > 0.01:
                    has_color = True
                    break
            if has_color:
                break

        if has_color:
            print("DEBUG: Cycles baking successful - vertex colors updated")
            return True
        else:
            print("WARNING: Cycles bake produced black colors - check lights and materials")
            return False

    def _calculateDanaeVertexLighting(self, ordered_polys, scene):
        """Bake vertex lighting the way the original DANAE editor did.

        Works straight off the FTS polygon data in Arx units, so no Blender mesh,
        material or renderer is involved and the result needs no colour space or
        brightness correction before it goes into the .llf file.
        """
        import time

        from .danae_lighting import (POLY_IGNORE, POLY_TRANS, POLY_WATER,
                                     DanaePolygon, compute_vertex_colors)

        props = scene.arx_lighting
        lights = gather_danae_lights(scene)

        polygons = []
        for poly in ordered_polys:
            count = 4 if poly.get('is_quad', False) else 3
            vertices = [(v['ssx'], v['sy'], v['ssz']) for v in poly['vertices'][:count]]
            normals = [(n['x'], n['y'], n['z']) for n in poly['vertex_normals'][:count]]
            while len(normals) < count:
                normals.append((0.0, 1.0, 0.0))

            center = (
                sum(v[0] for v in vertices) / count,
                sum(v[1] for v in vertices) / count,
                sum(v[2] for v in vertices) / count,
            )
            polygons.append(DanaePolygon(
                vertices=vertices,
                normals=normals,
                center=center,
                ignore=bool(int(poly.get('poly_type', 0)) & POLY_IGNORE),
            ))

        visible = None
        if props.danae_raylaunch:
            # Every polygon goes in, so that face i in the tree is polygon i and
            # the bake can recognise a hit on the polygon it is currently lighting.
            occluders = []
            for shaded in polygons:
                vertices = shaded.vertices
                if len(vertices) == 4:
                    # FTS stores quads in Z order, so the perimeter is 0,1,3,2.
                    # Handing the stored order to the BVH would build a bowtie.
                    vertices = [vertices[0], vertices[1], vertices[3], vertices[2]]
                occluders.append(vertices)
            visible = build_danae_occluder(occluders)
            if visible is None:
                print("WARNING: No occluding geometry found, baking without shadows")

        ambient = (props.danae_ambient,) * 3
        started = time.time()

        def progress(index, total):
            print(f"  DANAE lighting: polygon {index}/{total}")

        colors = compute_vertex_colors(
            polygons, lights,
            ambient=ambient,
            use_normals=props.danae_use_normals,
            visible=visible,
            progress=progress,
        )

        # A vertex sitting exactly on the ambient floor received nothing at all.
        # A high count means the lights never reached it - out of fallend range, a
        # normal facing away, or a shadow ray that hit something - rather than the
        # bake being merely dim.
        floor = tuple(int(c * 255.0 + 0.5) for c in ambient)
        unlit = sum(1 for c in colors if c[:3] == floor)
        print(f"DEBUG: DANAE bake of {len(polygons)} polygons took {time.time() - started:.1f}s")
        print(f"DEBUG: {unlit}/{len(colors)} vertices came out at the ambient floor {floor}")
        return colors

    def _calculateCyclesVertexLighting(self, converted_faces, mesh_obj, scene, fts_data):
        """Calculate vertex lighting using Blender Cycles renderer with actual baking"""
        import bmesh
        import bpy
        from mathutils import Vector

        print(f"DEBUG: Starting Cycles vertex lighting baking for {len(converted_faces)} faces")

        # Debug: Check what types of faces we have
        regular_faces = 0
        portal_faces = 0
        quad_count = 0
        tri_count = 0
        expected_vertices = 0
        for face_data in converted_faces:
            if face_data.get('tex', -1) == -1 and face_data.get('room', None) is not None:
                # Portal-like face (no texture but has room assignment)
                portal_faces += 1
            else:
                regular_faces += 1

            # Count face types and expected vertex count
            is_quad = face_data.get('is_quad', False)
            if is_quad:
                quad_count += 1
                expected_vertices += 4
            else:
                tri_count += 1
                expected_vertices += 3  # Triangles need 3 colors

        print(f"DEBUG: Face breakdown - regular: {regular_faces}, portal-like: {portal_faces}")
        print(f"DEBUG: {quad_count} quads (4 verts each) + {tri_count} triangles (3 verts each) = {expected_vertices} vertices expected for LLF")

        # Store original settings
        original_engine = scene.render.engine
        original_samples = None
        original_active = scene.view_layers[0].objects.active
        original_selection = [obj for obj in scene.objects if obj.select_get()]

        try:
            # Set up Cycles for baking
            scene.render.engine = 'CYCLES'
            bpy.context.view_layer.update()

            # Configure Cycles settings from user preferences
            lighting_props = scene.arx_lighting
            cycles = scene.cycles if hasattr(scene, 'cycles') else None
            if cycles:
                original_samples = cycles.samples
                cycles.samples = lighting_props.cycles_samples
                cycles.use_denoising = lighting_props.cycles_use_denoising

            # Ensure mesh object is selected and active
            for obj in scene.objects:
                obj.select_set(False)
            mesh_obj.select_set(True)
            scene.view_layers[0].objects.active = mesh_obj

            # Use the core baking function
            bake_success = self._bakeLightingToCyclesVertexColors(mesh_obj, scene, lighting_props)

            if not bake_success:
                print("ERROR: Cycles baking failed - using fallback manual lighting")
                # Fallback: Simple manual lighting calculation
                mesh = mesh_obj.data
                vcol_layer = None
                for vcol in mesh.vertex_colors:
                    if vcol.name == "light-color":
                        vcol_layer = vcol
                        break

                if vcol_layer:
                    for poly_idx, poly in enumerate(mesh.polygons):
                        if poly_idx % 1000 == 0:
                            print(f"  Processing face {poly_idx}/{len(mesh.polygons)}")

                        # Get face center
                        face_center = mesh_obj.matrix_world @ poly.center
                        face_normal = mesh_obj.matrix_world.to_3x3() @ poly.normal

                        # Simple lighting: accumulate from nearby lights
                        total_light = Vector((0.1, 0.1, 0.1))  # Ambient

                        for light_obj in [obj for obj in scene.objects if obj.type == 'LIGHT'][:50]:  # Limit to 50 lights
                            light_pos = light_obj.location
                            light_dir = (light_pos - face_center).normalized()
                            distance = (light_pos - face_center).length

                            # Simple distance falloff
                            if distance < 20.0:  # Arbitrary cutoff
                                dot = max(0, face_normal.dot(light_dir))
                                intensity = dot * (1.0 - distance / 20.0) * light_obj.data.energy / 1000.0
                                total_light += Vector(light_obj.data.color) * intensity

                        # Clamp and apply to all loops of this face
                        for loop_idx in poly.loop_indices:
                            vcol_layer.data[loop_idx].color = (
                                min(1.0, total_light.x),
                                min(1.0, total_light.y),
                                min(1.0, total_light.z),
                                1.0
                            )

                    mesh.update()
                    print("Manual lighting calculation complete")

            # Now read the baked vertex colors in the exact order needed for LLF
            # CRITICAL: We need to read colors in the same cell-by-cell order as FTS!
            # The engine reads polygons from cells in Y-then-X order (GridYXIterator)
            vertex_colors = []

            # First, rebuild the cell grid to know the polygon order
            cell_grid = self._reconstructCellGrid(converted_faces, fts_data)

            # Create a bmesh to access the vertex colors
            bm = bmesh.new()
            bm.from_mesh(mesh)
            bm.faces.ensure_lookup_table()

            # Build a mapping from face data to bmesh face index
            face_to_bmesh = {}
            for i, face_data in enumerate(converted_faces):
                face_to_bmesh[id(face_data)] = i

            # Get the "light-color" layer that we baked to
            color_layer = None
            for layer in bm.loops.layers.color:
                if layer.name == "light-color":
                    color_layer = layer
                    break

            # Fallback to active layer if specific layer not found
            if not color_layer:
                color_layer = bm.loops.layers.color.active

            if not color_layer:
                raise Exception("Failed to get vertex color layer after baking")

            print(f"DEBUG: Using color layer: {color_layer.name if hasattr(color_layer, 'name') else 'unknown'}")

            # Map Blender faces to converted_faces
            # CRITICAL: We need to ensure faces are matched correctly
            if len(bm.faces) != len(converted_faces):
                print(f"WARNING: Face count mismatch: Blender has {len(bm.faces)}, converted has {len(converted_faces)}")

            # No need for complex mapping - faces should be in the same order
            # The import process creates faces sequentially, so they should match

            # Build face index mapping - converted_faces index to bmesh face
            # This assumes converted_faces and bm.faces have the same faces in the same order
            # (they should, since converted_faces was built from iterating bm.faces)

            # CRITICAL: We must iterate in EXACTLY the same order as FTS!
            # The FTS file writes polygons cell by cell (Y first, then X)
            # We need to read vertex colors in the same order

            debug_samples = 0
            triangle_count = 0
            faces_processed = 0

            # Iterate through cells in Y-then-X order (matching FTS write order)
            for z in range(160):
                for x in range(160):
                    if cell_grid[z][x] is not None:
                        # Process each polygon in this cell
                        for poly_dict in cell_grid[z][x]:
                            # poly_dict is the converted face data
                            face_data = poly_dict

                            # Index of the bmesh face this polygon came from. Not the
                            # position in converted_faces: faces with an unsupported
                            # vertex count are skipped during conversion, so the two
                            # drift apart after the first one.
                            face_index = poly_dict.get('bmesh_index', -1)

                            # Check if we found the face index
                            if face_index < 0:
                                print(f"WARNING: Could not find bmesh face for cell ({x},{z}) polygon")
                                is_quad = face_data.get('is_quad', False)
                                vertex_count = 4 if is_quad else 3
                                for i in range(vertex_count):
                                    vertex_colors.append((128, 128, 128, 255))
                                continue

                            # Now process this face's vertex colors
                            is_quad = face_data.get('is_quad', False)
                            vertex_count = 4 if is_quad else 3
                            faces_processed += 1

                            if face_index < len(bm.faces):
                                blender_face = bm.faces[face_index]

                                # Check for vertex count mismatch
                                actual_verts = len(blender_face.loops)
                                if actual_verts != vertex_count:
                                    print(f"MISMATCH at face {face_index}: converted says {vertex_count} verts, Blender has {actual_verts}")

                                # Get colors from the baked vertex color layer
                                # IMPORTANT: For quads, vertices were swapped in convertMeshToFtsCells

                                if is_quad and len(blender_face.loops) == 4:
                                    # For quads: vertices are swapped during export
                                    loop_indices = [0, 1, 3, 2]  # Read in this order to match the swap
                                else:
                                    # For triangles or degenerate quads: Read available loops
                                    if not is_quad:
                                        triangle_count += 1
                                    loop_indices = list(range(len(blender_face.loops)))

                                colors_for_face = []

                                # Process the colors we can get from the actual loops
                                colors_before = len(vertex_colors)
                                for i in range(vertex_count):
                                    if i < len(loop_indices) and loop_indices[i] < len(blender_face.loops):
                                        idx = loop_indices[i]
                                        loop = blender_face.loops[idx]
                                        color = loop[color_layer]

                                        # Apply gamma correction (linear to sRGB)
                                        def linear_to_srgb(c):
                                            if c <= 0.0031308:
                                                return 12.92 * c
                                            else:
                                                return 1.055 * (c ** (1.0/2.4)) - 0.055

                                        # Convert to sRGB and then to 0-255 range
                                        r = min(255, max(0, int(linear_to_srgb(color[0]) * 255)))
                                        g = min(255, max(0, int(linear_to_srgb(color[1]) * 255)))
                                        b = min(255, max(0, int(linear_to_srgb(color[2]) * 255)))

                                        colors_for_face.append((r, g, b, 255))
                                    else:
                                        # For degenerate quads, duplicate last vertex color
                                        if faces_processed < 10:
                                            print(f"DEBUG: Face {face_index} needs padding - is_quad={is_quad}, blender_loops={len(blender_face.loops)}, i={i}")
                                        if len(colors_for_face) > 0:
                                            colors_for_face.append(colors_for_face[-1])
                                        else:
                                            colors_for_face.append((128, 128, 128, 255))

                                # Add colors for this face
                                if len(colors_for_face) != vertex_count and faces_processed < 10:
                                    print(f"WARNING: Face {face_index} expected {vertex_count} colors but got {len(colors_for_face)}")
                                vertex_colors.extend(colors_for_face)
                            else:
                                # No bmesh face found, use default colors
                                for i in range(vertex_count):
                                    vertex_colors.append((128, 128, 128, 255))

            bm.free()
            print(f"DEBUG: Read {len(vertex_colors)} baked vertex colors from Cycles")
            return vertex_colors

        except Exception as e:
            print(f"ERROR: Cycles baking failed: {e}")
            import traceback
            traceback.print_exc()
            # No fallback - Cycles baking is required
            raise Exception(f"Cycles vertex color baking failed: {e}")

        finally:
            # Restore original settings
            scene.render.engine = original_engine
            if cycles and original_samples is not None:
                cycles.samples = original_samples

            # Restore selection
            for obj in scene.objects:
                obj.select_set(False)
            for obj in original_selection:
                obj.select_set(True)
            if original_active:
                scene.view_layers[0].objects.active = original_active
    
    def _evaluateVertexLighting(self, world_pos, world_normal, lights, scene):
        """Evaluate lighting at a specific vertex position using scene lights"""
        from mathutils import Vector
        
        # Start with ambient lighting
        ambient_strength = 0.2
        ambient_color = Vector((0.3, 0.3, 0.4))  # Cool ambient
        final_color = ambient_color * ambient_strength
        
        # Add contribution from each light
        for light_obj in lights:
            if not light_obj.data:
                continue
                
            light_data = light_obj.data
            light_pos = light_obj.location
            
            # Calculate light direction and distance
            light_dir = (light_pos - world_pos).normalized()
            light_distance = (light_pos - world_pos).length
            
            # Calculate attenuation based on light type and distance
            if light_data.type == 'POINT':
                # Point light attenuation
                if hasattr(light_data, 'cutoff_distance') and light_data.cutoff_distance > 0:
                    max_distance = light_data.cutoff_distance
                    attenuation = max(0.0, 1.0 - (light_distance / max_distance))
                else:
                    # Inverse square falloff
                    attenuation = 1.0 / (1.0 + light_distance * light_distance * 0.001)
                    
            elif light_data.type == 'SUN':
                # Directional light - no distance attenuation
                attenuation = 1.0
                light_dir = -Vector(light_obj.matrix_world.to_3x3() @ Vector((0, 0, -1)))
                
            else:
                # Default attenuation for other light types
                attenuation = 1.0 / (1.0 + light_distance * 0.1)
            
            # Calculate diffuse lighting (Lambertian)
            dot_product = max(0.0, world_normal.dot(light_dir))
            
            # Get light color and energy
            light_color = Vector(light_data.color[:3])
            light_energy = light_data.energy * 0.001  # Scale down for vertex lighting
            
            # Add light contribution
            light_contribution = light_color * light_energy * attenuation * dot_product
            final_color += light_contribution
        
        # Clamp and convert to 0-255 range
        final_color = Vector((
            max(0.0, min(1.0, final_color.x)),
            max(0.0, min(1.0, final_color.y)),
            max(0.0, min(1.0, final_color.z))
        ))
        
        return (
            int(final_color.x * 255),
            int(final_color.y * 255), 
            int(final_color.z * 255),
            255
        )
    
    def _storeLightsForLighting(self, llfData):
        """Store lights from LLF data for lighting calculations"""
        self._scene_lights = llfData.lights if llfData and hasattr(llfData, 'lights') else []
        print(f"DEBUG: Stored {len(self._scene_lights)} lights for vertex lighting calculation")

class CUSTOM_OT_arx_view_face_attributes(Operator):
    bl_idname = "arx.view_face_attributes"
    bl_label = "View Face Attributes"
    bl_description = "Show FTS polygon properties for selected faces"
    
    def execute(self, context):
        obj = context.active_object
        if not obj or obj.type != 'MESH':
            self.report({'ERROR'}, "Select a mesh object")
            return {'CANCELLED'}
        
        if not obj.data.polygons:
            self.report({'ERROR'}, "Mesh has no faces")
            return {'CANCELLED'}
        
        # Check for FTS attribute layers
        mesh = obj.data
        has_transval = 'arx_transval' in mesh.attributes
        has_area = 'arx_area' in mesh.attributes  
        has_room = 'arx_room' in mesh.attributes
        has_polytype = 'arx_polytype' in mesh.attributes
        
        if not (has_transval or has_area or has_room or has_polytype):
            self.report({'ERROR'}, "No FTS face attributes found. Reimport the level to get polygon properties.")
            return {'CANCELLED'}
        
        import bmesh
        bm = bmesh.new()
        bm.from_mesh(mesh)
        bm.faces.ensure_lookup_table()
        
        # Get attribute layers
        transval_layer = bm.faces.layers.float.get('arx_transval') if has_transval else None
        area_layer = bm.faces.layers.float.get('arx_area') if has_area else None
        room_layer = bm.faces.layers.int.get('arx_room') if has_room else None
        polytype_layer = bm.faces.layers.int.get('arx_polytype') if has_polytype else None
        
        # Collect statistics
        stats = {
            'total_faces': len(bm.faces),
            'transval_values': [],
            'area_values': [],
            'room_values': [],
            'polytype_values': []
        }
        
        for face in bm.faces:
            if transval_layer: stats['transval_values'].append(face[transval_layer])
            if area_layer: stats['area_values'].append(face[area_layer])
            if room_layer: stats['room_values'].append(face[room_layer])
            if polytype_layer: stats['polytype_values'].append(face[polytype_layer])
        
        # Show selected faces in detail (up to 10)
        selected_faces = [f for f in bm.faces if f.select]
        if not selected_faces:
            # If no faces selected, show first 5
            selected_faces = bm.faces[:5]
            self.report({'INFO'}, "No faces selected, showing first 5 faces")
        
        # Print detailed face info to console
        print("\n" + "="*60)
        print(f"FTS FACE ATTRIBUTES for {obj.name}")
        print("="*60)
        print(f"Total faces: {stats['total_faces']}")
        
        if has_transval:
            vals = stats['transval_values']
            print(f"TransVal: min={min(vals):.3f}, max={max(vals):.3f}, unique={len(set(vals))}")
        if has_area:
            vals = stats['area_values'] 
            print(f"Area: min={min(vals):.3f}, max={max(vals):.3f}, unique={len(set(vals))}")
        if has_room:
            vals = stats['room_values']
            print(f"Room: min={min(vals)}, max={max(vals)}, unique={len(set(vals))}")
        if has_polytype:
            vals = stats['polytype_values']
            print(f"PolyType: min={min(vals)}, max={max(vals)}, unique={len(set(vals))}")
        
        print(f"\nDetailed view of {len(selected_faces)} faces:")
        print("-" * 60)
        
        for i, face in enumerate(selected_faces[:10]):  # Limit to 10 faces
            print(f"Face {face.index}:")
            if transval_layer: print(f"  TransVal: {face[transval_layer]:.6f}")
            if area_layer: print(f"  Area: {face[area_layer]:.6f}")  
            if room_layer: print(f"  Room: {face[room_layer]}")
            if polytype_layer: 
                ptype = face[polytype_layer]
                print(f"  PolyType: {ptype} (0x{ptype:08x})")
        
        print("="*60 + "\n")
        
        bm.free()
        
        self.report({'INFO'}, f"Face attributes shown in console. Found {stats['total_faces']} faces.")
        return {'FINISHED'}

def animation_slots_for_model(context, model_name):
    """The animations a model's own entity script binds, as {slot: path}.

    Keyed off LOADANIM rather than off the file name. Matching animation files by
    substring puts every human animation under any model whose name happens to be
    a substring, and finds nothing at all for a model whose animations are named
    after something else, which is most of them.
    """
    addon = getAddon(context)
    reader = ASLReader(addon.sceneManager.dataPath)

    # NPC models live at graph/obj3d/interactive/npc/<name>/<name>.
    class_path = f"graph/obj3d/interactive/npc/{model_name}/{model_name}"
    return reader.animation_set(class_path, npc=True)


class ARX_OT_list_animation_sets(Operator):
    bl_idname = "arx.list_animation_sets"
    bl_label = "List Animation Sets"
    bl_description = ("Print every script in the data that binds animations, so a new "
                      "model can borrow an existing set")

    def execute(self, context):
        addon = getAddon(context)
        reader = ASLReader(addon.sceneManager.dataPath)
        sets = reader.find_animation_sets()

        if not sets:
            self.report({'WARNING'}, "No scripts bind animations under this data path")
            return {'CANCELLED'}

        print("\nScripts binding animations, largest set first:")
        for path, animations in sorted(sets.items(), key=lambda kv: -len(kv[1]))[:25]:
            print(f"  {len(animations):4d} slots  {path}")
        print(f"\n{len(sets)} scripts in total. A humanoid borrowing the human rig")
        print("wants graph/obj3d/interactive/player/player or .../npc/human_base/human_base.\n")

        self.report({'INFO'}, f"{len(sets)} animation sets; see the console")
        return {'FINISHED'}


class ArxAnimationTestProperties(PropertyGroup):
    model: StringProperty(name="Model", description="Selected NPC model")
    # Layer 0: Base/movement animation (walk, run, idle)
    animation_layer0: StringProperty(name="Layer 0 (Base)", description="Base animation - movement, idle (full body)")
    # Layer 1: Primary overlay (combat stance, actions)
    animation_layer1: StringProperty(name="Layer 1 (Overlay)", description="Overlay animation - typically upper body")
    # Layer 2: Secondary overlay
    animation_layer2: StringProperty(name="Layer 2", description="Secondary overlay animation")
    # Layer 3: Tertiary overlay
    animation_layer3: StringProperty(name="Layer 3", description="Tertiary overlay animation")
    # Legacy single animation field
    animation: StringProperty(name="Animation", description="Selected animation")
    flip_w: BoolProperty(name="Flip W", default=False, description="Flip quaternion W component")
    flip_x: BoolProperty(name="Flip X", default=False, description="Flip quaternion X component")
    flip_y: BoolProperty(name="Flip Y", default=False, description="Flip quaternion Y component")
    flip_z: BoolProperty(name="Flip Z", default=False, description="Flip quaternion Z component")
    negate_overlay_quat: BoolProperty(
        name="Negate Overlay Quat",
        default=True,
        description="Negate quaternion for overlay layers (fixes backwards arms)"
    )
    axis_mapping: EnumProperty(
        name="Axis Mapping",
        items=[
            ('XYZ', "X→X, Y→Y, Z→Z", "No remapping"),
            ('XZY', "X→X, Y→Z, Z→-Y", "Map Y to Z, Z to -Y"),
            ('YZX', "X→-Z, Y→Y, Z→X", "Map X to -Z, Z to X"),
            ('ZXY', "X→Y, Y→Z, Z→X", "Map X to Y, Y to Z, Z to X"),
            ('ZYX', "X→Z, Y→Y, Z→X", "Map X to Z, Z to X"),
            ('YXZ', "X→Y, Y→X, Z→Z", "Map X to Y, Y to X")
        ],
        default='ZXY',
        description="Axis remapping for quaternions"
    )

class ArxModelListProperties(PropertyGroup):
    model_list: CollectionProperty(type=bpy.types.PropertyGroup)
    model_list_loaded: BoolProperty(default=False)

class ArxModelListItem(PropertyGroup):
    name: StringProperty()

class ArxOperatorRefreshModelList(Operator):
    bl_idname = "arx.refresh_model_list"
    bl_label = "Refresh Model List"
    
    def execute(self, context):
        addon = getAddon(context)
        arx_files = addon.arxFiles
        
        arx_files.updateAll()
        context.scene.arx_model_list_props.model_list.clear()
        
        for key in arx_files.models.data.keys():
            if key[0] == "npc":
                item = context.scene.arx_model_list_props.model_list.add()
                item.name = key[-1]
        
        context.scene.arx_model_list_props.model_list_loaded = True
        
        print(f"Arx directory: {arx_files.rootPath}")
        print(f"Models: {list(arx_files.models.data.keys())}")
        print(f"Animations: {list(arx_files.animations.data.keys())}")
        
        if not context.scene.arx_model_list_props.model_list:
            self.report({'WARNING'}, "No NPC models found")
        else:
            self.report({'INFO'}, f"Found {len(context.scene.arx_model_list_props.model_list)} NPC models")
        
        return {'FINISHED'}

class ArxOperatorTestGoblinAnimations(Operator):
    bl_idname = "arx.test_goblin_animations"
    bl_label = "Test Selected Animation"
    bl_options = {'REGISTER'}

    def invoke(self, context, event):
        return context.window_manager.invoke_confirm(self, event)

    def execute(self, context):
        addon = getAddon(context)
        arx_files = addon.arxFiles
        if not arx_files.models.data or not arx_files.animations.data:
            arx_files.updateAll()
            print(f"Models: {list(arx_files.models.data.keys())}")
            print(f"Animations: {list(arx_files.animations.data.keys())}")
        
        bpy.ops.object.select_all(action='SELECT')
        bpy.ops.object.delete(use_global=False)
        for action in bpy.data.actions:
            bpy.data.actions.remove(action)
        for collection in bpy.data.collections:
            bpy.data.collections.remove(collection)
        for mesh in bpy.data.meshes:
            bpy.data.meshes.remove(mesh)
        for armature in bpy.data.armatures:
            bpy.data.armatures.remove(armature)
        
        props = context.scene.arx_animation_test
        model_name = props.model
        if not model_name:
            self.report({'ERROR'}, "No model selected")
            return {'CANCELLED'}
        
        model_key = tuple(["npc", model_name])
        if model_key not in arx_files.models.data:
            self.report({'ERROR'}, f"Model {model_name} not found in ArxFiles")
            return {'CANCELLED'}
        
        model_data = arx_files.models.data[model_key]
        model_path = os.path.join(model_data.path, model_data.model)
        
        try:
            addon.objectManager.loadFile(context, model_path, context.scene, import_tweaks=False)
        except ArxException as e:
            self.report({'ERROR'}, f"Failed to import model {model_name}: {str(e)}")
            return {'CANCELLED'}
        
        obj = None
        for o in bpy.data.objects:
            if o.name.startswith(f"npc/{model_name}") and o.type == 'MESH':
                obj = o
                break
        if not obj:
            self.report({'ERROR'}, f"Model mesh {model_name} not found")
            return {'CANCELLED'}
        
        armature_obj = None
        for o in bpy.data.objects:
            if o.name.startswith(f"npc/{model_name}") and o.type == 'ARMATURE':
                armature_obj = o
                break
        if not armature_obj:
            for modifier in obj.modifiers:
                if modifier.type == 'ARMATURE' and modifier.object:
                    armature_obj = modifier.object
                    break
        if not armature_obj:
            self.report({'ERROR'}, f"No armature found for mesh '{obj.name}'")
            return {'CANCELLED'}
        
        anim_name = props.animation
        if not anim_name:
            self.report({'ERROR'}, "No animation selected")
            return {'CANCELLED'}
        
        anim_key = anim_name
        if anim_key not in arx_files.animations.data:
            self.report({'ERROR'}, f"Animation {anim_key} not found in ArxFiles")
            return {'CANCELLED'}
        
        anim_path = arx_files.animations.data[anim_key]
        frame_rate = context.scene.render.fps
        
        bpy.context.view_layer.objects.active = obj
        obj.select_set(True)
        try:
            action = addon.animationManager.loadAnimation(
                anim_path, 
                f"g{model_name}_{anim_name}", 
                frame_rate=frame_rate, 
                axis_transform=None,
                flip_w=props.flip_w,
                flip_x=props.flip_x,
                flip_y=props.flip_y,
                flip_z=props.flip_z
            )
            if action is None:
                self.report({'ERROR'}, f"Failed to apply animation {anim_key}: possible group count mismatch (check log)")
                return {'CANCELLED'}
            self.report({'INFO'}, f"Imported animation: {anim_name}")
        except ArxException as e:
            self.report({'ERROR'}, f"Failed to import {anim_key}: {str(e)}")
            return {'CANCELLED'}
        
        return {'FINISHED'}

class CUSTOM_OT_arx_area_export_fts(Operator):
    bl_idname = "arx.area_export_fts"
    bl_label = "Export FTS"
    bl_description = "Export area background geometry to FTS format only"
    
    def invoke(self, context, event):
        """Call main export operator with FTS only"""
        print("DEBUG: FTS export button pressed")
        return bpy.ops.arx.area_list_export_all('INVOKE_DEFAULT', export_fts=True, export_llf=False, export_dlf=False)

class CUSTOM_OT_arx_area_export_llf(Operator):
    bl_idname = "arx.area_export_llf"
    bl_label = "Export LLF"
    bl_description = "Export lighting data to LLF format only"
    
    def invoke(self, context, event):
        area_list = context.window_manager.arx_areas_col
        if not area_list:
            self.report({'ERROR'}, "No area list loaded")
            return {'CANCELLED'}
            
        area = area_list[context.window_manager.arx_areas_idx]
        scene_name = f"Area_{area.area_id:02d}"
        scene = bpy.data.scenes.get(scene_name)
        
        if not scene:
            self.report({'ERROR'}, f"Scene '{scene_name}' not found. Import the area first.")
            return {'CANCELLED'}
        
        try:
            self.exportLlf(context, scene, area.area_id)
            self.report({'INFO'}, f"Exported LLF for Area {area.area_id}")
        except Exception as e:
            self.report({'ERROR'}, f"LLF export failed: {str(e)}")
            return {'CANCELLED'}
            
        return {'FINISHED'}
    
    def invoke(self, context, event):
        """Call main export operator with LLF only"""
        return bpy.ops.arx.area_list_export_all('INVOKE_DEFAULT', export_fts=False, export_llf=True, export_dlf=False)

class CUSTOM_OT_arx_area_export_dlf(Operator):
    bl_idname = "arx.area_export_dlf"
    bl_label = "Export DLF"
    bl_description = "Export entity and level data to DLF format only"
    
    def invoke(self, context, event):
        area_list = context.window_manager.arx_areas_col
        if not area_list:
            self.report({'ERROR'}, "No area list loaded")
            return {'CANCELLED'}
            
        area = area_list[context.window_manager.arx_areas_idx]
        scene_name = f"Area_{area.area_id:02d}"
        scene = bpy.data.scenes.get(scene_name)
        
        if not scene:
            self.report({'ERROR'}, f"Scene '{scene_name}' not found. Import the area first.")
            return {'CANCELLED'}
        
        try:
            self.exportDlf(context, scene, area.area_id)
            self.report({'INFO'}, f"Exported DLF for Area {area.area_id}")
        except Exception as e:
            self.report({'ERROR'}, f"DLF export failed: {str(e)}")
            return {'CANCELLED'}
            
        return {'FINISHED'}
    
    def invoke(self, context, event):
        """Call main export operator with DLF only"""
        return bpy.ops.arx.area_list_export_all('INVOKE_DEFAULT', export_fts=False, export_llf=False, export_dlf=True)

class ArxAnimationTestPanel(Panel):
    bl_idname = "SCENE_PT_arx_animation_test"
    bl_label = "Arx Animation Test"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context = "scene"

    def draw(self, context):
        layout = self.layout
        props = context.scene.arx_animation_test
        addon = getAddon(context)
        arx_files = addon.arxFiles
        
        if not context.scene.arx_model_list_props.model_list_loaded:
            layout.operator("arx.refresh_model_list", text="Load Models")
            return
        
        layout.operator("arx.refresh_model_list", text="Refresh Models")
        
        if not context.scene.arx_model_list_props.model_list:
            layout.label(text="WARNING: No NPC models found", icon='ERROR')
            return
        
        row = layout.row()
        row.label(text="Model:")
        row.operator("arx.select_model", text=props.model if props.model else "Select Model")
        
        if props.model:
            anim_list = sorted(animation_slots_for_model(context, props.model))
            if not anim_list:
                box = layout.box()
                box.label(text="This model's script binds no animations", icon='ERROR')
                box.label(text="Animations come from LOADANIM in an entity script,")
                box.label(text="not from the file name. Use List Animation Sets to")
                box.label(text="find a script whose set this model can borrow.")
            else:
                layout.label(text=f"{len(anim_list)} animations declared by its script",
                             icon='CHECKMARK')
            layout.operator("arx.list_animation_sets", icon='PRESET')

            # Animation Layer Selection
            layout.separator()
            layout.label(text="Animation Layers (Arx uses 4 layers):")

            # Layer 0 - Base/Movement
            box = layout.box()
            box.label(text="Layer 0 (Base - Movement/Idle):", icon='ARMATURE_DATA')
            row = box.row()
            op = row.operator("arx.select_animation_layer", text=props.animation_layer0 if props.animation_layer0 else "Select Base Animation")
            op.layer = 0
            if props.animation_layer0:
                row.operator("arx.clear_animation_layer", text="", icon='X').layer = 0

            # Layer 1 - Primary Overlay
            box = layout.box()
            box.label(text="Layer 1 (Overlay - Combat/Action):", icon='MOD_ARMATURE')
            row = box.row()
            op = row.operator("arx.select_animation_layer", text=props.animation_layer1 if props.animation_layer1 else "Select Overlay Animation")
            op.layer = 1
            if props.animation_layer1:
                row.operator("arx.clear_animation_layer", text="", icon='X').layer = 1

            # Layer 2 - Secondary Overlay
            box = layout.box()
            box.label(text="Layer 2 (Secondary Overlay):", icon='MOD_ARMATURE')
            row = box.row()
            op = row.operator("arx.select_animation_layer", text=props.animation_layer2 if props.animation_layer2 else "Select Animation")
            op.layer = 2
            if props.animation_layer2:
                row.operator("arx.clear_animation_layer", text="", icon='X').layer = 2

            # Layer 3 - Tertiary Overlay
            box = layout.box()
            box.label(text="Layer 3 (Tertiary Overlay):", icon='MOD_ARMATURE')
            row = box.row()
            op = row.operator("arx.select_animation_layer", text=props.animation_layer3 if props.animation_layer3 else "Select Animation")
            op.layer = 3
            if props.animation_layer3:
                row.operator("arx.clear_animation_layer", text="", icon='X').layer = 3

            layout.separator()

            # Legacy single animation (for backwards compatibility)
            row = layout.row()
            row.label(text="Single Animation (Legacy):")
            row.operator("arx.select_animation", text=props.animation if props.animation else "Select Animation")

        layout.separator()
        layout.prop(props, "axis_mapping", text="Axis Mapping")
        layout.prop(props, "flip_w", text="Flip W")
        layout.prop(props, "flip_x", text="Flip X")
        layout.prop(props, "flip_y", text="Flip Y")
        layout.prop(props, "flip_z", text="Flip Z")
        layout.prop(props, "negate_overlay_quat", text="Negate Overlay Quaternion")

        layout.separator()
        layout.operator("arx.test_layered_animations", text="Test Layered Animations", icon='PLAY')
        layout.operator("arx.test_goblin_animations", text="Test Single Animation (Legacy)")

class ArxSelectModelOperator(Operator):
    bl_idname = "arx.select_model"
    bl_label = "Select Model"
    model: StringProperty()

    def invoke(self, context, event):
        context.window_manager.invoke_props_dialog(self, width=200)
        return {'RUNNING_MODAL'}

    def draw(self, context):
        layout = self.layout
        model_list = context.scene.arx_model_list_props.model_list
        for item in model_list:
            layout.operator("arx.set_model", text=item.name).model = item.name
        if not model_list:
            layout.label(text="No models available")

    def execute(self, context):
        return {'FINISHED'}

class ArxSetModelOperator(Operator):
    bl_idname = "arx.set_model"
    bl_label = "Set Model"
    model: StringProperty()

    def execute(self, context):
        props = context.scene.arx_animation_test
        props.model = self.model
        props.animation = ""
        return {'FINISHED'}

class ArxSelectAnimationOperator(Operator):
    bl_idname = "arx.select_animation"
    bl_label = "Select Animation"
    animation: StringProperty()

    def invoke(self, context, event):
        context.window_manager.invoke_props_dialog(self, width=200)
        return {'RUNNING_MODAL'}

    def draw(self, context):
        layout = self.layout
        props = context.scene.arx_animation_test
        arx_files = getAddon(context).arxFiles
        matching_anims = []
        model_words = props.model.lower().split('_')
        for anim in sorted(arx_files.animations.data.keys()):
            anim_lower = anim.lower()
            if any(word in anim_lower for word in model_words):
                matching_anims.append(anim)
        
        for anim in matching_anims:
            display_name = anim.replace('.tea', '') if anim.endswith('.tea') else anim
            layout.operator("arx.set_animation", text=display_name).animation = anim
        
        if not matching_anims:
            layout.label(text="No animations available")

    def execute(self, context):
        return {'FINISHED'}

class ArxSetAnimationOperator(Operator):
    bl_idname = "arx.set_animation"
    bl_label = "Set Animation"
    animation: StringProperty()

    def execute(self, context):
        props = context.scene.arx_animation_test
        props.animation = self.animation
        return {'FINISHED'}


class ArxSelectAnimationLayerOperator(Operator):
    """Select animation for a specific layer"""
    bl_idname = "arx.select_animation_layer"
    bl_label = "Select Animation for Layer"
    layer: IntProperty(default=0, min=0, max=3)

    def invoke(self, context, event):
        context.window_manager.invoke_props_dialog(self, width=300)
        return {'RUNNING_MODAL'}

    def draw(self, context):
        layout = self.layout
        props = context.scene.arx_animation_test
        arx_files = getAddon(context).arxFiles

        layout.label(text=f"Select animation for Layer {self.layer}:")

        # Filter animations by model name
        model_words = props.model.lower().split('_')
        matching_anims = []
        for anim in sorted(arx_files.animations.data.keys()):
            anim_lower = anim.lower()
            if any(word in anim_lower for word in model_words):
                matching_anims.append(anim)

        if not matching_anims:
            layout.label(text="No animations found for model", icon='ERROR')
            return

        # Categorize animations for easier selection
        walk_anims = [a for a in matching_anims if 'walk' in a.lower() or 'run' in a.lower() or 'wait' in a.lower()]
        combat_anims = [a for a in matching_anims if any(w in a.lower() for w in ['strike', 'hit', 'fight', 'bare', '1h', '2h', 'dagger', 'missile'])]
        other_anims = [a for a in matching_anims if a not in walk_anims and a not in combat_anims]

        if self.layer == 0 and walk_anims:
            layout.label(text="Movement Animations:", icon='ARMATURE_DATA')
            for anim in walk_anims[:10]:  # Limit to prevent huge dialogs
                display_name = anim.replace('.tea', '')
                op = layout.operator("arx.set_animation_layer", text=display_name)
                op.animation = anim
                op.layer = self.layer

        if self.layer >= 1 and combat_anims:
            layout.label(text="Combat/Action Animations:", icon='MOD_ARMATURE')
            for anim in combat_anims[:10]:
                display_name = anim.replace('.tea', '')
                op = layout.operator("arx.set_animation_layer", text=display_name)
                op.animation = anim
                op.layer = self.layer

        layout.separator()
        layout.label(text="All Matching Animations:")
        for anim in matching_anims[:20]:  # Limit display
            display_name = anim.replace('.tea', '')
            op = layout.operator("arx.set_animation_layer", text=display_name)
            op.animation = anim
            op.layer = self.layer

    def execute(self, context):
        return {'FINISHED'}


class ArxSetAnimationLayerOperator(Operator):
    """Set animation for a specific layer"""
    bl_idname = "arx.set_animation_layer"
    bl_label = "Set Animation Layer"
    animation: StringProperty()
    layer: IntProperty(default=0, min=0, max=3)

    def execute(self, context):
        props = context.scene.arx_animation_test
        if self.layer == 0:
            props.animation_layer0 = self.animation
        elif self.layer == 1:
            props.animation_layer1 = self.animation
        elif self.layer == 2:
            props.animation_layer2 = self.animation
        elif self.layer == 3:
            props.animation_layer3 = self.animation
        return {'FINISHED'}


class ArxClearAnimationLayerOperator(Operator):
    """Clear animation from a layer"""
    bl_idname = "arx.clear_animation_layer"
    bl_label = "Clear Animation Layer"
    layer: IntProperty(default=0, min=0, max=3)

    def execute(self, context):
        props = context.scene.arx_animation_test
        if self.layer == 0:
            props.animation_layer0 = ""
        elif self.layer == 1:
            props.animation_layer1 = ""
        elif self.layer == 2:
            props.animation_layer2 = ""
        elif self.layer == 3:
            props.animation_layer3 = ""
        return {'FINISHED'}


class ArxTestLayeredAnimationsOperator(Operator):
    """
    Composite layered animations using Arx engine logic.

    This bypasses Blender's NLA and computes final bone transforms
    exactly like the Arx engine does:
    - Process layers 3 → 2 → 1 → 0 (highest priority first)
    - For each bone, use highest layer with non-void data
    - Bake result as single animation
    """
    bl_idname = "arx.test_layered_animations"
    bl_label = "Composite Layered Animations"
    bl_options = {'REGISTER'}

    def invoke(self, context, event):
        return context.window_manager.invoke_confirm(self, event)

    def execute(self, context):
        from .arx_io_animation import ArxLayerCompositor

        addon = getAddon(context)
        arx_files = addon.arxFiles
        if not arx_files.models.data or not arx_files.animations.data:
            arx_files.updateAll()

        # Clear scene
        bpy.ops.object.select_all(action='SELECT')
        bpy.ops.object.delete(use_global=False)
        for action in bpy.data.actions:
            bpy.data.actions.remove(action)
        for collection in bpy.data.collections:
            bpy.data.collections.remove(collection)
        for mesh in bpy.data.meshes:
            bpy.data.meshes.remove(mesh)
        for armature in bpy.data.armatures:
            bpy.data.armatures.remove(armature)

        props = context.scene.arx_animation_test
        model_name = props.model
        if not model_name:
            self.report({'ERROR'}, "No model selected")
            return {'CANCELLED'}

        # Load model
        model_key = tuple(["npc", model_name])
        if model_key not in arx_files.models.data:
            self.report({'ERROR'}, f"Model {model_name} not found")
            return {'CANCELLED'}

        model_data = arx_files.models.data[model_key]
        model_path = os.path.join(model_data.path, model_data.model)

        try:
            addon.objectManager.loadFile(context, model_path, context.scene, import_tweaks=False)
        except ArxException as e:
            self.report({'ERROR'}, f"Failed to import model: {str(e)}")
            return {'CANCELLED'}

        # Find mesh and armature
        obj = None
        armature_obj = None
        for o in bpy.data.objects:
            if o.name.startswith(f"npc/{model_name}"):
                if o.type == 'MESH':
                    obj = o
                elif o.type == 'ARMATURE':
                    armature_obj = o

        if not obj:
            self.report({'ERROR'}, "Model mesh not found")
            return {'CANCELLED'}

        if not armature_obj:
            for modifier in obj.modifiers:
                if modifier.type == 'ARMATURE' and modifier.object:
                    armature_obj = modifier.object
                    break

        if not armature_obj:
            self.report({'ERROR'}, "No armature found")
            return {'CANCELLED'}

        # Get animation paths for each layer
        layer_anims = [
            props.animation_layer0,
            props.animation_layer1,
            props.animation_layer2,
            props.animation_layer3
        ]

        layer_paths = {}
        for layer_idx, anim_name in enumerate(layer_anims):
            if not anim_name:
                continue

            if anim_name not in arx_files.animations.data:
                self.report({'WARNING'}, f"Animation {anim_name} not found for layer {layer_idx}")
                continue

            layer_paths[layer_idx] = arx_files.animations.data[anim_name]
            self.report({'INFO'}, f"Layer {layer_idx}: {anim_name}")

        if not layer_paths:
            self.report({'ERROR'}, "No valid animations selected")
            return {'CANCELLED'}

        # Use the layer compositor to create properly blended animation
        compositor = ArxLayerCompositor()

        frame_rate = context.scene.render.fps
        action = compositor.composite_from_paths(
            armature_obj,
            layer_paths,
            frame_rate=frame_rate,
            scale_factor=0.1,
            flip_w=props.flip_w,
            flip_x=props.flip_x,
            flip_y=props.flip_y,
            flip_z=props.flip_z,
            negate_overlay_quat=props.negate_overlay_quat
        )

        if not action:
            self.report({'ERROR'}, "Layer composition failed")
            return {'CANCELLED'}

        # Set scene frame range
        context.scene.frame_start = 1
        context.scene.frame_end = int(action.frame_range[1])

        # Select armature for preview
        bpy.ops.object.select_all(action='DESELECT')
        armature_obj.select_set(True)
        bpy.context.view_layer.objects.active = armature_obj

        self.report({'INFO'}, f"Composited {len(layer_paths)} layers into {int(action.frame_range[1])} frames")
        return {'FINISHED'}


def is_portal_object(obj):
    """A portal is a mesh sitting in the scene's portals collection."""
    if not obj or obj.type != 'MESH':
        return False
    if 'arx_room_1' in obj:
        return True
    return any('portal' in collection.name.lower() for collection in obj.users_collection)


def portal_corners_in_arx(obj):
    """The portal's corners in Arx coordinates, in the order the exporter writes them."""
    mesh = obj.data
    if not mesh.polygons:
        return []

    face = mesh.polygons[0]
    corners = []
    for i in range(4):
        index = face.vertices[i] if i < len(face.vertices) else face.vertices[-1]
        world = obj.matrix_world @ mesh.vertices[index].co
        corners.append(Vector(blender_pos_to_arx(world)) * 10.0)

    # Same swap the exporter applies to restore FTS Z order.
    corners[2], corners[3] = corners[3], corners[2]
    return corners


# --- Navigation mesh -------------------------------------------------------
#
# A level's anchors are its pathfinding graph, and the addon used to only ever
# copy them out of the original FTS. Geometry built or reshaped in Blender
# therefore had nowhere for an NPC to stand, and the failure is silent: NPCs
# just never move. arx.generate_anchors rebuilds the graph from whatever is in
# the scene, and exportArea falls back to it rather than write a level nothing
# can walk in.
#
# The graph lives in the scene as the mesh the importer already makes: one
# vertex per anchor, one edge per link. The cylinder each anchor was measured
# for rides along as per-vertex attributes, because it is per anchor - DANAE
# grew each one until it touched something.
ANCHOR_RADIUS_ATTRIBUTE = 'arx_anchor_radius'
ANCHOR_HEIGHT_ATTRIBUTE = 'arx_anchor_height'
ANCHOR_FLAGS_ATTRIBUTE = 'arx_anchor_flags'

POLY_NOPATH = 1 << 18


def anchor_mesh_object(scene):
    """The scene's anchor mesh, wherever the importer or the user put it."""
    name = scene.name + '-anchors'
    for obj in scene.objects:
        if obj.type == 'MESH' and (obj.name == name or 'anchor' in obj.name.lower()):
            return obj
    return None


def background_mesh_object(scene):
    for obj in scene.objects:
        if obj.type == 'MESH' and obj.name.endswith('-background'):
            return obj
    return None


def arx_polygons_of(mesh_obj):
    """A Blender mesh as polygons in Arx world coordinates.

    The same conversion convertMeshToFtsCells does. blender_pos_to_arx is a
    handedness preserving permutation, so it is valid on the normals too.
    """
    mesh = mesh_obj.data
    matrix = mesh_obj.matrix_world
    rotation = matrix.to_3x3()
    polytype = mesh.attributes.get('arx_polytype')
    polygons = []
    for face in mesh.polygons:
        verts = [tuple(Vector(blender_pos_to_arx(matrix @ mesh.vertices[i].co)) * 10.0)
                 for i in face.vertices]
        normal = Vector(blender_pos_to_arx((rotation @ face.normal).normalized()))
        nopath = False
        if polytype is not None:
            nopath = bool(polytype.data[face.index].value & POLY_NOPATH)
        polygons.append({'v': verts, 'norm': tuple(normal), 'nopath': nopath})
    return polygons


def write_anchor_mesh(scene, anchors):
    """Replace the scene's anchor mesh with this graph."""
    mesh = bpy.data.meshes.new(scene.name + '-anchors-mesh')
    verts = [tuple(arx_pos_to_blender_for_model(a['pos']) * 0.1) for a in anchors]
    edges = sorted({(min(i, j), max(i, j))
                    for i, anchor in enumerate(anchors) for j in anchor['links']})
    mesh.from_pydata(verts, edges, [])
    mesh.update()
    store_anchor_attributes(mesh, anchors)

    obj = anchor_mesh_object(scene)
    if obj:
        old = obj.data
        obj.data = mesh
        if old.users == 0:
            bpy.data.meshes.remove(old)
    else:
        obj = bpy.data.objects.new(scene.name + '-anchors', mesh)
        scene.collection.objects.link(obj)
    obj.display_type = 'WIRE'
    return obj


def store_anchor_attributes(mesh, anchors):
    """Keep each anchor's cylinder on the mesh, one value per vertex.

    Without this the export has nothing to write and falls back to a default.
    That default used to be radius 50 and height +100, and a positive height
    fails the `height <= cyl.height` test in AnchorData_GetNearest against a
    human's -165 - so every anchor a level round tripped through Blender was
    quietly unusable, and the level came out with a navmesh that pathfinding
    would not touch.
    """
    for name, key, kind in ((ANCHOR_RADIUS_ATTRIBUTE, 'radius', 'FLOAT'),
                            (ANCHOR_HEIGHT_ATTRIBUTE, 'height', 'FLOAT'),
                            (ANCHOR_FLAGS_ATTRIBUTE, 'flags', 'INT')):
        existing = mesh.attributes.get(name)
        if existing is not None:
            mesh.attributes.remove(existing)
        attribute = mesh.attributes.new(name, kind, 'POINT')
        for index, anchor in enumerate(anchors):
            attribute.data[index].value = anchor[key]


def read_anchor_attributes(mesh):
    """Per-vertex cylinders, defaulting to what the engine will actually accept."""
    count = len(mesh.vertices)
    values = {}
    for name, key, default in (
            (ANCHOR_RADIUS_ATTRIBUTE, 'radius', anchor_generation.ANCHOR_RADIUS),
            (ANCHOR_HEIGHT_ATTRIBUTE, 'height', anchor_generation.ANCHOR_HEIGHT),
            (ANCHOR_FLAGS_ATTRIBUTE, 'flags', 0)):
        attribute = mesh.attributes.get(name)
        if attribute is None or attribute.domain != 'POINT' or len(attribute.data) != count:
            values[key] = [default] * count
        else:
            values[key] = [item.value for item in attribute.data]
    return values


def generate_anchors_for_scene(scene, report=None):
    """Build the graph from the scene's background mesh and store it."""
    background = background_mesh_object(scene)
    if not background:
        if report:
            report({'ERROR'}, f"No background mesh in scene '{scene.name}'")
        return None
    anchors = anchor_generation.build_anchors(arx_polygons_of(background))
    if not anchors:
        if report:
            report({'ERROR'}, "No walkable floor found, so there is nothing to anchor to")
        return None
    write_anchor_mesh(scene, anchors)
    if report:
        links = sum(len(a['links']) for a in anchors) // 2
        report({'INFO'}, f"Generated {len(anchors)} anchors and {links} links")
    return anchors


class ARX_OT_generate_anchors(Operator):
    bl_idname = "arx.generate_anchors"
    bl_label = "Generate Anchors"
    bl_description = ("Rebuild the pathfinding graph from the background geometry. "
                      "NPCs cannot move in a level that has none")
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        scene = context.scene
        if generate_anchors_for_scene(scene, self.report) is None:
            return {'CANCELLED'}
        return {'FINISHED'}


class ARX_OT_portal_init(Operator):
    bl_idname = "arx.portal_init"
    bl_label = "Add Portal Properties"
    bl_description = "Give this object the room links a portal needs"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        obj = context.object
        obj['arx_room_1'] = obj.get('arx_room_1', 0)
        obj['arx_room_2'] = obj.get('arx_room_2', 0)
        obj['arx_useportal'] = obj.get('arx_useportal', 1)
        self.report({'INFO'}, "Portal properties added; set the two rooms it joins")
        return {'FINISHED'}


# src/ai/Paths.h
PATH_AMBIANCE = 1 << 1
PATH_RGB = 1 << 2
PATH_FARCLIP = 1 << 3


def _zone_flag(bit):
    """A checkbox backed by one bit of the zone's flags, kept on the object itself."""

    def getter(self):
        return bool(self.id_data.get("arx_zone_flags", 0) & bit)

    def setter(self, value):
        flags = self.id_data.get("arx_zone_flags", 0)
        self.id_data["arx_zone_flags"] = (flags | bit) if value else (flags & ~bit)

    return getter, setter


def _zone_colour_get(self):
    return tuple(self.id_data.get("arx_zone_rgb", (0.0, 0.0, 0.0)))


def _zone_colour_set(self, value):
    self.id_data["arx_zone_rgb"] = tuple(value)


_ambiance_get, _ambiance_set = _zone_flag(PATH_AMBIANCE)
_rgb_get, _rgb_set = _zone_flag(PATH_RGB)
_farclip_get, _farclip_set = _zone_flag(PATH_FARCLIP)


class ARX_zone_properties(PropertyGroup):
    """The zone flags, as named switches over the bitfield the format stores."""

    use_ambiance: BoolProperty(
        name="Ambiance",
        description="Play the named ambient track while the player is inside",
        get=_ambiance_get, set=_ambiance_set)

    use_rgb: BoolProperty(
        name="Colour",
        description="Tint the view while the player is inside",
        get=_rgb_get, set=_rgb_set)

    use_farclip: BoolProperty(
        name="Far Clip",
        description="Override the view distance while the player is inside",
        get=_farclip_get, set=_farclip_set)

    colour: FloatVectorProperty(
        name="Tint", subtype='COLOR', size=3, min=0.0, max=1.0,
        get=_zone_colour_get, set=_zone_colour_set)


class ARX_PT_zone_properties(Panel):
    bl_idname = "OBJECT_PT_arx_zone"
    bl_label = "Arx Zone"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context = "object"

    @classmethod
    def poll(cls, context):
        obj = context.object
        return obj is not None and 'arx_zone_height' in obj

    def draw(self, context):
        layout = self.layout
        obj = context.object
        zone = obj.arx_zone

        layout.prop(obj, '["arx_zone_height"]', text="Height")
        if obj.get("arx_zone_height", 0) == 0:
            box = layout.box()
            box.label(text="A height of zero makes this a path, not a zone", icon='ERROR')
            box.label(text="The engine only treats it as a volume when height is set.")

        column = layout.column(align=True)
        column.label(text="Behaviour while the player is inside:")
        column.prop(zone, "use_ambiance")
        if zone.use_ambiance:
            row = column.row(align=True)
            row.prop(obj, '["arx_zone_ambiance"]', text="Track")
            column.prop(obj, '["arx_zone_amb_max_vol"]', text="Max Volume")
        column.prop(zone, "use_rgb")
        if zone.use_rgb:
            column.prop(zone, "colour")
        column.prop(zone, "use_farclip")
        if zone.use_farclip:
            column.prop(obj, '["arx_zone_farclip"]', text="View Distance")

        outline = next((child for child in obj.children
                        if child.name.startswith('zone_outline:')
                        and child.type == 'MESH'), None)
        info = layout.box()
        if outline is None:
            legacy = [child for child in obj.children
                      if child.name.startswith('zone_waypoint:')]
            info.label(text=f"{len(legacy)} waypoint empties (old style)", icon='INFO')
            info.label(text="Reimport to get an editable outline mesh.")
            return

        points = len(outline.data.vertices)
        info.label(text=f"Outline: {points} points, edit it like any mesh", icon='INFO')
        if points < 3:
            info.label(text="Fewer than three cannot enclose anything", icon='ERROR')
        info.label(text="Only the footprint matters; the engine tests X and Z.")


class ARX_face_tool_properties(PropertyGroup):
    """Scratch values for editing Arx face attributes by hand."""

    room: IntProperty(
        name="Room",
        description="Room id to assign. Rooms are numbered from 1; 0 is the engine's "
                    "unused first slot and -1 means the polygon belongs to no room",
        default=1,
        min=-1
    )


def arx_face_layer(obj, name):
    """The named face attribute, whichever mode the object happens to be in.

    Returns (accessor, is_bmesh). In edit mode Blender keeps the live data in a
    bmesh and the mesh attribute arrays are stale, so the two cases cannot share
    a code path.
    """
    import bmesh

    if obj.mode == 'EDIT':
        bm = bmesh.from_edit_mesh(obj.data)
        return bm, bm.faces.layers.int.get(name)
    return None, obj.data.attributes.get(name)


class ARX_OT_assign_face_room(Operator):
    bl_idname = "arx.assign_face_room"
    bl_label = "Assign Room To Selected"
    bl_description = "Write the room id above onto every selected face"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        import bmesh

        obj = context.object
        room = context.scene.arx_face_tools.room
        bm, layer = arx_face_layer(obj, 'arx_room')
        if layer is None:
            self.report({'ERROR'}, "This mesh has no arx_room attribute; import a level first")
            return {'CANCELLED'}

        count = 0
        if bm is not None:
            for face in bm.faces:
                if face.select:
                    face[layer] = room
                    count += 1
            bmesh.update_edit_mesh(obj.data)
        else:
            for index, poly in enumerate(obj.data.polygons):
                if poly.select:
                    layer.data[index].value = room
                    count += 1

        if not count:
            self.report({'WARNING'}, "No faces selected")
            return {'CANCELLED'}

        self.report({'INFO'}, f"Set room {room} on {count} faces")
        return {'FINISHED'}


class ARX_OT_select_faces_by_room(Operator):
    bl_idname = "arx.select_faces_by_room"
    bl_label = "Select Faces In Room"
    bl_description = "Select every face carrying the room id above"
    bl_options = {'REGISTER', 'UNDO'}

    extend: BoolProperty(name="Extend", default=False)

    def execute(self, context):
        import bmesh

        obj = context.object
        room = context.scene.arx_face_tools.room
        bm, layer = arx_face_layer(obj, 'arx_room')
        if layer is None or bm is None:
            self.report({'ERROR'}, "Enter edit mode on a mesh with Arx attributes")
            return {'CANCELLED'}

        count = 0
        for face in bm.faces:
            if face[layer] == room:
                face.select_set(True)
                count += 1
            elif not self.extend:
                face.select_set(False)
        bm.select_flush(True)
        bmesh.update_edit_mesh(obj.data)

        self.report({'INFO'}, f"Selected {count} faces in room {room}")
        return {'FINISHED'}


class ARX_OT_report_room_usage(Operator):
    bl_idname = "arx.report_room_usage"
    bl_label = "List Rooms"
    bl_description = "Print how many faces each room holds, and the first free id"

    def execute(self, context):
        obj = context.object
        bm, layer = arx_face_layer(obj, 'arx_room')
        if layer is None:
            self.report({'ERROR'}, "This mesh has no arx_room attribute")
            return {'CANCELLED'}

        counts = {}
        if bm is not None:
            for face in bm.faces:
                counts[face[layer]] = counts.get(face[layer], 0) + 1
        else:
            for entry in layer.data:
                counts[entry.value] = counts.get(entry.value, 0) + 1

        print("\nArx rooms in this mesh:")
        for room in sorted(counts):
            print(f"  room {room:4d}: {counts[room]} faces")
        used = {room for room in counts if room > 0}
        free = next(i for i in range(1, len(used) + 2) if i not in used)
        print(f"  {len(used)} rooms in use, lowest free id is {free}")
        print(f"  the engine refuses to draw anything past {MAX_ROOMS} rooms\n")

        self.report({'INFO'}, f"{len(used)} rooms in use, lowest free id {free}; "
                              f"see the console for the breakdown")
        return {'FINISHED'}


class ARX_PT_face_attributes(Panel):
    bl_idname = "DATA_PT_arx_face_attributes"
    bl_label = "Arx Face Attributes"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context = "data"

    @classmethod
    def poll(cls, context):
        obj = context.object
        return obj and obj.type == 'MESH' and 'arx_room' in obj.data.attributes

    def draw(self, context):
        layout = self.layout
        obj = context.object
        tools = context.scene.arx_face_tools

        layout.prop(tools, "room")

        if obj.mode != 'EDIT':
            layout.label(text="Enter edit mode to work on a selection", icon='INFO')
        else:
            import bmesh
            bm = bmesh.from_edit_mesh(obj.data)
            layer = bm.faces.layers.int.get('arx_room')
            active = bm.faces.active
            # Only the active face is inspected here: walking every face on each
            # redraw would stall the panel on a level sized mesh.
            if active is not None and layer is not None:
                layout.label(text=f"Active face is in room {active[layer]}")
            layout.label(text=f"{obj.data.total_face_sel} faces selected")

        column = layout.column(align=True)
        column.operator("arx.assign_face_room", icon='CHECKMARK')
        row = column.row(align=True)
        row.enabled = (obj.mode == 'EDIT')
        row.operator("arx.select_faces_by_room", icon='RESTRICT_SELECT_OFF')
        layout.operator("arx.report_room_usage", icon='PRESET')

        box = layout.box()
        box.label(text="Rooms are authored, not derived", icon='INFO')
        box.label(text="Every room needs a portal or it is never drawn.")


class ARX_PT_portal_properties(Panel):
    bl_idname = "OBJECT_PT_arx_portal"
    bl_label = "Arx Portal"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context = "object"

    @classmethod
    def poll(cls, context):
        return is_portal_object(context.object)

    def draw(self, context):
        layout = self.layout
        obj = context.object

        if 'arx_room_1' not in obj:
            layout.label(text="No portal properties on this object", icon='ERROR')
            layout.label(text="A duplicated portal keeps the room ids it was copied from.")
            layout.operator("arx.portal_init", icon='ADD')
            return

        column = layout.column(align=True)
        column.prop(obj, '["arx_room_1"]', text="Room 1 (normal faces this way)")
        column.prop(obj, '["arx_room_2"]', text="Room 2")
        if 'arx_useportal' in obj:
            column.prop(obj, '["arx_useportal"]', text="Use Portal")

        room_1 = obj.get('arx_room_1', 0)
        room_2 = obj.get('arx_room_2', 0)

        box = layout.box()
        if room_1 == room_2:
            box.label(text="Both sides name the same room", icon='ERROR')
            box.label(text="The engine can never travel through this portal.")
        elif room_1 <= 0 or room_2 <= 0:
            box.label(text="Room 0 is the engine's unused slot", icon='ERROR')
            box.label(text="Rooms are numbered from 1.")

        corners = portal_corners_in_arx(obj)
        if len(corners) == 4:
            normal = portal_plane_normal(corners)
            centre = sum(corners, Vector((0.0, 0.0, 0.0))) / 4.0
            radius = max((corner - centre).length for corner in corners)
            info = layout.box()
            info.label(text="Computed for export", icon='INFO')
            info.label(text=f"Plane normal  {normal.x:.2f}, {normal.y:.2f}, {normal.z:.2f}")
            info.label(text=f"Bounding radius  {radius:.1f}")
            info.label(text="The exporter flips the winding if this normal does not "
                            "face Room 1.")
        else:
            layout.label(text="Portal mesh needs one quad", icon='ERROR')


class ArxLightingPanel(Panel):
    bl_idname = "SCENE_PT_arx_lighting"
    bl_label = "Arx Lighting Controls"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context = "scene"

    def draw(self, context):
        layout = self.layout
        props = context.scene.arx_lighting
        
        # Import settings
        box = layout.box()
        box.label(text="Import Settings", icon='IMPORT')
        box.prop(props, "import_original_lighting")
        
        # Export lighting settings
        box = layout.box()
        box.label(text="Export Lighting", icon='LIGHT')
        box.prop(props, "regenerate_lighting")
        
        if props.regenerate_lighting:
            box.prop(props, "lighting_method")
            
            # DANAE bake settings
            if props.lighting_method == 'DANAE':
                sub_box = box.box()
                sub_box.label(text="DANAE Bake Settings")
                sub_box.prop(props, "danae_use_normals")
                sub_box.prop(props, "danae_raylaunch")
                sub_box.prop(props, "danae_ambient")
                sub_box.label(text="Uses the Arx light properties, not Blender wattage", icon='INFO')

            # Cycles-specific settings
            elif props.lighting_method == 'CYCLES':
                sub_box = box.box()
                sub_box.label(text="Cycles Settings")
                sub_box.prop(props, "cycles_samples")
                sub_box.prop(props, "cycles_use_denoising")
                sub_box.separator()

                sub_box.prop(props, "cycles_bake_type")

                if props.cycles_bake_type == 'DIFFUSE':
                    sub_box.label(text="Pass Contributions:")
                    col = sub_box.column(align=True)
                    col.prop(props, "cycles_use_direct_light")
                    col.prop(props, "cycles_use_indirect_light")
                    col.prop(props, "cycles_use_color")
                elif props.cycles_bake_type == 'AO':
                    sub_box.prop(props, "cycles_ao_distance")
                
            # Simple lighting settings  
            elif props.lighting_method == 'SIMPLE':
                sub_box = box.box()
                sub_box.label(text="Simple Lighting Parameters")
                sub_box.prop(props, "ambient_strength")
                sub_box.prop(props, "light_falloff_power")
                sub_box.prop(props, "max_light_contribution")

            # Export intensity multiplier (renderer based methods only)
            if props.lighting_method not in {'DANAE', 'SKIP'}:
                box.separator()
                row = box.row()
                row.label(text="Export Settings", icon='EXPORT')
                row = box.row()
                row.prop(props, "export_intensity_multiplier", slider=True)
                row = box.row()
                row.label(text="Note: Multiplier only affects exported LLF, not viewport preview", icon='INFO')

        # Lighting operations
        box = layout.box()
        box.label(text="Operations", icon='TOOL_SETTINGS')
        row = box.row()
        row.operator("arx.regenerate_lighting", text="Regenerate Lighting")
        row.operator("arx.preview_lighting", text="Preview")

class CUSTOM_OT_arx_regenerate_lighting(Operator):
    bl_idname = "arx.regenerate_lighting"
    bl_label = "Regenerate Lighting"
    bl_description = "Regenerate vertex lighting for the current scene"

    def execute(self, context):
        scene = context.scene
        props = scene.arx_lighting

        # Find background mesh
        background_obj = None
        for obj in scene.objects:
            if obj.type == 'MESH' and obj.name.endswith('-background'):
                background_obj = obj
                break

        if not background_obj:
            self.report({'ERROR'}, "No background mesh found in scene")
            return {'CANCELLED'}

        self.report({'INFO'}, f"Starting {props.lighting_method} lighting regeneration on {background_obj.name}")

        if props.lighting_method == 'DANAE':
            # Same bake as the export path, run straight off the Blender mesh so the
            # result can be seen in the viewport. Colours land per loop, in loop
            # order; the export path is what reorders them for the .llf file.
            from .danae_lighting import DanaePolygon, compute_vertex_colors

            mesh = background_obj.data

            vcol_layer = None
            for vcol in mesh.vertex_colors:
                if vcol.name == "light-color":
                    vcol_layer = vcol
                    break
            if not vcol_layer:
                self.report({'ERROR'}, "No 'light-color' vertex color layer found!")
                return {'CANCELLED'}

            matrix = background_obj.matrix_world
            normal_matrix = matrix.to_3x3()

            polygons = []
            for poly in mesh.polygons:
                vertices = []
                normals = []
                for loop_idx in poly.loop_indices:
                    vertex = mesh.vertices[mesh.loops[loop_idx].vertex_index]
                    world = matrix @ vertex.co
                    vertices.append(tuple(Vector(blender_pos_to_arx(world)) * 10.0))
                    normal = (normal_matrix @ vertex.normal).normalized()
                    normals.append(tuple(Vector(blender_pos_to_arx(normal))))

                count = len(vertices)
                center = (
                    sum(v[0] for v in vertices) / count,
                    sum(v[1] for v in vertices) / count,
                    sum(v[2] for v in vertices) / count,
                )
                polygons.append(DanaePolygon(vertices=vertices, normals=normals,
                                             center=center, ignore=False))

            visible = None
            if props.danae_raylaunch:
                visible = build_danae_occluder([p.vertices for p in polygons])

            colors = compute_vertex_colors(
                polygons, gather_danae_lights(scene),
                ambient=(props.danae_ambient,) * 3,
                use_normals=props.danae_use_normals,
                visible=visible,
            )

            mesh.vertex_colors.active = vcol_layer
            position = 0
            for poly in mesh.polygons:
                for loop_idx in poly.loop_indices:
                    r, g, b, _ = colors[position]
                    vcol_layer.data[loop_idx].color = (r / 255.0, g / 255.0, b / 255.0, 1.0)
                    position += 1

            mesh.update()
            self.report({'INFO'}, f"DANAE lighting applied to {len(polygons)} polygons")

        elif props.lighting_method == 'CYCLES':
            # Use actual Cycles baking
            import bpy

            # Store original settings
            original_engine = scene.render.engine
            original_active = context.view_layer.objects.active
            original_selection = [obj for obj in scene.objects if obj.select_get()]

            try:
                # Switch to Cycles
                scene.render.engine = 'CYCLES'

                # Configure Cycles from user settings
                if hasattr(scene, 'cycles'):
                    scene.cycles.samples = props.cycles_samples
                    scene.cycles.use_denoising = props.cycles_use_denoising

                # Select and activate mesh
                for obj in scene.objects:
                    obj.select_set(False)
                background_obj.select_set(True)
                context.view_layer.objects.active = background_obj

                mesh = background_obj.data

                # CRITICAL: Use the SAME layer that materials expect: 'light-color'
                # The Arx Material node group looks for 'light-color' specifically!
                vcol_layer = None
                for vcol in mesh.vertex_colors:
                    if vcol.name == "light-color":
                        vcol_layer = vcol
                        break

                if not vcol_layer:
                    self.report({'ERROR'}, "No 'light-color' vertex color layer found!")
                    return {'CANCELLED'}

                mesh.vertex_colors.active = vcol_layer
                self.report({'INFO'}, "Using 'light-color' vertex color layer for baking")

                # Clear to black
                for poly in mesh.polygons:
                    for loop_idx in poly.loop_indices:
                        vcol_layer.data[loop_idx].color = (0, 0, 0, 1)

                # Ensure UVs exist
                if len(mesh.uv_layers) == 0:
                    mesh.uv_layers.new(name="BakeUV")

                # CRITICAL FIX: Temporarily disconnect vertex colors in materials!
                # The Arx materials multiply by vertex colors, creating circular dependency during baking
                # We need to disconnect the vertex color nodes so Cycles can bake fresh lighting

                disconnected_links = []
                for mat in mesh.materials:
                    if mat and mat.use_nodes:
                        # Find the Arx Material node group
                        for node in mat.node_tree.nodes:
                            if node.type == 'GROUP' and node.node_tree:
                                # Check inside the node group for vertex color nodes
                                for group_node in node.node_tree.nodes:
                                    if group_node.type == 'VERTEX_COLOR' and group_node.layer_name == 'light-color':
                                        # Disconnect the Color output
                                        for link in node.node_tree.links:
                                            if link.from_node == group_node and link.from_socket.name == 'Color':
                                                # Store references before removing the link
                                                to_node = link.to_node
                                                to_socket = link.to_socket
                                                from_socket = link.from_socket

                                                # Set the multiply node's Color2 to white instead
                                                if to_node.type == 'MIX_RGB':
                                                    to_socket.default_value = (1, 1, 1, 1)

                                                # Store the link info for restoration
                                                disconnected_links.append((node.node_tree, from_socket, to_socket))

                                                # Now remove the link
                                                node.node_tree.links.remove(link)

                                                self.report({'INFO'}, f"Disconnected vertex color in {mat.name}")
                                                break

                # Configure bake settings
                scene.render.bake.target = 'VERTEX_COLORS'
                scene.render.bake.use_clear = True
                scene.render.bake.margin = 0

                # Use user-defined bake type and settings
                bake_type = props.cycles_bake_type

                # Configure passes based on user settings
                if bake_type == 'DIFFUSE' and hasattr(scene.render.bake, 'use_pass_direct'):
                    scene.render.bake.use_pass_direct = props.cycles_use_direct_light
                    scene.render.bake.use_pass_indirect = props.cycles_use_indirect_light
                    scene.render.bake.use_pass_color = props.cycles_use_color
                elif bake_type == 'AO':
                    # Configure AO settings
                    if hasattr(scene.world.light_settings, 'distance'):
                        scene.world.light_settings.distance = props.cycles_ao_distance

                self.report({'INFO'}, f"Baking {bake_type} with {scene.cycles.samples} samples...")

                # Perform bake
                try:
                    override = context.copy()
                    override['object'] = background_obj
                    override['active_object'] = background_obj
                    with context.temp_override(**override):
                        result = bpy.ops.object.bake(type=bake_type)
                except:
                    result = bpy.ops.object.bake(type=bake_type)

                if result == {'FINISHED'}:
                    # Check if it worked
                    has_color = False
                    for poly in mesh.polygons[:10]:
                        for loop_idx in poly.loop_indices:
                            color = vcol_layer.data[loop_idx].color
                            if color[0] > 0.01 or color[1] > 0.01 or color[2] > 0.01:
                                has_color = True
                                break
                        if has_color:
                            break

                    if has_color:
                        self.report({'INFO'}, "Cycles baking successful!")
                    else:
                        self.report({'WARNING'}, "Cycles bake produced black colors - check lights and materials")
                else:
                    self.report({'ERROR'}, "Cycles bake failed")

                # Restore vertex color connections in materials
                for node_tree, from_socket, to_socket in disconnected_links:
                    node_tree.links.new(from_socket, to_socket)
                    self.report({'INFO'}, "Restored vertex color connection")

            finally:
                # Restore settings
                scene.render.engine = original_engine
                for obj in scene.objects:
                    obj.select_set(False)
                for obj in original_selection:
                    obj.select_set(True)
                context.view_layer.objects.active = original_active

        elif props.lighting_method == 'SIMPLE':
            # Simple lighting calculation
            import bmesh

            mesh = background_obj.data

            # Use 'light-color' layer that the materials expect
            vcol_layer = None
            for vcol in mesh.vertex_colors:
                if vcol.name == "light-color":
                    vcol_layer = vcol
                    break

            if not vcol_layer:
                self.report({'ERROR'}, "No 'light-color' vertex color layer found!")
                return {'CANCELLED'}

            mesh.vertex_colors.active = vcol_layer

            # Simple lighting based on normals
            for poly in mesh.polygons:
                face_normal = background_obj.matrix_world.to_3x3() @ poly.normal
                brightness = max(props.ambient_strength, abs(face_normal.z) * 0.8 + props.ambient_strength)

                for loop_idx in poly.loop_indices:
                    vcol_layer.data[loop_idx].color = (brightness, brightness, brightness, 1.0)

            mesh.update()
            self.report({'INFO'}, "Simple lighting applied")

        # Update viewport
        for area in context.screen.areas:
            if area.type == 'VIEW_3D':
                area.tag_redraw()

        return {'FINISHED'}

class CUSTOM_OT_arx_preview_lighting(Operator):
    bl_idname = "arx.preview_lighting"
    bl_label = "Preview Lighting"
    bl_description = "Preview lighting in the viewport"

    def execute(self, context):
        # Find background mesh
        background_obj = None
        for obj in context.scene.objects:
            if obj.type == 'MESH' and obj.name.endswith('-background'):
                background_obj = obj
                break

        if background_obj:
            # Ensure the mesh has a material that shows vertex colors
            if len(background_obj.data.materials) == 0:
                # Create a simple material to display vertex colors
                import bpy
                mat = bpy.data.materials.new(name="VertexColorPreview")
                mat.use_nodes = True

                # Get the material's node tree
                nodes = mat.node_tree.nodes
                links = mat.node_tree.links

                # Clear default nodes
                nodes.clear()

                # Add vertex color node
                vcol_node = nodes.new('ShaderNodeVertexColor')
                vcol_node.location = (-200, 0)
                vcol_node.layer_name = "light-color"  # Use the standard lighting layer

                # Add principled BSDF
                bsdf = nodes.new('ShaderNodeBsdfPrincipled')
                bsdf.location = (0, 0)

                # Add output node
                output = nodes.new('ShaderNodeOutputMaterial')
                output.location = (200, 0)

                # Connect vertex color to base color
                links.new(vcol_node.outputs['Color'], bsdf.inputs['Base Color'])
                links.new(bsdf.outputs['BSDF'], output.inputs['Surface'])

                # Assign material to mesh
                background_obj.data.materials.append(mat)
            else:
                # Update existing materials to show vertex colors
                for mat in background_obj.data.materials:
                    if mat and mat.use_nodes:
                        nodes = mat.node_tree.nodes
                        links = mat.node_tree.links

                        # Check if vertex color node exists
                        vcol_node = None
                        for node in nodes:
                            if node.type == 'VERTEX_COLOR':
                                vcol_node = node
                                vcol_node.layer_name = "light-color"
                                break

                        if not vcol_node:
                            # Add vertex color node
                            vcol_node = nodes.new('ShaderNodeVertexColor')
                            vcol_node.location = (-400, 0)
                            vcol_node.layer_name = "light-color"

                            # Connect to Principled BSDF
                            bsdf = nodes.get("Principled BSDF")
                            if bsdf:
                                links.new(vcol_node.outputs['Color'], bsdf.inputs['Base Color'])

        # Switch to Solid shading with vertex colors
        for area in context.screen.areas:
            if area.type == 'VIEW_3D':
                for space in area.spaces:
                    if space.type == 'VIEW_3D':
                        space.shading.type = 'SOLID'
                        space.shading.color_type = 'VERTEX'
                        # Also try to show the specific vertex color layer
                        if background_obj and background_obj.data.vertex_colors:
                            for vcol in background_obj.data.vertex_colors:
                                if vcol.name == "light-color":
                                    background_obj.data.vertex_colors.active = vcol
                                    break
                        break

        self.report({'INFO'}, "Switched to vertex color preview mode - showing 'light-color' layer")
        return {'FINISHED'}

def asl_class_path(obj):
    """The class path for an entity object, derived if the scene predates storing it."""
    stored = obj.get("arx_class_path")
    if stored:
        return stored
    name = obj.get("arx_entity_name")
    if name:
        return ASLReader.class_path_from_name(name)
    return None


def asl_text_name(class_path, ident, scope):
    """Text block name that says exactly which file is open, so saving cannot guess."""
    return f"ASL[{scope}] {class_path}#{ident:04d}"


def parse_asl_text_name(name):
    """Recover (class_path, ident, scope) from a text block created by us."""
    import re
    match = re.match(r"^ASL\[(instance|class)\] (.+)#(\d{4})$", name)
    if not match:
        return None, None, None
    return match.group(2), int(match.group(3)), match.group(1)


def open_asl_in_editor(context, class_path, ident, scope, create=False):
    """Load one script into a text block, optionally creating it first."""
    addon = getAddon(context)
    reader = ASLReader(addon.sceneManager.dataPath)
    path, scope, exists = reader.resolve(class_path, ident, scope)

    if not exists:
        if not create:
            return None, path, scope
        path.parent.mkdir(parents=True, exist_ok=True)
        name = class_path.rsplit('/', 1)[-1]
        path.write_text(f"// {name} {scope} script\nON INIT {{\n ACCEPT\n}}\n",
                        encoding='iso-8859-15')

    content = reader.read_path(path)
    if content is None:
        return None, path, scope

    text_name = asl_text_name(class_path, ident, scope)
    text_block = bpy.data.texts.get(text_name)
    if text_block is None:
        text_block = bpy.data.texts.new(text_name)
    text_block.clear()
    text_block.write(content)

    for area in context.screen.areas if context.screen else []:
        if area.type == 'TEXT_EDITOR':
            area.spaces.active.text = text_block
            break

    return text_block, path, scope


class ARX_OT_open_asl(Operator):
    bl_idname = "arx.open_asl"
    bl_label = "Open ASL"
    bl_description = "Open an entity's script in the text editor"

    scope: EnumProperty(
        name="Script",
        items=[('auto', "Whichever Exists", "Instance script if there is one, else the class script"),
               ('instance', "This Entity Only", "The script belonging to this one placed entity"),
               ('class', "Every Entity Of This Type", "The script shared by the whole class")],
        default='auto')

    create: BoolProperty(name="Create If Missing", default=False)

    def execute(self, context):
        obj = context.active_object
        if not obj or not obj.name.startswith('e:'):
            self.report({'ERROR'}, "Select an entity")
            return {'CANCELLED'}

        ident = obj.get("arx_entity_ident")
        class_path = asl_class_path(obj)
        if ident is None or not class_path:
            self.report({'ERROR'}, "This object has no entity identity")
            return {'CANCELLED'}

        text_block, path, scope = open_asl_in_editor(context, class_path, ident,
                                                     self.scope, self.create)
        if text_block is None:
            self.report({'WARNING'}, f"No {scope} script at {path}")
            return {'CANCELLED'}

        self.report({'INFO'}, f"Opened {scope} script: {path}")
        return {'FINISHED'}


class ARX_OT_save_asl(Operator):
    bl_idname = "arx.save_asl"
    bl_label = "Save ASL"
    bl_description = "Write the open script back to the exact file it came from"

    def execute(self, context):
        space = context.space_data
        text_block = getattr(space, 'text', None) if space else None
        if text_block is None:
            self.report({'ERROR'}, "No text block open")
            return {'CANCELLED'}

        class_path, ident, scope = parse_asl_text_name(text_block.name)
        if class_path is None:
            self.report({'ERROR'}, f"'{text_block.name}' was not opened as an Arx script")
            return {'CANCELLED'}

        addon = getAddon(context)
        reader = ASLReader(addon.sceneManager.dataPath)
        # Resolve with the scope recorded in the name, never 'auto': an instance
        # edit must not be able to land on the shared class script.
        path, _scope, _exists = reader.resolve(class_path, ident, scope)

        try:
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(text_block.as_string(), encoding='iso-8859-15')
        except OSError as error:
            self.report({'ERROR'}, f"Could not write {path}: {error}")
            return {'CANCELLED'}

        self.report({'INFO'}, f"Saved {scope} script: {path}")
        return {'FINISHED'}


# Commands whose next word names something else in the level. Kept small and
# explicit rather than guessed at, so a jump either lands somewhere real or is not
# offered at all.
ASL_REFERENCE_COMMANDS = {
    'setcontrolledzone': 'zone',
    'unsetcontrolledzone': 'zone',
    'setpath': 'path',
    'goto': 'label',
    'gosub': 'label',
}

ASL_ENTITY_COMMANDS = {'sendevent', 'cameraactivate', 'settarget', 'spawn',
                       'destroy', 'teleport', 'attach'}

# Keywords that appear where a name would, but do not name anything: setpath none
# clears the path, cameraactivate none hands the camera back, and so on.
ASL_NOT_A_TARGET = {'none', 'self', 'me', 'player', 'off', 'on'}


def scan_asl_references(text):
    """Find the things a script points at, as (kind, target, line number).

    Entity instance ids are recognised by shape - a name followed by four digits -
    because that is how the engine names them, and they can appear as the argument
    of several commands rather than in one fixed position.
    """
    import re

    references = []
    seen = set()
    entity_pattern = re.compile(r'\b([a-z_][a-z0-9_]*)_(\d{4})\b', re.IGNORECASE)

    for number, line in enumerate(text.splitlines(), start=1):
        stripped = line.split('//', 1)[0]
        words = stripped.replace('"', ' ').split()
        if not words:
            continue

        lowered = [w.lower() for w in words]
        for index, word in enumerate(lowered):
            kind = ASL_REFERENCE_COMMANDS.get(word)
            if kind and index + 1 < len(words):
                target = words[index + 1]
                if target.lower() in ASL_NOT_A_TARGET:
                    continue
                key = (kind, target.lower())
                if key not in seen:
                    seen.add(key)
                    references.append((kind, target, number))

        if any(word in ASL_ENTITY_COMMANDS for word in lowered):
            for match in entity_pattern.finditer(stripped):
                target = match.group(0)
                key = ('entity', target.lower())
                if key not in seen:
                    seen.add(key)
                    references.append(('entity', target, number))

    return references


def find_entity_object(id_string):
    """The scene object for an entity id like intro_0001, or None."""
    name, _, digits = id_string.rpartition('_')
    if not digits.isdigit():
        return None
    ident = int(digits)
    for obj in bpy.data.objects:
        if obj.get("arx_entity_ident") != ident:
            continue
        class_path = asl_class_path(obj)
        if class_path and class_path.rsplit('/', 1)[-1].lower() == name.lower():
            return obj
    return None


def find_named_object(prefix, target):
    """A zone or path object by its name, ignoring case."""
    wanted = f"{prefix}:{target}".lower()
    for obj in bpy.data.objects:
        if obj.name.lower() == wanted:
            return obj
    return None


class ARX_OT_asl_jump(Operator):
    bl_idname = "arx.asl_jump"
    bl_label = "Go To Reference"
    bl_description = "Follow this reference to whatever it names"

    kind: StringProperty()
    target: StringProperty()
    line: IntProperty(default=0)

    def execute(self, context):
        if self.kind == 'label':
            text_block = context.space_data.text
            wanted = '>>' + self.target.lower()
            for number, line in enumerate(text_block.lines):
                if line.body.strip().lower().startswith(wanted):
                    text_block.current_line_index = number
                    text_block.select_end_line_index = number
                    self.report({'INFO'}, f"Label {self.target} on line {number + 1}")
                    return {'FINISHED'}
            self.report({'WARNING'}, f"No label {self.target} in this script")
            return {'CANCELLED'}

        if self.kind == 'entity':
            obj = find_entity_object(self.target)
            if obj is None:
                self.report({'WARNING'}, f"No entity {self.target} in the open scenes")
                return {'CANCELLED'}
            class_path = asl_class_path(obj)
            ident = obj.get("arx_entity_ident")
            text_block, path, scope = open_asl_in_editor(context, class_path, ident, 'auto')
            if text_block is None:
                self.report({'WARNING'}, f"{self.target} has no script yet ({path})")
                return {'CANCELLED'}
            self.report({'INFO'}, f"Opened {scope} script of {self.target}")
            return {'FINISHED'}

        obj = find_named_object(self.kind, self.target)
        if obj is None:
            self.report({'WARNING'}, f"No {self.kind} named {self.target} in the scene")
            return {'CANCELLED'}

        for other in context.view_layer.objects:
            other.select_set(False)
        obj.select_set(True)
        context.view_layer.objects.active = obj
        self.report({'INFO'}, f"Selected {self.kind} {obj.name}")
        return {'FINISHED'}


class ARX_PT_asl_references(Panel):
    bl_idname = "TEXT_PT_arx_asl_references"
    bl_label = "Arx Script"
    bl_space_type = 'TEXT_EDITOR'
    bl_region_type = 'UI'
    bl_category = "Arx"

    @classmethod
    def poll(cls, context):
        return context.space_data and context.space_data.text is not None

    def draw(self, context):
        layout = self.layout
        text_block = context.space_data.text

        class_path, ident, scope = parse_asl_text_name(text_block.name)
        if class_path is None:
            layout.label(text="Not an Arx script", icon='INFO')
            return

        box = layout.box()
        box.label(text=f"{class_path}", icon='TEXT')
        box.label(text=f"instance {ident:04d}, {scope} script")
        if scope == 'class':
            box.label(text="Shared by every entity of this type", icon='ERROR')
        layout.operator("arx.save_asl", icon='FILE_TICK')

        references = scan_asl_references(text_block.as_string())
        if not references:
            layout.label(text="No references found")
            return

        icons = {'entity': 'OBJECT_DATA', 'zone': 'MESH_CIRCLE',
                 'path': 'CURVE_PATH', 'label': 'ANCHOR'}
        column = layout.column(align=True)
        column.label(text="References:")
        for kind, target, line in references:
            row = column.row(align=True)
            op = row.operator("arx.asl_jump", text=f"{target}  ({kind}, line {line})",
                              icon=icons.get(kind, 'DOT'))
            op.kind = kind
            op.target = target
            op.line = line


class CUSTOM_OT_arx_open_entity_asl(Operator):
    bl_idname = "arx.open_entity_asl"
    bl_label = "Open Entity ASL"
    bl_description = "Open the ASL file for the selected entity in a text editor"
    
    def execute(self, context):
        obj = context.active_object
        if not obj:
            self.report({'ERROR'}, "No active object selected")
            return {'CANCELLED'}
            
        # Check if object is an entity
        if not obj.name.startswith('e:'):
            self.report({'ERROR'}, "Selected object is not an entity")
            return {'CANCELLED'}
            
        # Get entity identifier
        entity_ident = obj.get("arx_entity_ident")
        if entity_ident is None:
            self.report({'ERROR'}, "Entity identifier not found")
            return {'CANCELLED'}
        
        # Get object identifier for better ASL file resolution
        object_id = obj.get("arx_object_id")
            
        # Get data path from addon
        addon = getAddon(context)
        if not hasattr(addon, 'sceneManager') or not hasattr(addon.sceneManager, 'dataPath'):
            self.report({'ERROR'}, "Arx data path not configured")
            return {'CANCELLED'}
            
        # Read ASL file
        asl_reader = ASLReader(addon.sceneManager.dataPath)
        asl_content = asl_reader.read_asl_file(entity_ident, object_id)
        
        if asl_content is None:
            self.report({'ERROR'}, f"ASL file not found for entity {entity_ident:04d}")
            return {'CANCELLED'}
            
        # Create a new text block in Blender with descriptive name
        text_name = get_asl_text_name(entity_ident, object_id)
        text_block = bpy.data.texts.get(text_name)
        
        if text_block:
            # Update existing text block
            text_block.clear()
            text_block.write(asl_content)
        else:
            # Create new text block
            text_block = bpy.data.texts.new(text_name)
            text_block.write(asl_content)
            
        # Switch to text editor to show the ASL content
        for area in context.screen.areas:
            if area.type == 'TEXT_EDITOR':
                area.spaces.active.text = text_block
                break
        else:
            # If no text editor found, show info message
            asl_path = asl_reader.get_asl_file_path(entity_ident, object_id)
            self.report({'INFO'}, f"ASL file loaded in text block '{text_name}' from {asl_path}")
            
        return {'FINISHED'}

class CUSTOM_OT_arx_show_asl_info(Operator):
    bl_idname = "arx.show_asl_info"
    bl_label = "Show ASL Info"
    bl_description = "Show information about the ASL file for the selected entity"
    
    def execute(self, context):
        obj = context.active_object
        if not obj:
            self.report({'ERROR'}, "No active object selected")
            return {'CANCELLED'}
            
        # Check if object is an entity
        if not obj.name.startswith('e:'):
            self.report({'ERROR'}, "Selected object is not an entity")
            return {'CANCELLED'}
            
        # Get entity identifier
        entity_ident = obj.get("arx_entity_ident")
        if entity_ident is None:
            self.report({'ERROR'}, "Entity identifier not found")
            return {'CANCELLED'}
            
        # Get data path from addon
        addon = getAddon(context)
        if not hasattr(addon, 'sceneManager') or not hasattr(addon.sceneManager, 'dataPath'):
            self.report({'ERROR'}, "Arx data path not configured")
            return {'CANCELLED'}
            
        # Get ASL file info
        asl_reader = ASLReader(addon.sceneManager.dataPath)
        asl_info = asl_reader.get_asl_file_info(entity_ident)
        
        if asl_info is None:
            self.report({'ERROR'}, f"ASL file not found for entity {entity_ident:04d}")
            return {'CANCELLED'}
            
        # Show info in a popup
        def draw(self, context):
            layout = self.layout
            layout.label(text=f"ASL File Info for Entity {entity_ident:04d}")
            layout.separator()
            layout.label(text=f"Path: {asl_info['path']}")
            layout.label(text=f"Size: {asl_info['size']} bytes")
            layout.label(text=f"Modified: {asl_info['modified']}")
            
        bpy.context.window_manager.popup_menu(draw, title="ASL File Information", icon='INFO')
        return {'FINISHED'}

class CUSTOM_OT_arx_save_entity_asl(Operator):
    bl_idname = "arx.save_entity_asl"
    bl_label = "Save Entity ASL"
    bl_description = "Save the current ASL text block back to the file"
    
    def execute(self, context):
        # Try to get current text block from text editor context first
        text_block = None
        if context.space_data and hasattr(context.space_data, 'text'):
            text_block = context.space_data.text
        
        # If we have a text block, parse entity info from its name
        if text_block and text_block.name.startswith('ASL_'):
            entity_ident, object_id = parse_asl_text_name(text_block.name)
            if entity_ident is None:
                self.report({'ERROR'}, f"Cannot parse entity info from text name: {text_block.name}")
                return {'CANCELLED'}
        else:
            # Fallback to selected object method for backward compatibility
            obj = context.active_object
            if not obj or not obj.name.startswith('e:'):
                self.report({'ERROR'}, "No ASL text open and no entity selected")
                return {'CANCELLED'}
                
            entity_ident = obj.get("arx_entity_ident")
            if entity_ident is None:
                self.report({'ERROR'}, "Entity identifier not found")
                return {'CANCELLED'}
            
            object_id = obj.get("arx_object_id")
                
            # Get the text block
            text_name = get_asl_text_name(entity_ident, object_id)
            text_block = bpy.data.texts.get(text_name)
            if not text_block:
                self.report({'ERROR'}, f"No ASL text block found: {text_name}")
                return {'CANCELLED'}
            
        # Get data path from addon
        addon = getAddon(context)
        if not hasattr(addon, 'sceneManager') or not hasattr(addon.sceneManager, 'dataPath'):
            self.report({'ERROR'}, "Arx data path not configured")
            return {'CANCELLED'}
            
        # Find the ASL file path using the same method as the reader
        asl_reader = ASLReader(addon.sceneManager.dataPath)
        asl_path = asl_reader.get_asl_file_path(entity_ident, object_id)
        
        if not asl_path:
            self.report({'ERROR'}, f"ASL file not found for entity {entity_ident:04d}")
            return {'CANCELLED'}
            
        try:
            # Get text content from Blender text block
            content = text_block.as_string()
            
            # Write to file with ISO-8859-15 encoding
            with open(asl_path, 'w', encoding='iso-8859-15') as f:
                f.write(content)
                
            self.report({'INFO'}, f"ASL file saved: {asl_path}")
            return {'FINISHED'}
            
        except Exception as e:
            self.report({'ERROR'}, f"Error saving ASL file: {e}")
            return {'CANCELLED'}

class CUSTOM_OT_arx_reload_entity_asl(Operator):
    bl_idname = "arx.reload_entity_asl"
    bl_label = "Reload Entity ASL"
    bl_description = "Reload the ASL file from disk, discarding any changes in the text editor"
    
    def execute(self, context):
        obj = context.active_object
        if not obj or not obj.name.startswith('e:'):
            self.report({'ERROR'}, "No entity selected")
            return {'CANCELLED'}
            
        entity_ident = obj.get("arx_entity_ident")
        if entity_ident is None:
            self.report({'ERROR'}, "Entity identifier not found")
            return {'CANCELLED'}
        
        object_id = obj.get("arx_object_id")
            
        # Get data path from addon
        addon = getAddon(context)
        if not hasattr(addon, 'sceneManager') or not hasattr(addon.sceneManager, 'dataPath'):
            self.report({'ERROR'}, "Arx data path not configured")
            return {'CANCELLED'}
            
        # Read ASL file
        asl_reader = ASLReader(addon.sceneManager.dataPath)
        asl_content = asl_reader.read_asl_file(entity_ident, object_id)
        
        if asl_content is None:
            self.report({'ERROR'}, f"ASL file not found for entity {entity_ident:04d}")
            return {'CANCELLED'}
            
        # Update the text block
        text_name = get_asl_text_name(entity_ident, object_id)
        text_block = bpy.data.texts.get(text_name)
        
        if text_block:
            text_block.clear()
            text_block.write(asl_content)
            self.report({'INFO'}, f"ASL file reloaded: {text_name}")
        else:
            # Create new text block if it doesn't exist
            text_block = bpy.data.texts.new(text_name)
            text_block.write(asl_content)
            self.report({'INFO'}, f"ASL file loaded in new text block: {text_name}")
            
        return {'FINISHED'}

def draw_asl_header_buttons(self, context):
    """Draw function to append to text editor header"""
    if context.space_data and hasattr(context.space_data, 'text'):
        text = context.space_data.text
        
        if text and text.name.startswith('ASL_'):
            layout = self.layout
            
            # Extract entity info from text name
            try:
                name_parts = text.name.split('_')
                if len(name_parts) >= 2:
                    entity_info = '_'.join(name_parts[1:])
                    
                    row = layout.row()
                    row.separator()
                    row.operator("arx.save_entity_asl", text="Save ASL", icon='FILE_TICK')
                    row.operator("arx.reload_entity_asl", text="Reload", icon='FILE_REFRESH')
                    row.separator()
                    row.operator("arx.enable_asl_navigation", text="Navigation", icon='OUTLINER_OB_LIGHTPROBE')
                    row.operator("arx.format_asl", text="Analyze", icon='VIEWZOOM')
                    row.separator()
                    row.label(text=f"ASL: {entity_info}")
                    
            except (ValueError, IndexError):
                pass

class CUSTOM_OT_arx_enable_asl_navigation(Operator):
    bl_idname = "arx.enable_asl_navigation"
    bl_label = "Enable ASL Navigation"
    bl_description = "Enable ctrl+click navigation and syntax highlighting for ASL files"
    
    def execute(self, context):
        # Store the navigator in global variable
        global g_asl_navigator
        if g_asl_navigator is None:
            addon = getAddon(context)
            g_asl_navigator = ASLNavigator(addon)
        
        # Enable the modal operator for handling clicks
        bpy.ops.arx.asl_navigation_modal('INVOKE_DEFAULT')
        
        self.report({'INFO'}, "ASL navigation enabled - use Ctrl+Click to follow references")
        return {'FINISHED'}

class CUSTOM_OT_arx_asl_navigation_modal(Operator):
    bl_idname = "arx.asl_navigation_modal"
    bl_label = "ASL Navigation Modal"
    bl_description = "Modal operator for handling ASL navigation"
    
    def modal(self, context, event):
        # Only handle events in text editor
        if context.area and context.area.type == 'TEXT_EDITOR':
            space = context.space_data
            text = space.text
            
            # Only handle ASL files
            if text and text.name.startswith('ASL_'):
                if event.type == 'LEFTMOUSE' and event.value == 'PRESS' and event.ctrl:
                    # Get cursor position
                    cursor_line = text.current_line_index
                    cursor_char = text.current_character
                    
                    # Get or create the navigator
                    navigator = getattr(context.window_manager, 'arx_asl_navigator', None)
                    if not navigator:
                        addon = getAddon(context)
                        navigator = ASLNavigator(addon)
                        context.window_manager.arx_asl_navigator = navigator
                    
                    # Parse the current line for references
                    line_text = text.lines[cursor_line].body
                    references = navigator.syntax_highlighter.find_references('\n'.join([line.body for line in text.lines]))
                    
                    # Find reference at cursor position
                    for ref in references:
                        if ref['line'] == cursor_line and ref['start'] <= cursor_char <= ref['end']:
                            if navigator.navigate_to_reference(context, ref):
                                return {'RUNNING_MODAL'}
                            else:
                                self.report({'WARNING'}, f"Could not navigate to {ref['type']}: {ref['name']}")
                            break
        
        # Continue running
        if event.type == 'ESC':
            return {'CANCELLED'}
        
        return {'PASS_THROUGH'}
    
    def invoke(self, context, event):
        context.window_manager.modal_handler_add(self)
        return {'RUNNING_MODAL'}

class CUSTOM_OT_arx_format_asl(Operator):
    bl_idname = "arx.format_asl"
    bl_label = "Format ASL Code"
    bl_description = "Format and analyze ASL code for syntax errors"
    
    def execute(self, context):
        if context.area.type != 'TEXT_EDITOR':
            self.report({'ERROR'}, "Must be in text editor")
            return {'CANCELLED'}
            
        text = context.space_data.text
        if not text or not text.name.startswith('ASL_'):
            self.report({'ERROR'}, "Not an ASL file")
            return {'CANCELLED'}
            
        # Get or create the navigator for syntax analysis
        global g_asl_navigator
        if g_asl_navigator is None:
            addon = getAddon(context)
            g_asl_navigator = ASLNavigator(addon)
        navigator = g_asl_navigator
        
        # Find all references in the text
        content = text.as_string()
        references = navigator.syntax_highlighter.find_references(content)
        
        # Show analysis results
        def draw(self, context):
            layout = self.layout
            layout.label(text=f"ASL Analysis Results:")
            layout.separator()
            layout.label(text=f"Found {len(references)} references:")
            
            for ref in references[:10]:  # Show first 10 references
                row = layout.row()
                row.label(text=f"Line {ref['line'] + 1}: {ref['type']} '{ref['name']}'")
                
            if len(references) > 10:
                layout.label(text=f"... and {len(references) - 10} more")
        
        bpy.context.window_manager.popup_menu(draw, title="ASL Code Analysis", icon='INFO')
        return {'FINISHED'}

class ArxEntityPanel(Panel):
    bl_idname = "SCENE_PT_arx_entity"
    bl_label = "Arx Entity ASL"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context = "object"
    
    @classmethod
    def poll(cls, context):
        obj = context.active_object
        return obj and obj.name.startswith('e:') and obj.get("arx_entity_ident") is not None
    
    def draw(self, context):
        layout = self.layout
        obj = context.active_object
        
        entity_ident = obj.get("arx_entity_ident")
        entity_name = obj.get("arx_entity_name", "Unknown")
        class_path = asl_class_path(obj)

        layout.label(text=f"Entity: {entity_name}")
        layout.label(text=f"ID: {entity_ident:04d}")
        if class_path:
            layout.label(text=class_path, icon='FILE_FOLDER')
        else:
            layout.label(text="No class path; reimport the level", icon='ERROR')
            return

        addon = getAddon(context)
        reader = ASLReader(addon.sceneManager.dataPath)
        _ip, _s1, has_instance = reader.resolve(class_path, entity_ident, 'instance')
        _cp, _s2, has_class = reader.resolve(class_path, entity_ident, 'class')

        # Offered separately on purpose. The class script is shared by every entity
        # of this type, so editing it when you meant to edit this one entity is a
        # change to all of them, and that has to be a deliberate choice.
        column = layout.column(align=True)
        row = column.row(align=True)
        op = row.operator("arx.open_asl", icon='TEXT',
                          text="This Entity" + ("" if has_instance else " (create)"))
        op.scope = 'instance'
        op.create = not has_instance

        row = column.row(align=True)
        row.enabled = has_class
        op = row.operator("arx.open_asl", icon='COPY_ID',
                          text="Whole Class" if has_class else "Whole Class (none)")
        op.scope = 'class'
        op.create = False

        box = layout.box()
        box.label(text=f"instance script: {'yes' if has_instance else 'no'}")
        box.label(text=f"class script: {'yes' if has_class else 'no'}")
        box.label(text="Save from the Arx tab in the text editor.", icon='INFO')

classes = (
    CUSTOM_OT_arx_area_list_reload,
    ARX_area_properties,
    ARX_lighting_properties,
    SCENE_UL_arx_area_list,
    CUSTOM_OT_arx_area_list_import_selected,
    CUSTOM_OT_arx_area_list_export_all,
    CUSTOM_OT_arx_area_export_fts,
    CUSTOM_OT_arx_area_export_llf,
    CUSTOM_OT_arx_area_export_dlf,
    CUSTOM_OT_arx_view_face_attributes,
    ArxOperatorImportAllLevels,
    ArxAnimationTestProperties,
    ArxModelListProperties,
    ArxModelListItem,
    ArxOperatorRefreshModelList,
    ArxOperatorTestGoblinAnimations,
    ArxAnimationTestPanel,
    ArxSelectModelOperator,
    ArxSetModelOperator,
    ArxSelectAnimationOperator,
    ArxSetAnimationOperator,
    ArxSelectAnimationLayerOperator,
    ArxSetAnimationLayerOperator,
    ArxClearAnimationLayerOperator,
    ArxTestLayeredAnimationsOperator,
    ArxLightingPanel,
    ARX_OT_generate_anchors,
    ARX_OT_portal_init,
    ARX_PT_portal_properties,
    ARX_zone_properties,
    ARX_PT_zone_properties,
    ARX_face_tool_properties,
    ARX_OT_assign_face_room,
    ARX_OT_select_faces_by_room,
    ARX_OT_report_room_usage,
    ARX_PT_face_attributes,
    ARX_OT_open_asl,
    ARX_OT_save_asl,
    ARX_OT_asl_jump,
    ARX_PT_asl_references,
    ARX_OT_list_animation_sets,
    CUSTOM_OT_arx_regenerate_lighting,
    CUSTOM_OT_arx_preview_lighting,
    CUSTOM_OT_arx_open_entity_asl,
    CUSTOM_OT_arx_show_asl_info,
    CUSTOM_OT_arx_save_entity_asl,
    CUSTOM_OT_arx_reload_entity_asl,
    CUSTOM_OT_arx_enable_asl_navigation,
    CUSTOM_OT_arx_asl_navigation_modal,
    CUSTOM_OT_arx_format_asl,
    ArxEntityPanel,
)

def arx_ui_area_register():
    from bpy.utils import register_class
    for cls in classes:
        register_class(cls)
    bpy.types.WindowManager.arx_areas_col = CollectionProperty(type=ARX_area_properties)
    bpy.types.WindowManager.arx_areas_idx = IntProperty()
    bpy.types.Scene.arx_animation_test = PointerProperty(type=ArxAnimationTestProperties)
    bpy.types.Scene.arx_model_list_props = PointerProperty(type=ArxModelListProperties)
    bpy.types.Scene.arx_lighting = PointerProperty(type=ARX_lighting_properties)
    bpy.types.Scene.arx_face_tools = PointerProperty(type=ARX_face_tool_properties)
    bpy.types.Object.arx_zone = PointerProperty(type=ARX_zone_properties)
    
    # Register text editor header extension
    bpy.types.TEXT_HT_header.append(draw_asl_header_buttons)

def arx_ui_area_unregister():
    from bpy.utils import unregister_class
    
    # Clean up global ASL navigator
    global g_asl_navigator
    g_asl_navigator = None
    
    # Unregister text editor header extension
    bpy.types.TEXT_HT_header.remove(draw_asl_header_buttons)
    
    for cls in reversed(classes):
        unregister_class(cls)
    del bpy.types.WindowManager.arx_areas_col
    del bpy.types.WindowManager.arx_areas_idx
    del bpy.types.Scene.arx_animation_test
    del bpy.types.Scene.arx_model_list_props
    del bpy.types.Object.arx_zone
    del bpy.types.Scene.arx_face_tools
    del bpy.types.Scene.arx_lighting
