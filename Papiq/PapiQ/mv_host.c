/* The very first practice to do Matrix-Vector operation for PapiQ*/
/* Communicate with the one rank of DPUs via MRAM*/
/* Populate the MRAM with a collection of quantum gates (matrices.h) and states */

#include "papiq.h"

unsigned int i;

static F* A;
static F* B;
// static F* C;
static F* C_dpu;


// Initialize input vectors
static void init_vector(F* A, F* B /*unsinged int m_size, unsigned int n_size*/)
{
    //Memory allocation for inputs (eg., Vector A, B, C, etc...)
    // A = malloc(2 * 1 * sizeof(F));
    // B = malloc(2 * 1 * sizeof(F));

    // A[2] = {1, 1};
    // B[2] = {2, 2};
    A[0] = 1;
    A[1] = 1;
    B[0] = 2;
    B[1] = 2;

    for(int i = 0; i < 2; i++)
    {
        printf("A[%d] = %f\n", i, A[i]);
    }

    for(int i = 0; i < 2; i++)
    {
        printf("B[%d] = %f\n", i, B[i]);
    }



}

int main(/* int argc, char **argv */)
{
    //Basic setting for # of dpus
    struct dpu_set_t dpu_set, dpu;
    uint32_t nr_of_dpus = 1;
    

    DPU_ASSERT(dpu_alloc(nr_of_dpus/*NR_DPUS*/, NULL, &dpu_set));
    DPU_ASSERT(dpu_load(dpu_set, DPU_BINARY, NULL));
    DPU_ASSERT(dpu_get_nr_dpus(dpu_set, &nr_of_dpus));
    printf("!!!nr_of_dpus = %d\n", nr_of_dpus);

    //Measure energy consumption
    #if ENERGY
	    struct dpu_probe_t probe;
	    DPU_ASSERT(dpu_probe_init("energy_probe", &probe));
    #endif

    //Memory allocation for inputs (eg., Vector A, B, C, etc...) & outputs (e.g., C, C_dpu)
    A = malloc(2 * 1 * sizeof(F));
    B = malloc(2 * 1 * sizeof(F));
    // C = malloc(1 * 1 * sizeof(F));
    H = malloc(2 * 2 * sizeof(F));

    init_vector(A, B);
    hadamard(H);
    C_dpu = malloc(2 * 1 * sizeof(F));

    //MRAM memory allocation for whole DPU set (matrix A, B, C & vector H  **in progress)
    DPU_ASSERT(dpu_push_xfer(dpu_set, DPU_XFER_TO_DPU, DPU_MRAM_HEAP_POINTER_NAME, 0, 2 * 1 * sizeof(F) + 2 * 1 * sizeof(F) + 2 * 2 * sizeof(F), DPU_XFER_DEFAULT));


    //Copy input array and vector
    //Copy input vector A to DPUs
    i = 1;
    printf("\n");
    printf("======Start offloading to DPUs======\n");
    
    printf("\n");

    DPU_FOREACH(dpu_set, dpu)
    {
        printf("--OFFloading A %d times--\n", i);
        i++;
        DPU_ASSERT(dpu_prepare_xfer(dpu, A));
    }
    DPU_ASSERT(dpu_push_xfer(dpu_set, DPU_XFER_TO_DPU, DPU_MRAM_HEAP_POINTER_NAME, 0, 2 * 1 * sizeof(F), DPU_XFER_DEFAULT));

    //Copy input vector B to DPUs
    i = 1;
    printf("\n");

    DPU_FOREACH(dpu_set, dpu)
    {
        printf("--OFFloading B %d times--\n", i);
        i++;
        DPU_ASSERT(dpu_prepare_xfer(dpu, B));
    }
    DPU_ASSERT(dpu_push_xfer(dpu_set, DPU_XFER_TO_DPU, DPU_MRAM_HEAP_POINTER_NAME, 2 * 1 * sizeof(F), 2 * 1 * sizeof(F), DPU_XFER_DEFAULT));

    //Copy matrix of hadamard gate (H) to DPUs
    i = 1;
    printf("\n");

    DPU_FOREACH(dpu_set, dpu)
    {
        printf("--OFFloading H %d times--\n", i);
        i++;
        DPU_ASSERT(dpu_prepare_xfer(dpu, H));
    }
    DPU_ASSERT(dpu_push_xfer(dpu_set, DPU_XFER_TO_DPU, DPU_MRAM_HEAP_POINTER_NAME, 2 * 1 * sizeof(F) + 2 * 1 * sizeof(F), 2 * 2 * sizeof(F), DPU_XFER_DEFAULT));

    //Initiate operation in DPUs
    printf("\n");
    printf("======DPUs in operation!!!======\n\n");
    DPU_ASSERT(dpu_launch(dpu_set, DPU_SYNCHRONOUS));

    printf("****Printing log for dpu:****\n");
    printf("\n");
    DPU_FOREACH(dpu_set, dpu) {
      DPU_ASSERT(dpu_log_read(dpu, stdout));
    }
    printf("\n");

    i=1;
    //Retrieve results from DPUs
    printf("\n");
    DPU_FOREACH(dpu_set, dpu) 
    {
        printf("===Retive C %d times\n", i);
        i++;
		DPU_ASSERT(dpu_prepare_xfer(dpu, C_dpu));
	}
    DPU_ASSERT(dpu_push_xfer(dpu_set, DPU_XFER_FROM_DPU, DPU_MRAM_HEAP_POINTER_NAME, 2 * 1 * sizeof(F) + 2 * 1 * sizeof(F) + 2 * 2 * sizeof(F) , 2 * 1 * sizeof(F), DPU_XFER_DEFAULT));

    //Checkout output
    for(int p = 0; p < 2; p++)
    {
        printf("C_dpu[%d] = %f\n", p, C_dpu[p]);
    }


    //Deallocation
    free(A);
    free(B);
    // free(C);
    free(C_dpu);
    DPU_ASSERT(dpu_free(dpu_set));

    //Terminates energy measurement
    #if ENERGY
	    DPU_ASSERT(dpu_probe_deinit(&probe));
    #endif

    return 0;
}