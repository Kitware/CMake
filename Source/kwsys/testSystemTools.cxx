/*============================================================================
  KWSys - Kitware System Library
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "kwsysPrivate.h"

#if defined(_MSC_VER)
# pragma warning (disable:4786)
#endif

#include KWSYS_HEADER(SystemTools.hxx)

// Work-around CMake dependency scanning limitation.  This must
// duplicate the above list of headers.
#if 0
# include "SystemTools.hxx.in"
#endif

// Include with <> instead of "" to avoid getting any in-source copy
// left on disk.
#include <testSystemTools.h>

#include <iostream>
#include <string.h> /* strcmp */
#if defined(_WIN32) && !defined(__CYGWIN__)
# include <io.h> /* _umask (MSVC) / umask (Borland) */
# ifdef _MSC_VER
#  define umask _umask // Note this is still umask on Borland
# endif
#endif
#include <sys/stat.h> /* umask (POSIX), _S_I* constants (Windows) */
// Visual C++ does not define mode_t (note that Borland does, however).
#if defined( _MSC_VER )
typedef unsigned short mode_t;
#endif

//----------------------------------------------------------------------------
static const char* toUnixPaths[][2] =
{
    { "/usr/local/bin/passwd", "/usr/local/bin/passwd" },
    { "/usr/lo cal/bin/pa sswd", "/usr/lo cal/bin/pa sswd" },
    { "/usr/lo\\ cal/bin/pa\\ sswd", "/usr/lo\\ cal/bin/pa\\ sswd" },
    { "c:/usr/local/bin/passwd", "c:/usr/local/bin/passwd" },
    { "c:/usr/lo cal/bin/pa sswd", "c:/usr/lo cal/bin/pa sswd" },
    { "c:/usr/lo\\ cal/bin/pa\\ sswd", "c:/usr/lo\\ cal/bin/pa\\ sswd" },
    { "\\usr\\local\\bin\\passwd", "/usr/local/bin/passwd" },
    { "\\usr\\lo cal\\bin\\pa sswd", "/usr/lo cal/bin/pa sswd" },
    { "\\usr\\lo\\ cal\\bin\\pa\\ sswd", "/usr/lo\\ cal/bin/pa\\ sswd" },
    { "c:\\usr\\local\\bin\\passwd", "c:/usr/local/bin/passwd" },
    { "c:\\usr\\lo cal\\bin\\pa sswd", "c:/usr/lo cal/bin/pa sswd" },
    { "c:\\usr\\lo\\ cal\\bin\\pa\\ sswd", "c:/usr/lo\\ cal/bin/pa\\ sswd" },
    { "\\\\usr\\local\\bin\\passwd", "//usr/local/bin/passwd" },
    { "\\\\usr\\lo cal\\bin\\pa sswd", "//usr/lo cal/bin/pa sswd" },
    { "\\\\usr\\lo\\ cal\\bin\\pa\\ sswd", "//usr/lo\\ cal/bin/pa\\ sswd" },
    {0, 0}
};

static bool CheckConvertToUnixSlashes(std::string input,
                                      std::string output)
{
  std::string result = input;
  kwsys::SystemTools::ConvertToUnixSlashes(result);
  if ( result != output )
    {
    std::cerr
      << "Problem with ConvertToUnixSlashes - input: " << input
      << " output: " << result << " expected: " << output
      << std::endl;
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
static const char* checkEscapeChars[][4] =
{
  { "1 foo 2 bar 2", "12", "\\", "\\1 foo \\2 bar \\2"},
  { " {} ", "{}", "#", " #{#} "},
  {0, 0, 0, 0}
};

static bool CheckEscapeChars(std::string input,
                             const char *chars_to_escape,
                             char escape_char,
                             std::string output)
{
  std::string result = kwsys::SystemTools::EscapeChars(
    input.c_str(), chars_to_escape, escape_char);
  if (result != output)
    {
    std::cerr
      << "Problem with CheckEscapeChars - input: " << input
      << " output: " << result << " expected: " << output
      << std::endl;
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
static bool CheckFileOperations()
{
  bool res = true;
  const std::string testNonExistingFile(TEST_SYSTEMTOOLS_SOURCE_DIR
    "/testSystemToolsNonExistingFile");
  const std::string testDotFile(TEST_SYSTEMTOOLS_SOURCE_DIR
    "/.");
  const std::string testBinFile(TEST_SYSTEMTOOLS_SOURCE_DIR
    "/testSystemTools.bin");
  const std::string testTxtFile(TEST_SYSTEMTOOLS_SOURCE_DIR
    "/testSystemTools.cxx");
  const std::string testNewDir(TEST_SYSTEMTOOLS_BINARY_DIR
    "/testSystemToolsNewDir");
  const std::string testNewFile(testNewDir + "/testNewFile.txt");

  if (kwsys::SystemTools::DetectFileType(testNonExistingFile.c_str()) !=
      kwsys::SystemTools::FileTypeUnknown)
    {
    std::cerr
      << "Problem with DetectFileType - failed to detect type of: "
      << testNonExistingFile << std::endl;
    res = false;
    }

  if (kwsys::SystemTools::DetectFileType(testDotFile.c_str()) !=
      kwsys::SystemTools::FileTypeUnknown)
    {
    std::cerr
      << "Problem with DetectFileType - failed to detect type of: "
      << testDotFile << std::endl;
    res = false;
    }

  if (kwsys::SystemTools::DetectFileType(testBinFile.c_str()) !=
      kwsys::SystemTools::FileTypeBinary)
    {
    std::cerr
      << "Problem with DetectFileType - failed to detect type of: "
      << testBinFile << std::endl;
    res = false;
    }

  if (kwsys::SystemTools::DetectFileType(testTxtFile.c_str()) !=
      kwsys::SystemTools::FileTypeText)
    {
    std::cerr
      << "Problem with DetectFileType - failed to detect type of: "
      << testTxtFile << std::endl;
    res = false;
    }

  if (kwsys::SystemTools::FileLength(testBinFile) != 766)
    {
    std::cerr
      << "Problem with FileLength - incorrect length for: "
      << testBinFile << std::endl;
    res = false;
    }

  if (!kwsys::SystemTools::MakeDirectory(testNewDir))
    {
    std::cerr
      << "Problem with MakeDirectory for: "
      << testNewDir << std::endl;
    res = false;
    }

  if (!kwsys::SystemTools::Touch(testNewFile.c_str(), true))
    {
    std::cerr
      << "Problem with Touch for: "
      << testNewFile << std::endl;
    res = false;
    }

  // Reset umask
#if defined(_WIN32) && !defined(__CYGWIN__)
  // NOTE:  Windows doesn't support toggling _S_IREAD.
  mode_t fullMask = _S_IWRITE;
#else
  // On a normal POSIX platform, we can toggle all permissions.
  mode_t fullMask = S_IRWXU | S_IRWXG | S_IRWXO;
#endif
  mode_t orig_umask = umask(fullMask);

  // Test file permissions without umask
  mode_t origPerm, thisPerm;
  if (!kwsys::SystemTools::GetPermissions(testNewFile, origPerm))
    {
    std::cerr
      << "Problem with GetPermissions (1) for: "
      << testNewFile << std::endl;
    res = false;
    }

  if (!kwsys::SystemTools::SetPermissions(testNewFile, 0))
    {
    std::cerr
      << "Problem with SetPermissions (1) for: "
      << testNewFile << std::endl;
    res = false;
    }

  if (!kwsys::SystemTools::GetPermissions(testNewFile, thisPerm))
    {
    std::cerr
      << "Problem with GetPermissions (2) for: "
      << testNewFile << std::endl;
    res = false;
    }

  if ((thisPerm & fullMask) != 0)
    {
    std::cerr
      << "SetPermissions failed to set permissions (1) for: "
      << testNewFile << ": actual = " << thisPerm << "; expected = "
      << 0 << std::endl;
    res = false;
    }

  // While we're at it, check proper TestFileAccess functionality.
  if (kwsys::SystemTools::TestFileAccess(testNewFile,
                                         kwsys::TEST_FILE_WRITE))
    {
    std::cerr
      << "TestFileAccess incorrectly indicated that this is a writable file:"
      << testNewFile << std::endl;
    res = false;
    }

  if (!kwsys::SystemTools::TestFileAccess(testNewFile,
                                          kwsys::TEST_FILE_OK))
    {
    std::cerr
      << "TestFileAccess incorrectly indicated that this file does not exist:"
      << testNewFile << std::endl;
    res = false;
    }

  // Test restoring/setting full permissions.
  if (!kwsys::SystemTools::SetPermissions(testNewFile, fullMask))
    {
    std::cerr
      << "Problem with SetPermissions (2) for: "
      << testNewFile << std::endl;
    res = false;
    }

  if (!kwsys::SystemTools::GetPermissions(testNewFile, thisPerm))
    {
    std::cerr
      << "Problem with GetPermissions (3) for: "
      << testNewFile << std::endl;
    res = false;
    }

  if ((thisPerm & fullMask) != fullMask)
    {
    std::cerr
      << "SetPermissions failed to set permissions (2) for: "
      << testNewFile << ": actual = " << thisPerm << "; expected = "
      << fullMask << std::endl;
    res = false;
    }

  // Test setting file permissions while honoring umask
  if (!kwsys::SystemTools::SetPermissions(testNewFile, fullMask, true))
    {
    std::cerr
      << "Problem with SetPermissions (3) for: "
      << testNewFile << std::endl;
    res = false;
    }

  if (!kwsys::SystemTools::GetPermissions(testNewFile, thisPerm))
    {
    std::cerr
      << "Problem with GetPermissions (4) for: "
      << testNewFile << std::endl;
    res = false;
    }

  if ((thisPerm & fullMask) != 0)
    {
    std::cerr
      << "SetPermissions failed to honor umask for: "
      << testNewFile << ": actual = " << thisPerm << "; expected = "
      << 0 << std::endl;
    res = false;
    }

  // Restore umask
  umask(orig_umask);

  // Restore file permissions
  if (!kwsys::SystemTools::SetPermissions(testNewFile, origPerm))
    {
    std::cerr
      << "Problem with SetPermissions (4) for: "
      << testNewFile << std::endl;
    res = false;
    }

  // Remove the test file
  if (!kwsys::SystemTools::RemoveFile(testNewFile))
    {
    std::cerr
      << "Problem with RemoveFile: "
      << testNewFile << std::endl;
    res = false;
    }

  std::string const testFileMissing(testNewDir + "/testMissingFile.txt");
  if (!kwsys::SystemTools::RemoveFile(testFileMissing))
    {
    std::string const& msg = kwsys::SystemTools::GetLastSystemError();
    std::cerr <<
      "RemoveFile(\"" << testFileMissing << "\") failed: " << msg << "\n";
    res = false;
    }

  std::string const testFileMissingDir(testNewDir + "/missing/file.txt");
  if (!kwsys::SystemTools::RemoveFile(testFileMissingDir))
    {
    std::string const& msg = kwsys::SystemTools::GetLastSystemError();
    std::cerr <<
      "RemoveFile(\"" << testFileMissingDir << "\") failed: " << msg << "\n";
    res = false;
    }

  kwsys::SystemTools::Touch(testNewFile.c_str(), true);
  if (!kwsys::SystemTools::RemoveADirectory(testNewDir))
    {
    std::cerr
      << "Problem with RemoveADirectory for: "
      << testNewDir << std::endl;
    res = false;
    }

#ifdef KWSYS_TEST_SYSTEMTOOLS_LONG_PATHS
  // Perform the same file and directory creation and deletion tests but
  // with paths > 256 characters in length.

  const std::string testNewLongDir(
    TEST_SYSTEMTOOLS_BINARY_DIR "/"
    "012345678901234567890123456789012345678901234567890123456789"
    "012345678901234567890123456789012345678901234567890123456789"
    "012345678901234567890123456789012345678901234567890123456789"
    "012345678901234567890123456789012345678901234567890123456789"
    "01234567890123");
  const std::string testNewLongFile(testNewLongDir + "/"
    "012345678901234567890123456789012345678901234567890123456789"
    "012345678901234567890123456789012345678901234567890123456789"
    "012345678901234567890123456789012345678901234567890123456789"
    "012345678901234567890123456789012345678901234567890123456789"
    "0123456789.txt");

  if (!kwsys::SystemTools::MakeDirectory(testNewLongDir))
    {
    std::cerr
      << "Problem with MakeDirectory for: "
      << testNewLongDir << std::endl;
    res = false;
    }

  if (!kwsys::SystemTools::Touch(testNewLongFile.c_str(), true))
    {
    std::cerr
      << "Problem with Touch for: "
      << testNewLongFile << std::endl;
    res = false;
    }

  if (!kwsys::SystemTools::RemoveFile(testNewLongFile))
    {
    std::cerr
      << "Problem with RemoveFile: "
      << testNewLongFile << std::endl;
    res = false;
    }

  kwsys::SystemTools::Touch(testNewLongFile.c_str(), true);
  if (!kwsys::SystemTools::RemoveADirectory(testNewLongDir))
    {
    std::cerr
      << "Problem with RemoveADirectory for: "
      << testNewLongDir << std::endl;
    res = false;
    }
#endif

  return res;
}

//----------------------------------------------------------------------------
static bool CheckStringOperations()
{
  bool res = true;

  std::string test = "mary had a little lamb.";
  if (kwsys::SystemTools::CapitalizedWords(test) != "Mary Had A Little Lamb.")
    {
    std::cerr
      << "Problem with CapitalizedWords "
      << '"' << test << '"' << std::endl;
    res = false;
    }

  test = "Mary Had A Little Lamb.";
  if (kwsys::SystemTools::UnCapitalizedWords(test) !=
      "mary had a little lamb.")
    {
    std::cerr
      << "Problem with UnCapitalizedWords "
      << '"' << test << '"' << std::endl;
    res = false;
    }

  test = "MaryHadTheLittleLamb.";
  if (kwsys::SystemTools::AddSpaceBetweenCapitalizedWords(test) !=
      "Mary Had The Little Lamb.")
    {
    std::cerr
      << "Problem with AddSpaceBetweenCapitalizedWords "
      << '"' << test << '"' << std::endl;
    res = false;
    }

  char * cres =
    kwsys::SystemTools::AppendStrings("Mary Had A"," Little Lamb.");
  if (strcmp(cres,"Mary Had A Little Lamb."))
    {
    std::cerr
      << "Problem with AppendStrings "
      << "\"Mary Had A\" \" Little Lamb.\"" << std::endl;
    res = false;
    }
  delete [] cres;

  cres =
    kwsys::SystemTools::AppendStrings("Mary Had"," A ","Little Lamb.");
  if (strcmp(cres,"Mary Had A Little Lamb."))
    {
    std::cerr
      << "Problem with AppendStrings "
      << "\"Mary Had\" \" A \" \"Little Lamb.\"" << std::endl;
    res = false;
    }
  delete [] cres;

  if (kwsys::SystemTools::CountChar("Mary Had A Little Lamb.",'a') != 3)
    {
    std::cerr
      << "Problem with CountChar "
      << "\"Mary Had A Little Lamb.\"" << std::endl;
    res = false;
    }

  cres =
    kwsys::SystemTools::RemoveChars("Mary Had A Little Lamb.","aeiou");
  if (strcmp(cres,"Mry Hd A Lttl Lmb."))
    {
    std::cerr
      << "Problem with RemoveChars "
      << "\"Mary Had A Little Lamb.\"" << std::endl;
    res = false;
    }
  delete [] cres;

  cres =
    kwsys::SystemTools::RemoveCharsButUpperHex("Mary Had A Little Lamb.");
  if (strcmp(cres,"A"))
    {
    std::cerr
      << "Problem with RemoveCharsButUpperHex "
      << "\"Mary Had A Little Lamb.\"" << std::endl;
    res = false;
    }
  delete [] cres;

  char *cres2 = new char [strlen("Mary Had A Little Lamb.")+1];
  strcpy(cres2,"Mary Had A Little Lamb.");
  kwsys::SystemTools::ReplaceChars(cres2,"aeiou",'X');
  if (strcmp(cres2,"MXry HXd A LXttlX LXmb."))
    {
    std::cerr
      << "Problem with ReplaceChars "
      << "\"Mary Had A Little Lamb.\"" << std::endl;
    res = false;
    }
  delete [] cres2;

  if (!kwsys::SystemTools::StringStartsWith("Mary Had A Little Lamb.",
                                            "Mary "))
    {
    std::cerr
      << "Problem with StringStartsWith "
      << "\"Mary Had A Little Lamb.\"" << std::endl;
    res = false;
    }

  if (!kwsys::SystemTools::StringEndsWith("Mary Had A Little Lamb.",
                                          " Lamb."))
    {
    std::cerr
      << "Problem with StringEndsWith "
      << "\"Mary Had A Little Lamb.\"" << std::endl;
    res = false;
    }

  cres = kwsys::SystemTools::DuplicateString("Mary Had A Little Lamb.");
  if (strcmp(cres,"Mary Had A Little Lamb."))
    {
    std::cerr
      << "Problem with DuplicateString "
      << "\"Mary Had A Little Lamb.\"" << std::endl;
    res = false;
    }
  delete [] cres;

  test = "Mary Had A Little Lamb.";
  if (kwsys::SystemTools::CropString(test,13) !=
      "Mary ...Lamb.")
    {
    std::cerr
      << "Problem with CropString "
      << "\"Mary Had A Little Lamb.\"" << std::endl;
    res = false;
    }

  std::vector<std::string> lines;
  kwsys::SystemTools::Split("Mary Had A Little Lamb.",lines,' ');
  if (lines[0] != "Mary" || lines[1] != "Had" ||
      lines[2] != "A" || lines[3] != "Little" || lines[4] != "Lamb.")
    {
    std::cerr
      << "Problem with Split "
      << "\"Mary Had A Little Lamb.\"" << std::endl;
    res = false;
    }

#ifdef _WIN32
  if (kwsys::SystemTools::ConvertToWindowsExtendedPath
      ("L:\\Local Mojo\\Hex Power Pack\\Iffy Voodoo") !=
      L"\\\\?\\L:\\Local Mojo\\Hex Power Pack\\Iffy Voodoo")
    {
    std::cerr
      << "Problem with ConvertToWindowsExtendedPath "
      << "\"L:\\Local Mojo\\Hex Power Pack\\Iffy Voodoo\""
      << std::endl;
    res = false;
    }

  if (kwsys::SystemTools::ConvertToWindowsExtendedPath
      ("L:/Local Mojo/Hex Power Pack/Iffy Voodoo") !=
      L"\\\\?\\L:\\Local Mojo\\Hex Power Pack\\Iffy Voodoo")
    {
    std::cerr
      << "Problem with ConvertToWindowsExtendedPath "
      << "\"L:/Local Mojo/Hex Power Pack/Iffy Voodoo\""
      << std::endl;
    res = false;
    }

  if (kwsys::SystemTools::ConvertToWindowsExtendedPath
      ("\\\\Foo\\Local Mojo\\Hex Power Pack\\Iffy Voodoo") !=
      L"\\\\?\\UNC\\Foo\\Local Mojo\\Hex Power Pack\\Iffy Voodoo")
    {
    std::cerr
      << "Problem with ConvertToWindowsExtendedPath "
      << "\"\\\\Foo\\Local Mojo\\Hex Power Pack\\Iffy Voodoo\""
      << std::endl;
    res = false;
    }

  if (kwsys::SystemTools::ConvertToWindowsExtendedPath
      ("//Foo/Local Mojo/Hex Power Pack/Iffy Voodoo") !=
      L"\\\\?\\UNC\\Foo\\Local Mojo\\Hex Power Pack\\Iffy Voodoo")
    {
    std::cerr
      << "Problem with ConvertToWindowsExtendedPath "
      << "\"//Foo/Local Mojo/Hex Power Pack/Iffy Voodoo\""
      << std::endl;
    res = false;
    }

  if (kwsys::SystemTools::ConvertToWindowsExtendedPath("//") !=
      L"//")
    {
    std::cerr
      << "Problem with ConvertToWindowsExtendedPath "
      << "\"//\""
      << std::endl;
    res = false;
    }

  if (kwsys::SystemTools::ConvertToWindowsExtendedPath("\\\\.\\") !=
      L"\\\\.\\")
    {
    std::cerr
      << "Problem with ConvertToWindowsExtendedPath "
      << "\"\\\\.\\\""
      << std::endl;
    res = false;
    }

  if (kwsys::SystemTools::ConvertToWindowsExtendedPath("\\\\.\\X") !=
      L"\\\\.\\X")
    {
    std::cerr
      << "Problem with ConvertToWindowsExtendedPath "
      << "\"\\\\.\\X\""
      << std::endl;
    res = false;
    }

  if (kwsys::SystemTools::ConvertToWindowsExtendedPath("\\\\.\\X:") !=
      L"\\\\?\\X:")
    {
    std::cerr
      << "Problem with ConvertToWindowsExtendedPath "
      << "\"\\\\.\\X:\""
      << std::endl;
    res = false;
    }

  if (kwsys::SystemTools::ConvertToWindowsExtendedPath("\\\\.\\X:\\") !=
      L"\\\\?\\X:\\")
    {
    std::cerr
      << "Problem with ConvertToWindowsExtendedPath "
      << "\"\\\\.\\X:\\\""
      << std::endl;
    res = false;
    }

  if (kwsys::SystemTools::ConvertToWindowsExtendedPath("NUL") !=
      L"\\\\.\\NUL")
    {
    std::cerr
      << "Problem with ConvertToWindowsExtendedPath "
      << "\"NUL\""
      << std::endl;
    res = false;
    }

#endif

  if (kwsys::SystemTools::ConvertToWindowsOutputPath
      ("L://Local Mojo/Hex Power Pack/Iffy Voodoo") !=
      "\"L:\\Local Mojo\\Hex Power Pack\\Iffy Voodoo\"")
    {
    std::cerr
      << "Problem with ConvertToWindowsOutputPath "
      << "\"L://Local Mojo/Hex Power Pack/Iffy Voodoo\""
      << std::endl;
    res = false;
    }

  if (kwsys::SystemTools::ConvertToWindowsOutputPath
      ("//grayson/Local Mojo/Hex Power Pack/Iffy Voodoo") !=
      "\"\\\\grayson\\Local Mojo\\Hex Power Pack\\Iffy Voodoo\"")
    {
    std::cerr
      << "Problem with ConvertToWindowsOutputPath "
      << "\"//grayson/Local Mojo/Hex Power Pack/Iffy Voodoo\""
      << std::endl;
    res = false;
    }

  if (kwsys::SystemTools::ConvertToUnixOutputPath
      ("//Local Mojo/Hex Power Pack/Iffy Voodoo") !=
      "//Local\\ Mojo/Hex\\ Power\\ Pack/Iffy\\ Voodoo")
    {
    std::cerr
      << "Problem with ConvertToUnixOutputPath "
      << "\"//Local Mojo/Hex Power Pack/Iffy Voodoo\""
      << std::endl;
    res = false;
    }

  return res;
}

//----------------------------------------------------------------------------

static bool CheckPutEnv(const std::string& env, const char* name, const char* value)
{
  if(!kwsys::SystemTools::PutEnv(env))
    {
    std::cerr << "PutEnv(\"" << env
                    << "\") failed!" << std::endl;
    return false;
    }
  const char* v = kwsys::SystemTools::GetEnv(name);
  v = v? v : "(null)";
  if(strcmp(v, value) != 0)
    {
    std::cerr << "GetEnv(\"" << name << "\") returned \""
                    << v << "\", not \"" << value << "\"!" << std::endl;
    return false;
    }
  return true;
}

static bool CheckUnPutEnv(const char* env, const char* name)
{
  if(!kwsys::SystemTools::UnPutEnv(env))
    {
    std::cerr << "UnPutEnv(\"" << env << "\") failed!"
                    << std::endl;
    return false;
    }
  if(const char* v = kwsys::SystemTools::GetEnv(name))
    {
    std::cerr << "GetEnv(\"" << name << "\") returned \""
                    << v << "\", not (null)!" << std::endl;
    return false;
    }
  return true;
}

static bool CheckEnvironmentOperations()
{
  bool res = true;
  res &= CheckPutEnv("A=B", "A", "B");
  res &= CheckPutEnv("B=C", "B", "C");
  res &= CheckPutEnv("C=D", "C", "D");
  res &= CheckPutEnv("D=E", "D", "E");
  res &= CheckUnPutEnv("A", "A");
  res &= CheckUnPutEnv("B=", "B");
  res &= CheckUnPutEnv("C=D", "C");
  /* Leave "D=E" in environment so a memory checker can test for leaks.  */
  return res;
}


static bool CheckRelativePath(
  const std::string& local,
  const std::string& remote,
  const std::string& expected)
{
  std::string result = kwsys::SystemTools::RelativePath(local, remote);
  if(expected != result)
    {
    std::cerr << "RelativePath(" << local << ", " << remote
      << ")  yielded " << result << " instead of " << expected << std::endl;
    return false;
    }
  return true;
}

static bool CheckRelativePaths()
{
  bool res = true;
  res &= CheckRelativePath("/usr/share", "/bin/bash", "../../bin/bash");
  res &= CheckRelativePath("/usr/./share/", "/bin/bash", "../../bin/bash");
  res &= CheckRelativePath("/usr//share/", "/bin/bash", "../../bin/bash");
  res &= CheckRelativePath("/usr/share/../bin/", "/bin/bash", "../../bin/bash");
  res &= CheckRelativePath("/usr/share", "/usr/share//bin", "bin");
  return res;
}

static bool CheckCollapsePath(
  const std::string& path,
  const std::string& expected)
{
  std::string result = kwsys::SystemTools::CollapseFullPath(path);
  if(expected != result)
    {
    std::cerr << "CollapseFullPath(" << path
      << ")  yielded " << result << " instead of " << expected << std::endl;
    return false;
    }
  return true;
}

static bool CheckCollapsePath()
{
  bool res = true;
  res &= CheckCollapsePath("/usr/share/*", "/usr/share/*");
  res &= CheckCollapsePath("C:/Windows/*", "C:/Windows/*");
  return res;
}

//----------------------------------------------------------------------------
int testSystemTools(int, char*[])
{
  bool res = true;

  int cc;
  for ( cc = 0; toUnixPaths[cc][0]; cc ++ )
    {
    res &= CheckConvertToUnixSlashes(toUnixPaths[cc][0], toUnixPaths[cc][1]);
    }

  // Special check for ~
  std::string output;
  if(kwsys::SystemTools::GetEnv("HOME", output))
    {
    output += "/foo bar/lala";
    res &= CheckConvertToUnixSlashes("~/foo bar/lala", output);
    }

  for (cc = 0; checkEscapeChars[cc][0]; cc ++ )
    {
    res &= CheckEscapeChars(checkEscapeChars[cc][0], checkEscapeChars[cc][1],
                            *checkEscapeChars[cc][2], checkEscapeChars[cc][3]);
    }

  res &= CheckFileOperations();

  res &= CheckStringOperations();

  res &= CheckEnvironmentOperations();

  res &= CheckRelativePaths();

  res &= CheckCollapsePath();

  return res ? 0 : 1;
}
