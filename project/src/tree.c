#include "tree.h"
#include "io.h"
#include "sort.h"
#include <omp.h>
#include <stdio.h>
#include <string.h>

/**
 * Instantiate and return a new node of the tree.
 *
 * @param[in] key the key of the node
 * @param[in] value the value of the node
 * @param[in] parent the parent of the node in the tree
 * @return a pointer to the node created
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

void tree_print(Tree tree) {
    int n_nodes = cvector_size(tree);

    for (int i = 0; i < n_nodes; i++) {
        printf("Node (%d: %d)\n", tree[i]->key, tree[i]->value);
        hashmap_print(tree[i]->adj);
    }
}

void tree_node_free(TreeNode *node) {
    if (node != NULL) {
        hashmap_free(node->adj);
        free(node);
    }
}

Tree tree_new() {
    TreeNode *node = tree_node_new(TREE_NODE_NULL, -1, 0);
    Tree tree = NULL;
    cvector_push_back(tree, node);
    return tree;
}

void free_tree(Tree *tree) {
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
 * Return the index of the added node in the tree
 */
int tree_add_node(Tree *tree, TreeNode *node) {
    cvector_push_back((*tree), node);
    assert(*tree != NULL);
    int new_id = cvector_size((*tree)) - 1;
    TreeNode *parent = (*tree)[node->parent];
    hashmap_put(parent->adj, &(node->key), sizeof(int), new_id);
    assert(new_id != node->parent);
    return new_id;
}

void add_subtree(Tree *dest, Tree source, int nd, int ns) {
    // add myself
    // printf("Add node nd: %d, ns: %d\n", nd, ns);
    cvector_vector_type(hashmap_element) neighbours = NULL;
    hashmap_get_elements(source[ns]->adj, &neighbours);
    // printf("get elements %lu\n", cvector_size(neighbours));
    hashmap_free(source[ns]->adj);
    source[ns]->adj = hashmap_new();

    source[ns]->parent = nd;
    int new_pos = tree_add_node(dest, source[ns]);
    int num_adj_s = cvector_size(neighbours);
    int i;

    // printf("map free\n");
    // add children recursively
    for (i = 0; i < num_adj_s; i++) {
        // printf("Adding child %d\n", i);
        assert(neighbours[i].value != ns);
        add_subtree(dest, source, new_pos, neighbours[i].value);
    }
    // printf("Node ok\n");
    source[ns] = NULL;
    cvector_free(neighbours);
}

void tree_merge_dfs(Tree *dest, Tree source, int nd, int ns) {
    // printf("Merging trees nd: %d, ns: %d ld: %lu, ls: %lu \n", nd, ns,
    //    cvector_size((*dest)), cvector_size(source));
    // printf("Source has key %d\n", source[ns]->key);
    int i;
    // assert(source[ns]->adj != NULL);
    // int num_adj_d = hashmap_length((*dest)[nd]->adj);
    cvector_vector_type(hashmap_element) neighbours = NULL;
    hashmap_get_elements(source[ns]->adj, &neighbours);
    int num_adj_s = cvector_size(neighbours);

    for (i = 0; i < num_adj_s; i++) {
        // printf("%dth neighbour\n", i);
        // int source_item = (int *)(neighbours[i].key);
        int source_pos = neighbours[i].value;
        assert(ns != neighbours[i].value);
        int dest_pos;
        if (hashmap_get((*dest)[nd]->adj, neighbours[i].key, sizeof(int),
                        &dest_pos) == MAP_OK) {
            // printf("Present\n");
            // int new_value = (*dest)[dest_pos]->value +
            // source[ns]->value; assert(hashmap_put((*dest)[nd]->adj,
            // neighbours[i].key, sizeof(int),
            //                    new_value) == MAP_OK);
            (*dest)[dest_pos]->value += source[source_pos]->value;
            // printf("Increase value (new value: %d)\n",
            //        (*dest)[dest_pos]->value);

            tree_merge_dfs(dest, source, dest_pos, source_pos);
            // printf("recusive call ended\n");

        } else {
            // printf("Not there, adding subtree\n");
            add_subtree(dest, source, nd, source_pos);
            // printf("Subtree added\n");
        }
    }
    cvector_free(neighbours);
}

void tree_merge(Tree *dest, Tree source) { tree_merge_dfs(dest, source, 0, 0); }

Tree tree_build_from_transaction(int rank, int world_size,
                                 Transaction *transaction, IndexMap index_map,
                                 hashmap_element *items_count, int num_items,
                                 int *sorted_indices) {
    // printf("%d C.\n", rank);

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
    free_transaction(transaction);

    // printf("%d D.\n", rank);
    n_items = cvector_size(elements);

    int *transaction_sorted_indices = (int *)malloc(n_items * sizeof(int));
    sort(elements, n_items, transaction_sorted_indices, 0, n_items - 1, 1);

    Tree tree = tree_new();
    // printf("%d E.\n", rank);
    for (int i = 0; i < n_items; i++) {
        assert(transaction_sorted_indices[i] >= 0);
        assert(transaction_sorted_indices[i] < n_items);
        Item item = elements[transaction_sorted_indices[i]].key;
        // printf("%d Ea.\n", rank);
        int item_size = elements[transaction_sorted_indices[i]].key_length;
        // printf("%d Eb.\n", rank);
        int pos;
        assert(hashmap_get(index_map, item, item_size, &pos) == MAP_OK);
        // printf("%d Ec pos: %d (out of %d)\n", rank, pos, num_items);
        assert(pos >= 0);
        assert(pos < num_items);
        assert(sorted_indices[pos] >= 0);
        assert(sorted_indices[pos] < num_items);
        // int value = items_count[sorted_indices[pos]].value;

        TreeNode *node = tree_node_new(sorted_indices[pos], 1, i);
        assert(node != NULL);
        assert(tree_add_node(&tree, node) == i + 1);

        // printf("%d Ed value %d.\n", rank, value);
        // printf("(%s : %d [%d]) ", item, pos, value);
    }
    cvector_free(elements);

    free(transaction_sorted_indices);
    // free_tree(&tree);

    // build tree

    // free(transaction_sorted_indices);
    // free(elements);
    // printf("%d F.\n", rank);
    return tree;
}

Tree tree_build_from_transactions(int rank, int world_size,
                                  TransactionsList transactions,
                                  IndexMap index_map,
                                  hashmap_element *items_count, int num_items,
                                  int *sorted_indices, int num_threads) {
    // Tree process_tree;
    // # omp parallel for
    // Tree trees[];
    // for i, transaction in transactions:
    //     trees[i] = tree_build_from_transactions(transaction)

    // 0
    //
    // 0       4
    //
    // 0   2   4   6
    //
    // 0 1 2 3 4 5 6 7

    int n_transactions = cvector_size(transactions);
    Tree *trees = (Tree *)malloc(n_transactions * sizeof(Tree));
    int i, pow;
    // printf("%d B trans %d.\n", rank, n_transactions);

#pragma omp parallel default(none)                                             \
    shared(n_transactions, trees, rank, world_size, transactions, index_map,   \
           items_count, num_items, sorted_indices) private(pow, i)             \
        num_threads(num_threads)
    for (pow = 1; pow < 2 * n_transactions; pow *= 2) {
        int start = pow == 1 ? 0 : pow / 2;
#pragma omp for
        for (i = start; i < n_transactions; i += pow) {
            if (pow > 1) {
                // dest - source

                // printf("Process %d Thread %d Merging trees %d and %d\n",
                // rank,
                //        omp_get_thread_num(), i - pow / 2, i);
                tree_merge(&trees[i - pow / 2], trees[i]);
                // printf("Before freeing\n");
                free_tree(&(trees[i]));
                // printf("DONE MERGING Thread %d Merging trees %d and %d\n",
                //        omp_get_thread_num(), i - pow / 2, i);
            } else {
                // if (i % 100000 == 0)
                //     printf("Process %d Thread %d Building tree %d\n", rank,
                //            omp_get_thread_num(), i);
                trees[i] = tree_build_from_transaction(
                    rank, world_size, &(transactions[i]), index_map,
                    items_count, num_items, sorted_indices);
                // printf("DONE BUILDING Thread %d Building tree %d size:
                // %lu\n",
                //        omp_get_thread_num(), i, cvector_size(trees[i]));
                // tree_print(trees[i]);
                // for (int j = 1; j < cvector_size(trees[i]); j++) {
                //     assert(trees[i][j]->parent != j);
                //     int x;
                //     assert(hashmap_get(trees[i][j]->adj, &(trees[i][j]->key),
                //                        sizeof(int), &x) == MAP_MISSING);
                // }
            }
        }
        // free_transactions(&transactions);
    }
    // tree_print(trees[0]);
    // free_tree(trees[0]);
    // for (int i = 0; i < n_transactions; i++)
    //     free_tree(&(trees[i]));
    Tree res = trees[0];
    free(trees);
    return res;
}

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
