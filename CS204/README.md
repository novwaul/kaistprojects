Programming Problems of Discrete Mathematics
============================================

PA1
---
James and his friends developed the game “Treasure Hunter” and they are now creating a hint system to get the users to see the ad. They decide to give only the shortest distance as a hint, but they have not yet implemented. So the developer James is looking for an algorithm to solve the problem.

* Hint system: Find the shortest distance to treasure in the map.
* INPUT: Map.
* OUTPUT: The shortest distance from start position to treasure.
* Restriction:
1. Start position : (1, 1)
2. m, n : 1~10 (except m=n=1)
3. 0: Wall, 1: Passage, 2: Treasure
4. You can go up, down, left, right directions.
5. If there is no path to treasure, return -1.

* Example) 5x5 maps
![PA1](https://user-images.githubusercontent.com/53179332/63480423-c6408380-c4cc-11e9-8c40-fe0245788194.png)

PA2
---
The each vertex of below graph G has index from ‘0’ to ‘9’ and letter from ‘A’ to ‘E’. When the sequence of string is given, you have to determine whether it can be satisfied by passing through connected edges. (Each vertex can be used several times.)
For instance, sequence of string ‘BDDEA’ can be satisfied by vertex path [6,8,3,4,0]. If there is vertex path, print out the order of the characters given. If there is no path, the output will be -1. (The input string will have length more than 1)
Implement python program to determine if there is possible vertex path.

![PA2](https://user-images.githubusercontent.com/53179332/63480534-29321a80-c4cd-11e9-97ff-c0788f545803.png)






         
