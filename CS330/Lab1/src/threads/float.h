#include <stdint.h>

#define F 16384 /* 2^14 */

int tofloat (int);
int toint (int);
int round (int);
int addfloat (int, int);
int subfloat (int, int);
int addfloatint (int, int);
int subfloatint (int, int);
int mulfloat (int, int);
int mulfloatint (int, int);
int divfloat (int, int);
int divfloatint (int, int);

int 
tofloat (int n)
{
  return n * F;
}

int 
toint (int x)
{
  return x / F;
}

int 
round (int x)
{
  if (x >= 0)
   return (x + F / 2) / F;
  else 
   return (x - F / 2) / F;
}

int 
addfloat (int x, int y)
{
  return x + y;
}

int
subfloat (int x, int y)
{
  return x - y;
}

int
addfloatint (int x, int n)
{
  return x + n * F;
}

int
subfloatint (int x, int n)
{
  return x - n * F;
}

int
mulfloat (int x, int y)
{
  return ((int64_t) x) * y / F;
}

int
mulfloatint (int x, int n)
{
  return x * n;
}

int
divfloat (int x, int y)
{
  return ((int64_t) x) * F / y;
}

int 
divfloatint (int x, int n)
{
  return x / n;
}
