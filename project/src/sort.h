#ifndef SORT_H
#define SORT_H

#include "hashmap/hashmap.h"
#include "types.h"

void sort(uint8_t **keys, int *sorted_indices, SupportMap support_map, int start, int end);

#endif