#include <stdio.h>
#include <string.h>
#include <mpi.h>
#include <stdlib.h>
#include <unistd.h>
#define repeats 1000

void print_hostname(){
    int length = 100;
    char* hostname = malloc(length * sizeof(char));
    gethostname(hostname, length);
    printf("Hostname: %s\n", hostname);
    fflush(stdout);
}

void print_seconds(double* seconds, int times){
    int length = 1024;
    for(int i=0; i<times; i++){
        printf("Seconds for size %10d: %lf, bandwidth: %.03lfbytes/s\n", length, seconds[i], length/seconds[i]);
    }
}

int main(int argc, char **argv){
    int my_rank, world_size;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);  
    print_hostname();
    int length = 1024;
    if(my_rank == 0){ 
        double* seconds = malloc(repeats * sizeof(double));
        for(int times = 0; times < repeats; times++){
            char* message = malloc(length * sizeof(char));
            double start = MPI_Wtime();
            MPI_Send(message, length, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
            MPI_Recv(message, length, MPI_CHAR, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
            double finish = MPI_Wtime();
            seconds[times] += finish - start; 
            free(message);
        }
        print_seconds(seconds, repeats);
        free(seconds);
    } else {
        int length = 1024;
        for(int times = 0; times < repeats; times++){
            char* message = malloc(length * sizeof(char));
            MPI_Recv(message, length, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
            MPI_Send(message, length, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
            free(message);
        }
    }
    MPI_Finalize();
    return 0; 
}
