#include "mem.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* 
 * This program is a testbench
 * When it is run, it should execute a large number of calls to the getmem and freemem functions
 * to allocate and free blocks of random sizes and in random order
 * This program should allow users to specify parameters to control the test.
 */

// bench [ntrials] [pctget] [pctlarge] [small_limit] [large_limit] [random_seed]
// ntrials, total number of getmem and freemem calls to perform, default 10000
// pctget: percent of total calls that should be getmems. Default 50%
// pctlarge: percent of getmem calls for large blocks. Default 10
// small_limit: largest size of a small block. Default 200.
// large_limit: largest size of a large block. Default 20000.
// random_seed: initial seed value of the random number generator.
//              Default: some random number (/dev/urandom)


// Helper method that prints information about the heap and execution time to stdout
void print_info(time_t * start, time_t * end, uintptr_t * total_size, uintptr_t * total_free, uintptr_t * n_free_blocks);

// Helper method that initializes up to the first 16 bytes of a memory block to the value 0xFE
void init_bytes(void * block_start, int n_bytes);

int main(int argc, char ** argv) {
    int ntrials = 10000;
    int pctget = 50;
    int pctlarge = 10;
    int small_limit = 200;
    int large_limit = 20000;
    unsigned int random_seed = 9847353;
    int num_blocks = 0;
    switch (argc) {
        case 7:
            random_seed = atoi(argv[6]);
        case 6:
            large_limit = atoi(argv[5]);
        case 5:
            small_limit = atoi(argv[4]);
        case 4:
            pctlarge = atoi(argv[3]);
        case 3:
            pctget = atoi(argv[2]);
        case 2:
            ntrials = atoi(argv[1]);
        case 1:
            break;
        default:
            printf("Usage %s [ntrials] [pctget] [pctlarge] [small_limit] [large_limit] [random_seed]\n", argv[0]);
            return 1;
    }
    if (ntrials < 10) {
        printf("ntrials must be greater than or equal to 10, %d\n", ntrials);
        return 1;
    } 
    srand(random_seed);
    void * blocks[ntrials];
    clock_t start, end;
    const int n_10 = ntrials / 10;
    //printf("%d\n", n_10);
    uintptr_t * total_size = (uintptr_t *) malloc(sizeof(uintptr_t));
    uintptr_t * total_free = (uintptr_t *) malloc(sizeof(uintptr_t));
    uintptr_t * n_free_blocks = (uintptr_t *) malloc(sizeof(uintptr_t));
    uintptr_t num;
    start = clock();
    for (int i = 1; i <= ntrials; i++){
        //printf("i: %d\n", i);
        num = rand()%100;
        if (num <= pctget) {
            num = rand()%100;
            if (num <= pctlarge) {
                num  = rand()%(large_limit - small_limit + 1) + small_limit;
            } else {
                num = rand()%(small_limit + 1);
            }
            blocks[num_blocks++] = getmem(num);
            //printf("blocks[%d] = %p\n", num_blocks-1, blocks[num_blocks-1]);
            init_bytes(blocks[num_blocks-1], num);
        } else {
            if (num_blocks > 0) {
                num = rand()%num_blocks;
                //printf("freeing blocks[%ld]\n", num);
                freemem(blocks[num]);
                blocks[num] = blocks[--num_blocks];
            } else {
                i--;
                continue;
            }
        }       
        if (i % n_10 == 0) {
            print_info(&start, &end, total_size, total_free, n_free_blocks);
        }
    }
    return 0;
}

void print_info(clock_t * start, clock_t * end, uintptr_t * total_size, uintptr_t * total_free, uintptr_t * n_free_blocks) {
    *end = clock();
    double time_elapsed = (*end - *start) / (double) CLOCKS_PER_SEC;
    printf("Time elapsed is %f seconds.\n", time_elapsed);
    get_mem_stats(total_size, total_free, n_free_blocks);
    printf("Total size of memory in the heap: %ld\n", *total_size);
    printf("Total size of free memory: %ld\n", *total_free);
    printf("Total number of free blocks: %ld\n", *n_free_blocks);
    printf("Average size of a free block: %.4f\n", ((float) *total_free / (float) *n_free_blocks));
}

void init_bytes(void * block_start, int n_bytes) {
    n_bytes = (n_bytes > 16) ? 16 : n_bytes;
    uintptr_t * block = (uintptr_t *) block_start;
    for (int i = 0; i <  n_bytes; i++) {
        *(uintptr_t *)((uintptr_t) block + i) = 0xFE;
    }
}
