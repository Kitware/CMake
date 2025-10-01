#include <ctime>

#include "cm_parse_date.h"

#include "testCommon.h"

namespace {

bool parse_date()
{
  std::cout << "parse_date()\n";
  std::time_t now;
  std::time(&now);
  {
    std::time_t t = cm_parse_date(now, "20000101 00:01:02 -0000");
    ASSERT_EQUAL(t, 946684862);
  }
  {
    std::time_t t = cm_parse_date(now, "20380601 00:01:02 -0000");
    ASSERT_EQUAL(t, sizeof(time_t) <= 4 ? -1 : 2158963262);
  }
  return true;
}

}

int testDateTime(int /*unused*/, char* /*unused*/[])
{
  return runTests({
    parse_date,
  });
}
