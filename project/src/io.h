#ifndef IO_H
#define IO_H

#include "types.h"

void free_transactions(TransactionsList* transactions);
void write_file(int rank, TransactionsList transactions);
int parse_item(int rank, int i, char* chunk, int chunksize, Transaction* transaction);
int parse_transaction(int rank, int i, char* chunk, int my_size, TransactionsList* transactions);
void read_chunk(char* filename, int rank, int world_size, char** chunk, int* my_size, int* read_size);
void read_transactions(TransactionsList* transactions, char* filename, int rank, int world_size);

#endif