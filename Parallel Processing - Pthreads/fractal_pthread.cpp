/*
Fractal code for CS 4380 / CS 5351

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

Modifed by: Alex Tejada
*/

#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <sys/time.h>
#include "cs43805351.h"

// 2 including pthread header file
#include <pthread.h>

// 3.a Thread count
// 13 The thread count will divide program data
static long threads;

// 6.a Global Variables
// 11.a These values will be passed to the function staticlly
static int width;
static int frames;
static unsigned char* pic;

// 6.b Changed the void to void pointer and parameters to global variables
// 7 The function will now only take the thead number as an argument
static void* fractal(void* arg)
{
  // 9.a Passing in thread number
  // 12 arg turned into a long
  const long my_rank = (long)arg;
  const double Delta = 0.006;
  const double xMid = 0.232997;
  const double yMid = 0.550325;

  // 9.b Rewrote the loop for a cyclic assignment based on thread count
  for (int frame = my_rank; frame < frames; frame+=threads) {  // frames
    const double delta = Delta * pow(0.985, frame);
    const double xMin = xMid - delta;
    const double yMin = yMid - delta;
    const double dw = 2.0 * delta / width;
    for (int row = 0; row < width; row++) {  // rows
      const double cy = yMin + row * dw;
      for (int col = 0; col < width; col++) {  // columns
        const double cx = xMin + col * dw;
        double x = cx;
        double y = cy;
        int depth = 256;
        double x2, y2;
        do {
          x2 = x * x;
          y2 = y * y;
          y = 2 * x * y + cy;
          x = x2 - y2 + cx;
          depth--;
        } while ((depth > 0) && ((x2 + y2) < 5.0));
		        pic[frame * width * width + row * width + col] = (unsigned char)depth;
      }
    }
  }
}

int main(int argc, char *argv[])
{
  printf("Fractal v1.8\n");

  // check command line
  if (argc != 4) {fprintf(stderr, "USAGE: %s frame_width num_frames\n", argv[0]); exit(-1);}
  width = atoi(argv[1]);
  if (width < 10) {fprintf(stderr, "ERROR: frame_width must be at least 10\n"); exit(-1);}
  frames = atoi(argv[2]);
  if (frames < 1) {fprintf(stderr, "ERROR: num_frames must be at least 1\n"); exit(-1);}
  // 3.b Determine specified thread count
  threads = atoi(argv[3]);
  // 4 Determine if the number of threads specified is allowed
  if (threads < 1) {fprintf(stderr, "error: threads must be at least 1\n"); exit(-1);}
  printf("frames: %d\n", frames);
  printf("width: %d\n", width);
  // 5 Printing thread count
  printf("threads: %ld\n", threads);

  // allocate picture array
  pic = new unsigned char [frames * width * width];

  // 8 Initiate the thread handlier
  pthread_t* const handle = new pthread_t [threads - 1];

  // start time
  timeval start, end;
  gettimeofday(&start, NULL);

  // 8.b Creating threads specified for the function
  // 11.b Passing rank, thread, returing NULL
  for (long thread = 0; thread < threads - 1; thread++) {
    pthread_create(&handle[thread], NULL, fractal, (void *)thread);
  }

  // 8.c Master thread operation
  fractal((void*)(threads - 1));

  // call timed function
  // dggesrhhsghsrhtar
  // fractal(pic);

  // 8.d join threads
  for (long thread = 0; thread < threads - 1; thread++) {
    pthread_join(handle[thread], NULL);
  }

  // end time
  gettimeofday(&end, NULL);
  const double runtime = end.tv_sec - start.tv_sec + (end.tv_usec - start.tv_usec) / 1000000.0;
  printf("compute time: %.4f s\n", runtime);

  // write result to BMP files
  if ((width <= 256) && (frames <= 100)) {
    for (int frame = 0; frame < frames; frame++) {
      char name[32];
      sprintf(name, "fractal%d.bmp", frame + 1000);
      writeBMP(width, width, &pic[frame * width * width], name);
    }
  }

  // 14 Deallocating the global array
  delete [] pic;
  return 0;
}
