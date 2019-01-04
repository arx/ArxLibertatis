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

bl_info = {
    "name": "Arx Libertatis Addon",
    "author": "Arx Libertatis Team",
    "version": (0, 0, 1),
    "blender": (2, 74, 0),
    "location": "",
    "description": "Addon for managing Arx Libertatis assets",
    "warning": "This is alpha software, read the documentation, keep your original blend files!",
    "wiki_url": "http://wiki.arx-libertatis.org/Blender",
    "tracker_url": "https://bugs.arx-libertatis.org/arx/issues/new",
    "category": "Import-Export",
    "support": "COMMUNITY",
}

import imp

try:
    imp.find_module('bpy')
    blenderFound = True
except ImportError:
    blenderFound = False

print("Running inside blender: " + str(blenderFound))

if blenderFound:
    import importlib
    if "main" in locals():
        importlib.reload(main)

    from .main import register

    def register():
        main.register();

    def unregister():
        main.unregister();


