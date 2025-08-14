// Public Domain
// Compile with GCC -O3 for best performance

// In standard comparison versions, -1 -> str1 is less, 0 -> equal, 1 -> str1 is greater.
// In equality-only versions, a return value of 0 means equal, -1 means unequal.

#include "memfuncs.h"

int memcmp_ (const void *str1, const void *str2, size_t count)
{
  const uint8_t *s1 = (uint8_t *)str1;
  const uint8_t *s2 = (uint8_t *)str2;

  while (count--)
  {
    if (*s1++ != *s2++)
    {
      return s1[-1] < s2[-1] ? -1 : 1;
    }
  }
  return 0;
}

// Equality-only version
int memcmp_eq (const void *str1, const void *str2, size_t count)
{
  const uint8_t *s1 = (uint8_t *)str1;
  const uint8_t *s2 = (uint8_t *)str2;

  while (count--)
  {
    if (*s1++ != *s2++)
    {
      return -1; // Makes more sense to me if -1 means unequal.
    }
  }
  return 0; // Return 0 if equal to match normal memcmp
}

// 16-bit (2 bytes at a time)
// Count is (# of total bytes/2), so it's "# of 16-bits"

int memcmp_16bit(const void *str1, const void *str2, size_t count)
{
  const uint16_t *s1 = (uint16_t*)str1;
  const uint16_t *s2 = (uint16_t*)str2;

  while (count--)
  {
    if (*s1++ != *s2++)
    {
      return s1[-1] < s2[-1] ? -1 : 1;
    }
  }
  return 0;
}

// Equality-only version
int memcmp_16bit_eq(const void *str1, const void *str2, size_t count)
{
  const uint16_t *s1 = (uint16_t*)str1;
  const uint16_t *s2 = (uint16_t*)str2;

  while (count--)
  {
    if (*s1++ != *s2++)
    {
      return -1;
    }
  }
  return 0;
}

// 32-bit (4 bytes at a time - 1 pixel in a 32-bit linear frame buffer)
// Count is (# of total bytes/4), so it's "# of 32-bits"

int memcmp_32bit(const void *str1, const void *str2, size_t count)
{
  const uint32_t *s1 = (uint32_t*)str1;
  const uint32_t *s2 = (uint32_t*)str2;

  while (count--)
  {
    if (*s1++ != *s2++)
    {
      return s1[-1] < s2[-1] ? -1 : 1;
    }
  }
  return 0;
}

// Equality-only version
int memcmp_32bit_eq(const void *str1, const void *str2, size_t count)
{
  const uint32_t *s1 = (uint32_t*)str1;
  const uint32_t *s2 = (uint32_t*)str2;

  while (count--)
  {
    if (*s1++ != *s2++)
    {
      return -1;
    }
  }
  return 0;
}

//
// SH4 has a cmp/str instruction, which checks for byte-corresponding inequality
// (e.g. ignoring null-termination, "ABCD" vs. "WXYZ" or 0x12345678 vs. 0x87654321,
// but not "BAND" vs. "CANS" or 0x55559999 vs. 0x66567777). It makes for a kind of
// an "inverse memcmp."
//

// If str1 and str2 have no corresponding bytes, return the total length that was
// compared, in bytes (= count * 4). Otherwise return the byte offset (array index)
// at which the first equality was found.
// Count is (# of total bytes/4), so it's "# of 32-bits"

int inv_memcmp_32bit(const void *str1, const void *str2, size_t count)
{
  if(!count)
  {
    return count;
  }

  uint32_t sizetmp;
  uint32_t str1tmp;
  register uint32_t str2tmp __asm__("r0"); // Need this to be r0

  asm volatile (
    "clrs\n" // SR.S bit is not preserved across function calls (CO to force parallelism to start here)
    // SH4a needs a different one since 'clrs' is CO on SH4 only - "stc SR, Rn" (just make Rn some dummy
    // variable, e.g. "stc SR, %[str_1_tmp]") is as fast on SH4a as 'clrs' is on SH4, and is CO on both
  ".align 2\n\t"
    "mov %[size], %[size_tmp]\n\t" // sizetmp = count (MT)
    "add #1, %[size_tmp]\n" // dt is a pre-dec (EX)
  "1:\n\t" // while(sizetmp-- && (*str1++ != *str2++))
    "mov.l @%[str_1]+, %[str_1_tmp]\n\t" // str1tmp = *str1++ (LS)
    "dt %[size_tmp]\n\t" // (--sizetmp == 0) ? 1 -> T : 0 -> T (EX)
    "mov.l @%[str_2]+, %[str_2_tmp]\n\t" // str2tmp = *str2++ (LS)
    "nop\n\t" // maintain parallelism (MT)
    "bt.s 3f\n\t" // size ran out (sizetmp = 0 now) (BR)
    " cmp/str %[str_1_tmp], %[str_2_tmp]\n\t" // (any byte in str1tmp == corresponding byte in str2tmp) ? 0 -> T : 1 -> T (MT)
    "bf.s 1b\n\t" // nope (BR)
    // cmp/str found an equal byte
    " xor %[str_1_tmp], %[str_2_tmp]\n\t" // yep, which one? str2tmp ^= str1tmp (EX)
    "shll2 %[size_tmp]\n\t" // turn sizetmp into a byte offset to subtract count by (size_tmp >= 1 here always) (EX)
#ifdef BigEnd
    "add #-4, %[size_tmp]\n\t" // on big endian 0x01020304 is stored as 0x01 0x02 0x03 0x04 in memory and we want a byte offset (EX - will recombine with MT below)
#endif
    "nop\n" // Try and keep the parallelism going (MT)
  "2:\n\t"
    "nop\n\t" // Try and keep the parallelism going (MT - can be parallelized with MT)
    "tst #0xff, %[str_2_tmp]\n\t" // If the lower byte is 0, 1 -> T and sizetmp holds the offset to subtract count by (MT)
    "bt.s 3f\n\t" // (BR)
    " shlr8 %[str_2_tmp]\n\t" // next byte (EX)
    "bra 2b\n\t" // (BR)
#ifdef BigEnd
    " add #1, %[size_tmp]\n" // increment sizetmp (big endian) (EX)
#else
    " add #-1, %[size_tmp]\n" // decrement sizetmp (little endian) (EX)
#endif
  "3:\n\t" // all done
    "shll2 %[size]\n\t" // turn count into a byte total (EX)
    "sub %[size_tmp], %[size]\n" // count -= sizetmp (EX)
    : [size] "+&r" (count), [str_1] "+&r" ((uint32_t)str1), [str_2] "+&r" ((uint32_t)str2), [str_1_tmp] "=&r" (str1tmp), [str_2_tmp] "=&z" (str2tmp), [size_tmp] "=&r" (sizetmp) // outputs
    : // inputs
    : "t" // clobbers
  );

  return count;
}

// Equality-only version:
// Return value of 0 means the inputs have no corresponding bytes
// Return value of -1 means the inputs have at least one corresponding byte somewhere
int inv_memcmp_32bit_eq(const void *str1, const void *str2, size_t count)
{
  if(!count)
  {
    return count;
  }

  uint32_t str1tmp;
  uint32_t str2tmp;

  asm volatile (
    "clrs\n" // SR.S bit is not preserved across function calls (CO to force parallelism to start here)
    // SH4a needs a different one since 'clrs' is CO on SH4 only - "stc SR, Rn" (just make Rn some dummy
    // variable, e.g. "stc SR, %[str_1_tmp]") is as fast on SH4a as 'clrs' is on SH4, and is CO on both
  ".align 2\n\t"
    "add #1, %[size]\n\t" // dt is a pre-dec (EX)
    "mov.l @%[str_1]+, %[str_1_tmp]\n" // str1tmp = *str1++ (LS)
  "1:\n\t" // while(count-- && (*str1++ != *str2++))
    "dt %[size]\n\t" // (--count == 0) ? 1 -> T : 0 -> T (EX)
    "mov.l @%[str_2]+, %[str_2_tmp]\n\t" // str2tmp = *str2++ (LS)
    "bt.s 2f\n\t" // size ran out (count = 0 now) (BR)
    " cmp/str %[str_1_tmp], %[str_2_tmp]\n\t" // (any byte in str1tmp == corresponding byte in str2tmp) ? 0 -> T : 1 -> T (MT)
    "bf.s 1b\n\t" // nope (BR)
    " mov.l @%[str_1]+, %[str_1_tmp]\n\t" // str1tmp = *str1++ (LS)
    "mov #-1, %[size]\n" // yep, done (EX)
  "2:\n"
    // cmp/str found an equal byte
    : [size] "+&r" (count), [str_1] "+&r" ((uint32_t)str1), [str_2] "+&r" ((uint32_t)str2), [str_1_tmp] "=&r" (str1tmp), [str_2_tmp] "=&r" (str2tmp) // outputs
    : // inputs
    : "t" // clobbers
  );

  return count;
}
