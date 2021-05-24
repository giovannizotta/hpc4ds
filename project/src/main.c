#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "io.h"
#include "reduce.h"
#include "sort.h"
#include "utils.h"

int main(int argc, char **argv) {
    int rank, world_size;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    // printf("World size: %d\n", world_size);

    if (argc < 2) {
        if (rank == 0)
            fprintf(stderr, "Usage: %s filename [numthreads]\n", argv[0]);
        MPI_Finalize();
        exit(1);
    }
    int num_threads = 0;
    if (argc > 2)
        num_threads = atoi(argv[2]);

    /*--- READ TRANSACTION AND SUPPORT MAP ---*/
    TransactionsList transactions = NULL;
    SupportMap support_map = hashmap_new();
    read_transactions(&transactions, argv[1], rank, world_size, &support_map);
    // write_transactions(rank, transactions);
    MPI_Datatype DT_HASHMAP_ELEMENT = define_datatype_hashmap_element();
    hashmap_element *items_count = NULL;
    int num_items;
    get_global_map(rank, world_size, &support_map, &items_count, &num_items,
                   DT_HASHMAP_ELEMENT);
    hashmap_free(support_map);

    /*--- SORT ITEMS BY SUPPORT ---*/
    size_t length = 1 + (num_items - 1) / world_size;
    int *sorted_indices = (int *)malloc(num_items * sizeof(int));
    int start = length * rank;
    int end = min(length * (rank + 1), num_items) - 1;
    sort(items_count, num_items, sorted_indices, start, end, num_threads);
    get_sorted_indices(rank, world_size, sorted_indices, start, end, length,
                       items_count, num_items);

    /*--- PRINT ITEMS SORTED ---*/
    if (rank == 0) {
        for (int i = 0; i < num_items; i++) {
            int value = items_count[sorted_indices[i]].value;
            uint8_t *key = items_count[sorted_indices[i]].key;
            printf("%s: %d\n", key, value);
        }
    }

    /*--- FREE MEMORY ---*/
    free(sorted_indices);
    if (rank != 0)
        free(items_count);
    else
        cvector_free(items_count);
    free_transactions(&transactions);
    MPI_Finalize();

    return 0;
}
