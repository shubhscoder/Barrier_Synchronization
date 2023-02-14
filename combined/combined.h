#ifndef GTMPI_H
#define GTMPI_H

void gtmp_init(int num_threads, int num_processes);
void gtmp_barrier();
void gtmp_finalize();

#endif