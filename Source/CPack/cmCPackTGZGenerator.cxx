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
#include <cmzlib/zutil.h>
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

static const size_t cmCPackTGZ_Data_BlockSize = 16384;

//----------------------------------------------------------------------
class cmCPackTGZ_Data
{
public:
  cmCPackTGZ_Data(cmCPackTGZGenerator* gen) :
    OutputStream(0), Generator(gen),
    m_CompressionLevel(Z_DEFAULT_COMPRESSION) {}
  std::ostream* OutputStream;
  cmCPackTGZGenerator* Generator;
  char m_CompressedBuffer[cmCPackTGZ_Data_BlockSize];
  int m_CompressionLevel;
  z_stream m_ZLibStream;
  uLong m_CRC;
};

//----------------------------------------------------------------------
extern "C" {
  int cmCPackTGZ_Data_Open(void *client_data, const char* name, int oflags,
    mode_t mode);
  ssize_t cmCPackTGZ_Data_Write(void *client_data, void *buff, size_t n);
  int cmCPackTGZ_Data_Close(void *client_data);
}


//----------------------------------------------------------------------
int cmCPackTGZ_Data_Open(void *client_data, const char* pathname,
  int, mode_t)
{
  cmCPackTGZ_Data *mydata = (cmCPackTGZ_Data*)client_data;

  mydata->m_ZLibStream.zalloc = Z_NULL;
  mydata->m_ZLibStream.zfree = Z_NULL;
  mydata->m_ZLibStream.opaque = Z_NULL;
  int strategy = Z_DEFAULT_STRATEGY;
  if ( deflateInit2(&mydata->m_ZLibStream, mydata->m_CompressionLevel,
      Z_DEFLATED, -MAX_WBITS, DEF_MEM_LEVEL, strategy) != Z_OK )
    {
    return -1;
    }

  cmGeneratedFileStream* gf = new cmGeneratedFileStream;
  // Open binary
  gf->Open(pathname, false, true);
  mydata->OutputStream = gf;
  if ( !*mydata->OutputStream )
    {
    return -1;
    }
  
  if ( !cmCPackTGZGeneratorForward::GenerateHeader(mydata->Generator,gf))
    {
    return -1;
    }

  mydata->m_CRC = crc32(0L, Z_NULL, 0);

  return 0;
}

//----------------------------------------------------------------------
ssize_t cmCPackTGZ_Data_Write(void *client_data, void *buff, size_t n)
{
  cmCPackTGZ_Data *mydata = (cmCPackTGZ_Data*)client_data;

  mydata->m_ZLibStream.avail_in = n;
  mydata->m_ZLibStream.next_in  = reinterpret_cast<Bytef*>(buff);

  do {
    mydata->m_ZLibStream.avail_out = cmCPackTGZ_Data_BlockSize;
    mydata->m_ZLibStream.next_out
      = reinterpret_cast<Bytef*>(mydata->m_CompressedBuffer);
    // no bad return value
    int ret = deflate(&mydata->m_ZLibStream, (n?Z_NO_FLUSH:Z_FINISH));
    if(ret == Z_STREAM_ERROR)
      {
      return 0;
      }

    size_t compressedSize
      = cmCPackTGZ_Data_BlockSize - mydata->m_ZLibStream.avail_out;

    mydata->OutputStream->write(
      reinterpret_cast<const char*>(mydata->m_CompressedBuffer),
      compressedSize);
  } while ( mydata->m_ZLibStream.avail_out == 0 );

  if ( !*mydata->OutputStream )
    {
    return 0;
    }
  if ( n )
    {
    mydata->m_CRC = crc32(mydata->m_CRC, reinterpret_cast<Bytef *>(buff), n);
    }
  return n;
}

//----------------------------------------------------------------------
int cmCPackTGZ_Data_Close(void *client_data)
{
  cmCPackTGZ_Data *mydata = (cmCPackTGZ_Data*)client_data;

  cmCPackTGZ_Data_Write(client_data, 0, 0);

  char buffer[8];
  int n;
  uLong x = mydata->m_CRC;
  for (n = 0; n < 4; n++) {
    buffer[n] = (int)(x & 0xff);
    x >>= 8;
  }
  x = mydata->m_ZLibStream.total_in;
  for (n = 0; n < 4; n++) {
    buffer[n+4] = (int)(x & 0xff);
    x >>= 8;
  }

  mydata->OutputStream->write(buffer, 8);
  (void)deflateEnd(&mydata->m_ZLibStream);
  delete mydata->OutputStream;
  mydata->OutputStream = 0;
  return (0);
}

//----------------------------------------------------------------------
int cmCPackTGZGenerator::CompressFiles(const char* outFileName,
  const char* toplevel, const std::vector<std::string>& files)
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
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem with tar_open(): "
      << strerror(errno) << std::endl);
    return 0;
    }

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
int cmCPackTGZGenerator::GenerateHeader(std::ostream* os)
{
  const int gz_magic[2] = {0x1f, 0x8b}; /* gzip magic header */
  char header[11];
  sprintf(header, "%c%c%c%c%c%c%c%c%c%c", gz_magic[0], gz_magic[1],
    Z_DEFLATED, 0 /*flags*/, 0,0,0,0 /*time*/, 0 /*xflags*/, OS_CODE);
  os->write(header, 10);
  return 1;
}
