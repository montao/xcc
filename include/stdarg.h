#pragma once

#include "stdint.h"

#if !defined(__WASM)

#if defined(__GNUC__)

// TODO:
#if defined(__APPLE__)
#include "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib/clang/14.0.0/include/stdarg.h"
#elif defined(__x86_64__)
#include "/usr/lib/gcc/x86_64-linux-gnu/9/include/stdarg.h"
#elif defined(__aarch64__)
#include "/usr/lib/gcc/aarch64-linux-gnu/8/include/stdarg.h"
#else
#error xxx
#endif

#elif defined(__APPLE__) && defined(__aarch64__)
typedef void **va_list;

#define va_start(ap,p)    __builtin_va_start(ap,&(p))
#define va_end(ap)        /*(void)*/(ap = 0)
#define va_arg(ap, type)  (*(type*)(ap)++)  // Assume little endian
#define va_copy(dst,src)  (dst = src)

#else  // not __APPLE__ nor __aarch64__

struct __va_elem {
  unsigned int gp_offset;
  unsigned int fp_offset;
  void *overflow_arg_area;
  void *reg_save_area;
};

typedef struct __va_elem __builtin_va_list[1];

typedef __builtin_va_list __gnuc_va_list;
typedef __gnuc_va_list va_list;

#define va_start(ap,p)    __builtin_va_start(ap,&(p))
#define va_end(ap)        __builtin_va_end(ap)
#define va_arg(ap,ty)     __builtin_va_arg(ap,ty)
#define va_copy(dst,src)  __builtin_va_copy(dst,src)

#if defined(__aarch64__)
#define __GP_REG_ARGS  (8)
#else
#define __GP_REG_ARGS  (6)
#endif
#define __FP_REG_ARGS  (8)
#define __WORD_SIZE    (8)
#define __DBL_SIZE     (8)

#define __GP_OFFSET_MAX  (__GP_REG_ARGS * __WORD_SIZE)
#define __FP_OFFSET_MAX  (__GP_OFFSET_MAX + __FP_REG_ARGS * __DBL_SIZE)

//

#define __va_arg_mem(ap, sz, align) ({ \
  uintptr_t p = (uintptr_t)((ap)->overflow_arg_area); \
  if ((align) > 8)  p = (p + 15) / 16 * 16; \
  (ap)->overflow_arg_area = (void*)((p + (sz) + 7) & -8); \
  (char*)p; })

#define __va_arg_gp(ap, sz, align) \
  ((ap)->gp_offset < __GP_OFFSET_MAX \
    ? (char*)(ap)->reg_save_area + ((ap)->gp_offset += __WORD_SIZE) - __WORD_SIZE \
    : __va_arg_mem(ap, sz, align))

#define __va_arg_fp(ap, sz, align) \
  ((ap)->fp_offset < __FP_OFFSET_MAX \
    ? (char*)(ap)->reg_save_area + ((ap)->fp_offset += __DBL_SIZE) - __DBL_SIZE \
    : __va_arg_mem(ap, sz, align))

#ifndef __NO_FLONUM
#define __builtin_va_arg_fp_case(ap, ty) \
  case /*flonum*/ 6: \
    p = __va_arg_fp(ap, sizeof(ty), _Alignof(ty)); break;
#else
#define __builtin_va_arg_fp_case(ap, ty)  /*none*/
#endif

#define __builtin_va_arg(ap, ty) ({ \
  char *p; \
  switch (__builtin_type_kind(ty)) { \
  __builtin_va_arg_fp_case(ap, ty) \
  case /*fixnum*/ 1: case /*ptr*/ 2: \
    p = __va_arg_gp(ap, sizeof(ty), _Alignof(ty)); break; \
  default: \
    p = __va_arg_mem(ap, sizeof(ty), _Alignof(ty)); break; \
  } \
  *(ty *)p; })

#define __builtin_va_copy(dest, src) ((dest)[0] = (src)[0])

#define __builtin_va_end(ap)  /* none */

#endif

#else  // __WASM

typedef __builtin_va_list __gnuc_va_list;
typedef __gnuc_va_list va_list;

#define va_start(ap,p)    __builtin_va_start(ap,p)
#define va_end(ap)        __builtin_va_end(ap)
#define va_arg(ap,ty)     __builtin_va_arg(ap,ty)
#define va_copy(dst,src)  __builtin_va_copy(dst,src)

#endif
