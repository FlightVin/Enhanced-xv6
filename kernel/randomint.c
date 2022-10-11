#ifndef _RANDINT_H_
#define _RANDINT_H_
#include "types.h"
/* Written in 2019 by David Blackman and Sebastiano Vigna (vigna@acm.org)
To the extent possible under law, the author has dedicated all copyright
and related and neighboring rights to this software to the public domain
worldwide. This software is distributed without any warranty.
See <http://creativecommons.org/publicdomain/zero/1.0/>. */

/* This is xoshiro128++ 1.0, one of our 32-bit all-purpose, rock-solid
generators. It has excellent speed, a state size (128 bits) that is
large enough for mild parallelism, and it passes all tests we are aware
of.
For generating just single-precision (i.e., 32-bit) floating-point
numbers, xoshiro128+ is even faster.
The state must be seeded so that it is not everywhere zero. */

static inline uint rotl(const uint x, int k)
{
    return (x << k) | (x >> (32 - k));
}

static uint s[4] = {1, 1, 1, 1};

uint next(void)
{
    const uint result = rotl(s[0] + s[3], 7) + s[0];

    const uint t = s[1] << 9;

    s[2] ^= s[0];
    s[3] ^= s[1];
    s[1] ^= s[2];
    s[0] ^= s[3];

    s[2] ^= t;

    s[3] = rotl(s[3], 11);

    return result;
}

uint32 rand_range(uint32 min, uint32 max) {
	if (min > max)
		return 0;
	else if (min == max)
		return min;
	else {
		return (next() % (max - min) )+ min;
	}
}

#endif