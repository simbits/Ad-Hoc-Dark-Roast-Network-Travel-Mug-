#!/usr/bin/evn python

import OledTestCaseWrapper
import unittest

class OledCommunicationTestCase(OledTestCaseWrapper.OledInitializationWrapper):
    def test_unknown_command_returns_ERR(self):
        success, msg = self._oled.Cmd('%c%c%c' % (0xfa, 0x01, 0x01))
        self.assertFalse(success, msg)
        self.assertEqual(msg, 'ERR:Unknown command')

    def test_no_command_returns_ERR(self):
        success, msg = self._oled.Cmd('')
        self.assertFalse(success, msg)
        self.assertEqual(msg, 'ERR:No command')

    def test_too_little_arguments_returns_ERR(self):
        success, msg = self._oled.Cmd('%c%c' % (0x01 ,0x01))
        self.assertFalse(success, msg)
        self.assertEqual(msg, 'ERR:Wrong arg count')

    def test_too_many_arguments_returns_ERR(self):
        success, msg = self._oled.Cmd('%c%c%c%c' % (0x01, 0x01, 0x02, 0x03))
        self.assertFalse(success, msg)
        self.assertEqual(msg, 'ERR:Wrong arg count')

def suite():
    return unittest.TestLoader().loadTestsFromTestCase(OledCommunicationTestCase)

if __name__ == '__main__':
    unittest.TextTestRunner(verbosity=2).run(suite())
                                                           