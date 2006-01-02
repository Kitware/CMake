/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "cmCPackTGZGenerator.h"

#include "cmake.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmSystemTools.h"
#include "cmMakefile.h"
#include "cmGeneratedFileStream.h"
#include "cmCPackLog.h"

#include <cmsys/SystemTools.hxx>
#include <cmzlib/zlib.h>
#include <libtar/libtar.h>
#include <memory> // auto_ptr
#include <fcntl.h>
#include <errno.h>

//----------------------------------------------------------------------
class cmCPackTGZGeneratorForward
{
  public:
    static int GenerateHeader(cmCPackTGZGenerator* gg, std::ostream* os)
      {
      return gg->GenerateHeader(os);
      }
};

//----------------------------------------------------------------------
cmCPackTGZGenerator::cmCPackTGZGenerator()
{
}

//----------------------------------------------------------------------
cmCPackTGZGenerator::~cmCPackTGZGenerator()
{
}

//----------------------------------------------------------------------
int cmCPackTGZGenerator::ProcessGenerator()
{
  return this->Superclass::ProcessGenerator();
}

//----------------------------------------------------------------------
int cmCPackTGZGenerator::Initialize(const char* name)
{
  return this->Superclass::Initialize(name);
}

//----------------------------------------------------------------------
class cmCPackTGZ_Data
{
public:
  cmCPackTGZ_Data(cmCPackTGZGenerator* gen) :
    Name(0), OutputStream(0), Generator(gen) {}
  const char *Name;
  std::ostream* OutputStream;
  cmCPackTGZGenerator* Generator;
};

//----------------------------------------------------------------------
extern "C" {
  int cmCPackTGZ_Data_Open(void *client_data, const char* name, int oflags, mode_t mode);
  ssize_t cmCPackTGZ_Data_Write(void *client_data, void *buff, size_t n);
  int cmCPackTGZ_Data_Close(void *client_data);
}


//----------------------------------------------------------------------
int cmCPackTGZ_Data_Open(void *client_data, const char* pathname, int, mode_t)
{
  cmCPackTGZ_Data *mydata = (cmCPackTGZ_Data*)client_data;

  cmGeneratedFileStream* gf = new cmGeneratedFileStream(pathname);
  mydata->OutputStream = gf;
  if ( !*mydata->OutputStream )
    {
    return -1;
    }

  gf->SetCompression(true);
  gf->SetCompressionExtraExtension(false);

  if ( !cmCPackTGZGeneratorForward::GenerateHeader(mydata->Generator,gf))
    {
    return -1;
    }
  return 0;
}

//----------------------------------------------------------------------
ssize_t cmCPackTGZ_Data_Write(void *client_data, void *buff, size_t n)
{
  cmCPackTGZ_Data *mydata = (cmCPackTGZ_Data*)client_data;

  mydata->OutputStream->write(reinterpret_cast<const char*>(buff), n);
  if ( !*mydata->OutputStream )
    {
    return 0;
    }
  return n;
}

//----------------------------------------------------------------------
int cmCPackTGZ_Data_Close(void *client_data)
{
  cmCPackTGZ_Data *mydata = (cmCPackTGZ_Data*)client_data;

  delete mydata->OutputStream;
  mydata->OutputStream = 0;
  return (0);
}

//----------------------------------------------------------------------
int cmCPackTGZGenerator::CompressFiles(const char* outFileName, const char* toplevel,
  const std::vector<std::string>& files)
{
  cmCPackLogger(cmCPackLog::LOG_DEBUG, "Toplevel: " << toplevel << std::endl);
  cmCPackTGZ_Data mydata(this);
  TAR *t;
  char buf[TAR_MAXPATHLEN];
  char pathname[TAR_MAXPATHLEN];

  tartype_t gztype = {
    (openfunc_t)cmCPackTGZ_Data_Open,
    (closefunc_t)cmCPackTGZ_Data_Close,
    (readfunc_t)0,
    (writefunc_t)cmCPackTGZ_Data_Write,
    &mydata
  };

  // Ok, this libtar is not const safe. for now use auto_ptr hack
  char* realName = new char[ strlen(outFileName) + 1 ];
  std::auto_ptr<char> realNamePtr(realName);
  strcpy(realName, outFileName);
  int flags = O_WRONLY | O_CREAT;
  if (tar_open(&t, realName,
      &gztype,
      flags, 0644,
      (m_GeneratorVerbose?TAR_VERBOSE:0)
      | 0) == -1)
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem with tar_open(): " << strerror(errno) << std::endl);
    return 0;
    }

  std::vector<std::string>::const_iterator fileIt;
  for ( fileIt = files.begin(); fileIt != files.end(); ++ fileIt )
    {
    strncpy(pathname, fileIt->c_str(), sizeof(pathname));
    pathname[sizeof(pathname)-1] = 0;
    strncpy(buf, pathname, sizeof(buf));
    buf[sizeof(buf)-1] = 0;
    if (tar_append_tree(t, buf, pathname) != 0)
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
        "Problem with tar_append_tree(\"" << buf << "\", \"" << pathname << "\"): "
        << strerror(errno) << std::endl);
      tar_close(t);
      return 0;
      }
    }
  if (tar_append_eof(t) != 0)
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem with tar_append_eof(): " << strerror(errno) << std::endl);
    tar_close(t);
    return 0;
    }

  if (tar_close(t) != 0)
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem with tar_close(): " << strerror(errno) << std::endl);
    return 0;
    }
  return 1;
}

//----------------------------------------------------------------------
int cmCPackTGZGenerator::GenerateHeader(std::ostream* os)
{
  (void)os;
  return 1;
}


