#ifndef TREE_H
#define TREE_H
#define TREE_NODE_NULL -1
#include "types.h"

typedef struct TreeNode {
    int key;
    int value;
    int parent;
    map_t adj;
} TreeNode;
typedef struct TreeNodeToSend {
    int key;
    int value;
    int parent;
} TreeNodeToSend;

typedef cvector_vector_type(TreeNode *) Tree;

TreeNode *tree_node_new(int key, int value, int parent);
void tree_node_free(TreeNode *node);

Tree tree_new();
void tree_free(Tree *tree);
int tree_add_node(Tree *tree, TreeNode *node);
void tree_merge(Tree *dest, Tree source);
void tree_get_nodes(Tree tree, cvector_vector_type(TreeNodeToSend) * nodes);
void tree_print(Tree tree);

Tree tree_build_from_transactions(int rank, int world_size,
                                  TransactionsList transactions,
                                  IndexMap index_map,
                                  hashmap_element *items_count, int num_items,
                                  int *sorted_indices, int num_threads);

#endif