#ifndef IO_H
#define IO_H

#include "types.h"
#include <mpi.h>

void update_supports(Item item, SupportMap *support_map);
void free_transactions(TransactionsList *transactions);
void write_transactions(int rank, TransactionsList transactions);
int parse_item(int rank, int i, char *chunk, int chunksize,
               Transaction *transaction, SupportMap *support_map);
int parse_transaction(int rank, int i, char *chunk, int my_size,
                      TransactionsList *transactions, SupportMap *support_map);
void read_chunk(char *filename, int rank, int world_size, char **chunk,
                int *my_size, int *read_size);
void read_transactions(TransactionsList *transactions, char *filename, int rank,
                       int world_size, SupportMap *support_map);
void write_keys(int rank, int start, int end, uint8_t **keys);
MPI_Datatype *define_MPI_SupportMap();
#endif