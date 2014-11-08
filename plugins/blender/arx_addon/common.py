# Copyright 2014 Arx Libertatis Team (see the AUTHORS file)
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

import bpy

def createMaterial(rootDirectory, textureName):
    relativePath, fileExtension = os.path.splitext(textureName.replace("\\", "/").lower())
    foo, fileName = os.path.split(relativePath)
    
    mat = bpy.data.materials.new(fileName + "-mat")
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
        tex = bpy.data.textures.new(relativePath, type = 'IMAGE')
        tex.image = img
    
    mtex = mat.texture_slots.add()
    mtex.texture = tex
    mtex.texture_coords = 'UV'
    mtex.use_map_color_diffuse = True
    return mat