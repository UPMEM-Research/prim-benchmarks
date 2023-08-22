/* Header file for 'Energy Efficient Processing-in-Memory API'
   1. Hahadmard Gate
   2. CNOT Gate
   3.

   **This file is in-progress.
*/

#ifndef _PAPIQ_H_
#define _PAPIQ_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <dpu.h>
#include <dpu_log.h>
#include <unistd.h>
#include <getopt.h>
#include <assert.h>
#include <math.h>
#include <float.h>
// #include <fp_lib.h>

//libarary for energy measurement
#if ENERGY
#include <dpu_probe.h>
#endif

//Define the DPU binary
#ifndef DPU_BINARY
#define DPU_BINARY "mv"
#endif 

//Data type for Matrix-Vector multiplication
#define I uint32_t
#define T uint64_t
#define F float
// #define float F
#define D double
// #define SINGLE_PRECISION

//Transfer size between MRAM and WRAM (not used in this file)
#ifdef BL
#define BLOCK_SIZE_LOG2 8
#define BLOCK_SIZE (1 << BLOCK_SIZE_LOG2)
#define BL BLOCK_SIZE_LOG2
#endif


static F* H;
static F* CNOT;


// /* Code for hadamard gate in quantum computing */
static void hadamard(F* H /*unsigned int m_size, unsigned int n_size*/)
{
   F A = (1 / sqrt(2)) * 1;
   F B = (1 / sqrt(2)) * (-1);


   /* Code below is for future study
      Let's suppose 2X2 Matrix for the simplification*/

   // uint32_t numRows = m_size;
   // uint32_t numCols = n_size;

   // H = malloc(numRows * numCols* sizeof(float));

   // for(unsigned int i = 0; i < numRows; i++)
   //     for(unsigned int j = 0; j < numCols; j++)
   //     {
   //         H[i][j] = ;
   //     }

   //Memory allocation for 2 X 2 matrix for Hadamard (H) gate
   // H = malloc(2 * 2 * sizeof(F));

   // H[4] = {A, B, A, B};
   H[0] = A; 
   H[1] = A; 
   H[2] = A; 
   H[3] = A;
   

   for(unsigned int i = 0; i < 4; i++)
   {
      printf("H[%d] = %f\n", i, H[i]);
   }

}


/* Code for CNOT gate in quantum computing */
// static float CNOT(   /*uint32_t rows, uint32_t cols*/)
// {
//     // uint32_t numRows = rows;
//     // uint32_t numCols = cols;

//     // H = malloc(numRows * numCols* sizeof(float));


//     return Cnot[2][2];
// }

#endif


