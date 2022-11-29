// Comes from:
// https://docs.nvidia.com/cuda/curand/host-api-overview.html#host-api-example

#ifdef _WIN32
#  define EXPORT __declspec(dllexport)
#else
#  define EXPORT
#endif

/*
 * This program uses the host CURAND API to generate 100
 * pseudorandom floats.
 */
#include <cuda.h>
#include <curand.h>
#include <stdio.h>
#include <stdlib.h>

#define CUDA_CALL(x)                                                          \
  do {                                                                        \
    if ((x) != cudaSuccess) {                                                 \
      printf("Error at %s:%d\n", __FILE__, __LINE__);                         \
      return EXIT_FAILURE;                                                    \
    }                                                                         \
  } while (0)
#define CURAND_CALL(x)                                                        \
  do {                                                                        \
    if ((x) != CURAND_STATUS_SUCCESS) {                                       \
      printf("Error at %s:%d\n", __FILE__, __LINE__);                         \
      return EXIT_FAILURE;                                                    \
    }                                                                         \
  } while (0)

EXPORT int curand_main()
{
  size_t n = 100;
  size_t i;
  curandGenerator_t gen;
  float *devData, *hostData;

  /* Allocate n floats on host */
  hostData = (float*)calloc(n, sizeof(float));

  /* Allocate n floats on device */
  CUDA_CALL(cudaMalloc((void**)&devData, n * sizeof(float)));

  /* Create pseudo-random number generator */
  CURAND_CALL(curandCreateGenerator(&gen, CURAND_RNG_PSEUDO_DEFAULT));

  /* Set seed */
  CURAND_CALL(curandSetPseudoRandomGeneratorSeed(gen, 1234ULL));

  /* Generate n floats on device */
  CURAND_CALL(curandGenerateUniform(gen, devData, n));

  /* Copy device memory to host */
  CUDA_CALL(
    cudaMemcpy(hostData, devData, n * sizeof(float), cudaMemcpyDeviceToHost));

  /* Cleanup */
  CURAND_CALL(curandDestroyGenerator(gen));
  CUDA_CALL(cudaFree(devData));
  free(hostData);
  return EXIT_SUCCESS;
}
