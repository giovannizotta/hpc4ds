#ifndef TYPES_H
#define TYPES_H

#include "cvector/cvector.h"
#include "hashmap/hashmap.h"
#include <stdbool.h>

typedef map_t SupportMap;
typedef map_t IndexMap;

typedef cvector_vector_type(uint8_t) Item;
typedef cvector_vector_type(Item) Transaction;
typedef cvector_vector_type(Transaction) TransactionsList;

#endif
