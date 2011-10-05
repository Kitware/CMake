
#ifndef CLASS_B_H
#define CLASS_B_H

class classA;

class classB
{
public:
  classB();

  classA* a() const;
};

#endif