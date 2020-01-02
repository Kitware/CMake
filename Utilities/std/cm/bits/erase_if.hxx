// -*-c++-*-
// vim: set ft=cpp:

/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#ifndef cm_bits_erase_if_hxx
#define cm_bits_erase_if_hxx

namespace cm {
namespace internals {

template <typename Container, typename Predicate>
void erase_if(Container& cont, Predicate pred)
{
  for (typename Container::iterator iter = cont.begin(), last = cont.end();
       iter != last;) {
    if (pred(*iter)) {
      iter = cont.erase(iter);
    } else {
      ++iter;
    }
  }
}

} // namespace internals
} // namespace cm

#endif
