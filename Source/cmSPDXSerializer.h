/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <array>
#include <cstddef>
#include <map>
#include <new>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <cm/optional>
#include <cm/type_traits>

#include <cm3p/json/value.h>

#include "cmStringAlgorithms.h"

// Base Class

struct cmSPDXSerializationBase
{
  enum SPDXTypeId
  {
    INVALID = -1,
    NULL_ID = 0,

#define X_SPDX(classtype, enumid, title, camel) enumid,
#include "cmSPDXTypes.def"

    SPDX_TYPE_ID_MAX,
  };

  SPDXTypeId getTypeId() const;

  std::string NodeId;

protected:
  cmSPDXSerializationBase(SPDXTypeId id);
  cmSPDXSerializationBase(SPDXTypeId id, std::string nodeId);

private:
  SPDXTypeId TypeId;
};

// Convenience Classes

struct cmSPDXIdentifierReference : cmSPDXSerializationBase
{
  cmSPDXIdentifierReference();
  cmSPDXIdentifierReference(cmSPDXSerializationBase const& ref);
  cmSPDXIdentifierReference(std::string const& ref);

  Json::Value toJsonLD() const;
};

struct cmSPDXNonElementBase : cmSPDXSerializationBase
{
protected:
  cmSPDXNonElementBase(SPDXTypeId id);

  Json::Value toJsonLD() const;
};

// SPDX Core Data Types

// Nominally these are supposed to be validated strings
using cmSPDXDateTime = std::string;
using cmSPDXMediaType = std::string;
using cmSPDXSemVer = std::string;

// SPDX Core Enums

struct cmSPDXAnnotationType
{
  enum cmSPDXAnnotationTypeId
  {
    INVALID = -1,
    NULL_ID = 0,
    OTHER,
    REVIEW,
  };

  cmSPDXAnnotationTypeId TypeId;

  cmSPDXAnnotationType(cmSPDXAnnotationTypeId typeId = NULL_ID);

  Json::Value toJsonLD() const;
};

struct cmSPDXExternalIdentifierType
{
  enum cmSPDXExternalIdentifierTypeId
  {
    INVALID = -1,
    NULL_ID = 0,
    CPE22,
    CPE23,
    CVE,
    EMAIL,
    GITOID,
    OTHER,
    PACKAGE_URL,
    SECURITY_OTHER,
    SWHID,
    SWID,
    URL_SCHEME,
  };

  cmSPDXExternalIdentifierTypeId TypeId;

  cmSPDXExternalIdentifierType(
    cmSPDXExternalIdentifierTypeId typeId = NULL_ID);

  Json::Value toJsonLD() const;
};

struct cmSPDXExternalRefType
{
  enum cmSPDXExternalRefTypeId
  {
    INVALID = -1,
    NULL_ID = 0,
    ALT_DOWNLOAD_LOCATION,
    ALT_WEB_PAGE,
    BINARY_ARTIFACT,
    BOWER,
    BUILD_META,
    BUILD_SYSTEM,
    CERTIFICATION_REPORT,
    CHAT,
    COMPONENT_ANALYSIS_REPORT,
    CWE,
    DOCUMENTATION,
    DYNAMIC_ANALYSIS_REPORT,
    EOL_NOTICE,
    EXPORT_CONTROL_ASSESSMENT,
    FUNDING,
    ISSUE_TRACKER,
    LICENSE,
    MAILING_LIST,
    MAVEN_CENTRAL,
    METRICS,
    NPM,
    NUGET,
    OTHER,
    PRIVACY_ASSESSMENT,
    PRODUCT_METADATA,
    PURCHASE_ORDER,
    QUALITY_ASSESSMENT_REPORT,
    RELEASE_HISTORY,
    RELEASE_NOTES,
    RISK_ASSESSMENT,
    RUNTIME_ANALYSIS_REPORT,
    SECURE_SOFTWARE_ATTESTATION,
    SECURITY_ADVERSARY_MODEL,
    SECURITY_ADVISORY,
    SECURITY_FIX,
    SECURITY_OTHER,
    SECURITY_PEN_TEST_REPORT,
    SECURITY_POLICY,
    SECURITY_THREAT_MODEL,
    SOCIAL_MEDIA,
    SOURCE_ARTIFACT,
    STATIC_ANALYSIS_REPORT,
    SUPPORT,
    VCS,
    VULNERABILITY_DISCLOSURE_REPORT,
    VULNERABILITY_EXPLOITABILITY_ASSESSMENT,
  };

  cmSPDXExternalRefTypeId TypeId;

  cmSPDXExternalRefType(cmSPDXExternalRefTypeId typeId = NULL_ID);

  Json::Value toJsonLD() const;
};

struct cmSPDXHashAlgorithm
{
  enum cmSPDXHashAlgorithmId
  {
    INVALID = -1,
    NULL_ID = 0,
    ADLER32,
    BLAKE2B256,
    BLAKE2B384,
    BLAKE2B512,
    BLAKE3,
    CRYSTALS_DILITHIUM,
    CRYSTALS_KYBER,
    FALCON,
    MD2,
    MD4,
    MD5,
    MD6,
    OTHER,
    SHA1,
    SHA224,
    SHA256,
    SHA384,
    SHA3_224,
    SHA3_256,
    SHA3_384,
    SHA3_512,
    SHA512,
  };

  cmSPDXHashAlgorithmId TypeId;

  cmSPDXHashAlgorithm(cmSPDXHashAlgorithmId typeId = NULL_ID);

  Json::Value toJsonLD() const;
};

struct cmSPDXLifecycleScopeType
{
  enum cmSPDXLifecycleScopeTypeId
  {
    INVALID = -1,
    NULL_ID = 0,
    BUILD,
    DESIGN,
    DEVELOPMENT,
    OTHER,
    RUNTIME,
    TEST,
  };

  cmSPDXLifecycleScopeTypeId TypeId;

  cmSPDXLifecycleScopeType(cmSPDXLifecycleScopeTypeId typeId = NULL_ID);

  Json::Value toJsonLD() const;
};

struct cmSPDXProfileIdentifierType
{
  enum cmSPDXProfileIdentifierTypeId
  {
    INVALID = -1,
    NULL_ID = 0,
    AI,
    BUILD,
    CORE,
    DATASET,
    EXPANDED_LICENSING,
    EXTENSION,
    LITE,
    SECURITY,
    SIMPLE_LICENSING,
    SOFTWARE,
  };

  cmSPDXProfileIdentifierTypeId TypeId;

  cmSPDXProfileIdentifierType(cmSPDXProfileIdentifierTypeId typeId = NULL_ID);

  Json::Value toJsonLD() const;
};

struct cmSPDXRelationshipCompletenessType
{
  enum cmSPDXRelationshipCompletenessTypeId
  {
    INVALID = -1,
    NULL_ID = 0,
    COMPLETE,
    INCOMPLETE,
    NO_ASSERTION,
  };

  cmSPDXRelationshipCompletenessTypeId TypeId;

  cmSPDXRelationshipCompletenessType(
    cmSPDXRelationshipCompletenessTypeId typeId = NULL_ID);

  Json::Value toJsonLD() const;
};

struct cmSPDXRelationshipType
{
  enum cmSPDXRelationshipTypeId
  {
    INVALID = -1,
    NULL_ID = 0,
    AFFECTS,
    AMENDED_BY,
    ANCESTOR_OF,
    AVAILABLE_FROM,
    CONFIGURES,
    CONTAINS,
    COORDINATED_BY,
    COPIED_TO,
    DELEGATED_TO,
    DEPENDS_ON,
    DESCENDANT_OF,
    DESCRIBES,
    DOES_NOT_AFFECT,
    EXPANDS_TO,
    EXPLOIT_CREATED_BY,
    FIXED_BY,
    FIXED_IN,
    FOUND_BY,
    GENERATES,
    HAS_ADDED_FILE,
    HAS_ASSESSMENT_FOR,
    HAS_ASSOCIATED_VULNERABILITY,
    HAS_CONCLUDED_LICENSE,
    HAS_DATA_FILE,
    HAS_DECLARED_LICENSE,
    HAS_DELETED_FILE,
    HAS_DEPENDENCY_MANIFEST,
    HAS_DISTRIBUTION_ARTIFACT,
    HAS_DOCUMENTATION,
    HAS_DYNAMIC_LINK,
    HAS_EVIDENCE,
    HAS_EXAMPLE,
    HAS_HOST,
    HAS_INPUT,
    HAS_METADATA,
    HAS_OPTIONAL_COMPONENT,
    HAS_OPTIONAL_DEPENDENCY,
    HAS_OUTPUT,
    HAS_PREREQUISITE,
    HAS_PROVIDED_DEPENDENCY,
    HAS_REQUIREMENT,
    HAS_SPECIFICATION,
    HAS_STATIC_LINK,
    HAS_TEST,
    HAS_TEST_CASE,
    HAS_VARIANT,
    INVOKED_BY,
    MODIFIED_BY,
    OTHER,
    PACKAGED_BY,
    PATCHED_BY,
    PUBLISHED_BY,
    REPORTED_BY,
    REPUBLISHED_BY,
    SERIALIZED_IN_ARTIFACT,
    TESTED_ON,
    TRAINED_ON,
    UNDER_INVESTIGATION_FOR,
    USES_TOOL,
  };

  cmSPDXRelationshipTypeId TypeId;

  cmSPDXRelationshipType(cmSPDXRelationshipTypeId typeId = NULL_ID);

  Json::Value toJsonLD() const;
};

struct cmSPDXSupportType
{
  enum cmSPDXSupportTypeId
  {
    INVALID = -1,
    NULL_ID = 0,
    DEPLOYED,
    DEVELOPMENT,
    END_OF_SUPPORT,
    LIMITED_SUPPORT,
    NO_ASSERTION,
    NO_SUPPORT,
    SUPPORT,
  };

  cmSPDXSupportTypeId TypeId;

  cmSPDXSupportType(cmSPDXSupportTypeId typeId = NULL_ID);

  Json::Value toJsonLD() const;
};

// SPDX Core NonElement Classes, Abstract

struct cmSPDXIntegrityMethod : cmSPDXNonElementBase
{

  cm::optional<std::string> Comment;

protected:
  cmSPDXIntegrityMethod(SPDXTypeId id);

  Json::Value toJsonLD() const;
};

// SPDX Core NonElement Classes, Concrete

struct cmSPDXCreationInfo : cmSPDXNonElementBase
{
  cm::optional<std::string> Comment;
  cmSPDXDateTime Created;
  std::vector<cmSPDXIdentifierReference> CreatedBy;
  cm::optional<std::vector<cmSPDXIdentifierReference>> CreatedUsing;

  cmSPDXCreationInfo();

  Json::Value toJsonLD() const;
};

struct cmSPDXDictionaryEntry : cmSPDXNonElementBase
{
  std::string Key;
  cm::optional<std::string> Value;

  cmSPDXDictionaryEntry();
  cmSPDXDictionaryEntry(std::string key);
  cmSPDXDictionaryEntry(std::string key, std::string val);

  Json::Value toJsonLD() const;
};

struct cmSPDXExternalIdentifier : cmSPDXNonElementBase
{
  cm::optional<std::string> Comment;
  cmSPDXExternalIdentifierType ExternalIdentifierType;
  std::string Identifier;
  cm::optional<std::vector<std::string>> IdentifierLocator;
  cm::optional<std::string> IssuingAuthority;

  cmSPDXExternalIdentifier();

  Json::Value toJsonLD() const;
};

struct cmSPDXExternalMap : cmSPDXNonElementBase
{
  cm::optional<cmSPDXIdentifierReference> DefiningArtifact;
  std::string ExternalSpdxId;
  cm::optional<std::string> LocationHint;
  cm::optional<std::vector<cmSPDXIdentifierReference>> IntegrityMethod;

  cmSPDXExternalMap();

  Json::Value toJsonLD() const;
};

struct cmSPDXExternalRef : cmSPDXNonElementBase
{
  cm::optional<std::string> Comment;
  cm::optional<cmSPDXMediaType> ContentType;
  cm::optional<cmSPDXExternalRefType> ExternalRefType;
  cm::optional<std::vector<std::string>> Locator;

  cmSPDXExternalRef();

  Json::Value toJsonLD() const;
};

struct cmSPDXHash : cmSPDXIntegrityMethod
{
  cmSPDXHashAlgorithm Algorithm;
  std::string HashValue;

  cmSPDXHash();

  Json::Value toJsonLD() const;
};

struct cmSPDXNamespaceMap : cmSPDXNonElementBase
{
  std::string Namespace;
  std::string Prefix;

  cmSPDXNamespaceMap();

  Json::Value toJsonLD() const;
};

struct cmSPDXPackageVerificationCode : cmSPDXIntegrityMethod
{
  cmSPDXHashAlgorithm Algorithm;
  std::string HashValue;
  cm::optional<std::vector<std::string>> PackageVerificationCodeExcludedFile;

  cmSPDXPackageVerificationCode();

  Json::Value toJsonLD() const;
};

struct cmSPDXPositiveIntegerRange : cmSPDXNonElementBase
{
  unsigned int BeginIntegerRange;
  unsigned int EndIntegerRange;

  cmSPDXPositiveIntegerRange(unsigned int beingIntegerRange = 0,
                             unsigned int endIntegerRange = 0);

  Json::Value toJsonLD() const;
};

// SPDX Core Element Classes, Abstract

struct cmSPDXElement : cmSPDXSerializationBase
{
  cm::optional<std::string> Comment;
  cmSPDXIdentifierReference CreationInfo;
  cm::optional<std::string> Description;
  cm::optional<std::vector<cmSPDXIdentifierReference>> Extension;
  cm::optional<std::vector<cmSPDXIdentifierReference>> ExternalIdentifier;
  cm::optional<std::vector<cmSPDXIdentifierReference>> ExternalRef;
  cm::optional<std::string> Name;
  // SpdxId is the NodeId
  cm::optional<std::string> Summary;
  cm::optional<std::vector<cmSPDXIdentifierReference>> VerifiedUsing;

protected:
  cmSPDXElement(SPDXTypeId id, cmSPDXCreationInfo const& creationInfo);

  Json::Value toJsonLD() const;
};

struct cmSPDXArtifact : cmSPDXElement
{
  cm::optional<cmSPDXDateTime> BuiltTime;
  cm::optional<std::vector<cmSPDXIdentifierReference>> OriginatedBy;
  cm::optional<cmSPDXDateTime> ReleaseTime;
  cm::optional<std::vector<std::string>> StandardName;
  cm::optional<cmSPDXIdentifierReference> SuppliedBy;
  cm::optional<std::vector<cmSPDXSupportType>> SupportType;
  cm::optional<cmSPDXDateTime> ValidUntilTime;

protected:
  cmSPDXArtifact(SPDXTypeId id, cmSPDXCreationInfo const& creationInfo);

  Json::Value toJsonLD() const;
};

struct cmSPDXElementCollection : cmSPDXElement
{
  cm::optional<std::vector<cmSPDXIdentifierReference>> Element;
  cm::optional<std::vector<cmSPDXProfileIdentifierType>> ProfileConformance;
  cm::optional<std::vector<cmSPDXIdentifierReference>> RootElement;

protected:
  cmSPDXElementCollection(SPDXTypeId id,
                          cmSPDXCreationInfo const& creationInfo);

  Json::Value toJsonLD() const;
};

// SPDX Implicit Core Element Classes, Abstract

// Nominally an inheritable class, but adds nothing to Element
using cmSPDXAgentAbstract = cmSPDXElement;

struct cmSPDXBundleAbstract : cmSPDXElementCollection
{
  std::string Context;

protected:
  cmSPDXBundleAbstract(SPDXTypeId id, cmSPDXCreationInfo const& creationInfo);

  Json::Value toJsonLD() const;
};

// Nominally an inheritable class, but adds nothing to Bundle
using cmSPDXBomAbstract = cmSPDXBundleAbstract;

struct cmSPDXRelationshipAbstract : cmSPDXElement
{
  cm::optional<cmSPDXRelationshipCompletenessType> Completeness;
  cm::optional<cmSPDXDateTime> EndTime;
  cmSPDXIdentifierReference From;
  cmSPDXRelationshipType RelationshipType;
  cm::optional<cmSPDXDateTime> StartTime;
  std::vector<cmSPDXIdentifierReference> To;

protected:
  cmSPDXRelationshipAbstract(SPDXTypeId id,
                             cmSPDXCreationInfo const& creationInfo);

  Json::Value toJsonLD() const;
};

// SPDX Core Element Classes, Concrete

struct cmSPDXAgent : cmSPDXAgentAbstract
{
  cmSPDXAgent(cmSPDXCreationInfo const& creationInfo);

  Json::Value toJsonLD() const;
};

struct cmSPDXAnnotation : cmSPDXElement
{
  cmSPDXAnnotationType AnnotationType;
  cm::optional<cmSPDXMediaType> ContentType;
  cm::optional<std::string> Statement;
  cmSPDXIdentifierReference Subject;

  cmSPDXAnnotation(cmSPDXCreationInfo const& creationInfo,
                   cmSPDXIdentifierReference subject = {});

  Json::Value toJsonLD() const;
};

struct cmSPDXBom : cmSPDXBomAbstract
{
  cmSPDXBom(cmSPDXCreationInfo const& creationInfo);

  Json::Value toJsonLD() const;
};

struct cmSPDXBundle : cmSPDXBundleAbstract
{
  cmSPDXBundle(cmSPDXCreationInfo const& creationInfo);

  Json::Value toJsonLD() const;
};

struct cmSPDXIndividualElement : cmSPDXElement
{
  cmSPDXIndividualElement(cmSPDXCreationInfo const& creationInfo);

  Json::Value toJsonLD() const;
};

struct cmSPDXLifecycleScopedRelationship : cmSPDXRelationshipAbstract
{
  cm::optional<cmSPDXLifecycleScopeType> Scope;

  cmSPDXLifecycleScopedRelationship(cmSPDXCreationInfo const& creationInfo);

  Json::Value toJsonLD() const;
};

struct cmSPDXOrganization : cmSPDXAgentAbstract
{
  cmSPDXOrganization(cmSPDXCreationInfo const& creationInfo);

  Json::Value toJsonLD() const;
};

struct cmSPDXPerson : cmSPDXAgentAbstract
{
  cmSPDXPerson(cmSPDXCreationInfo const& creationInfo);

  Json::Value toJsonLD() const;
};

struct cmSPDXRelationship : cmSPDXRelationshipAbstract
{
  cmSPDXRelationship(cmSPDXCreationInfo const& creationInfo);

  Json::Value toJsonLD() const;
};

struct cmSPDXSoftwareAgent : cmSPDXAgentAbstract
{
  cmSPDXSoftwareAgent(cmSPDXCreationInfo const& creationInfo);

  Json::Value toJsonLD() const;
};

struct cmSPDXSpdxDocument : cmSPDXElementCollection
{
  cm::optional<cmSPDXIdentifierReference> DataLicense;
  cm::optional<cmSPDXIdentifierReference> ExternalMap;
  cm::optional<cmSPDXIdentifierReference> NamespaceMap;

  cmSPDXSpdxDocument(cmSPDXCreationInfo const& creationInfo);

  Json::Value toJsonLD() const;
};

struct cmSPDXTool : cmSPDXElement
{
  cmSPDXTool(cmSPDXCreationInfo const& creationInfo);

  Json::Value toJsonLD() const;
};

// SPDX Software Enums

struct cmSPDXContentIdentifierType
{
  enum cmSPDXContentIdentifierTypeId
  {
    INVALID = -1,
    NULL_ID = 0,
    GITOID,
    SWHID,
  };

  cmSPDXContentIdentifierTypeId TypeId;

  cmSPDXContentIdentifierType(cmSPDXContentIdentifierTypeId typeId = NULL_ID);

  Json::Value toJsonLD() const;
};

struct cmSPDXFileKindType
{
  enum cmSPDXFileKindTypeId
  {
    INVALID = -1,
    NULL_ID = 0,
    DIRECTORY,
    FILE,
  };

  cmSPDXFileKindTypeId TypeId;

  cmSPDXFileKindType(cmSPDXFileKindTypeId typeId = NULL_ID);

  Json::Value toJsonLD() const;
};

struct cmSPDXSbomType
{
  enum cmSPDXSbomTypeId
  {
    INVALID = -1,
    NULL_ID = 0,
    ANALYZED,
    BUILD,
    DEPLOYED,
    DESIGN,
    RUNTIME,
    SOURCE,
  };

  cmSPDXSbomTypeId TypeId;

  cmSPDXSbomType(cmSPDXSbomTypeId typeId = NULL_ID);

  Json::Value toJsonLD() const;
};

struct cmSPDXSoftwarePurpose
{
  enum cmSPDXSoftwarePurposeId
  {
    INVALID = -1,
    NULL_ID = 0,
    APPLICATION,
    ARCHIVE,
    BOM,
    CONFIGURATION,
    CONTAINER,
    DATA,
    DEVICE,
    DEVICE_DRIVER,
    DISK_IMAGE,
    DOCUMENTATION,
    EVIDENCE,
    EXECUTABLE,
    FILE,
    FILESYSTEM_IMAGE,
    FIRMWARE,
    FRAMEWORK,
    INSTALL,
    LIBRARY,
    MANIFEST,
    MODEL,
    MODULE,
    OPERATING_SYSTEM,
    OTHER,
    PATCH,
    PLATFORM,
    REQUIREMENT,
    SOURCE,
    SPECIFICATION,
    TEST,
  };

  cmSPDXSoftwarePurposeId TypeId;

  cmSPDXSoftwarePurpose(cmSPDXSoftwarePurposeId typeId = NULL_ID);

  Json::Value toJsonLD() const;
};

// SPDX Software NonElement Classes, Concrete

struct cmSPDXContentIdentifier : cmSPDXIntegrityMethod
{
  cmSPDXContentIdentifierType ContentIdentifierType;
  std::string ContentIdentifierValue;

  cmSPDXContentIdentifier();

  Json::Value toJsonLD() const;
};

// SPDX Software Element Classes, Abstract

struct cmSPDXSoftwareArtifact : cmSPDXArtifact
{
  cm::optional<std::vector<cmSPDXSoftwarePurpose>> AdditionalPurpose;
  cm::optional<std::string> AttributionText;
  cm::optional<cmSPDXIdentifierReference> ContentIdentifier;
  cm::optional<std::string> CopyrightText;
  cm::optional<cmSPDXSoftwarePurpose> PrimaryPurpose;

protected:
  cmSPDXSoftwareArtifact(SPDXTypeId id,
                         cmSPDXCreationInfo const& creationInfo);

  Json::Value toJsonLD() const;
};

// SPDX Software Element Classes, Concrete

struct cmSPDXFile : cmSPDXSoftwareArtifact
{
  cm::optional<cmSPDXMediaType> ContentType;
  cm::optional<cmSPDXFileKindType> FileKind;

  cmSPDXFile(cmSPDXCreationInfo const& creationInfo);

  Json::Value toJsonLD() const;
};

struct cmSPDXPackage : cmSPDXSoftwareArtifact
{
  cm::optional<std::string> DownloadLocation;
  cm::optional<std::string> HomePage;
  cm::optional<std::string> PackageUrl;
  cm::optional<std::string> PackageVersion;
  cm::optional<std::string> SourceInfo;

  cmSPDXPackage(cmSPDXCreationInfo const& creationInfo);

  Json::Value toJsonLD() const;
};

struct cmSPDXSbom : cmSPDXBomAbstract
{
  cm::optional<std::vector<cmSPDXSbomType>> SbomType;

  cmSPDXSbom(cmSPDXCreationInfo const& creationInfo);

  Json::Value toJsonLD() const;
};

struct cmSPDXSnippet : cmSPDXSoftwareArtifact
{
  cm::optional<cmSPDXIdentifierReference> ByteRange;
  cm::optional<cmSPDXIdentifierReference> LineRange;
  cmSPDXIdentifierReference SnippetFromFile;

  cmSPDXSnippet(cmSPDXCreationInfo const& creationInfo);

  Json::Value toJsonLD() const;
};

// SPDX SimpleLicensing Element Classes, Abstract

// Nominally an inheritable class, but adds nothing to Element
using cmSPDXAnyLicenseInfo = cmSPDXElement;

// SPDX SimpleLicensing Element Classes, Concrete

struct cmSPDXLicenseExpression : cmSPDXAnyLicenseInfo
{
  cm::optional<std::vector<cmSPDXIdentifierReference>> CustomIdToUri;
  std::string LicenseExpression;
  cm::optional<cmSPDXSemVer> LicenseListVersion;

  cmSPDXLicenseExpression(cmSPDXCreationInfo const& CreationInfo);

  Json::Value toJsonLD() const;
};

struct cmSPDXSimpleLicensingText : cmSPDXElement
{
  std::string LicenseText;

  cmSPDXSimpleLicensingText(cmSPDXCreationInfo const& CreationInfo);

  Json::Value toJsonLD() const;
};

// Graph Manipulation

template <typename T>
cmSPDXSerializationBase::SPDXTypeId cmSPDXGetTypeId();

template <typename T>
std::string cmSPDXGetTypeName();

#define X_SPDX(classtype, enumid, member, camel)                              \
  template <>                                                                 \
  cmSPDXSerializationBase::SPDXTypeId cmSPDXGetTypeId<classtype>();           \
                                                                              \
  template <>                                                                 \
  std::string cmSPDXGetTypeName<classtype>();
#include "cmSPDXTypes.def"

template <class T>
struct cmSPDXTag
{
  using type = T;
};

union cmSPDXObject
{

  cmSPDXSerializationBase SerializationBase;

  cmSPDXObject();
  cmSPDXObject(cmSPDXObject const& other);
  cmSPDXObject(cmSPDXObject&& other) noexcept;

#define X_SPDX(classtype, enumid, member, camel)                              \
  classtype member;                                                           \
  cmSPDXObject(classtype val);                                                \
                                                                              \
  template <typename... Args>                                                 \
  cmSPDXObject(cmSPDXTag<classtype>, Args&&... args)                          \
  {                                                                           \
    new (&member) classtype(std::forward<Args>(args)...);                     \
  }
#include "cmSPDXTypes.def"

  cmSPDXObject& operator=(cmSPDXObject const& other);
  cmSPDXObject& operator=(cmSPDXObject&& other) noexcept;

  ~cmSPDXObject();

  template <typename T>
  T* get()
  {
    T* ptr;
    get(&ptr);
    return ptr;
  }

  Json::Value toJsonLD() const;

private:
#define X_SPDX(classtype, enumid, member, camel) void get(classtype** ptr);
#include "cmSPDXTypes.def"
};

struct cmSPDXSimpleGraph
{
  cmSPDXSimpleGraph(std::string iriBase, cmSPDXCreationInfo creationInfo = {});

  template <typename T, typename... Args>
  cm::enable_if_t<std::is_base_of<cmSPDXElement, T>::value, T>& insert(
    Args&&... args)
  {
    std::string nodeId = cmStrCat(IRIBase, IRICount++);
    auto const& it =
      Graph.emplace(std::piecewise_construct, std::forward_as_tuple(nodeId),
                    std::forward_as_tuple(cmSPDXTag<T>{}, *CreationInfo,
                                          std::forward<Args>(args)...));
    auto& node = *it.first->second.template get<T>();
    node.NodeId = std::move(nodeId);
    return node;
  }

  cmSPDXCreationInfo& getCreationInfo();

  template <typename T, typename... Args>
  cm::enable_if_t<std::is_base_of<cmSPDXNonElementBase, T>::value, T>& insert(
    Args&&... args)
  {
    std::size_t nodeCount = BlankCounts[cmSPDXGetTypeId<T>()]++;
    std::string nodeId =
      cmStrCat("_:", cmSPDXGetTypeName<T>(), "_", nodeCount);
    auto const& it = Graph.emplace(
      std::piecewise_construct, std::forward_as_tuple(nodeId),
      std::forward_as_tuple(cmSPDXTag<T>{}, std::forward<Args>(args)...));
    auto& node = *it.first->second.template get<T>();
    node.NodeId = std::move(nodeId);
    return node;
  }

  template <typename T>
  T* get(std::string const& key)
  {
    auto it = Graph.find(key);
    if (it == Graph.end()) {
      return nullptr;
    }
    return it->second.get<T>();
  }

  template <typename T>
  T* get(cmSPDXIdentifierReference const& key)
  {
    auto it = Graph.find(key.NodeId);
    if (it == Graph.end()) {
      return nullptr;
    }
    return it->second.get<T>();
  }

  Json::Value toJsonLD();

private:
  std::string IRIBase;
  std::size_t IRICount{ 0 };
  std::array<std::size_t, cmSPDXSerializationBase::SPDX_TYPE_ID_MAX>
    BlankCounts{};

  std::map<std::string, cmSPDXObject> Graph;
  cmSPDXCreationInfo* CreationInfo;
};
