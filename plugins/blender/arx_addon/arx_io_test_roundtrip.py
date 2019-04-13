
import os
import logging

import bpy

from .arx_io_util import InconsistentStateException
from .dataFtl import FtlFace
from .files import ArxFiles

logging.basicConfig(level=logging.INFO)

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


def arx_ftl_compare(a, b):
    root = DiffNode("root")
    arx_ftl_compareMetadata(a.metadata, b.metadata, root)
    compare_array("Vertex", a.verts, b.verts, root, arx_ftl_compareVert)
    compare_array("Faces", a.faces, b.faces, root, arx_ftl_compareFace)
    compare_array("Materials", a.mats, b.mats, root, arx_ftl_compareMaterial)
    compare_array("Groups", a.groups, b.groups, root, arx_ftl_compareGroup)
    compare_array("Actions", a.actions, b.actions, root, arx_ftl_compareAction)
    compare_array("Selections", a.sels, b.sels, root, arx_ftl_compareSelection)
    return root

def arx_ftl_compareMetadata(a, b, parent):
    compare_attrib('name', a, b, parent)
    compare_attrib('org', a, b, parent)

def arx_ftl_compareVert(a, b, parent):
    compare_attrib('xyz', a, b, parent, equal_vector)
    compare_attrib('n', a, b, parent, equal_vector)

def arx_ftl_compareFace(a, b, parent):
    compare_attrib('vids', a, b, parent)
    compare_attrib('uvs', a, b, parent)
    compare_attrib('normal', a, b, parent)
    compare_attrib('texid', a, b, parent)
    compare_attrib('facetype', a, b, parent)
    compare_attrib('transval', a, b, parent)

def arx_ftl_compareMaterial(a, b, parent):
    # TODO either export with file ext BMP or ignore both import and export
    foo = os.path.splitext(b.upper())[0] + ".BMP"
    compare_value("name", a, foo, parent)

def arx_ftl_compareGroup(a, b, parent):
    compare_value("name", a[0], b[0], parent)
    compare_value("origin", a[1], b[1], parent)
    compare_array("vertices", a[2], b[2], parent, lambda a, b, p: compare_array_value(a, b, p))
    compare_value("father", a[3], b[3], parent)

def arx_ftl_compareAction(a, b, parent):
    compare_value("name", a[0], b[0], parent)
    compare_value("index", a[1], b[1], parent)

def arx_ftl_compareSelection(a, b, parent):
    compare_value("name", a[0], b[0], parent)
    compare_array("vertices", a[1], b[1], parent, lambda a, b, p: compare_array_value(a, b, p))

# ======================================================================================================================

def formatColumns(values):
    columnWidth = 15
    return '\t'.join([val.ljust(columnWidth) for val in values]) + '\n'

def test_export_of_current_scene_objects(assetManager):
    result_summary = bpy.data.texts.new('arx_test_export')
    result_summary.write(formatColumns(["Compare", "File"]))

    for bobj in bpy.data.objects:
        aobjId = tuple(bobj.name.split("/"))
        #TODO this is a hack to check if its a real object
        if len(aobjId) < 2:
            continue

        fsInfo = assetManager.arxFiles.models.data[aobjId]
        fileName = os.path.join(fsInfo.path, fsInfo.model)
        dataA = assetManager.objectManager.loadFile_data(fileName)

        dataB = assetManager.objectManager.toFtlData(bobj)

        comparison = arx_ftl_compare(dataA, dataB)

        txt = bpy.data.texts.new('arx_test_export_' + bobj.name)
        if not comparison.children:
            result_summary.write(formatColumns(['ok', bobj.name]))
            txt.from_string("Ok")
        else:
            txt.from_string(str(comparison))
            result_summary.write(formatColumns(['ERROR', bobj.name]))
