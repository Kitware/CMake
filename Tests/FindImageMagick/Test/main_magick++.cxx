#include <cstring>
#include <iostream>
#include <string>

#include <Magick++.h>

int main()
{
  Magick::InitializeMagick("");

  std::string found_version =
    std::string(MagickLibVersionText) + MagickLibAddendum;

  std::cout << "Found ImageMagick version " << found_version
            << ", expected version " << CMAKE_EXPECTED_IMAGEMAGICK_VERSION
            << "\n";

  return std::strcmp(found_version.c_str(),
                     CMAKE_EXPECTED_IMAGEMAGICK_VERSION);
}
