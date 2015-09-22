#!/usr/bin/evn python

from OLED import OLED
import unittest

class OledInitializationWrapper(unittest.TestCase):
    _oled = None
    
    @classmethod
    def setUpClass(cls):
        success = False
        
        cls._oled = OLED('COM14')
        cls._oled.Open()

        cls._oled.timeout = 10

        for i in range(0, 10):
            #print 'Waiting for display'
            s, m = cls._oled.Read()
            if s:
                success = True
                break
            
        if not success:
            raise Exception('Error opening OLED connection')

    @classmethod        
    def tearDownClass(cls):
        cls._oled.Close()
    