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

import os
import platform
import ctypes
import logging

log = logging.getLogger('arx addon')

class ArxIO(object):
    messageBufferSize = 512

    def __init__(self):
        if platform.system() == "Windows":
            libPath = os.path.realpath(__file__ + "\..\ArxIO.dll")
        else:
            libPath = os.path.realpath(__file__ + "/../ArxIO.so")
        
        self.lib = ctypes.cdll.LoadLibrary(libPath)
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
