// This should not link, as the mex function is missing.
// This is mostly for checking we are passing the right arguments to the
// add_library

#include <algorithm>

#include "mex.h"

void mexFunctionXX(const int nlhs, mxArray* plhs[], const int nrhs,
                   const mxArray* prhs[])
{
  mexErrMsgTxt("Should not be running");
}
