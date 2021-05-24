#include "sort.h"
#include "types.h"
#include "utils.h"
#include <omp.h>
#include <stdio.h>
#include <string.h>

#define INSERTION_SORT_THRESH 100

void insertion_sort(hashmap_element *items_count, int num_items,
                    int *sorted_indices, int start, int end) {
    int i, j, i_val, j_val;
    for (i = start + 1; i <= end; i++) {
        bool stop = false;
        i_val = items_count[sorted_indices[i]].value;
        int tmp = sorted_indices[i];
        for (j = i; j > 0 && !stop; j--) {
            j_val = items_count[sorted_indices[j - 1]].value;
            if (j_val > i_val) {
                sorted_indices[j] = sorted_indices[j - 1];
            } else {
                stop = true;
            }
        }
        if (stop)
            j++;
        sorted_indices[j] = tmp;
    }
}

void swap(int *a, int i, int j) {
    int tmp = a[i];
    a[i] = a[j];
    a[j] = tmp;
}

int pivot(hashmap_element *items_count, int num_items, int *sorted_indices,
          int start, int end, int m) {
    swap(sorted_indices, start, m);
    int i, j = start, i_value, pivot = sorted_indices[start], pivot_value;
    // i_key_length = ulength(keys[pivot]);
    pivot_value = items_count[pivot].value;
    for (i = j + 1; i <= end; i++) {
        i_value = items_count[sorted_indices[i]].value;
        if (i_value < pivot_value) {
            j++;
            swap(sorted_indices, i, j);
        }
    }
    sorted_indices[start] = sorted_indices[j];
    sorted_indices[j] = pivot;
    return j;
}

int choose_pivot(hashmap_element *items_count, int num_items, int *ind,
                 int start, int end) {
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
    // int l[3];
    int v[3];
    // l[0] = items_count[ind[start]].key_length;
    // l[1] = items_count[ind[m]].key_length;
    // l[2] = items_count[ind[end]].key_length;

    v[0] = items_count[ind[start]].value;
    v[1] = items_count[ind[m]].value;
    v[2] = items_count[ind[end]].value;

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

void parallel_quick_sort(hashmap_element *items_count, int num_items,
                         int *sorted_indices, int start, int end,
                         cvector_vector_type(int) * stack,
                         int *num_busy_threads, int *num_threads) {
    bool idle = true;
    while (true) {
        if (end - start < INSERTION_SORT_THRESH) {
            insertion_sort(items_count, num_items, sorted_indices, start, end);
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
        int m =
            choose_pivot(items_count, num_items, sorted_indices, start, end);
        int i = pivot(items_count, num_items, sorted_indices, start, end, m);
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
void sort(hashmap_element *items_count, int num_items, int *sorted_indices,
          int start, int end, int num_threads) {

    cvector_vector_type(int) stack = NULL;
    int num_busy_threads = 0;
#pragma omp parallel shared(items_count, num_items, sorted_indices, stack,     \
                            num_threads, num_busy_threads)
    {
        int i;
#pragma omp for
        for (i = 0; i < num_items; i++) {
            sorted_indices[i] = i;
        }
        if (omp_get_thread_num() == 0) {
            parallel_quick_sort(items_count, num_items, sorted_indices, start,
                                end, &stack, &num_busy_threads, &num_threads);
        } else {
            parallel_quick_sort(items_count, num_items, sorted_indices, start,
                                start, &stack, &num_busy_threads, &num_threads);
        }
    }
}