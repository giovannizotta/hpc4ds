/**
 * @file utils.h
 * @brief Utility functions
 * 
 */
#ifndef UTILS_H
#define UTILS_H

/**
 * @brief Minimum between two integers
 *
 * @param a first parameter
 * @param b second parameter
 * @return minimum between a and b
 */
int min(int a, int b);

/**
 * @brief Maximum between two integers
 *
 * @param a first parameter
 * @param b second parameter
 * @return maximum between a and b
 */
int max(int a, int b);

/**
 * @brief Length of an Item
 *
 * @param s Item
 * @return Number of characters in s
 */
int ulength(uint8_t *s);

#endif