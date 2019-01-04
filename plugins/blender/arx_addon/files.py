# Copyright 2015-2018 Arx Libertatis Team (see the AUTHORS file)
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
from collections import namedtuple, OrderedDict


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
        self.paths = ["graph/obj3d/interactive"]
        self.danglingPaths = []
        self.data = {}

    def update(self, absroot):

        for root, dirs, files in os.walk(absroot):
            split = splitPath(root[len(absroot) + len(self.paths):])

            mainScript = None

            for f in files:
                name, ext = os.path.splitext(f)
                if ext == ".asl" and name == split[-1]:
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
        self.paths = ["game/graph/obj3d/interactive"]
        self.danglingPaths = []
        self.data = {}

    def update(self, absroot):

        for root, dirs, files in os.walk(absroot):
            split = splitPath(root[len(absroot) + len(self.paths):])

            model = None

            for f in files:
                name, ext = os.path.splitext(f)
                if ext == ".ftl":
                    if name == split[-1]:
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
        self.paths = ["speech"]
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
                    if not os.path.isdir(f) and ext == ".wav":
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
        self.paths = ["graph/levels", "game/graph/levels"]
        self.danglingPaths = []
        self.levels = OrderedDict()

    def update(self, absroot):
        dirs = os.listdir(absroot)

        def sortFunc(x):
            l = []
            for p in re.split('([0-9]+)',  x):
                if p.isdigit():
                    l.append(int(p))
                elif len(p) > 0:
                    l.append(p)
            return tuple(l)

        dirs = sorted(dirs, key=sortFunc)
        for lvlName in dirs:
            l = os.path.join(absroot, lvlName)
            if os.path.isdir(l):
                foo = os.listdir(l)

                if lvlName in self.levels:
                    info = self.levels[lvlName]
                else:
                    info = LevelInfos()

                for bar in foo:
                    blubb = os.path.join(l, bar)
                    if not os.path.isdir(blubb):
                        name, ext = os.path.splitext(bar)
                        if ext == ".fts":
                            info.fts = blubb
                        elif ext == ".llf":
                            info.llf = blubb
                        elif ext == ".dlf":
                            info.dlf = blubb
                        elif bar == "map.bmp":
                            info.map = blubb
                        elif name == "loading":
                            info.load = blubb
                        else:
                            self.danglingPaths.append(blubb)
                    else:
                        self.danglingPaths.append(blubb)

                self.levels[lvlName] = info
            else:
                self.danglingPaths.append(l)


class Cinematics:
    def __init__(self):
        self.paths = ["graph/interface"] # TODO why not "graph/interface/illustrations"
        self.danglingPaths = []
        self.cins = []
        self.textures = {}

    def update(self, root):
        root = os.path.join(root, 'illustrations') # TODO hack
        sub = os.listdir(root)
        for s in sub:
            foo = os.path.join(root, s)
            if not os.path.isdir(foo):
                name, ext = os.path.splitext(s)
                if ext == ".cin":
                    self.cins.append(foo)
                else:
                    self.danglingPaths.append(foo)
            else:
                if s == "illust":
                    bar = os.listdir(foo)
                    for b in bar:
                        name, ext = os.path.splitext(b)
                        if ext == ".tga":
                            self.textures[name] = b
                        else:
                            self.danglingPaths.append(b)
                else:
                    self.danglingPaths.append(foo)


class Textures:
    def __init__(self):
        self.paths = ["graph/particles", "graph/interface", "graph/obj3d/textures"]
        self.danglingPaths = []
        self.textures = []

    def update(self, root):
        pass


class Animations:
    def __init__(self):
        self.paths = ["graph/obj3d/anims"]
        self.danglingPaths = []
        self.amins = []
        self.data = {}

    def update(self, root):
        for root, dirs, files in os.walk(root):

            for f in files:
                foo = os.path.join(root, f)
                name, ext = os.path.splitext(f)
                if ext == ".tea":
                    self.amins.append(foo)
                    self.data[name] = foo
                else:
                    self.danglingPaths.append(foo)


class AudioEffects:
    def __init__(self):
        self.paths = ["sfx"]
        self.danglingPaths = []
        self.effects = []
        self.ambiances = []
        self.environments = []

    def update(self, root):
        for root, dirs, files in os.walk(root):

            for f in files:
                foo = os.path.join(root, f)
                name, ext = os.path.splitext(f)
                if ext == ".wav":
                    self.effects.append(name)
                elif ext == ".amb":
                    self.ambiances.append(name)
                elif ext == ".aef":
                    self.environments.append(name)
                else:
                    self.danglingPaths.append(foo)

class AudioSpeech:
    def __init__(self):
        self.paths = ["speech"]
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
        for root, dirs, files in os.walk(self.rootPath):
            relRoot = os.path.relpath(root, self.rootPath)

            for f in files:
                relFile = os.path.join(relRoot, f)
                self.allFiles.add(relFile)

        for root, dirs, files in os.walk(self.rootPath):
            relRoot = os.path.relpath(root, self.rootPath)

            for h in self.handlers:
                if relRoot in h.paths:
                    h.update(root)
                    dirs.clear()
                    files.clear()
                    continue

            for f in files:
                self.danglingPaths.append(os.path.join(root, f))

                # self.entities.parseResourceReferences()
