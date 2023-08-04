/**
* app.c
* Arithmetic Throughput Host Application Source File
*
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <dpu.h>
#include <dpu_log.h>
#include <unistd.h>
#include <getopt.h>
#include <assert.h>

#include "../support/common.h"
#include "../support/timer.h"
#include "../support/params.h"

// Define the DPU Binary path as DPU_BINARY here
#ifndef DPU_BINARY
#define DPU_BINARY "./bin/dpu_code"
#endif

// Pointer declaration
static T* A;
static T* B;
static T* C2;

// Create input arrays
static void read_input(T* A, T* B, unsigned int nr_elements) {
    srand(0);
    printf("nr_elements\t%u\t", nr_elements);
    for (unsigned int i = 0; i < nr_elements; i++) {
        A[i] = (T) (rand());
        B[i] = (T) (rand());
    }
}

// Compute output in the host
static void update_host(T* C, T* A, unsigned int nr_elements) {
    for (unsigned int i = 0; i < nr_elements; i++) {
#if ADD
        C[i] = A[i] + (nr_elements / NR_DPUS);
#elif SUB
        C[i] = A[i] - (nr_elements / NR_DPUS);
#elif MUL
        C[i] = A[i] * (nr_elements / NR_DPUS);
#elif DIV
        C[i] = A[i] / (nr_elements / NR_DPUS);
#endif
    }
}

// Main of the Host Application
int main(int argc, char **argv) {

    struct Params p = input_params(argc, argv); // params 받아와

    struct dpu_set_t dpu_set, dpu; // dpu_set : 전체 설정 / dpu : for문 돌리면서 하나씩 설정해주는 변수
    uint32_t nr_of_dpus; // sh에서 받아온 dpu 개수
    
    // Allocate DPUs and load binary
    DPU_ASSERT(dpu_alloc(NR_DPUS, NULL, &dpu_set));
    DPU_ASSERT(dpu_load(dpu_set, DPU_BINARY, NULL)); // 
    DPU_ASSERT(dpu_get_nr_dpus(dpu_set, &nr_of_dpus)); // dpus 개수 유효검사
    printf("Allocated %d DPU(s)\n", nr_of_dpus);

    unsigned int i = 0; // 각 DPU 주소 설정
    double cc = 0; // performance 측정 용도
    double cc_min = 0; // performance 측정 용도
    const unsigned int input_size = p.exp == 0 ? p.input_size * nr_of_dpus : p.input_size; 
    // exp :week
    // input size 설정

    // Input/output allocation
    A = malloc(input_size * sizeof(T));
    B = malloc(input_size * sizeof(T)); // A,B 공간 확보
    T *bufferA = A; // buffer A 시작 주소
    T *bufferB = B; // buffer B 시작 주소
    C2 = malloc(input_size * sizeof(T)); // A,B 공간 확보

    // Create an input file with arbitrary data
    read_input(A, B, input_size); // A와 B에 숫자 넣기

    // Timer declaration
    Timer timer;

    printf("NR_TASKLETS\t%d\tBL\t%d\n", NR_TASKLETS, BL); // thread, block size 확인

    // Loop over main kernel
    for(int rep = 0; rep < p.n_warmup + p.n_reps; rep++) { // rep = 0, p.n_warmup = 1, p.n_reps = 3

        // Compute output on CPU (performance comparison and verification purposes)
        if(rep >= p.n_warmup)
            start(&timer, 0, rep - p.n_warmup);
        update_host(C2, A, input_size);
        if(rep >= p.n_warmup)
            stop(&timer, 0);

        printf("Load input data\n");
        if(rep >= p.n_warmup)
            start(&timer, 1, rep - p.n_warmup);
        // Input arguments
        const unsigned int input_size_dpu = input_size / nr_of_dpus;
        unsigned int kernel = 0;
        dpu_arguments_t input_arguments = {input_size_dpu * sizeof(T), kernel};
        DPU_ASSERT(dpu_copy_to(dpu_set, "DPU_INPUT_ARGUMENTS", 0, (const void *)&input_arguments, sizeof(input_arguments))); // dpu malloc
        // Copy input arrays
        i = 0;
        DPU_FOREACH (dpu_set, dpu) {
            DPU_ASSERT(dpu_copy_to(dpu, DPU_MRAM_HEAP_POINTER_NAME, 0, bufferA + input_size_dpu * i, input_size_dpu * sizeof(T))); // 각각 데이터 카피
            i++;
        }
        if(rep >= p.n_warmup)
            stop(&timer, 1);

        printf("Run program on DPU(s) \n");
        // Run DPU kernel
        if(rep >= p.n_warmup)
            start(&timer, 2, rep - p.n_warmup);
        DPU_ASSERT(dpu_launch(dpu_set, DPU_SYNCHRONOUS)); // 여기에서 DPU 파일 실행
        if(rep >= p.n_warmup)
            stop(&timer, 2);

#if PRINT // 각 DPU 로그
        {
            unsigned int each_dpu = 0;
            printf("Display DPU Logs\n");
            DPU_FOREACH (dpu_set, dpu) {
                printf("DPU#%d:\n", each_dpu);
                DPU_ASSERT(dpulog_read_for_dpu(dpu.dpu, stdout));
                each_dpu++;
            }
        }
#endif

        printf("Retrieve results\n");
        if(rep >= p.n_warmup)
            start(&timer, 3, rep - p.n_warmup);
        i = 0;
        DPU_FOREACH (dpu_set, dpu) {
            // Copy output array
            DPU_ASSERT(dpu_copy_from(dpu, DPU_MRAM_HEAP_POINTER_NAME, input_size_dpu * sizeof(T), bufferB + input_size_dpu * i, input_size_dpu * sizeof(T)));
			// B에 데이터 받기

        dpu_results_t results[nr_of_dpus];    
#if PERF
            results[i].cycles = 0;
            // Retrieve tasklet timings
            for (unsigned int each_tasklet = 0; each_tasklet < NR_TASKLETS; each_tasklet++) {
                dpu_results_t result;
                result.cycles = 0;
                DPU_ASSERT(dpu_copy_from(dpu, "DPU_RESULTS", each_tasklet * sizeof(dpu_results_t), &result, sizeof(dpu_results_t)));
                if (result.cycles > results[i].cycles)
                    results[i].cycles = result.cycles; // 각 DPU thread 중 최고 사이클
            }
#endif
            i++;
        }
        if(rep >= p.n_warmup)
            stop(&timer, 3);

#if PERF
        uint64_t max_cycles = 0;
        uint64_t min_cycles = 0xFFFFFFFFFFFFFFFF;
        // Print performance results
        if(rep >= p.n_warmup){
            i = 0;
            DPU_FOREACH(dpu_set, dpu) { // 최고 사이클 / 최저 사이클
                if(results[i].cycles > max_cycles)
                    max_cycles = results[i].cycles;
                if(results[i].cycles < min_cycles)
                    min_cycles = results[i].cycles;
                i++;
            }
            cc += (double)max_cycles;
            cc_min += (double)min_cycles;
        }
#endif

    }
#ifdef ADD
    printf("ADD\n");
#elif SUB
    printf("SUB\n");
#elif MUL
    printf("MUL\n");
#elif DIV
    printf("DIV\n");
#endif
    printf("DPU cycles  = %g cc\n", cc / p.n_reps);

    // Print timing results
    printf("CPU ");
    print(&timer, 0, p.n_reps);
    printf("CPU-DPU ");
    print(&timer, 1, p.n_reps);
    printf("DPU Kernel ");
    print(&timer, 2, p.n_reps);
    printf("DPU-CPU ");
    print(&timer, 3, p.n_reps);

    // Check output
    bool status = true;
    for (i = 0; i < input_size; i++) { // 받은 B의 값과 host에서 계산한 C2의 값 비교
        if(C2[i] != bufferB[i]){ 
            status = false;
#if PRINT
            printf("%d: %u -- %u\n", i, C2[i], bufferB[i]);
#endif
        }
    }
    if (status) {
        printf("[" ANSI_COLOR_GREEN "OK" ANSI_COLOR_RESET "] Outputs are equal\n");
    } else {
        printf("[" ANSI_COLOR_RED "ERROR" ANSI_COLOR_RESET "] Outputs differ!\n");
    }

    // Deallocation
    free(A);
    free(B);
    free(C2);
    DPU_ASSERT(dpu_free(dpu_set));
	
    return status ? 0 : -1;
}
