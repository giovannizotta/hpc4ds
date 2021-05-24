#ifndef SORT_H
#define SORT_H

#include "hashmap/hashmap.h"
#include "types.h"

void sort(hashmap_element *items_count, int num_items, int *sorted_indices, int start, int end, int num_threads);

int pivot(hashmap_element *items_count, int num_items, int *sorted_indices, int start, int end, int m);

int select_m(hashmap_element *items_count, int num_items, int *sorted_indices, int start, int end, int k);

void insertion_sort(hashmap_element *items_count, int num_items, int *sorted_indices, int start, int end);
#endif