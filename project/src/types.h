#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "cvector/cvector.h"
#include "hashmap/hashmap.h"


typedef map_t SupportMap;

typedef cvector_vector_type(char) Item;
typedef cvector_vector_type(Item) Transaction;
typedef cvector_vector_type(Transaction) TransactionsList;


#endif
