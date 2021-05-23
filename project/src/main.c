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
            fprintf(stderr, "Usage: %s infilename\n", argv[0]);
        MPI_Finalize();
        exit(1);
    }
    printf("%d A\n", rank);

    TransactionsList transactions = NULL;
    SupportMap support_map = hashmap_new();
    read_transactions(&transactions, argv[1], rank, world_size, &support_map);
    printf("%d B\n", rank);
    // write_transactions(rank, transactions);
    MPI_Datatype DT_HASHMAP_ELEMENT = define_datatype_hashmap_element();
    get_global_map(rank, world_size, &support_map, DT_HASHMAP_ELEMENT);
    printf("%d C\n", rank);
    // if (rank == 0)
    //     hashmap_print(support_map);
    cvector_vector_type(uint8_t *) keys = NULL;
    hashmap_get_keys(support_map, &keys);
    printf("%d D\n", rank);

    size_t length = 1 + (cvector_size(keys) - 1) / world_size;

    printf("%d E\n", rank);
    int *sorted_indices = (int *)malloc(cvector_size(keys) * sizeof(int));

    int start = length * rank,
        end = min(length * (rank + 1), cvector_size(keys)) - 1;
    // printf("### %d length: %d, nkeys: %d, start: %d, end: %d\n", rank,
    // length,
    //        cvector_size(keys), start, end);
    printf("%d F\n", rank);

    sort(keys, sorted_indices, support_map, start, end);

    // write_keys(rank, length * rank, length * (rank + 1) - 1, keys);

    // item_count *s, *tmp = NULL;
    // HASH_ITER(hh, (support_map), s, tmp) {
    //     printf("%s appears %d\n", s->item, s->count);
    // }
    // printf("%d : %d\n", rank, hashmap_length(support_map));
    // hashmap_print(support_map);
    printf("%d G\n", rank);
    get_sorted_indices(rank, world_size, sorted_indices, start, end, length,
                       &support_map, keys);
    printf("%d H\n", rank);

    if (rank == 0) {
        int value;
        for (int i = 0; i < cvector_size(keys); i++) {
            hashmap_get(support_map, keys[sorted_indices[i]],
                        ulength(keys[sorted_indices[i]]), &value);
            printf("%s: %d\n", keys[sorted_indices[i]], value);
        }
    }
    printf("%d I\n", rank);
    // [3,2,0,1,    4,5,6,7]
    hashmap_free(support_map);
    printf("%d J\n", rank);
    free(sorted_indices);
    printf("%d K\n", rank);
    cvector_free(keys);
    printf("%d L\n", rank);
    free_transactions(&transactions);
    printf("%d M\n", rank);
    MPI_Finalize();

    return 0;
}
