#ifndef SORT_H
#define SORT_H

#include "hashmap/hashmap.h"
#include "types.h"

void sort(uint8_t **keys, int *sorted_indices, SupportMap support_map, int start, int end, int num_threads);

int pivot(uint8_t **keys, int *sorted_indices, int start, int end, int m,
          SupportMap support_map);

int select_m(uint8_t **keys, int *sorted_indices, int start, int end, int k,
             SupportMap support_map);

void insertion_sort(uint8_t **keys, int *sorted_indices, int start, int end,
                    SupportMap support_map);
#endif