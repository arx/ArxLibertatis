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

def getBlackTexture():
    if "solid_black" in bpy.data.textures:
        return bpy.data.textures["solid_black"]
    else:
        img = bpy.data.images.new("solid_black", 32, 32)
        img.source = 'GENERATED'
        img.generated_type = "BLANK"
        img.generated_color = [0, 0, 0, 1]

        tex = bpy.data.textures.new("solid_black", type='IMAGE')
        tex.image = img

        return tex


def createMaterial(rootDirectory, textureName) -> bpy.types.Material:
    relativePath, fileExtension = os.path.splitext(textureName.replace("\\", "/").lower())
    foo, fileName = os.path.split(relativePath)
    
    matName = fileName + "-mat"
    
    mats = bpy.data.materials
    matches = [m for m in mats if m.name == matName]
    if matches:
        return matches[0]
    
    mat = bpy.data.materials.new(matName)
    mat.use_shadeless = True
    
    extensions = [".png", ".jpg", ".jpeg", ".bmp", ".tga"]
    for ext in extensions:
        fullPath = os.path.join(rootDirectory, relativePath) + ext
        if os.path.exists(fullPath):
            break
    
    if not os.path.exists(fullPath):
        log.warning("Texture not found: %s" % textureName)
        return mat
    
    if relativePath in bpy.data.images:
        img = bpy.data.images[relativePath]
    else:
        img = bpy.data.images.load(fullPath)
        img.name = relativePath
    
    
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
