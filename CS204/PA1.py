import copy
from random import *
#CS204C Fall 2018, Homework 3, Problem 4

#"Treusure Hunter" Hint system

#Example maps
map1 = [[1,0,1,1,1],[1,0,1,0,1],[1,0,1,1,1],[1,1,1,0,1],[0,0,0,0,2]]
map2 = [[1,0,1,1,1],[1,0,1,0,1],[1,0,1,1,1],[1,1,1,0,0],[0,0,0,0,2]]
map3 = [[1,1,1,1,1],[0,0,0,0,1],[1,1,1,1,1],[1,0,0,0,0],[1,1,1,1,2]]
map4 = [[1,1,1,1,1],[1,0,1,0,1],[1,1,2,1,1],[1,0,1,0,1],[1,1,1,1,1]]
map5 = [[1,1,1,1,1,1,1,1,1],[0,0,0,0,0,0,0,0,1],[1,1,1,1,1,1,1,0,1],[1,0,0,0,0,0,1,0,1],[1,0,1,1,2,0,1,0,1],[1,0,1,0,0,0,1,0,1],[1,0,1,1,1,1,1,0,1],[1,0,0,0,0,0,0,0,1], [1,1,1,1,1,1,1,1,1]]

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
    #maps[randint(0,m-1)][randint(0,n-1)] = 2

    #2. Fixed Tresure
    maps[m-1][n-1] = 2

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


def solution(maps):
    answer = 0

    #----------You should write here-------------
    #--------Make other function is OK-----------
    #--------------------------------------------
    def isNode(curPos, rank, column, maps):
        count = 0
        if curPos[0] > 0 and maps[curPos[0]-1][curPos[1]] == 1:
            count += 1
        if curPos[1] > 0 and maps[curPos[0]][curPos[1]-1] == 1:
            count += 1
        if curPos[0] < rank-1 and maps[curPos[0]+1][curPos[1]] == 1:
            count += 1
        if curPos[1] < column-1 and maps[curPos[0]][curPos[1]+1] == 1:
            count += 1
        if count == 2:
            return 0
        return 1
    def findDistance(prevPos,Pos, rank, column, maps, node, disSet, distance):
        if Pos in node:
            return # error
        if maps[Pos[0]][Pos[1]] == 2:
            disSet.append(distance)
            return # found treasure!
        if Pos[0] > 0 and maps[Pos[0]-1][Pos[1]] != 0 and prevPos[0] != Pos[0]-1: # move down
            if isNode(Pos,rank,column,maps):
                findDistance(Pos,[Pos[0]-1,Pos[1]],rank,column,maps,node + (Pos,),disSet,distance+1)
            else:
                findDistance(Pos,[Pos[0]-1,Pos[1]],rank,column,maps,node,disSet,distance+1)
        if Pos[1] > 0 and maps[Pos[0]][Pos[1]-1] != 0 and prevPos[1] != Pos[1]-1: # move left
            if isNode(Pos,rank,column,maps):
                findDistance(Pos,[Pos[0],Pos[1]-1],rank,column,maps,node + (Pos,),disSet,distance+1)
            else:
                findDistance(Pos,[Pos[0],Pos[1]-1],rank,column,maps,node,disSet,distance+1)
        if Pos[0] < rank-1 and maps[Pos[0]+1][Pos[1]] != 0 and prevPos[0] != Pos[0] + 1: # move up
            if isNode(Pos,rank,column,maps):
                findDistance(Pos,[Pos[0]+1,Pos[1]],rank,column,maps,node + (Pos,),disSet,distance+1)
            else:
                findDistance(Pos,[Pos[0]+1,Pos[1]],rank,column,maps,node,disSet,distance+1)
        if Pos[1] < column-1 and maps[Pos[0]][Pos[1]+1] != 0 and prevPos[1] != Pos[1] + 1: # move right
            if isNode(Pos,rank,column,maps):
                findDistance(Pos,[Pos[0],Pos[1]+1],rank,column,maps,node + (Pos,),disSet,distance+1)
            else:
                findDistance(Pos,[Pos[0],Pos[1]+1],rank,column,maps,node,disSet,distance+1)
        return #function end

    rank = len(maps)
    column = len(maps[0])
    node = ()
    disSet = []
    Pos = [0,0]
    dis = 0
    findDistance(Pos,Pos,rank,column,maps,node,disSet,dis)
    if len(disSet) == 0:
        answer = -1
    else:
        answer = min(disSet)

    #--------------------------------------------

    print("Shortest Length : %d" %(answer))
    return answer

#maps = Generate_maps(10,10)
maps = map5
Display_maps(maps)
solution(maps)
