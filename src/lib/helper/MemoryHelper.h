// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_HELPER_MEMORYHELPER_H_
#define SRC_LIB_HELPER_MEMORYHELPER_H_

/* reallocs space and fills with value if needed */
void *realloc_and_fill(size_t fill, void *pBuffer, size_t oldSize, size_t newSize) {
  void *pNew = realloc(pBuffer, newSize);

  if (newSize > oldSize && pNew) {
    size_t diff = newSize - oldSize;
    void *pStart = ((char *)pNew) + oldSize;
    memset(pStart, fill, diff);
  }

  return pNew;
}

/* round up to the nearest multiple of 8 */
size_t round_to_8(size_t bits) {
  if (bits % 8 != 0) {
    bits += 8 - (bits % 8);
  }

  return bits;
}

#endif  // SRC_LIB_HELPER_MEMORYHELPER_H_
