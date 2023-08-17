/*
 * Matrix vector multiplication with multiple tasklet
 *
 */
#include <stdint.h>
#include <stdio.h>
#include <defs.h>
#include <mram.h>
#include <alloc.h>
#include <barrier.h>
#include <seqread.h>

#include "../support/common.h"

#define roundup(n, m) ((n / m) * m + m)

//Argument = ni**er
__host dpu_arguments_t DPU_INPUT_ARGUMENTS;

// GEMV
static void gemv(T *bufferC, T *bufferA, T *bufferB, int pos) {
	printf("GEMV BLOCK_SIZE = %d pos = %d\n", pos, BLOCK_SIZE);
	for (unsigned int i = 0; i < BLOCK_SIZE / sizeof(T); i++) {
		bufferC[pos] += bufferA[i] * bufferB[i];
	}
	return;
}

// Barrier
BARRIER_INIT(my_barrier, NR_TASKLETS);

// main
int main() {
	unsigned int tasklet_id = me();
	printf("!!! tasklet_id = %d\n", tasklet_id);
#if PRINT
	// printf("tasklet_id = %u\n", tasklet_id);
#endif
	if (tasklet_id == 0){ // Initialize once the cycle counter
		mem_reset(); // Reset the heap
	}
	// Barrier
	barrier_wait(&my_barrier);

	int32_t n_size = DPU_INPUT_ARGUMENTS.n_size;
	int32_t n_size_pad = DPU_INPUT_ARGUMENTS.n_size_pad;
	uint32_t nr_rows = DPU_INPUT_ARGUMENTS.nr_rows;
	uint32_t max_rows = DPU_INPUT_ARGUMENTS.max_rows;
	printf("!!! n_size = %d\n", n_size);
	printf("!!! n_size_pad = %d\n", n_size_pad);
	printf("!!! nr_rows = %d\n", nr_rows);
	printf("!!! max_rows = %d\n", max_rows);

	unsigned int element_per_cacheC = 8/sizeof(T);
	printf("!!! element_per_cacheC = %d\n", element_per_cacheC);

	unsigned int nrows = nr_rows;
	unsigned int rows_per_tasklet; 
	unsigned int start_row;
	unsigned int chunks = nrows / (NR_TASKLETS * element_per_cacheC);
	unsigned int dbl_chunks = chunks * element_per_cacheC; //chunks + chunks; 
	rows_per_tasklet = dbl_chunks;
	unsigned int rest_rows = nrows % (NR_TASKLETS * element_per_cacheC); //(NR_TASKLETS + NR_TASKLETS);
	printf("!!! nrows = %d\n", nrows);
	printf("!!! rows_per_tasklet = %d\n", rows_per_tasklet);
	printf("!!! chunks = %d\n", chunks);
	printf("!!! dbl_chunks = %d\n", dbl_chunks);
	printf("!!! rest_rows = %d\n", rest_rows);

	if ((tasklet_id * element_per_cacheC) < rest_rows){
		rows_per_tasklet += element_per_cacheC;
		printf("1 rows_per_tasklet = %d\n", rows_per_tasklet);
	}
	if (rest_rows > 0) {
		if ((tasklet_id * element_per_cacheC) >= rest_rows) {
			// unsigned int hlf_rest_rows = rest_rows >> 1;
			if ((rest_rows % element_per_cacheC) != 0) {
				start_row = roundup(rest_rows, element_per_cacheC) + tasklet_id * dbl_chunks; 
				// start_row = (hlf_rest_rows + 1) * (dbl_chunks + 2) + (tasklet_id - 1 - hlf_rest_rows) * dbl_chunks;
				printf("2 start_row = %d\n", start_row);
			}
			else {
				start_row = rest_rows + tasklet_id * dbl_chunks; 
				// start_row = (hlf_rest_rows) * (dbl_chunks + 2) + (tasklet_id - hlf_rest_rows) * dbl_chunks;
				printf("3 start_row = %d\n", start_row);
			}
		} else {
			start_row = tasklet_id * (dbl_chunks + element_per_cacheC);
			// start_row = tasklet_id * (dbl_chunks + 2);
			printf("4 start_row = %d\n", start_row);
		}
	} else {
		start_row = tasklet_id * (dbl_chunks);
		printf("5 start_row = %d\n", start_row);
	}

	// Address of the current row in MRAM
	uint32_t mram_base_addr_A = (uint32_t) (DPU_MRAM_HEAP_POINTER + start_row * n_size * sizeof(T));
	uint32_t mram_base_addr_B = (uint32_t) (DPU_MRAM_HEAP_POINTER + max_rows * n_size_pad * sizeof(T));
	uint32_t mram_base_addr_C = (uint32_t) (DPU_MRAM_HEAP_POINTER + max_rows * n_size_pad * sizeof(T) + n_size_pad * sizeof(T) + start_row * sizeof(T));
	uint32_t mram_temp_addr_A = mram_base_addr_A;
	uint32_t mram_temp_addr_B = mram_base_addr_B;
	printf("@@@ mram_base_addr_A = %d\n", mram_base_addr_A);
	printf("@@@ mram_base_addr_B = %d\n", mram_base_addr_B);
	printf("@@@ mram_base_addr_C = %d\n", mram_base_addr_C);
	printf("@@@ mram_temp_addr_A = %d\n", mram_temp_addr_A);
	printf("@@@ mram_temp_addr_B = %d\n", mram_temp_addr_B);

	// Inititalize a local cache to store the MRAM block
	T *cache_A = (T *) mem_alloc(BLOCK_SIZE + 8);
	T *cache_A_aux = (T *) mem_alloc(8);
	T *cache_B = (T *) mem_alloc(BLOCK_SIZE);
	T *cache_C = (T *) mem_alloc(8);

	int offset = 0;

	#if PRINT
	printf("id: %d, rows_per_tasklet = %d\n",tasklet_id, rows_per_tasklet);
	printf("id: %d, start_row = %d\n",tasklet_id, start_row);
	#endif

	// Iterate over nr_rows
	// for (unsigned int i = start_row; i < start_row + rows_per_tasklet; i += 2) {
	for (unsigned int i = start_row; i < start_row + rows_per_tasklet; i += element_per_cacheC) {

		mram_temp_addr_A = (uint32_t) (DPU_MRAM_HEAP_POINTER + i * n_size * sizeof(T));
		mram_temp_addr_B = mram_base_addr_B;
		printf("====== Iteration1 %d =======\n", i);
		printf("Iteration1 mram_temp_addr_A = %d\n", mram_temp_addr_A);
		printf("Iteration1 mram_temp_addr_B = %d\n", mram_temp_addr_B);

		// cache_C[0] = 0;
		// cache_C[1] = 0;

		// clear the cache
		for(unsigned int c = 0; c < element_per_cacheC; c++){
			cache_C[c] = 0; 
		}

		// for(unsigned int pos = 0; pos < 2 && i + pos < nr_rows; pos++){
		// for(unsigned int pos = 0; (pos < element_per_cacheC) && ((i + pos) < (start_row + rows_per_tasklet)); pos++){
		// for(unsigned int pos = 0; pos < element_per_cacheC && i + pos < nr_rows; pos++){ 
		for(unsigned int pos = 0; pos < element_per_cacheC; pos++){ 
			printf("====== Iteration2 %d =======\n", pos);

			if(i + pos >= nr_rows){
				printf("BREAK!!!! id: %d, nrows: %d, error\n", tasklet_id, nrows);
				break;
			} 

			int n = 0, j;
			for (n = 0; n < (int32_t) (n_size - (BLOCK_SIZE/sizeof(T))); n += (BLOCK_SIZE / sizeof(T)))
			{
				printf("====== Iteration3 %d =======\n", n);

				mram_read((__mram_ptr void const*) (mram_temp_addr_A), cache_A, BLOCK_SIZE);
				mram_read((__mram_ptr void const*) (mram_temp_addr_B), cache_B, BLOCK_SIZE);

				if(offset)
				{
				    printf("Iteration3 2 offset = %d\n", offset);
					for(unsigned int off = 0; off < (BLOCK_SIZE / sizeof(T)) - 1; off++)
					{
						cache_A[off] = cache_A[off + 1];
					}

					mram_read((__mram_ptr void const*) (mram_temp_addr_A + BLOCK_SIZE), cache_A_aux, 8);

					cache_A[BLOCK_SIZE / sizeof(T) - 1] = cache_A_aux[0];
				}

				// Compute GEMV
				gemv(cache_C, cache_A, cache_B, pos);

				// Update memory addresses
				mram_temp_addr_A += BLOCK_SIZE;
				mram_temp_addr_B += BLOCK_SIZE;
			    printf("Iteration3 mram_temp_addr_A = %d\n", mram_temp_addr_A);
		        printf("Iteration3 mram_temp_addr_B = %d\n", mram_temp_addr_B);
			}

			mram_read((__mram_ptr void const*) (mram_temp_addr_A), cache_A, BLOCK_SIZE);


			if(offset)
			{
				printf("Iteration2 1 offset = %d\n", offset);
				for(unsigned int off = 0; off < (BLOCK_SIZE / sizeof(T)) -1; off++)
				{

					cache_A[off] = cache_A[off + 1];
				}

				mram_read((__mram_ptr void const*) (mram_temp_addr_A + BLOCK_SIZE ), cache_A_aux, 8);

  			    cache_A[BLOCK_SIZE / sizeof(T) - 1] = cache_A_aux[0];
			}


			mram_read((__mram_ptr void const*) (mram_temp_addr_B), cache_B, BLOCK_SIZE);

			for (j = 0; j < (int) (n_size - n); j++) {
				// Compute GEMV
				printf("====== Iteration4 %d =======\n", j);

				if(j >= (int)(BLOCK_SIZE / sizeof(T))){ 
					printf("error\n");
					break;
				}
				cache_C[pos] += cache_A[j] * cache_B[j];
			}

			mram_temp_addr_A += (BLOCK_SIZE - ((BLOCK_SIZE / sizeof(T)) - (n_size - n)) * sizeof(T));
			mram_temp_addr_B = mram_base_addr_B;
			printf("Iteration2 mram_temp_addr_A = %d\n", mram_temp_addr_A);
			printf("Iteration2 mram_temp_addr_B = %d\n", mram_temp_addr_B);

			if(mram_temp_addr_A % 8 != 0)
			{
				offset = 1;
				printf("Iteration2 2 offset = %d\n", offset);
			}
			else
			{
				offset = 0;
				printf("Iteration2 3 offset = %d\n", offset);
			}
		}
		// Write cache to current MRAM block
		mram_write(cache_C, (__mram_ptr void *) (mram_base_addr_C), 8);

		// Update memory address
		// mram_base_addr_C += 2 * sizeof(T);
		mram_base_addr_C += 8; 
		printf("Iteration1 mram_base_addr_C = %d\n", mram_base_addr_C);
	}

	return 0;
}
