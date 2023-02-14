#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <mpi.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <combined.h>

int main(int argc, char** argv)
{
  setvbuf(stdout, NULL, _IOLBF, 0);
  srand(time(0));
  int num_threads;
  int num_processes, mpi_num_iter = 7;

  MPI_Init(&argc, &argv);

  if (argc < 4){
    fprintf(stderr, "Usage: ./harness [NUM_THREADS]\n");
    exit(EXIT_FAILURE);
  }

  num_threads = strtol(argv[2], NULL, 10);
  num_processes = strtol(argv[1], NULL, 10);
  mpi_num_iter = strtol(argv[3], NULL, 10);
  long double* times = calloc(num_threads, sizeof(long double));

   int cur_rank = 0;
   MPI_Comm_rank(MPI_COMM_WORLD, &cur_rank);


   omp_set_dynamic(0);
   if (omp_get_dynamic()) {
    printf("Warning: dynamic adjustment of threads has been set\n");
   }

   omp_set_num_threads(num_threads);
   gtmp_init(num_threads, num_processes);

  #pragma omp parallel shared(num_threads)
  {
    for (int i = 0; i < mpi_num_iter; i++) {
        struct timeval start, end;
        // clock_t begin = clock();
        int cpu_num = sched_getcpu();
        printf("Thread num : %d   Cpu num : %d\n", omp_get_thread_num(), cpu_num);
        gettimeofday(&start, NULL);
        gtmp_barrier();
        gettimeofday(&end, NULL);
        long double time_req = (end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.    tv_usec);
        times[omp_get_thread_num()] += (time_req);
        // printf("Completed barrier for thread : %d for process %d\n", omp_get_thread_num(), cur_rank);
    }
  }

  long double avg_thread = 0;
    if (cur_rank == 0) {
        printf("Average time for threads across iterations for 0th processes ");
    }
  for (int i = 0; i < num_threads; i++) {
    times[i] /= mpi_num_iter;
    avg_thread += (times[i]);
    if (cur_rank == 0) {
        printf("%Lf ", times[i]);
    }
  }

  if (cur_rank == 0) {
    printf("\n");
  }

  avg_thread /= num_threads;

  long double* answer = calloc(num_processes, sizeof(long double)); 
  MPI_Gather(&avg_thread, 1, MPI_LONG_DOUBLE, answer, 1, MPI_LONG_DOUBLE, 0, MPI_COMM_WORLD);

  long double avg = 0;
  if (cur_rank == 0) {
    printf("Average across threads across iterations across processes ");
    for (int i = 0; i < num_processes; i++) {
        printf("%Lf ", answer[i]);
        avg += answer[i];
    }

    avg /= (num_processes);
    printf("\nAverage time for 1 barrier for 1 thread %Lf\n", avg);
  }
   
   gtmp_finalize();

   MPI_Finalize();

   return 0;
}
