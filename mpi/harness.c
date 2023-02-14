#include <stdio.h>
#include <stdlib.h>
#include<unistd.h>
#include <sys/time.h>
#include <mpi.h>
#include <time.h>
#include "gtmpi.h"

int main(int argc, char** argv)
{
  srand(time(0));
  int num_processes, num_rounds = 10000;

  MPI_Init(&argc, &argv);
  int cur_rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &cur_rank);

  
  if (argc < 2){
    fprintf(stderr, "Usage: ./harness [NUM_PROCS]\n");
    exit(EXIT_FAILURE);
  }

  num_processes = strtol(argv[1], NULL, 10);

  gtmpi_init(num_processes);

  long double avg_time = 0;
  
  int k;
  for(k = 0; k < num_rounds; k++){
    // introduce a random delay in every process before reaching barrier. This simulates work.
    // int rtime = rand() % 10;
    // sleep(rtime);
    // clock_t begin = clock();
    struct timeval start, end;
      // clock_t begin = clock();
    gettimeofday(&start, NULL);
    gtmpi_barrier();
    // clock_t end = clock();
    // long double time_req = (long double) (end - begin);
    // time_req = time_req / CLOCKS_PER_SEC;
      gettimeofday(&end, NULL);
      // clock_t end = clock();
      long double time_req = (end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.    tv_usec);;
      avg_time += time_req;
      // printf("%Lf\n", time_req);  
    }

  avg_time /= (num_rounds);

  long double* times = calloc(num_processes, sizeof(long double));
  MPI_Gather(&avg_time, 1, MPI_LONG_DOUBLE, times, 1, MPI_LONG_DOUBLE, 0, MPI_COMM_WORLD);

  if (cur_rank == 0) {
    long double global_avg = 0;
    for (int i = 0; i < num_processes; i++) {
      printf("%Lf ", times[i]);
      global_avg += (times[i] / num_processes);
    }
    printf("\n");
    printf("Average time %Lf\n", global_avg);
  }

  gtmpi_finalize();  

  MPI_Finalize();

  return 0;
}
