// A simple function that computes the square root of a number
#include <iostream>
#include <sstream>
#include <string>

#include "MathFunctions.h"

double string_square_root(std::string const& value)
{
  return mathfunctions::sqrt(std::stod(value));
}
