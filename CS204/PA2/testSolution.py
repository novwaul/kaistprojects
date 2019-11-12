import unittest
import solution

class testSolution(unittest.TestCase):
    def testValidDuplicateString(self):
        stringForTest = "BDDEA"
        result = solution.Main(stringForTest)
        self.assertEqual([6,8,3,4,0], result)

if __name__ == '__main__':
    unittest.main()