
#include "cmValue.h"

#include <string>

#include <cmext/string_view>

#include "cmStringAlgorithms.h"

std::string cmValue::Empty;

bool cmValue::IsOn(cm::string_view value) noexcept
{
  switch (value.size()) {
    case 1:
      return value[0] == '1' || value[0] == 'Y' || value[0] == 'y';
    case 2:
      return                                    //
        (value[0] == 'O' || value[0] == 'o') && //
        (value[1] == 'N' || value[1] == 'n');
    case 3:
      return                                    //
        (value[0] == 'Y' || value[0] == 'y') && //
        (value[1] == 'E' || value[1] == 'e') && //
        (value[2] == 'S' || value[2] == 's');
    case 4:
      return                                    //
        (value[0] == 'T' || value[0] == 't') && //
        (value[1] == 'R' || value[1] == 'r') && //
        (value[2] == 'U' || value[2] == 'u') && //
        (value[3] == 'E' || value[3] == 'e');
    default:
      break;
  }

  return false;
}

bool cmValue::IsOff(cm::string_view value) noexcept
{
  switch (value.size()) {
    case 0:
      return true;
    case 1:
      return value[0] == '0' || value[0] == 'N' || value[0] == 'n';
    case 2:
      return                                    //
        (value[0] == 'N' || value[0] == 'n') && //
        (value[1] == 'O' || value[1] == 'o');
    case 3:
      return                                    //
        (value[0] == 'O' || value[0] == 'o') && //
        (value[1] == 'F' || value[1] == 'f') && //
        (value[2] == 'F' || value[2] == 'f');
    case 5:
      return                                    //
        (value[0] == 'F' || value[0] == 'f') && //
        (value[1] == 'A' || value[1] == 'a') && //
        (value[2] == 'L' || value[2] == 'l') && //
        (value[3] == 'S' || value[3] == 's') && //
        (value[4] == 'E' || value[4] == 'e');
    case 6:
      return                                    //
        (value[0] == 'I' || value[0] == 'i') && //
        (value[1] == 'G' || value[1] == 'g') && //
        (value[2] == 'N' || value[2] == 'n') && //
        (value[3] == 'O' || value[3] == 'o') && //
        (value[4] == 'R' || value[4] == 'r') && //
        (value[5] == 'E' || value[5] == 'e');
    default:
      break;
  }

  return IsNOTFOUND(value);
}

bool cmValue::IsNOTFOUND(cm::string_view value) noexcept
{
  return (value == "NOTFOUND"_s) || cmHasSuffix(value, "-NOTFOUND"_s);
}

bool cmValue::IsInternallyOn(cm::string_view value) noexcept
{
  return (value.size() == 4) &&             //
    (value[0] == 'I' || value[0] == 'i') && //
    (value[1] == '_') &&                    //
    (value[2] == 'O' || value[2] == 'o') && //
    (value[3] == 'N' || value[3] == 'n');
}

int cmValue::Compare(cmValue value) const noexcept
{
  if (this->Value == nullptr && !value) {
    return 0;
  }
  if (this->Value == nullptr) {
    return -1;
  }
  if (!value) {
    return 1;
  }
  return this->Value->compare(*value);
}

int cmValue::Compare(cm::string_view value) const noexcept
{
  if (this->Value == nullptr && value.data() == nullptr) {
    return 0;
  }
  if (this->Value == nullptr) {
    return -1;
  }
  if (value.data() == nullptr) {
    return 1;
  }
  return cm::string_view(*this->Value).compare(value);
}

std::ostream& operator<<(std::ostream& o, cmValue v)
{
  o << *v;
  return o;
}
