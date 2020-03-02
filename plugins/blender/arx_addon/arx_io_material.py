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
import logging

import bpy
from mathutils import Vector

log = logging.getLogger('Materials')

def arx_create_image(rootDirectory, relativePath):
    extensions = [".png", ".jpg", ".jpeg", ".bmp", ".tga"]
    for ext in extensions:
        fullPath = os.path.join(rootDirectory, relativePath) + ext
        if os.path.exists(fullPath):
            break

    if not os.path.exists(fullPath):
        log.warning("Texture not found: %s" % relativePath)
        return None

    if relativePath in bpy.data.images:
        img = bpy.data.images[relativePath]
    else:
        img = bpy.data.images.load(fullPath)
        img.name = relativePath

    return img

def arx_create_material_nodes(material, image):
    tree = material.node_tree

    n_output = None
    n_principled = None
    for node in tree.nodes:
        if node.bl_idname == 'ShaderNodeOutputMaterial':
            n_output = node
            continue
        if node.bl_idname == 'ShaderNodeBsdfPrincipled':
            n_principled = node
            continue

    # Remove default principled bsdf node
    tree.nodes.remove(n_principled)

    n_output.location = (0, 0)

    n_arx = tree.nodes.new('ShaderNodeGroup')
    n_arx.name = 'n_arx'
    n_arx.location = (-200, 0)
    n_arx.node_tree = arx_get_material_node_group()

    n_tex_image = tree.nodes.new('ShaderNodeTexImage')
    n_tex_image.name = 'n_tex_image'
    n_tex_image.location = (-600, 0)

    tree.links.new(n_arx.outputs['Shader'], n_output.inputs['Surface'])
    tree.links.new(n_tex_image.outputs['Color'], n_arx.inputs['Color'])

    n_tex_image.image = image

def createMaterial(rootDirectory, textureName) -> bpy.types.Material:

    relativePath, fileExtension = os.path.splitext(textureName.replace("\\", "/").lower())
    foo, fileName = os.path.split(relativePath)
    
    matName = fileName + "-mat"
    
    mats = bpy.data.materials
    matches = [m for m in mats if m.name == matName]
    if matches:
        return matches[0]

    image = arx_create_image(rootDirectory, relativePath)

    mat = bpy.data.materials.new(matName)
    mat.use_nodes = True
    mat.blend_method = 'HASHED'
    arx_create_material_nodes(mat, image)

    return mat

    if relativePath in bpy.data.textures:
        tex = bpy.data.textures[relativePath]
    else:
        tex = bpy.data.textures.new(relativePath, type='IMAGE')
        tex.image = img
    
    mtex = mat.texture_slots.add()
    mtex.texture = tex
    mtex.texture_coords = 'UV'
    mtex.use_map_color_diffuse = True

    # This is a hack for testing !
    if "[stone]_fire_torch01" in relativePath:
        print("=================_fire_torch01")
        mat.use_transparency = True
        mat.transparency_method = "Z_TRANSPARENCY"
        mat.offset_z = 1
        img.use_alpha = False

        mtex.use_map_alpha = True
        mtex.alpha_factor = -1
        mtex.blend_type = 'SUBTRACT'

        black = getBlackTexture()
        slot = mat.texture_slots.add()
        slot.texture = black

    return mat

#from arx_addon.materials import arx_create_material_node_group
#arx_create_material_node_group()
#am = D.node_groups['Arx Material']


def arx_get_material_node_group():
    arx_material_node_group_name = 'Arx Material'

    if arx_material_node_group_name in bpy.data.node_groups:
        return bpy.data.node_groups[arx_material_node_group_name]

    group = bpy.data.node_groups.new(arx_material_node_group_name, 'ShaderNodeTree')

    group.outputs.new('NodeSocketShader', 'Shader')
    group.inputs.new('NodeSocketColor', 'Color')

    n_out = group.nodes.new('NodeGroupOutput')
    n_out.name = 'n_out'
    n_out.location = Vector((0.0, 0.0))

    n_mix = group.nodes.new('ShaderNodeMixShader')
    n_mix.name = 'n_mix'
    n_mix.location = Vector((-220.0, 0.0))

    n_transparent = group.nodes.new('ShaderNodeBsdfTransparent')
    n_transparent.name = 'n_transparent'
    n_transparent.location = Vector((-520.0, -140.0))

    n_diffuse = group.nodes.new('ShaderNodeBsdfDiffuse')
    n_diffuse.name = 'n_diffuse'
    n_diffuse.location = Vector((-520.0, 0.0))

    n_math1 = group.nodes.new('ShaderNodeMath')
    n_math1.name = 'n_math1'
    n_math1.location = Vector((-520.0, 180.0))
    n_math1.operation = 'FLOOR'

    n_geometry = group.nodes.new('ShaderNodeNewGeometry')
    n_geometry.name = 'n_geometry'
    n_geometry.location = Vector((-760.0, 420.0))

    n_colMix = group.nodes.new('ShaderNodeMixRGB')
    n_colMix.name = 'n_colMix'
    n_colMix.blend_type = 'MULTIPLY'
    n_colMix.inputs['Fac'].default_value = 1.0
    n_colMix.location = Vector((-730.0, -238.0))

    n_vertexColor = group.nodes.new('ShaderNodeVertexColor')
    n_vertexColor.name = 'n_vertexColor'
    n_vertexColor.layer_name = 'light-color'
    n_vertexColor.location = Vector((-970.0, -91.0))

    n_in = group.nodes.new('NodeGroupInput')
    n_in.name = 'n_in'
    n_in.location = Vector((-1300.0, 0.0))

    group.links.new(n_mix.outputs['Shader'], n_out.inputs['Shader'])

    group.links.new(n_math1.outputs['Value'], n_mix.inputs['Fac'])
    group.links.new(n_diffuse.outputs['BSDF'], n_mix.inputs[1])
    group.links.new(n_transparent.outputs['BSDF'], n_mix.inputs[2])

    group.links.new(n_geometry.outputs['Backfacing'], n_math1.inputs['Value'])

    group.links.new(n_colMix.outputs['Color'], n_diffuse.inputs['Color'])
    group.links.new(n_in.outputs['Color'], n_colMix.inputs['Color1'])
    group.links.new(n_vertexColor.outputs['Color'], n_colMix.inputs['Color2'])

    return group
