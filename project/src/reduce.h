/**
 * @file reduce.h
 * @brief Functions that perform the exchange of data between MPI processes
 *
 */
#ifndef REDUCE_H
#define REDUCE_H

#include "mpi.h"
#include "tree.h"
#include "types.h"


/**
 * @brief Define a datatype for an hashmap element in order to be able to send
 * it with MPI
 *
 * @return MPI_Datatype describing an hashmap element
 */
MPI_Datatype define_datatype_hashmap_element();

/**
 * @brief Define an MPI Datatype for a TreeNode in order to be able to send it
 * with MPI
 *
 * @return MPI_Datatype describing a TreeNode
 */
MPI_Datatype define_datatype_tree_node();

/**
 * @brief Merge the current map with an array of elements
 *
 * @param support_map A pointer to the map which will be populated with the
 * merged items
 * @param elements The array of hashmap elements to merge
 * @param size The size of the elements array
 */
void merge_map(SupportMap *support_map, hashmap_element *elements, int size);

/**
 * @brief Receive an array of hashmap elements with MPI
 *
 * First, receive the size of the array of hashmap elements and then receive the
 * actual elements
 *
 * @param rank MPI process rank
 * @param world_size Number of MPI processes in the current world
 * @param source The rank of the MPI process that sends the array of elements
 * @param support_map A pointer to the map where the received elements have to
 * be inserted
 * @param DT_HASHMAP_ELEMENT An MPI datatype that describes the data structure
 * received
 */
void recv_map(int rank, int world_size, int source, SupportMap *support_map,
              MPI_Datatype DT_HASHMAP_ELEMENT);

/**
 * @brief Send an array of hashmap elements with MPI.
 *
 * First, send the size of the array of hashmap elements and then send the
 * actual elements.
 *
 * @param rank MPI process rank
 * @param world_size Number of MPI processes in the current world
 * @param dest The rank of the MPI process that will receive the array of
 * elements
 * @param support_map A pointer to the map where the elements to be sent are
 * stored
 * @param DT_HASHMAP_ELEMENT An MPI datatype that describes the data structure
 * that has to be sent with MPI
 */
void send_map(int rank, int world_size, int dest, SupportMap *support_map,
              MPI_Datatype DT_HASHMAP_ELEMENT);

/**
 * @brief Broadcast all the elements of a SupportMap to every MPI process
 *
 * This is used by the master process to send the global version of the map to
 * every MPI process in the world
 *
 * @param rank The rank of the process that broadcasts the data
 * @param world_size The number of MPI processes in the world
 * @param support_map The map from which to extract the elements to be sent
 * @param items_count A pointer to an array of hashmap elements having the item
 * string as a key and the support count as a value
 * @param num_items A pointer to an integer describing the total number of items
 * in the broadcasted map
 * @param DT_HASHMAP_ELEMENT An MPI datatype that describes the data structure
 * that has to be sent with MPI
 * @param min_support The mininum support that an item has to have in order to
 * be contained in the final map
 */
void broadcast_map(int rank, int world_size, SupportMap *support_map,
                   hashmap_element **items_count, int *num_items,
                   MPI_Datatype DT_HASHMAP_ELEMENT, int min_support);

/**
 * @brief Get the global map object for every MPI process
 *
 * This function follows a tree-like structure to build the global map of items
 * of our program. Initially, every process has its own partial map of items.
 * Then, processes which meet the condition (rank % pow != 0), where pow is a
 * function of the current level in the tree, sends its partial map to the
 * specified destination. On the contrary, processes which meet the condition
 * (rank % pow == 0) receive the map from the other processes, and then merge
 * the received elements with their current map. This is done until the only
 * process in the current level is the process number 0. Upon reaching that
 * state, it means that process 0 now has the complete map of every process and
 * can broadcast its knowledge to the whole domain.
 *
 * @param rank The rank of the process that broadcasts the data
 * @param world_size The number of MPI processes in the world
 * @param support_map The map from which to extract the elements to be sent
 * @param items_count A pointer to an array of hashmap elements having the item
 * string as a key and the support count as a value
 * @param num_items A pointer to an integer describing the total number of items
 * in the broadcasted map
 * @param min_support The mininum support that an item has to have in order to
 * be contained in the final map
 */
void get_global_map(int rank, int world_size, SupportMap *support_map,
                    hashmap_element **items_count, int *num_items,
                    int min_support);

/**
 * @brief Merge two arrays of sorted indices into a unique array of sorted
 * indices
 *
 * @param rank The rank of the process that broadcasts the data
 * @param sorted_indices The array of sorted indices that contains both arrays
 * that have to be sorted
 * @param start1 The starting index of the first array
 * @param end1 The ending index of the first array
 * @param start2 The starting index of the second array
 * @param end2 The ending index of the second array
 * @param items_count An array of hashmap elements having the item string as a
 * key and the support count as a value
 * @param num_items The number of items in the sorted_indices array
 */
void merge_indices(int rank, int *sorted_indices, int start1, int end1,
                   int start2, int end2, hashmap_element *items_count,
                   int num_items);

/**
 * @brief Receive an array of sorted indices
 *
 * @param rank The rank of the process that broadcasts the data
 * @param source The rank of the MPI process that sends the data
 * @param sorted_indices The array of sorted indices where the received data
 * will be stored
 * @param start The starting index of the items that the function will receive
 * @param end The ending index of the items that the function will receive
 * @param length The length of each process' original interval width
 * @param size The current size of the received data
 * @param items_count An array of hashmap elements having the item string as a
 * key and the support count as a value
 * @param num_items The number of items in the sorted_indices array
 */
void recv_indices(int rank, int source, int *sorted_indices, int start,
                  int *end, int length, int size, hashmap_element *items_count,
                  int num_items);

/**
 * @brief Send an array of sorted indices
 *
 * @param rank The rank of the process that broadcasts the data
 * @param dest The rank of the MPI process that will receive the data
 * @param sorted_indices The array of sorted indices that will be used to send
 * the data
 * @param start The starting index of the items that the function will send
 * @param end The ending index of the items that the function will send
 * @param length The length of each process' original interval width
 */
void send_indices(int rank, int dest, int *sorted_indices, int start, int end,
                  int length);

/**
 * @brief Broadcast the final global array of indices to every MPI process in
 * the world
 *
 * @param rank The rank of the process that broadcasts the data
 * @param sorted_indices The array of sorted indices that has to be broadcasted
 * @param num_items The number of elements of the sorted_indices array
 */
void broadcast_indices(int rank, int *sorted_indices, int num_items);

/**
 * @brief Get the global sorted indices array for every MPI process
 *
 * This function follows a tree-like structure to build the global sorted array
 * of indices of the items of our program. Initially, every process has its own
 * partial array of sorted indices. Then, processes which meet the condition
 * (rank % pow != 0), where pow is a function of the current level in the tree,
 * sends its partial array to the specified destination. On the contrary,
 * processes which meet the condition (rank % pow == 0) receive the array from
 * the other processes, and then merge the received elements with their current
 * array. This is done until the only process in the current level is the
 * process number 0. Upon reaching that state, it means that process 0 now has
 * the complete array of every process and can broadcast its knowledge to the
 * whole domain.
 *
 * @param rank The rank of the process that broadcasts the data
 * @param world_size The number of MPI processes in the current world
 * @param sorted_indices The array of sorted indices that will be used to send
 * the data
 * @param start The starting index of the items that the function will send
 * @param end The ending index of the items that the function will send
 * @param length The length of each process' original interval width
 * @param items_count An array of hashmap elements having the item string as a
 * key and the support count as a value
 * @param num_items The number of items in the sorted_indices array
 */
void get_sorted_indices(int rank, int world_size, int *sorted_indices,
                        int start, int end, int length,
                        hashmap_element *items_count, int num_items);

/**
 * @brief Parse an array of TreeNodesToSend into a Tree structure
 *
 * @param nodes The array of TreeNodesToSend that have to be parsed
 * @param num_nodes The size of the nodes array
 * @param dest A pointer to the tree that will be created
 */
void parse_tree(TreeNodeToSend *nodes, int num_nodes, Tree *dest);

/**
 * @brief Sends a tree to an MPI process and frees up the memory
 *
 * @param dest The destination process that will receive the tree
 * @param tree The tree that has to be sent
 * @param DT_TREE_NODE MPI_Datatype describing a TreeNode
 */
void send_tree(int dest, Tree *tree, MPI_Datatype DT_TREE_NODE);

/**
 * @brief Receive a tree from an MPI process
 *
 * @param source The MPI process that is sending the data
 * @param tree A pointer to partial tree of the current process, which will be
 * integrated by merging the received tree
 * @param DT_TREE_NODE MPI_Datatype describing a TreeNode
 */
void recv_tree(int source, Tree *tree, MPI_Datatype DT_TREE_NODE);

/**
 * @brief Broadcast the final FP-Tree to every MPI process in the world
 *
 * @param rank The rank of the process that broadcasts the data
 * @param tree The tree that has to be broadcasted in case the process rank is
 * 0, otherwise the tree which the final tree will be stored
 * @param DT_TREE_NODE MPI_Datatype describing a TreeNode
 */
void broadcast_tree(int rank, Tree *tree, MPI_Datatype DT_TREE_NODE);

/**
 * @brief Get the global FP-tree on every MPI process
 *
 * This function follows a tree-like structure to build the final FP-tree.
 * Initially, every process has its own partial tree composed by the
 * transactions of its local assigned chunk. Then, processes which meet the
 * condition (rank % pow != 0), where pow is a function of the current level in
 * the tree, sends its partial tree to the specified destination. On the
 * contrary, processes which meet the condition (rank % pow == 0) receive the
 * FP-tree from the other processes, and then merge the received partial FP-tree
 * with their current FP-tree. This is done until the only process in the
 * current level is the process number 0. Upon reaching that state, it means
 * that process 0 now has the complete FP-tree and can broadcast its knowledge
 * to the whole domain.
 *
 * @param rank The rank of the process that broadcasts the data
 * @param world_size The number of processes in the current world
 * @param tree The tree that has to be sent/received. This structure is heavily
 * manipulated during the execution of this function.
 */

void get_global_tree(int rank, int world_size, Tree *tree);


#endif