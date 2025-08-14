// Public Domain
// Compile with GCC -O3 for best performance

#include "memfuncs.h"

// Set 1 byte at a time

void * memset_ (void *dest, const uint8_t val, size_t len)
{
  if(!len)
  {
    return dest;
  }

  uint8_t * d = (uint8_t*)dest;
  uint8_t * nextd = d + len;

  asm volatile (
    "clrs\n\t" // Align for parallelism (CO) - SH4a use "stc SR, Rn" instead with a dummy Rn
    "dt %[size]\n" // Decrement and test size here once to prevent extra jump (EX 1)
  ".align 2\n"
  "1:\n\t"
    // *--nextd = val
    "mov.b %[in], @-%[out]\n\t" // (LS 1/1)
    "bf.s 1b\n\t" // (BR 1/2)
    " dt %[size]\n" // (--len) ? 0 -> T : 1 -> T (EX 1)
    : [out] "+r" ((uint32_t)nextd), [size] "+&r" (len) // outputs
    : [in] "r" (val) // inputs
    : "t", "memory" // clobbers
  );

  return dest;
}

// Set 2 bytes at a time
// Len is (# of total bytes/2), so it's "# of 16-bits"
// Destination must be 2-byte aligned

void * memset_16bit(void *dest, const uint16_t val, size_t len)
{
  if(!len)
  {
    return dest;
  }

  uint16_t * d = (uint16_t*)dest;
  uint16_t * nextd = d + len;

  asm volatile (
    "clrs\n\t" // Align for parallelism (CO) - SH4a use "stc SR, Rn" instead with a dummy Rn
    "dt %[size]\n" // Decrement and test size here once to prevent extra jump (EX 1)
  ".align 2\n"
  "1:\n\t"
    // *--nextd = val
    "mov.w %[in], @-%[out]\n\t" // (LS 1/1)
    "bf.s 1b\n\t" // (BR 1/2)
    " dt %[size]\n" // (--len) ? 0 -> T : 1 -> T (EX 1)
    : [out] "+r" ((uint32_t)nextd), [size] "+&r" (len) // outputs
    : [in] "r" (val) // inputs
    : "t", "memory" // clobbers
  );

  return dest;
}


// Set 4 bytes at a time
// Len is (# of total bytes/4), so it's "# of 32-bits"
// Destination must be 4-byte aligned

void * memset_32bit(void *dest, const uint32_t val, size_t len)
{
  if(!len)
  {
    return dest;
  }

  uint32_t * d = (uint32_t*)dest;
  uint32_t * nextd = d + len;

  asm volatile (
    "clrs\n\t" // Align for parallelism (CO) - SH4a use "stc SR, Rn" instead with a dummy Rn
    "dt %[size]\n" // Decrement and test size here once to prevent extra jump (EX 1)
  ".align 2\n"
  "1:\n\t"
    // *--nextd = val
    "mov.l %[in], @-%[out]\n\t" // (LS 1/1)
    "bf.s 1b\n\t" // (BR 1/2)
    " dt %[size]\n" // (--len) ? 0 -> T : 1 -> T (EX 1)
    : [out] "+r" ((uint32_t)nextd), [size] "+&r" (len) // outputs
    : [in] "r" (val) // inputs
    : "t", "memory" // clobbers
  );

  return dest;
}

// 32-bit input --> 64-bit output (8 bytes at a time from 4-byte input)
// Len is (# of total bytes/8), so it's "# of 64-bits"
// Takes 32-bit data as input and does two writes at a time
// Destination must be 8-byte aligned

void * memset_64bit_4B(void *dest, const uint32_t val, size_t len)
{
  if(!len)
  {
    return dest;
  }

  _Complex float * d = (_Complex float*)dest;
  _Complex float * nextd = d + len;

  asm volatile (
    "lds %[in], fpul\n\t" // (LS)
    "fsts fpul, fr0\n\t"
    "fsts fpul, fr1\n\t"
    "fschg\n\t" // Switch to pair move mode (FE)
    "dt %[size]\n\t" // Decrement and test size here once to prevent extra jump (EX 1)
    "clrs\n" // Align for parallelism (CO) - SH4a use "stc SR, Rn" instead with a dummy Rn
  ".align 2\n"
  "1:\n\t"
    // *--nextd = val
    "fmov.d DR0, @-%[out]\n\t" // (LS 1/1)
    "bf.s 1b\n\t" // (BR 1/2)
    " dt %[size]\n\t" // (--len) ? 0 -> T : 1 -> T (EX 1)
    "fschg\n" // Switch back to single move mode (FE)
    : [out] "+r" ((uint32_t)nextd), [size] "+&r" (len) // outputs
    : [in] "r" (val) // inputs
    : "t", "fr0", "fr1", "memory" // clobbers
  );

  return dest;
}

// Set 4 bytes of 0 at a time
// Len is (# of total bytes/4), so it's "# of 32-bits"
// Destination must be 4-byte aligned

void * memset_zeroes_32bit(void *dest, size_t len)
{
  if(!len)
  {
    return dest;
  }

  float * d = (float*)dest;
  float * nextd = d + len;

  float float_scratch;

  asm volatile (
    "fldi0 %[scratch]\n\t"
    "dt %[size]\n\t" // Decrement and test size here once to prevent extra jump (EX 1)
    "clrs\n" // Align for parallelism (CO) - SH4a use "stc SR, Rn" instead with a dummy Rn
  ".align 2\n"
  "1:\n\t"
    // *--nextd = val
    "fmov.s %[scratch], @-%[out]\n\t" // (LS 1/1)
    "bf.s 1b\n\t" // (BR 1/2)
    " dt %[size]\n" // (--len) ? 0 -> T : 1 -> T (EX 1)
    : [out] "+r" ((uint32_t)nextd), [scratch] "=&f" (float_scratch), [size] "+&r" (len) // outputs
    : // inputs
    : "t", "memory" // clobbers
  );

  return dest;
}

// Set 8 bytes of 0 at a time
// Len is (# of total bytes/8), so it's "# of 64-bits"
// Destination must be 8-byte aligned

void * memset_zeroes_64bit(void *dest, size_t len)
{
  if(!len)
  {
    return dest;
  }

  _Complex float * d = (_Complex float*)dest;
  _Complex float * nextd = d + len;

  asm volatile (
    "fldi0 fr0\n\t"
    "fldi0 fr1\n\t"
    "fschg\n\t" // Switch to pair move mode (FE)
    "dt %[size]\n\t" // Decrement and test size here once to prevent extra jump (EX 1)
    "clrs\n" // Align for parallelism (CO) - SH4a use "stc SR, Rn" instead with a dummy Rn
  ".align 2\n"
  "1:\n\t"
    // *--nextd = val
    "fmov.d DR0, @-%[out]\n\t" // (LS 1/1)
    "bf.s 1b\n\t" // (BR 1/2)
    " dt %[size]\n\t" // (--len) ? 0 -> T : 1 -> T (EX 1)
    "fschg\n" // Switch back to single move mode (FE)
    : [out] "+r" ((uint32_t)nextd), [size] "+&r" (len) // outputs
    : // inputs
    : "t", "fr0", "fr1", "memory" // clobbers
  );

  return dest;
}
