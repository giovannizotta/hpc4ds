#include "types.h"
#include <string.h>

int min(int a, int b) { return a < b ? a : b; }

int max(int a, int b) { return a < b ? b : a; }

int ulength(uint8_t *s) { return strlen((char *)s) + 1; }