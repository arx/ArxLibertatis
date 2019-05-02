# Copyright 2015-2017 Arx Libertatis Team (see the AUTHORS file)
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

import importlib
import sys
import os
import logging
import itertools

if "bpy" in locals():
    importlib.reload(sys.modules["arx_addon.arx_io_animation"])
    importlib.reload(sys.modules["arx_addon.arx_io_area"])
    importlib.reload(sys.modules["arx_addon.arx_io_material"])
    importlib.reload(sys.modules["arx_addon.arx_io_model"])
    importlib.reload(sys.modules["arx_addon.arx_io_test_roundtrip"])
    importlib.reload(sys.modules["arx_addon.arx_io_util"])
    importlib.reload(sys.modules["arx_addon.dataCommon"])
    importlib.reload(sys.modules["arx_addon.dataDlf"])
    importlib.reload(sys.modules["arx_addon.dataFtl"])
    importlib.reload(sys.modules["arx_addon.dataFts"])
    importlib.reload(sys.modules["arx_addon.dataLlf"])
    importlib.reload(sys.modules["arx_addon.dataTea"])
    importlib.reload(sys.modules["arx_addon.files"])
    importlib.reload(sys.modules["arx_addon.lib"])

import bpy
import bmesh

from .lib import ArxIO
from .files import *

from .arx_io_animation import ArxAnimationManager
from .arx_io_area import ArxSceneManager
from .arx_io_model import ArxObjectManager
from .arx_io_test_roundtrip import test_export_of_current_scene_objects

log = logging.getLogger('ArxAddon')

def triangulate(bm):
    while True:
        nonTris = [f for f in bm.faces if len(f.verts) > 3]
        if nonTris:
            nt = nonTris[0]
            pivotLoop = nt.loops[0]
            allVerts = nt.verts
            vert1 = pivotLoop.vert
            wrongVerts = [vert1, pivotLoop.link_loop_next.vert, pivotLoop.link_loop_prev.vert]
            bmesh.utils.face_split(nt, vert1, [v for v in allVerts if v not in wrongVerts][0])

            for seq in [bm.verts, bm.faces, bm.edges]:
                seq.index_update()
        else:
            break
    return bm


def strip_wires(bm):
    [bm.verts.remove(v) for v in bm.verts if v.is_wire]
    [bm.verts.remove(v) for v in bm.verts if not v.link_faces[:]]
    [bm.edges.remove(e) for e in bm.edges if not e.link_faces[:]]
    [bm.faces.remove(f) for f in bm.faces if len(f.edges) < 3]
    for seq in [bm.verts, bm.faces, bm.edges]: seq.index_update()
    return bm



class ArxAssetManager(object):
    def __init__(self, arxFiles, objectManager, sceneManager):
        self.log = logging.getLogger('ArxAssetManager')
        self.log.setLevel(logging.DEBUG)
        
        self.arxFiles = arxFiles
        self.objectManager = objectManager
        self.sceneManager = sceneManager

    def importAllModels(self, context):
        sortedModels = sorted(self.arxFiles.models.data.items())
        
        for i, (key, val) in enumerate(sortedModels):
            """
            sceneName = "m:" + '/'.join(key)
            
            scene = bpy.data.scenes.get(sceneName)
            
            if scene is None:
                self.log.info("Creating new scene [{}]".format(sceneName))
                scene = bpy.data.scenes.new(name=sceneName)
                scene.unit_settings.system = 'METRIC'
                scene.unit_settings.scale_length = 0.01
            else:
                self.log.info("Scene [{}] already exists".format(sceneName))
                continue
            """
            scene = None

            import_file = os.path.join(val.path, val.model)
            self.objectManager.loadFile(context, import_file, scene);


class ArxAddon(object):
    def __init__(self, dataPath, allowLibFallback):
        self.log = logging.getLogger('Arx Addon')
        
        try:
            ioLib = ArxIO()
        except Exception as e:
            if not allowLibFallback:
                raise e
            else:
                self.log.error("Failed to load native io library, using slow fallback. Exception: " + str(e));
                
                from .naivePkware import decompress_ftl
                class ArxIOFallback(object):
                    def __init__(self):
                        pass
                    def unpack(self, data):
                        return decompress_ftl(data)
                        
                ioLib = ArxIOFallback()
        
        self.arxFiles = ArxFiles(dataPath)
        self.arxFiles.updateAll()
        
        self.objectManager = ArxObjectManager(ioLib, dataPath)
        self.sceneManager = ArxSceneManager(ioLib, dataPath, self.arxFiles, self.objectManager)
        self.animationManager = ArxAnimationManager()
        self.assetManager = ArxAssetManager(self.arxFiles, self.objectManager, self.sceneManager)
    
    def testModelExport(self):
        
        test_export_of_current_scene_objects(self.assetManager)

# ======================================================================================================================

g_arxAddon = None

def arxAddonReload():
    log.info("invalidating")
    global g_arxAddon
    g_arxAddon = None

def getAddon(context):
    global g_arxAddon
    if not g_arxAddon:
        log.info("creating")
        addon_prefs = context.preferences.addons[__package__].preferences
        g_arxAddon = ArxAddon(addon_prefs.arxAssetPath, addon_prefs.arxAllowLibFallback)
    
    return g_arxAddon
