
from ctypes import *

class ArxIO:
    messageBufferSize = 512

    def __init__(self, libraryPath):
        self.lib = cdll.LoadLibrary(libraryPath)
        self.lib.ArxIO_init()

    def getError(self):
        errorBuffer = create_string_buffer(self.messageBufferSize)
        self.lib.ArxIO_getError(errorBuffer, self.messageBufferSize)
        return RuntimeError(errorBuffer.value)
        
    def printLog(self):
        readMore = True
        while readMore:
            logBuffer = create_string_buffer(self.messageBufferSize)
            result = self.lib.ArxIO_getLogLine(logBuffer, self.messageBufferSize)
            print("Lib " + logBuffer.value)

            if result == 0:
                readMore = False;

    def getRawFtl(self, filePath):
        CfilePath = c_char_p(filePath)
        result = self.lib.ArxIO_ftlLoad(CfilePath)
        if result < 0:
            raise self.getError()

        dataBufferSize = self.lib.ArxIO_ftlGetRawDataSize()
        dataBuffer = create_string_buffer(dataBufferSize)
        self.lib.ArxIO_ftlGetRawData(dataBuffer, dataBufferSize)
        if result < 0:
            raise self.getError()

        self.lib.ArxIO_ftlRelease()

        return dataBuffer.raw

if __name__ == '__main__':
    # Example
    libraryPath = 'libArxIO.so'
    ftlPath = "game/graph/obj3d/interactive/fix_inter/chest/chest_wood.ftl"

    arx = ArxIO(libraryPath)
    data = arx.getRawFtl(ftlPath)
    arx.printLog()

    print len(data)

