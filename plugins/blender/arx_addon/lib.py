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

import os
import platform
import ctypes
import logging

log = logging.getLogger('arx addon')

import sys
def getCurrentMachine():
    if sys.maxsize > 2**32:
        return 'x64'
    else:
        return 'x86'

import struct
def getDllMachine(dllFile):
    with open(dllFile, "rb") as f:
        f.seek(60)
        e_lfanew, = struct.unpack("<l", f.read(4))
        f.seek(e_lfanew)
        pemagic = f.read(4)
        if pemagic == b'PE\x00\x00':
            machine, = struct.unpack("<H", f.read(2))
            if machine == 0x8664:
                return 'x64'
            elif machine == 0x014c:
                return 'x86'
            else:
                raise RuntimeError("Unknown machine type in PE header" + str(machine))
        else:
            raise RuntimeError("PE magic not found" + str(pemagic))

def checkDll(dllFile):
    a = getDllMachine(dllFile)
    b = getCurrentMachine()
    if a != b:
        raise RuntimeError("Can not load '"+a+"' '" + dllFile + "' dll in '"+b+"' Blender, please use matching dll")

class ArxIO(object):
    messageBufferSize = 512

    def __init__(self):
        if platform.system() == "Windows":
            libPaths = [ os.path.realpath(__file__ + "\..\ArxIO.dll") ]
            checkDll(libPaths[0])
        else:
            libPaths = [
                os.path.realpath(__file__ + "/../libArxIO.so.0"),
                os.path.realpath(__file__ + "/../libArxIO.so"),
                "libArxIO.so.0"
            ]
        
        self.lib = None
        lastException = None
        for libPath in libPaths:
            if os.path.isfile(libPath):
                try:
                    self.lib = ctypes.cdll.LoadLibrary(libPath)
                    break
                except Exception as e:
                    lastException = e
                    continue
        if self.lib is None:
            raise Exception('could not load the ArxIO library from: [\n' + ',\n'.join(libPaths) + '\n]\nLoadLibrary Exception: ' + str(lastException))
        self.lib.ArxIO_init()

    def getError(self):
        errorBuffer = ctypes.create_string_buffer(self.messageBufferSize)
        self.lib.ArxIO_getError(errorBuffer, self.messageBufferSize)
        return RuntimeError(errorBuffer.value)
        
    def printLog(self):
        readMore = True
        while readMore:
            logBuffer = ctypes.create_string_buffer(self.messageBufferSize)
            result = self.lib.ArxIO_getLogLine(logBuffer, self.messageBufferSize)
            log.info("Lib " + logBuffer.value.decode("utf-8"))
            if result == 0:
                readMore = False;

    def unpack(self, data):
        cData = ctypes.create_string_buffer(data, len(data))
        
        out = ctypes.POINTER(ctypes.c_char)()
        outSize = ctypes.c_size_t()
        # FIXME error handling
        self.lib.ArxIO_unpack_alloc(cData, len(cData), ctypes.byref(out), ctypes.byref(outSize))
        
        result = ctypes.create_string_buffer(outSize.value)
        ctypes.memmove(result, out, outSize.value)
        
        self.lib.ArxIO_unpack_free(out)
        
        return result
