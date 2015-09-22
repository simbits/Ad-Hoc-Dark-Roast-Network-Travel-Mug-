#!/usr/bin/evn python

import OledTestCaseWrapper
import unittest
import time

class OledMessageDisplayTestCase(OledTestCaseWrapper.OledInitializationWrapper):
    def test_display_short_message_should_not_scroll(self):
        success, msg = self._oled.Cmd('%c%s' % (0x06, 'no scrolling\0'))
        self.assertTrue(success, msg)
        time.sleep(1)

    def test_display_long_message_should_scroll(self):
        success, msg = self._oled.Cmd('%c%s' % (0x06, 'Just a scrolling message\0'))
        self.assertTrue(success, msg)
        time.sleep(2)

class OledMessageDisplayBoundsTestCase(OledTestCaseWrapper.OledInitializationWrapper):
    def test_display_empty_selected_message(self):
        success, msg = self._oled.Cmd('%c%s' % (0x06, 'pre empty\0'))
        self.assertTrue(success, msg)
        time.sleep(1)
        success, msg = self._oled.Cmd('%c%s' % (0x06, '\0'))
        self.assertTrue(success, msg)
        time.sleep(1)

    #@unittest.skip("Needs fixing, visual check")    
    def test_display_selected_message_32_chars(self):
        success, msg = self._oled.Cmd('%c%s' % (0x06, 'A short message to scroll 123456\0'))
        self.assertTrue(success, msg)
        time.sleep(15)

    @unittest.skip("Needs fixing, visual check")
    def test_display_selected_message_64_chars(self):
        success, msg = self._oled.Cmd('%c%s' % (0x06, 'A short message to scroll 123456 A short message to scroll 1234\0'))
        self.assertTrue(success, msg)
        time.sleep(5)
        
    @unittest.skip("Fix 128 char freeze, visual check")
    def test_display_selected_message_128_chars(self):
        success, msg = self._oled.Cmd('%c%s' % (0x08, 'A short message to scroll 123456 A short message to scroll 12345 A short message to scroll 123456 A short message  \0'))
        self.assertTrue(success, msg)
        time.sleep(15)
        
class OledScrollWindowTestCase(OledTestCaseWrapper.OledInitializationWrapper):
    def test_write_selected_message(self):
        success, msg = self._oled.Cmd('%c%s' % (0x06, 'P(Si=qi)\0'))
        self.assertTrue(success, msg)
        time.sleep(0)
        success, msg = self._oled.Cmd('%c' % (0x07))
        self.assertTrue(success, msg)

    def test_scroll_message_window(self):
        success, msg = self._oled.Cmd('%c%s' % (0x06, '1\0'))
        self.assertTrue(success, msg)
        success, msg = self._oled.Cmd('%c' % (0x07))
        self.assertTrue(success, msg)
        time.sleep(0)
        success, msg = self._oled.Cmd('%c%s' % (0x06, '2\0'))
        self.assertTrue(success, msg)
        success, msg = self._oled.Cmd('%c' % (0x07))
        self.assertTrue(success, msg)
        success, msg = self._oled.Cmd('%c%s' % (0x06, 'Een heel lang zinnetje om te wrappen\0'))
        self.assertTrue(success, msg)
        success, msg = self._oled.Cmd('%c' % (0x07))
        self.assertTrue(success, msg)

    def test_direct_write_message_window(self):
        success, msg = self._oled.Cmd('%c%s' % (0x08, 'direct write\0'))
        self.assertTrue(success, msg)
        success, msg = self._oled.Cmd('%c%s' % (0x08, 'direct write very long message hoeuheee\0'))
        self.assertTrue(success, msg)
        success, msg = self._oled.Cmd('%c%s' % (0x08, '/me Een heel lang zinnetje om te wrappen\0'))
        self.assertTrue(success, msg)
        success, msg = self._oled.Cmd('%c%s' % (0x08, '/simon Bla en bla en bla hahhhaha\0'))
        self.assertTrue(success, msg)        

    def test_set_contrast(self):
        success, msg = self._oled.Cmd('%c%c' % (0x09, 0x01))
        self.assertTrue(success, msg)
        time.sleep(1)
        success, msg = self._oled.Cmd('%c%c' % (0x09, 0x04))
        self.assertTrue(success, msg)
        time.sleep(1)
        success, msg = self._oled.Cmd('%c%c' % (0x09, 0x0a))
        self.assertTrue(success, msg)
        time.sleep(1)
        success, msg = self._oled.Cmd('%c%c' % (0x09, 0x10))
        self.assertTrue(success, msg)
        time.sleep(1)

        
def suite():
    OledMessageDisplayTestSuite = unittest.TestLoader().loadTestsFromTestCase(OledMessageDisplayTestCase)
    OledMessageDisplayBoundsTestSuite = unittest.TestLoader().loadTestsFromTestCase(OledMessageDisplayBoundsTestCase)
    OledScrollWindowTestSuite = unittest.TestLoader().loadTestsFromTestCase(OledScrollWindowTestCase)

    AllSuites = [#OledMessageDisplayTestSuite,
                 #OledMessageDisplayBoundsTestSuite,
                 OledScrollWindowTestSuite]
                 
    return unittest.TestSuite(AllSuites)

if __name__ == '__main__':
    unittest.TextTestRunner(verbosity=2).run(suite()) 