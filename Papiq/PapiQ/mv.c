//Program for DPUs in Matrix-Vector multiplication

#include <stdint.h>
#include <stdio.h>
#include <defs.h>
#include <mram.h>
#include <alloc.h>
#include <barrier.h>
#include <seqread.h>
#include <float.h>
#include <fp_lib.h>

#define F float

// Matrix-Vector multiplication
static void mv(F* A, F* B, F* H, F* C)
{
    //Verifying that the data is well offloaded
    printf("*** DPU data in WRAM ***\n");

    for(unsigned p = 0; p < 2; p++)
    {
        printf("A[%d] = %f\n", p, A[p]);
    }

    for(unsigned q = 0; q < 2; q++)
    {
        printf("B[%d] = %f\n", q, B[q]);
    }

    for(unsigned u = 0; u < 4; u++)
    {
        printf("H[%d] = %f\n", u, H[u]);
    }

    for(int i = 0; i < 2; i++)
    {
        C[0] += H[i] * A[i];
    }
    printf("C[0] = %f\n", C[0]);


    for(int j = 0; j < 2; j++)
    {
        C[1] += H[j+2] * A[j];
    }
    printf("C[1] = %f\n", C[1]);

}


//Initialize barrier
BARRIER_INIT(my_barrier, NR_TASKLETS);

int main()
{
    //Identify tasklet_it
    unsigned int tasklet_id = me();
    #if printf
        printf("tasklet_id = %u\n", tasklet_id);
    #endif

    //Initialize once the cycle counter & rest the heap
    if(tasklet_id == 0)
    {
        mem_reset();
    }

    //Terminate barrier
    barrier_wait(&my_barrier);

//----------Not mandatory for PapiQ------------------
    //Complex method to divide operation by the # of tasklets
    unsigned int element_per_cacheC = 8/sizeof(F);
    // printf("!!! element_per_cacheC = %d\n", element_per_cacheC);

    unsigned int rows_per_tasklet;
    unsigned int start_row;
    // unsigned int chunks = 2 / (NR_TASKLETS * element_per_cacheC);
    // unsigned int dbl_chunks = chunks * element_per_cacheC;

    // rows_per_tasklet = dbl_chunks;
//---------------------------------------------------------

    //Address of the current row in MRAM
    uint32_t mram_base_addr_A = (uint32_t) (DPU_MRAM_HEAP_POINTER);
    printf("\nMRAM address A = %d\n", mram_base_addr_A);
	uint32_t mram_base_addr_B = (uint32_t) (DPU_MRAM_HEAP_POINTER + 2 * 1 * sizeof(F));
    printf("\nMRAM address B = %d\n", mram_base_addr_B);
	uint32_t mram_base_addr_H = (uint32_t) (DPU_MRAM_HEAP_POINTER + 2 * 1 * sizeof(F) + 2 * 1 * sizeof(F));
    printf("\nMRAM address H = %d\n", mram_base_addr_H);
	uint32_t mram_base_addr_C = (uint32_t) (DPU_MRAM_HEAP_POINTER + 2 * 1 * sizeof(F) + 2 * 1 * sizeof(F) + 2 * 2 * sizeof(F));
    printf("\nMRAM address C = %d\n\n", mram_base_addr_C);

	uint32_t mram_temp_addr_A = mram_base_addr_A;
	uint32_t mram_temp_addr_B = mram_base_addr_B;
	uint32_t mram_temp_addr_H = mram_base_addr_H;

    // printf("MRAM address of vector A = %d\n", mram_base_addr_A);

    // printf("\nVector A data in MRAM\n");
    // for(unsigned o x = 0; x < 2; x++)
    // {
    //     printf("A[%d] = %f\n", x, *(mram_base_addr_A + sizeof(F) * x));
    // }

    // printf("MRAM address of vector B = %d\n", mram_base_addr_B);
    // printf("\nVector B data in MRAM\n");
    // for(unsigned y = 0; y < 2; y++)
    // {
    //     printf("B[%d] = %f\n", y, *(mram_base_addr_A + sizeof(F) * y));
    // }

    printf("MRAM address of vector H = %d\n", mram_base_addr_H);
    // printf("H[0] = %f\n", mram_temp_addr_H[0]);
    // printf("H[1] = %f\n", mram_temp_addr_H[1]);
    // for(unsigned y = 0; y < 2; y++)
    // {
        
    //     printf("B[%d] = %f\n", y, *(mram_temp_addr_H + sizeof(F) * y));
    // }

    
    // Inititalize a local cache to store the MRAM block
	F *cache_A = (F *) mem_alloc(2 * 1 * sizeof(F));
	// T *cache_A_aux = (F *) mem_alloc(8);
	F *cache_B = (F *) mem_alloc(2 * 1 * sizeof(F));
	F *cache_H = (F *) mem_alloc(2 * 2 * sizeof(F));
	F *cache_C = (F *) mem_alloc(2 * 1 * sizeof(F));

    int offset = 0;

    //Initialize the output cache
    // for(unsigned int i = 0; i < 4; i++)
    // {
    //     cache_C[i] = 0;
    // }

    //Copy matrix data (A, B, ...) to the WRAM
    mram_read((__mram_ptr void const*) (mram_temp_addr_A), cache_A, 2 * 1 * sizeof(F));
    mram_read((__mram_ptr void const*) (mram_temp_addr_B), cache_B, 2 * 1 * sizeof(F));
    mram_read((__mram_ptr void const*) (mram_temp_addr_H), cache_H, 2 * 2 * sizeof(F));

    //Verify the data copy
    

    //Compute GEMV
    mv(cache_A, cache_B, cache_H, cache_C);

    // for(unsigned r = 0; r < 4; r++)
    // {
    //     printf("H[%d] = %f\n", r, *(cache_H + sizeof(F) * r));
    // }

    //Update memory addresses (not mandatory in this case)
    // mram_temp_addr_A += 2 * 1 * sizeof(F);
    // mram_temp_addr_B += 2 * 1 * sizeof(F);
    // mram_temp_addr_H += 2 * 1 * sizeof(F);


    //Write cache to current MRAM block
    mram_write(cache_C, (__mram_ptr void *) (mram_base_addr_C), 2 * 1 * sizeof(F));

    //Update memory address (not mandatory in this case)
    // mram_base_addr_C += 8;


    return 0;
}