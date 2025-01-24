// This should not link, as the mex function is missing.
// This is mostly for checking we are passing the right arguments to the
// add_library

#include <algorithm>

#include "mex.h"

void mexFunctionXX(int const nlhs, mxArray* plhs[], int const nrhs,
                   mxArray const* prhs[])
{
  mexErrMsgTxt("Should not be running");
}
