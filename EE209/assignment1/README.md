# KAIST EE 209: Programming Structures for EE
## Assignment 1: Customer Management Program
### Part 1 - Argument Validation

* Purpose

The purpose of this assignment is to help you learn or review (1) the fundamentals of the C programming language, (2) the details of implementing command prompts in C, (3) how to use the GNU/Unix programming tools, especially bash, emacs, and gcc209.

* Rules

Make sure you study the course Policy web page before doing this assignment or any of the EE 209 assignments. In particular, note that you may consult with the course instructors, lab TAs, KLMS, etc. while doing assignments, as prescribed by that web page. However, there is one exception...

Throughout the semester, each assignment will have an "on your own" part. You must do that part of the assignment completely on your own, without consulting with the course instructors, lab TAs, listserv, etc., except for clarification of requirements. You might think of the "on your own" part of each assignment as a small take-home exam.

For this assignment, "checking for a missing or malformed option parameter" (as described below) is the "on your own" part. That part is worth 10% of this assignment.

* Background

A command-line interface (CLI) is a means of interacting with a program where the user issues text commands to the program.

A command prompt (or just prompt) is a string used in a CLI to inform and literally prompt the users to type commands. A prompt usually ends with one of the characters $, %, #, :, >

A bash shell, embedded in many Unix systems, uses a prompt of the form:
[time] user@host: work_dir $
DOS's COMMAND.COM and the Windows's command-line interpreter cmd.exe use the prompt of the form:
C:\>
where 'C' represents the default main disk label in most modern systems.
A command-line argument or parameter is an item of information delivered to a program when it is started. In Unix and Unix-like environments, an example of a command-line argument is:

mkdir ee209
where "ee209" is a command-line argument which tells the program mkdir to create a new folder named "ee209".
The Task
In this assignment, your task is to write a simple CLI that validates user commmand for a customer management program as described below.

Customer Management Program
Throughout Assignment 1 and 3, you will develop a customer management program, which handles customer information and the operations on them. The requirements for the customer management program are:

A client can register a new customer and store her information.
A client can unregister a customer and remove her information.
A client can search for a customer and retrieve her information.
The customer information to be managed includes:
ID: the online ID of the customer
Name: the name of the customer
Purchase amount: the amount of money that the customer has purchased
In this assignment, you only have to validate the command-line input line-by-line, i.e. your program will just read each line and check whether it is a valid command. If the command is invalid, the CLI prints out an error message to the standard error(stderr) stream and waits for the next command. If the command is valid, it does nothing and waits for the next command. There is no dependency between the commands.

