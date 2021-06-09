/**
 * @file types.h
 * @brief Definition of datatypes used by the program 
 * 
 */
#ifndef TYPES_H
#define TYPES_H

#include "cvector/cvector.h"
#include "hashmap/hashmap.h"
#include <stdbool.h>

/**
 * @brief Map from Item to the corresponding support
 */
typedef map_t SupportMap;
/**
 * @brief Map from Item to its id
 */
typedef map_t IndexMap;

/**
 * @brief Item of a transaction
 */
typedef cvector_vector_type(uint8_t) Item;
/**
 * @brief Transaction of items
 */
typedef cvector_vector_type(Item) Transaction;
/**
 * @brief List of transactions
 */
typedef cvector_vector_type(Transaction) TransactionsList;

#endif
