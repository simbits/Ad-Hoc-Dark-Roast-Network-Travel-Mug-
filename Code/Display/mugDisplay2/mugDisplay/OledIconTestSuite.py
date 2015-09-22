#!/usr/bin/evn python

import OledTestCaseWrapper
import unittest
import time

class OledPowerIconTestCase(OledTestCaseWrapper.OledInitializationWrapper):
    def test_show_power_level(self):
        success, msg = self._oled.Cmd('%c%c%c' % (0x01, 0x01, 0x01))
        self.assertTrue(success, msg)

    def test_hide_power_level(self):
        success, msg = self._oled.Cmd('%c%c%c' % (0x01, 0x01, 0x00))
        self.assertTrue(success, msg)

    def test_set_power_levels(self):
        success, msg = self._oled.Cmd('%c%c%c' % (0x01, 0x01, 0x01))
        self.assertTrue(success, msg)
        for level in range(0, 6):
            success, msg = self._oled.Cmd('%c%c' % (0x02, level))
            self.assertTrue(success, msg)

    def test_set_power_level_while_hidden_does_not_show(self):
        success, msg = self._oled.Cmd('%c%c%c' % (0x01, 0x01, 0x01))
        self.assertTrue(success, msg)
        success, msg = self._oled.Cmd('%c%c' % (0x02, 0x00))
        self.assertTrue(success, msg)
        success, msg = self._oled.Cmd('%c%c%c' % (0x01, 0x01, 0x00))
        self.assertTrue(success, msg)
        success, msg = self._oled.Cmd('%c%c' % (0x02, 0x03))
        self.assertTrue(success, msg)
        success, msg = self._oled.Cmd('%c%c%c' % (0x01, 0x01, 0x01))
        self.assertTrue(success, msg)

class OledSignalIconTestCase(OledTestCaseWrapper.OledInitializationWrapper):
    def test_show_signal_level(self):
        success, msg = self._oled.Cmd('%c%c%c' % (0x01, 0x02, 0x01))
        self.assertTrue(success, msg)

    def test_hide_signal_level(self):
        success, msg = self._oled.Cmd('%c%c%c' % (0x01, 0x02, 0x00))
        self.assertTrue(success, msg)

    def test_set_signal_levels(self):
        success, msg = self._oled.Cmd('%c%c%c' % (0x01, 0x02, 0x01))
        self.assertTrue(success, msg)
        for level in range(0, 5):
            success, msg = self._oled.Cmd('%c%c' % (0x03, level))
            self.assertTrue(success, msg)

    def test_set_signal_level_while_hidden_does_not_show(self):
        success, msg = self._oled.Cmd('%c%c%c' % (0x01, 0x02, 0x01))
        self.assertTrue(success, msg)
        success, msg = self._oled.Cmd('%c%c' % (0x03, 0x00))
        self.assertTrue(success, msg)
        success, msg = self._oled.Cmd('%c%c%c' % (0x01, 0x02, 0x00))
        self.assertTrue(success, msg)
        success, msg = self._oled.Cmd('%c%c' % (0x03, 0x03))
        self.assertTrue(success, msg)
        success, msg = self._oled.Cmd('%c%c%c' % (0x01, 0x02, 0x01))
        self.assertTrue(success, msg)            

class OledTransmissionAnimationTestCase(OledTestCaseWrapper.OledInitializationWrapper):
    def test_show_transmission_animation(self):
        success, msg = self._oled.Cmd('%c%c%c' % (0x01, 0x03, 0x01))
        self.assertTrue(success, msg)

    def test_hide_transmission_animation(self):
        success, msg = self._oled.Cmd('%c%c%c' % (0x01, 0x03, 0x01))
        self.assertTrue(success, msg)
        time.sleep(1)
        success, msg = self._oled.Cmd('%c%c%c' % (0x01, 0x03, 0x00))
        self.assertTrue(success, msg)

class OledIconBoundConditionsTestCase(OledTestCaseWrapper.OledInitializationWrapper):
    def test_set_power_level_outofbounds(self):
        success, msg = self._oled.Cmd('%c%c' % (0x02, 0x45))
        self.assertFalse(success, msg)
        self.assertEqual(msg, 'ERR:Invalid parameter')

    def test_set_signal_level_outofbounds(self):
        success, msg = self._oled.Cmd('%c%c' % (0x03, 0x45))
        self.assertFalse(success, msg)
        self.assertEqual(msg, 'ERR:Invalid parameter')
        
    def test_show_unknown_icon(self):
        success, msg = self._oled.Cmd('%c%c%c' % (0x01, 0x45, 0x01))
        self.assertFalse(success, msg)    
        self.assertEqual(msg, 'ERR:Unknown icon')

        
def suite():
    OledPowerIconTestSuite = unittest.TestLoader().loadTestsFromTestCase(OledPowerIconTestCase)
    OledSignalIconTestSuite = unittest.TestLoader().loadTestsFromTestCase(OledSignalIconTestCase)
    OledTransmissionAnimationTestSuite = unittest.TestLoader().loadTestsFromTestCase(OledTransmissionAnimationTestCase)
    OledIconBoundConditionsTestSuite = unittest.TestLoader().loadTestsFromTestCase(OledIconBoundConditionsTestCase)

    AllSuites = [OledPowerIconTestSuite,
                 OledSignalIconTestSuite,
                 OledTransmissionAnimationTestSuite,
                 OledIconBoundConditionsTestSuite]
                 
    return unittest.TestSuite(AllSuites)

if __name__ == '__main__':
    unittest.TextTestRunner(verbosity=2).run(suite())
                                                           