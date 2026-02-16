/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once
#include <string>
#include <vector>

#include <cm/optional>

#include "cmSbomObject.h"

class cmSbomSerializer;

using Datetime = std::string;
using MediaType = std::string;
using SemVer = std::string;

struct cmSpdxExternalIdentifier
{
  cm::optional<std::string> SpdxId;
  cm::optional<std::string> ExternalIdentifierType;
  cm::optional<std::string> Identifier;
  cm::optional<std::string> Comment;
  cm::optional<std::string> IdentifierLocation;
  cm::optional<std::string> IssuingAuthority;

  void Serialize(cmSbomSerializer&) const;
};

struct cmSpdxExternalRef
{
  cm::optional<std::string> SpdxId;
  cm::optional<std::string> ExternalRefType;
  cm::optional<std::string> Locator;
  cm::optional<std::string> ContentType;
  cm::optional<std::string> Comment;

  void Serialize(cmSbomSerializer&) const;
};

struct cmSpdxCreationInfo
{
  cm::optional<std::string> Id;
  cm::optional<std::string> SpdxId;
  cm::optional<SemVer> SpecVersion;
  cm::optional<std::string> Comment;
  cm::optional<Datetime> Created;
  std::vector<std::string> CreatedBy;
  std::vector<cmSbomObject> CreatedUsing;

  void Serialize(cmSbomSerializer&) const;
};

struct cmSpdxIntegrityMethod
{
  cm::optional<std::string> SpdxId;
  cm::optional<std::string> Comment;

  enum HashAlgorithmId
  {
    ADLER32,
    BLAKE2B256,
    BLAKE2B384,
    BLAKE2B512,
    BLAKE3,
    MD2,
    MD4,
    MD5,
    MD6,
    SHA1,
    SHA224,
    SHA256,
    SHA384,
    SHA512,
    SHA3_256,
    SHA3_384,
    SHA3_512,
  };

  cmSpdxIntegrityMethod() = default;
  cmSpdxIntegrityMethod(cmSpdxIntegrityMethod const&) = default;
  cmSpdxIntegrityMethod(cmSpdxIntegrityMethod&&) = default;
  cmSpdxIntegrityMethod& operator=(cmSpdxIntegrityMethod const&) = default;
  cmSpdxIntegrityMethod& operator=(cmSpdxIntegrityMethod&&) = default;

  virtual void Serialize(cmSbomSerializer&) const;
  virtual ~cmSpdxIntegrityMethod() = default;
};

struct cmSpdxChecksum
{
  cm::optional<std::string> SpdxId;
  cmSpdxIntegrityMethod::HashAlgorithmId Algorithm;
  std::string ChecksumValue;

  void Serialize(cmSbomSerializer&) const;
};

struct cmSpdxElement
{
  cm::optional<std::string> SpdxId;
  cm::optional<std::string> Name;
  cm::optional<std::string> Summary;
  cm::optional<std::string> Description;
  cm::optional<std::string> Comment;
  cm::optional<cmSbomObject> CreationInfo;
  cm::optional<cmSbomObject> VerifiedUsing;
  std::vector<cmSbomObject> ExternalRef;
  std::vector<cmSbomObject> ExternalIdentifier;
  cm::optional<cmSbomObject> Extension;

  cmSpdxElement() = default;
  cmSpdxElement(cmSpdxElement const&) = default;
  cmSpdxElement(cmSpdxElement&&) = default;
  cmSpdxElement& operator=(cmSpdxElement const&) = default;
  cmSpdxElement& operator=(cmSpdxElement&&) = default;

  virtual ~cmSpdxElement() = default;
  virtual void Serialize(cmSbomSerializer&) const;
};

struct cmSpdxTool final : cmSpdxElement
{
  void Serialize(cmSbomSerializer&) const override;
};

struct cmSpdxAgent : cmSpdxElement
{
  void Serialize(cmSbomSerializer&) const override;
};

struct cmSpdxOrganization final : cmSpdxAgent
{
  void Serialize(cmSbomSerializer&) const override;
};

struct cmSpdxPerson final : cmSpdxAgent
{
  void Serialize(cmSbomSerializer&) const override;
};

struct cmSpdxSoftwareAgent final : cmSpdxAgent
{
  void Serialize(cmSbomSerializer&) const override;
};

struct cmSpdxPositiveIntegerRange
{
  cm::optional<std::string> SpdxId;
  cm::optional<std::string> BeginIntegerRange;
  cm::optional<std::string> EndIntegerRange;

  void Serialize(cmSbomSerializer&) const;
};

struct cmSpdxRelationship : cmSpdxElement
{
  enum RelationshipTypeId
  {
    DESCRIBES,
    CONTAINS,
    DEPENDS_ON,
    OTHER
  };

  cm::optional<cmSbomObject> From;
  std::vector<cmSbomObject> To;
  cm::optional<RelationshipTypeId> RelationshipType;
  cm::optional<Datetime> StartTime;
  cm::optional<Datetime> EndTime;

  void Serialize(cmSbomSerializer&) const override;
};

struct cmSpdxLifecycleScopedRelationship final : cmSpdxRelationship
{
  enum ScopeId
  {
    BUILD,
    DESIGN,
    RUNTIME,
    TEST
  };

  cm::optional<ScopeId> Scope;

  void Serialize(cmSbomSerializer&) const override;
};

struct cmSpdxArtifact : cmSpdxElement
{
  enum SupportTypeId
  {
    COMMUNITY,
    COMMERCIAL,
    NONE
  };

  std::vector<cmSbomObject> OriginatedBy;
  cm::optional<cmSbomObject> SuppliedBy;
  cm::optional<Datetime> BuiltTime;
  cm::optional<Datetime> ReleaseTime;
  cm::optional<Datetime> ValidUntilTime;
  cm::optional<std::string> StandardName;
  cm::optional<SupportTypeId> Support;

  void Serialize(cmSbomSerializer&) const override;
};

struct cmSpdxIndividualElement final : cmSpdxElement
{
  void Serialize(cmSbomSerializer&) const override;
};

struct cmSpdxAnnotation final : cmSpdxElement
{
  enum AnnotationTypeId
  {
    REVIEW,
    OTHER
  };

  cm::optional<AnnotationTypeId> AnnotationType;
  cm::optional<MediaType> ContentType;
  cm::optional<std::string> Statement;
  cm::optional<cmSbomObject> Element;

  void Serialize(cmSbomSerializer&) const override;
};

struct cmSpdxExternalMap
{
  cm::optional<std::string> SpdxId;
  cm::optional<std::string> ExternalSpdxId;
  cm::optional<cmSbomObject> VerifiedUsing;
  cm::optional<std::string> LocationHistory;
  cm::optional<cmSbomObject> DefiningArtifact;

  void Serialize(cmSbomSerializer&) const;
};

struct cmSpdxNamespaceMap
{
  cm::optional<std::string> SpdxId;
  cm::optional<std::string> Prefix;
  cm::optional<std::string> Namespace;

  void Serialize(cmSbomSerializer&) const;
};

struct cmSpdxElementCollection : cmSpdxElement
{
  std::vector<cmSbomObject> Elements;
  std::vector<cmSbomObject> RootElements;
  std::vector<std::string> ProfileConformance;

  void Serialize(cmSbomSerializer&) const override;
};

struct cmSpdxPackageVerificationCode final : cmSpdxIntegrityMethod
{
  cm::optional<HashAlgorithmId> Algorithm;
  cm::optional<std::string> HashValue;
  cm::optional<std::string> PackageVerificationCodeExcludedFile;

  void Serialize(cmSbomSerializer&) const override;
};

struct cmSpdxHash final : cmSpdxIntegrityMethod
{
  HashAlgorithmId HashAlgorithm;
  std::string HashValue;

  void Serialize(cmSbomSerializer&) const override;
};

struct cmSpdxBundle : cmSpdxElementCollection
{
  cm::optional<std::string> Context;

  void Serialize(cmSbomSerializer&) const override;
};

struct cmSpdxBom : cmSpdxBundle
{
  void Serialize(cmSbomSerializer&) const override;
};

struct cmSpdxSbom final : cmSpdxBom
{
  enum TypeId
  {
    ANALYZED,
    BUILD,
    DEPLOYED,
    DESIGN,
    RUNTIME,
    SOURCE,
    TEST
  };

  cm::optional<std::vector<TypeId>> Types;
  cm::optional<TypeId> LifecycleScope;

  void Serialize(cmSbomSerializer&) const override;
};

struct cmSpdxSoftwareArtifact : cmSpdxArtifact
{
  enum PurposeId
  {
    APPLICATION,
    ARCHIVE,
    CONTAINER,
    DATA,
    DEVICE,
    FIRMWARE,
    FILE,
    INSTALL,
    LIBRARY,
    MODULE,
    OPERATING_SYSTEM,
    SOURCE
  };

  cm::optional<PurposeId> PrimaryPurpose;
  cm::optional<std::vector<PurposeId>> AdditionalPurpose;
  cm::optional<std::string> CopyrightText;
  cm::optional<std::string> AttributionText;
  cm::optional<cmSbomObject> ContentIdentifier;
  cm::optional<std::string> ArtifactSize;

  void Serialize(cmSbomSerializer&) const override;
};

struct cmSpdxPackage final : cmSpdxSoftwareArtifact
{
  cm::optional<std::string> DownloadLocation;
  cm::optional<std::string> Homepage;
  cm::optional<std::string> PackageVersion;
  cm::optional<std::string> PackageUrl;
  cm::optional<std::string> SourceInfo;

  void Serialize(cmSbomSerializer&) const override;
};

struct cmSpdxDocument final : cmSpdxElementCollection
{
  cm::optional<cmSbomObject> ExternalMap;
  cm::optional<cmSbomObject> NamespaceMap;
  std::string DataLicense;

  void Serialize(cmSbomSerializer& serializer) const override;
};

struct cmSpdxContentIdentifier final : cmSpdxIntegrityMethod
{
  cm::optional<std::string> ContentIdentifierType;
  cm::optional<std::string> ContentValue;

  void Serialize(cmSbomSerializer&) const override;
};

struct cmSpdxFile final : cmSpdxArtifact
{
  enum FileKindId
  {
    DIRECTORY,
    FILE
  };
  cm::optional<MediaType> ContentType;
  cm::optional<FileKindId> FileType;

  void Serialize(cmSbomSerializer&) const override;
};

struct cmSpdxSnippet final : cmSpdxSoftwareArtifact
{
  cm::optional<std::string> ByteRange;
  cm::optional<std::string> LineRange;
  cm::optional<cmSbomObject> SnippetFromFile;

  void Serialize(cmSbomSerializer&) const override;
};

struct cmSbomDocument
{
  cm::optional<std::string> Context;
  std::vector<cmSbomObject> Graph;

  void Serialize(cmSbomSerializer&) const;
};
