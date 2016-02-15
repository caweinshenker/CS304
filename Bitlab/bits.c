/*
 * CS:APP Data Lab
 *
 * Colin Weinshenker 930873477
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
/*
 * bitAnd - x&y using only ~ and |
 *   Example: bitAnd(6, 5) = 4
 *   Legal ops: ~ |
 *   Max ops: 8
 *   Rating: 1
 */
int bitAnd(int x, int y) {
  return ~(~x | ~y);
  //return 2;
}
/*
 * copyLSB - set all bits of result to least significant bit of x
 *   Example: copyLSB(5) = 0xFFFFFFFF, copyLSB(6) = 0x00000000
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int copyLSB(int x) {
  //bitmask by 255 to expose LSB
  //Bitshift left to move LSB to MSB position
  //Arithmetic shift right to copy LSB
  x = x & 255;
  x = x << 31;
  x = x >> 31;
  return x;
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
  //Generate byte-long bitmask
  //Left shift n by 3 to create integer equal to number of places the bitmask must move
  //Move bitmask
  int mask;
  int y;
  mask = 255;
  y = n << 3;
  mask = mask  << y;
  x = x & mask;
  return ((x >> y) & 255);
}
/*
 * isEqual - return 1 if x == y, and 0 otherwise
 *   Examples: isEqual(5,5) = 1, isEqual(4,5) = 0
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int isEqual(int x, int y) {
  //Observe that the xor of two equal number is 0
  //Xor of two unequal numbers is not 0
  //Return !(x^y)
  return !(x ^ y);
}
/*
 * bitMask - Generate a mask consisting of all 1's
 *   lowbit and highbit
 *   Examples: bitMask(5,3) = 0x38
 *   Assume 0 <= lowbit <= 31, and 0 <= highbit <= 31
 *   If lowbit > highbit, then mask should be all 0's
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int bitMask(int highbit, int lowbit) {
  //To determine whether lowbit is bigger than highbit:
  // 1. Get two's compliment of lowbit
  // 2. Add highbit value to lowbit two's compliment
  // 3. If that value has MSB 1, lowbit > highbit
  // Generate bitmask from bit 32 to lowbit
  // Generate bitmask from bit 32 to highbit
  // Xor bitmasks
  int difference;
  int highGreater;
  int lowShift;
  int highShift;
  int high31;
  difference = ~(lowbit) + 1 + highbit;
  highGreater= ~difference >> 31 << 31;
  lowShift = (31 + (~lowbit + 1));
  highShift =(31 + ~ highbit);
  high31 = ~(!(highbit ^ 31) << 31 >> 31);
  return ((highGreater >> highShift & high31) ^ (highGreater >> lowShift));
}
/*
 * reverseBytes - reverse the bytes of x
 *   Example: reverseBytes(0x01020304) = 0x04030201
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 25
 *   Rating: 3
 */
int reverseBytes(int x) {
  int byte0;
  int byte1;
  int byte2;
  int byte3;;
  byte0 = x << 24;
  byte1 = (x << 8) & (255 << 16);
  byte2 = (x << 8 >> 24 << 8) & (255 << 8);
  byte3 = (x >> 24) & 255;
  return (byte0 + byte1 + byte2 + byte3);
}
/*
 * bang - Compute !x without using !
 *   Examples: bang(3) = 0, bang(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4
 */
int bang(int x) {
  //Any number bit-wise or'ed with its two's compliment gives 1 in sign bit except zero
  // Or x with two's complement of x and right shift 31;
  // Then xor with 0xffffffff to turn non-zero case to all zeroes and zero case to all ones
  // Finally, xor with one to isolate single one
  x = x | (~x + 1);
  x = x >> 31;
  x = x ^ (1 << 31 >> 31);
  return (1 & x);
}
/*
 * leastBitPos - return a mask that marks the position of the
 *               least significant 1 bit. If x == 0, return 0
 *   Example: leastBitPos(96) = 0x20
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 2
 */
int leastBitPos(int x) {
  int y;
  int z;
  y = 1 << 31;
  y = y >> 31;
  z = x ^ y;
  z = z + 1;
  return (z & x);
}
/*
 * minusOne - return a value of -1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 2
 *   Rating: 1
 */
int minusOne(void) {
  int x = 1;
  return ~x + 1;
}
/*
 * TMax - return maximum two's complement integer
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmax(void) {
  int x = 1;
  int y = 1;
  x = x << 31;
  x = x >> 31;
  y = y << 31;
  return (x ^ y);
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
  //Shift x over by 32 - n bits
  //Then shift back 32 - n bits and check whether the shifted and original are the same.
  int original;
  int shifted;
  original = x;
  shifted = (x << (32 + ~n + 1)) >> (32 + ~n + 1);
  return  !(original ^ shifted);
}
/*
 * addOK - Determine if can compute x+y without overflow
 *   Example: addOK(0x80000000,0x80000000) = 0,
 *            addOK(0x80000000,0x70000000) = 1,
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3
 */
int addOK(int x, int y) {
  //Determine whether x and y has the same sign
  //If so, return whether the sign of their sum is the same as their signs
  //If so, return 1
  int signX;
  int signY;
  int signXPlusY;
  int xYSameSign;
  signX = 1 & (x >> 31);
  signY = 1 & (y >> 31);
  xYSameSign = !(signX ^ signY);
  signXPlusY = 1 & ((x + y) >> 31);
  return  !(xYSameSign) | !(signXPlusY  ^ signX);
}
/*
 * isGreater - if x > y  then return 1, else return 0
 *   Example: isGreater(4,5) = 0, isGreater(5,4) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isGreater(int x, int y) {
  //Perform check for x-y equality -- must be 0
  //Observe that if x and y have different signs, the sign of x has to be 0
  //Check for x -y overflow. If so, return sign of y -x;
  int signX;
  int signY;
  int overflowAndSameSign;
  int sameSign;
  int difference;
  int differenceSign;
  int yMinusXSign;
  int isEqual;
  isEqual = !(x ^ y);
  signX = 1 & (x >> 31);
  signY = 1 & (y >> 31);
  sameSign = !(signX ^ signY);
  difference = x + (~y + 1);
  differenceSign = 1 & (difference  >> 31);
  overflowAndSameSign = !!(sameSign ^ differenceSign);
  yMinusXSign = !differenceSign;
  return !isEqual & ((!(sameSign | (signX ^ 0)) | (overflowAndSameSign & yMinusXSign))) ;
}
/*
 * isNegative - return 1 if x < 0, return 0 otherwise
 *   Example: isNegative(-1) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 2
 */
int isNegative(int x) {
  //Right shift 31 to isolate sign bit
  // And with one to eliminate lefthand 1s on negative numbers
  x = x >> 31;
  x = x & 1;
  return x;
}
/*
 * multFiveEighths - multiplies by 5/8 rounding toward 0.
 *   Should exactly duplicate effect of C expression (x*5/8),
 *   including overflow behavior.
 *   Examples: multFiveEighths(77) = 48
 *             multFiveEighths(-22) = -13
 *             multFiveEighths(1073741824) = 13421728 (overflow)
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 3
 */
int multFiveEighths(int x) {
  //Observe that a carrying problem occurs if the least significant byte of a negative x is 0 or 8
  // If so, must add one to solution.
  // Shift x left 2 and add x to get 5x
  // Shift right to get 5/8x
  // Add one as appropriate
  int neg;
  int fiveTimes;
  int fiveEighths;
  int sign;
  int lowestByte;
  int lowestByteDivisibleby8;
  sign = 1 & (x >> 31);
  neg = !(1 ^ sign);
  lowestByte = x & 15;
  lowestByteDivisibleby8 = neg & !(!((lowestByte ^ 8) & (lowestByte ^ 0)));
  fiveTimes = (x << 2) +  x;
  fiveEighths = fiveTimes >> 3;
  // printf("%08x\n", x);
  //printf("%08x\n", fiveTimes);
  //printf("%08x\n", fiveEighths);
  return fiveEighths + lowestByteDivisibleby8;
}
/*
 * sm2tc - Convert from sign-magnitude to two's complement
 *   where the MSB is the sign bit
 *   Example: sm2tc(0x80000005) = -5.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 4
 */
int sm2tc(int x) {
  int sign;
  int magnitude;
  int sm2tcPos;
  int sm2tcNeg;
  sign = 1 & (x >> 31);
  magnitude = x & (~0 ^ 1 << 31);
  sm2tcPos = (magnitude & (~!(sign) + 1));
  sm2tcNeg = (~(magnitude) + 1) & (~!!(sign) + 1);
  return sm2tcPos | sm2tcNeg;


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
 int leadingOnePos;
 int tmpX;
 int sign;
 int shifts;
 int exponent;
 int result;
 int significandMask;
 int significand;
 int exponentShown;
 significandMask = (255 << 15) + (255 << 7) + 127;
 exponent = 0;
 significand = x;
 shifts = 0;
 sign = 1 & (x >> 31);
 if (!(x ^  0)){ //can return 0 if x is zero
     return 0;
   }
 if (!(x ^ (1 <<31))){ // 0x80000000 represented in 32-bit floating point, but absolute value is possible.
     return (207 << 24);
   }
 if (sign & 1){ //get absolute value if x is negative;
    significand = -x;
    printf("%08x\n", significand);
 }
 while ( !(((1 << 23) & significand) ^ 0)){
   printf("%08x\n", significand);
   printf("%d\n", shifts);
   shifts = shifts + 1;
   significand = significand << 1;
 }
 printf("%d\n", shifts);
 significand = significand & (significandMask);
 exponentShown = (127 + 23 - shifts) << 23;
 sign = sign << 31;
 result = (sign + exponentShown + significand);
 return result;

}
