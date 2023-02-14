#include <stdlib.h>
#include <mpi.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "gtmpi.h"

#define CHAMPION 1

enum Roles {
    champion,
    winner,
    loser,
    dropout,
    bye
};

struct Info {
    // specifies the role of the current process for a given round.
    enum Roles role;

    // specifies the opponent of the current process for a given round.
    int opponent;
};

int communicators = 0;
int cur_rank = 0;
int num_rounds = 0;
struct Info* cur_info;

int getrounds(int p) {
    int cnt = 0, powr = 1;

    while (powr < p) {
        powr *= 2;
        cnt++;
    }

    return cnt + 1;
}

enum Roles getrole(int i, int num_processes) {
    int pow2 = (int)pow(2, i);
    int prevpow2 = (int)pow(2, i - 1);

    if (i == 0) {
        return dropout /* dropout */;
    } else if (cur_rank % pow2 == 0 && cur_rank + prevpow2 < num_processes && pow2 < num_processes) {
        return winner /* winner */;
    } else if (cur_rank % pow2 == prevpow2) {
        return loser /* loser */;
    } else if(cur_rank == 0 && pow2 >= num_processes) {
        return champion /* champion */;
    } else if (cur_rank % pow2 == 0 && cur_rank + prevpow2 >= num_processes) {
        return bye /* bye */;
    }

    return bye;
}

void gtmpi_init(int num_processes) {

    MPI_Comm_size(MPI_COMM_WORLD, &communicators);
    MPI_Comm_rank(MPI_COMM_WORLD, &cur_rank);
    assert(communicators == num_processes);
    // printf("Size : %d, Rank : %d", communicators, cur_rank);

    num_rounds = getrounds(num_processes);
    cur_info = calloc(num_rounds, sizeof(struct Info));

    for (int i = 0; i < num_rounds; i++) {
        int prevpow2 = (int)pow(2, i - 1);
        
        cur_info[i].role = getrole(i, num_processes);

        if (cur_info[i].role == champion || cur_info[i].role == winner) {
            cur_info[i].opponent = cur_rank + prevpow2;
        } else {
            // value will be used only for loser
            cur_info[i].opponent = cur_rank - prevpow2;
        }
    }

}

void gtmpi_barrier() {
    int round = 1;
    int continue_loop = 1;

    while (continue_loop)
    {
        switch (cur_info[round].role)
        {
        case loser: {
            /*
            Perform the following steps:
                1. Send message to the winner that cur_rank has arrived at the barrier.
                2. Keep spinning until winner noties back that, everyone has arrived at the barrier.
                3. recv in case of MPI will be blocking, so it is equivalent to spinning.
                4. Once message is received, cur_rank can break the loop, and notify children in the next loop if any. 
            */

            MPI_Send(NULL, 0, MPI_INT, cur_info[round].opponent, 0, MPI_COMM_WORLD);

            MPI_Recv(NULL, 0, MPI_INT, cur_info[round].opponent, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            continue_loop = 0;
            break;
        }
        case winner: {
            // The winner just needs to wait for a message from the loser.
            MPI_Recv(NULL, 0, MPI_INT, cur_info[round].opponent, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // Now, we don't need to break the loop here. This process will go to the next round of competition.
            break;
        }
        case bye: {
            break;
        }
        case champion: {
            /*
                Perform the following steps:
                    1. Wait for loser to reach the barrier.
                    2. Send message to the loser that the barrier is complete.
                    3. Once the message is send, break out of the loop.
            */
            MPI_Recv(NULL, 0, MPI_INT, cur_info[round].opponent, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            MPI_Send(NULL, 0, MPI_INT, cur_info[round].opponent, 0, MPI_COMM_WORLD);

            continue_loop = 0;
            break;
        }

        case dropout: {
            assert(0);
            break;
        }

        default:
            break;
        }

        round++;       
    }

    continue_loop = 1;

    while (continue_loop) {
        round = round - 1;
        switch (cur_info[round].role)
        {
        case loser: {
            break;
        }
        case winner: {
            MPI_Send(NULL, 0, MPI_INT, cur_info[round].opponent, 0, MPI_COMM_WORLD);
            break;
        }

        case bye: {
            break;
        }
        case champion: {
            break;
        }
        case dropout: {
            continue_loop = 0;
            break;
        }
        default:
            break;
        }

    }   
}

void gtmpi_finalize() {
    free(cur_info);
}
