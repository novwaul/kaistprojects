from enum import Enum

class vertex:
    def __init__(self, id, neighborIds):
        self.id = id
        self.neighborIds = neighborIds

    def getId(self):
        return self.id
    
    def getNeighborIds(self):
        return self.neighborIds
        
class vertexId(Enum):
    outerA = 0
    outerB = 1
    outerC = 2
    outerD = 3
    outerE = 4
    innerA = 5
    innerB = 6
    innerC = 7
    innerD = 8
    innerE = 9

#TODO chenge magic number to known name
class vertexFactory(vertexId):
    def makeInnerAndOuterVertex(self, name):
        if (name == 'A')
            return [vertex(innerA, [outerA,innerD,innerC]), vertex(outerA, [outerB,outerE,innerA])]
        elif (name == 'B')
            return [vertex(innerB, [outerB,innerD,innerE]), vertex(outerB, [outerA,outerC,innerB])]
        elif (name == 'C')
            return [vertex(innerC, [outerC,innerA,innerE]), vertex(outerC, [,3,7])]
        elif (name == 'D')
            return [vertex(innerD, [3,5,6]), vertex(outerD, [2,4,8])]
        elif (name == 'E')
            return [vertex(innerE, [4,6,7]), vertex(outerE, [0,3,9])]

#TODO use OOP to reduce duplication
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
