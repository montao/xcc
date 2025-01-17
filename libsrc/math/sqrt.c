#include "math.h"

#ifndef __NO_FLONUM
double sqrt(double x) {
#if defined(__x86_64__) && !defined(__GNUC__)
  __asm("sqrtsd %xmm0, %xmm0");
#elif defined(__aarch64__) && !defined(__GNUC__)
  __asm("fsqrt d0, d0");
#else
  if (x < 0)
    return NAN;
  if (!isfinite(x))  // NAN or HUGE_VAL
    return x;

  double l = 0, r = x >= 1 ? x : 1.0;
  for (int i = 0; i < 32; ++i) {
    double m = (l + r) * 0.5;
    double mm = m * m;
    if (mm < x)
      l = m;
    else
      r = m;
  }
  return (l + r) * 0.5;
#endif
}
#endif
