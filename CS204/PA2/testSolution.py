import unittest
import solution

class testSolution(unittest.TestCase):
    def testMakeInnerVertex(self):
        testVertexName = 'B'
        vertexFactory = solution.vertexFactory()
        vertexForTest = vertexFactory.makeInnerVertex(testVertexName)
        vertexForAnswer = solution.vertex(6, 1, [8, 9])
        self.assertEqual(True, vertexForAnswer == vertexForTest)

    def testMakeOuterVertex(self):
        testVertexName = 'B'
        vertexFactory = solution.vertexFactory()
        vertexForTest = vertexFactory.makeOuterVertex(testVertexName)
        vertexForAnswer = solution.vertex(1, 6, [0, 2])
        self.assertEqual(True, vertexForAnswer == vertexForTest)

    def testValidDuplicateString(self):
        stringForTest = "BDDEA"
        result = solution.Main(stringForTest)
        self.assertEqual([6,8,3,4,0], result)

    def testValidNoneDupString(self):
        stringForTest = "ADBEC"
        result = solution.Main(stringForTest)
        self.assertEqual([5,8,6,9,7], result)
    
    def testValidDoubleDupString(self):
        stringForTest = "BBCCEBB"
        result = solution.Main(stringForTest)
        self.assertEqual([6,1,2,7,9,6,1], result)

    def testValidTripleString(self):
        stringForTest = "ABBBC"
        result = solution.Main(stringForTest)
        self.assertEqual([0,1,6,1,2], result)
    
    def testInvalidString(self):
        stringForTest = "ADCBB"
        result = solution.Main(stringForTest)
        self.assertEqual(-1, result)
        

if __name__ == '__main__':
    unittest.main()