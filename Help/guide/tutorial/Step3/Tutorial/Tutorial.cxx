// A simple program that computes the square root of a number

// TODO3: Include <format>

#include <iostream>
#include <string>

#include <MathFunctions.h>

int main(int argc, char* argv[])
{
  if (argc < 2) {
    // TODO4: Convert the print to use std::format
    std::cout << "Usage: " << argv[0] << " number" << std::endl;
    return 1;
  }

  // convert input to double
  double const inputValue = std::stod(argv[1]);

  // calculate square root
  double const outputValue = mathfunctions::sqrt(inputValue);
  // TODO5: Convert the print to use std::format
  std::cout << "The square root of " << inputValue << " is " << outputValue
            << std::endl;
}
