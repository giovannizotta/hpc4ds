#include "reduce.h"
#include "utils.h"
#include <stdio.h>

/**
 * @brief Define a datatype for an hashmap element in order to be able to send it with MPI
 * 
 * @return MPI_Datatype describing an hashmap element
 */
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

/**
 * @brief Define an MPI Datatype for a TreeNode in order to be able to send it with MPI
 * 
 * @return MPI_Datatype describing a TreeNode
 */
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

/**
 * @brief Merge the current map with an array of elements
 * 
 * @param support_map A pointer to the map which will be populated with the merged items
 * @param elements The array of hashmap elements to merge
 * @param size The size of the elements array
 */
void merge_map(SupportMap *support_map, hashmap_element *elements, int size) {
    int i;

    for (i = 0; i < size; i++) {
        hashmap_increment(*support_map, elements[i].key, elements[i].key_length,
                          elements[i].value);
    }
}

/**
 * @brief Receive an array of hashmap elements with MPI
 * 
 * First, receive the size of the array of hashmap elements and then receive the actual elements
 * 
 * @param rank MPI process rank
 * @param world_size Number of MPI processes in the current world
 * @param source The rank of the MPI process that sends the array of elements
 * @param support_map A pointer to the map where the received elements have to be inserted
 * @param DT_HASHMAP_ELEMENT An MPI datatype that describes the data structure received
 */
void recv_map(int rank, int world_size, int source, SupportMap *support_map,
              MPI_Datatype DT_HASHMAP_ELEMENT) {

    int size;
    MPI_Status status;

    MPI_Recv(&size, 1, MPI_INT, source, 0, MPI_COMM_WORLD, &status);
    hashmap_element *elements =
        (hashmap_element *)malloc(size * sizeof(hashmap_element));

    MPI_Recv(elements, size, DT_HASHMAP_ELEMENT, source, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);

    merge_map(support_map, elements, size);
    free(elements);
}

/**
 * @brief Send an array of hashmap elements with MPI.
 * 
 * First, send the size of the array of hashmap elements and then send the actual elements.
 * 
 * @param rank MPI process rank
 * @param world_size Number of MPI processes in the current world
 * @param dest The rank of the MPI process that will receive the array of elements
 * @param support_map A pointer to the map where the elements to be sent are stored
 * @param DT_HASHMAP_ELEMENT An MPI datatype that describes the data structure that has to be sent with MPI
 */
void send_map(int rank, int world_size, int dest, SupportMap *support_map,
              MPI_Datatype DT_HASHMAP_ELEMENT) {

    int size = hashmap_length(*support_map);
    cvector_vector_type(hashmap_element) elements = NULL;

    hashmap_get_elements(*support_map, &elements);
    // send size
    MPI_Send(&size, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
    // send buffer
    MPI_Send(elements, size, DT_HASHMAP_ELEMENT, dest, 0, MPI_COMM_WORLD);

    cvector_free(elements);
}

/**
 * @brief Broadcast all the elements of a SupportMap to every MPI process
 * 
 * This is used by the master process to send the global version of the map to every MPI process in the world
 * 
 * @param rank The rank of the process that broadcasts the data
 * @param world_size The number of MPI processes in the world
 * @param support_map The map from which to extract the elements to be sent
 * @param items_count A pointer to an array of hashmap elements having the item string as a key and the support count as a value
 * @param num_items A pointer to an integer describing the total number of items in the broadcasted map
 * @param DT_HASHMAP_ELEMENT An MPI datatype that describes the data structure that has to be sent with MPI
 * @param min_support The mininum support that an item has to have in order to be contained in the final map
 */
void broadcast_map(int rank, int world_size, SupportMap *support_map,
                   hashmap_element **items_count, int *num_items,
                   MPI_Datatype DT_HASHMAP_ELEMENT, int min_support) {
    if (rank == 0) {
        cvector_vector_type(hashmap_element) elements = NULL;
        hashmap_get_elements_with_support(*support_map, &elements, min_support);
        int size = cvector_size(elements);
        MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(elements, size, DT_HASHMAP_ELEMENT, 0, MPI_COMM_WORLD);
        *items_count = elements;
        *num_items = size;
    } else {
        int size;
        MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);
        hashmap_element *elements =
            (hashmap_element *)malloc(size * sizeof(hashmap_element));
        MPI_Bcast(elements, size, DT_HASHMAP_ELEMENT, 0, MPI_COMM_WORLD);
        *items_count = elements;
        *num_items = size;
    }
}

/**
 * @brief Get the global map object for every MPI process
 * 
 * This function follows a tree-like structure to build the global map of items of our program.
 * Initially, every process has its own partial map of items.
 * Then, processes which meet the condition (rank % pow != 0), where pow is a function of the current level in the tree, sends its partial map to the specified destination.
 * On the contrary, processes which meet the condition (rank % pow == 0) receive the map from the other processes, and then merge the received elements with their current map.
 * This is done until the only process in the current level is the process number 0. 
 * Upon reaching that state, it means that process 0 now has the complete map of every process and can broadcast its knowledge to the whole domain.
 * 
 * @param rank The rank of the process that broadcasts the data
 * @param world_size The number of MPI processes in the world
 * @param support_map The map from which to extract the elements to be sent
 * @param items_count A pointer to an array of hashmap elements having the item string as a key and the support count as a value
 * @param num_items A pointer to an integer describing the total number of items in the broadcasted map
 * @param min_support The mininum support that an item has to have in order to be contained in the final map
 */
void get_global_map(int rank, int world_size, SupportMap *support_map,
                    hashmap_element **items_count, int *num_items,
                    int min_support) {

    MPI_Datatype DT_HASHMAP_ELEMENT = define_datatype_hashmap_element();
    int pow;
    bool sent = false;
    for (pow = 2; pow < 2 * world_size && !sent; pow *= 2) {
        if (rank % pow == 0) {
            // receive and merge
            int source = rank + pow / 2;
            if (source < world_size) {
                recv_map(rank, world_size, source, support_map,
                         DT_HASHMAP_ELEMENT);
            }
        } else {
            int dest = rank - pow / 2;
            send_map(rank, world_size, dest, support_map, DT_HASHMAP_ELEMENT);
            sent = true;
        }
    }

    /** REINITIALIZE MAP TO HAVE ELEMENTS IN THE SAME ORDER AS OTHER PROCESSES
     * **/

    broadcast_map(rank, world_size, support_map, items_count, num_items,
                  DT_HASHMAP_ELEMENT, min_support);

    return;
}

/**
 * @brief Merge two arrays of sorted indices into a unique array of sorted indices
 * 
 * @param rank The rank of the process that broadcasts the data
 * @param sorted_indices The array of sorted indices that contains both arrays that have to be sorted
 * @param start1 The starting index of the first array
 * @param end1 The ending index of the first array
 * @param start2 The starting index of the second array
 * @param end2 The ending index of the second array
 * @param items_count An array of hashmap elements having the item string as a key and the support count as a value
 * @param num_items The number of items in the sorted_indices array
 */
void merge_indices(int rank, int *sorted_indices, int start1, int end1,
                   int start2, int end2, hashmap_element *items_count,
                   int num_items) {

    int tot_size = end1 - start1 + 1 + end2 - start2 + 1;
    assert(start1 + tot_size <= num_items);
    assert(end1 <= num_items);

    assert(end2 <= num_items);
    int *tmp = (int *)malloc(tot_size * sizeof(int));
    assert(tmp != NULL);

    int i = 0, j, i1 = start1, i2 = start2;

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
    while (i1 <= end1) {
        tmp[i] = sorted_indices[i1];
        i1++;
        i++;
    }
    while (i2 <= end2) {
        tmp[i] = sorted_indices[i2];
        i2++;
        i++;
    }
    assert(i == tot_size);
    assert(start1 + i <= num_items);
    // copy into original array
    for (j = 0; j < tot_size; j++) {
        sorted_indices[start1 + j] = tmp[j];
    }
    free(tmp);
}

/**
 * @brief Receive an array of sorted indices
 * 
 * @param rank The rank of the process that broadcasts the data
 * @param source The rank of the MPI process that sends the data
 * @param sorted_indices The array of sorted indices where the received data will be stored
 * @param start The starting index of the items that the function will receive
 * @param end The ending index of the items that the function will receive
 * @param length The length of each process' original interval width
 * @param size The current size of the received data
 * @param items_count An array of hashmap elements having the item string as a key and the support count as a value
 * @param num_items The number of items in the sorted_indices array
 */

void recv_indices(int rank, int source, int *sorted_indices,
                  int start, int *end, int length, int size,
                  hashmap_element *items_count, int num_items) {

    MPI_Status status;

    int position = source * length;
    MPI_Recv(sorted_indices + position, size, MPI_INT, source, 0,
             MPI_COMM_WORLD, &status);
    MPI_Get_count(&status, MPI_INT, &size);
    merge_indices(rank, sorted_indices, start, *end, position,
                  position + size - 1, items_count, num_items);
    *end = position + size - 1;
}

/**
 * @brief Send an array of sorted indices
 * 
 * @param rank The rank of the process that broadcasts the data
 * @param dest The rank of the MPI process that will receive the data
 * @param sorted_indices The array of sorted indices that will be used to send the data
 * @param start The starting index of the items that the function will send
 * @param end The ending index of the items that the function will send
 * @param length The length of each process' original interval width
 */
void send_indices(int rank, int dest, int *sorted_indices,
                  int start, int end, int length) {
    int size = end - start + 1;

    assert(size >= 0);
    // send buffer
    MPI_Send(sorted_indices + start, size, MPI_INT, dest, 0, MPI_COMM_WORLD);
}

/**
 * @brief Broadcast the final global array of indices to every MPI process in the world
 * 
 * @param rank The rank of the process that broadcasts the data
 * @param sorted_indices The array of sorted indices that has to be broadcasted
 * @param num_items The number of elements of the sorted_indices array
 */
void broadcast_indices(int rank, int *sorted_indices,
                       int num_items) {
    MPI_Bcast(sorted_indices, num_items, MPI_INT, 0, MPI_COMM_WORLD);
}

/**
 * @brief Get the global sorted indices array for every MPI process
 * 
 * This function follows a tree-like structure to build the global sorted array of indices of the items of our program.
 * Initially, every process has its own partial array of sorted indices.
 * Then, processes which meet the condition (rank % pow != 0), where pow is a function of the current level in the tree, sends its partial array to the specified destination.
 * On the contrary, processes which meet the condition (rank % pow == 0) receive the array from the other processes, and then merge the received elements with their current array.
 * This is done until the only process in the current level is the process number 0. 
 * Upon reaching that state, it means that process 0 now has the complete array of every process and can broadcast its knowledge to the whole domain.
 * 
 * @param rank The rank of the process that broadcasts the data
 * @param world_size The number of MPI processes in the current world
 * @param sorted_indices The array of sorted indices that will be used to send the data
 * @param start The starting index of the items that the function will send
 * @param end The ending index of the items that the function will send
 * @param length The length of each process' original interval width
 * @param items_count An array of hashmap elements having the item string as a key and the support count as a value
 * @param num_items The number of items in the sorted_indices array
 */

void get_sorted_indices(int rank, int world_size, int *sorted_indices,
                        int start, int end, int length,
                        hashmap_element *items_count, int num_items) {
    int pow;
    bool sent = false;
    for (pow = 2; pow < 2 * world_size && !sent; pow *= 2) {
        if (rank % pow == 0) {
            // receive and merge
            int source = rank + pow / 2;
            if (source < world_size) {
                recv_indices(rank, source, sorted_indices, start,
                             &end, length, length * (pow / 2), items_count,
                             num_items);
            }
        } else {
            int dest = rank - pow / 2;
            assert(end - start + 1 <= num_items);
            send_indices(rank, dest, sorted_indices, start, end,
                         length);
            sent = true;
        }
    }
    broadcast_indices(rank, sorted_indices, num_items);
    return;
}

/**
 * @brief Parse an array of TreeNodesToSend into a Tree structure
 * 
 * @param nodes The array of TreeNodesToSend that have to be parsed
 * @param num_nodes The size of the nodes array
 * @param dest A pointer to the tree that will be created
 */
void parse_tree(TreeNodeToSend *nodes, int num_nodes, Tree *dest) {
    *dest = tree_new();
    for (int i = 1; i < num_nodes; i++) {
        TreeNode *node =
            tree_node_new(nodes[i].key, nodes[i].value, nodes[i].parent);
        tree_add_node(dest, node);
    }
}

/**
 * @brief Sends a tree to an MPI process and frees up the memory
 *  
 * @param dest The destination process that will receive the tree
 * @param tree The tree that has to be sent
 * @param DT_TREE_NODE MPI_Datatype describing a TreeNode
 */
void send_tree(int dest, Tree *tree,
               MPI_Datatype DT_TREE_NODE) {
    int size = cvector_size((*tree));
    cvector_vector_type(TreeNodeToSend) nodes = NULL;

    tree_get_nodes(*tree, &nodes);
    free_tree(tree);

    // send size
    MPI_Send(&size, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
    // send buffer
    MPI_Send(nodes, size, DT_TREE_NODE, dest, 0, MPI_COMM_WORLD);

    cvector_free(nodes);
}

/**
 * @brief Receive a tree from an MPI process
 * 
 * @param source The MPI process that is sending the data
 * @param tree A pointer to partial tree of the current process, which will be integrated by merging the received tree
 * @param DT_TREE_NODE MPI_Datatype describing a TreeNode
 */
void recv_tree(int source, Tree *tree,
               MPI_Datatype DT_TREE_NODE) {

    int size;
    MPI_Status status;
    MPI_Recv(&size, 1, MPI_INT, source, 0, MPI_COMM_WORLD, &status);

    TreeNodeToSend *nodes =
        (TreeNodeToSend *)malloc(size * sizeof(TreeNodeToSend));

    MPI_Recv(nodes, size, DT_TREE_NODE, source, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
    Tree received_tree;
    parse_tree(nodes, size, &received_tree);

    tree_merge(tree, received_tree);
    free(nodes);
}

/**
 * @brief Broadcast the final FP-Tree to every MPI process in the world
 * 
 * @param rank The rank of the process that broadcasts the data
 * @param tree The tree that has to be broadcasted in case the process rank is 0, otherwise the tree which the final tree will be stored
 * @param DT_TREE_NODE MPI_Datatype describing a TreeNode
 */

void broadcast_tree(int rank, Tree *tree,
                    MPI_Datatype DT_TREE_NODE) {
    if (rank == 0) {
        int size = cvector_size((*tree));
        cvector_vector_type(TreeNodeToSend) nodes = NULL;
        tree_get_nodes(*tree, &nodes);
        // send size
        MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);
        // send buffer
        MPI_Bcast(nodes, size, DT_TREE_NODE, 0, MPI_COMM_WORLD);

        cvector_free(nodes);
    } else {
        int size;
        MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);
        TreeNodeToSend *nodes =
            (TreeNodeToSend *)malloc(size * sizeof(TreeNodeToSend));

        MPI_Bcast(nodes, size, DT_TREE_NODE, 0, MPI_COMM_WORLD);
        Tree received_tree;
        parse_tree(nodes, size, &received_tree);
        *tree = received_tree;

        free(nodes);
    }
}

/**
 * @brief Get the global FP-tree on every MPI process
 * 
 * This function follows a tree-like structure to build the final FP-tree.
 * Initially, every process has its own partial tree composed by the transactions of its local assigned chunk.
 * Then, processes which meet the condition (rank % pow != 0), where pow is a function of the current level in the tree, sends its partial tree to the specified destination.
 * On the contrary, processes which meet the condition (rank % pow == 0) receive the FP-tree from the other processes, and then merge the received partial FP-tree with their current FP-tree.
 * This is done until the only process in the current level is the process number 0. 
 * Upon reaching that state, it means that process 0 now has the complete FP-tree and can broadcast its knowledge to the whole domain.
 * 
 * @param rank The rank of the process that broadcasts the data
 * @param world_size The number of processes in the current world
 * @param tree The tree that has to be sent/received. This structure is heavily manipulated during the execution of this function.
 */

void get_global_tree(int rank, int world_size, Tree *tree) {
    MPI_Datatype DT_TREE_NODE = define_datatype_tree_node();
    int pow;
    bool sent = false;
    for (pow = 2; pow < 2 * world_size && !sent; pow *= 2) {
        if (rank % pow == 0) {
            // receive and merge
            int source = rank + pow / 2;
            if (source < world_size) {
                recv_tree(source, tree, DT_TREE_NODE);
            }
        } else {
            int dest = rank - pow / 2;
            send_tree(dest, tree, DT_TREE_NODE);
            sent = true;
        }
    }

    broadcast_tree(rank, tree, DT_TREE_NODE);

    return;
}