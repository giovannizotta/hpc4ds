#include "sort.h"
#include "types.h"
#include "utils.h"
#include <omp.h>
#include <stdio.h>
#include <string.h>

#define INSERTION_SORT_THRESH 100

/**
 * @brief Sort the indices contained in the array sorted_indices
 * from start to end using insertion sort. The indices are sorted
 * according to the corresponding element in items_count.
 *
 * @param items_count array of key-value pairs
 * @param num_items length of items_count
 * @param sorted_indices array of indices that are going to be sorted
 * @param start start position of the sub-array to sort
 * @param end end position of the sub-array to sort
 */
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

/**
 * @brief Swap elements in position i and j in the array a
 *
 * @param a array where to swap the elements
 * @param i position of the first element to swap
 * @param j position of the second element to swap
 */
void swap(int *a, int i, int j) {
    int tmp = a[i];
    a[i] = a[j];
    a[j] = tmp;
}

/**
 * @brief Perform the pivot operation of the quicksort algorithm on
 * the sub-array sorted_indices from start to end (included).
 *
 * The indices which value in items_count is smaller than the value of
 * the pivot are moved to the left of the pivot,
 * the ones with a value greater or equal are moved to its right.
 *
 * @param items_count array of key-value pairs
 * @param num_items length of items_count
 * @param sorted_indices array of indices that are going to be sorted
 * @param start start position of the sub-array to sort
 * @param end end position of the sub-array to sort
 * @param m position of the pivot
 * @return the new position of the pivot
 */
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

/**
 * @brief Chooses a pivot according to the Median-of-three heuristic.
 *
 * The pivot is chosen as the median of the first, middle and last element of
 * the sub-array sorted_indices from start to end, according to the
 * corresponding value in items_count. These three indices are swapped, so that
 * the median at the end is in position start.
 *
 *
 * @param items_count array of key-value pairs
 * @param num_items length of items_count
 * @param sorted_indices array of indices that are going to be sorted
 * @param start start position of the sub-array to sort
 * @param end end position of the sub-array to sort
 * @return the position of the pivot
 */
int choose_pivot(hashmap_element *items_count, int num_items,
                 int *sorted_indices, int start, int end) {
    int m = (start + end) / 2;
    int v[3];
    v[0] = items_count[sorted_indices[start]].value;
    v[1] = items_count[sorted_indices[m]].value;
    v[2] = items_count[sorted_indices[end]].value;

    if (v[0] > v[2]) {
        swap(v, 0, 2);
        swap(sorted_indices, start, end);
    }
    if (v[1] > v[2]) {
        swap(v, 1, 2);
        swap(sorted_indices, m, end);
    }
    if (v[1] > v[0]) {
        swap(v, 1, 0);
        swap(sorted_indices, m, start);
    }
    return start;
}

/**
 * @brief Subroutine that performs a parallel version of the QuickSort
 * algoritm.
 *
 * @param items_count array of key-value pairs
 * @param num_items length of items_count
 * @param sorted_indices array of indices that are going to be sorted
 * @param start start position of the sub-array to sort
 * @param end end position of the sub-array to sort
 * @param stack stack containing the jobs that need to be done by free threads
 * @param num_busy_threads Number of currently busy threads
 * @param num_threads Number of threads that perform the sorting
 */
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
 * @brief Puts in the array sorted_indices the indices of the elements of
 * the array items_count from position start to end, after they are sorted
 * according to their value field.
 *
 * The array items_count is not modified. The algorithm implement a
 * parallel version of Quicksort, described in the paper
 * Süß M, Leopold C. A user’s experience with parallel sorting and OpenMP.
 * InProceedings of the Sixth European Workshop on OpenMP-EWOMP’04 2004 Oct 18
 * (pp. 23-38).
 *
 * @param items_count array of key-value pairs
 * @param num_items length of items_count
 * @param sorted_indices array of indices that are going to be sorted
 * @param start start position of the sub-array to sort
 * @param end end position of the sub-array to sort
 * @param num_threads Number of threads that perform the sorting
 */
void sort(hashmap_element *items_count, int num_items, int *sorted_indices,
          int start, int end, int num_threads) {

    cvector_vector_type(int) stack = NULL;
    int num_busy_threads = 0;
    int i;

#pragma omp parallel default(none)                                             \
    shared(items_count, num_items, sorted_indices, start, end, stack,          \
           num_threads, num_busy_threads) private(i) num_threads(num_threads)
    {
#pragma omp for
        for (i = start; i <= end; i++) {
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
    cvector_free(stack);
}