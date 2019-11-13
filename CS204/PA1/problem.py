import copy
from random import *
from solution import problemSolver

#CS204C Fall 2018, Homework 3, Problem 4
#"Treusure Hunter" Hint system

#m : 1~10, n : 1~10 (except m=n=1)
def Generate_maps(m,n):
    #0 : Wall, 1: Passage, 2: Target
    maps = []
    for i in range(0, m):
        row = []
        for j in range(0, n):
            row.append(0);
        maps.append(row)

    #Generate random maps
    S = [0, 0, 1, 1, 1] #60% : passage
    for i in range(0, m):
        for j in range(0, n):
                maps[i][j] = choice(S)

    #Start Position (0,0)
    maps[0][0] = 1

    #Tresure Position (1. random or 2. [m,n])

    #1. Random Tresure
    maps[randint(0,m-1)][randint(0,n-1)] = 2

    #2. Fixed Tresure
    #maps[m-1][n-1] = 2

    return maps

def Display_maps(matrix):
    print("")
    print("----------------------------------------------------")
    print("Tresure Map - only for developer")
    print("----------------------------------------------------")
    print("")
    for i in range(0,len(matrix)):
        for j in range(0,len(matrix[0])):
            print(matrix[i][j], end=' ')
        print("")

    print("")
    print("----------------------------------------------------")
    print("0 : Wall, 1: Passage, 2: Target")
    print("----------------------------------------------------")
    print("")

maps = Generate_maps(10,10)
minLenProblemSolver = problemSolver()
Display_maps(maps)
answer = minLenProblemSolver.giveSolution(maps)
print ("Shortest Length: ", answer)
