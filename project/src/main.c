#include <stdio.h>
#include <string.h>
#include <mpi.h>
#include <math.h>
#include <stdlib.h>

#include "io.h"
#include "reduce.h"

int main(int argc, char **argv){
    int rank, world_size;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); 
    // printf("World size: %d\n", world_size);

    if (argc < 2) {
        if (rank == 0) 
            fprintf(stderr, "Usage: %s infilename\n", argv[0]);
        MPI_Finalize();
        exit(1);
    }

    TransactionsList transactions = NULL;
    SupportMap support_map = hashmap_new();
    read_transactions(&transactions, argv[1], rank, world_size, &support_map);

    MPI_Datatype *DT_HASHMAP_ELEMENT = define_datatype_hashmap_element();
    get_global_map(rank, world_size, &support_map, DT_HASHMAP_ELEMENT);
    // MPI_Datatype *MPI_SupportMap = define_MPI_SupportMap();

    // item_count *s, *tmp = NULL;
    // HASH_ITER(hh, (support_map), s, tmp) {
    //     printf("%s appears %d\n", s->item, s->count);
    // }
    // hashmap_print(support_map);
    // printf("%d : %d\n", rank, hashmap_length(support_map));
    // hashmap_print(support_map);
    hashmap_free(support_map);
    free_transactions(&transactions);  
    MPI_Finalize();

    return 0;
    
}
