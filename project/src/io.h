/**
 * @file io.h
 * @brief Functions that handle the input and output of the program
 *
 */
#ifndef IO_H
#define IO_H

#include "types.h"
#include <mpi.h>

/**
 * @brief Increase the support of the given item in the map.
 * If the item is not present it is inserted with support 1
 *
 * @param item Item of wich to increase the support
 * @param support_map A map from items to the respective support
 */
void update_supports(Item item, SupportMap *support_map);

/**
 * @brief Free the given transaction and all the items in it
 *
 * @param transaction The transaction to free
 */
void transaction_free(Transaction *transaction);

/**
 * @brief Free all the transaction in the list and the list itself
 *
 * @param transactions The list of transactions to free
 */
void transactions_free(TransactionsList *transactions);

/**
 * @brief Write a list of transactions to the file named as the rank
 * of the process
 *
 * @param rank Rank of the current process
 * @param transactions List of transactions to write
 */
void transactions_write(int rank, TransactionsList transactions);

/**
 * @brief Parse an item from the string chunk, starting from
 * position i up to the first space, newline or '\0'. The item
 * is added to the transaction and the corresponding support is
 * increased
 *
 * @param rank Rank of the current process
 * @param i Start position from where to start parsing
 * @param chunk String containing the item to parse
 * @param chunk_size Size of the chunk
 * @param transaction The transaction where to add the item
 * @param support_map The map from items to respective support
 * @return The index where the parsed item ends (excluded)
 */
int item_parse(int rank, int i, char *chunk, int chunk_size,
               Transaction *transaction, SupportMap *support_map);

/**
 * @brief Parse an transaction from the string chunk, starting from
 * position i up to the first newline or '\0'. The transaction is
 * added to the list of transactions. The support of the items in
 * the transaction is increased as they get read
 *
 * @param rank Rank of the current process
 * @param i Start position from where to start parsing
 * @param chunk String containing the item to parse
 * @param chunk_size Size of the chunk
 * @param transactions The list of transactions where to add the transaction
 * @param support_map The map from items to respective support
 * @return The index where the parsed transaction ends (excluded)
 */
int transaction_parse(int rank, int i, char *chunk, int chunk_size,
                      TransactionsList *transactions, SupportMap *support_map);

/**
 * @brief Read a chunk of the given file
 *
 * Read up to 2 * my_size, since we do not know where
 * transactions start exactly.
 *
 * @param filename File where transactions are stored
 * @param rank Rank of the current process
 * @param world_size Number of active processes
 * @param chunk String where to store the read bytes
 * @param my_size Number of bytes each process is assigned to
 * @param read_size Number of bytes the current process has read
 */
void read_chunk(char *filename, int rank, int world_size, char **chunk,
                int *my_size, int *read_size);

/**
 * @brief Read a list of transactions from the portion of
 * file assigned to the current process. The support of the
 * items read is increased in the support_map
 *
 *
 * @param transactions List of transactions where to store the data
 * @param filename Name of the file from which to read
 * @param rank Rank of the current process
 * @param world_size Number of active processes
 * @param support_map A map from items to the respective support
 */
void transactions_read(TransactionsList *transactions, char *filename, int rank,
                       int world_size, SupportMap *support_map);

#endif