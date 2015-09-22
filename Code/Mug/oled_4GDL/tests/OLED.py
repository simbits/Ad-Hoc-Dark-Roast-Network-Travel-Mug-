#!/usr/bin/evn python
import time
try:
    import serial
except ImportError, e:
    raise Exception('pyserial needs to be installed')

class OLED(object):
    def __init__(self, port, baudrate=19200, timeout=10):
        self._serial = serial.Serial(port, baudrate, timeout=timeout, dsrdtr=False)

    def GetTimeout(self):
        return self._serial.timeout
    
    def SetTimeout(self, timeout):
        self._serial.timeout = timeout

    timeout = property(GetTimeout, SetTimeout, None, 'Set timeout')

    def isOpen(self):
        return self._serial.isOpen()
 
    def Open(self):
        if (self.isOpen()):
            return
        try:
            self._serial.open()
        except Exception, e:
            raise Exception('Error opening serial port: %s' %(e))

    def Close(self):
        self._serial.close()

    def Read(self, eol='\n'):
        self._serial.setDTR(False)
        msg = self._serial.readline(eol).rstrip('\n\r')
        if msg == 'OK':
            return True, ''
        return False, msg

    def _constructPackage(self, msg):
        packet = ''
        
        if (msg == ''):
            packet = '$%c%c' % (0x01, 0x00)
        else:
            length = len(msg) + 1        
            raw = map(lambda x: ord(x), msg)
            checksum = ~(reduce(lambda x, y: x+y, raw)) + 1
            packet = '$%c%s%c' % (length & 0xff, msg, checksum & 0xff)

        return packet
            
    def Write(self, msg):
        packet = self._construct_packet(msg)
        self._serial.write(packet)
        self._serial.flush()

        return str((map(lambda x: ord(x), packet)))       

    def Cmd(self, msg):
        self.Write(msg)
        return self.Read()