/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmCryptoHash_h
#define cmCryptoHash_h

#include "cmStandardIncludes.h"

class cmCryptoHash
{
public:
  std::string HashString(const char* input);
  std::string HashFile(const char* file);
protected:
  virtual void Initialize()=0;
  virtual void Append(unsigned char const*, int)=0;
  virtual std::string Finalize()=0;
};

class cmCryptoHashMD5: public cmCryptoHash
{
  struct cmsysMD5_s* MD5;
public:
  cmCryptoHashMD5();
  ~cmCryptoHashMD5();
protected:
  virtual void Initialize();
  virtual void Append(unsigned char const* buf, int sz);
  virtual std::string Finalize();
};

#endif
