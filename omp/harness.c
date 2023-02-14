#include <stdio.h>
#include <stdlib.h>
#include<unistd.h>
#include <sys/time.h>
#include <omp.h>
#include <time.h>
#include "gtmp.h"

int main(int argc, char** argv)
{
  srand(time(0));
  int num_threads, num_iter=100000;

  if (argc < 2){
    fprintf(stderr, "Usage: ./harness [NUM_THREADS]\n");
    exit(EXIT_FAILURE);
  }
  num_threads = strtol(argv[1], NULL, 10);

  omp_set_dynamic(0);
  if (omp_get_dynamic())
    printf("Warning: dynamic adjustment of threads has been set\n");

  omp_set_num_threads(num_threads);
  
  gtmp_init(num_threads);

  long double* avg_time = calloc(num_threads, sizeof(long double));

#pragma omp parallel shared(num_threads)
   {
     int i;
     for(i = 0; i < num_iter; i++){
      //  int rtime = rand() % 10;
      //  sleep(rtime);
      struct timeval start, end;
      // clock_t begin = clock();
      gettimeofday(&start, NULL);
      gtmp_barrier();
      gettimeofday(&end, NULL);
      // clock_t end = clock();
      long double time_req = (end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.    tv_usec);
      // time_req = time_req / CLOCKS_PER_SEC;
      avg_time[omp_get_thread_num()] += (time_req);
     }
   }
   gtmp_finalize();

  long double global_avg = 0;
  for (int i = 0; i < num_threads; i++) {
    avg_time[i] /= (num_iter);
    printf("%Lf ", avg_time[i]);
    global_avg += avg_time[i];
  }

  printf("\n");

  global_avg /= num_threads;

  printf("Average : %Lf\n", global_avg);
   return 0;
}
