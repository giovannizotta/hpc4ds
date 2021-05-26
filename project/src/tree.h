#ifndef TREE_H
#define TREE_H
#define TREE_NODE_NULL -1
#include "types.h"

typedef struct TreeNode {
    int key;
    int value;
    int parent;
    map_t adj; // key: key
} TreeNode;

typedef cvector_vector_type(TreeNode *) Tree;

Tree build_MPI_tree(int rank, int world_size, TransactionsList transactions,
                    map_t index_map, hashmap_element *items_count,
                    int num_items, int *sorted_indices, int num_threads);

TreeNode *init_tree_node(int key, int value, int parent);
void free_tree_node(TreeNode *node);
Tree init_tree();
void free_tree(Tree *tree);
int add_tree_node(Tree *tree, TreeNode *node);
void merge_trees(Tree *dest, Tree source);
void merge_trees_dfs(Tree *dest, Tree source, int nd, int ns);
// MPI_DATATYPE {
//     int k;
//     int v;
//     int parent;
// }
// Node * nodes;

#endif