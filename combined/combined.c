#include <omp.h>
#include <mpi.h>
#include<malloc.h>
#include<stdbool.h>
#include "mpi_tournament.h"
#include "combined.h"


#define ARE_ALL_CHILDREN_DONE(pvid) nodes[pvid].childNotReady[0] || nodes[pvid].childNotReady[1] || nodes[pvid].childNotReady[2] || nodes[pvid].childNotReady[3]
#define IS_PARENT_DONE(pvid) nodes[pvid].parentSense != nodes[pvid].sense


typedef struct treeNode{
    bool sense;

    bool parentSense;

    bool* parentPointer;

    bool* childPointers[2];

    bool haveChild[4];

    bool childNotReady[4];

    int vpid;
    
    bool dummy;
} treeNode, *treeNodePtr;

static treeNodePtr nodes;
static int vp_num;

void initialiseChild(treeNodePtr node){
    for (int i = 0; i < 4; i++){
        node->childNotReady[i] = node->haveChild[i];
    }
}

void gtmp_init(int num_threads, int num_processes) {
    gtmpi_init(num_processes);
    nodes = malloc(num_threads * sizeof(treeNode));
    vp_num = num_threads;

    for(int i = 0; i < num_threads; i++){
        nodes[i].vpid = i;
        nodes[i].dummy = false;
        nodes[i].sense = true;
        nodes[i].parentSense = false;

        for(int j = 0; j < 4; j++){
            nodes[i].haveChild[j] = (4*i + j + 1 < vp_num) ? true : false;
        }

        nodes[i].parentPointer = (i == 0) ? &nodes[i].dummy : &nodes[(i-1)/4].childNotReady[(i-1)%4];
        nodes[i].childPointers[0] = (2*i + 1 >= vp_num) ? &nodes[i].dummy : &nodes[2*i + 1].parentSense;
        nodes[i].childPointers[1] = (2*i + 2 >= vp_num) ? &nodes[i].dummy : &nodes[2*i + 2].parentSense;

        initialiseChild(&nodes[i]);
    }
}


void gtmp_barrier(){
    int vpid = omp_get_thread_num();

    while(ARE_ALL_CHILDREN_DONE(vpid));

    initialiseChild(&nodes[vpid]);

    *nodes[vpid].parentPointer = false;

    if (nodes[vpid].vpid != 0) {
        while(IS_PARENT_DONE(vpid));
    } else {
        gtmpi_barrier();
    }

    *nodes[vpid].childPointers[0] = nodes[vpid].sense;
    *nodes[vpid].childPointers[1] = nodes[vpid].sense; 
    nodes[vpid].sense = !(nodes[vpid].sense);
}

void gtmp_finalize(){
    free(nodes);
}

