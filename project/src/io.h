#ifndef IO_H
#define IO_H

#include "types.h"
#include <mpi.h>

void update_supports(Item item, SupportMap *support_map);

void transactions_free(TransactionsList *transactions);
void transaction_free(Transaction *transaction);
void transactions_write(int rank, TransactionsList transactions);
int item_parse(int rank, int i, char *chunk, int chunksize,
               Transaction *transaction, SupportMap *support_map);
int transaction_parse(int rank, int i, char *chunk, int my_size,
                      TransactionsList *transactions, SupportMap *support_map);
void read_chunk(char *filename, int rank, int world_size, char **chunk,
                int *my_size, int *read_size);
void transactions_read(TransactionsList *transactions, char *filename, int rank,
                       int world_size, SupportMap *support_map);

#endif