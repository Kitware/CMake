#ifndef cmXCode21Object_h
#define cmXCode21Object_h

#include "cmXCodeObject.h"

class cmXCode21Object : public cmXCodeObject
{
public:
  cmXCode21Object(PBXType ptype, Type type);
  virtual void PrintComment(std::ostream&);
  static void PrintList(std::vector<cmXCodeObject*> const&,
                        std::ostream& out,
                        PBXType t);
  static void PrintList(std::vector<cmXCodeObject*> const&,
                        std::ostream& out);
};
#endif
