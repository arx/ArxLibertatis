# this script requires blender as a module:
# https://archive.blender.org/wiki/index.php/User:Ideasman42/BlenderAsPyModule/

import argparse

parser = argparse.ArgumentParser()
parser.add_argument('-data')
parser.add_argument('-out')

args = parser.parse_args()

import os
import bpy
from math import degrees, radians, fabs, atan
from mathutils import Vector

from arx_addon.files import ArxFiles
from arx_addon.managers import ArxAddon

import html


class HtmlWriter:
    def __init__(self, path):
        self.path = path
        self.file = None

    def open(self):
        self.file = open(self.path, 'w')
        self.file.write("<!doctype html>\n<title>Arx Files</title>\n")

    def close(self):
        self.file.close()

    def escape(self, foo):
        return html.escape(foo).encode('ascii', 'xmlcharrefreplace').decode('ascii')

    def add(self, inner):
        self.file.write(inner + "\n")

    def li(self, inner):
        if isinstance(inner, list):
            for e in inner:
                self.li(e)
        else:
            self.file.write("<li>" + self.escape(inner) + "</li>\n")


def toHtml(arxFiles, path):
    html = HtmlWriter(os.path.join(path, "files.html"))
    html.open()

    keys = sorted(set(list(arxFiles.entities.data.keys()) + list(arxFiles.models.data.keys())))

    html.add("<table>")
    html.add("<tr>")
    html.add("<th>Key</th>")
    html.add("<th>Instances</th>")
    html.add("<th>Model</th>")
    html.add("<th>Tweaks</th>")
    html.add("</tr>")

    for key in keys:
        html.add("<tr>")
        html.add("<td>" + '/'.join(key) + "</td>")

        if key in arxFiles.entities.data:
            e = arxFiles.entities.data[key]
            html.add("<td>" + str(len(e.instances)) + "</td>")
        else:
            html.add("<td></td>")

        if key in arxFiles.models.data:
            m = arxFiles.models.data[key]
            html.add("<td>" + m.model + "</td>")
            html.add("<td>" + str(len(m.tweaks)) + "</td>")
        else:
            html.add("<td></td>")
            html.add("<td></td>")

        html.add("</tr>")
    html.add("</table>")

    html.add("<hr>")

    html.add("Dangling files:")
    html.add("<ul>")

    for h in arxFiles.handlers:
        html.add("<li>")
        html.add(str(h.__class__.__name__))
        html.add("<ul>")
        html.li(h.danglingPaths)
        html.add("</ul>")
        html.add("</li>")

    html.li(arxFiles.danglingPaths)
    html.add("</ul>")

    html.close()


def clearScene():
    scene = bpy.context.scene

    for ob in scene.objects:
        if ob.type == 'MESH' and ob.name.startswith("Cube"):
            ob.select = True
        else:
            ob.select = False

    bpy.ops.object.delete()


def setupObject(obj):
    obj.scale = [0.01, 0.01, 0.01]
    obj.rotation_euler = [radians(-150), radians(-20), radians(-20)]

    bpy.context.screen.scene.update()


def setupCamera(cam, obj):
    if cam.type != 'CAMERA':
        print("Wrong type")

    gBox = [obj.matrix_world * Vector(corner) for corner in obj.bound_box]


    left = 10000
    bottom = 10000
    right = -10000
    top = -10000

    for i in range(8):
        left = min(left, gBox[i][0])
        right = max(right, gBox[i][0])

        bottom = min(bottom, gBox[i][1])
        top = max(top, gBox[i][1])

        print(str(i) + ": " + str((gBox[i][0], gBox[i][1], gBox[i][2])))

    #left = gBox[0][0]
    #bottom = gBox[0][1]
    #right = gBox[4][0]
    #top = gBox[1][1]
    print((left, bottom, right, top))

    width = (fabs(left) + fabs(right)) / 2
    height = (fabs(bottom) + fabs(top)) / 2

    cx = left + width
    cy = bottom + height

    dist = 40

    angH = atan(width / dist)
    angV = atan(height / dist)

    if angH > angV:
        cam.data.angle_x = angH * 2
    else:
        cam.data.angle_y = angV * 2

    cam.location.x = cx
    cam.location.y = cy
    cam.location.z = dist
    cam.rotation_euler = (0, 0, 0)

    cam.data.type = 'PERSP'
    cam.data.lens_unit = 'FOV'

    bpy.context.screen.scene.update()


if args.data and args.out:
    print("Using data dir: " + args.data)
    print("Using out dir: " + args.out)

    dataPath = args.data
    outPath = args.out

    clearScene()

    arxFiles = ArxFiles(args.data)
    arxFiles.updateAll()

    toHtml(arxFiles, outPath)

    scene = bpy.context.scene
    scene.render.alpha_mode = 'TRANSPARENT'
    scene.render.resolution_x = 500
    scene.render.resolution_y = 500
    scene.render.resolution_percentage = 100

    arxAddon = ArxAddon(args.data)

    # key = ("npc", "spider_base")
    # key = ('items', 'magic', 'waterlily')
    # key = ('items','weapons','giant_slayer')
    # key = ('npc', 'human_base')
    # key = ('items', 'weapons', 'long_sword')
    # if 1:
    for i, key in enumerate(arxFiles.models.data):
        val = arxFiles.models.data[key]

        fileName = os.path.join(val.path, val.model)
        # fileName = os.path.join(val.path, "spider_base.ORIGINAL.ftl")

        # fileName += ".test.ftl"
        obj = arxAddon.objectManager.loadFile(fileName)
        cam = bpy.data.objects["Camera"]

        bpy.ops.object.mode_set(mode='OBJECT')

        setupObject(obj)
        setupCamera(cam, obj)

        renderFile = '::'.join(key) + ".png"
        bpy.context.scene.render.filepath = os.path.join(outPath, renderFile)
        bpy.ops.render.render(write_still=True)

        arxAddon.objectManager.saveFile(fileName + ".test.ftl")

        obj.select = True
        bpy.ops.object.delete()
