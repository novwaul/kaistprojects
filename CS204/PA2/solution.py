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
        self.defaultInnerVertexId = (self.defaultVertexNum + self.defaultOuterVertexId) // 2
        
    def getDefaultInnerVertexNeighborsId(self):
        neighborIdWithSmallerOne = self.getMiddleInnerVertexId()
        neighborIdwithBiggerOne = self.getNextId(neighborIdWithSmallerOne)
        return [neighborIdWithSmallerOne, neighborIdwithBiggerOne]

    def getDefaultOuterVertexNeighborsId(self):
        neighborIdWithSmallerOne = self.getLastOuterVertexId()
        neighborIdwithBiggerOne = self.getNextId(self.defaultOuterVertexId)
        return [neighborIdWithSmallerOne, neighborIdwithBiggerOne]

    def getNextId(self, id):
        return (id + 1) % self.defaultVertexNum

    def getMiddleInnerVertexId(self):
        return self.defaultInnerVertexId + self.defaultVertexNum // 4

    def getLastOuterVertexId(self):
        return self.defaultOuterVertexId + self.defaultVertexNum // 2 - 1

    def getVertexOffsetWithBoundCheck(self, vertexName):
        offset = self.getVertexOffset(vertexName)
        offsetBound = self.getBound()
        if abs(offset) < offsetBound:
            return offset
        else:
            raise NoEntryException()

    def getBound(self):
        return self.defaultVertexNum // 2

    def getVertexOffset(self, vertexName):
        return ord(vertexName) - ord(self.defaultVertexName)

    def getOppositeSideId(self, id):
        return (id + self.defaultVertexNum // 2) % self.defaultVertexNum

class innerVertexFactory(vertexConnection): 
    def makeInnerVertex(self, vertexName):
        id = self.getInnerVertexId(vertexName)
        oppositeSideId = self.getOppositeSideId(id)
        neighborsId = self.getInnerVertexNeighborsId(vertexName)
        return vertex(id, oppositeSideId, neighborsId)

    def getInnerVertexId(self, vertexName):
        vertexOffset = self.getVertexOffsetWithBoundCheck(vertexName)
        return self.defaultInnerVertexId + vertexOffset

    def getInnerVertexNeighborsId(self, vertexName):
        defaultInnerVertexNeighborsId = self.getDefaultInnerVertexNeighborsId()
        vertexOffset = self.getVertexOffsetWithBoundCheck(vertexName)
        return self.addInnerVertexOffsetToAllEntry(vertexOffset, defaultInnerVertexNeighborsId)

    def addInnerVertexOffsetToAllEntry(self, vertexOffset, defaultInnerVertexNeighborsId):
        newInnerVertexNeighborsId = []
        for e in defaultInnerVertexNeighborsId:
            result = self.addInnerVertexOffsetWithModOperation(e, vertexOffset)
            newInnerVertexNeighborsId.append(result)
        return newInnerVertexNeighborsId

    def addInnerVertexOffsetWithModOperation(self, value, vertexOffset):
        innerVertexNum = self.defaultVertexNum // 2
        return innerVertexNum + (value + vertexOffset) % innerVertexNum

class outerVertexFactory(vertexConnection):
    def makeOuterVertex(self, vertexName):
        id = self.getOuterVertexId(vertexName)
        oppositeSideId = self.getOppositeSideId(id)
        neighborsId = self.getOuterVertexNeighborsId(vertexName)
        return vertex(id, oppositeSideId, neighborsId)
    
    def getOuterVertexId(self, vertexName):
        vertexOffset = self.getVertexOffsetWithBoundCheck(vertexName)
        return self.defaultOuterVertexId + vertexOffset

    def getOuterVertexNeighborsId(self, vertexName):
        defaultOuterVertexNeighborsId = self.getDefaultOuterVertexNeighborsId()
        vertexOffset = self.getVertexOffsetWithBoundCheck(vertexName)
        return self.addOuterVertexOffsetToAllEntry(vertexOffset, defaultOuterVertexNeighborsId)

    def addOuterVertexOffsetToAllEntry(self, vertexOffset, defaultOuterVertexNeighborsId):
        newOuterVertexNeighborsId = []
        for e in defaultOuterVertexNeighborsId:
            result = self.addOuterVertexOffsetWithModOperation(e, vertexOffset)
            newOuterVertexNeighborsId.append(result)
        return newOuterVertexNeighborsId

    def addOuterVertexOffsetWithModOperation(self, value, vertexOffset):
        outerVertexNum = self.defaultVertexNum // 2
        return (value + vertexOffset) % outerVertexNum
    
class NoEntryException(Exception):
    def __init__(self, errorValue = -1):
        self.errorValue = errorValue
    
    def getErrorValue(self):
        return self.errorValue

class pathFinder:
    def checkPathExist(self, stringPath):
        self.IVFactory = innerVertexFactory()
        self.OVFactory = outerVertexFactory()
        self.stringPath = stringPath
        self.possibleIdPaths = []
        return self.findPath()

    def findPath(self):
        try:
            return self.findOnePossiblePath()
        except NoEntryException:
            return -1

    def findOnePossiblePath(self):
        self.findOnePossiblePathByFindVerticies()
        if len(self.possibleIdPaths) == 0:
            return -1
        else:
            return self.possibleIdPaths[0]

    def findOnePossiblePathByFindVerticies(self):
        foundVertexNum = 0
        foundVerticies = []
        self.findVerticies(foundVertexNum, foundVerticies)

    def findVerticies(self, foundVertexNum, foundVerticies):
        if self.isFirstVertexToFind(foundVertexNum):
            self.findVerticiesWithInnerAndOuterVertex(foundVertexNum, foundVerticies)
            return
        elif self.areAllVerticesFound(foundVertexNum):
            self.addToPossibleIdPaths(foundVerticies)
            return
        else:
            self.findVerticiesWithNextToCheck(foundVertexNum, foundVerticies)
            return
    
    def isFirstVertexToFind(self, foundVertexNum):
        return foundVertexNum == 0

    def findVerticiesWithInnerAndOuterVertex(self, foundVertexNum, foundVerticies):
        vertexName = self.stringPath[foundVertexNum]
        innerVertex = self.IVFactory.makeInnerVertex(vertexName)
        outerVertex = self.OVFactory.makeOuterVertex(vertexName)
        self.findVerticies(foundVertexNum + 1, foundVerticies + [innerVertex.getId()])
        self.findVerticies(foundVertexNum + 1, foundVerticies + [outerVertex.getId()])

    def areAllVerticesFound(self, foundVertexNum):
        return len(self.stringPath) == foundVertexNum

    def addToPossibleIdPaths(self, foundVerticies):
        self.possibleIdPaths.append(foundVerticies)

    def findVerticiesWithNextToCheck(self, foundVertexNum, foundVerticies):
        vertexName = self.stringPath[foundVertexNum]
        innerVertex = self.IVFactory.makeInnerVertex(vertexName)
        outerVertex = self.OVFactory.makeOuterVertex(vertexName)
        if self.isCurrVertexNextToPrevVertex(innerVertex, foundVerticies):
            self.findVerticies(foundVertexNum + 1, foundVerticies + [innerVertex.getId()])
        elif self.isCurrVertexNextToPrevVertex(outerVertex, foundVerticies):
            self.findVerticies(foundVertexNum + 1, foundVerticies + [outerVertex.getId()])
        else:
            return

    def isCurrVertexNextToPrevVertex(self, currVertex, foundVerticies):
        prevVertexId = foundVerticies[-1]
        return prevVertexId in currVertex.getNeighborsId() or prevVertexId == currVertex.getOppositeSideId()
    
    
    


