#include <stdio.h>
#include <string.h>
#include <mpi.h>
#include <math.h>
#include <stdlib.h>

#include "io.h"

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
    printf("AAA\n");
    read_transactions(&transactions, argv[1], rank, world_size, &support_map);
    printf("BBB\n");
    // MPI_Datatype *MPI_SupportMap = define_MPI_SupportMap();

    // item_count *s, *tmp = NULL;
    // HASH_ITER(hh, (support_map), s, tmp) {
    //     printf("%s appears %d\n", s->item, s->count);
    // }
    // hashmap_print(support_map);
    printf("%d : %d\n", rank, hashmap_length(support_map));
    printf("CCC\n");

    // hashmap_free(support_map);
    free_transactions(&transactions);  
    MPI_Finalize();

    return 0;
    
}
