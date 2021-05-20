#include "reduce.h"


MPI_Datatype *define_datatype_hashmap_element(){
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
    MPI_Get_address(&dummy_element.value[0], &displacements[3]);
    displacements[0] = MPI_Aint_diff(displacements[0], base_address);
    displacements[1] = MPI_Aint_diff(displacements[1], base_address);
    displacements[2] = MPI_Aint_diff(displacements[2], base_address);
    displacements[3] = MPI_Aint_diff(displacements[3], base_address);
 
    MPI_Datatype types[4] = { MPI_UNSIGNED_CHAR, MPI_INT, MPI_INT, MPI_INT };
    MPI_Type_create_struct(4, lengths, displacements, types, &DT_HASHMAP_ELEMENT);
    MPI_Type_commit(&DT_HASHMAP_ELEMENT);
    return &DT_HASHMAP_ELEMENT;
}

void merge_map(SupportMap *support_map, hashmap_element *elements, int size){
    int i;
    void *value;

    for (i = 0; i < size; i++){
        
        if (hashmap_get(*support_map, elements[i].key, elements[i].key_length, &value) == MAP_MISSING){
            // fprintf(stderr, "New, putting %s\n", item);
            // value = (void *) malloc(sizeof(int));
            // *((int *) value) = 1;
            hashmap_put(*support_map, elements[i].key, elements[i].key_length, elements[i].value);
            // fprintf(stderr, "Done %s\n", item);
        } else {
            // fprintf(stderr, "Already in %s : %d\n", item, * ((int *) value));
            
            /**
             * TODO: change this to hashmap put (value must be integer)
             */
            (*((int *) value)) += *((int *)elements[i].value);
            // fprintf(stderr, "Incremented\n");
        }
    }

    // fprintf(stderr, "Looking for %s\n", item);
    

}

void recv_map(int rank, int world_size, int source, SupportMap *support_map, 
    MPI_Datatype *DT_HASHMAP_ELEMENT){
    
    int size;

    MPI_Recv(&size, 1, MPI_INT, source, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    hashmap_element *elements = (hashmap_element *) malloc (size * sizeof(hashmap_element));

    MPI_Recv(elements, size, *DT_HASHMAP_ELEMENT, source, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    merge_map(support_map, elements, size);
    free(elements);
}


void send_map(int rank, int world_size, int dest, SupportMap *support_map, MPI_Datatype *DT_HASHMAP_ELEMENT){
    
    int size = hashmap_length(*support_map);
    cvector_vector_type(hashmap_element) elements;

    hashmap_get_elements(*support_map, &elements);
    // send size
    MPI_Send(&size, 1, MPI_INT, dest, MPI_ANY_TAG, MPI_COMM_WORLD);
    // send buffer
    MPI_Send(elements, size, *DT_HASHMAP_ELEMENT, dest, MPI_ANY_TAG, MPI_COMM_WORLD);

    cvector_free(elements);
}


void get_global_map(int rank, int world_size, SupportMap *support_map, MPI_Datatype *DT_HASHMAP_ELEMENT){
    
    int pow;
    bool sent = false;
    for (pow = 2; pow < 2 * world_size && !sent; pow*=2){
        if (rank % pow == 0){
            // receive and merge
            int source = rank + pow/2;
            recv_map(rank, world_size, source, support_map, DT_HASHMAP_ELEMENT);
        } else {
            int dest = rank - pow/2;
            send_map(rank, world_size, dest, support_map, DT_HASHMAP_ELEMENT);
            sent = true;
        }
    }
    // broadcast

}