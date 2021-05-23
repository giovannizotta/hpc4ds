#include "reduce.h"
#include "utils.h"
#include <stdio.h>

MPI_Datatype define_datatype_hashmap_element() {
    // Create the datatype
    MPI_Datatype DT_HASHMAP_ELEMENT;
    int lengths[4] = {KEY_STATIC_LENGTH, 1, 1, 1};

    MPI_Aint displacements[4];
    hashmap_element dummy_element;
    MPI_Aint base_address;
    MPI_Get_address(&dummy_element, &base_address);
    MPI_Get_address(&dummy_element.key[0], &displacements[0]);
    MPI_Get_address(&dummy_element.key_length, &displacements[1]);
    MPI_Get_address(&dummy_element.in_use, &displacements[2]);
    MPI_Get_address(&dummy_element.value, &displacements[3]);
    displacements[0] = MPI_Aint_diff(displacements[0], base_address);
    displacements[1] = MPI_Aint_diff(displacements[1], base_address);
    displacements[2] = MPI_Aint_diff(displacements[2], base_address);
    displacements[3] = MPI_Aint_diff(displacements[3], base_address);

    MPI_Datatype types[4] = {MPI_UNSIGNED_CHAR, MPI_INT, MPI_INT, MPI_INT};
    MPI_Type_create_struct(4, lengths, displacements, types,
                           &DT_HASHMAP_ELEMENT);
    MPI_Type_commit(&DT_HASHMAP_ELEMENT);
    return DT_HASHMAP_ELEMENT;
}

void merge_map(SupportMap *support_map, hashmap_element *elements, int size) {
    int i;

    for (i = 0; i < size; i++) {
        hashmap_increment(*support_map, elements[i].key, elements[i].key_length,
                          elements[i].value);
    }

    // fprintf(stderr, "Looking for %s\n", item);
}

void recv_map(int rank, int world_size, int source, SupportMap *support_map,
              MPI_Datatype DT_HASHMAP_ELEMENT) {

    int size;
    MPI_Status status;

    // printf("RECEIVE: receiving\n");
    MPI_Recv(&size, 1, MPI_INT, source, 0, MPI_COMM_WORLD, &status);
    // printf("######### %d %d %d %d %d\n", status._ucount,
    //   status._cancelled,
    //   status.MPI_SOURCE,
    //   status.MPI_TAG,
    //   status.MPI_ERROR);
    // printf("RECEIVE: size: %d\n", size);
    hashmap_element *elements =
        (hashmap_element *)malloc(size * sizeof(hashmap_element));
    // printf("RECEIVE: Elements allocated\n");

    MPI_Recv(elements, size, DT_HASHMAP_ELEMENT, source, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
    // printf("RECEIVE: get elements\n");

    merge_map(support_map, elements, size);
    // printf("RECEIVE: map merged!\n");
    // hashmap_print(*support_map);
    free(elements);
}

void send_map(int rank, int world_size, int dest, SupportMap *support_map,
              MPI_Datatype DT_HASHMAP_ELEMENT) {

    int size = hashmap_length(*support_map);
    // printf("Sending %d elements\n", size);
    cvector_vector_type(hashmap_element) elements = NULL;

    hashmap_get_elements(*support_map, &elements);
    // printf("%lu elements allocated\n", cvector_size(elements));
    // send size
    MPI_Send(&size, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
    // printf("Size sent\n");
    // send buffer
    MPI_Send(elements, size, DT_HASHMAP_ELEMENT, dest, 0, MPI_COMM_WORLD);
    // printf("Elements sent\n");

    cvector_free(elements);
    // printf("Free elements\n");
}

void broadcast_result(int rank, int world_size, SupportMap *support_map,
                      MPI_Datatype DT_HASHMAP_ELEMENT) {
    if (rank == 0) {
        int size = hashmap_length(*support_map);
        cvector_vector_type(hashmap_element) elements = NULL;
        hashmap_get_elements(*support_map, &elements);
        MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(elements, size, DT_HASHMAP_ELEMENT, 0, MPI_COMM_WORLD);
        cvector_free(elements);
    } else {
        hashmap_free(*support_map);
        *support_map = hashmap_new();
        int size;
        MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);
        hashmap_element *elements =
            (hashmap_element *)malloc(size * sizeof(hashmap_element));
        MPI_Bcast(elements, size, DT_HASHMAP_ELEMENT, 0, MPI_COMM_WORLD);
        merge_map(support_map, elements, size);
        free(elements);
    }
}

void get_global_map(int rank, int world_size, SupportMap *support_map,
                    MPI_Datatype DT_HASHMAP_ELEMENT) {
    int pow;
    bool sent = false;
    for (pow = 2; pow < 2 * world_size && !sent; pow *= 2) {
        if (rank % pow == 0) {
            // receive and merge
            int source = rank + pow / 2;
            if (source < world_size) {
                printf("%d receiving from %d\n", rank, source);
                recv_map(rank, world_size, source, support_map,
                         DT_HASHMAP_ELEMENT);
                printf("%d succesfully received from %d\n", rank, source);
            }
        } else {
            int dest = rank - pow / 2;
            printf("%d sending to %d\n", rank, dest);
            send_map(rank, world_size, dest, support_map, DT_HASHMAP_ELEMENT);
            printf("%d succesfully sent to %d\n", rank, dest);
            sent = true;
        }
    }

    /** REINITIALIZE MAP TO HAVE ELEMENTS IN THE SAME ORDER AS OTHER PROCESSES
     * **/

    broadcast_result(rank, world_size, support_map, DT_HASHMAP_ELEMENT);

    if (rank == 0) {
        SupportMap tmp_map = hashmap_new();

        int size = hashmap_length(*support_map);
        cvector_vector_type(hashmap_element) elements = NULL;
        hashmap_get_elements(*support_map, &elements);
        merge_map(&tmp_map, elements, size);
        SupportMap tmp_ptr = *support_map;
        *support_map = tmp_map;
        hashmap_free(tmp_ptr);
        cvector_free(elements);
    }
    return;
}

void merge_indices(int rank, int *sorted_indices, int start1, int end1,
                   int start2, int end2, SupportMap *support_map,
                   uint8_t **keys) {

    int tot_size = end1 - start1 + 1 + end2 - start2 + 1;
    assert(start1 + tot_size <= hashmap_length(*support_map));
    assert(end1 <= hashmap_length(*support_map));
    if (end2 > hashmap_length(*support_map)) {
        printf("%d end2: %d, hashmaplenght: %d\n", rank, end2,
               hashmap_length(*support_map));
    }
    assert(end2 <= hashmap_length(*support_map));
    // printf("MERGING %d-%d and %d-%d (totsize:%d)\n", start1, end1, start2,
    // end2,
    //        tot_size);
    if (rank == 0)
        printf("Allocating %d buffer\n", tot_size);
    int *tmp = (int *)malloc(tot_size * sizeof(int));
    if (tmp == NULL)
        printf("ERROR \n");
    assert(tmp != NULL);

    if (rank == 0)
        printf("Allocated %d buffer\n", tot_size);

    int i = 0, j, i1 = start1, i2 = start2;

    // printf("%d-%d A.\n", start1, end2);
    while (i1 <= end1 && i2 <= end2) {
        int v1, v2, l1, l2;
        // printf("%d-%d i: %d, i1: %d, i2: %d.\n", start1, end2, i, i1, i2);
        uint8_t *k1 = keys[sorted_indices[i1]], *k2 = keys[sorted_indices[i2]];
        l1 = ulength(k1);
        l2 = ulength(k2);
        // printf("%d-%d i: %d, i1: %d, i2: %d. %s (%d) %s (%d) OK\n", start1,
        //        end2, i, i1, i2, k1, l1, k2, l2);

        assert(hashmap_get((*support_map), k1, l1, &v1) == MAP_OK);
        // printf("%d-%d i: %d, i1: %d, i2: %d. K1\n", start1, end2, i, i1, i2);
        assert(hashmap_get((*support_map), k2, l2, &v2) == MAP_OK);
        // printf("%d-%d i: %d, i1: %d, i2: %d. K2\n", start1, end2, i, i1, i2);
        assert(i < tot_size);
        if (v1 < v2) {
            tmp[i] = sorted_indices[i1];
            i1++;
        } else {
            tmp[i] = sorted_indices[i2];
            i2++;
        }
        i++;
    }
    // printf("%d-%d B.\n", start1, end2);
    while (i1 <= end1) {
        tmp[i] = sorted_indices[i1];
        i1++;
        i++;
    }
    // printf("%d-%d C.\n", start1, end2);
    while (i2 <= end2) {
        tmp[i] = sorted_indices[i2];
        i2++;
        i++;
    }
    assert(i == tot_size);
    assert(start1 + i <= hashmap_length(*support_map));
    // printf("%d-%d D.\n", start1, end2);
    // copy into original array
    for (j = 0; j < tot_size; j++) {
        sorted_indices[start1 + j] = tmp[j];
    }
    // printf("%d-%d E.\n", start1, end2);
    free(tmp);
    // printf("!!! MERGED %d-%d and %d-%d (totsize:%d)\n", start1, end1, start2,
    //        end2, tot_size);
}

void recv_indices(int rank, int world_size, int source, int *sorted_indices,
                  int start, int *end, int length, int size,
                  SupportMap *support_map, uint8_t **keys) {

    MPI_Status status;

    int position = source * length;
    // printf("RECEIVE: receiving\n");
    if (rank == 0)
        printf("%d Receiving %d buffer\n", rank, length);
    MPI_Recv(sorted_indices + position, size, MPI_INT, source, 0,
             MPI_COMM_WORLD, &status);
    MPI_Get_count(&status, MPI_INT, &size);
    if (rank == 0)
        printf("%d Received %d buffer\n", rank, size);
    merge_indices(rank, sorted_indices, start, *end, position,
                  position + size - 1, support_map, keys);
    if (rank == 0)
        printf("%d Merged %d buffer\n", rank, size);
    *end = position + size - 1;
}

void send_indices(int rank, int world_size, int dest, int *sorted_indices,
                  int start, int end, int length) {
    int size = end - start + 1;
    // printf("Sending %d elements\n", size);

    // printf("%lu elements allocated\n", cvector_size(elements));
    // send size
    // printf("PRIMA (%d)\n", size);
    assert(size >= 0);
    MPI_Send(sorted_indices + start, size, MPI_INT, dest, 0, MPI_COMM_WORLD);
    // printf("DOPO\n");
    // printf("Size sent\n");
    // send buffer
    // MPI_Send(elements, size, DT_HASHMAP_ELEMENT, dest, 0, MPI_COMM_WORLD);
    // // printf("Elements sent\n");

    // cvector_free(elements);
    // printf("Free elements\n");
}

void get_sorted_indices(int rank, int world_size, int *sorted_indices,
                        int start, int end, int length, SupportMap *support_map,
                        uint8_t **keys) {
    // printf("%d get sorted indices[%d, %d]\n", rank, start, end);
    int pow;
    bool sent = false;
    for (pow = 2; pow < 2 * world_size && !sent; pow *= 2) {
        if (rank % pow == 0) {
            // receive and merge
            int source = rank + pow / 2;
            if (source < world_size) {
                printf("%d receiving [%d-%d] from %d\n", rank, source * length,
                       source * length + length * (pow / 2) - 1, source);
                recv_indices(rank, world_size, source, sorted_indices, start,
                             &end, length, length * (pow / 2), support_map,
                             keys);
                printf("%d succesfully received from %d\n", rank, source);
            }
        } else {
            int dest = rank - pow / 2;
            printf("%d sending [%d-%d] to %d\n", rank, start, end, dest);
            assert(end - start + 1 <= hashmap_length(*support_map));
            send_indices(rank, world_size, dest, sorted_indices, start, end,
                         length);
            printf("%d succesfully sent to %d\n", rank, dest);
            sent = true;
        }
    }
    // broadcast_result(rank, world_size, support_map, DT_HASHMAP_ELEMENT);
    return;
}