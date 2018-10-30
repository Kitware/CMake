
// simple workaround to some compiler specific problems
// see
// http://stackoverflow.com/questions/22367516/mex-compile-error-unknown-type-name-char16-t/23281916#23281916
#include <algorithm>

#include "mex.h"

// This test uses the new complex-interleaved C API (R2018a and newer)

// The input should be a complex array (scalar is OK). It returns the number of
// bytes in a matrix element. For the old (R2017b) API, this is 8. For the new
// (R2018a) API, this is 16.

void mexFunction(const int nlhs, mxArray* plhs[], const int nrhs,
                 const mxArray* prhs[])
{
  if (nrhs != 1 || !mxIsComplex(prhs[0])) {
    mexErrMsgTxt("Incorrect arguments");
  }
  plhs[0] = mxCreateDoubleScalar(mxGetElementSize(prhs[0]));
}
