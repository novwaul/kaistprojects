#############################################
#  CS204C Fall 2018, Homework 5, Problem 4  #
#  TA : Youngjin An, qwer6019@kaist.ac.kr   #
#############################################

# Example sequence of string : 'BDDEA'
S = "BDDEA"
# Vertex Path : [6,8,3,4,0]

Sol = []
Input = list(S)

def FindVertexPath(solution, input):

    # Write down your code here
    # New function or any change is Ok.
    # Don't forget each vertex can be used several times.
    # Input string will be more than 1.

    if(len(input) == 0): #check input length
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


def Main():

    if(FindVertexPath(Sol, Input)):
        print(Sol)
    else:
        print(-1)


Main()
