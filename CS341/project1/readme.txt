Student ID: 20160788
Name: InJe Hwang

<client.c>
My client program checks invalid input arguments, rigidly. To implement it, I had to do many dirty jobs, so the code is not neat. 
My client program may use huge memory to transmit, because the program collects all the data before sending it to server

<server.c>
There are two funcitons in my server program, "formchk" and "respond" funciton. "formchk" fuction checks any error in inputs, and is used in "respond" function. "respond" function controls the respond of the require of clients(make file, give file, give message, etc.).
To implement non-blocking program, I put additional "select" function in "respond" function. More precisely, I wrote "select" function where much time may be needed to complete the task of client.
I also did rigid test of input arguments, so my code is not neat.
My server program get portnumber only in brackets([]). For example, ./server -p [11111], then the server program knows 11111 is the portnumber.
