#include "types.h"
#include <string.h>

/**
 * @brief Minimum between two integers
 *
 * @param a first parameter
 * @param b second parameter
 * @return minimum between a and b
 */
int min(int a, int b) { return a < b ? a : b; }

/**
 * @brief Maximum between two integers
 *
 * @param a first parameter
 * @param b second parameter
 * @return maximum between a and b
 */
int max(int a, int b) { return a < b ? b : a; }

/**
 * @brief Length of an Item
 *
 * @param s Item
 * @return Number of characters in s
 */
int ulength(uint8_t *s) { return strlen((char *)s) + 1; }