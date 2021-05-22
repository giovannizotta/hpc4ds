#include "io.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>

// MPI_Datatype *define_MPI_HashMap_Element() {
//     // #define INITIAL_SIZE 256
//     // #define LINEAR_PROBE_LENGTH 8
//     // #define KEY_STATIC_LENGTH 32
//     //
//     // /* We need to keep keys and values */
//     // typedef struct _hashmap_element{
//     //     char key_static[KEY_STATIC_LENGTH];
//     //     char *key_dynamic;
//     //     int key_length;

//     //     int in_use;
//     //     any_t value; // int *
//     // } hashmap_element;

//     // MPI_Datatype MPI_HASHMAP_ELEMENT, MPI_HASHMAP;

//     // MPI_Aint displacements[5];
//     // hashmap_element dummy_hashmap_element;
//     // MPI_Aint base_address;
//     // MPI_Get_address(&dummy_hashmap_element, &base_address);
//     // MPI_Get_address(dummy_hashmap_element.key_static, &displacements[0]);
//     // MPI_Get_address(dummy_hashmap_element.key_dynamic, &displacements[1]);
//     // MPI_Get_address(&dummy_hashmap_element.key_length, &displacements[2]);
//     // MPI_Get_address(&dummy_hashmap_element.in_use, &displacements[3]);
//     // MPI_Get_address(dummy_hashmap_element.value, &displacements[4]);
//     // displacements[0] = MPI_Aint_diff(displacements[0], base_address);
//     // displacements[1] = MPI_Aint_diff(displacements[1], base_address);
//     // displacements[2] = MPI_Aint_diff(displacements[2], base_address);
//     // displacements[3] = MPI_Aint_diff(displacements[3], base_address);
//     // displacements[4] = MPI_Aint_diff(displacements[4], base_address);
//     // MPI_Datatype types[5] = {MPI_CHAR, MPI_CHAR, MPI_INT, MPI_INT, };

// }

// MPI_Datatype *define_MPI_SupportMap(){
//     // MPI_Datatype *MPI_HASHMAP_ELEMENT define_MPI_HashMap_Element)=;

//     // /* A hashmap has some maximum size and current size,
//     // * as well as the data to hold. */
//     // typedef struct _hashmap_map{
//     //     int table_size;
//     //     int size;
//     //     hashmap_element *data;
//     // } hashmap_map;
// }

void free_transactions(TransactionsList *transactions) {
    size_t n_transactions = cvector_size((*transactions));
    size_t i, j;
    for (i = 0; i < n_transactions; i++) {
        size_t n_items = cvector_size((*transactions)[i]);
        for (j = 0; j < n_items; j++) {
            cvector_free((*transactions)[i][j]);
        }
        cvector_free((*transactions)[i]);
    }
    cvector_free((*transactions));
    *transactions = NULL;
}

void write_keys(int rank, int start, int end, uint8_t **keys) {
    char filename[10];
    MPI_File out;
    sprintf(filename, "%d.txt", rank);
    printf("%d writing output to %s\n", rank, filename);
    int ierr =
        MPI_File_open(MPI_COMM_WORLD, filename,
                      MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &out);
    if (ierr) {
        printf("Error while writing!\n");
        MPI_Finalize();
        exit(1);
    }
    size_t n_keys = cvector_size(keys);
    printf("%d Writing %lu keys (from %d to %d)\n", rank, n_keys, start, end);
    size_t i;
    char space[2] = " ";
    char newline[2] = "\n";
    for (i = 0; i < n_keys; i++) {
        int key_size = strlen(keys[i]);
        MPI_File_write(out, keys[i], key_size, MPI_CHAR, MPI_STATUS_IGNORE);
        if (i < n_keys - 1)
            MPI_File_write(out, space, 1, MPI_CHAR, MPI_STATUS_IGNORE);
    }
    MPI_File_write(out, newline, 1, MPI_CHAR, MPI_STATUS_IGNORE);
    MPI_File_close(&out);
}

void write_transactions(int rank, TransactionsList transactions) {
    char filename[10];
    MPI_File out;
    sprintf(filename, "%d.txt", rank);
    printf("%d writing output to %s\n", rank, filename);
    int ierr =
        MPI_File_open(MPI_COMM_WORLD, filename,
                      MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &out);
    if (ierr) {
        printf("Error while writing!\n");
        MPI_Finalize();
        exit(1);
    }
    size_t n_transactions = cvector_size(transactions);
    printf("%d Writing %lu transactions\n", rank, n_transactions);
    size_t i, j;
    char space[2] = " ";
    char newline[2] = "\n";
    for (i = 0; i < n_transactions; i++) {
        size_t n_items = cvector_size((transactions[i]));
        // printf("\t%d Writing transaction %d, having %d elements\n", rank, i,
        // n_items);

        for (j = 0; j < n_items; j++) {
            size_t item_size = cvector_size((transactions[i][j])) - 1;
            // printf("\t\t%d Writing item %d of transaction %d, long %d
            // chars\n", rank, j, i, item_size);
            MPI_File_write(out, transactions[i][j], item_size, MPI_CHAR,
                           MPI_STATUS_IGNORE);
            if (j < n_items - 1)
                MPI_File_write(out, space, 1, MPI_CHAR, MPI_STATUS_IGNORE);
        }
        MPI_File_write(out, newline, 1, MPI_CHAR, MPI_STATUS_IGNORE);
    }
    MPI_File_close(&out);
}

void update_supports(Item item, SupportMap *support_map) {
    // item_count *s, *tmp = NULL;

    // HASH_FIND_STR((support_map), item, tmp);
    // if (tmp == NULL) {
    //     s = (item_count *)malloc(sizeof *s);
    //     s->item = item;
    //     s->count = 1;
    //     HASH_ADD_KEYPTR(hh, (support_map), s->item, strlen(s->item), s);
    // }else{
    //     tmp->count++;
    // }
    size_t size = cvector_size(item);
    if (hashmap_increment(*support_map, item, size, 1) == MAP_KEY_TOO_LONG) {
        MPI_Finalize();
        exit(1);
    }
}

int parse_item(int rank, int i, char *chunk, int chunksize,
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

int parse_transaction(int rank, int i, char *chunk, int my_size,
                      TransactionsList *transactions, SupportMap *support_map) {
    // printf("read transaction from pos %d\n", i);
    while (chunk[i] == '\n') {
        i++;
    }
    if (chunk[i] == '\0') {
        return i;
    }
    Transaction transaction = NULL;

    while (chunk[i] != '\n' && chunk[i] != '\0') {
        // assert(chunk[i] != '\0');
        i = parse_item(rank, i, chunk, my_size, &transaction, support_map);
        // assert(chunk[i] == '\n' || chunk[i] == ' ');
    }
    cvector_push_back((*transactions), transaction);

    return i;
}

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
    *my_size = filesize / world_size + 1;
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

void read_transactions(TransactionsList *transactions, char *filename, int rank,
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
    while (i < my_size) {
        i = parse_transaction(rank, i, chunk, read_size, transactions,
                              support_map);
        assert(cvector_size((*transactions)) > 0);
    }
    // write_file(rank, transactions);
    free(chunk);
    size_t n_transactions = cvector_size((*transactions));
    printf("%d Read %lu transactions\n", rank, n_transactions);
}