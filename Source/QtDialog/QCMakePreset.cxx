/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "QCMakePreset.h"

bool operator==(const QCMakePreset& lhs, const QCMakePreset& rhs)
{
  return lhs.name == rhs.name && lhs.displayName == rhs.displayName &&
    lhs.description == rhs.description && lhs.generator == rhs.generator &&
    lhs.architecture == rhs.architecture &&
    lhs.setArchitecture == rhs.setArchitecture && lhs.toolset == rhs.toolset &&
    lhs.setToolset == rhs.setToolset && lhs.enabled == rhs.enabled;
}

bool operator!=(const QCMakePreset& lhs, const QCMakePreset& rhs)
{
  return !(lhs == rhs);
}

bool operator<(const QCMakePreset& lhs, const QCMakePreset& rhs)
{
  return lhs.name < rhs.name ||
    (lhs.name == rhs.name &&
     (lhs.displayName < rhs.displayName ||
      (lhs.displayName == rhs.displayName &&
       (lhs.description < rhs.description ||
        (lhs.description == rhs.description &&
         (lhs.generator < rhs.generator ||
          (lhs.generator == rhs.generator &&
           (lhs.architecture < rhs.architecture ||
            (lhs.architecture == rhs.architecture &&
             (lhs.setArchitecture < rhs.setArchitecture ||
              (lhs.setArchitecture == rhs.setArchitecture &&
               (lhs.toolset < rhs.toolset ||
                (lhs.toolset == rhs.toolset &&
                 (lhs.setToolset < rhs.setToolset ||
                  (lhs.setToolset == rhs.setToolset &&
                   (lhs.enabled < rhs.enabled))))))))))))))));
}

bool operator<=(const QCMakePreset& lhs, const QCMakePreset& rhs)
{
  return rhs >= lhs;
}

bool operator>(const QCMakePreset& lhs, const QCMakePreset& rhs)
{
  return rhs < lhs;
}

bool operator>=(const QCMakePreset& lhs, const QCMakePreset& rhs)
{
  return !(lhs < rhs);
}
