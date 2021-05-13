#include <stdio.h>
#include <string.h>
#include <mpi.h>
#include "../cvector/cvector.h"

double f(double x){
    return (x - 3) * (x - 4) * (x - 5) * (x - 6) * (x - 7) + 4;
}

void read_file(int argc, char **argv, int rank, int world_size){
    MPI_File in;
    int ierr;
    cvector_vector_type(char**) transactions = NULL;

    const int overlap = 100;
    char a[101];

    if (argc != 2) {
        if (rank == 0) fprintf(stderr, "Usage: %s infilename\n", argv[0]);
        MPI_Finalize();
        exit(1);
    }

    ierr = MPI_File_open(MPI_COMM_WORLD, argv[1], MPI_MODE_RDONLY, MPI_INFO_NULL, &in);
    if (ierr) {
        if (rank == 0) fprintf(stderr, "%s: Couldn't open file %s\n", argv[0], argv[1]);
        MPI_Finalize();
        exit(2);
    }
    MPI_Offset start;
    MPI_Offset end;
    MPI_Offset filesize;

    MPI_File_get_size(in, &filesize);
    filesize--;
    int mysize = (filesize / world_size) + 1;
    start = max(0, rank * mysize - 1);
    end = min(filesize, start + (2 * mysize) - 1);
    char* chunk = malloc( (2 * mysize + 1) * sizeof(char));
    
    MPI_File_read_at(in, start, chunk, mysize, MPI_CHAR, MPI_STATUS_IGNORE);
    chunk[mysize] = '\0';
    
    int i = 0;
    while(chunk[i] != '\n'){
        i++;
    }
    i++;
    cvector_vector_type(char*) transaction = NULL;
    cvector_vector_type(char) item = NULL;
    while(i < mysize){
        while(i < mysize && chunk[i] != ' ' && chunk[i] != '\n'){
            cvector_push_back(item, chunk[i]);
            i++;
        }
        if(i < mysize){
            cvector_push_back(item, '\0');
            cvector_push_back(transaction, item);
            item = NULL;
            if(chunk[i] == '\n'){
                cvector_push_back(transactions, transaction);
                transaction = NULL;
            } 
            i++;
        }
    } 

    while(chunk[i] != '\n'){
        while(chunk[i] != ' ' && chunk[i] != '\n'){
            cvector_push_back(item, chunk[i]);
            i++;
        }
        cvector_push_back(item, '\0');
        cvector_push_back(transaction, item);
        item = NULL;
        if(chunk[i] == '\n'){
            cvector_push_back(transactions, transaction);
            transaction = NULL;
        } 
        i++;
    }

    free(chunk);        
}

int main(int argc, char **argv){
    int my_rank, world_size;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); 


    double a, b;
    sscanf(argv[1], "%lf", &a);
    sscanf(argv[2], "%lf", &b);
    double interval_width = (double)(b-a)/(double)(world_size-1);
    if(my_rank != 0){
        double my_a = a + (interval_width * (my_rank - 1));
        double my_b = a + (interval_width * my_rank);
        double area = (interval_width / 2) * (f(my_a) + f(my_b));
        double message[3] = {area, my_a, my_b};
        MPI_Send(message, 3, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    } else {
        printf("I'm listening for areas\n");
        double result = 0;
        for(int i = 1; i < world_size; i++){
            double area[3];
            MPI_Recv(area, 3, MPI_DOUBLE, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            printf("Received a trapezoid of size %lf from %lf to %lf\n", area[0], area[1], area[2]);
            result += area[0];
        }
        printf("The final result is %lf\n", result);
    }

    MPI_Finalize();
    return 0;
    
}
