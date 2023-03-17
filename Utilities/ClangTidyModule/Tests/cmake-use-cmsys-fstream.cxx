#include <fstream>
#include <vector>

namespace cmsys {
using std::ifstream;
using std::ofstream;
using std::fstream;
}

namespace ns {
using std::ifstream;
using std::ofstream;
using std::fstream;

namespace ns {
using std::ifstream;
using std::ofstream;
using std::fstream;
}

class cl
{
public:
  using ifstream = std::ifstream;
  using ofstream = std::ofstream;
  using fstream = std::fstream;
};

using ifs = std::ifstream;
using ofs = std::ofstream;
using fs = std::fstream;
}

int main()
{
  using std::ifstream;
  using std::ofstream;
  using std::fstream;

  // Correction needed
  ifstream ifsUnqual;
  std::ifstream ifsQual;
  ns::ifstream ifsNS;
  ns::ns::ifstream ifsNested;
  ns::cl::ifstream ifsClass;
  ns::ifs ifsRenamed;

  ofstream ofsUnqual;
  std::ofstream ofsQual;
  ns::ofstream ofsNS;
  ns::ns::ofstream ofsNested;
  ns::cl::ofstream ofsClass;
  ns::ofs ofsRenamed;

  fstream fsUnqual;
  std::fstream fsQual;
  ns::fstream fsNS;
  ns::ns::fstream fsNested;
  ns::ns::fstream fsClass;
  ns::fs fsRenamed;

  std::ifstream::off_type offsetQual = 0;
  ifstream::off_type offsetUnqual = 0;
  ns::ifstream::off_type offsetNS = 0;
  ns::ns::ifstream::off_type offsetNested = 0;
  ns::ns::ifstream::traits_type::off_type offsetTraitsNested = 0;
  ns::cl::ifstream::traits_type::off_type offsetTraitsClass = 0;

  std::vector<ifstream> ifsVectorUnqual;

  // No correction needed
  cmsys::ifstream ifsCmsys;
  cmsys::ofstream ofsCmsys;
  cmsys::fstream fsCmsys;
  cmsys::ifstream::off_type offsetCmsys = 0;
  cmsys::ifstream::traits_type::off_type offsetTraitsCmsys = 0;
  std::vector<cmsys::ifstream> ifsVectorCmsys;
  std::basic_ifstream<wchar_t> ifsWchar;

  return 0;
}
