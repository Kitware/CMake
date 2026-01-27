// A simple program that computes the square root of a number
#include <format>
#include <iostream>
#include <string>

#include <MathFunctions.h>

int main(int argc, char* argv[])
{
  if (argc < 2) {
    std::cout << std::format("Usage: {} number\n", argv[0]);
    return 1;
  }

  // convert input to double
  double const inputValue = std::stod(argv[1]);

  // calculate square root
  double const outputValue = mathfunctions::sqrt(inputValue);
  std::cout << std::format("The square root of {} is {}\n", inputValue,
                           outputValue);

  // TODO11: Check the calculated square root using mathfunctions::OpMul to
  //         square the outputValue. Output the result with the format:
  //           "The square of {} is {}\n"
}
