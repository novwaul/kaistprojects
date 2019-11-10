class problemSolver:

	def __init__(self, maps):
		self.maps = maps
		self.rank = len(maps)
		self.column = len(maps[0])
		self.lengthCollection = []
	
	def giveSolution(self):
		try:
			return self.findMinLenToTreasure()
		except:
			return -1
	
	def findMinLenToTreasure(self):
		passedPositions = ()
		startPosition = [0,0]

		self.collectPathLen(startPosition, passedPositions)
		if len(self.lengthCollection) == 0:
			raise Exception
		else:
			return min(self.lengthCollection)
	
	def collectPathLen(self, Pos, passedPositions):
		if Pos in passedPositions:
			return
		if self.isTreasure(Pos):
			self.addPathLen(passedPositions)
			return
		if Pos[0] > 0 and self.maps[Pos[0]-1][Pos[1]] != 0:
			self.collectPathLen([Pos[0]-1,Pos[1]],passedPositions + (Pos,))
		if Pos[1] > 0 and self.maps[Pos[0]][Pos[1]-1] != 0:
			self.collectPathLen([Pos[0],Pos[1]-1],passedPositions + (Pos,))
		if Pos[0] < self.rank-1 and self.maps[Pos[0]+1][Pos[1]] != 0:
			self.collectPathLen([Pos[0]+1,Pos[1]],passedPositions + (Pos,))
		if Pos[1] < self.column-1 and self.maps[Pos[0]][Pos[1]+1] != 0:
			self.collectPathLen([Pos[0],Pos[1]+1],passedPositions + (Pos,))
		return

	def isTreasure(self, Position):
		return self.maps[Position[0]][Position[1]] == 2
	
	def addPathLen(self, passedPositions):
		length = len(passedPositions)
		lengthCollection = self.lengthCollection
		lengthCollection.append(length)
		return
	 
