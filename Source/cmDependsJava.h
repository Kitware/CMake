/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmDependsJava_h
#define cmDependsJava_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <set>
#include <string>

#include "cmDepends.h"

/** \class cmDependsJava
 * \brief Dependency scanner for Java class files.
 */
class cmDependsJava : public cmDepends
{
public:
  /** Checking instances need to know the build directory name and the
      relative path from the build directory to the target file.  */
  cmDependsJava();

  /** Virtual destructor to cleanup subclasses properly.  */
  ~cmDependsJava() override;

  cmDependsJava(cmDependsJava const&) = delete;
  cmDependsJava& operator=(cmDependsJava const&) = delete;

protected:
  // Implement writing/checking methods required by superclass.
  bool WriteDependencies(const std::set<std::string>& sources,
                         const std::string& file, std::ostream& makeDepends,
                         std::ostream& internalDepends) override;
  bool CheckDependencies(std::istream& internalDepends,
                         const std::string& internalDependsFileName,
                         DependencyMap& validDeps) override;
};

#endif
