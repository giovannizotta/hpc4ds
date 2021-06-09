/**
 * @file sort.h
 * @brief Functions that implement parallel QuickSort with OpenMP
 * 
 */
#ifndef SORT_H
#define SORT_H

#include "hashmap/hashmap.h"
#include "types.h"

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
                    int *sorted_indices, int start, int end);
/**
 * @brief Swap elements in position i and j in the array a
 *
 * @param a array where to swap the elements
 * @param i position of the first element to swap
 * @param j position of the second element to swap
 */
void swap(int *a, int i, int j);

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
          int start, int end, int m);

/**
 * @brief Choose a pivot according to the Median-of-three heuristic.
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
                 int *sorted_indices, int start, int end);

/**
 * @brief Parallel version of the QuickSort algoritm
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
                         int *num_busy_threads, int *num_threads);

/**
 * @brief Put in the array sorted_indices the indices of the elements of
 * the array items_count from position start to end, after they are sorted
 * according to their value field.
 *
 * The array items_count is not modified. The algorithm implement a
 * parallel version of QuickSort, described in the paper
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
          int start, int end, int num_threads);
#endif