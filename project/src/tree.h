/**
 * @file tree.h
 * @brief Definition of FP-Tree and functions to build it
 * 
 */
#ifndef TREE_H
#define TREE_H
#define TREE_NODE_NULL -1
#include "types.h"

/**
 * @brief A node of an FP-Tree
 *
 */
typedef struct TreeNode {
    /**
     * @brief Id of the item represented by the node   
     */
    int key;   
    /**
     * @brief Value of the item represented by the node  
     */
    int value; 
    /**
     * @brief Index of the parent of the node in the tree  
     */
    int parent;
    /**
     * @brief Adjacency map of the node, where the values are the
     *        indices of the children of the node in the tree and
     *        the keys are the ids of the corresponding items 
     */
    map_t adj;
} TreeNode;

/**
 * @brief A node of an FP-Tree to send with MPI.
 * 
 */
typedef struct TreeNodeToSend {
    /**
     * @brief Id of the item represented by the node   
     */
    int key;
    /**
     * @brief Value of the item represented by the node  
     */
    int value;
    /**
     * @brief Index of the parent of the node in the tree  
     */
    int parent;
} TreeNodeToSend;

/**
 * @brief FP-Tree
 */
typedef cvector_vector_type(TreeNode *) Tree;


/**
 * @brief Instantiate a new node of the tree.
 *
 * @param key The key of the node
 * @param value The value of the node
 * @param parent The parent of the node in the tree
 * @return Pointer to the node created
 */
TreeNode *tree_node_new(int key, int value, int parent);

/**
 * @brief Free a tree node
 *
 * @param node Pointer to the node to free
 */
void tree_node_free(TreeNode *node);

/**
 * @brief Instantiate a new tree
 *
 * @return The new tree
 */
Tree tree_new();

/**
 * @brief Free the tree
 *
 * @param tree Pointer to the tree to free
 */
void tree_free(Tree *tree);

/**
 * @brief Add a node to the tree
 *
 * @param tree Pointer to the tree
 * @param node Pointer to the node to add
 * @return The id of the node in the tree
 */
int tree_add_node(Tree *tree, TreeNode *node);

/**
 * @brief Add the subtree rooted in the ns(th) node of the tree
 * source as a child of the nd(th) node of the tree dest. The
 * source tree is modified, as the nodes are moved to the
 * destination tree.
 *
 * @param dest Pointer to the destination tree
 * @param source Pointer to the source tree
 * @param nd Id of the node in the destination tree
 * @param ns Id of the node in the source tree
 */
void tree_add_subtree(Tree *dest, Tree source, int nd, int ns);

/**
 * @brief Merge the subtree of dest rooted in node with id nd
 * with the subtree of source rooted in ns and store the result in dest.
 * Also the source tree is modified.
 *
 * @param dest The destination tree
 * @param source The source tree
 * @param nd Id of the node in the destination tree
 * @param ns Id of the node in the source tree
 */
void tree_merge_dfs(Tree *dest, Tree source, int nd, int ns);

/**
 * @brief Merge the trees dest and source and store the result in dest.
 * The source tree is modified. It is a wrapper for @see tree_merge_dfs()
 *
 * @param dest The destination tree
 * @param source The source tree
 */
void tree_merge(Tree *dest, Tree source);

/**
 * @brief Inserts into the vector nodes the nodes to send
 *
 * @param tree The trees from which to get the nodes
 * @param nodes The vector in which the nodes are put
 */
void tree_get_nodes(Tree tree, cvector_vector_type(TreeNodeToSend) * nodes);

/**
 * @brief Print the tree
 *
 * @param tree The tree to print
 */
void tree_print(Tree tree);

/**
 * @brief Build a tree given a transaction
 *
 * @param rank The rank of the process
 * @param world_size The number of processes in the world
 * @param transaction The transaction
 * @param index_map The map from item to the corresponding id
 * @param items_count The array of hashmap elements having the item string as a
 * key and the support count as a value
 * @param num_items The number of items in the sorted_indices array
 * @param sorted_indices The array of the sorted indices of the items
 * @return The built tree
 */
Tree tree_build_from_transaction(int rank, int world_size,
                                 Transaction *transaction, IndexMap index_map,
                                 hashmap_element *items_count, int num_items,
                                 int *sorted_indices);

/**
 * @brief Build a tree given a list of transactions
 *
 * First, we build the trees for the single transactions.
 * Then, we merge them in a binary-tree-like fashion.
 *
 * @param rank The rank of the process
 * @param world_size The number of processes in the world
 * @param transactions
 * @param index_map The map from item to the corresponding id
 * @param items_count The array of hashmap elements having the item string as a
 * key and the support count as a value
 * @param num_items The number of items in the sorted_indices array
 * @param sorted_indices The array of the sorted indices of the items
 * @param num_threads The number of threads requested to perform the building
 * @return The built tree
 */
Tree tree_build_from_transactions(int rank, int world_size,
                                  TransactionsList transactions,
                                  IndexMap index_map,
                                  hashmap_element *items_count, int num_items,
                                  int *sorted_indices, int num_threads);

#endif