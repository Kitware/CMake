/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmXCodeScheme_h
#define cmXCodeScheme_h

#include <cmConfigure.h> // IWYU pragma: keep

#include "cmXCodeObject.h"
#include "cmGlobalXCodeGenerator.h"
#include "cmXMLWriter.h"
#include "cmSystemTools.h"

/** \class cmXCodeScheme
 * \brief Write shared schemes for native targets in Xcode project.
 */
class cmXCodeScheme
{
public:
    cmXCodeScheme(cmXCodeObject* xcObj, unsigned int xcVersion);

    void WriteXCodeSharedScheme(const std::string& xcProjDir, const std::string sourceRoot);
private:
    const std::string& targetName;
    const std::string& targetId;
    unsigned int XcodeVersion;

    void WriteXCodeXCScheme(std::ostream& fout, const std::string& xcProjDir);

    void WriteBuildAction(cmXMLWriter& xout, const std::string& xcProjDir);
    void WriteTestAction(cmXMLWriter& xout, std::string configuration);
    void WriteLaunchAction(cmXMLWriter& xout, std::string configuration,
                           const std::string& xcProjDir);
    void WriteProfileAction(cmXMLWriter& xout, std::string configuration);
    void WriteAnalyzeAction(cmXMLWriter& xout, std::string configuration);
    void WriteArchiveAction(cmXMLWriter& xout, std::string configuration);

    std::string WriteVersionString();
};

#endif
