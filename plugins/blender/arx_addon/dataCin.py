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

from ctypes import (
    LittleEndianStructure,
    c_char,
    c_int16,
    c_int32,
    c_uint32,
    c_float
)

from .dataCommon import (
    SavedVec3,
    SavedColor,
    UnexpectedValueException
)


class CIN_HEADER(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("magic",         c_char * 4),
        ("version",       c_uint32)
    ]

class CIN_SIZE(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("count",         c_int32)
    ]

class CIN_TRACK(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("startframe", c_int32),
        ("endframe",   c_int32),
        ('currframe',  c_float),
        ('fps',        c_float),
        ('nbkey',      c_int32),
        ('pause',      c_int32)
    ]
    def __str__(self):
        return 's {} e {} curr {} fps {} keys {} paused {}'.format(
            self.startframe, self.endframe, self.currframe, self.fps, self.nbkey, self.pause)


class CinematicLight_1_71(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ('pos',           SavedVec3),
        ('fallin',        c_float),
        ('fallout',       c_float),
        ('color',         SavedColor),
        ('intensity',     c_float),
        ('intensiternd',  c_float),
        ('prev',          c_int32), # ignored
        ('next',          c_int32)  # ignored
    ]

class CIN_KEY_1_75(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ('frame',  c_int32),
        ('numbitmap',  c_int32),
        ('fx',  c_int32),
        ('typeinterp',  c_int16),
        ('force',  c_int16),
        ('pos',  SavedVec3),
        ('angz',  c_float),
        ('color',  c_int32),
        ('colord',  c_int32),
        ('colorf',  c_int32),
        ('idsound',  c_int32),
        ('speed',  c_float),
        ('light',  CinematicLight_1_71),
        ('posgrille',  SavedVec3),
        ('angzgrille',  c_float),
        ('speedtrack',  c_float)
    ]

class CIN_KEY_1_76(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ('frame', c_int32),
        ('numbitmap', c_int32),
        ('fx', c_int32), # associated fx
        ('typeinterp', c_int16),
        ('force', c_int16),
        ('pos', SavedVec3),
        ('angz', c_float),
        ('color', c_int32),
        ('colord', c_int32),
        ('colorf', c_int32),
        ('speed', c_float),
        ('light', CinematicLight_1_71),
        ('posgrille', SavedVec3),
        ('angzgrille', c_float),
        ('speedtrack', c_float),
        ('idsound', c_int32 * 16)
    ]


import logging
from ctypes import sizeof
import struct

def read_s32(data, pos):
    value = struct.unpack_from('<i', data, pos)[0]
    endPos = pos + 4;
    return (value, endPos)

def readCstr(data, pos):
    strEnd = data[pos:].index(b'\x00')
    strEndPos = pos + strEnd
    somStr = data[pos:strEndPos]
    return (somStr, strEndPos + 1) # +1 for the null

class CinSerializer(object):
    def __init__(self):
        self.log = logging.getLogger('CinSerializer')

    def read(self, fileName):
        f = open(fileName, "rb")
        data = f.read()
        f.close()
        self.log.info("Read %i bytes from file %s" % (len(data), fileName))

        pos = 0

        header = CIN_HEADER.from_buffer_copy(data, pos)
        pos += sizeof(CIN_HEADER)

        assert(header.magic == b'KFA'), "Magic mismatch"
        assert(header.version == 65611 or header.version == 65612), "Version invalid"

        self.log.info(
            "Header - Identity: {0}; Version: {1}".format(header.magic, header.version))

        weirdStr, pos = readCstr(data, pos)
        nbitmaps, pos = read_s32(data, pos)

        for i in range(nbitmaps):
            scale, pos = read_s32(data, pos)
            texture, pos = readCstr(data, pos)
            self.log.info("Texture {} - Scale: {}; Path: {}".format(i, scale, texture))

        nsounds, pos = read_s32(data, pos)
        for i in range(nsounds):
            if(header.version == 65612):
                pos += 2 # ignore sound id
            sound, pos = readCstr(data, pos)
            self.log.info("Sound {} - Path: {}".format(i, sound))

        track = CIN_TRACK.from_buffer_copy(data, pos)
        pos += sizeof(CIN_TRACK)

        self.log.info(
            "Track - {}".format(str(track)))

        if track.startframe != 0:
            raise UnexpectedValueException("startframe = " + str(track.startframe))


        for i in range(track.nbkey):
            if (header.version == 65611):
                keyframe = CIN_KEY_1_75.from_buffer_copy(data, pos)
                pos += sizeof(CIN_KEY_1_75)
            else:
                keyframe = CIN_KEY_1_76.from_buffer_copy(data, pos)
                pos += sizeof(CIN_KEY_1_76)

        self.log.info("Cin File loaded")
