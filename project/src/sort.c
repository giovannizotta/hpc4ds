#include "sort.h"
#include "types.h"
#include "utils.h"
#include <omp.h>
#include <stdio.h>
#include <string.h>

#define INSERTION_SORT_THRESH 100

void insertion_sort(uint8_t **keys, int *sorted_indices, int start, int end,
                    SupportMap support_map) {
    int i, j, i_key_length, j_key_length, i_val, j_val;
    for (i = start + 1; i <= end; i++) {
        i_key_length = ulength(keys[sorted_indices[i]]);
        bool stop = false;
        assert(hashmap_get(support_map, keys[sorted_indices[i]], i_key_length,
                           &i_val) == MAP_OK);
        int tmp = sorted_indices[i];
        for (j = i; j > 0 && !stop; j--) {
            j_key_length = ulength(keys[sorted_indices[j - 1]]);
            assert(hashmap_get(support_map, keys[sorted_indices[j - 1]],
                               j_key_length, &j_val) == MAP_OK);
            if (j_val > i_val) {
                sorted_indices[j] = sorted_indices[j - 1];
            } else {
                stop = true;
            }
        }
        if (stop)
            j++;
        sorted_indices[j] = tmp;
        // int k;
        // for (k = start; k <= end; k++) {
        //     printf("%d(%s) ", sorted_indices[k], keys[sorted_indices[k]]);
        // }
        // printf("\n");
    }
}

void swap(int *a, int i, int j) {
    int tmp = a[i];
    a[i] = a[j];
    a[j] = tmp;
}

int pivot(uint8_t **keys, int *sorted_indices, int start, int end, int m,
          SupportMap support_map) {
    swap(sorted_indices, start, m);
    int i, j = start, i_value, i_key_length, pivot = sorted_indices[start],
           pivot_value;
    i_key_length = ulength(keys[pivot]);
    assert(hashmap_get(support_map, keys[pivot], i_key_length, &pivot_value) !=
           MAP_MISSING);
    for (i = j + 1; i <= end; i++) {
        i_key_length = ulength(keys[sorted_indices[i]]);
        assert(hashmap_get(support_map, keys[sorted_indices[i]], i_key_length,
                           &i_value) == MAP_OK);
        if (i_value < pivot_value) {
            j++;
            swap(sorted_indices, i, j);
        }
    }
    sorted_indices[start] = sorted_indices[j];
    sorted_indices[j] = pivot;
    return j;
}

int choose_pivot(uint8_t **keys, int *ind, int start, int end,
                 SupportMap support_map) {
    // int m = b(lo + hi )/2c
    // if A[lo] > A[hi ] then
    // swap (A, lo, hi )
    // % Sposta il massimo in ultima posizione
    // if A[m] > A[hi ] then
    // swap (A, m, hi ) % Sposta il massimo in ultima posizione
    // if A[m] > A[lo] then
    // swap (A, m, lo) % Sposta il mediano in prima posizione
    // Item pivot = A[lo]
    // [...]
    int m = (start + end) / 2;
    int l[3];
    int v[3];
    l[0] = ulength(keys[ind[start]]);
    l[1] = ulength(keys[ind[m]]);
    l[2] = ulength(keys[ind[end]]);

    assert(hashmap_get(support_map, keys[ind[start]], l[0], &v[0]) !=
           MAP_MISSING);

    assert(hashmap_get(support_map, keys[ind[m]], l[1], &v[1]) == MAP_OK);
    assert(hashmap_get(support_map, keys[ind[end]], l[2], &v[2]) !=
           MAP_MISSING);

    if (v[0] > v[2]) {
        swap(v, 0, 2);
        swap(ind, start, end);
    }
    if (v[1] > v[2]) {
        swap(v, 1, 2);
        swap(ind, m, end);
    }
    if (v[1] > v[0]) {
        swap(v, 1, 0);
        swap(ind, m, start);
    }
    return start;
}

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
            if (*num_busy_threads == 0) {
                return;
            }
        }
        int m = choose_pivot(keys, sorted_indices, start, end, support_map);
        int i = pivot(keys, sorted_indices, start, end, m, support_map);
#pragma omp critical
        {
            cvector_push_back((*stack), i - 1);
            cvector_push_back((*stack), start);
            // push(pair(q, i - 1));
        } /* iteratively sort elements right of pivot */
        start = i + 1;
    }
}

/**
 * Source: doi=10.1.1.100.809
 * (https://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.100.809&rep=rep1&type=pdf#page=31)
 */
void sort(uint8_t **keys, int *sorted_indices, SupportMap support_map,
          int start, int end, int num_threads) {

    cvector_vector_type(int) stack = NULL;
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