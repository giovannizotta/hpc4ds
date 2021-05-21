#include "reduce.h"
#include <stdio.h>


MPI_Datatype define_datatype_hashmap_element(){
    // Create the datatype
    MPI_Datatype DT_HASHMAP_ELEMENT;
    int lengths[4] = { KEY_STATIC_LENGTH, 1, 1, 1 };

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
 
    MPI_Datatype types[4] = { MPI_UNSIGNED_CHAR, MPI_INT, MPI_INT, MPI_INT };
    MPI_Type_create_struct(4, lengths, displacements, types, &DT_HASHMAP_ELEMENT);
    MPI_Type_commit(&DT_HASHMAP_ELEMENT);
    return DT_HASHMAP_ELEMENT;
}

void merge_map(SupportMap *support_map, hashmap_element *elements, int size){
    int i;

    for (i = 0; i < size; i++){
        hashmap_increment(*support_map, elements[i].key, elements[i].key_length, elements[i].value);
    }

    // fprintf(stderr, "Looking for %s\n", item);
    

}

void recv_map(int rank, int world_size, int source, SupportMap *support_map, 
    MPI_Datatype DT_HASHMAP_ELEMENT){
    
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
    hashmap_element *elements = (hashmap_element *) malloc (size * sizeof(hashmap_element));
    // printf("RECEIVE: Elements allocated\n");

    MPI_Recv(elements, size, DT_HASHMAP_ELEMENT, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    // printf("RECEIVE: get elements\n");

    merge_map(support_map, elements, size);
    // printf("RECEIVE: map merged!\n");
    // hashmap_print(*support_map);
    free(elements);
}


void send_map(int rank, int world_size, int dest, SupportMap *support_map, MPI_Datatype DT_HASHMAP_ELEMENT){
    
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


void get_global_map(int rank, int world_size, SupportMap *support_map, MPI_Datatype DT_HASHMAP_ELEMENT){
    
    int pow;
    bool sent = false;
    for (pow = 2; pow < 2 * world_size && !sent; pow*=2){
        if (rank % pow == 0){
            // receive and merge
            int source = rank + pow/2;
            if (source < world_size){
                printf("%d receiving from %d\n", rank, source);
                recv_map(rank, world_size, source, support_map, DT_HASHMAP_ELEMENT);
                printf("%d succesfully received from %d\n", rank, source);
            }
        } else {
            int dest = rank - pow/2;
            printf("%d sending to %d\n", rank, dest);
            send_map(rank, world_size, dest, support_map, DT_HASHMAP_ELEMENT);
            printf("%d succesfully sent to %d\n", rank, dest);
            sent = true;
        }
    }
    // broadcast
    return;

}