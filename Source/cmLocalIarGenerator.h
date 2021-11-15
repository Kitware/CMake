/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmLocalIarGenerator_h
#define cmLocalIarGenerator_h

#include "cmLocalGenerator.h"

class cmGeneratedFileStream;

/** \class cmLocalIarGenerator
 * \brief Write Green Hills MULTI project files.
 *
 * cmLocalIarGenerator produces a set of .ewp
 * file for each target in its mirrored directory.
 */
class cmLocalIarGenerator : public cmLocalGenerator
{
public:
  cmLocalIarGenerator(cmGlobalGenerator* gg, cmMakefile* mf);

  ~cmLocalIarGenerator() override;

  /**
   * Generate the makefile for this directory.
   */
  virtual void Generate();
};

#endif
