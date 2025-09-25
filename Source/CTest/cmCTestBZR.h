/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <string>

#include "cmCTestGlobalVC.h"

class cmCTest;
class cmMakefile;

/** \class cmCTestBZR
 * \brief Interaction with bzr command-line tool
 *
 */
class cmCTestBZR : public cmCTestGlobalVC
{
public:
  /** Construct with a CTest instance and update log stream.  */
  cmCTestBZR(cmCTest* ctest, cmMakefile* mf, std::ostream& log);

  ~cmCTestBZR() override;

private:
  // Implement cmCTestVC internal API.
  bool NoteOldRevision() override;
  bool NoteNewRevision() override;
  bool UpdateImpl() override;

  // URL of repository directory checked out in the working tree.
  std::string URL;

  std::string LoadInfo();
  bool LoadModifications() override;
  bool LoadRevisions() override;

  // Parsing helper classes.
  class InfoParser;
  class LogParser;
  class RevnoParser;
  class StatusParser;
  class UpdateParser;

  friend class InfoParser;
  friend class LogParser;
  friend class RevnoParser;
  friend class UpdateParser;
  friend class StatusParser;
};
