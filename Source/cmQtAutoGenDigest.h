/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmQtAutoGenDigest_h
#define cmQtAutoGenDigest_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <memory>
#include <string>
#include <vector>

class cmGeneratorTarget;

class cmQtAutoGenDigestQrc
{
public:
  cmQtAutoGenDigestQrc()
    : Generated(false)
    , Unique(false)
  {
  }

public:
  std::string QrcFile;
  std::string QrcName;
  std::string PathChecksum;
  std::string RccFile;
  bool Generated;
  bool Unique;
  std::vector<std::string> Options;
  std::vector<std::string> Resources;
};

/** \class cmQtAutoGenDigest
 * \brief Filtered set of QtAutogen variables for a specific target
 */
class cmQtAutoGenDigest
{
public:
  cmQtAutoGenDigest(cmGeneratorTarget* target)
    : Target(target)
    , MocEnabled(false)
    , UicEnabled(false)
    , RccEnabled(false)
  {
  }

public:
  cmGeneratorTarget* Target;
  std::string QtVersionMajor;
  std::string QtVersionMinor;
  bool MocEnabled;
  bool UicEnabled;
  bool RccEnabled;
  std::vector<std::string> Headers;
  std::vector<std::string> Sources;
  std::vector<cmQtAutoGenDigestQrc> Qrcs;
};

// Utility types
typedef std::unique_ptr<cmQtAutoGenDigest> cmQtAutoGenDigestUP;
typedef std::vector<cmQtAutoGenDigestUP> cmQtAutoGenDigestUPV;

#endif
