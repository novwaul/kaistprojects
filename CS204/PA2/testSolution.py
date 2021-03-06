import unittest
import solution

class testSolution(unittest.TestCase):
    def testMakeInnerVertex(self):
        testVertexName = 'B'
        vertexFactory = solution.innerVertexFactory()
        vertexForTest = vertexFactory.makeInnerVertex(testVertexName)
        vertexForAnswer = solution.vertex(6, 1, [8, 9])
        self.assertEqual(True, vertexForAnswer == vertexForTest)

    def testMakeOuterVertex(self):
        testVertexName = 'B'
        vertexFactory = solution.outerVertexFactory()
        vertexForTest = vertexFactory.makeOuterVertex(testVertexName)
        vertexForAnswer = solution.vertex(1, 6, [0, 2])
        self.assertEqual(True, vertexForAnswer == vertexForTest)
    
    def testMakeOuterVertexWithInvalidInput(self):
        testVertexName = 'F'
        vertexFactory = solution.outerVertexFactory()
        self.assertRaises(solution.NoEntryException, vertexFactory.makeOuterVertex, testVertexName)

    def testValidDuplicateString(self):
        stringForTest = "BDDEA"
        pathFinder = solution.pathFinder()
        result = pathFinder.checkPathExist(stringForTest)
        self.assertEqual([6,8,3,4,0], result)

    def testValidNoneDupString(self):
        stringForTest = "ADBEC"
        pathFinder = solution.pathFinder()
        result = pathFinder.checkPathExist(stringForTest)
        self.assertEqual([5,8,6,9,7], result)
    
    def testValidDoubleDupString(self):
        stringForTest = "BBCCEBB"
        pathFinder = solution.pathFinder()
        result = pathFinder.checkPathExist(stringForTest)
        self.assertEqual([6,1,2,7,9,6,1], result)

    def testValidTripleString(self):
        stringForTest = "ABBBC"
        pathFinder = solution.pathFinder()
        result = pathFinder.checkPathExist(stringForTest)
        self.assertEqual([0,1,6,1,2], result)
    
    def testNoPathString(self):
        stringForTest = "ADCBB"
        pathFinder = solution.pathFinder()
        result = pathFinder.checkPathExist(stringForTest)
        self.assertEqual(-1, result)
    
    def testInvalidStringWithInvalidVertex(self):
        stringForTest = "FFFFFFFF"
        pathFinder = solution.pathFinder()
        result = pathFinder.checkPathExist(stringForTest)
        self.assertEqual(-1, result)
        

if __name__ == '__main__':
    unittest.main()