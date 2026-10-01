/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

/* Source of the checked-in object files used by the CreateDef test.
   See CreateDef.cmake for the commands used to regenerate them.  */

class Virt
{
public:
  Virt();
  virtual ~Virt();
  virtual int Method();
};
Virt::Virt()
{
}
Virt::~Virt()
{
}
int Virt::Method()
{
  return 1;
}
/* Makes the compiler emit the scalar deleting destructor "??_G".  */
void DeleteOne(Virt* v)
{
  delete v;
}

int writableData = 1;
int bssData;
extern int const constData;
int const constData = 2;
int const* UseConstData()
{
  return &constData;
}

int CodeFunction()
{
  return 3;
}
static int LocalFunction()
{
  return 4;
}
int UseLocalFunction()
{
  return LocalFunction();
}

/* On i386 these lose their leading underscore, and __stdcall loses its
   "@<n>" suffix.  */
extern "C" int CFunction(int a)
{
  return a;
}
extern "C" int cWritableData = 5;
#ifdef _M_IX86
extern "C" int __stdcall StdcallFunction(int a)
{
  return a;
}
#endif
