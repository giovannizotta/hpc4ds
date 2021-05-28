#include "io.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>

/**
 * @brief Increase the support of the given item in the map.
 * If the item is not present it is inserted with support 1
 *
 * @param item Item of wich to increase the support
 * @param support_map A map from items to the respective support
 */
void update_supports(Item item, SupportMap *support_map) {
    size_t size = cvector_size(item);
    if (hashmap_increment(*support_map, item, size, 1) == MAP_KEY_TOO_LONG) {
        MPI_Finalize();
        exit(1);
    }
}

/**
 * @brief Free the fiven transaction and all the items in it
 *
 * @param transaction The transaction to free
 */
void transaction_free(Transaction *transaction) {
    if (*transaction != NULL) {
        size_t n_items = cvector_size((*transaction));
        int i;
        for (i = 0; i < n_items; i++) {
            cvector_free((*transaction)[i]);
        }
        cvector_free((*transaction));
        *transaction = NULL;
    }
}

/**
 * @brief Free all the transaction in the list and the list itself
 *
 * @param transactions The list of transactions to free
 */
void transactions_free(TransactionsList *transactions) {
    if (*transactions != NULL) {
        size_t n_transactions = cvector_size((*transactions));
        size_t i, j;
        for (i = 0; i < n_transactions; i++) {
            if ((*transactions)[i] != NULL) {
                size_t n_items = cvector_size((*transactions)[i]);
                for (j = 0; j < n_items; j++) {
                    cvector_free((*transactions)[i][j]);
                }
                cvector_free((*transactions)[i]);
            }
        }
        cvector_free((*transactions));
        *transactions = NULL;
    }
}

/**
 * @brief Write a list of transactions to the file named as the rank
 * of the process
 *
 * @param rank Rank of the current process
 * @param transactions List of transactions to write
 */
void transactions_write(int rank, TransactionsList transactions) {
    char filename[10];
    MPI_File out;
    sprintf(filename, "%d.txt", rank);
    // printf("%d writing output to %s\n", rank, filename);
    int ierr =
        MPI_File_open(MPI_COMM_WORLD, filename,
                      MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &out);
    if (ierr) {
        printf("Error while writing!\n");
        MPI_Finalize();
        exit(1);
    }
    size_t n_transactions = cvector_size(transactions);
    // printf("%d Writing %lu transactions\n", rank, n_transactions);
    size_t i, j;
    char space[2] = " ";
    char newline[2] = "\n";
    for (i = 0; i < n_transactions; i++) {
        size_t n_items = cvector_size((transactions[i]));

        for (j = 0; j < n_items; j++) {
            size_t item_size = cvector_size((transactions[i][j])) - 1;
            MPI_File_write(out, transactions[i][j], item_size, MPI_CHAR,
                           MPI_STATUS_IGNORE);
            if (j < n_items - 1)
                MPI_File_write(out, space, 1, MPI_CHAR, MPI_STATUS_IGNORE);
        }
        MPI_File_write(out, newline, 1, MPI_CHAR, MPI_STATUS_IGNORE);
    }
    MPI_File_close(&out);
}

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
               Transaction *transaction, SupportMap *support_map) {
    // see if actually there is an item
    while (chunk[i] == ' ') {
        i++;
    }
    if (chunk[i] == '\0' || chunk[i] == '\n') {
        return i;
    }

    // read the item
    Item item = NULL;
    while (chunk[i] != ' ' && chunk[i] != '\n' && chunk[i] != '\0') {
        cvector_push_back(item, chunk[i]);
        i++;
    }
    cvector_push_back(item, '\0');
    // push it into the current transaction
    cvector_push_back((*transaction), item);
    update_supports(item, support_map);
    return i;
}

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
                      TransactionsList *transactions, SupportMap *support_map) {
    while (chunk[i] == '\n') {
        i++;
    }
    if (chunk[i] == '\0') {
        return i;
    }
    Transaction transaction = NULL;

    while (chunk[i] != '\n' && chunk[i] != '\0') {
        i = item_parse(rank, i, chunk, chunk_size, &transaction, support_map);
    }
    cvector_push_back((*transactions), transaction);

    return i;
}

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
                int *my_size, int *read_size) {
    MPI_File in;
    int ierr;
    ierr = MPI_File_open(MPI_COMM_WORLD, filename, MPI_MODE_RDONLY,
                         MPI_INFO_NULL, &in);
    if (ierr) {
        if (rank == 0)
            fprintf(stderr, "Process %d: Couldn't open file %s\n", rank,
                    filename);
        MPI_Finalize();
        exit(2);
    }

    MPI_Offset start;
    MPI_Offset filesize;
    MPI_File_get_size(in, &filesize);

    //------ READ CHUNK -------
    *my_size = (filesize - 1) / world_size + 1;
    start = max(0, rank * (*my_size) - 1);
    *read_size = 2 * (*my_size);
    if (start + *read_size >= filesize) {
        *read_size = filesize - start;
        (*my_size) = min((*my_size), *read_size);
    }
    *chunk = malloc((*read_size + 1) * sizeof(char));
    MPI_File_read_at(in, start, *chunk, *read_size, MPI_CHAR,
                     MPI_STATUS_IGNORE);

    (*chunk)[*read_size] = '\0';

    MPI_File_close(&in);
}

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
                       int world_size, SupportMap *support_map) {
    char *chunk;
    int my_size, read_size;
    read_chunk(filename, rank, world_size, &chunk, &my_size, &read_size);

    //------ READ TRANSACTIONS ----------
    int i = 0;
    // skip first incomplete transaction
    if (rank > 0) {
        while (chunk[i] != '\n') {
            i++;
        }
    }
    // read transactions starting before the ending of the bytes assigned
    // to the current process
    while (i < my_size) {
        i = transaction_parse(rank, i, chunk, read_size, transactions,
                              support_map);
        assert(cvector_size((*transactions)) > 0);
    }
    free(chunk);
}