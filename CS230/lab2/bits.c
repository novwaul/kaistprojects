/* 
 * CS:APP Data Lab 
 * 
 * <name: InJe Hwang, studentID: 20160788, userID: cs20160788>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting an integer by more
     than the word size.

EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implent floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function. 
     The max operator count is checked by dlc. Note that '=' is not 
     counted; you may use as many of these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
/* Copyright (C) 1991-2014 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */
/* This header is separate from features.h so that the compiler can
   include it implicitly at the start of every compilation.  It must
   not itself include <features.h> or any other header that includes
   <features.h> because the implicit include comes before any feature
   test macros that may be defined in a source file before it first
   explicitly includes a system header.  GCC knows the name of this
   header in order to preinclude it.  */
/* glibc's intent is to support the IEC 559 math functionality, real
   and complex.  If the GCC (4.9 and later) predefined macros
   specifying compiler intent are available, use them to determine
   whether the overall intent is to support these features; otherwise,
   presume an older compiler has intent to support these features and
   define these macros by default.  */
/* wchar_t uses ISO/IEC 10646 (2nd ed., published 2011-03-15) /
   Unicode 6.0.  */
/* We do not support C11 <threads.h>.  */
/* 
 * bitAnd - x&y using only ~ and | 
 *   Example: bitAnd(6, 5) = 4
 *   Legal ops: ~ |
 *   Max ops: 8
 *   Rating: 1
 */
int bitAnd(int x, int y) {
  return ~(~x|~y);
}
/* 
 * getByte - Extract byte n from word x
 *   Bytes numbered from 0 (LSB) to 3 (MSB)
 *   Examples: getByte(0x12345678,1) = 0x56
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 2
 */
int getByte(int x, int n) {
  int mask = 0xFF;
  int offset = n;
  offset = offset << 3;
  return ((x >> offset) & mask);
}
/* 
 * logicalShift - shift x to the right by n, using a logical shift
 *   Can assume that 0 <= n <= 31
 *   Examples: logicalShift(0x87654321,4) = 0x08765432
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3 
 */
int logicalShift(int x, int n) {
  int mask = 0xFF;
  mask = mask | (mask << 8);
  mask = mask | (mask << 16);
  mask = mask << n;
  mask = ~mask;
  mask = mask << (32 + ~n + 1);
  mask = ~mask;
  return (x >> n) & mask;
}
/*
 * bitCount - returns count of number of 1's in word
 *   Examples: bitCount(5) = 2, bitCount(7) = 3
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 40
 *   Rating: 4
 */
int bitCount(int x) {
  int mask = 0x01;
  int collector = 0xFF;
  int partialCount = 0;
  int count = 0;
  mask = mask|(mask << 8);
  mask = mask|(mask << 16);
  /* parallel count partitioned by 8 bits */
  partialCount = x & mask;
  partialCount = partialCount + ((x >> 1) & mask);
  partialCount = partialCount + ((x >> 2) & mask);
  partialCount = partialCount + ((x >> 3) & mask);
  partialCount = partialCount + ((x >> 4) & mask);
  partialCount = partialCount + ((x >> 5) & mask);
  partialCount = partialCount + ((x >> 6) & mask);
  partialCount = partialCount + ((x >> 7) & mask);
  /* summation of partial counts */
  count = (partialCount + (partialCount >> 8) + (partialCount >> 16) + (partialCount >> 24) )& collector;
  return count;
}
/* 
 * bang - Compute !x without using !
 *   Examples: bang(3) = 0, bang(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int bang(int x) {
  /* make MSB to 1 */
  int sign = x >> 31;
  x = (sign & x) | ( ~sign & (~x + 1));
  return (x >> 31) + 1;
}
/* 
 * tmin - return minimum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {
  return 0x80 << 24;
}
/* 
 * fitsBits - return 1 if x can be represented as an 
 *  n-bit, two's complement integer.
 *   1 <= n <= 32
 *   Examples: fitsBits(5,3) = 0, fitsBits(-4,3) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
int fitsBits(int x, int n) {
  int sign = x >> 31;
  int decOne = ~0;
  int Tmin = 0x01 << (n + decOne);
  int Tmax = Tmin + decOne;
  return !(((sign & (x + Tmin)) + (~sign & (Tmax + ~x + 1))) >> 31);
}
/* 
 * divpwr2 - Compute x/(2^n), for 0 <= n <= 30
 *  Round toward zero
 *   Examples: divpwr2(15,1) = 7, divpwr2(-33,4) = -2
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
int divpwr2(int x, int n) {
  int divisor = 1 << n;
  int sign = x >> 31;
  int bias = sign & (divisor + ~0); // if x is non-negative, bias = 0
  return (x + bias) >> n; //if x is negative, execute (x + divisor - 1) / divisor
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
  return ~x + 1;
}
/* 
 * isPositive - return 1 if x > 0, return 0 otherwise 
 *   Example: isPositive(-1) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 8
 *   Rating: 3
 */
int isPositive(int x) {
  return !(x >> 31) & !!x; // if x is positive or negative, !!x = 1. if x is 0, !!x = 0                           .
}
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
  int Xsign = x >> 31;
  int Ysign = y >> 31;
  int trueCase1 = !(~Xsign | Ysign); //x is negative and y is positve
  int trueCase2 = ( (Xsign & Ysign) | (~Xsign & ~Ysign) ) & !((y + ~x + 1) >> 31); // x and y have same sign, and y - x is positive
  return trueCase1 | trueCase2 ;
}
/*
 * ilog2 - return floor(log base 2 of x), where x > 0
 *   Example: ilog2(16) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 90
 *   Rating: 4
 */
int ilog2(int x) {
  int mask = 0x01;
  int collector = 0xFF;
  int partialCount = 0;
  int count = 0;
  mask = mask | (mask << 8);
  mask = mask | (mask << 16);
  /* change all 0s to 1 which are after the first 1. ex. 001001100... -> 001111111...*/
  x = x | (x >> 1);
  x = x | (x >> 2);
  x = x | (x >> 4);
  x = x | (x >> 8);
  x = x | (x >> 16);
  /* parallel count */
  partialCount = x & mask;
  partialCount = partialCount + ((x >> 1) & mask);
  partialCount = partialCount + ((x >> 2) & mask);
  partialCount = partialCount + ((x >> 3) & mask);
  partialCount = partialCount + ((x >> 4) & mask);
  partialCount = partialCount + ((x >> 5) & mask);
  partialCount = partialCount + ((x >> 6) & mask);
  partialCount = partialCount + ((x >> 7) & mask);
  /* summation of partial counts */
  count = (partialCount + (partialCount >> 8) + (partialCount >> 16) + (partialCount >> 24) ) & collector;
  return count + ~0; // subtact the first 1
}
/* 
 * float_neg - Return bit-level equivalent of expression -f for
 *   floating point argument uf
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representations of
 *   single-precision floating point values.
 *   When argument is NaN, return argument.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 10
 *   Rating: 2
 */
unsigned float_neg(unsigned uf) {
 unsigned adder = 0x80000000;
 unsigned mask = 0xFF;
 unsigned exp = (uf >> 23) & mask;
 if( exp == 0xFF && (uf << 9) != 0) return uf; // if NaN, return argument
 return uf + adder;
}
/* 
 * float_i2f - Return bit-level equivalent of expression (float) x
 *   Result is returned as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point values.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_i2f(int x) {
 int maskRemain = 0x000001FF;
 int maskFirstBit =0x80000000;
 int maskFrac = 0x007FFFFF;
 int maskLastFracBit = 0x00000200;
 int sign;
 int exp;
 int frac;
 int countToDelete;
 int remain;
 int result;
 /* denormalized case */
 if(x == 0) return 0x00000000;
 /* normalized case */
 sign = x & maskFirstBit;
 if(sign) x = ~x + 1;
 countToDelete = 0;
 while( (x << countToDelete) > 0 ) countToDelete = countToDelete + 1; 
 countToDelete = countToDelete + 1;// include leading 1
 x =  x << countToDelete;
 frac = (x >> 9) & maskFrac;
 exp = (159 + ~countToDelete + 1) << 23; //exp = 127 + 32 - countToDelete
 result = sign | exp | frac;
 /* rounding */
 remain = x & maskRemain;
 if( remain != 0 && ((x & maskLastFracBit) + remain == 768 || remain > 256) ) // Last bit of fraction is 2^9 = 512
    result = result + 1;                                                      // Therefore, frac can increase when remain > 256 or
 return result;                                                               // remain = 256 and the last bit of fraction is 1
}
/* 
 * float_twice - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_twice(unsigned uf) {
  int maskSign = 0x80000000;
  int maskExp = 0xFF;
  int maskFraction = 0x7FFFFF;
  int maskCarry = 0x800000;
  int sign = uf & maskSign;
  int exp = (uf >> 23) & maskExp;
  int frac = uf & maskFraction; 
  if( exp == 0xFF || uf == 0x00000000 || uf == 0x80000000 ) return uf; // return for Infinity, NaN and 0
  else if( exp == 0 ) frac = frac << 1; // denormalized case
  else exp = exp + 1; // normalized case
  if( frac & maskCarry )
  {
    frac = frac & maskFraction;
    exp = exp + 1;
  }
  return sign | exp << 23 | frac;
}
