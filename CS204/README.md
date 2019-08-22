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
\Users\123dl\Desktop\PA1.png

PA2
---
The each vertex of below graph G has index from ‘0’ to ‘9’ and letter from ‘A’ to ‘E’. When the sequence of string is given, you have to determine whether it can be satisfied by passing through connected edges. (Each vertex can be used several times.)
For instance, sequence of string ‘BDDEA’ can be satisfied by vertex path [6,8,3,4,0]. If there is vertex path, print out the order of the characters given. If there is no path, the output will be -1. (The input string will have length more than 1)
Implement python program to determine if there is possible vertex path.



                __________________A (0)__________________
               /                    |                    \
              /                     |                     \
             /                      |                      \ 
            /                       |                       \
           /                      A (4)                      \
          /                      /    \                       \
         /                      /      \                       \
	E (4)-------- E (9) ---/--------\--- B (6) ------------B (1)
 	 \		     \/          \/                    /
          \                  /\          /\                   /
           \                /  \        /  \                 /
            \	           /    \______/__ C (7)            /
             \           D (8)_______ /      |	           /
              \           |                  |            /
               \          |                  |	         /
                \________D (3) ----------- C (2)________/






         
