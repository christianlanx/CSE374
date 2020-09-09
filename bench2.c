#include "mem.h"

#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
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

int main(int argc, char ** argv) {
    int ntrials = 10000;
    int pctget = 50;
    int pctlarge = 10;
    int small_limit = 200;
    int large_limit = 20000;
    unsigned int random_seed = 9847353;
    void * blocks[ntrials]; 
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
            return 0;
    }
    
    srand(random_seed);
    struct timeval start, end;
    gettimeofday(&start, NULL);    
    int n_10 = ntrials / 10;
    long micros, seconds;
    uintptr_t * total_size, * total_free, * n_free_blocks;
    uintptr_t num;
    for (int i = 0; i < ntrials; i++){
        num = rand()%100;
        if (num <= pctget) {
            num = rand()%100;
            if (num <= pctlarge) {
                num  = rand()%(large_limit - small_limit + 1) + small_limit;
            } else {
                num = rand()%(small_limit + 1);
            }
            blocks[num_blocks++] = getmem(num);
        } else {
            if (num_blocks > 0) {
                num = rand()%num_blocks;
                freemem(blocks[num]);
                blocks[num] = blocks[--num_blocks];
            } else {
                i--;
            }
        }       
        if (i % n_10 == 0) {
            gettimeofday(&end, NULL);
            micros = ((end.tv_sec - start.tv_sec) * 1000000) + end.tv_usec - start.tv_usec;
            seconds = micros / 1000000;
            micros = micros - seconds;
            printf("Time elapsed is %ld.%ld seconds\n", seconds, micros);
            get_mem_stats(total_size, total_free, n_free_blocks);
            printf("Total size of memory in the heap: %ld\n", *total_size);
            printf("Total size of free memory: %ld\n", *total_free);
            printf("Total number of free blocks: %ld\n", *n_free_blocks);
            printf("Average size of a free block: %ld\n", (*total_free / *n_free_blocks));
        }
    }

    return 0;
}


/*Jordano Comments 5/27/2020 12:40am...
 1) run a for loop for ntrails... for(int i = 1; i <= ntrails; i++)... keep in mind the case where we may say i-- !!! (could also do a while loop. either works).
    a) every time the loop completes another 10% of its ntrails we print to stdout...
        1: CPU time used by benchmark test so far (provide fractional digits as necessary (floats??)
                use gettimeofday and the print method as shown
        
        2: printf("total number of free storage = %d \n", FREE_SIZE);
        3: printf("total number of free blocks = %d \n", NUM_FREE);
	4: printf("Average free storage per free block = %d/%d \n", FREE_SIZE, NUM_FREE);
    b) Start a tic tok timer sequence before entering the loop so we can keep track of total time it is taking to compute the testbench
    c) set up a random genorator that picks between option A: getmem... or option B: freemem... here are some cases
        i) if freemem was called, and there is no more blocks to free, then "p" is NULL... so freemem does nothing.
	   THIS IS THE CASE WHERE: i-- to make sure this doesnt count towards ntrails***
	ii) if getmem is called, and pointer returned is not NULL, store 0xFE in each of the first 16 bytes STARTING AT pointer returned by getmem
	iii) if freemem is called, we need to used RANDOM to get a random returned pointer from getmem to free
    d) furthermore cases if getmem is called, (as it typically should)...
        i) RANDOM between chance of a "large block" (pctlarge)
	   a) if "large block": allocate a block between small_limit and large_limit
	   b) else.. its a "small block": allocate a block between size 1 and small_limit
*/
