# KAIST EE 209: Programming Structures for EE
## Assignment 2: String Manipulation
(Acknowledgment: This assignment is borrowed and modified from Princeton's COS 217)

### Purpose
The purpose of this assignment is to help you learn/review (1) arrays and pointers in the C programming language, (2) how to create and use stateless modules in C, (3) the "design by contract" style of programming, and (4) how to use the GNU/UNIX programming tools, especially *bash*, *emacs*, *gcc*, and *gdb*.

### Rules
Implement the string functions listed by the Table in Part 1. and Part 2 is the "on your own" part of this assignment, which is worth 50% of this assignment.

You will get an extra 10% of the full score if you implement the Part 1 only with pointer notation when you access the character. See the Extra credit section below (Extra Credit for Part 1).

### Background
As you know, the C programming environment provides a standard library. The facilities provided in the standard library are declared in header files. One of those header files is string.h; it contains the declarations of "string functions," that is, functions that perform operations on character strings. Appendix D of the King textbook, Appendix B3 of the Kernighan and Ritchie textbook, and the UNIX "man" pages describe the string functions. The string functions are used heavily in programming systems; certainly any editor, compiler, assembler, or operating system created with the C programming language would use them.

### Overview of Your Task
Your task in this assignment is to use C to create the "Str" module that provides string manipulation functions. Specifically, design your Str module so that each function behaves the same as described below. Your task in this assignment is threefold.

[Part 1] Read the description of the basic string library functions carefully, and implement each function. The basic functions are most commonly used standard string functions. Each function should behave the same as its corresponding standard C function.

[Part 2] Implement a simplified version of *grep* using *Str* functions. Read the provided file that contains skeleton code carefully, edit the file to make it process the required functionalities: find, replace, diff.

### Part 1: The Basic Str Function Implementation
Your task for the first part is to use C to implement five basic string manipulation functions: *StrGetLength()*, *StrCopy()*, *StrCompare()*, *StrSearch()*, *StrConcat()*. Those five functions should follow the format of the corresponding standard C library functions. You can easily find the function's description and operation in the UNIX "man" page.

The following table shows the required basic string functions in Part 1 and their corresponding function names in the standard C library.

|Str Function|	Standard C Function	|Man page link|
|---|---|---|
|strGetLength	|strlen	|strlen man page|
|StrCopy	|strcpy	|strcpy man page|
|StrCompare	|strcmp	|strcmp man page|
|StrSearch	|strstr	|strstr man page|
|StrConcat	|strcat	|strcat man page|

Use the *Str* module's interface in a file named str.h, and place your *Str* function definitions in str.c.

Note that your Str functions should not call any of the standard string functions. In the context of this assignment, pretend that the standard string functions do not exist. However, your functions may call each other, and you may define additional (non-interface) functions.

Design each function definition so it calls the *assert* macro to validate the function's parameters. In that way, your *Str* functions should differ from the standard C string functions. Specifically, design each function definition to assert that each parameter is not *NULL*. See the note below for more information of the *assert()* macro function.

Beware of type mismatches. In particular, beware of the difference between type size_t and type int: a variable of type size_t can store larger integers than a variable of type int can. Your functions should (in principle) be able to handle strings whose lengths exceed the capacity of type int. Also beware of type mismatches related to the use of the const keyword.

### Extra Credit for Part 1: Implement functions with pointer notation
There are various ways to implement the functions in Part 1. Especially, you can access the character by pointer dereferencing like **pcSrc* or by using an array notation such as *pcSrc[uiLength]* .

Here are two examples of *StrGetLength()* implementation. The first code implements the StrGetLength() function with the array notation; it traverses each given string or accesses the character using an index relative to the beginning of the string. However, with the pointer notation, the second version traverses each given string using an incremented pointer.
```
 size_t StrGetLength(const char pcSrc[]) /* Use array notation */
{
   size_t uiLength = 0U;
   assert(pcSrc != NULL);
   while (pcSrc[uiLength] != '\0')
      uiLength++;
   return uiLength;
}
 size_t StrGetLength(const char *pcSrc) /* Use pointer notation */
{
   const char *pcEnd;
   assert(pcSrc != NULL);
   pcEnd = pcSrc;
   while (*pcEnd != '\0') /* note that *(pcSrc + uiLength) is valid but is "NOT" acceptable as pointer notation */
      pcEnd++;
   return (size_t)(pcEnd - pcSrc);
}
```
You can freely implement Part 1. However, if you implement part 1 only with pointer notation, you can get extra 10% of the full score. Please write your choice of implementation in the readme file. That is, specify if you used only the pointer notation for Part 1 in the readme file.

### Test your Str Functions
We provide a test client (client.c) that compares the results of the Str and C standard library functions with various input. Please use this code for testing and debugging. You can compile the test client with *gcc209* as follows.
```
gcc209 -o client client.c str.c
```
Then, test each of your *Str* functions separately by providing a function name as an argument. For example,
```
./client StrCopy
```
will test *StrCopy*. Actually, the client accepts any one of the *Str* function names as a command-line argument:
```
./client [StrGetLength|StrCopy|StrCompare|StrSearch|StrConcat]
```
Note that passing all tests provided by the client does not mean that your function always behaves correctly. Please devise your own testing (e.g., by changing the client code) for more confidence. Note that we may use a different test client for grading.

### Part 2: Simple Grep
*grep* is a popular UNIX tool that manipulates input strings. In this part, you implement a simplified version of *grep* called *sgrep*. *sgrep* provides these three functionalities.

* Find (*-f search-string*): reads each line from standard input (stdin) and prints out only the lines that contain *search-string* to standard output (stdout).
* Replace (*-r string1 string2*): reads each line from standard input (stdin), replaces all occurrences of *string1* with *string2*, and prints it out to standard output (stdout). If *string1* is not found in the line, the original line is copied to stdout.
* Diff (*-d file1 file2*): compares the two files (*file1* and *file2*) line by line, and prints out only the different lines with their line numbers to standard output (stdout). The output format of the different line should be as follows:
  ```
  file1@line-num:file1's line
  file2@line-num:file2's line
  ```
Here are some usage examples of *sgrep* (you can also see the test files (google_wiki.txt and microsoft.txt) which are used in the following example):
```
 $ ./sgrep -f Google < google_wiki.txt
Google Inc. is an American multinational technology company specializing 
Google was founded by Larry Page and Sergey Brin while they were Ph.D.
 supervoting stock. They incorporated Google as a privately held company on
 be evil.[9][10] In 2004, Google moved to its new headquarters in Mountain
View, California, nicknamed the Googleplex.[11] In August 2015, Google 
Alphabet Inc. When this restructuring took place on October 2, 2015, Google 
became Alphabet's leading subsidiary, as well as the parent for Google's 

$ ./sgrep -r Google Microsoft < google_wiki.txt > microsoft.txt
$ cat microsoft.txt (output is abbreviated with ...)
Microsoft Inc. is an American multinational technology company specializing
...
Microsoft was founded by Larry Page and Sergey Brin while they were Ph.D.
...
supervoting stock. They incorporated Microsoft as a privately held company on
...
be evil.[9][10] In 2004, Microsoft moved to its new headquarters in Mountain 
...
View, California, nicknamed the Microsoftplex.[11] In August 2015, Microsoft  
...
Alphabet Inc. When this restructuring took place on October 2, 2015, Microsoft 
...
became Alphabet's leading subsidiary, as well as the parent for Microsoft's 

$ ./sgrep -d google_wiki.txt microsoft.txt
google_wiki.txt@1:Google Inc. is an American multinational technology company specializing
microsoft.txt@1:Microsoft Inc. is an American multinational technology company specializing
google_wiki.txt@6:Google was founded by Larry Page and Sergey Brin while they were Ph.D.
microsoft.txt@6:Microsoft was founded by Larry Page and Sergey Brin while they were Ph.D.
google_wiki.txt@9: supervoting stock. They incorporated Google as a privately held company on
microsoft.txt@9: supervoting stock. They incorporated Microsoft as a privately held company on
google_wiki.txt@15: be evil.[9][10] In 2004, Google moved to its new headquarters in Mountain
microsoft.txt@15:be evil.[9][10] In 2004, Microsoft moved to its new headquarters in Mountain 
google_wiki.txt@16:View, California, nicknamed the Googleplex.[11] In August 2015, Google
microsoft.txt@16:View, California, nicknamed the Microsoftplex.[11] In August 2015, Microsoft  
google_wiki.txt@18:Alphabet Inc. When this restructuring took place on October 2, 2015, Google
microsoft.txt@18:Alphabet Inc. When this restructuring took place on October 2, 2015, Microsoft 
google_wiki.txt@19:became Alphabet's leading subsidiary, as well as the parent for Google's 
microsoft.txt@19:became Alphabet's leading subsidiary, as well as the parent for Microsoft's 
```
#### Rules:

* General rules 
   * Use your Str functions to implement the functionality. That is, you need to finish Part 1 first to get any credit for this task. You should not use any string function in string.h
  * Assume each line (including a new line character ('\n') is no more than 1023 bytes. You should stop your program with a proper error message if you encounter a line larger than 1023 bytes.
  * A string argument (*search-string*, *string1*, *string2*) refers to a sequence of any non-space characters. It can be enclosed with double quote(") characters and can be either empty ("") or can include a space (e.g., "hello world"). No need to handle an escape character for this assignment.
   * Assume a string or a file argument is no more than 1023 bytes. You should stop your program with a proper error message if you encounter a command-line argument that's too long.
  * Any error message should be printed out to standard error (stderr). Use *fprintf(stderr, ...);* for that.
  * Unlike Part 1, you don't have to add assert function while implementing sgrep. In other words, after printing out the error messages in error case, sgrep should be finished with EXIT_FAILURE as the return value. Details are written in skeleton code, so please read the comments in skeletion code (sgrep.c).

* Rules for Find and Replace
  * *string1* cannot be an empty string. In such a case, your program should stop with a proper error message (e.g., "Error: Can't replace an empty substring").
  * Other string arguments (*string2*, *search-string*) can be an empty string. That is, an empty search string matches any line, and if *string2* is an empty string, it removes any occurrences of *string1* in the matching line.
##### Tips:

We provide a skeleton code file (sgrep.c). You can start with the file.
*fgets* should be useful for reading a line from a file. 'man fgets' should give you more information.
