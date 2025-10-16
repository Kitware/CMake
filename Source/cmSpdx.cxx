/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmSpdx.h"

#include <stdexcept>
#include <string>

#include "cmSbomSerializer.h"

inline void SerializeIfPresent(cmSbomSerializer& s, std::string const& key,
                               cm::optional<std::string> const& v)
{
  if (v) {
    s.AddString(key, *v);
  }
}

std::string to_string(cmSpdxIntegrityMethod::HashAlgorithmId id)
{
  switch (id) {
    case cmSpdxIntegrityMethod::HashAlgorithmId::ADLER32:
      return "ADLER32";
    case cmSpdxIntegrityMethod::HashAlgorithmId::BLAKE2B256:
      return "BLAKE2B256";
    case cmSpdxIntegrityMethod::HashAlgorithmId::BLAKE2B384:
      return "BLAKE2B384";
    case cmSpdxIntegrityMethod::HashAlgorithmId::BLAKE2B512:
      return "BLAKE2B512";
    case cmSpdxIntegrityMethod::HashAlgorithmId::BLAKE3:
      return "BLAKE3";
    case cmSpdxIntegrityMethod::HashAlgorithmId::MD2:
      return "MD2";
    case cmSpdxIntegrityMethod::HashAlgorithmId::MD4:
      return "MD4";
    case cmSpdxIntegrityMethod::HashAlgorithmId::MD5:
      return "MD5";
    case cmSpdxIntegrityMethod::HashAlgorithmId::MD6:
      return "MD6";
    case cmSpdxIntegrityMethod::HashAlgorithmId::SHA1:
      return "SHA1";
    case cmSpdxIntegrityMethod::HashAlgorithmId::SHA224:
      return "SHA224";
    case cmSpdxIntegrityMethod::HashAlgorithmId::SHA256:
      return "SHA256";
    case cmSpdxIntegrityMethod::HashAlgorithmId::SHA384:
      return "SHA384";
    case cmSpdxIntegrityMethod::HashAlgorithmId::SHA512:
      return "SHA512";
    case cmSpdxIntegrityMethod::HashAlgorithmId::SHA3_256:
      return "SHA3_256";
    case cmSpdxIntegrityMethod::HashAlgorithmId::SHA3_384:
      return "SHA3_384";
    case cmSpdxIntegrityMethod::HashAlgorithmId::SHA3_512:
      return "SHA3_512";
  }
  throw std::invalid_argument("Unknown HashAlgorithmId");
}

std::string to_string(cmSpdxSoftwareArtifact::PurposeId id)
{
  switch (id) {
    case cmSpdxSoftwareArtifact::PurposeId::APPLICATION:
      return "APPLICATION";
    case cmSpdxSoftwareArtifact::PurposeId::ARCHIVE:
      return "ARCHIVE";
    case cmSpdxSoftwareArtifact::PurposeId::CONTAINER:
      return "CONTAINER";
    case cmSpdxSoftwareArtifact::PurposeId::DATA:
      return "DATA";
    case cmSpdxSoftwareArtifact::PurposeId::DEVICE:
      return "DEVICE";
    case cmSpdxSoftwareArtifact::PurposeId::FIRMWARE:
      return "FIRMWARE";
    case cmSpdxSoftwareArtifact::PurposeId::FILE:
      return "FILE";
    case cmSpdxSoftwareArtifact::PurposeId::INSTALL:
      return "INSTALL";
    case cmSpdxSoftwareArtifact::PurposeId::LIBRARY:
      return "LIBRARY";
    case cmSpdxSoftwareArtifact::PurposeId::MODULE:
      return "MODULE";
    case cmSpdxSoftwareArtifact::PurposeId::OPERATING_SYSTEM:
      return "OPERATING_SYSTEM";
    case cmSpdxSoftwareArtifact::PurposeId::SOURCE:
      return "SOURCE";
  }
  throw std::invalid_argument("Unknown PurposeId");
}

std::string to_string(cmSpdxSbom::TypeId id)
{
  switch (id) {
    case cmSpdxSbom::TypeId::ANALYZED:
      return "ANALYZED";
    case cmSpdxSbom::TypeId::BUILD:
      return "BUILD";
    case cmSpdxSbom::TypeId::DEPLOYED:
      return "DEPLOYED";
    case cmSpdxSbom::TypeId::DESIGN:
      return "DESIGN";
    case cmSpdxSbom::TypeId::RUNTIME:
      return "RUNTIME";
    case cmSpdxSbom::TypeId::SOURCE:
      return "SOURCE";
    case cmSpdxSbom::TypeId::TEST:
      return "TEST";
  }
  throw std::invalid_argument("Unknown Sbom::TypeId");
}

std::string to_string(cmSpdxFile::FileKindId id)
{
  switch (id) {
    case cmSpdxFile::FileKindId::DIRECTORY:
      return "DIRECTORY";
    case cmSpdxFile::FileKindId::FILE:
      return "FILE";
  }
  throw std::invalid_argument("Unknown File::FileKindId");
}

std::string to_string(cmSpdxRelationship::RelationshipTypeId id)
{
  switch (id) {
    case cmSpdxRelationship::RelationshipTypeId::DESCRIBES:
      return "DESCRIBES";
    case cmSpdxRelationship::RelationshipTypeId::CONTAINS:
      return "CONTAINS";
    case cmSpdxRelationship::RelationshipTypeId::DEPENDS_ON:
      return "DEPENDS_ON";
    case cmSpdxRelationship::RelationshipTypeId::OTHER:
      return "OTHER";
  }
  throw std::invalid_argument("Unknown RelationshipTypeId");
}

std::string to_string(cmSpdxLifecycleScopedRelationship::ScopeId id)
{
  switch (id) {
    case cmSpdxLifecycleScopedRelationship::ScopeId::BUILD:
      return "BUILD";
    case cmSpdxLifecycleScopedRelationship::ScopeId::DESIGN:
      return "DESIGN";
    case cmSpdxLifecycleScopedRelationship::ScopeId::RUNTIME:
      return "RUNTIME";
    case cmSpdxLifecycleScopedRelationship::ScopeId::TEST:
      return "TEST";
  }
  throw std::invalid_argument("Unknown Lifecycle ScopeId");
}

std::string to_string(cmSpdxAnnotation::AnnotationTypeId id)
{
  switch (id) {
    case cmSpdxAnnotation::AnnotationTypeId::REVIEW:
      return "REVIEW";
    case cmSpdxAnnotation::AnnotationTypeId::OTHER:
      return "OTHER";
  }
  throw std::invalid_argument("Unknown AnnotationTypeId");
}

std::string to_string(cmSpdxArtifact::SupportTypeId id)
{
  switch (id) {
    case cmSpdxArtifact::SupportTypeId::COMMUNITY:
      return "COMMUNITY";
    case cmSpdxArtifact::SupportTypeId::COMMERCIAL:
      return "COMMERCIAL";
    case cmSpdxArtifact::SupportTypeId::NONE:
      return "NONE";
  }
  throw std::invalid_argument("Unknown SupportTypeId");
}

void cmSpdxExternalIdentifier::Serialize(cmSbomSerializer& serializer) const
{
  serializer.AddString("type", "ExternalIdentifier");
  SerializeIfPresent(serializer, "externalIdentifierType",
                     ExternalIdentifierType);
  SerializeIfPresent(serializer, "identifier", Identifier);
  SerializeIfPresent(serializer, "comment", Comment);
  SerializeIfPresent(serializer, "identifierLocation", IdentifierLocation);
  SerializeIfPresent(serializer, "issuingAuthority", IssuingAuthority);
}

void cmSpdxExternalRef::Serialize(cmSbomSerializer& serializer) const
{
  serializer.AddString("type", "ExternalRef");
  SerializeIfPresent(serializer, "externalRefType", ExternalRefType);
  SerializeIfPresent(serializer, "locator", Locator);
  SerializeIfPresent(serializer, "contentType", ContentType);
  SerializeIfPresent(serializer, "comment", Comment);
}

void cmSpdxChecksum::Serialize(cmSbomSerializer& serializer) const
{
  serializer.AddString("type", "Checksum");
  serializer.AddString("algorithm", to_string(Algorithm));
  serializer.AddString("checksumValue", ChecksumValue);
}

void cmSpdxCreationInfo::Serialize(cmSbomSerializer& serializer) const
{
  serializer.AddString("type", "CreationInfo");
  SerializeIfPresent(serializer, "@id", SpdxId);
  if (SpecVersion) {
    serializer.AddString("specVersion", *SpecVersion);
  }
  SerializeIfPresent(serializer, "comment", Comment);
  SerializeIfPresent(serializer, "created", Created);
  if (!CreatedBy.empty()) {
    serializer.AddVectorIfPresent("createdBy", CreatedBy);
  }
  if (!CreatedUsing.empty()) {
    serializer.AddVectorIfPresent("createdUsing", CreatedUsing);
  }
}

void cmSpdxIntegrityMethod::Serialize(cmSbomSerializer& serializer) const
{
  serializer.AddString("type", "IntegrityMethod");
  SerializeIfPresent(serializer, "comment", Comment);
}

void cmSpdxElement::Serialize(cmSbomSerializer& serializer) const
{
  serializer.AddString("type", "Element");
  SerializeIfPresent(serializer, "@id", SpdxId);
  SerializeIfPresent(serializer, "name", Name);
  SerializeIfPresent(serializer, "summary", Summary);
  SerializeIfPresent(serializer, "description", Description);
  SerializeIfPresent(serializer, "comment", Comment);
  if (CreationInfo) {
    serializer.AddVisitable("creationInfo", *CreationInfo);
  }
  if (VerifiedUsing) {
    serializer.AddVisitable("verifiedUsing", *VerifiedUsing);
  }
  if (!ExternalRef.empty()) {
    serializer.AddVectorIfPresent("externalRef", ExternalRef);
  }
  if (!ExternalIdentifier.empty()) {
    serializer.AddVectorIfPresent("externalIdentifier", ExternalIdentifier);
  }
  if (Extension) {
    serializer.AddVisitable("extension", *Extension);
  }
}

void cmSpdxTool::Serialize(cmSbomSerializer& serializer) const
{
  cmSpdxElement::Serialize(serializer);
  serializer.AddString("type", "Tool");
}

void cmSpdxAgent::Serialize(cmSbomSerializer& serializer) const
{
  cmSpdxElement::Serialize(serializer);
  serializer.AddString("type", "Agent");
}

void cmSpdxOrganization::Serialize(cmSbomSerializer& serializer) const
{
  cmSpdxAgent::Serialize(serializer);
  serializer.AddString("type", "Organization");
}

void cmSpdxPerson::Serialize(cmSbomSerializer& serializer) const
{
  cmSpdxAgent::Serialize(serializer);
  serializer.AddString("type", "Person");
}

void cmSpdxSoftwareAgent::Serialize(cmSbomSerializer& serializer) const
{
  cmSpdxAgent::Serialize(serializer);
  serializer.AddString("type", "SoftwareAgent");
}

void cmSpdxPositiveIntegerRange::Serialize(cmSbomSerializer& serializer) const
{
  serializer.AddString("type", "PositiveIntegerRange");
  SerializeIfPresent(serializer, "beginIntegerRange", BeginIntegerRange);
  SerializeIfPresent(serializer, "endIntegerRange", EndIntegerRange);
}

void cmSpdxRelationship::Serialize(cmSbomSerializer& serializer) const
{
  cmSpdxElement::Serialize(serializer);
  serializer.AddString("type", "Relationship");
  if (From) {
    serializer.AddVisitable("from", *From);
  }
  if (!To.empty()) {
    serializer.AddVectorIfPresent("to", To);
  }
  if (RelationshipType) {
    serializer.AddString("relationshipType", to_string(*RelationshipType));
  }
  SerializeIfPresent(serializer, "startTime", StartTime);
  SerializeIfPresent(serializer, "endTime", EndTime);
}

void cmSpdxLifecycleScopedRelationship::Serialize(
  cmSbomSerializer& serializer) const
{
  cmSpdxRelationship::Serialize(serializer);
  serializer.AddString("type", "LifecycleScopedRelationship");
  if (Scope) {
    serializer.AddString("scope", to_string(*Scope));
  }
}

void cmSpdxArtifact::Serialize(cmSbomSerializer& serializer) const
{
  cmSpdxElement::Serialize(serializer);
  serializer.AddString("type", "Artifact");
  if (!OriginatedBy.empty()) {
    serializer.AddVectorIfPresent("originatedBy", OriginatedBy);
  }
  if (SuppliedBy) {
    serializer.AddVisitable("suppliedBy", *SuppliedBy);
  }
  SerializeIfPresent(serializer, "builtTime", BuiltTime);
  SerializeIfPresent(serializer, "releaseTime", ReleaseTime);
  SerializeIfPresent(serializer, "validUntilTime", ValidUntilTime);
  SerializeIfPresent(serializer, "standardName", StandardName);
  if (Support) {
    serializer.AddString("support", to_string(*Support));
  }
}

void cmSpdxIndividualElement::Serialize(cmSbomSerializer& serializer) const
{
  cmSpdxElement::Serialize(serializer);
  serializer.AddString("type", "IndividualElement");
}

void cmSpdxAnnotation::Serialize(cmSbomSerializer& serializer) const
{
  cmSpdxElement::Serialize(serializer);
  serializer.AddString("type", "Annotation");
  if (AnnotationType) {
    serializer.AddString("annotationType", to_string(*AnnotationType));
  }
  SerializeIfPresent(serializer, "contentType", ContentType);
  SerializeIfPresent(serializer, "statement", Statement);
  if (Element) {
    serializer.AddVisitable("element", *Element);
  }
}

void cmSpdxExternalMap::Serialize(cmSbomSerializer& serializer) const
{
  serializer.AddString("type", "ExternalMap");
  SerializeIfPresent(serializer, "externalSpdxId", ExternalSpdxId);
  if (VerifiedUsing) {
    serializer.AddVisitable("verifiedUsing", *VerifiedUsing);
  }
  SerializeIfPresent(serializer, "locationHistory", LocationHistory);
  if (DefiningArtifact) {
    serializer.AddVisitable("definingArtifact", *DefiningArtifact);
  }
}

void cmSpdxNamespaceMap::Serialize(cmSbomSerializer& serializer) const
{
  serializer.AddString("type", "NamespaceMap");
  SerializeIfPresent(serializer, "prefix", Prefix);
  SerializeIfPresent(serializer, "namespace", Namespace);
}

void cmSpdxElementCollection::Serialize(cmSbomSerializer& serializer) const
{
  cmSpdxElement::Serialize(serializer);
  serializer.AddString("type", "ElementCollection");
  if (!Elements.empty()) {
    serializer.AddVectorIfPresent("element", Elements);
  }
  if (!RootElements.empty()) {
    serializer.AddVectorIfPresent("rootElement", RootElements);
  }
  if (!ProfileConformance.empty()) {
    serializer.AddVectorIfPresent("profileConformance", ProfileConformance);
  }
}

void cmSpdxPackageVerificationCode::Serialize(
  cmSbomSerializer& serializer) const
{
  serializer.AddString("type", "PackageVerificationCode");
  if (Algorithm) {
    serializer.AddString("algorithm", to_string(*Algorithm));
  }
  SerializeIfPresent(serializer, "hashValue", HashValue);
  SerializeIfPresent(serializer, "packageVerificationCodeExcludedFile",
                     PackageVerificationCodeExcludedFile);
}

void cmSpdxHash::Serialize(cmSbomSerializer& serializer) const
{
  serializer.AddString("type", "Hash");
  serializer.AddString("hashAlgorithm", to_string(HashAlgorithm));
  serializer.AddString("hashValue", HashValue);
}

void cmSpdxBundle::Serialize(cmSbomSerializer& serializer) const
{
  cmSpdxElementCollection::Serialize(serializer);
  serializer.AddString("type", "Bundle");
  SerializeIfPresent(serializer, "context", Context);
}

void cmSpdxBom::Serialize(cmSbomSerializer& serializer) const
{
  cmSpdxBundle::Serialize(serializer);
  serializer.AddString("type", "Bom");
}

void cmSpdxSbom::Serialize(cmSbomSerializer& serializer) const
{
  cmSpdxBom::Serialize(serializer);
  serializer.AddString("type", "software_Sbom");
  if (Types) {
    for (auto const& t : *Types) {
      serializer.AddString("sbomType", to_string(t));
    }
  }
  if (LifecycleScope) {
    serializer.AddString("lifecycleScope", to_string(*LifecycleScope));
  }
}

void cmSpdxSoftwareArtifact::Serialize(cmSbomSerializer& serializer) const
{
  cmSpdxArtifact::Serialize(serializer);
  serializer.AddString("type", "software_SoftwareArtifact");
  if (PrimaryPurpose) {
    serializer.AddString("primaryPurpose", to_string(*PrimaryPurpose));
  }
  if (AdditionalPurpose) {
    for (auto const& p : *AdditionalPurpose) {
      serializer.AddString("additionalPurpose", to_string(p));
    }
  }
  SerializeIfPresent(serializer, "copyrightText", CopyrightText);
  SerializeIfPresent(serializer, "attributionText", AttributionText);
  if (ContentIdentifier) {
    serializer.AddVisitable("contentIdentifier", *ContentIdentifier);
  }
  SerializeIfPresent(serializer, "artifactSize", ArtifactSize);
}

void cmSpdxPackage::Serialize(cmSbomSerializer& serializer) const
{
  cmSpdxSoftwareArtifact::Serialize(serializer);
  serializer.AddString("type", "software_Package");
  SerializeIfPresent(serializer, "downloadLocation", DownloadLocation);
  SerializeIfPresent(serializer, "homePage", Homepage);
  SerializeIfPresent(serializer, "packageVersion", PackageVersion);
  SerializeIfPresent(serializer, "packageUrl", PackageUrl);
  SerializeIfPresent(serializer, "sourceInfo", SourceInfo);
}

void cmSpdxFile::Serialize(cmSbomSerializer& serializer) const
{
  cmSpdxArtifact::Serialize(serializer);
  serializer.AddString("type", "software_File");
  if (ContentType) {
    serializer.AddString("contentType", *ContentType);
  }
  if (FileType) {
    serializer.AddString("fileType", to_string(*FileType));
  }
}

void cmSpdxContentIdentifier::Serialize(cmSbomSerializer& serializer) const
{
  cmSpdxIntegrityMethod::Serialize(serializer);
  serializer.AddString("type", "software_ContentIdentifier");
  SerializeIfPresent(serializer, "contentIdentifierType",
                     ContentIdentifierType);
  SerializeIfPresent(serializer, "contentValue", ContentValue);
}

void cmSpdxSnippet::Serialize(cmSbomSerializer& serializer) const
{
  cmSpdxSoftwareArtifact::Serialize(serializer);
  serializer.AddString("type", "software_Snippet");
  SerializeIfPresent(serializer, "byteRange", ByteRange);
  SerializeIfPresent(serializer, "lineRange", LineRange);
  if (SnippetFromFile) {
    serializer.AddVisitable("snippetFromFile", *SnippetFromFile);
  }
}

void cmSpdxDocument::Serialize(cmSbomSerializer& serializer) const
{
  cmSpdxElementCollection::Serialize(serializer);

  serializer.AddString("type", "SpdxDocument");
  if (ExternalMap) {
    serializer.AddVisitable("externalMap", *ExternalMap);
  }
  if (NamespaceMap) {
    serializer.AddVisitable("namespaceMap", *NamespaceMap);
  }
  SerializeIfPresent(serializer, "dataLicense", DataLicense);
}

void cmSbomDocument::Serialize(cmSbomSerializer& serializer) const
{
  serializer.AddString(
    "@context",
    Context.value_or("https://spdx.org/rdf/3.0.1/spdx-context.jsonld"));
  if (!Graph.empty()) {
    serializer.AddVectorIfPresent("@graph", Graph);
  }
}
