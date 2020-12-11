// Mandelbrot

// Compile:
//   $ ./xcc -omandelbrot -Iinc examples/mandelbrot.c lib/sprintf.c lib/umalloc.c lib/lib.c lib/crt0.c
// Run:
//   $ time ./mandelbrot

#include <stdio.h>
#include <stdlib.h>

#ifdef USE_FLOAT
typedef float Number;
#else
typedef double Number;
#endif

// 0..finite, >0..divergence count.
unsigned int mandelbrot(Number cx, Number cy, unsigned int threshold) {
  Number x = 0, y = 0;
  for (unsigned int n = 1; n <= threshold; ++n) {
    if (x * x + y * y > 4)
      return n;
    Number nx = x * x - y * y + cx;
    Number ny = 2 * x * y     + cy;
    x = nx;
    y = ny;
  }
  return 0;
}

unsigned int calc_color(unsigned int n) {
  unsigned char r = n * 3;
  unsigned char g = n * 2;
  unsigned char b = n * 11;
  return (((unsigned int)r) << 16) | (((unsigned int)g) << 8) | b;
}

int main() {
  const unsigned int threshold = 3000;
  const int W = 512, H = 512;
  const Number XMIN = -1.75;
  const Number YMIN = -1.125;
  const Number XS = 2.25, YS = XS * H / W;

  unsigned char *buf = malloc(W * H * 3);
  unsigned char *p = buf;

  for (int i = 0; i < H; ++i) {
    Number cy = YS * i / H + YMIN;
    for (int j = 0; j < W; ++j) {
      Number cx = XS * j / W + XMIN;
      unsigned int n = mandelbrot(cx, cy, threshold);
      unsigned int c = calc_color(n);
      *p++ = c >> 16;
      *p++ = c >> 8;
      *p++ = c;
    }
  }

  FILE *fp = fopen("mandelbrot.ppm", "wb");
  if (fp == NULL) {
    fprintf(stderr, "Cannot open output file\n");
    exit(1);
  }
  fprintf(fp, "P6\n%d %d\n255\n", W, H);
  fwrite(buf, W * H * 3, 1, fp);
  fclose(fp);

  return 0;
}
