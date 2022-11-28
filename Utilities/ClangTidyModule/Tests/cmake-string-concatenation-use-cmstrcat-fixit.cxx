#include <string>

template <typename... Args>
std::string cmStrCat(Args&&... args)
{
  return "";
}

std::string a = "This is a string variable";
std::string b = " and this is a string variable";
std::string concat;

// Correction needed
void test1()
{
  concat = cmStrCat(a, b);
  concat = cmStrCat(a, " and this is a string literal");
  concat = cmStrCat(a, 'O');
  concat = cmStrCat("This is a string literal", b);
  concat = cmStrCat('O', a);
  concat = cmStrCat(a, " and this is a string literal", 'O', b);

  concat = cmStrCat(concat, b);
  concat = cmStrCat(concat, " and this is a string literal");
  concat = cmStrCat(concat, 'o');
  concat = cmStrCat(concat, b, " and this is a string literal ", 'o', b);
}

// No correction needed
void test2()
{
  a = b;
  a = "This is a string literal";
  a = 'X';
  cmStrCat(a, b);
}
