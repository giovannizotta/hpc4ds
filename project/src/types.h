#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "cvector/cvector.h"
#include "uthash/uthash.h"

typedef struct item_count {
        const char *item;          /* key */
        int count;
        UT_hash_handle hh;         /* makes this structure hashable */
} item_count;

typedef item_count* SupportMap;

typedef cvector_vector_type(char) Item;
typedef cvector_vector_type(Item) Transaction;
typedef cvector_vector_type(Transaction) TransactionsList;


#endif
