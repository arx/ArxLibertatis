# Copyright 2019 Arx Libertatis Team (see the AUTHORS file)
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

import logging

import bpy
from mathutils import Vector, Quaternion

from .dataTea import TeaSerializer


class ArxAnimationManager(object):
    def __init__(self):
        self.log = logging.getLogger('ArxAnimationManager')
        self.teaSerializer = TeaSerializer()
        
    def loadAnimation(self, path):
        data = self.teaSerializer.read(path)
        
        selectedObj = bpy.context.active_object

        #Walk up object tree to amature
        walkObj = selectedObj
        while not walkObj.name.endswith('-amt') and walkObj.parent:
            walkObj = walkObj.parent

        if not walkObj.name.endswith('-amt'):
            self.log.warning("Amature object nof found for: {}".format(obj.name))
            return

        armatureObj = walkObj
        obj = walkObj.children[0]

        bones = armatureObj.pose.bones
        
        bpy.context.scene.frame_set(1)
        for frame in data:
            bpy.context.scene.objects.active = obj
            
            if frame.translation:
                translation = frame.translation
                obj.location = (translation.x,translation.y,translation.z)
                obj.keyframe_insert(data_path='location')
                
            if frame.rotation:
                rotation = frame.rotation
                obj.rotation_quaternion = (rotation.w,rotation.x,rotation.y,rotation.z)
                obj.keyframe_insert(data_path='rotation_quaternion')

            #bpy.ops.anim.keyframe_insert(type='LocRotScale', confirm_success=False)
            
            bpy.context.scene.objects.active = armatureObj
            bpy.ops.object.mode_set(mode='POSE')

            if len(bones) != len(frame.groups):
                #raise InconsistentStateException("Bones in amature must match animation groups, existing {} new {}".format(len(bones), len(frame['groups'])))
                self.log.warning("Bones in amature must match animation groups, existing {} new {}".format(len(bones), len(frame.groups)))
                #break

            # This seems to be required to handle mismatched data
            maxBone = min(len(bones), len(frame.groups))

            for groupIndex in range(maxBone - 1, -1, -1): # group index = bone index
                group = frame.groups[groupIndex]
                bone = bones[groupIndex]
                location = Vector((group.translate.x,group.translate.y,group.translate.z))
                #self.log.info("moving bone to %s" % str(group.translate))
                #bone.location = location
                rotation = Quaternion((group.Quaternion.w,group.Quaternion.x,group.Quaternion.y,group.Quaternion.z))
                #self.log.info("rotating bone to %s" % str(rotation))
                bone.rotation_quaternion = rotation
                scale = Vector((group.zoom.x,group.zoom.y,group.zoom.z))
                #self.log.info("scaling bone to %s" % str(group.zoom))
                #bone.scale = scale
                bone.keyframe_insert(data_path="location")
                bone.keyframe_insert(data_path="rotation_quaternion")
                bone.keyframe_insert(data_path="scale")
                #bpy.ops.anim.keyframe_insert(type='LocRotScale', confirm_success=False)
                # FIXME translation and zoom are both 0,0,0
                # TODO keyframes of rotation are glitchy. probably cause im not doing it right. keyframe has to be set via bpy.ops.anim.keyframe_insert but i dont know how to select the bone.
                
            bpy.ops.object.mode_set(mode='OBJECT')
            
            bpy.context.scene.frame_set(bpy.context.scene.frame_current + frame.duration)
            #self.log.info("Loaded Frame")
        bpy.context.scene.frame_end = bpy.context.scene.frame_current