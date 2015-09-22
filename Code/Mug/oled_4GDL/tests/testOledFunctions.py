#!/usr/bin/env python
import time
import OledTestCaseWrapper
import OledCommunicationTestSuite
import OledIconTestSuite
import OledTextTestSuite
import unittest
        
if __name__ == '__main__':
    OledCommunicationSuite = OledCommunicationTestSuite.suite()
    OledIconSuite = OledIconTestSuite.suite()
    OledTextSuite = OledTextTestSuite.suite()
    
    AllTestSuites = unittest.TestSuite([OledCommunicationSuite,
                                        OledIconSuite,
                                        OledTextSuite])

    unittest.TextTestRunner(verbosity=2).run(AllTestSuites)
    