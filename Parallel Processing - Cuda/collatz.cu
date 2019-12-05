/*
Collatz code for CS 4380 / CS 5351

Copyright (c) 2019 Texas State University. All rights reserved.

Redistribution in source or binary form, with or without modification,
is *not* permitted. Use in source and binary forms, with or without
modification, is only permitted for academic use in CS 4380 or CS 5351
at Texas State University.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Author: Martin Burtscher


*/

#include <cstdio>
#include <algorithm>
#include <sys/time.h>
// 1 - Cuda Header File
#include <cuda.h>

// 2 - Thread count per block for the collatz code
static const int ThreadsPerBlock = 512;

// 3 - kernel function
static __global__ void collatz(int* maxlen, const long range)
{
  // compute sequence lengths
  // global identifier
  const long idx = threadIdx.x + blockIdx.x * (long)blockDim.x;

  //for (long i = 1; i <= range; i += 2) {
  if (idx%2 != 0) {
    long val = idx;
    int len = 1;
    while (val != 1) {
      len++;
      if ((val % 2) == 0) {
        val = val / 2;  // even
      } else {
        val = 3 * val + 1;  // odd
      }
    }

    //maxlen = std::max(maxlen, len);
  if ( *maxlen < len)
    atomicMax(maxlen, len);
  //}
  }
  //return maxlen;
}

static void CheckCuda()
{
  cudaError_t e;
  cudaDeviceSynchronize();
  if (cudaSuccess != (e = cudaGetLastError())) {
    fprintf(stderr, "CUDA error %d: %s\n", e, cudaGetErrorString(e));
    exit(-1);
  }
}

int main(int argc, char *argv[])
{
  printf("Collatz v1.1\n");

  // check command line
  if (argc != 2) {fprintf(stderr, "USAGE: %s range\n", argv[0]); exit(-1);}
  const long range = atol(argv[1]);
  if (range < 3) {fprintf(stderr, "ERROR: range must be at least 3\n"); exit(-1);}
  printf("range bound: %ld\n", range);

  // 10 - alloc space for device copy
  int* d_maxlen;
  const int size = sizeof(int);
  cudaMalloc((void **)&d_maxlen, size);

  // 10 - alloc host copy
  int maxlen = 0;

  //  copy to device
  if (cudaSuccess != cudaMemcpy(d_maxlen, &maxlen, size, cudaMemcpyHostToDevice)) {
     fprintf(stderr, "Error: failed to copy to device\n");
     exit(-1);
   }


  // start time
  timeval start, end;
  gettimeofday(&start, NULL);

  // call timed function
  // 11 - round up      8 - pass maxlen to kernel
  collatz<<<( range + ThreadsPerBlock - 1) / ThreadsPerBlock, ThreadsPerBlock>>>(d_maxlen, range);
  // 12 -
  cudaDeviceSynchronize();

  // end time
  gettimeofday(&end, NULL);
  const double runtime = end.tv_sec - start.tv_sec + (end.tv_usec - start.tv_usec) / 1000000.0;
  printf("compute time: %.4f s\n", runtime);
  // 13 - check fpr errors
  CheckCuda();

  // 14 - copy result back to host
  if (cudaSuccess != cudaMemcpy(&maxlen, d_maxlen, size, cudaMemcpyDeviceToHost)) {
    fprintf(stderr, "Error: failed to copy from device\n");
    exit(-1);
  }
  printf("longest sequence: %d elements\n", maxlen);

  // 15 - free allocated memory
  cudaFree(d_maxlen);

  return 0;
}
