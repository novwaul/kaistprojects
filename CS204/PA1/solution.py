class problemSolver:

	def __init__(self, maps):
		self.maps = maps
		self.rank = len(maps) #rank
		self.column = len(maps[0]) #column
		self.lengthCollection = []
	
	def findMinLenToTreasure(self):
		passedPosition = ()
		startPosition = [0,0]
		self.collectPathLen(startPosition, passedPosition)
		if len(self.lengthCollection) == 0:
			return -1
		else:
			return min(self.lengthCollection)
		#print("Shortest Length : %d" %(answer))
	
	def collectPathLen(self, Pos, passedPosition):
		if Pos in passedPosition:
			return # error
		if self.isTreasure(Pos):
			length = len(passedPosition)
			lc = self.lengthCollection
			lc.append(length)
			return # found treasure!
		if Pos[0] > 0 and self.maps[Pos[0]-1][Pos[1]] != 0: # move down
			self.collectPathLen([Pos[0]-1,Pos[1]],passedPosition + (Pos,))
		if Pos[1] > 0 and self.maps[Pos[0]][Pos[1]-1] != 0: # move left
			self.collectPathLen([Pos[0],Pos[1]-1],passedPosition + (Pos,))
		if Pos[0] < self.rank-1 and self.maps[Pos[0]+1][Pos[1]] != 0: # move up:
			self.collectPathLen([Pos[0]+1,Pos[1]],passedPosition + (Pos,))
		if Pos[1] < self.column-1 and self.maps[Pos[0]][Pos[1]+1] != 0: # move right 
                	self.collectPathLen([Pos[0],Pos[1]+1],passedPosition + (Pos,))
		return #function end
	
	def isTreasure(self, Position):
		return self.maps[Position[0]][Position[1]] == 2
	 
