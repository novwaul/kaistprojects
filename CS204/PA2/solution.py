class vertex:
    def __init__(self, id, oppositeSideId, neighborsId):
        self.id = id
        self.oppositeSideId = oppositeSideId
        self.neighborsId = neighborsId
    
    def __eq__(self, other):
        return self.id == other.id and self.oppositeSideId == other.oppositeSideId and self.neighborsId == other.neighborsId

    def getId(self):
        return self.id

    def getOppositeSideId(self):
        return self.oppositeSideId

    def getNeighborsId(self):
        return self.neighborsId

class vertexConnection:
    #CAUTION: defaultVertexNum must be even.
    def __init__ (self, defaultVertexName = 'A', defaultVertexId = 0, defaultVertexNum = 10):
        self.defaultVertexName = defaultVertexName
        self.defaultVertexNum = defaultVertexNum
        self.defaultOuterVertexId = defaultVertexId
        self.defaultInnerVertexId = self.calculateDefaultInnerVertexId()

    def calculateDefaultInnerVertexId(self):
        return (self.defaultVertexNum + self.defaultOuterVertexId) // 2

    def calculateDefaultInnerVertexNeighborsId(self):
        neighborIdWithSmallerOne = self.getMiddleInnerVertexId()
        neighborIdwithBiggerOne = self.getNextId(neighborIdWithSmallerOne)
        return [neighborIdWithSmallerOne, neighborIdwithBiggerOne]

    def calculateDefaultOuterVertexNeighborsId(self):
        neighborIdWithSmallerOne = self.getLastOuterVertexId()
        neighborIdwithBiggerOne = self.getNextId(self.defaultOuterVertexId)
        return [neighborIdWithSmallerOne, neighborIdwithBiggerOne]

    def addInnerVertexOffsetWithModOperation(self, value, vertexOffset):
        innerVertexNum = self.defaultVertexNum // 2
        return innerVertexNum + (value + vertexOffset) % innerVertexNum

    def addOuterVertexOffsetWithModOperation(self, value, vertexOffset):
        outerVertexNum = self.defaultVertexNum // 2
        return (value + vertexOffset) % outerVertexNum

    def calculateVertexOffsetWithBoundCheck(self, vertexName):
        offset = self.calculateVertexOffset(vertexName)
        offsetBound = self.calculateBound()
        if abs(offset) < offsetBound:
            return offset
        else:
            raise NoEntryException()
    
    def calculateVertexOffset(self, vertexName):
        return ord(vertexName) - ord(self.defaultVertexName)

    def calculateBound(self):
        return self.defaultVertexNum // 2

    def calculateOppositeSideId(self, id):
        return (id + self.defaultVertexNum // 2) % self.defaultVertexNum

    def getNextId(self, id):
        return id + 1

    def getMiddleInnerVertexId(self):
        return self.defaultInnerVertexId + self.defaultVertexNum // 4

    def getLastOuterVertexId(self):
        return self.defaultOuterVertexId + self.defaultVertexNum // 2 - 1

class innerVertexFactory(vertexConnection): 
    def makeInnerVertex(self, vertexName):
        id = self.calculateInnerVertexId(vertexName)
        oppositeSideId = self.calculateOppositeSideId(id)
        neighborsId = self.calculateInnerVertexNeighborsId(vertexName)
        return vertex(id, oppositeSideId, neighborsId)

    def calculateInnerVertexId(self, vertexName):
        vertexOffset = self.calculateVertexOffsetWithBoundCheck(vertexName)
        return self.defaultInnerVertexId + vertexOffset

    def calculateInnerVertexNeighborsId(self, vertexName):
        defaultInnerVertexNeighborsId = self.calculateDefaultInnerVertexNeighborsId()
        vertexOffset = self.calculateVertexOffsetWithBoundCheck(vertexName)
        return self.addInnerVertexOffsetToAllEntry(vertexOffset, defaultInnerVertexNeighborsId)

    def addInnerVertexOffsetToAllEntry(self, vertexOffset, defaultInnerVertexNeighborsId):
        newInnerVertexNeighborsId = []
        for e in defaultInnerVertexNeighborsId:
            result = self.addInnerVertexOffsetWithModOperation(e, vertexOffset)
            newInnerVertexNeighborsId.append(result)
        return newInnerVertexNeighborsId

class outerVertexFactory(vertexConnection):
    def makeOuterVertex(self, vertexName):
        id = self.calculateOuterVertexId(vertexName)
        oppositeSideId = self.calculateOppositeSideId(id)
        neighborsId = self.calculateOuterVertexNeighborsId(vertexName)
        return vertex(id, oppositeSideId, neighborsId)
    
    def calculateOuterVertexId(self, vertexName):
        vertexOffset = self.calculateVertexOffsetWithBoundCheck(vertexName)
        return self.defaultOuterVertexId + vertexOffset

    def calculateOuterVertexNeighborsId(self, vertexName):
        defaultOuterVertexNeighborsId = self.calculateDefaultOuterVertexNeighborsId()
        vertexOffset = self.calculateVertexOffsetWithBoundCheck(vertexName)
        return self.addOuterVertexOffsetToAllEntry(vertexOffset, defaultOuterVertexNeighborsId)

    def addOuterVertexOffsetToAllEntry(self, vertexOffset, defaultOuterVertexNeighborsId):
        newOuterVertexNeighborsId = []
        for e in defaultOuterVertexNeighborsId:
            result = self.addOuterVertexOffsetWithModOperation(e, vertexOffset)
            newOuterVertexNeighborsId.append(result)
        return newOuterVertexNeighborsId
    
class NoEntryException(Exception):
    def __init__(self, errorValue = -1):
        self.errorValue = errorValue
    
    def getErrorValue(self):
        return self.errorValue

class pathFinder:
    def __init__(self):
        self.IVFactory = innerVertexFactory()
        self.OVFactory = outerVertexFactory()
        self.solution = []

    def Main(self, S):
        self.stringPath = S
        try:
            if(self.findVertex(0)):
                return self.solution
            else:
                return -1
        except NoEntryException:
            return -1

    def findVertex(self, currentIndex):
        if self.isStartOfIndex(currentIndex):
            vertexName = self.stringPath[currentIndex]
            innerVertex = self.IVFactory.makeInnerVertex(vertexName)
            outerVertex = self.OVFactory.makeOuterVertex(vertexName)
            solution = self.solution
            nextIndex = currentIndex + 1
            solution.append(innerVertex.getId())
            if(False == self.findVertex(nextIndex)):
                solution.pop()
                solution.append(outerVertex.getId())
                if (False == self.findVertex(nextIndex)):
                    solution.pop()
                    return False
                else: 
                    return True
            else: 
                return True
        elif self.isEndOfIndex(currentIndex):
            return True
        else:
            vertexName = self.stringPath[currentIndex]
            innerVertex = self.IVFactory.makeInnerVertex(vertexName)
            outerVertex = self.OVFactory.makeOuterVertex(vertexName)
            solution = self.solution
            nextIndex = currentIndex + 1
            if self.isPrevVertexNextToCurrVertex(innerVertex):
                solution.append(innerVertex.getId())
                if(False == self.findVertex(nextIndex)):
                    solution.pop()
                    return False
                else: 
                    return True
            elif self.isPrevVertexNextToCurrVertex(outerVertex):
                solution.append(outerVertex.getId())
                if(False == self.findVertex(nextIndex)):
                    solution.pop()
                    return False
                else: 
                    return True
            else:
                return False
    
    def isStartOfIndex(self, currentIndex):
        return currentIndex == 0

    def isEndOfIndex(self, currentIndex):
        return len(self.stringPath) == currentIndex

    def isPrevVertexNextToCurrVertex(self, currVertex):
        prevVertexId = self.solution[-1]
        return prevVertexId in currVertex.getNeighborsId() or prevVertexId == currVertex.getOppositeSideId()
    


