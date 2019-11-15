#TODO use OOP to reduce duplication

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


class vertexFactory:
    #CAUTION: defaultVertexNum must be even.
    def __init__ (self, defaultVertexName = 'A', defaultVertexId = 0, defaultVertexNum = 10):
        self.defaultVertexName = defaultVertexName
        self.defaultVertexNum = defaultVertexNum
        self.defaultOuterVertexId = defaultVertexId
        self.defaultInnerVertexId = self.calculateDefaultInnerVertexId()
    
    def calculateDefaultInnerVertexId(self):
        return (self.defaultVertexNum + self.defaultOuterVertexId) // 2

    def makeInnerVertex(self, vertexName):
        id = self.calculateInnerVertexId(vertexName)
        neighborsId = self.calculateInnerVertexNeighborsId(vertexName)
        oppositeSideId = self.calculateOuterVertexId(vertexName)
        return vertex(id, oppositeSideId, neighborsId)

    def calculateInnerVertexId(self, vertexName):
        vertexOffset = self.calculateVertexOffset(vertexName)
        return self.defaultInnerVertexId + vertexOffset

    def calculateVertexOffset(self, vertexName):
        return ord(vertexName) - ord(self.defaultVertexName)

    def calculateInnerVertexNeighborsId(self, vertexName):
        defaultInnerVertexNeighborsId = self.calculateDefaultInnerVertexNeighborsId()
        vertexOffset = self.calculateVertexOffset(vertexName)
        newInnerVertexNeighborsId = []
        for e in defaultInnerVertexNeighborsId:
            result = self.addInnerVertexOffsetWithModOperation(e, vertexOffset)
            newInnerVertexNeighborsId.append(result)
        return newInnerVertexNeighborsId

    def calculateDefaultInnerVertexNeighborsId(self):
        neighborsId = []    
        neighborIdWithSmallerOne = self.getMiddleInnerVertexId()
        neighborIdwithBiggerOne = self.getNextId(neighborIdWithSmallerOne)
        neighborsId.append(neighborIdWithSmallerOne)
        neighborsId.append(neighborIdwithBiggerOne)
        return neighborsId

    def getMiddleInnerVertexId(self):
        return self.defaultInnerVertexId + self.defaultVertexNum // 4

    def getNextId(self, id):
        return id + 1

    def addInnerVertexOffsetWithModOperation(self, value, vertexOffset):
        innerVertexNum = self.defaultVertexNum // 2
        return innerVertexNum + (value + vertexOffset) % innerVertexNum

    def makeOuterVertex(self, vertexName):
        id = self.calculateOuterVertexId(vertexName)
        neighborsId = self.calculateOuterVertexNeighborsId(vertexName)
        oppositeSideId = self.calculateInnerVertexId(vertexName)
        return vertex(id, oppositeSideId, neighborsId)

    def calculateOuterVertexId(self, vertexName):
        vertexOffset = self.calculateVertexOffset(vertexName)
        return self.defaultOuterVertexId + vertexOffset
    
    def calculateOuterVertexNeighborsId(self, vertexName):
        defaultOuterVertexNeighborsId = self.calculateDefaultOuterVertexNeighborsId()
        vertexOffset = self.calculateVertexOffset(vertexName)
        newOuterVertexNeighborsId = []
        for e in defaultOuterVertexNeighborsId:
            result = self.addOuterVertexOffsetWithModOperation(e, vertexOffset)
            newOuterVertexNeighborsId.append(result)
        return newOuterVertexNeighborsId

    def calculateDefaultOuterVertexNeighborsId(self):
        neighborsId = []    
        neighborIdWithSmallerOne = self.getLastOuterVertexId()
        neighborIdwithBiggerOne = self.getNextId(self.defaultOuterVertexId)
        neighborsId.append(neighborIdWithSmallerOne)
        neighborsId.append(neighborIdwithBiggerOne)
        return neighborsId

    def getLastOuterVertexId(self):
        return self.defaultOuterVertexId + self.defaultVertexNum // 2 - 1

    def addOuterVertexOffsetWithModOperation(self, value, vertexOffset):
        outerVertexNum = self.defaultVertexNum // 2
        return (value + vertexOffset) % outerVertexNum

#TODO
def FindVertexPath(solution, input):
    if(len(input) == 0):
        return True
    else:
        vertex = input.pop(0)

    if(vertex == 'A'):
        if(len(solution) == 0):
            solution.append(0)
            if(False == FindVertexPath(solution, input)):
                solution.pop()
                solution.append(5)
                if(False == FindVertexPath(solution, input)):
                    input.insert(0,'A')
                    solution.pop()
                    return False
                else: return True
            else: return True
        else:
            last_vertex = solution[-1]
            if(last_vertex == 1 or last_vertex == 5 or last_vertex == 4):
                solution.append(0)
                if(False == FindVertexPath(solution, input)):
                    input.insert(0,'A')
                    solution.pop()
                    return False
                else: return True
            elif(last_vertex == 0 or last_vertex == 7 or last_vertex == 8):
                solution.append(5)
                if(False == FindVertexPath(solution, input)):
                    input.insert(0,'A')
                    solution.pop()
                    return False
                else: return True
            else:
                input.insert(0,'A')
                return False
    elif(vertex == 'B'):
        if(len(solution) == 0):
            solution.append(1)
            if(False == FindVertexPath(solution, input)):
                solution.pop()
                solution.append(6)
                if(False == FindVertexPath(solution, input)):
                    solution.pop()
                    input.insert(0,'B')
                    return False
                else: return True
            else: return True
        else:
            last_vertex = solution[-1]
            if(last_vertex == 0 or last_vertex == 2 or last_vertex == 6):
                solution.append(1)
                if(False == FindVertexPath(solution, input)):
                    solution.pop()
                    input.insert(0,'B')
                    return False
                else: return True
            elif(last_vertex == 1 or last_vertex == 8 or last_vertex == 9):
                solution.append(6)
                if(False == FindVertexPath(solution, input)):
                    solution.pop()
                    input.insert(0,'B')
                    return False
                else: return True
            else:
                input.insert(0,'B')
                return False
    elif(vertex == 'C'):
        if(len(solution) == 0):
            solution.append(2)
            if(False == FindVertexPath(solution, input)):
                solution.pop()
                solution.append(7)
                if(False == FindVertexPath(solution, input)):
                    solution.pop()
                    input.insert(0,'C')
                    return False
                else: return True
            else: return True
        else:
            last_vertex = solution[-1]
            if(last_vertex == 1 or last_vertex == 3 or last_vertex == 7):
                solution.append(2)
                if(False == FindVertexPath(solution, input)):
                    solution.pop()
                    input.insert(0,'C')
                    return False
                else: return True
            elif(last_vertex == 2 or last_vertex == 5 or last_vertex == 9):
                solution.append(7)
                if(False == FindVertexPath(solution, input)):
                    solution.pop()
                    input.insert(0,'C')
                    return False
                else: return True
            else:
                input.insert(0,'C')
                return False
    elif(vertex == 'D'):
        if(len(solution) == 0):
            solution.append(3)
            if(False == FindVertexPath(solution, input)):
                solution.pop()
                solution.append(8)
                if(False == FindVertexPath(solution, input)):
                    solution.pop()
                    input.insert(0,'D')
                    return False
                else: return True
            else: return True
        else:
            last_vertex = solution[-1]
            if(last_vertex == 2 or last_vertex == 4 or last_vertex == 8):
                solution.append(3)
                if(False == FindVertexPath(solution, input)):
                    solution.pop()
                    input.insert(0,'D')
                    return False
                else: return True
            elif(last_vertex == 3 or last_vertex == 5 or last_vertex == 6):
                solution.append(8)
                if(False == FindVertexPath(solution, input)):
                    solution.pop()
                    input.insert(0,'D')
                    return False
                else: return True
            else:
                input.insert(0,'D')
                return False
    elif(vertex == 'E'):
        if(len(solution) == 0):
            solution.append(4)
            if(False == FindVertexPath(solution, input)):
                solution.pop()
                solution.append(9)
                if(False == FindVertexPath(solution, input)):
                    solution.pop()
                    input.insert(0,'E')
                    return False
                else: return True
            else: return True
        else:
            last_vertex = solution[-1]
            if(last_vertex == 0 or last_vertex == 3 or last_vertex == 9):
                solution.append(4)
                if(False == FindVertexPath(solution, input)):
                    solution.pop()
                    input.insert(0,'E')
                    return False
                else: return True
            elif(last_vertex == 4 or last_vertex == 6 or last_vertex == 7):
                solution.append(9)
                if(False == FindVertexPath(solution, input)):
                    solution.pop()
                    input.insert(0,'E')
                    return False
                else: return True
            else:
                input.insert(0,'E')
                return False
    else:
        input.insert(0,vertex)
        return False

def Main(S):
    Sol = []
    Input = list(S)
    if(FindVertexPath(Sol, Input)):
        return Sol
    else:
        return -1

