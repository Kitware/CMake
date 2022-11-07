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
  using ifstream = cmsys::ifstream;
  using ofstream = cmsys::ofstream;
  using fstream = cmsys::fstream;
};

using ifs = cmsys::ifstream;
using ofs = cmsys::ofstream;
using fs = cmsys::fstream;
}

int main()
{
  using std::ifstream;
  using std::ofstream;
  using std::fstream;

  // Correction needed
  cmsys::ifstream ifsUnqual;
  cmsys::ifstream ifsQual;
  cmsys::ifstream ifsNS;
  cmsys::ifstream ifsNested;
  cmsys::ifstream ifsClass;
  cmsys::ifstream ifsRenamed;

  cmsys::ofstream ofsUnqual;
  cmsys::ofstream ofsQual;
  cmsys::ofstream ofsNS;
  cmsys::ofstream ofsNested;
  cmsys::ofstream ofsClass;
  cmsys::ofstream ofsRenamed;

  cmsys::fstream fsUnqual;
  cmsys::fstream fsQual;
  cmsys::fstream fsNS;
  cmsys::fstream fsNested;
  cmsys::fstream fsClass;
  cmsys::fstream fsRenamed;

  cmsys::ifstream::off_type offsetQual = 0;
  cmsys::ifstream::off_type offsetUnqual = 0;
  cmsys::ifstream::off_type offsetNS = 0;
  cmsys::ifstream::off_type offsetNested = 0;
  cmsys::ifstream::traits_type::off_type offsetTraitsNested = 0;
  cmsys::ifstream::traits_type::off_type offsetTraitsClass = 0;

  std::vector<cmsys::ifstream> ifsVectorUnqual;

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
