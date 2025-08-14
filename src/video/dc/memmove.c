// Public Domain
// Compile with GCC -O3 for best performance

// Handles overlapping memory areas, too!

#include "memfuncs.h"

// Default (8-bit, 1 byte at a time)
void * memmove_ (void *dest, const void *src, size_t len)
{
  if(!len)
  {
    return dest;
  }

  const uint8_t *s = (uint8_t *)src;
  uint8_t *d = (uint8_t *)dest;

  if (s > d)
  {
    uint32_t diff = (uint32_t)d - (uint32_t)(s + 1); // extra offset because input gets incremented before output is calculated
    // This will underflow and be like adding a negative offset

    // Can use 'd' as a scratch reg now
    asm volatile (
      "clrs\n" // Align for parallelism (CO) - SH4a use "stc SR, Rn" instead with a dummy Rn
    ".align 2\n"
    "0:\n\t"
      "dt %[size]\n\t" // (--len) ? 0 -> T : 1 -> T (EX 1)
      "mov.b @%[in]+, %[scratch]\n\t" // scratch = *(s++) (LS 1/2)
      "bf.s 0b\n\t" // while(s != nexts) aka while(!T) (BR 1/2)
      " mov.b %[scratch], @(%[offset], %[in])\n" // *(datatype_of_s*) ((char*)s + diff) = scratch, where src + diff = dest (LS)
      : [in] "+&r" ((uint32_t)s), [scratch] "=&r" ((uint32_t)d), [size] "+&r" (len) // outputs
      : [offset] "z" (diff) // inputs
      : "t", "memory" // clobbers
    );
  }
  else // s < d
  {
    uint8_t *nextd = d + len;

    uint32_t diff = (uint32_t)s - (uint32_t)(d + 1); // extra offset because input calculation occurs before output is decremented
    // This will underflow and be like adding a negative offset

    // Can use 's' as a scratch reg now
    asm volatile (
      "clrs\n" // Align for parallelism (CO) - SH4a use "stc SR, Rn" instead with a dummy Rn
    ".align 2\n"
    "0:\n\t"
      "dt %[size]\n\t" // (--len) ? 0 -> T : 1 -> T (EX 1)
      "mov.b @(%[offset], %[out_end]), %[scratch]\n\t" // scratch = *(--nexts) where --nexts is nextd + underflow result (LS 2)
      "bf.s 0b\n\t" // while(nextd != d) aka while(!T) (BR 1/2)
      " mov.b %[scratch], @-%[out_end]\n" // *(--nextd) = scratch (LS 1/1)
      : [out_end] "+&r" ((uint32_t)nextd), [scratch] "=&r" ((uint32_t)s), [size] "+&r" (len) // outputs
      : [offset] "z" (diff) // inputs
      : "t", "memory" // clobbers
    );
  }

  return dest;
}

// 16-bit (2 bytes at a time)
// Len is (# of total bytes/2), so it's "# of 16-bits"
// Source and destination buffers must both be 2-byte aligned

void * memmove_16bit(void *dest, const void *src, size_t len)
{
  if(!len)
  {
    return dest;
  }

  const uint16_t* s = (uint16_t*)src;
  uint16_t* d = (uint16_t*)dest;

  if (s > d)
  {
    uint32_t diff = (uint32_t)d - (uint32_t)(s + 1); // extra offset because input gets incremented before output is calculated
    // This will underflow and be like adding a negative offset

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
  }
  else // s < d
  {
    uint16_t *nextd = d + len;

    uint32_t diff = (uint32_t)s - (uint32_t)(d + 1); // extra offset because input calculation occurs before output is decremented
    // This will underflow and be like adding a negative offset

    // Can use 's' as a scratch reg now
    asm volatile (
      "clrs\n" // Align for parallelism (CO) - SH4a use "stc SR, Rn" instead with a dummy Rn
    ".align 2\n"
    "0:\n\t"
      "dt %[size]\n\t" // (--len) ? 0 -> T : 1 -> T (EX 1)
      "mov.w @(%[offset], %[out_end]), %[scratch]\n\t" // scratch = *(--nexts) where --nexts is nextd + underflow result (LS 2)
      "bf.s 0b\n\t" // while(nextd != d) aka while(!T) (BR 1/2)
      " mov.w %[scratch], @-%[out_end]\n" // *(--nextd) = scratch (LS 1/1)
      : [out_end] "+&r" ((uint32_t)nextd), [scratch] "=&r" ((uint32_t)s), [size] "+&r" (len) // outputs
      : [offset] "z" (diff) // inputs
      : "t", "memory" // clobbers
    );
  }

  return dest;
}

// 32-bit (4 bytes at a time - 1 pixel in a 32-bit linear frame buffer)
// Len is (# of total bytes/4), so it's "# of 32-bits"
// Source and destination buffers must both be 4-byte aligned

void * memmove_32bit(void *dest, const void *src, size_t len)
{
  if(!len)
  {
    return dest;
  }

  const uint32_t* s = (uint32_t*)src;
  uint32_t* d = (uint32_t*)dest;

  if (s > d)
  {
    uint32_t diff = (uint32_t)d - (uint32_t)(s + 1); // extra offset because input gets incremented before output is calculated
    // This will underflow and be like adding a negative offset

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
  }
  else // s < d
  {
    uint32_t *nextd = d + len;

    uint32_t diff = (uint32_t)s - (uint32_t)(d + 1); // extra offset because input calculation occurs before output is decremented
    // This will underflow and be like adding a negative offset

    // Can use 's' as a scratch reg now
    asm volatile (
      "clrs\n" // Align for parallelism (CO) - SH4a use "stc SR, Rn" instead with a dummy Rn
    ".align 2\n"
    "0:\n\t"
      "dt %[size]\n\t" // (--len) ? 0 -> T : 1 -> T (EX 1)
      "mov.l @(%[offset], %[out_end]), %[scratch]\n\t" // scratch = *(--nexts) where --nexts is nextd + underflow result (LS 2)
      "bf.s 0b\n\t" // while(nextd != d) aka while(!T) (BR 1/2)
      " mov.l %[scratch], @-%[out_end]\n" // *(--nextd) = scratch (LS 1/1)
      : [out_end] "+&r" ((uint32_t)nextd), [scratch] "=&r" ((uint32_t)s), [size] "+&r" (len) // outputs
      : [offset] "z" (diff) // inputs
      : "t", "memory" // clobbers
    );
  }

  return dest;
}

// 64-bit (8 bytes at a time)
// Len is (# of total bytes/8), so it's "# of 64-bits"
// Source and destination buffers must both be 8-byte aligned

void * memmove_64bit(void *dest, const void *src, size_t len)
{
  if(!len)
  {
    return dest;
  }

  const _Complex float* s = (_Complex float*)src;
  _Complex float* d = (_Complex float*)dest;

  _Complex float double_scratch;

  if (s > d)
  {
    uint32_t diff = (uint32_t)d - (uint32_t)(s + 1); // extra offset because input gets incremented before output is calculated
    // This will underflow and be like adding a negative offset

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
  }
  else // s < d
  {
    _Complex float *nextd = d + len;

    uint32_t diff = (uint32_t)s - (uint32_t)(d + 1); // extra offset because input calculation occurs before output is decremented
    // This will underflow and be like adding a negative offset

    asm volatile (
      "fschg\n\t"
      "clrs\n" // Align for parallelism (CO) - SH4a use "stc SR, Rn" instead with a dummy Rn
    ".align 2\n"
    "0:\n\t"
      "dt %[size]\n\t" // (--len) ? 0 -> T : 1 -> T (EX 1)
      "fmov.d @(%[offset], %[out_end]), %[scratch]\n\t" // scratch = *(--nexts) where --nexts is nextd + underflow result (LS 2)
      "bf.s 0b\n\t" // while(nextd != d) aka while(!T) (BR 1/2)
      " fmov.d %[scratch], @-%[out_end]\n\t" // *(--nextd) = scratch (LS 1/1)
      "fschg\n"
      : [out_end] "+&r" ((uint32_t)nextd), [scratch] "=&d" (double_scratch), [size] "+&r" (len) // outputs
      : [offset] "z" (diff) // inputs
      : "t", "memory" // clobbers
    );
  }

  return dest;
}
