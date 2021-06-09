#include "tree.h"
#include "io.h"
#include "sort.h"
#include <omp.h>
#include <stdio.h>
#include <string.h>

/**
 * @brief Instantiate a new node of the tree.
 *
 * @param key The key of the node
 * @param value The value of the node
 * @param parent The parent of the node in the tree
 * @return Pointer to the node created
 */
TreeNode *tree_node_new(int key, int value, int parent) {
    TreeNode *node = (TreeNode *)malloc(sizeof(TreeNode));
    assert(node != NULL);
    node->key = key;
    node->value = value;
    node->parent = parent;
    node->adj = hashmap_new();
    assert(node->adj != NULL);
    return node;
}

/**
 * @brief Free a tree node
 *
 * @param node Pointer to the node to free
 */
void tree_node_free(TreeNode *node) {
    if (node != NULL) {
        hashmap_free(node->adj);
        free(node);
    }
}

/**
 * @brief Instantiate a new tree
 *
 * @return The new tree
 */
Tree tree_new() {
    TreeNode *node = tree_node_new(TREE_NODE_NULL, -1, 0);
    Tree tree = NULL;
    cvector_push_back(tree, node);
    return tree;
}

/**
 * @brief Free the tree
 *
 * @param tree Pointer to the tree to free
 */
void tree_free(Tree *tree) {
    if (*tree != NULL) {
        int n_nodes = cvector_size((*tree));
        int i;
        for (i = 0; i < n_nodes; i++) {
            tree_node_free((*tree)[i]);
        }
        cvector_free((*tree));
        *tree = NULL;
    }
}

/**
 * @brief Add a node to the tree
 *
 * @param tree Pointer to the tree
 * @param node Pointer to the node to add
 * @return The id of the node in the tree
 */
int tree_add_node(Tree *tree, TreeNode *node) {
    cvector_push_back((*tree), node);
    assert(*tree != NULL); // the malloc has not failed
    int new_id = cvector_size((*tree)) - 1;
    TreeNode *parent = (*tree)[node->parent];
    hashmap_put(parent->adj, &(node->key), sizeof(int), new_id);
    assert(new_id != node->parent);
    return new_id;
}

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
void tree_add_subtree(Tree *dest, Tree source, int nd, int ns) {
    // add node ns
    cvector_vector_type(hashmap_element) neighbours = NULL;
    hashmap_get_elements(source[ns]->adj, &neighbours);
    hashmap_free(source[ns]->adj);
    source[ns]->adj = hashmap_new();
    source[ns]->parent = nd;
    int new_pos = tree_add_node(dest, source[ns]);
    int num_adj_s = cvector_size(neighbours);

    // recursively add children
    int i;
    for (i = 0; i < num_adj_s; i++) {
        assert(neighbours[i].value != ns);
        tree_add_subtree(dest, source, new_pos, neighbours[i].value);
    }
    source[ns] = NULL;
    cvector_free(neighbours);
}

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
void tree_merge_dfs(Tree *dest, Tree source, int nd, int ns) {
    int i;
    cvector_vector_type(hashmap_element) neighbours = NULL;
    hashmap_get_elements(source[ns]->adj, &neighbours);
    int num_adj_s = cvector_size(neighbours);
    // foreach neighbour of node ns in source
    for (i = 0; i < num_adj_s; i++) {
        int source_pos = neighbours[i].value;
        assert(ns != neighbours[i].value);
        // if a node with the same key(item) is already present in the
        // children of nd, just increment the counter
        int dest_pos;
        if (hashmap_get((*dest)[nd]->adj, neighbours[i].key, sizeof(int),
                        &dest_pos) == MAP_OK) {

            (*dest)[dest_pos]->value += source[source_pos]->value;

            tree_merge_dfs(dest, source, dest_pos, source_pos);

        } else {
            // otherwise add the child and the subtree rooted in it to
            // the node nd in dest
            tree_add_subtree(dest, source, nd, source_pos);
        }
    }
    cvector_free(neighbours);
}

/**
 * @brief Merge the trees dest and source and store the result in dest.
 * The source tree is modified. It is a wrapper for @see tree_merge_dfs()
 *
 * @param dest The destination tree
 * @param source The source tree
 */
void tree_merge(Tree *dest, Tree source) { tree_merge_dfs(dest, source, 0, 0); }

/**
 * @brief Inserts into the vector nodes the nodes to send
 *
 * @param tree The trees from which to get the nodes
 * @param nodes The vector in which the nodes are put
 */
void tree_get_nodes(Tree tree, cvector_vector_type(TreeNodeToSend) * nodes) {
    int num_nodes = cvector_size(tree);
    for (int i = 0; i < num_nodes; i++) {
        TreeNodeToSend node;
        node.key = tree[i]->key;
        node.value = tree[i]->value;
        node.parent = tree[i]->parent;
        cvector_push_back((*nodes), node);
    }
}

/**
 * @brief Print the tree
 *
 * @param tree The tree to print
 */
void tree_print(Tree tree) {
    int n_nodes = cvector_size(tree);

    for (int i = 0; i < n_nodes; i++) {
        printf("Node (%d: %d)\n", tree[i]->key, tree[i]->value);
        hashmap_print(tree[i]->adj);
    }
}

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
                                 int *sorted_indices) {

    int n_items = cvector_size((*transaction));
    cvector_vector_type(hashmap_element) elements = NULL;

    for (int i = 0; i < n_items; i++) {
        int item_size = cvector_size((*transaction)[i]);
        hashmap_element element;
        // consider only items with support >= min_support (in index map)
        if (hashmap_get(index_map, (*transaction)[i], item_size,
                        &(element.value)) == MAP_OK) {
            element.key_length = item_size;
            memcpy(element.key, (*transaction)[i], item_size);
            cvector_push_back(elements, element);
        }
    }
    transaction_free(transaction);

    n_items = cvector_size(elements);

    int *transaction_sorted_indices = (int *)malloc(n_items * sizeof(int));
    sort(elements, n_items, transaction_sorted_indices, 0, n_items - 1, 1);

    Tree tree = tree_new();
    for (int i = 0; i < n_items; i++) {
        assert(transaction_sorted_indices[i] >= 0);
        assert(transaction_sorted_indices[i] < n_items);
        Item item = elements[transaction_sorted_indices[i]].key;
        int item_size = elements[transaction_sorted_indices[i]].key_length;

        int pos;
        assert(hashmap_get(index_map, item, item_size, &pos) == MAP_OK);
        assert(pos >= 0);
        assert(pos < num_items);
        assert(sorted_indices[pos] >= 0);
        assert(sorted_indices[pos] < num_items);

        TreeNode *node = tree_node_new(sorted_indices[pos], 1, i);
        assert(node != NULL);
        assert(tree_add_node(&tree, node) == i + 1);
    }
    cvector_free(elements);

    free(transaction_sorted_indices);
    return tree;
}

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
                                  int *sorted_indices, int num_threads) {

    int n_transactions = cvector_size(transactions);
    Tree *trees = (Tree *)malloc(n_transactions * sizeof(Tree));
    int i, pow;

#pragma omp parallel default(none)                                             \
    shared(n_transactions, trees, rank, world_size, transactions, index_map,   \
           items_count, num_items, sorted_indices) private(pow, i)             \
        num_threads(num_threads)
    for (pow = 1; pow < 2 * n_transactions; pow *= 2) {
        int start = pow == 1 ? 0 : pow / 2;
#pragma omp for
        for (i = start; i < n_transactions; i += pow) {
            if (pow > 1) {
                // at levels > 1, merge two subtrees
                tree_merge(&trees[i - pow / 2], trees[i]);
                tree_free(&(trees[i]));
            } else {
                // at first level, build the transaction trees
                trees[i] = tree_build_from_transaction(
                    rank, world_size, &(transactions[i]), index_map,
                    items_count, num_items, sorted_indices);
            }
        }
    }
    Tree res = trees[0];
    free(trees);
    return res;
}
