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
    read_transactions(&transactions, argv[1], rank, world_size); 

    free_transactions(&transactions);  
    MPI_Finalize();
    return 0;
    
}
