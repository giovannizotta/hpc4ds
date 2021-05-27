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
    MPI_Datatype types[4] = {MPI_UNSIGNED_CHAR, MPI_INT, MPI_C_BOOL, MPI_INT};
    MPI_Type_create_struct(4, lengths, displacements, types,
                           &DT_HASHMAP_ELEMENT);
    MPI_Type_commit(&DT_HASHMAP_ELEMENT);
    return DT_HASHMAP_ELEMENT;
}

MPI_Datatype define_datatype_tree_node() {
    // Create the datatype
    MPI_Datatype DT_TREE_NODE;
    int lengths[3] = {1, 1, 1};

    MPI_Aint displacements[3];
    TreeNodeToSend dummy_node;
    MPI_Aint base_address;
    MPI_Get_address(&dummy_node, &base_address);
    MPI_Get_address(&dummy_node.key, &displacements[0]);
    MPI_Get_address(&dummy_node.value, &displacements[1]);
    MPI_Get_address(&dummy_node.parent, &displacements[2]);
    displacements[0] = MPI_Aint_diff(displacements[0], base_address);
    displacements[1] = MPI_Aint_diff(displacements[1], base_address);
    displacements[2] = MPI_Aint_diff(displacements[2], base_address);
    MPI_Datatype types[3] = {MPI_INT, MPI_INT, MPI_INT};
    MPI_Type_create_struct(3, lengths, displacements, types, &DT_TREE_NODE);
    MPI_Type_commit(&DT_TREE_NODE);
    return DT_TREE_NODE;
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

void broadcast_map(int rank, int world_size, SupportMap *support_map,
                   hashmap_element **items_count, int *num_items,
                   MPI_Datatype DT_HASHMAP_ELEMENT) {
    if (rank == 0) {
        int size = hashmap_length(*support_map);
        cvector_vector_type(hashmap_element) elements = NULL;
        hashmap_get_elements(*support_map, &elements);
        MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(elements, size, DT_HASHMAP_ELEMENT, 0, MPI_COMM_WORLD);
        *items_count = elements;
        *num_items = size;
    } else {
        // hashmap_free(*support_map);
        // *support_map = hashmap_new();
        int size;
        MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);
        hashmap_element *elements =
            (hashmap_element *)malloc(size * sizeof(hashmap_element));
        MPI_Bcast(elements, size, DT_HASHMAP_ELEMENT, 0, MPI_COMM_WORLD);
        // merge_map(support_map, elements, size);
        // free(elements);
        *items_count = elements;
        *num_items = size;
    }
}

void get_global_map(int rank, int world_size, SupportMap *support_map,
                    hashmap_element **items_count, int *num_items) {

    MPI_Datatype DT_HASHMAP_ELEMENT = define_datatype_hashmap_element();
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

    broadcast_map(rank, world_size, support_map, items_count, num_items,
                  DT_HASHMAP_ELEMENT);

    return;
}

void merge_indices(int rank, int *sorted_indices, int start1, int end1,
                   int start2, int end2, hashmap_element *items_count,
                   int num_items) {

    int tot_size = end1 - start1 + 1 + end2 - start2 + 1;
    assert(start1 + tot_size <= num_items);
    assert(end1 <= num_items);
    if (end2 > num_items) {
        printf("%d end2: %d, hashmaplenght: %d\n", rank, end2, num_items);
    }
    assert(end2 <= num_items);
    // printf("MERGING %d-%d and %d-%d (totsize:%d)\n", start1, end1, start2,
    // end2,
    //        tot_size);
    int *tmp = (int *)malloc(tot_size * sizeof(int));
    assert(tmp != NULL);

    int i = 0, j, i1 = start1, i2 = start2;

    // printf("%d-%d A.\n", start1, end2);
    while (i1 <= end1 && i2 <= end2) {
        int v1 = items_count[sorted_indices[i1]].value;
        int v2 = items_count[sorted_indices[i2]].value;

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
    assert(start1 + i <= num_items);
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
                  hashmap_element *items_count, int num_items) {

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
                  position + size - 1, items_count, num_items);
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

void broadcast_indices(int rank, int world_size, int *sorted_indices,
                       int num_items) {
    MPI_Bcast(sorted_indices, num_items, MPI_INT, 0, MPI_COMM_WORLD);
}

void get_sorted_indices(int rank, int world_size, int *sorted_indices,
                        int start, int end, int length,
                        hashmap_element *items_count, int num_items) {
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
                             &end, length, length * (pow / 2), items_count,
                             num_items);
                printf("%d succesfully received from %d\n", rank, source);
            }
        } else {
            int dest = rank - pow / 2;
            printf("%d sending [%d-%d] to %d\n", rank, start, end, dest);
            assert(end - start + 1 <= num_items);
            send_indices(rank, world_size, dest, sorted_indices, start, end,
                         length);
            printf("%d succesfully sent to %d\n", rank, dest);
            sent = true;
        }
    }
    broadcast_indices(rank, world_size, sorted_indices, num_items);
    return;
}

void parse_tree(TreeNodeToSend *nodes, int num_nodes, Tree *dest) {
    *dest = init_tree();
    for (int i = 1; i < num_nodes; i++) {
        TreeNode *node =
            init_tree_node(nodes[i].key, nodes[i].value, nodes[i].parent);
        add_tree_node(dest, node);
    }
}

/**
 * Sends and frees the tree
 */
void send_tree(int rank, int world_size, int dest, Tree *tree,
               MPI_Datatype DT_TREE_NODE) {
    int size = cvector_size((*tree));
    // printf("Sending %d elements\n", size);
    cvector_vector_type(TreeNodeToSend) nodes = NULL;

    tree_get_nodes(*tree, &nodes);
    free_tree(tree);

    // printf("Size sent\n");
    // send buffer
    MPI_Send(&size, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
    // printf("Size sent\n");
    // send buffer
    MPI_Send(nodes, size, DT_TREE_NODE, dest, 0, MPI_COMM_WORLD);
    // printf("Elements sent\n");

    cvector_free(nodes);
    // printf("Free elements\n");
}

void recv_tree(int rank, int world_size, int source, Tree *tree,
               MPI_Datatype DT_TREE_NODE) {

    int size;
    MPI_Status status;
    // printf("RECEIVE: receiving\n");
    MPI_Recv(&size, 1, MPI_INT, source, 0, MPI_COMM_WORLD, &status);

    TreeNodeToSend *nodes =
        (TreeNodeToSend *)malloc(size * sizeof(TreeNodeToSend));
    // printf("RECEIVE: nodes allocated\n");

    MPI_Recv(nodes, size, DT_TREE_NODE, source, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
    // printf("RECEIVE: get nodes\n");
    Tree received_tree;
    parse_tree(nodes, size, &received_tree);

    merge_trees(tree, received_tree);
    // printf("RECEIVE: map merged!\n");
    // hashmap_print(*support_map);
    free(nodes);
}

void broadcast_tree(int rank, int world_size, Tree *tree,
                    MPI_Datatype DT_TREE_NODE) {
    if (rank == 0) {
        int size = cvector_size((*tree));
        cvector_vector_type(TreeNodeToSend) nodes = NULL;
        tree_get_nodes(*tree, &nodes);
        // send buffer
        MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(nodes, size, DT_TREE_NODE, 0, MPI_COMM_WORLD);

        cvector_free(nodes);
    } else {
        // hashmap_free(*support_map);
        // *support_map = hashmap_new();
        int size;
        MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);
        TreeNodeToSend *nodes =
            (TreeNodeToSend *)malloc(size * sizeof(TreeNodeToSend));
        // printf("RECEIVE: nodes allocated\n");

        MPI_Bcast(nodes, size, DT_TREE_NODE, 0, MPI_COMM_WORLD);
        // printf("RECEIVE: get nodes\n");
        Tree received_tree;
        parse_tree(nodes, size, &received_tree);
        *tree = received_tree;
        printf("RECEIVE: TREE RECEIVED FROM BROADCAST (size: %lu)!!!\n",
               cvector_size((*tree)));
        free(nodes);
    }
}

void get_global_tree(int rank, int world_size, Tree *tree,
                     hashmap_element *items_count, int num_items,
                     int *sorted_indices) {
    MPI_Datatype DT_TREE_NODE = define_datatype_tree_node();
    int pow;
    bool sent = false;
    for (pow = 2; pow < 2 * world_size && !sent; pow *= 2) {
        if (rank % pow == 0) {
            // receive and merge
            int source = rank + pow / 2;
            if (source < world_size) {
                printf("%d receiving from %d\n", rank, source);
                recv_tree(rank, world_size, source, tree, DT_TREE_NODE);
                printf("%d succesfully received from %d\n", rank, source);
            }
        } else {
            int dest = rank - pow / 2;
            printf("%d sending to %d\n", rank, dest);
            send_tree(rank, world_size, dest, tree, DT_TREE_NODE);
            printf("%d succesfully sent to %d\n", rank, dest);
            sent = true;
        }
    }

    /** REINITIALIZE MAP TO HAVE ELEMENTS IN THE SAME ORDER AS OTHER PROCESSES
     * **/

    broadcast_tree(rank, world_size, tree, DT_TREE_NODE);

    return;
}