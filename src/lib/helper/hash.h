// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <sys/types.h>


#define FNV1_64_PRIME ((u_int64_t)0x100000001b3ULL)
#define FNV1_64_INIT ((u_int64_t)0xcbf29ce484222325ULL)

#define FNV_64A_OP(hash, octet)                                 \
  (((u_int64_t)(hash) ^ (u_int8_t)(octet)) * FNV1_64_PRIME)

/*
 * NOTE: This is for illustration purposes.  Optimization,
 *       if needed, is left as an exercise for the reader.  :-)
 */
#define FNV_64A_INT(hash, integer)                                      \
  fnv_64a_int((u_int64_t)(hash), (u_int64_t)(integer), sizeof(integer))

/*
 * fnv_64a_int - hash an integer of up to 8 octets in size
 *
 * given:
 *hashcurrent 64 bit FNV-1a hash value
 *integerinteger value to hash
 *sizesize of the integer in octets (bytes)
 *
 * returns:
 *64 bit FNV-1a hash of the integer in low to high byte order
 */
u_int64_t
fnv_64a_int(u_int64_t hash, u_int64_t integer, size_t size);

