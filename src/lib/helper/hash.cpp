// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include "hash.h"

u_int64_t fnv_64a_int(u_int64_t hash, u_int64_t integer, size_t size) {
  switch (size) {
    case 8:
      hash = FNV_64A_OP(hash, integer);
      integer >>= 8;

    /*FALLTHRU*/
    case 7:
      hash = FNV_64A_OP(hash, integer);
      integer >>= 8;

    /*FALLTHRU*/
    case 6:
      hash = FNV_64A_OP(hash, integer);
      integer >>= 8;

    /*FALLTHRU*/
    case 5:
      hash = FNV_64A_OP(hash, integer);
      integer >>= 8;

    /*FALLTHRU*/
    case 4:
      hash = FNV_64A_OP(hash, integer);
      integer >>= 8;

    /*FALLTHRU*/
    case 3:
      hash = FNV_64A_OP(hash, integer);
      integer >>= 8;

    /*FALLTHRU*/
    case 2:
      hash = FNV_64A_OP(hash, integer);
      integer >>= 8;

    /*FALLTHRU*/
    case 1:
      hash = FNV_64A_OP(hash, integer);
      break;

    default:
      fprintf(stderr, "fnv_64a_int: BOGUS SIZE: %ld\n", size);
      exit(1);
  }

  return hash;
}
