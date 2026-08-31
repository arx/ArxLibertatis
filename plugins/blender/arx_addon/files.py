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

import os
import io
import re
import platform
from collections import namedtuple, OrderedDict


def normalize_arx_path(path):
    """Normalize path for cross-platform compatibility"""
    # Convert to lowercase
    path = path.lower()
    # Replace backslashes with forward slashes
    path = path.replace('\\', '/')
    # Remove duplicate slashes
    while '//' in path:
        path = path.replace('//', '/')
    return path

def find_arx_directory(base_path, target_dir):
    """Find directory with case-insensitive matching on Windows"""
    if not os.path.exists(base_path):
        return None

    if platform.system() == 'Windows':
        # Case-insensitive search on Windows
        for item in os.listdir(base_path):
            if item.lower() == target_dir.lower():
                return os.path.join(base_path, item)
    else:
        # Case-sensitive on Linux
        target_path = os.path.join(base_path, target_dir)
        if os.path.exists(target_path):
            return target_path
    return None

def splitPath(path):
    result = []
    while 1:
        path, folder = os.path.split(path)
        if folder != "":
            result.append(folder)
        else:
            if path != "":
                result.append(path)
            break
    result.reverse()
    return result;


EntityInstance = namedtuple("ArxInstance", ["id", "script"])
EntityData = namedtuple("EntityData", ["path", "script", "icon", "instances"])


class Entities:
    def __init__(self):
        # Use os.path.join for cross-platform compatibility
        # Check various case combinations
        self.paths = [
            os.path.join("graph", "obj3d", "interactive"),
            os.path.join("GRAPH", "OBJ3D", "INTERACTIVE"),
            os.path.join("Graph", "Obj3d", "Interactive"),
            os.path.join("GRAPH", "obj3d", "interactive"),
            os.path.join("game", "graph", "obj3d", "interactive")  # Nested variant
        ]
        self.danglingPaths = []
        self.data = {}

    def update(self, absroot):

        for root, dirs, files in os.walk(absroot):
            # Calculate relative path from absroot
            relpath = os.path.relpath(root, absroot)
            split = splitPath(relpath)

            mainScript = None

            for f in files:
                name, ext = os.path.splitext(f)
                if ext.lower() == ".asl" and name.lower() == split[-1].lower():
                    mainScript = f

            if mainScript is not None:
                files.remove(mainScript)

                icon = None

                for f in files:
                    fullPath = os.path.join(root, f)
                    name, ext = os.path.splitext(f)
                    if name.endswith("[icon]"):
                        icon = fullPath
                    else:
                        self.danglingPaths.append(fullPath)

                instances = []

                for d in dirs:
                    if d.startswith(split[-1]):
                        instanceId = int(d[-4:])

                        instanceDir = os.path.join(root, d)
                        instanceFiles = os.listdir(instanceDir)

                        instanceScript = None

                        for f in instanceFiles:
                            if os.path.isfile(os.path.join(instanceDir, f)):
                                if f == mainScript:
                                    instanceScript = os.path.join(root, d, f)
                                else:
                                    self.danglingPaths.append(os.path.join(root, d, f))
                            else:
                                self.danglingPaths.append(os.path.join(root, d, f))

                        instances.append(EntityInstance(instanceId, instanceScript))
                    else:
                        self.danglingPaths.append(os.path.join(root, d))

                instances.sort(key=lambda x: x.id)

                key = tuple(split)
                if key in self.data:
                    print("Warn: Duplicated name !!")

                e = EntityData(root, mainScript, icon, instances)
                self.data[key] = e


    def parseResourceReferences(self):
        for key in self.data:
            value = self.data[key]

            scriptPath = os.path.join(value.path, value.script)

            with io.open(scriptPath, 'r', encoding='latin1') as f:
                lines = f.readlines()
                for line in lines:
                    if "loadanim" in line.lower():
                        print(line)


ModelData = namedtuple("ModelData", ["path", "model", "tweaks"])

class Models:
    def __init__(self):
        # Use os.path.join for cross-platform compatibility
        # Check various case combinations
        self.paths = [
            os.path.join("game", "graph", "obj3d", "interactive"),
            os.path.join("GAME", "GRAPH", "OBJ3D", "INTERACTIVE"),
            os.path.join("Game", "Graph", "Obj3d", "Interactive"),
            os.path.join("graph", "obj3d", "interactive")  # Also check without "game"
        ]
        self.danglingPaths = []
        self.data = {}

    def update(self, absroot):

        for root, dirs, files in os.walk(absroot):
            # Calculate relative path from absroot
            relpath = os.path.relpath(root, absroot)
            split = splitPath(relpath)

            model = None

            for f in files:
                name, ext = os.path.splitext(f)
                if ext.lower() == ".ftl":
                    if name.lower() == split[-1].lower():
                        model = f
                    else:
                        self.danglingPaths.append(os.path.join(root, f))

            if model is not None:
                files.remove(model)

                for f in files:
                    if f.endswith(".unpack"):
                        pass
                    else:
                        self.danglingPaths.append(os.path.join(root, f))

                tweaks = []

                for d in dirs:
                    if d == "tweaks":
                        tweakDir = os.path.join(root, d)
                        tweakFiles = os.listdir(tweakDir)
                        for f in tweakFiles:
                            if f.endswith(".ftl"):
                                tweaks.append(f)
                            elif f.endswith(".unpack"):
                                pass
                            else:
                                self.danglingPaths.append(os.path.join(root, d, f))
                    else:
                        self.danglingPaths.append(os.path.join(root, d))

                dirs.clear()

                key = tuple(split)
                if key in self.data:
                    print("Warn: Duplicated name !!")

                m = ModelData(root, model, tweaks)
                self.data[key] = m


class Speeches:
    def __init__(self):
        self.paths = ["speech", "SPEECH", "Speech"]
        self.danglingPaths = []
        self.files = {}

    def update(self, absroot):
        dirs = os.listdir(absroot)
        for langName in dirs:
            l = os.path.join(absroot, langName)
            if os.path.isdir(l):
                foo = []
                sounds = os.listdir(l)
                for sound in sounds:
                    f = os.path.join(l, sound)
                    name, ext = os.path.splitext(sound)
                    if not os.path.isdir(f) and ext.lower() == ".wav":
                        foo.append(sound)
                    else:
                        self.danglingPaths.append(f)
                self.files[langName] = foo
            else:
                self.danglingPaths.append(l)


class LevelInfos(object):
    def __init__(self):
        self.fts = None
        self.dlf = None
        self.llf = None
        self.map = None
        self.load = None


class Levels:
    def __init__(self):
        # Use os.path.join for cross-platform compatibility
        # Check both possible locations for level data
        self.paths = [
            os.path.join("graph", "levels"),           # Primary location
            os.path.join("game", "graph", "levels"),   # Nested location (legacy)
            os.path.join("GRAPH", "levels"),           # Case variations
            os.path.join("Graph", "levels"),
            os.path.join("GAME", "GRAPH", "levels"),
            os.path.join("Game", "Graph", "levels")
        ]
        self.danglingPaths = []
        self.levels = OrderedDict()

    def update(self, absroot):
        print(f"DEBUG: Searching for level files in: {absroot}")
        dirs = os.listdir(absroot)
        for lvlName in dirs:
            l = os.path.join(absroot, lvlName)
            if not os.path.isdir(l):
                self.danglingPaths.append(l)
                continue

            # Case-insensitive check for level directories
            if not lvlName.lower().startswith('level'):
                self.danglingPaths.append(l)
                continue

            # Skip backup directories
            if '_backup' in lvlName.lower():
                continue

            try:
                # Extract level number (handle both "level0" and "Level0" and "LEVEL0")
                area_index = int(lvlName.lower().replace('level', ''))
            except ValueError:
                print(f"WARNING: Could not parse level number from: {lvlName}")
                continue

            info = self.levels.setdefault(area_index, LevelInfos())
            print(f"DEBUG: Processing level directory: {lvlName} (area {area_index})")

            foo = os.listdir(l)
            for bar in foo:
                blubb = os.path.join(l, bar)
                if not os.path.isdir(blubb):
                    name, ext = os.path.splitext(bar)
                    ext_lower = ext.lower()

                    # Skip backup files
                    if '.backup' in bar.lower() or '_backup' in bar.lower():
                        continue

                    # Only update if not already set (allows merging from multiple paths)
                    if ext_lower == ".fts":
                        if info.fts is None:
                            info.fts = blubb
                            print(f"  Found FTS file for area {area_index}: {blubb}")
                        else:
                            print(f"  Skipping duplicate FTS file for area {area_index}: {blubb}")
                    elif ext_lower == ".llf":
                        if info.llf is None:
                            info.llf = blubb
                            print(f"  Found LLF file for area {area_index}: {blubb}")
                        else:
                            print(f"  Skipping duplicate LLF file for area {area_index}: {blubb}")
                    elif ext_lower == ".dlf":
                        if info.dlf is None:
                            info.dlf = blubb
                            print(f"  Found DLF file for area {area_index}: {blubb}")
                        else:
                            print(f"  Skipping duplicate DLF file for area {area_index}: {blubb}")
                    elif bar.lower() == "map.bmp":
                        if info.map is None:
                            info.map = blubb
                    elif name.lower() == "loading":
                        if info.load is None:
                            info.load = blubb
                    else:
                        self.danglingPaths.append(blubb)
                else:
                    self.danglingPaths.append(blubb)

        # Final summary
        print(f"DEBUG: Found {len(self.levels)} levels total")
        for area_id, level_info in self.levels.items():
            missing = []
            if not level_info.dlf:
                missing.append("DLF")
            if not level_info.fts:
                missing.append("FTS")
            if not level_info.llf:
                missing.append("LLF")
            if missing:
                print(f"  Area {area_id}: Missing files: {', '.join(missing)}")

        self.levels = OrderedDict(sorted(self.levels.items()))


class Cinematics:
    def __init__(self):
        # Use os.path.join for cross-platform compatibility
        # Check various case combinations
        self.paths = [
            os.path.join("graph", "interface"),
            os.path.join("GRAPH", "INTERFACE"),
            os.path.join("Graph", "Interface"),
            os.path.join("game", "graph", "interface")  # Nested variant
        ]
        self.danglingPaths = []
        self.cins = []
        self.textures = {}

    def update(self, root):
        # Check for illustrations subdirectory with case-insensitive matching
        illustrations_dir = find_arx_directory(root, 'illustrations')
        if not illustrations_dir:
            # Try to find it manually with different cases
            for dirname in ['illustrations', 'Illustrations', 'ILLUSTRATIONS']:
                test_path = os.path.join(root, dirname)
                if os.path.exists(test_path):
                    illustrations_dir = test_path
                    break

        if not illustrations_dir:
            print(f"DEBUG: Illustrations directory not found in {root}")
            return

        if not os.path.isdir(illustrations_dir):
            print(f"DEBUG: Illustrations path exists but is not a directory: {illustrations_dir}")
            return

        sub = os.listdir(illustrations_dir)
        for s in sub:
            foo = os.path.join(illustrations_dir, s)
            if not os.path.isdir(foo):
                name, ext = os.path.splitext(s)
                if ext.lower() == ".cin":
                    self.cins.append(foo)
                else:
                    self.danglingPaths.append(foo)
            else:
                if s.lower() == "illust":
                    bar = os.listdir(foo)
                    for b in bar:
                        name, ext = os.path.splitext(b)
                        if ext.lower() == ".tga":
                            self.textures[name] = b
                        else:
                            self.danglingPaths.append(b)
                else:
                    self.danglingPaths.append(foo)


class Textures:
    def __init__(self):
        # Use os.path.join for cross-platform compatibility
        # Check various case combinations
        self.paths = [
            os.path.join("graph", "particles"),
            os.path.join("GRAPH", "PARTICLES"),
            os.path.join("Graph", "Particles"),
            os.path.join("graph", "interface"),
            os.path.join("GRAPH", "INTERFACE"),
            os.path.join("graph", "obj3d", "textures"),
            os.path.join("GRAPH", "OBJ3D", "TEXTURES"),
            os.path.join("game", "graph", "particles"),      # Nested variants
            os.path.join("game", "graph", "interface"),
            os.path.join("game", "graph", "obj3d", "textures")
        ]
        self.danglingPaths = []
        self.textures = []

    def update(self, root):
        pass


class Animations:
    def __init__(self):
        # Use os.path.join for cross-platform compatibility
        # Check various case combinations
        self.paths = [
            os.path.join("graph", "obj3d", "anims"),
            os.path.join("GRAPH", "OBJ3D", "ANIMS"),
            os.path.join("Graph", "Obj3d", "Anims"),
            os.path.join("game", "graph", "obj3d", "anims")  # Nested variant
        ]
        self.danglingPaths = []
        self.amins = []
        self.data = {}

    def update(self, root):
        for root, dirs, files in os.walk(root):

            for f in files:
                foo = os.path.join(root, f)
                name, ext = os.path.splitext(f)
                if ext.lower() == ".tea":
                    self.amins.append(foo)
                    self.data[name] = foo
                else:
                    self.danglingPaths.append(foo)


class AudioEffects:
    def __init__(self):
        self.paths = ["sfx", "SFX", "Sfx"]
        self.danglingPaths = []
        self.effects = []
        self.ambiances = []
        self.environments = []

    def update(self, root):
        for root, dirs, files in os.walk(root):

            for f in files:
                foo = os.path.join(root, f)
                name, ext = os.path.splitext(f)
                ext_lower = ext.lower()
                if ext_lower == ".wav":
                    self.effects.append(name)
                elif ext_lower == ".amb":
                    self.ambiances.append(name)
                elif ext_lower == ".aef":
                    self.environments.append(name)
                else:
                    self.danglingPaths.append(foo)

class AudioSpeech:
    def __init__(self):
        self.paths = ["speech", "SPEECH", "Speech"]
        self.danglingPaths = []
        self.speeches = {}

    def update(self, root):
        langDirNames = os.listdir(root)
        for langDirName in langDirNames:
            langDir = os.path.join(root, langDirName)
            fileNames = os.listdir(langDir)
            files = []
            for fileName in fileNames:
                files.append(fileName)

            self.speeches[langDirName] = files


class ArxFiles(object):
    def __init__(self, rootPath):
        self.rootPath = str(rootPath)
        self.allFiles = set()

        self.entities = Entities()
        self.models = Models()
        self.speeches = Speeches()
        self.levels = Levels()
        self.cinematics = Cinematics()
        self.animations = Animations()
        self.textures = Textures()
        self.audioEffects = AudioEffects()
        self.audioSpeech = AudioSpeech()

        self.handlers = [self.entities, self.models, self.speeches, self.levels, self.cinematics, self.animations,
                         self.textures, self.audioEffects, self.audioSpeech]

        self.danglingPaths = []

    def updateAll(self):
        print(f"DEBUG: Scanning Arx data from root path: {self.rootPath}")

        for root, dirs, files in os.walk(self.rootPath):
            relRoot = os.path.relpath(root, self.rootPath)

            for f in files:
                relFile = os.path.join(relRoot, f)
                self.allFiles.add(relFile)

        found_paths = set()

        for root, dirs, files in os.walk(self.rootPath):
            relRoot = os.path.relpath(root, self.rootPath)

            for h in self.handlers:
                # Fix for Windows: normalize paths before comparison
                # On Windows, paths may have different separators
                normalized_relRoot = os.path.normpath(relRoot)

                for handler_path in h.paths:
                    normalized_handler_path = os.path.normpath(handler_path)

                    # Always do case-insensitive comparison for Windows compatibility
                    # This works on both Windows and Linux
                    if normalized_relRoot.lower() == normalized_handler_path.lower():
                        if relRoot not in found_paths:
                            print(f"DEBUG: Found path '{relRoot}' -> handler {h.__class__.__name__}")
                            found_paths.add(relRoot)
                        h.update(root)
                        dirs.clear()
                        files.clear()
                        break

            for f in files:
                self.danglingPaths.append(os.path.join(root, f))

        # Summary of what was found
        print(f"DEBUG: Resource scan complete. Found:")
        print(f"  - {len(self.levels.levels)} levels")
        print(f"  - {len(self.models.data)} models")
        print(f"  - {len(self.entities.data)} entities")
        print(f"  - {len(self.animations.data)} animations")

        # self.entities.parseResourceReferences()
