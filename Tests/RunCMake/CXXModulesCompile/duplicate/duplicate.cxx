module;

#include <iostream>

export module duplicate;

export int from_import()
{
  std::cerr << "From duplicate #" << NDUPLICATE << std::endl;
  return 0;
}
