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

void print_log_header(bool debug) {
    if (debug) {
        printf("rank, time, msg\n");
    }
}

void print_log(bool debug, int rank, double start, double end, char *msg) {
    if (debug) {
        printf("%d, %lf, %s\n", rank, end - start, msg);
    }
}

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

    int num_threads = 1, min_support = 1;
    bool debug = false;
    if (argc > 2)
        num_threads = atoi(argv[2]);
    if (argc > 3)
        min_support = atoi(argv[3]);
    if (argc > 4)
        debug = atoi(argv[4]);

    if (rank == 0)
        print_log_header(debug);

    double start_time, end_time;
    /*--- READ TRANSACTION AND SUPPORT MAP ---*/
    start_time = MPI_Wtime();
    TransactionsList transactions = NULL;
    SupportMap support_map = hashmap_new();
    transactions_read(&transactions, argv[1], rank, world_size, &support_map);
    end_time = MPI_Wtime();
    print_log(debug, rank, start_time, end_time, "read transactions");

    // transactions_write(rank, transactions);
    start_time = MPI_Wtime();
    hashmap_element *items_count = NULL;
    int num_items;
    get_global_map(rank, world_size, &support_map, &items_count, &num_items,
                   min_support);
    hashmap_free(support_map);
    end_time = MPI_Wtime();
    print_log(debug, rank, start_time, end_time, "received global map");

    /*--- SORT ITEMS BY SUPPORT ---*/
    start_time = MPI_Wtime();
    size_t length = 1 + (num_items - 1) / world_size;
    int *sorted_indices = (int *)malloc(num_items * sizeof(int));
    int start = length * rank;
    int end = min(length * (rank + 1), num_items) - 1;
    sort(items_count, num_items, sorted_indices, start, end, num_threads);
    end_time = MPI_Wtime();
    print_log(debug, rank, start_time, end_time, "sorted local items");
    start_time = MPI_Wtime();
    get_sorted_indices(rank, world_size, sorted_indices, start, end, length,
                       items_count, num_items);

    end_time = MPI_Wtime();
    print_log(debug, rank, start_time, end_time,
              "received sorted global items");

    /*--- PRINT ITEMS SORTED ---*/
    // if (rank == 0) {
    //     for (int i = 0; i < num_items; i++) {
    //         int value = items_count[sorted_indices[i]].value;
    //         uint8_t *key = items_count[sorted_indices[i]].key;
    //         printf("%s: %d\n", key, value);
    //         // items_count[i] = items_count[sorted_indices[i]];
    //     }
    // }

    start_time = MPI_Wtime();
    IndexMap index_map = hashmap_new(); // item -> pos in the sorted array
    for (int i = 0; i < num_items; i++) {
        uint8_t *key = items_count[sorted_indices[i]].key;
        int key_length = items_count[sorted_indices[i]].key_length;
        hashmap_put(index_map, key, key_length, sorted_indices[i]);
    }

    // printf("%d built index map\n", rank);

    Tree tree = tree_build_from_transactions(rank, world_size, transactions,
                                             index_map, items_count, num_items,
                                             sorted_indices, num_threads);
    // printf("%d built tree\n", rank);
    hashmap_free(index_map);
    transactions_free(&transactions);
    end_time = MPI_Wtime();
    print_log(debug, rank, start_time, end_time, "built local tree");
    start_time = MPI_Wtime();

    get_global_tree(rank, world_size, &tree);
    end_time = MPI_Wtime();
    print_log(debug, rank, start_time, end_time, "received global tree");

    /*--- FREE MEMORY ---*/
    if (tree != NULL)
        tree_free(&tree);
    free(sorted_indices);
    if (rank != 0)
        free(items_count);
    else
        cvector_free(items_count);
    MPI_Finalize();

    return 0;
}
