/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <string>

#include "cmCTestGlobalVC.h"

class cmCTest;

/** \class cmCTestHG
 * \brief Interaction with Mercurial command-line tool
 *
 */
class cmCTestHG : public cmCTestGlobalVC
{
public:
  /** Construct with a CTest instance and update log stream.  */
  cmCTestHG(cmCTest* ctest, std::ostream& log);

  ~cmCTestHG() override;

private:
  std::string GetWorkingRevision();
  bool NoteOldRevision() override;
  bool NoteNewRevision() override;
  bool UpdateImpl() override;

  bool LoadRevisions() override;
  bool LoadModifications() override;

  // Parsing helper classes.
  class IdentifyParser;
  class LogParser;
  class StatusParser;

  friend class IdentifyParser;
  friend class LogParser;
  friend class StatusParser;
};
