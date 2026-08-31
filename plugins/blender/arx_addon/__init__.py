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

bl_info = {
    "name": "Arx Libertatis Tools",
    "author": "Arx Libertatis Team",
    "version": (1, 0, 0),
    "blender": (4, 4, 0),
    "location": "File > Import-Export, Properties > Scene",
    "description": "Import and export Arx Fatalis models and animations",
    "category": "Import-Export",
}

import bpy
from . import main

def register():
    main.register()

def unregister():
    main.unregister()

if __name__ == "__main__":
    register()
