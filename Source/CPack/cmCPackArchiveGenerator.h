/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmCPackArchiveGenerator_h
#define cmCPackArchiveGenerator_h

#include "cmConfigure.h"

#include "cmArchiveWrite.h"
#include "cmCPackGenerator.h"

#include <iosfwd>
#include <string>

class cmCPackComponent;

/** \class cmCPackArchiveGenerator
 * \brief A generator base for libarchive generation.
 * The generator itself uses the libarchive wrapper
 * \ref cmArchiveWrite.
 *
 */
class cmCPackArchiveGenerator : public cmCPackGenerator
{
public:
  typedef cmCPackGenerator Superclass;

  /**
   * Construct generator
   */
  cmCPackArchiveGenerator(cmArchiveWrite::Compress, std::string const& format);
  ~cmCPackArchiveGenerator() CM_OVERRIDE;
  // Used to add a header to the archive
  virtual int GenerateHeader(std::ostream* os);
  // component support
  bool SupportsComponentInstallation() const CM_OVERRIDE;

protected:
  int InitializeInternal() CM_OVERRIDE;
  /**
   * Add the files belonging to the specified component
   * to the provided (already opened) archive.
   * @param[in,out] archive the archive object
   * @param[in] component the component whose file will be added to archive
   */
  int addOneComponentToArchive(cmArchiveWrite& archive,
                               cmCPackComponent* component);

  /**
   * The main package file method.
   * If component install was required this
   * method will call either PackageComponents or
   * PackageComponentsAllInOne.
   */
  int PackageFiles() CM_OVERRIDE;
  /**
   * The method used to package files when component
   * install is used. This will create one
   * archive for each component group.
   */
  int PackageComponents(bool ignoreGroup);
  /**
   * Special case of component install where all
   * components will be put in a single installer.
   */
  int PackageComponentsAllInOne();
  const char* GetOutputExtension() CM_OVERRIDE = 0;
  cmArchiveWrite::Compress Compress;
  std::string ArchiveFormat;
};

#endif
