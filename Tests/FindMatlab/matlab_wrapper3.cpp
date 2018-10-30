#include "mex.hpp"
#include "mexAdapter.hpp"

// This test uses the new C++ API (R2018a and newer)

// The input should be a scalar double array. The output is a copy of that
// array.

using namespace matlab::data;
using matlab::mex::ArgumentList;

class MexFunction : public matlab::mex::Function
{
public:
  void operator()(ArgumentList outputs, ArgumentList inputs)
  {
    std::shared_ptr<matlab::engine::MATLABEngine> matlabPtr = getEngine();
    ArrayFactory factory;
    if (inputs[0].getType() != ArrayType::DOUBLE ||
        inputs[0].getType() == ArrayType::COMPLEX_DOUBLE ||
        inputs[0].getNumberOfElements() != 1) {
      matlabPtr->feval(
        u"error", 0,
        std::vector<Array>({ factory.createScalar("Incorrect arguments") }));
    }
    double a = inputs[0][0];
    outputs[0] = factory.createScalar(a);
  }
};
