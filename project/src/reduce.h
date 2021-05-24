#ifndef REDUCE_H
#define REDUCE_H

#include "mpi.h"
#include "types.h"
void get_global_map(int rank, int world_size, SupportMap *support_map,
                    hashmap_element **items_count, int *num_items,
                    MPI_Datatype DT_HASHMAP_ELEMENT);
void get_sorted_indices(int rank, int world_size, int *sorted_indices,
                        int start, int end, int length,
                        hashmap_element *items_count, int num_items);
MPI_Datatype define_datatype_hashmap_element();
#endif