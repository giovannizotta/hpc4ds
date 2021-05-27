#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "io.h"
#include "reduce.h"
#include "sort.h"
#include "tree.h"
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
    printf("%d read transactions\n", rank);
    // write_transactions(rank, transactions);
    hashmap_element *items_count = NULL;
    int num_items;
    get_global_map(rank, world_size, &support_map, &items_count, &num_items);
    printf("%d got global map\n", rank);

    hashmap_free(support_map);

    /*--- SORT ITEMS BY SUPPORT ---*/
    size_t length = 1 + (num_items - 1) / world_size;
    int *sorted_indices = (int *)malloc(num_items * sizeof(int));
    int start = length * rank;
    int end = min(length * (rank + 1), num_items) - 1;
    sort(items_count, num_items, sorted_indices, start, end, num_threads);
    printf("%d sorted items\n", rank);
    get_sorted_indices(rank, world_size, sorted_indices, start, end, length,
                       items_count, num_items);
    printf("%d got sorted items\n", rank);

    /*--- PRINT ITEMS SORTED ---*/
    // if (rank == 0) {
    //     for (int i = 0; i < num_items; i++) {
    //         int value = items_count[sorted_indices[i]].value;
    //         uint8_t *key = items_count[sorted_indices[i]].key;
    //         printf("%s: %d\n", key, value);
    //         // items_count[i] = items_count[sorted_indices[i]];
    //     }
    // }

    IndexMap index_map = hashmap_new();
    for (int i = 0; i < num_items; i++) {
        uint8_t *key = items_count[sorted_indices[i]].key;
        int key_length = items_count[sorted_indices[i]].key_length;
        hashmap_put(index_map, key, key_length, sorted_indices[i]);
    }
    printf("%d built index map\n", rank);

    Tree tree = build_tree(rank, world_size, transactions, index_map,
                           items_count, num_items, sorted_indices, num_threads);
    printf("%d built tree\n", rank);
    hashmap_free(index_map);
    free_transactions(&transactions);

    get_global_tree(rank, world_size, &tree, items_count, num_items,
                    sorted_indices);

    /*--- FREE MEMORY ---*/
    if (tree != NULL)
        free_tree(&tree);
    free(sorted_indices);
    if (rank != 0)
        free(items_count);
    else
        cvector_free(items_count);
    MPI_Finalize();

    return 0;
}
