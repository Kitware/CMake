/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmCPackArchiveGenerator_h
#define cmCPackArchiveGenerator_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <string>

#include "cmArchiveWrite.h"
#include "cmCPackGenerator.h"

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
  using Superclass = cmCPackGenerator;

  static cmCPackGenerator* Create7ZGenerator();
  static cmCPackGenerator* CreateTBZ2Generator();
  static cmCPackGenerator* CreateTGZGenerator();
  static cmCPackGenerator* CreateTXZGenerator();
  static cmCPackGenerator* CreateTZGenerator();
  static cmCPackGenerator* CreateTZSTGenerator();
  static cmCPackGenerator* CreateZIPGenerator();

  /**
   * Construct generator
   */
  cmCPackArchiveGenerator(cmArchiveWrite::Compress t, std::string format,
                          std::string extension);
  ~cmCPackArchiveGenerator() override;
  // Used to add a header to the archive
  virtual int GenerateHeader(std::ostream* os);
  // component support
  bool SupportsComponentInstallation() const override;

private:
  // get archive component filename
  std::string GetArchiveComponentFileName(const std::string& component,
                                          bool isGroupName);

protected:
  int InitializeInternal() override;
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
  int PackageFiles() override;
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

private:
  const char* GetNameOfClass() override { return "cmCPackArchiveGenerator"; }

  const char* GetOutputExtension() override
  {
    return this->OutputExtension.c_str();
  }

private:
  cmArchiveWrite::Compress Compress;
  std::string ArchiveFormat;
  std::string OutputExtension;
};

#endif
