/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmCPackTarCompressGenerator.h"

#include "cmake.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmSystemTools.h"
#include "cmMakefile.h"
#include "cmGeneratedFileStream.h"
#include "cmCPackLog.h"

#include <cmsys/SystemTools.hxx>
#include <cmcompress/cmcompress.h>
#include <libtar/libtar.h>
#include <fcntl.h>
#include <errno.h>

//----------------------------------------------------------------------
class cmCPackTarCompressGeneratorForward
{
public:
  static int GenerateHeader(cmCPackTarCompressGenerator* gg, std::ostream* os)
    {
    return gg->GenerateHeader(os);
    }
};

//----------------------------------------------------------------------
cmCPackTarCompressGenerator::cmCPackTarCompressGenerator()
{
}

//----------------------------------------------------------------------
cmCPackTarCompressGenerator::~cmCPackTarCompressGenerator()
{
}

//----------------------------------------------------------------------
class cmCPackTarCompress_Data
{
public:
  cmCPackTarCompress_Data(cmCPackTarCompressGenerator* gen) :
    OutputStream(0), Generator(gen) {}
  std::ostream* OutputStream;
  cmCPackTarCompressGenerator* Generator;
  cmcompress_stream CMCompressStream;
};

//----------------------------------------------------------------------
extern "C" {
  // For cmTar
  int cmCPackTarCompress_Data_Open(void *client_data, const char* name,
    int oflags, mode_t mode);
  ssize_t cmCPackTarCompress_Data_Write(void *client_data, void *buff,
    size_t n);
  int cmCPackTarCompress_Data_Close(void *client_data);

  // For cmCompress
  int cmCPackTarCompress_Compress_Output(void* cdata, const char* data,
    int len);
}


//----------------------------------------------------------------------
int cmCPackTarCompress_Data_Open(void *client_data, const char* pathname,
  int, mode_t)
{
  cmCPackTarCompress_Data *mydata = (cmCPackTarCompress_Data*)client_data;

  if ( !cmcompress_compress_initialize(&mydata->CMCompressStream) )
    {
    return -1;
    }

  mydata->CMCompressStream.client_data = mydata;
  mydata->CMCompressStream.output_stream = cmCPackTarCompress_Compress_Output;

  cmGeneratedFileStream* gf = new cmGeneratedFileStream;
  // Open binary
  gf->Open(pathname, false, true);
  mydata->OutputStream = gf;
  if ( !*mydata->OutputStream )
    {
    return -1;
    }

  if ( !cmcompress_compress_start(&mydata->CMCompressStream) )
    {
    return -1;
    }


  if ( !cmCPackTarCompressGeneratorForward::GenerateHeader(
      mydata->Generator,gf))
    {
    return -1;
    }

  return 0;
}

//----------------------------------------------------------------------
ssize_t cmCPackTarCompress_Data_Write(void *client_data, void *buff, size_t n)
{
  cmCPackTarCompress_Data *mydata = (cmCPackTarCompress_Data*)client_data;

  if ( !cmcompress_compress(&mydata->CMCompressStream, buff, n) )
    {
    return 0;
    }
  return n;
}

//----------------------------------------------------------------------
int cmCPackTarCompress_Data_Close(void *client_data)
{
  cmCPackTarCompress_Data *mydata = (cmCPackTarCompress_Data*)client_data;

  if ( !cmcompress_compress_finalize(&mydata->CMCompressStream) )
    {
    delete mydata->OutputStream;
    return -1;
    }

  delete mydata->OutputStream;
  mydata->OutputStream = 0;
  return (0);
}

//----------------------------------------------------------------------
int cmCPackTarCompressGenerator::InitializeInternal()
{
  this->SetOptionIfNotSet("CPACK_INCLUDE_TOPLEVEL_DIRECTORY", "1");
  return this->Superclass::InitializeInternal();
}

//----------------------------------------------------------------------
int cmCPackTarCompressGenerator::CompressFiles(const char* outFileName,
  const char* toplevel, const std::vector<std::string>& files)
{
  cmCPackLogger(cmCPackLog::LOG_DEBUG, "Toplevel: "
                << (toplevel ? toplevel : "(NULL)") << std::endl);
  cmCPackTarCompress_Data mydata(this);
  TAR *t;
  char buf[TAR_MAXPATHLEN];
  char pathname[TAR_MAXPATHLEN];

  tartype_t compressType = {
    (openfunc_t)cmCPackTarCompress_Data_Open,
    (closefunc_t)cmCPackTarCompress_Data_Close,
    (readfunc_t)0,
    (writefunc_t)cmCPackTarCompress_Data_Write,
    &mydata
  };

  // This libtar is not const safe. Make a non-const copy of outFileName
  char* realName = new char[ strlen(outFileName) + 1 ];
  strcpy(realName, outFileName);
  int flags = O_WRONLY | O_CREAT;  
  int options = 0;
  if(this->GeneratorVerbose)
    {
    options |= TAR_VERBOSE;
    }
#ifdef __CYGWIN__
  options |= TAR_GNU;
#endif 
  if (tar_open(&t, realName,
      &compressType,
      flags, 0644,
      options) == -1)
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem with tar_open(): "
      << strerror(errno) << std::endl);
    delete [] realName;
    return 0;
    }

  delete [] realName;

  std::vector<std::string>::const_iterator fileIt;
  for ( fileIt = files.begin(); fileIt != files.end(); ++ fileIt )
    {
    std::string rp = cmSystemTools::RelativePath(toplevel, fileIt->c_str());
    strncpy(pathname, fileIt->c_str(), sizeof(pathname));
    pathname[sizeof(pathname)-1] = 0;
    strncpy(buf, rp.c_str(), sizeof(buf));
    buf[sizeof(buf)-1] = 0;
    if (tar_append_tree(t, pathname, buf) != 0)
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
        "Problem with tar_append_tree(\"" << buf << "\", \""
        << pathname << "\"): "
        << strerror(errno) << std::endl);
      tar_close(t);
      return 0;
      }
    }
  if (tar_append_eof(t) != 0)
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem with tar_append_eof(): "
      << strerror(errno) << std::endl);
    tar_close(t);
    return 0;
    }

  if (tar_close(t) != 0)
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem with tar_close(): "
      << strerror(errno) << std::endl);
    return 0;
    }
  return 1;
}

//----------------------------------------------------------------------
int cmCPackTarCompress_Compress_Output(void* client_data,
  const char* data, int data_length)
{
  if(!client_data)
    {
    return 0;
    }
  cmcompress_stream *cstream = static_cast<cmcompress_stream*>(client_data);
  cmCPackTarCompress_Data *mydata
    = static_cast<cmCPackTarCompress_Data*>(cstream->client_data);
  if ( !mydata->OutputStream )
    {
    return 0;
    }
  mydata->OutputStream->write(data, data_length);
  return data_length;
}

//----------------------------------------------------------------------
int cmCPackTarCompressGenerator::GenerateHeader(std::ostream* os)
{
 (void)os;
  return 1;
}
