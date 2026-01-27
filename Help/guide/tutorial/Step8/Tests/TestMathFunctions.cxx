#include <string>

#include <MathFunctions.h>

int main(int argc, char* argv[])
{
  if (argc < 2) {
    return -1;
  }

  std::string op(argv[1]);

  if (op == "add") {
    return mathfunctions::OpAdd(1.0, 1.0) != 2.0;
  } else if (op == "mul") {
    return mathfunctions::OpMul(5.0, 5.0) != 25.0;
  } else if (op == "sqrt") {
    return mathfunctions::sqrt(25.0) != 5.0;
  } else if (op == "sub") {
    return mathfunctions::OpSub(5.0, 1.0) != 4.0;
  }

  return -1;
}
