# Copyright 2014 Arx Libertatis Team (see the AUTHORS file)
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

from ctypes import (
    LittleEndianStructure,
    c_char,
    c_int32,
    c_uint32,
    c_float
)

from .dataCommon import (
    SavedVec3,
    ArxQuat
)

class THEA_HEADER(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("identity",      c_char * 20), # THEO_SIZE_IDENTITY_ANIM
        ("version",       c_uint32),
        ("anim_name",     c_char * 256), # SIZE_NAME
        ("nb_frames",     c_int32),
        ("nb_groups",     c_int32),
        ("nb_key_frames", c_int32),
    ]

class THEA_KEYFRAME_2015(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("num_frame",        c_int32),
        ("flag_frame",       c_int32),
        ("info_frame",       c_char * 256), # SIZE_NAME
        ("master_key_frame", c_int32),
        ("key_frame",        c_int32),
        ("key_move",         c_int32),
        ("key_orient",       c_int32),
        ("key_morph",        c_int32),
        ("time_frame",       c_int32),
    ]

class THEO_GROUPANIM(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("key_group",  c_int32),
        ("angle",      c_char * 8), # ignored
        ("Quaternion", ArxQuat),
        ("translate",  SavedVec3),
        ("zoom",       SavedVec3),
    ]

class THEA_SAMPLE(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("sample_name",  c_char * 256), # SIZE_NAME
        ("sample_size",  c_int32)
    ]


import logging
from ctypes import sizeof

class TeaSerializer(object):
    def __init__(self):
        self.log = logging.getLogger('TeaSerializer')
    
    def read(self, fileName):
        f = open(fileName, "rb")
        data = f.read()
        f.close()
        self.log.info("Loaded %i bytes from file %s" % (len(data), fileName))
        
        pos = 0
        
        header = THEA_HEADER.from_buffer_copy(data, pos)
        pos += sizeof(THEA_HEADER)
        
        self.log.info("Identity: {0}; Version: {1}; Frames: {2}; Groups {3}; KeyFrames {4}".format(header.identity, header.version, header.nb_frames, header.nb_groups, header.nb_key_frames))
        
        results = []
        for i in range(header.nb_key_frames):
            frame = {}
            #self.log.debug("Reading Keyframe {0}".format(i))
            kf = THEA_KEYFRAME_2015.from_buffer_copy(data, pos)
            pos += sizeof(THEA_KEYFRAME_2015)
            frame['duration'] = kf.num_frame
            frame['flags'] = kf.flag_frame
            
            # Global translation
            if kf.key_move != 0:
                trans = SavedVec3.from_buffer_copy(data, pos)
                pos += sizeof(SavedVec3)
                frame['translation'] = trans
                
            
            # Global rotation
            if kf.key_orient != 0:
                pos += 8; # skip THEO_ANGLE
                
                rot = ArxQuat.from_buffer_copy(data, pos)
                pos += sizeof(ArxQuat)
                frame['rotation'] = rot
            
            # Global Morph
            if kf.key_morph != 0:
                pos += 16; # skip THEA_MORPH
            
            # Now go for Group Rotations/Translations/scaling for each GROUP
            groupsList = THEO_GROUPANIM * header.nb_groups
            groups = groupsList.from_buffer_copy(data, pos)
            pos += sizeof(groupsList)
            frame['groups'] = groups
            
            # Has a sound sample ?
            num_sample = c_int32.from_buffer_copy(data, pos)
            pos += sizeof(c_int32)
            
            if num_sample.value != -1:
                sample = THEA_SAMPLE.from_buffer_copy(data, pos)
                pos += sizeof(THEA_SAMPLE)
                self.log.debug("sample_size: {0}".format(sample.sample_size))
                pos += sample.sample_size # skip data
            
            pos += 4 # skip num_sfx
            #self.log.debug("Pos: {0}".format(pos))
            results.append(frame)
        
        return results
