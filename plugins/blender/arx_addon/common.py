# Copyright 2014-2017 Arx Libertatis Team (see the AUTHORS file)
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
import logging

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

    n_output = [node for node in tree.nodes if node.type == 'OUTPUT_MATERIAL'][0]
    n_output.location = (0, 0)

    n_diffuse = tree.nodes.new('ShaderNodeBsdfDiffuse')
    n_diffuse.name = 'n_diffuse'
    n_diffuse.location = (-200, 0)
    tree.links.new(n_diffuse.outputs['BSDF'], n_output.inputs['Surface'])

    n_tex_image = tree.nodes.new('ShaderNodeTexImage')
    n_tex_image.name = 'n_tex_image'
    n_tex_image.location = (-600, 0)
    tree.links.new(n_tex_image.outputs['Color'], n_diffuse.inputs['Color'])

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
