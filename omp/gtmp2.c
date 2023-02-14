#include <omp.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "gtmp.h"

int counter, sense;
int *local_sense;
int total_threads;

void gtmp_init(int num_threads) {
    total_threads = num_threads;
    counter = num_threads;
    sense = 1;
    local_sense = calloc(num_threads, sizeof(int));
    for (int i = 0; i < num_threads; i++) {
        local_sense[i] = 1;
    }
}

void gtmp_barrier() {
    int cur_id = omp_get_thread_num();
    local_sense[cur_id] = local_sense[cur_id] ^ 1;
    int add_val = 1;

    if (__sync_fetch_and_sub(&counter, add_val) == 1) {
        counter = total_threads;
        sense = local_sense[cur_id];
    } else {
        while (sense != local_sense[cur_id]);
    }
}

void gtmp_finalize() {
    free(local_sense);
}

