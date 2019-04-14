
import dataclasses
import logging
import os

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

def arx_deep_compare(a, b, parent) -> DiffNode:
    name = type(a).__name__
    node = DiffNode(name, parent)
    if a == b:
        DiffNode('OK', node)
    else:
        if dataclasses.is_dataclass(a):
            for field in dataclasses.fields(a):
                field_a = getattr(a, field.name)
                field_b = getattr(b, field.name)
                field_node = DiffNode(field.name, node)
                if field_a == field_b:
                    DiffNode('OK', field_node)
                    continue
                arx_deep_compare(field_a, field_b, field_node)
        elif isinstance(a, list):
            if len(a) != len(b):
                DiffNode("length; expecteded {0} actual {1}".format(len(a), len(b)), node)
            limit = 0
            for i, (field_a, field_b) in enumerate(zip(a, b)):
                if field_a == field_b:
                    continue
                index_node = DiffNode("Index: {0}".format(i), node)
                arx_deep_compare(field_a, field_b, index_node)
                limit += 1
                if limit > 2:
                    DiffNode("Omitted more errors ...", index_node)
                    break
        else:
            DiffNode("expecteded {0}".format(a), node)
            DiffNode("actual     {0}".format(b), node)

def arx_ftl_compare(a, b) -> DiffNode:
    root = DiffNode('root')
    arx_deep_compare(a, b, root)
    return root

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
