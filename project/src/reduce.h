#ifndef REDUCE_H
#define REDUCE_H

#include "mpi.h"
#include "types.h"
void get_global_map(int rank, int world_size, SupportMap *support_map,
                    MPI_Datatype DT_HASHMAP_ELEMENT);
MPI_Datatype define_datatype_hashmap_element();
#endif