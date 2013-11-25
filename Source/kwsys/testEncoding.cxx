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

#include KWSYS_HEADER(Encoding.hxx)
#include KWSYS_HEADER(ios/iostream)

#include <locale.h>

// Work-around CMake dependency scanning limitation.  This must
// duplicate the above list of headers.
#if 0
# include "Encoding.hxx.in"
# include "kwsys_ios_iostream.h.in"
#endif

//----------------------------------------------------------------------------
static const unsigned char helloWorldStrings[][32] =
{
  // English
  {'H','e','l','l','o',' ','W','o','r','l','d',0},
  // Japanese
  {0xE3, 0x81, 0x93, 0xE3, 0x82, 0x93, 0xE3, 0x81, 0xAB, 0xE3,
   0x81, 0xA1, 0xE3, 0x81, 0xAF, 0xE4, 0xB8, 0x96, 0xE7, 0x95,
   0x8C, 0},
   // Arabic
  {0xD9, 0x85, 0xD8, 0xB1, 0xD8, 0xAD, 0xD8, 0xA8, 0xD8, 0xA7,
   0x20, 0xD8, 0xA7, 0xD9, 0x84, 0xD8, 0xB9, 0xD8, 0xA7, 0xD9,
   0x84, 0xD9, 0x85, 0},
  // Yiddish
  {0xD7, 0x94, 0xD7, 0xA2, 0xD7, 0x9C, 0xD7, 0x90, 0x20, 0xD7,
   0x95, 0xD7, 0x95, 0xD7, 0xA2, 0xD7, 0x9C, 0xD7, 0x98, 0},
  // Russian
  {0xD0, 0xBF, 0xD1, 0x80, 0xD0, 0xB8, 0xD0, 0xB2, 0xD0, 0xB5,
   0xD1, 0x82, 0x20, 0xD0, 0xBC, 0xD0, 0xB8, 0xD1, 0x80, 0},
  // Latin
  {0x4D, 0x75, 0x6E, 0x64, 0x75, 0x73, 0x20, 0x73, 0x61, 0x6C,
   0x76, 0x65, 0},
  // Swahili
  {0x68, 0x75, 0x6A, 0x61, 0x6D, 0x62, 0x6F, 0x20, 0x44, 0x75,
   0x6E, 0x69, 0x61, 0},
  // Icelandic
  {0x48, 0x61, 0x6C, 0x6C, 0xC3, 0xB3, 0x20, 0x68, 0x65, 0x69,
   0x6D, 0x75, 0x72, 0},
  {0}
};

//----------------------------------------------------------------------------
static int testHelloWorldEncoding()
{
  int ret = 0;
  for(int i=0; helloWorldStrings[i][0] != 0; i++)
    {
    std::string str = reinterpret_cast<const char*>(helloWorldStrings[i]);
    std::cout << str << std::endl;
    std::wstring wstr = kwsys::Encoding::ToWide(str);
    std::string str2 = kwsys::Encoding::ToNarrow(wstr);
    if(!wstr.empty() && str != str2)
      {
      std::cout << "converted string was different: " << str2 << std::endl;
      ret++;
      }
    }
  return ret;
}

static int testRobustEncoding()
{
  // test that the conversion functions handle invalid
  // unicode correctly/gracefully

  int ret = 0;
  char cstr[] = {(char)-1, 0};
  // this conversion could fail
  std::wstring wstr = kwsys::Encoding::ToWide(cstr);

  wstr = kwsys::Encoding::ToWide(NULL);
  if(wstr != L"")
    {
    const wchar_t* wcstr = wstr.c_str();
    std::cout << "ToWide(NULL) returned";
    for(size_t i=0; i<wstr.size(); i++)
      {
      std::cout << " " << std::hex << (int)wcstr[i];
      }
    std::cout << std::endl;
    ret++;
    }
  wstr = kwsys::Encoding::ToWide("");
  if(wstr != L"")
    {
    const wchar_t* wcstr = wstr.c_str();
    std::cout << "ToWide(\"\") returned";
    for(size_t i=0; i<wstr.size(); i++)
      {
      std::cout << " " << std::hex << (int)wcstr[i];
      }
    std::cout << std::endl;
    ret++;
    }

#ifdef WIN32
  // 16 bit wchar_t - we make an invalid surrogate pair
  wchar_t cwstr[] = {0xD801, 0xDA00, 0};
  // this conversion could fail
  std::string win_str = kwsys::Encoding::ToNarrow(cwstr);
#endif

  std::string str = kwsys::Encoding::ToNarrow(NULL);
  if(str != "")
    {
    std::cout << "ToNarrow(NULL) returned " << str << std::endl;
    ret++;
    }

  str = kwsys::Encoding::ToNarrow(L"");
  if(wstr != L"")
    {
    std::cout << "ToNarrow(\"\") returned " << str << std::endl;
    ret++;
    }

  return ret;
}


//----------------------------------------------------------------------------
int testEncoding(int, char*[])
{
  const char* loc = setlocale(LC_ALL, "");
  if(loc)
    {
    std::cout << "Locale: " << loc << std::endl;
    }
  else
    {
    std::cout << "Locale: None" << std::endl;
    }

  int ret = 0;

  ret |= testHelloWorldEncoding();
  ret |= testRobustEncoding();

  return ret;
}
