// Public Domain
// Compile with GCC -O3 for best performance

// NOTE: Does NOT handle overlapping memory regions. Use memmove for that (it's better anyways)

#include "memfuncs.h"

// 8-bit (1 bytes at a time)
// Len is (# of total bytes/1), so it's "# of 8-bits"
// Source and destination buffers must both be 1-byte aligned (aka no alignment)

//#include <stdio.h>

void * memcpy_ (void *dest, const void *src, size_t len)
{
  if(!len)
  {
    return dest;
  }

  const uint8_t *s = (uint8_t *)src;
  uint8_t *d = (uint8_t *)dest;

  uint32_t diff = (uint32_t)d - (uint32_t)(s + 1); // extra offset because input gets incremented before output is calculated
  // Underflow would be like adding a negative offset

  // Can use 'd' as a scratch reg now
  asm volatile (
    "clrs\n" // Align for parallelism (CO) - SH4a use "stc SR, Rn" instead with a dummy Rn
  ".align 2\n"
  "0:\n\t"
    "dt %[size]\n\t" // (--len) ? 0 -> T : 1 -> T (EX 1)
    "mov.b @%[in]+, %[scratch]\n\t" // scratch = *(s++) (LS 1/2)
    "bf.s 0b\n\t" // while(s != nexts) aka while(!T) (BR 1/2)
    " mov.b %[scratch], @(%[offset], %[in])\n" // *(datatype_of_s*) ((char*)s + diff) = scratch, where src + diff = dest (LS 1)
    : [in] "+&r" ((uint32_t)s), [scratch] "=&r" ((uint32_t)d), [size] "+&r" (len) // outputs
    : [offset] "z" (diff) // inputs
    : "t", "memory" // clobbers
  );

  return dest;
}

// 16-bit (2 bytes at a time)
// Len is (# of total bytes/2), so it's "# of 16-bits"
// Source and destination buffers must both be 2-byte aligned

void * memcpy_16bit(void *dest, const void *src, size_t len)
{
  if(!len)
  {
    return dest;
  }

  const uint16_t* s = (uint16_t*)src;
  uint16_t* d = (uint16_t*)dest;

  uint32_t diff = (uint32_t)d - (uint32_t)(s + 1); // extra offset because input gets incremented before output is calculated
  // Underflow would be like adding a negative offset

  // Can use 'd' as a scratch reg now
  asm volatile (
    "clrs\n" // Align for parallelism (CO) - SH4a use "stc SR, Rn" instead with a dummy Rn
  ".align 2\n"
  "0:\n\t"
    "dt %[size]\n\t" // (--len) ? 0 -> T : 1 -> T (EX 1)
    "mov.w @%[in]+, %[scratch]\n\t" // scratch = *(s++) (LS 1/2)
    "bf.s 0b\n\t" // while(s != nexts) aka while(!T) (BR 1/2)
    " mov.w %[scratch], @(%[offset], %[in])\n" // *(datatype_of_s*) ((char*)s + diff) = scratch, where src + diff = dest (LS 1)
    : [in] "+&r" ((uint32_t)s), [scratch] "=&r" ((uint32_t)d), [size] "+&r" (len) // outputs
    : [offset] "z" (diff) // inputs
    : "t", "memory" // clobbers
  );

  return dest;
}

// 32-bit (4 bytes at a time)
// Len is (# of total bytes/4), so it's "# of 32-bits"
// Source and destination buffers must both be 4-byte aligned

void * memcpy_32bit(void *dest, const void *src, size_t len)
{
  if(!len)
  {
    return dest;
  }

  const uint32_t* s = (uint32_t*)src;
  uint32_t* d = (uint32_t*)dest;

  uint32_t diff = (uint32_t)d - (uint32_t)(s + 1); // extra offset because input gets incremented before output is calculated
  // Underflow would be like adding a negative offset

  // Can use 'd' as a scratch reg now
  asm volatile (
    "clrs\n" // Align for parallelism (CO) - SH4a use "stc SR, Rn" instead with a dummy Rn
  ".align 2\n"
  "0:\n\t"
    "dt %[size]\n\t" // (--len) ? 0 -> T : 1 -> T (EX 1)
    "mov.l @%[in]+, %[scratch]\n\t" // scratch = *(s++) (LS 1/2)
    "bf.s 0b\n\t" // while(s != nexts) aka while(!T) (BR 1/2)
    " mov.l %[scratch], @(%[offset], %[in])\n" // *(datatype_of_s*) ((char*)s + diff) = scratch, where src + diff = dest (LS 1)
    : [in] "+&r" ((uint32_t)s), [scratch] "=&r" ((uint32_t)d), [size] "+&r" (len) // outputs
    : [offset] "z" (diff) // inputs
    : "t", "memory" // clobbers
  );

  return dest;
}

// 64-bit (8 bytes at a time)
// Len is (# of total bytes/8), so it's "# of 64-bits"
// Source and destination buffers must both be 8-byte aligned

void * memcpy_64bit(void *dest, const void *src, size_t len)
{
  if(!len)
  {
    return dest;
  }

  const _Complex float* s = (_Complex float*)src;
  _Complex float* d = (_Complex float*)dest;

  _Complex float double_scratch;

  uint32_t diff = (uint32_t)d - (uint32_t)(s + 1); // extra offset because input gets incremented before output is calculated
  // Underflow would be like adding a negative offset

  asm volatile (
    "fschg\n\t"
    "clrs\n" // Align for parallelism (CO) - SH4a use "stc SR, Rn" instead with a dummy Rn
  ".align 2\n"
  "0:\n\t"
    "dt %[size]\n\t" // (--len) ? 0 -> T : 1 -> T (EX 1)
    "fmov.d @%[in]+, %[scratch]\n\t" // scratch = *(s++) (LS 1/2)
    "bf.s 0b\n\t" // while(s != nexts) aka while(!T) (BR 1/2)
    " fmov.d %[scratch], @(%[offset], %[in])\n\t" // *(datatype_of_s*) ((char*)s + diff) = scratch, where src + diff = dest (LS 1)
    "fschg\n"
    : [in] "+&r" ((uint32_t)s), [scratch] "=&d" (double_scratch), [size] "+&r" (len) // outputs
    : [offset] "z" (diff) // inputs
    : "t", "memory" // clobbers
  );

  return dest;
}

// 16 Bytes at a time
// Len is (# of total bytes/16), so it's "# of 16 Bytes"
// Source and destination buffers must both be 4-byte aligned

void * memcpy_32bit_16Bytes(void *dest, const void *src, size_t len)
{
  void * ret_dest = dest;

  if(!len)
  {
    return ret_dest;
  }

  uint32_t scratch_reg;
  uint32_t scratch_reg2;
  uint32_t scratch_reg3;
  uint32_t scratch_reg4;

  asm volatile (
    "clrs\n" // Align for parallelism (CO) - SH4a use "stc SR, Rn" instead with a dummy Rn
  ".align 2\n"
  "1:\n\t"
    // *dest++ = *src++
    "mov.l @%[in]+, %[scratch]\n\t" // (LS)
    "mov.l @%[in]+, %[scratch2]\n\t" // (LS)
    "mov.l @%[in]+, %[scratch3]\n\t" // (LS)
    "add #16, %[out]\n\t" // (EX)
    "mov.l @%[in]+, %[scratch4]\n\t" // (LS)
    "dt %[size]\n\t" // while(--len) (EX)
    "mov.l %[scratch4], @-%[out]\n\t" // (LS)
    "mov.l %[scratch3], @-%[out]\n\t" // (LS)
    "mov.l %[scratch2], @-%[out]\n\t" // (LS)
    "mov.l %[scratch], @-%[out]\n\t" // (LS)
    "bf.s 1b\n\t" // (BR)
    " add #16, %[out]\n" // (EX)
    : [in] "+&r" ((uint32_t)src), [out] "+&r" ((uint32_t)dest), [size] "+&r" (len),
    [scratch] "=&r" (scratch_reg), [scratch2] "=&r" (scratch_reg2), [scratch3] "=&r" (scratch_reg3), [scratch4] "=&r" (scratch_reg4) // outputs
    : // inputs
    : "t", "memory" // clobbers
  );

  return ret_dest;
}

// 32 Bytes at a time
// Len is (# of total bytes/32), so it's "# of 32 Bytes"
// Source and destination buffers must both be 8-byte aligned
// NOTE: Store Queues are better, though they require 32-byte alignment. This is
// just here for if you really need it.

void * memcpy_64bit_32Bytes(void *dest, const void *src, size_t len)
{
  void * ret_dest = dest;

  if(!len)
  {
    return ret_dest;
  }

  _Complex float double_scratch;
  _Complex float double_scratch2;
  _Complex float double_scratch3;
  _Complex float double_scratch4;

  asm volatile (
    "fschg\n\t" // Switch to pair move mode (FE)
    "clrs\n" // Align for parallelism (CO) - SH4a use "stc SR, Rn" instead with a dummy Rn
  ".align 2\n"
  "1:\n\t"
    // *dest++ = *src++
    "pref @%[out]\n\t" // pref dest (LS)
    "fmov.d @%[in]+, %[scratch]\n\t" // (LS)
    "fmov.d @%[in]+, %[scratch2]\n\t" // (LS)
    "fmov.d @%[in]+, %[scratch3]\n\t" // (LS)
    "fmov.d @%[in]+, %[scratch4]\n\t" // (LS)
    "add #32, %[out]\n\t" // (EX)
    "pref @%[in]\n\t" // pref the next iteration of src (LS)
    "dt %[size]\n\t" // while(--len) (EX)
    "fmov.d %[scratch4], @-%[out]\n\t" // (LS)
    "fmov.d %[scratch3], @-%[out]\n\t" // (LS)
    "fmov.d %[scratch2], @-%[out]\n\t" // (LS)
    "fmov.d %[scratch], @-%[out]\n\t" // (LS)
    "bf.s 1b\n\t" // (BR)
    " add #32, %[out]\n\t" // (EX)
    "fschg\n" // Switch back to single move mode (FE)
    : [in] "+&r" ((uint32_t)src), [out] "+&r" ((uint32_t)dest), [size] "+&r" (len),
    [scratch] "=&d" (double_scratch), [scratch2] "=&d" (double_scratch2), [scratch3] "=&d" (double_scratch3), [scratch4] "=&d" (double_scratch4) // outputs
    : // inputs
    : "t", "memory" // clobbers
  );

  return ret_dest;
}
