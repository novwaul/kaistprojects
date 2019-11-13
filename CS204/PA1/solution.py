class problemSolver:

	def __init__(self):
		self.maps = None
		self.rank = None
		self.column = None
		self.lengthCollection = []
	
	def giveSolution(self, maps):
		self.getProblem(maps)
		return self.solveProblem()

	def getProblem(self, maps):
		self.maps = maps
		self.rank = len(maps)
		self.column = len(maps[0])

	def solveProblem(self):
		try:
			return self.findMinLenToTreasure()
		except NoPathException as e:
			return e.returnErrValue()
	
	def findMinLenToTreasure(self):
		passedPositions = ()
		startPosition = [0,0]

		self.collectAllPathLen(startPosition, passedPositions)
		if len(self.lengthCollection) == 0:
			raise NoPathException()
		else:
			return min(self.lengthCollection)
	
	def collectAllPathLen(self, Pos, passedPositions):
		if self.isInvalidPosition(Pos, passedPositions):
			return
		elif self.isTreasure(Pos):
			self.addPathLen(passedPositions)
			return
		else:
			self.collectAllPathLen([Pos[0] + 1, Pos[1]],passedPositions + (Pos,))
			self.collectAllPathLen([Pos[0] - 1, Pos[1]],passedPositions + (Pos,))
			self.collectAllPathLen([Pos[0], Pos[1] + 1],passedPositions + (Pos,))
			self.collectAllPathLen([Pos[0], Pos[1] - 1],passedPositions + (Pos,))
			return
	
	def isInvalidPosition(self, position, passedPositions):
		return self.checkBound(position) or self.checkGoBack(position, passedPositions) or self.checkWall(position)

	def checkBound(self, position):
		return 0 > position[0] or position[0] >= self.rank or 0 > position[1] or position[1] >= self.column
	
	def checkGoBack(self, position, passedPositions):
		return position in passedPositions

	def checkWall(self, position):
		return self.maps[position[0]][position[1]] == 0

	def isTreasure(self, position):
		return self.maps[position[0]][position[1]] == 2
	
	def addPathLen(self, passedPositions):
		length = len(passedPositions)
		lengthCollection = self.lengthCollection
		lengthCollection.append(length)
		return

class NoPathException(Exception):
	def __init__(self, value = -1):
		self.value = value
	
	def returnErrValue(self):
		return self.value
