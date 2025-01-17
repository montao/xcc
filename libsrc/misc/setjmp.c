#if !defined(__WASM) && !defined(__GNUC__)
#include "setjmp.h"

#if defined(__x86_64__)
int setjmp(jmp_buf env) {
  __asm("mov (%rsp), %rax\n"  // return address.
        "mov %rax, 0(%rdi)\n"
        "mov %rbp, 8(%rdi)\n"
        "mov %rsp, 16(%rdi)\n"
        "mov %rbx, 24(%rdi)\n"
        "mov %r12, 32(%rdi)\n"
        "mov %r13, 40(%rdi)\n"
        "mov %r14, 48(%rdi)\n"
        "mov %r15, 56(%rdi)\n"
        "movsd %xmm0, 64(%rdi)\n"
        "movsd %xmm1, 72(%rdi)\n"
        "movsd %xmm2, 80(%rdi)\n"
        "movsd %xmm3, 88(%rdi)\n"
        "movsd %xmm4, 96(%rdi)\n"
        "movsd %xmm5, 104(%rdi)\n"
        "movsd %xmm6, 112(%rdi)\n"
        "movsd %xmm7, 120(%rdi)\n"
        "xor %eax, %eax");
}
#elif defined(__aarch64__)
int setjmp(jmp_buf env) {
  __asm("stp fp, lr, [x0]\n"
        "mov x9, sp\n"
        "stp x9, x19, [x0, 16]\n"
        "stp x20, x21, [x0, 32]\n"
        "stp x22, x23, [x0, 48]\n"
        "stp x24, x25, [x0, 64]\n"
        "stp x26, x27, [x0, 80]\n"
        "stp x28, x29, [x0, 96]\n"
        "stp d8, d9, [x0, 112]\n"
        "stp d10, d11, [x0, 128]\n"
        "stp d12, d13, [x0, 144]\n"
        "stp d14, d15, [x0, 160]\n"
        "mov w0, wzr");
}
#endif
#endif
