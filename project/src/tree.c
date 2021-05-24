#include "tree.h"
#include "sort.h"
#include <omp.h>
#include <stdio.h>
#include <string.h>

TreeNode *init_tree_node(int key, int value, int parent) {
    TreeNode *node = (TreeNode *)malloc(sizeof(TreeNode));
    node->key = key;
    node->value = value;
    node->parent = parent;
    node->adj = hashmap_new();
    return node;
}

void free_tree_node(TreeNode *node) {
    if (node != NULL) {
        hashmap_free(node->adj);
        free(node);
    }
}

Tree init_tree() {
    TreeNode *node = init_tree_node(TREE_NODE_NULL, -1, 0);
    Tree tree = NULL;
    cvector_push_back(tree, node);
    return tree;
}

void free_tree(Tree tree) {
    int n_nodes = cvector_size(tree);
    int i;
    for (i = 0; i < n_nodes; i++) {
        free_tree_node(tree[i]);
    }
    cvector_free(tree);
}

/**
 * Return the index of the added node in the tree
 */
int add_tree_node(Tree *tree, TreeNode *node) {
    cvector_push_back((*tree), node);
    int new_id = cvector_size((*tree)) - 1;
    TreeNode *parent = (*tree)[node->parent];
    hashmap_put(parent->adj, &(node->key), sizeof(int), new_id);
    assert(new_id != node->parent);
    return new_id;
}

void add_subtree(Tree *dest, Tree source, int nd, int ns) {
    // add myself
    printf("Add node nd: %d, ns: %d\n", nd, ns);
    cvector_vector_type(hashmap_element) neighbours = NULL;
    hashmap_get_elements(source[ns]->adj, &neighbours);
    printf("get elements %lu\n", cvector_size(neighbours));
    hashmap_free(source[ns]->adj);

    source[ns]->parent = nd;
    int new_pos = add_tree_node(dest, source[ns]);
    int num_adj_s = hashmap_length(source[ns]->adj);
    int i;

    printf("map free\n");
    // add children recursively
    for (i = 0; i < num_adj_s; i++) {
        printf("Adding child %d\n", i);
        assert(neighbours[i].value != ns);
        add_subtree(dest, source, new_pos, neighbours[i].value);
    }
    printf("Node ok\n");
    source[ns] = NULL;
    cvector_free(neighbours);
}

void merge_trees_dfs(Tree *dest, Tree source, int nd, int ns) {
    printf("Merging trees nd: %d, ns: %d ld: %lu, ls: %lu \n", nd, ns,
           cvector_size((*dest)), cvector_size(source));
    printf("Source has key %d\n", source[ns]->key);
    int i;
    assert(source[ns]->adj != NULL);
    int num_adj_s = hashmap_length(source[ns]->adj);
    // int num_adj_d = hashmap_length((*dest)[nd]->adj);
    cvector_vector_type(hashmap_element) neighbours = NULL;
    hashmap_get_elements(source[ns]->adj, &neighbours);
    for (i = 0; i < num_adj_s; i++) {
        printf("%dth neighbour\n", i);
        // int source_item = (int *)(neighbours[i].key);
        int source_pos = neighbours[i].value;
        assert(ns != neighbours[i].value);
        int dest_pos;
        if (hashmap_get((*dest)[nd]->adj, neighbours[i].key, sizeof(int),
                        &dest_pos) == MAP_OK) {
            printf("Present\n");
            int new_value = (*dest)[dest_pos]->value + source[ns]->value;
            assert(hashmap_put((*dest)[nd]->adj, neighbours[i].key, sizeof(int),
                               new_value) == MAP_OK);
            printf("Increase value\n");

            merge_trees_dfs(dest, source, dest_pos, source_pos);
            printf("recusive call ended\n");

        } else {
            printf("Not there, adding subtree\n");
            add_subtree(dest, source, nd, source_pos);
            printf("Subtree added\n");
        }
    }
    cvector_free(neighbours);
}

void merge_trees(Tree *dest, Tree source) {
    merge_trees_dfs(dest, source, 0, 0);
}

Tree build_OMP_tree(int rank, int world_size, Transaction transaction,
                    map_t index_map, hashmap_element *items_count,
                    int num_items, int *sorted_indices) {
    // printf("%d C.\n", rank);

    int n_items = cvector_size(transaction);
    hashmap_element *elements =
        (hashmap_element *)malloc(n_items * sizeof(hashmap_element));
    for (int i = 0; i < n_items; i++) {
        int item_size = cvector_size(transaction[i]);
        assert(hashmap_get(index_map, transaction[i], item_size,
                           &(elements[i].value)) == MAP_OK);
        elements[i].key_length = item_size;
        memcpy(elements[i].key, transaction[i], item_size);
    }
    // printf("%d D.\n", rank);

    int *transaction_sorted_indices = (int *)malloc(n_items * sizeof(int));
    sort(elements, n_items, transaction_sorted_indices, 0, n_items - 1, 1);
    // printf("%d E.\n", rank);
    Tree tree = init_tree();

    for (int i = 0; i < n_items; i++) {
        uint8_t *item = transaction[transaction_sorted_indices[i]];
        // printf("%d Ea.\n", rank);
        int item_size =
            cvector_size(transaction[transaction_sorted_indices[i]]);
        // printf("%d Eb.\n", rank);
        int pos;
        assert(hashmap_get(index_map, item, item_size, &pos) == MAP_OK);
        // printf("%d Ec pos: %d (out of %d)\n", rank, pos, num_items);
        int value = items_count[sorted_indices[pos]].value;

        TreeNode *node = init_tree_node(sorted_indices[pos], 1, i);

        assert(add_tree_node(&tree, node) == i + 1);

        // printf("%d Ed value %d.\n", rank, value);
        // printf("(%s : %d [%d]) ", item, pos, value);
    }

    // build tree

    free(transaction_sorted_indices);
    free(elements);
    // printf("%d F.\n", rank);
    return tree;
}

Tree build_MPI_tree(int rank, int world_size, TransactionsList transactions,
                    map_t index_map, hashmap_element *items_count,
                    int num_items, int *sorted_indices) {
    // Tree process_tree;
    // # omp parallel for
    // Tree trees[];
    // for i, transaction in transactions:
    //     trees[i] = build_tree(transaction)

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
    printf("%d B.\n", rank);

    for (pow = 1; pow <= 2; pow *= 2) {
        int start = pow == 1 ? 0 : pow / 2;
#pragma omp parallel for default(none)                                         \
    shared(start, pow, n_transactions, trees, rank, world_size, transactions,  \
           index_map, items_count, num_items, sorted_indices) private(i)       \
        num_threads(4)
        for (i = start; i < n_transactions; i += pow) {
            if (pow > 1) {
                // dest - source
                printf("Thread %d Merging trees %d and %d\n",
                       omp_get_thread_num(), i - pow / 2, i);
                merge_trees(&trees[i - pow / 2], trees[i]);
                printf("Before freeing\n");
                free_tree(trees[i]);
                printf("DONE MERGING Thread %d Merging trees %d and %d\n",
                       omp_get_thread_num(), i - pow / 2, i);
            } else {
                printf("Thread %d Building tree %d\n", omp_get_thread_num(), i);
                trees[i] =
                    build_OMP_tree(rank, world_size, transactions[i], index_map,
                                   items_count, num_items, sorted_indices);
                printf("DONE BUILDING Thread %d Building tree %d size: %lu\n",
                       omp_get_thread_num(), i, cvector_size(trees[i]));
                for (int j = 1; j < cvector_size(trees[i]); j++) {
                    assert(trees[i][j]->parent != j);
                    int x;
                    assert(hashmap_get(trees[i][j]->adj, &(trees[i][j]->key),
                                       sizeof(int), &x) == MAP_MISSING);
                }
            }
        }
    }

    free(trees);
    return NULL;
}