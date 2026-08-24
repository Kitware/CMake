#include <cstring>
#include <iostream>
#include <string>

#include <Magick++.h>

int main()
{
  Magick::InitializeMagick("");

  std::string found_version =
    std::string(MagickLibVersionText) + MagickLibAddendum;

  // Pre-release builds append an annotation such as " (Beta)" to
  // MagickLibAddendum which FindImageMagick's ImageMagick_VERSION does not
  // include.
  std::string::size_type annotation = found_version.find(" (");
  if (annotation != std::string::npos) {
    found_version.erase(annotation);
  }

  std::cout << "Found ImageMagick version " << found_version
            << ", expected version " << CMAKE_EXPECTED_IMAGEMAGICK_VERSION
            << "\n";

  return std::strcmp(found_version.c_str(),
                     CMAKE_EXPECTED_IMAGEMAGICK_VERSION);
}
