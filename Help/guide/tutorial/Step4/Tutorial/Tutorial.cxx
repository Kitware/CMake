// A simple program that computes the square root of a number
#include <format>
#include <iostream>
#include <string>

#include <MathFunctions.h>

#ifdef TUTORIAL_USE_VENDORLIB
#  include <Vendor.h>
#endif

int main(int argc, char* argv[])
{
  if (argc < 2) {
    std::cout << std::format("Usage: {} number\n", argv[0]);
    return 1;
  }

  int unused;

  // convert input to double
  double const inputValue = std::stod(argv[1]);

  // calculate square root
  double const outputValue = mathfunctions::sqrt(inputValue);
  std::cout << std::format("The square root of {} is {}\n", inputValue,
                           outputValue);

#ifdef TUTORIAL_USE_VENDORLIB
  if (CheckSqrt(inputValue, outputValue)) {
    std::cout << "Sqrt verified by vendor\n";
  } else {
    std::cout << "Sqrt rejected by vendor\n";
  }
#endif
}
