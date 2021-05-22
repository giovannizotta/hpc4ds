#include "types.h"
#include <omp.h>
#include <stdio.h>
#include <string.h>

#define INSERTION_SORT_THRESH 100

void insertion_sort(uint8_t **keys, int *sorted_indices, int start, int end,
                    SupportMap support_map) {
    printf("Insertion sort %d-%d!!!\n", start, end);
    // printf("-------------\n");
    // hashmap_print(support_map);
    // printf("-------------\n");

    int i, j, i_key_length, j_key_length, i_val, j_val;
    for (i = start + 1; i <= end; i++) {
        // uint8_t key[KEY_STATIC_LENGTH];
        i_key_length = strlen(keys[i]) + 1;
        // memcpy(key, keys[i], i_key_length); // tmp = v[i]
        bool stop = false;
        if (hashmap_get(support_map, keys[sorted_indices[i]], i_key_length,
                        &i_val) == MAP_MISSING) {
            printf("ATTENTION!!!! %s MISSING!!!\n", keys[sorted_indices[i]]);
            assert(false);
        } else {
            printf("%s OK\n", keys[sorted_indices[i]]);
        }
        for (j = i; j > 0 && !stop; j--) {
            j_key_length = strlen(keys[sorted_indices[j - 1]]) + 1;
            // assert(hashmap_get(support_map, keys[j-1], j_key_length, &j_val)
            // !=
            //        MAP_MISSING);
            if (hashmap_get(support_map, keys[sorted_indices[j - 1]],
                            j_key_length, &j_val) == MAP_MISSING) {
                printf("--- ATTENTION!!!! %s MISSING!!!\n", keys[j - 1]);
                assert(false);
            } else {
                printf("\t%s OK\n", keys[sorted_indices[j - 1]]);
            }
            // printf("###### %d %d\n", i_val, j_val);
            if (j_val > i_val) {
                sorted_indices[j] = sorted_indices[j - 1];
                // memcpy(keys[j], keys[j - 1], j_key_length);
            } else {
                stop = true;
            }
        }
        sorted_indices[j + 1] = i;
        printf("-------------\n");
        printf("-------------\n");
        printf("-------------\n");
        hashmap_print(support_map);
        printf("-------------\n");
        printf("-------------\n");
        printf("-------------\n");
    }
    printf("Insertion sort ended!!!\n");
}
int pivot(uint8_t **keys, int start, int end) {}

void parallel_quick_sort(uint8_t **keys, int *sorted_indices, int start,
                         int end, cvector_vector_type(int) * stack,
                         int *num_busy_threads, int *num_threads,
                         SupportMap support_map) {
    bool idle = true;
    while (true) {
        if (end - start < INSERTION_SORT_THRESH) {
            insertion_sort(keys, sorted_indices, start, end, support_map);
            start = end;
        }
        while (start >= end) {

#pragma omp critical
            {
                if (!cvector_empty((*stack))) {
                    if (idle)
                        (*num_busy_threads)++;
                    idle = false;
                    start = (*stack)[cvector_size((*stack)) - 1];
                    cvector_pop_back((*stack));
                    end = (*stack)[cvector_size((*stack)) - 1];
                    cvector_pop_back((*stack));
                } else {
                    if (!idle)
                        (*num_busy_threads)--;
                    idle = true;
                }
            }
            if (num_busy_threads == 0) {
                return;
            }
        }
        /* Skipped: choose pivot and do partitioning step */
        int i = pivot(keys, start, end);
#pragma omp critical
        {
            cvector_push_back((*stack), i - 1);
            cvector_push_back((*stack), start);
            // push(pair(q, i - 1));
        } /* iteratively sort elements right of pivot */
        start = i + 1;
        break;
    }
}

/**
 * Source: doi=10.1.1.100.809
 * (https://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.100.809&rep=rep1&type=pdf#page=31)
 */
void sort(uint8_t **keys, int *sorted_indices, SupportMap support_map,
          int start, int end) {
    // Skipped: Program Initialisation */
    cvector_vector_type(int) stack = NULL;
    int num_threads = 1;
    int num_busy_threads = 0;
#pragma omp parallel shared(keys, sorted_indices, stack, num_threads,          \
                            num_busy_threads, support_map)
    {
        int i;
#pragma omp for
        for (i = 0; i < cvector_size(keys); i++) {
            sorted_indices[i] = i;
        }
        if (omp_get_thread_num() == 0) {
            parallel_quick_sort(keys, sorted_indices, start, end, &stack,
                                &num_busy_threads, &num_threads, support_map);
        } else {
            parallel_quick_sort(keys, sorted_indices, start, start, &stack,
                                &num_busy_threads, &num_threads, support_map);
        }
    }
}