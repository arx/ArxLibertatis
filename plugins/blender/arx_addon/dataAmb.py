# Copyright 2018 Arx Libertatis Team (see the AUTHORS file)
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
    c_uint32,
    c_float
)

from .dataCommon import readCstr


class AMB_HEADER(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("magic",    c_uint32),
        ("version",  c_uint32),
        ("nbtracks", c_uint32)
    ]

class AMB_TRACK_HEADER(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("iflags", c_uint32),
        ("key_c",  c_uint32)
    ]

class AMB_TRACK_KeySetting(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("min",      c_float),
        ("max",      c_float),
        ("interval", c_uint32),
        ("flags",    c_uint32)
    ]

class AMB_TRACK_Key(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("flags",     c_uint32),
        ("start",     c_uint32),
        ("loop",      c_uint32),
        ("delay_min", c_uint32),
        ("delay_max", c_uint32),
        ("volume",    AMB_TRACK_KeySetting),
        ("pitch",     AMB_TRACK_KeySetting),
        ("pan",       AMB_TRACK_KeySetting),
        ("x",         AMB_TRACK_KeySetting),
        ("y",         AMB_TRACK_KeySetting),
        ("z",         AMB_TRACK_KeySetting)
    ]


import logging
from ctypes import sizeof
from collections import namedtuple
from typing import List

AmbTrack = namedtuple("AmbTrack", ["samplePath", "flags", "keys"])

class AmbSerializer(object):
    def __init__(self):
        self.log = logging.getLogger('AmbSerializer')

    def read(self, fileName) -> List[AmbTrack]:
        f = open(fileName, "rb")
        data = f.read()
        f.close()
        self.log.debug("Read %i bytes from file %s" % (len(data), fileName))

        pos = 0

        header = AMB_HEADER.from_buffer_copy(data, pos)
        pos += sizeof(AMB_HEADER)

        AMBIANCE_FILE_SIGNATURE = int('0x424d4147', 16)  # 'GAMB'
        VERSION_1001 = int('0x01000001', 16)
        VERSION_1002 = int('0x01000002', 16)
        VERSION_1003 = int('0x01000003', 16)

        assert(header.magic == AMBIANCE_FILE_SIGNATURE), "Magic mismatch"
        assert(header.version == VERSION_1001 or header.version == VERSION_1002 or header.version == VERSION_1003), "Bad version"

        tracks = []
        for i in range(header.nbtracks):
            samplePath, pos = readCstr(data, pos)
            samplePath = samplePath.replace('\\', '/').lower()

            if header.version >= VERSION_1002:
                trackName, pos = readCstr(data, pos)
                assert(not trackName)

            trackHeader = AMB_TRACK_HEADER.from_buffer_copy(data, pos)
            pos += sizeof(AMB_TRACK_HEADER)

            trackKeys = []
            for u in range(trackHeader.key_c):
                trackKey = AMB_TRACK_Key.from_buffer_copy(data, pos)
                pos += sizeof(AMB_TRACK_Key)
                trackKeys.append(trackKey)

            tracks.append(AmbTrack(
                samplePath=samplePath,
                flags=trackHeader.iflags,
                keys=trackKeys
            ))

        return tracks

