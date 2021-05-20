#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h> 
#include "cvector/cvector.h"
#include "hashmap/hashmap.h"


typedef map_t SupportMap;

typedef cvector_vector_type(char) Item;
typedef cvector_vector_type(Item) Transaction;
typedef cvector_vector_type(Transaction) TransactionsList;


#endif
