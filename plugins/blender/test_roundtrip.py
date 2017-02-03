#!/usr/bin/env python3

import argparse
import os

import logging
logging.basicConfig(level=logging.INFO)

import bpy

from arx_addon.dataFtl import FtlFace
from arx_addon.files import ArxFiles
from arx_addon import dataFtl, lib

from arx_addon.managers import InconsistentStateException

# ======================================================================================================================

class DiffNode(object):
    def __init__(self, name, parent=None):
        self.name = name
        self.parent = parent
        self.children = []
        if parent:
            parent.children.append(self)

    def addChild(self, child):
        child.parent = self
        self.children.append(child)

    def __str__(self):
        result = ""

        stack = list()
        stack.append(self)

        child_stack_list = list()
        child_stack_list.append(stack)

        while child_stack_list:
            child_stack = child_stack_list[-1]
            if child_stack:
                current_node = child_stack.pop(0)

                stack_indent = "";
                for i in range(0, len(child_stack_list) - 1):
                    if child_stack_list[i]:
                        stack_indent += "│ "
                    else:
                        stack_indent += "  "

                if child_stack:
                    current_indent = '├ '
                else:
                    current_indent = '╰ '

                result += stack_indent + current_indent + current_node.name + '\n'

                if current_node.children:
                    # Make sure to copy the list here
                    child_stack_list.append(list(current_node.children))
            else:
                child_stack_list.pop()

        return result

# ======================================================================================================================

def equal_float(a, b, tol = 0.00001):
    return abs(a - b) < tol

def equal_vector(a, b, tol = 0.00001):
    for a_e, b_e in zip(a, b):
        if not equal_float(a_e, b_e, tol):
            return False
    return True

def compare_value(name, a, b, parent, comp=lambda a, b: a==b):
    if not comp(a, b):
        value_node = DiffNode(name, parent)
        DiffNode("expecteded {0}".format(a), value_node)
        DiffNode("actual     {0}".format(b), value_node)

def compare_attrib(name, a, b, parent, comp=lambda a, b: a==b):
    attr_a = getattr(a, name)
    attr_b = getattr(b, name)
    compare_value(name, attr_a, attr_b, parent, comp)

skipArrayOnError = True

def compare_array(name, a, b, parent, entryComparer):
    array_node = DiffNode(name + " array")

    if len(a) != len(b):
        DiffNode("length; expecteded {0} actual {1}".format(len(a), len(b)), array_node)

    for i, (a_e, b_e) in enumerate(zip(a, b)):
        index_node = DiffNode("Index: {0}".format(i))

        entryComparer(a_e, b_e, index_node)

        if index_node.children:
            array_node.addChild(index_node)
            if skipArrayOnError:
                break

    if array_node.children:
        parent.addChild(array_node)

def compare_array_value(a, b, parent):
    if a != b:
        DiffNode("expecteded {0}".format(a), parent)
        DiffNode("actual     {0}".format(b), parent)

class FtlDiff(object):
    def __init__(self):
        self.ioLib = lib.ArxIO()
        self.serializer = dataFtl.FtlSerializer()

    def compareFiles(self, file_path_a, file_path_b):
        file_a = open(file_path_a, "rb")
        data_a = file_a.read()
        file_a.close()
        unpacked_a = self.ioLib.unpack(data_a)

        file_b = open(file_path_b, "rb")
        unpacked_b = file_b.read()
        file_b.close()

        deser_a = self.serializer.read(unpacked_a)
        deser_b = self.serializer.read(unpacked_b)

        return self.compare(deser_a, deser_b)

    def compare(self, a, b):
        root = DiffNode("root")
        self.compareMetadata(a.metadata, b.metadata, root)
        compare_array("Vertex", a.verts, b.verts, root, self.compareVert)
        compare_array("Faces", a.faces, b.faces, root, self.compareFace)
        compare_array("Materials", a.mats, b.mats, root, self.compareMaterial)
        compare_array("Groups", a.groups, b.groups, root, self.compareGroup)
        compare_array("Actions", a.actions, b.actions, root, self.compareAction)
        compare_array("Selections", a.sels, b.sels, root, self.compareSelection)
        return root

    def compareMetadata(self, a, b, parent):
        compare_attrib('name', a, b, parent)
        compare_attrib('org', a, b, parent)

    def compareVert(self, a, b, parent):
        compare_attrib('xyz', a, b, parent, equal_vector)
        compare_attrib('n', a, b, parent, equal_vector)

    def compareFace(self, a, b, parent):
        compare_attrib('vids', a, b, parent)
        compare_attrib('uvs', a, b, parent)
        compare_attrib('normal', a, b, parent)
        compare_attrib('texid', a, b, parent)
        compare_attrib('facetype', a, b, parent)
        compare_attrib('transval', a, b, parent)

    def compareMaterial(self, a, b, parent):
        # TODO either export with file ext BMP or ignore both import and export
        foo = os.path.splitext(b.upper())[0] + ".BMP"
        compare_value("name", a, foo, parent)

    def compareGroup(self, a, b, parent):
        compare_value("name", a[0], b[0], parent)
        compare_value("origin", a[1], b[1], parent)
        compare_array("vertices", a[2], b[2], parent, lambda a, b, p: compare_array_value(a, b, p))
        compare_value("father", a[3], b[3], parent)

    def compareAction(self, a, b, parent):
        compare_value("name", a[0], b[0], parent)
        compare_value("index", a[1], b[1], parent)

    def compareSelection(self, a, b, parent):
        compare_value("name", a[0], b[0], parent)
        compare_array("vertices", a[1], b[1], parent, lambda a, b, p: compare_array_value(a, b, p))

# ======================================================================================================================

def formatColumns(values):
    columnWidth = 15
    return '\t'.join([val.ljust(columnWidth) for val in values]) + '\n'

def formatFailedStatus(errorId):
    return "Failed {0:04d}".format(errorId)

def formatError(errorId, file, message):
        return "ErrorId: {0}\tFile: {1}\n\n{2}\n========================================\n".format(errorId, file, message)

class RoundtripTester(object):
    def __init__(self, dataDirectory):
        self.log = logging.getLogger('RoundtripTester')
        self.dataDirectory = dataDirectory
        self.skipExport = False
        self.skipCompare = False
        self.comparer = FtlDiff()

        self.arxFiles = ArxFiles(self.dataDirectory)
        self.arxFiles.updateAll()

    def setupCleanBlenderEnvironment(self):
        name = 'arx_addon'

        import addon_utils
        from bpy import context

        # Disable the module first to prevent log spam
        for module in addon_utils.modules():
            if module.__name__ == name:
                is_enabled, is_loaded = addon_utils.check(name)
                if is_loaded:
                    addon_utils.disable(name)

        #bpy.ops.wm.read_homefile()
        bpy.ops.wm.read_factory_settings()

        #addon_utils.modules_refresh()

        moduleFound = False
        for module in addon_utils.modules():
            if module.__name__ == name:
                default, enable = addon_utils.check(name)
                if not enable:
                    addon_utils.enable(name, default_set=True, persistent=False, handle_error=None)
                    context.user_preferences.addons[name].preferences.arxAssetPath = self.dataDirectory
                moduleFound = True

        if not moduleFound:
            raise Exception("Addon not found: {0}".format(name))

        # Cleanup the default scene
        def removeObj(name):
            defaultCube = bpy.context.scene.objects.get(name)
            if defaultCube:
                defaultCube.select = True
                bpy.ops.object.delete()

        removeObj('Cube')
        removeObj('Camera')
        removeObj('Lamp')

    def roundtripModels(self):
        print("Roundtrip Models")

        result_file = open("test_files.txt", "w")
        result_file.write(formatColumns(["Import", "Export", "Compare", "File"]))
        error_file = open("test_errors.txt", "w")

        current_error_id = 0

        for key, val in sorted(self.arxFiles.models.data.items()):
            import_file = os.path.join(val.path, val.model)
            import_file_relative = os.path.relpath(import_file, self.dataDirectory)
            export_file = "test.ftl"

            self.setupCleanBlenderEnvironment()

            try:
                bpy.ops.arx.import_ftl(filepath=import_file)
                import_status = "Ok"
                import_ok = True
            except RuntimeError as e:
                import_ok = False
                error_file.write(formatError(current_error_id, import_file_relative, e.args[0]))
                error_file.flush()
                import_status = formatFailedStatus(current_error_id)
                current_error_id += 1

            if not self.skipExport and import_ok:
                try:
                    bpy.ops.arx.export_ftl(filepath=export_file)
                    export_ok = True
                    export_status = "Ok"
                except RuntimeError as e:
                    export_ok = False
                    error_file.write(formatError(current_error_id, import_file_relative, e.args[0]))
                    error_file.flush()
                    export_status = formatFailedStatus(current_error_id)
                    current_error_id += 1
            else:
                export_ok = False
                export_status = "Skip"

            if not self.skipCompare and import_ok and export_ok:
                result = self.comparer.compareFiles(import_file, export_file)
                if result and not result.children:
                    compare_status = "Ok"
                else:
                    error_file.write(formatError(current_error_id, import_file_relative, str(result)))
                    error_file.flush()
                    compare_status = formatFailedStatus(current_error_id)
                    current_error_id += 1
            else:
                compare_status = "Skip"

            result_file.write(formatColumns([import_status, export_status, compare_status, str(import_file_relative)]))
            result_file.flush()

        result_file.close()
        error_file.close()

    def loadAnimations(self):
        print("Load Animations")

        for key, val in sorted(self.arxFiles.entities.data.items()):
            usedAnimations = []

            scriptPath = os.path.join(val.path, val.script)
            with open(scriptPath, 'rt', encoding='iso-8859-1') as scriptData:
                for line in scriptData:
                    line = line.strip().lower()
                    commentSplit = line.split("//", 2)
                    if commentSplit[0]:
                        commandSplit = commentSplit[0].split()
                        if commandSplit[0] == "loadanim":
                            # anim slot name is in [1]
                            animName = commandSplit[2].strip("\"")
                            # we can't resolve variable values here, ignore ...
                            if not animName.startswith("~"):
                                usedAnimations.append(animName)
                            else:
                                #print("Ignored anim variable: {}".format(animName))
                                pass

            if not usedAnimations:
                self.log.debug("No used animations for {}".format(key))
                pass
            else:
                if not key in self.arxFiles.models.data:
                    self.log.info("No model found for entity: {}".format(key))
                    pass
                else:
                    model = self.arxFiles.models.data[key]
                    import_file = os.path.join(model.path, model.model)

                    for anim in usedAnimations:
                        if not anim in self.arxFiles.animations.data:
                            self.log.warning("Animation {} not found for model {}".format(anim, key))
                            continue

                        animFileName = self.arxFiles.animations.data[anim]
                        self.log.info("Checking model [{}] anim [{}] ".format(key, anim))

                        self.setupCleanBlenderEnvironment()
                        bpy.ops.arx.import_ftl(filepath=import_file)
                        bpy.ops.object.mode_set(mode='OBJECT')

                        #Find root object
                        root = None
                        for o in bpy.data.objects:
                            if not o.parent:
                                root = o

                        root.select = True
                        bpy.context.scene.objects.active = root

                        try:
                            bpy.ops.arx.import_tea(filepath=animFileName)
                        except:
                        #except InconsistentStateException as ise:
                            pass
                            #self.log.error("Failed to import animation {} for model {}".format(anim, key))



if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('-d', '--data-dir', help='Where to find the data files', required=True)
    args = parser.parse_args()
    print("Using data dir: " + args.data_dir)

    tester = RoundtripTester(args.data_dir)
    tester.roundtripModels()
    tester.loadAnimations()
