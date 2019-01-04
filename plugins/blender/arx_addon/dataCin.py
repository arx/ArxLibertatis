# Copyright 2014-2018 Arx Libertatis Team (see the AUTHORS file)
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
    UnexpectedValueException,
    readCstr,
    read_s32
)

def structToString(struct):
    return ', '.join([f[0] + '=' + str(getattr(struct, f[0])) for f in struct._fields_])

class CIN_HEADER(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("magic",   c_char * 4),
        ("version", c_uint32)
    ]

class CIN_TRACK(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ('startframe', c_int32), # unused always 0
        ('endframe',   c_int32),
        ('currframe',  c_float), # unused mostly 0
        ('fps',        c_float),
        ('nbkey',      c_int32),
        ('pause',      c_int32)  # unused always 1
    ]
    def __str__(self):
        return 'e {} curr {} fps {} keys {}'.format(
            self.endframe, self.currframe, self.fps, self.nbkey)


class CinematicLight_1_71(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ('pos',          SavedVec3),
        ('fallin',       c_float),
        ('fallout',      c_float),
        ('color',        SavedColor),
        ('intensity',    c_float),
        ('intensiternd', c_float),
        ('prev',         c_int32), # unused
        ('next',         c_int32)  # unused
    ]
    def __str__(self):
        return structToString(self)

class CIN_KEY_1_75(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ('frame',      c_int32),
        ('numbitmap',  c_int32),
        ('fx',         c_int32),
        ('typeinterp', c_int16),
        ('force',      c_int16),
        ('pos',        SavedVec3),
        ('angz',       c_float),
        ('color',      c_int32),
        ('colord',     c_int32),
        ('colorf',     c_int32),
        ('idsound',    c_int32),
        ('speed',      c_float),
        ('light',      CinematicLight_1_71),
        ('posgrille',  SavedVec3),
        ('angzgrille', c_float),
        ('speedtrack', c_float)
    ]
    def __str__(self):
        return structToString(self)

class Cin_Idsound_List(c_int32 * 16):
    def __str__(self):
        return ', '.join(str(self[i]) for i in range(len(self)))

class CIN_KEY_1_76(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ('frame',      c_int32),
        ('numbitmap',  c_int32),
        ('fx',         c_int32), # associated fx
        ('typeinterp', c_int16),
        ('force',      c_int16),
        ('pos',        SavedVec3),
        ('angz',       c_float),
        ('color',      c_int32),
        ('colord',     c_int32),
        ('colorf',     c_int32),
        ('speed',      c_float),
        ('light',      CinematicLight_1_71),
        ('posgrille',  SavedVec3),
        ('angzgrille', c_float),
        ('speedtrack', c_float),
        ('idsound',    Cin_Idsound_List)
    ]
    def __str__(self):
        return structToString(self)


import logging
from ctypes import sizeof
import os

from collections import namedtuple

CinData = namedtuple('CinData',
    ['track', 'keyframes'])

CinKeyframe = namedtuple('CinKeyframe',
    ['frame', 'image', 'sound'])

def normalizeImagePath(path):
    parts = path.lower().split('\\')
    name, ext = os.path.splitext(parts[-1])
    return name

def normalizeSoundPath(path):
    parts = path.lower().split('\\')
    return parts[-1]

class CinSerializer(object):
    def __init__(self):
        self.log = logging.getLogger('CinSerializer')
        self.log.setLevel(logging.DEBUG)

    def read(self, fileName):
        f = open(fileName, "rb")
        data = f.read()
        f.close()
        self.log.debug("Read %i bytes from file %s" % (len(data), fileName))

        pos = 0

        header = CIN_HEADER.from_buffer_copy(data, pos)
        pos += sizeof(CIN_HEADER)

        assert(header.magic == b'KFA'), "Magic mismatch"
        assert(header.version == 65611 or header.version == 65612), "Version invalid"

        weirdStr, pos = readCstr(data, pos)
        #self.log.info('str = {}'.format(weirdStr))

        images = []
        nbitmaps, pos = read_s32(data, pos)
        for i in range(nbitmaps):
            scale, pos = read_s32(data, pos)
            texture, pos = readCstr(data, pos)
            normTexture = normalizeImagePath(texture)
            images.append((scale, normTexture))
            #self.log.info("Texture {} - Scale: {}; Path: {}".format(i, scale, texture))

        sounds = []
        nsounds, pos = read_s32(data, pos)
        for i in range(nsounds):
            if(header.version == 65612):
                pos += 2 # ignore sound id
            sound, pos = readCstr(data, pos)
            sounds.append(sound)
            #self.log.info("Sound {} - Path: {}".format(i, sound))

        track = CIN_TRACK.from_buffer_copy(data, pos)
        pos += sizeof(CIN_TRACK)

        #self.log.info('Cinematic {} - Version {} Images {} Sounds {} Keyframes {}'.format(
        #    fileName, header.version, nbitmaps, nsounds, track.nbkey))

        #self.log.info(
        #    "Track - {}".format(str(track)))

        if track.startframe != 0:
            raise UnexpectedValueException("startframe = " + str(track.startframe))
        if track.pause != 1:
            raise UnexpectedValueException("track.pause = " + str(track.pause))

        def resolve_sound_id(id):
            if id == -1:
                return None
            elif id < len(sounds):
                return normalizeSoundPath(sounds[id])
            else:
                self.log.warning('Invalid sound index {} id {} total sounds {}'.format(i, id, len(sounds)))
                return None


        initialSound = None

        keyframes = []
        for i in range(track.nbkey):
            if (header.version == 65611):
                keyframe = CIN_KEY_1_75.from_buffer_copy(data, pos)
                pos += sizeof(CIN_KEY_1_75)

                #self.log.debug('Raw Keyframe - {}'.format(keyframe))

                if i == 0:
                    initialSound = resolve_sound_id(keyframe.idsound)

                sound = None
            else:
                keyframe = CIN_KEY_1_76.from_buffer_copy(data, pos)
                pos += sizeof(CIN_KEY_1_76)

                #self.log.debug('Raw Keyframe - {}'.format(keyframe))

                if i == 0:
                    initialSound = resolve_sound_id(keyframe.idsound[0])

                sound = resolve_sound_id(keyframe.idsound[3])

            image = images[keyframe.numbitmap]

            foo = CinKeyframe(
                frame = keyframe.frame,
                image = image,
                sound = sound
            )

            #self.log.debug('Keyframe - {}'.format(foo))

            keyframes.append(foo)

            if keyframe.typeinterp not in (-1, 0, 1):
                self.log.error('keyframe.typeinterp = {}'.format(str(keyframe.typeinterp)))
                #raise UnexpectedValueException("keyframe.typeinterp = " + str(keyframe.typeinterp))

            if keyframe.posgrille.x != 0 or keyframe.posgrille.y != 0 or keyframe.posgrille.z != 0:
                raise UnexpectedValueException('Grid pos not at 0, {}'.format(keyframe.posgrille))
            if keyframe.angzgrille != 0:
                raise UnexpectedValueException('keyframe.angzgrille not 0, {}'.format(keyframe.angzgrille))

            if keyframe.fx >= 0:
                fxBytes = keyframe.fx.to_bytes(4, byteorder='little')
                fxBase = fxBytes[0]
                fxPre = fxBytes[1]
                fxPost = fxBytes[2]
                fxLight = fxBytes[3]

                fxBaseVals = ['None', 'FX_FADEIN', 'FX_FADEOUT', 'FX_BLUR']
                fxPreVals = ['None', 'FX_DREAM']
                fxPostVals = ['None', 'FX_FLASH', 'FX_APPEAR', 'FX_APPEAR2']
                fxLightVals = ['None', 'FX_LIGHT']

                if fxBase > len(fxBaseVals):
                    self.log.warning('Bad fx base {}'.format(fxBase))
                    fxBase = 0
                if fxPre > len(fxPreVals):
                    self.log.warning('Bad fx pre {}'.format(fxPre))
                    fxPre = 0
                if fxPost > len(fxPostVals):
                    self.log.warning('Bad fx post {}'.format(fxPost))
                    fxPost = 0
                if fxLight > len(fxLightVals):
                    self.log.warning('Bad fx post {}'.format(fxLight))
                    fxPost = 0

                self.log.info('FX: base {} pre {} post {} light {}'
                              .format(fxBaseVals[fxBase], fxPreVals[fxPre], fxPostVals[fxPost], fxLightVals[fxLight]))




            #if keyframe.color != 0:
            #    raise UnexpectedValueException('Unexpected color, {}'.format(keyframe.color))


            if i == 0:
                if keyframe.frame != 0:
                    raise UnexpectedValueException('First keyframe should be at frame 0')
            else:
                if keyframe.frame <= lastFrameNr:
                    raise UnexpectedValueException('Keyframes not in order')

            lastFrameNr = keyframe.frame

        self.log.debug('Initial sound: {}'.format(initialSound))

        if track.endframe != lastFrameNr:
            self.log.warning(
                'Track endframe {} does not match last keyframe frame {}'.format(track.endframe, lastFrameNr))

        return CinData(
            track=track,
            keyframes=keyframes
        )
