// Comes from
// https://devtalk.nvidia.com/default/topic/1037482/gpu-accelerated-libraries/help-me-help-you-with-modern-cmake-and-cuda-mwe-for-npp/post/5271066/#5271066

#ifdef _WIN32
#  define EXPORT __declspec(dllexport)
#else
#  define EXPORT
#endif

#include <cstdio>

#include <assert.h>
#include <cuda_runtime_api.h>
#include <nppi_filtering_functions.h>

EXPORT int nppif_main()
{
  /**
   * 8-bit unsigned single-channel 1D row convolution.
   */
  int const simgrows = 32;
  int const simgcols = 32;
  Npp8u *d_pSrc, *d_pDst;
  int const nMaskSize = 3;
  NppiSize oROI;
  oROI.width = simgcols - nMaskSize;
  oROI.height = simgrows;
  int const simgsize = simgrows * simgcols * sizeof(d_pSrc[0]);
  int const dimgsize = oROI.width * oROI.height * sizeof(d_pSrc[0]);
  int const simgpix = simgrows * simgcols;
  int const dimgpix = oROI.width * oROI.height;
  int const nSrcStep = simgcols * sizeof(d_pSrc[0]);
  int const nDstStep = oROI.width * sizeof(d_pDst[0]);
  int const pixval = 1;
  int const nDivisor = 1;
  Npp32s const h_pKernel[nMaskSize] = { pixval, pixval, pixval };
  Npp32s* d_pKernel;
  Npp32s const nAnchor = 2;
  cudaError_t err = cudaMalloc((void**)&d_pSrc, simgsize);
  if (err != cudaSuccess) {
    fprintf(stderr, "Cuda error %d\n", __LINE__);
    return 1;
  }
  err = cudaMalloc((void**)&d_pDst, dimgsize);
  if (err != cudaSuccess) {
    fprintf(stderr, "Cuda error %d\n", __LINE__);
    return 1;
  }
  err = cudaMalloc((void**)&d_pKernel, nMaskSize * sizeof(d_pKernel[0]));
  if (err != cudaSuccess) {
    fprintf(stderr, "Cuda error %d\n", __LINE__);
    return 1;
  }
  // set image to pixval initially
  err = cudaMemset(d_pSrc, pixval, simgsize);
  if (err != cudaSuccess) {
    fprintf(stderr, "Cuda error %d\n", __LINE__);
    return 1;
  }
  err = cudaMemset(d_pDst, 0, dimgsize);
  if (err != cudaSuccess) {
    fprintf(stderr, "Cuda error %d\n", __LINE__);
    return 1;
  }
  err = cudaMemcpy(d_pKernel, h_pKernel, nMaskSize * sizeof(d_pKernel[0]),
                   cudaMemcpyHostToDevice);
  if (err != cudaSuccess) {
    fprintf(stderr, "Cuda error %d\n", __LINE__);
    return 1;
  }
  // copy src to dst
  NppStatus ret =
    nppiFilterRow_8u_C1R(d_pSrc, nSrcStep, d_pDst, nDstStep, oROI, d_pKernel,
                         nMaskSize, nAnchor, nDivisor);
  assert(ret == NPP_NO_ERROR);
  Npp8u* h_imgres = new Npp8u[dimgpix];
  err = cudaMemcpy(h_imgres, d_pDst, dimgsize, cudaMemcpyDeviceToHost);
  if (err != cudaSuccess) {
    fprintf(stderr, "Cuda error %d\n", __LINE__);
    return 1;
  }
  // test for filtering
  for (int i = 0; i < dimgpix; i++) {
    if (h_imgres[i] != (pixval * pixval * nMaskSize)) {
      fprintf(stderr, "h_imgres at index %d failed to match\n", i);
      return 1;
    }
  }

  return 0;
}
