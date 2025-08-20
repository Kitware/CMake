#include <cstring>
#include <iostream>
#include <string>

#include <wx/filefn.h>
#include <wx/version.h>

int main()
{
  wxGetCwd();

  std::string found_version = std::to_string(wxMAJOR_VERSION) + "." +
    std::to_string(wxMINOR_VERSION) + "." + std::to_string(wxRELEASE_NUMBER);

#if wxSUBRELEASE_NUMBER > 0
  found_version += "." + std::to_string(wxSUBRELEASE_NUMBER);
#endif

  std::cout << "Found wxWidgets version " << found_version
            << ", expected version " << CMAKE_EXPECTED_WXWIDGETS_VERSION
            << "\n";

  return std::strcmp(found_version.c_str(), CMAKE_EXPECTED_WXWIDGETS_VERSION);
}
