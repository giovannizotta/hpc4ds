#include <stdio.h>
#include <string.h>
#include <mpi.h>
#include <stdlib.h>
#include <unistd.h>
#define repeats 10
#define max_msg_size 25

void print_hostname(){
    int length = 100;
    char* hostname = malloc(length * sizeof(char));
    gethostname(hostname, length);
    printf("Hostname: %s\n", hostname);
    fflush(stdout);
}

void print_seconds(double* seconds, int size){
    int current_size = 1;
    for(int i=0; i<size; i++){
        printf("Seconds for size %10d: %lf, bandwidth: %.03lfbytes/s\n", current_size, seconds[i], current_size/seconds[i]);
        current_size *= 2;
    }
}

int main(int argc, char **argv){
    int my_rank, world_size;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);  
    print_hostname();
    if(my_rank == 0){ 
        double* seconds = malloc(max_msg_size * sizeof(double));
        int current_size = 1;
        for(int i=0; i<max_msg_size; i++){
            seconds[i] = 0;
            for(int times = 0; times < repeats; times++){
                char* message = malloc(current_size * sizeof(char));
                double start = MPI_Wtime();
                MPI_Send(message, current_size, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
                MPI_Recv(message, current_size, MPI_CHAR, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
                double finish = MPI_Wtime();
                seconds[i] += finish - start; 
                free(message);
            }
            current_size *= 2;
            seconds[i] /= repeats;
        }
        print_seconds(seconds, max_msg_size);
        free(seconds);
    } else {
        int current_size = 1;
        for(int i=0; i<max_msg_size; i++){
            for(int times = 0; times < repeats; times++){
                char* message = malloc(current_size * sizeof(char));
                MPI_Recv(message, current_size, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
                MPI_Send(message, current_size, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
                free(message);
            }
            current_size *= 2;
        }
    }
    MPI_Finalize();
    return 0; 
}
