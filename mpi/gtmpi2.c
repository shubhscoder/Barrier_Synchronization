#include <stdlib.h>
#include <mpi.h>
#include <stdio.h>
#include "gtmpi.h"
#include <math.h>

static int num_of_process;
static int num_of_rounds = 0;

void gtmpi_init(int num_processes){
    num_of_process = num_processes;
    num_of_rounds = ceil(log2(num_of_process));
}

void gtmpi_barrier(){
    int process_id;
    MPI_Comm_rank(MPI_COMM_WORLD, &process_id);

    for(int i = 0; i < num_of_rounds; i++){
        //  Send message to (rank + 2^i % num_of_process)
        int destination_process_id = (process_id + (int)pow(2,i)) % num_of_process;

        MPI_Send(NULL, 0, MPI_INT, destination_process_id, 1, MPI_COMM_WORLD);

        // Wait for the message from ((rank - 2^i + num_of_process) % num_of_process)
        int source_process_id = (process_id - (int)pow(2,i) + num_of_process) % num_of_process;
        MPI_Recv(NULL, 0, MPI_INT, source_process_id, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    printf("Barrier completed for process %d\n", process_id);
}

void gtmpi_finalize() {}
