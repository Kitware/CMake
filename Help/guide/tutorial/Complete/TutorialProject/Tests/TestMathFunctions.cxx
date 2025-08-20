#include <MathFunctions.h>
#include <SimpleTest.h>

TEST("add")
{
  REQUIRE(mathfunctions::OpAdd(2.0, 2.0) == 4.0);
}

TEST("sub")
{
  REQUIRE(mathfunctions::OpSub(4.0, 2.0) == 2.0);
}

TEST("mul")
{
  REQUIRE(mathfunctions::OpMul(5.0, 5.0) == 25.0);
}

TEST("sqrt")
{
  REQUIRE(mathfunctions::sqrt(25.0) == 5.0);
}
