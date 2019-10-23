import unittest
from solution import problemSolver

class testSolution(unittest.TestCase):
      
      def testDifferentLen(self):
          mapForTest = [[1,0,1,1,1],
	      		[1,0,1,0,1],
			[1,0,1,1,1],
			[1,1,1,0,1],
			[0,0,0,0,2]]
          minLenProblemSolver = problemSolver(mapForTest)
          result = minLenProblemSolver.findMinLenToTreasure()
          self.assertEqual(10, result)
      
      def testSameLen(self):
          mapForTest = [[1,1,1,1,1],
	  		[1,0,1,0,1],
			[1,1,2,1,1],
			[1,0,1,0,1],
			[1,1,1,1,1]]
          minLenProblemSolver = problemSolver(mapForTest)
          result = minLenProblemSolver.findMinLenToTreasure()
          self.assertEqual(4, result)

      def testNoPath(self):
          mapForTest = [[1,0,1,1,1],
	  		[1,0,1,0,1],
			[1,0,1,1,1],
			[1,1,1,0,0],
			[0,0,0,0,2]]
          minLenProblemSolver = problemSolver(mapForTest)
          result = minLenProblemSolver.findMinLenToTreasure() 
          self.assertEqual(-1, result)

      def testOnlyOnePath(self):
          mapForTest = [[1,1,1,1,1],
	  		[0,0,0,0,1],
			[1,1,1,1,1],
			[1,0,0,0,0],
			[1,1,1,1,2]]
          minLenProblemSolver = problemSolver(mapForTest)
          result = minLenProblemSolver.findMinLenToTreasure()
          self.assertEqual(16, result)

      def testOnlyOnePathWithLargeSize(self):
          mapForTest = [[1,1,1,1,1,1,1,1,1],
	  		[0,0,0,0,0,0,0,0,1],
			[1,1,1,1,1,1,1,0,1],
			[1,0,0,0,0,0,1,0,1],
			[1,0,1,1,2,0,1,0,1],
			[1,0,1,0,0,0,1,0,1],
			[1,0,1,1,1,1,1,0,1],
			[1,0,0,0,0,0,0,0,1], 
			[1,1,1,1,1,1,1,1,1]]
          minLenProblemSolver = problemSolver(mapForTest)
          result = minLenProblemSolver.findMinLenToTreasure() 
          self.assertEqual(48, result)


if __name__ == '__main__':
	unittest.main()
