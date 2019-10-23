class problemSolver:

	def __init__(self, maps):
		self.maps = maps
		self.xLimit = len(maps) #rank
		self.yLimit = len(maps[0]) #column
	
	def findMinLenToTreasure(self):
		answer = 0
		node = ()
		disSet = []
		Pos = [0,0]
		dis = 0
		self.findDistance(Pos, Pos, node, disSet, dis)
		if len(disSet) == 0:
			answer = -1
		else:
			answer = min(disSet)
		#print("Shortest Length : %d" %(answer))
		return answer
	
	def findDistance(self, prevPos, Pos, node, disSet, distance):
        	if Pos in node:
            		return # error
        	if self.maps[Pos[0]][Pos[1]] == 2:
            		disSet.append(distance)
            		return # found treasure!
        	if Pos[0] > 0 and self.maps[Pos[0]-1][Pos[1]] != 0 and prevPos[0] != Pos[0]-1: # move down
            		if self.isCrossRoad(Pos):
                		self.findDistance(Pos,[Pos[0]-1,Pos[1]],node + (Pos,),disSet,distance+1)
            		else:
                		self.findDistance(Pos,[Pos[0]-1,Pos[1]],node,disSet,distance+1)
        	if Pos[1] > 0 and self.maps[Pos[0]][Pos[1]-1] != 0 and prevPos[1] != Pos[1]-1: # move left
            		if self.isCrossRoad(Pos):
                		self.findDistance(Pos,[Pos[0],Pos[1]-1],node + (Pos,),disSet,distance+1)
            		else:
                		self.findDistance(Pos,[Pos[0],Pos[1]-1],node,disSet,distance+1)
       		if Pos[0] < self.rank-1 and self.maps[Pos[0]+1][Pos[1]] != 0 and prevPos[0] != Pos[0] + 1: # move up
	       		if self.isCrossRoad(Pos):
			       	self.findDistance(Pos,[Pos[0]+1,Pos[1]],node + (Pos,),disSet,distance+1)
	       		else:
			        self.findDistance(Pos,[Pos[0]+1,Pos[1]],node,disSet,distance+1)
	        if Pos[1] < self.column-1 and self.maps[Pos[0]][Pos[1]+1] != 0 and prevPos[1] != Pos[1] + 1: # move right 
		        if self.isCrossRoad(Pos):
                		self.findDistance(Pos,[Pos[0],Pos[1]+1],node + (Pos,),disSet,distance+1)
		        else:
                		self.findDistance(Pos,[Pos[0],Pos[1]+1],node,disSet,distance+1)
	        return #function end

	def isCrossRoad(self, curPos):
        	count = 0
        	if curPos[0] > 0 and self.maps[curPos[0]-1][curPos[1]] == 1:
            		count += 1
        	if curPos[1] > 0 and self.maps[curPos[0]][curPos[1]-1] == 1:
            		count += 1
        	if curPos[0] < self.rank-1 and self.maps[curPos[0]+1][curPos[1]] == 1:
            		count += 1
        	if curPos[1] < self.column-1 and self.maps[curPos[0]][curPos[1]+1] == 1:
            		count += 1
        	if count == 2:
            		return False
        	return True

class 
