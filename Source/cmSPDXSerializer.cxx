/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmSPDXSerializer.h"

#include <new>
#include <string>

#include <cm/optional>

#include <cm3p/json/value.h>

// Serialization Utilities

template <typename T>
void addVectorSPDXValue(Json::Value& obj, std::string const& key,
                        std::vector<T> const& vec)
{
  auto& list = obj[key];
  list = Json::Value(Json::arrayValue);
  for (auto const& val : vec) {
    list.append(val.toJsonLD());
  }
}

template <>
void addVectorSPDXValue(Json::Value& obj, std::string const& key,
                        std::vector<std::string> const& vec)
{
  auto& list = obj[key];
  list = Json::Value(Json::arrayValue);
  for (auto const& val : vec) {
    list.append(val);
  }
}

template <typename T>
void addOptionalSPDXValue(Json::Value& obj, std::string const& key,
                          cm::optional<std::vector<T>> const& opt)
{
  if (opt) {
    addVectorSPDXValue(obj, key, *opt);
  }
}

template <typename T>
void addOptionalSPDXValue(Json::Value& obj, std::string const& key,
                          cm::optional<T> const& opt)
{
  if (opt) {
    obj[key] = opt->toJsonLD();
  }
}

template <>
void addOptionalSPDXValue(Json::Value& obj, std::string const& key,
                          cm::optional<std::string> const& opt)
{
  if (opt) {
    obj[key] = *opt;
  }
}

// Base Class

cmSPDXSerializationBase::SPDXTypeId cmSPDXSerializationBase::getTypeId() const
{
  return TypeId;
}

cmSPDXSerializationBase::cmSPDXSerializationBase(SPDXTypeId id)
  : TypeId(id)
{
}

cmSPDXSerializationBase::cmSPDXSerializationBase(SPDXTypeId id,
                                                 std::string nodeId)
  : NodeId(std::move(nodeId))
  , TypeId(id)
{
}

// Convenience Classes

cmSPDXIdentifierReference::cmSPDXIdentifierReference()
  : cmSPDXSerializationBase(CM_IDENTIFIER_REFERENCE)
{
}
cmSPDXIdentifierReference::cmSPDXIdentifierReference(
  cmSPDXSerializationBase const& ref)
  : cmSPDXSerializationBase(CM_IDENTIFIER_REFERENCE, ref.NodeId)
{
}
cmSPDXIdentifierReference::cmSPDXIdentifierReference(std::string const& ref)
  : cmSPDXSerializationBase(CM_IDENTIFIER_REFERENCE, ref)
{
}

Json::Value cmSPDXIdentifierReference::toJsonLD() const
{
  return NodeId;
}

cmSPDXNonElementBase::cmSPDXNonElementBase(SPDXTypeId id)
  : cmSPDXSerializationBase(id)
{
}

Json::Value cmSPDXNonElementBase::toJsonLD() const
{
  Json::Value obj(Json::objectValue);
  obj["@id"] = NodeId;
  return obj;
}

// SPDX Core Enums

cmSPDXAnnotationType::cmSPDXAnnotationType(cmSPDXAnnotationTypeId typeId)
  : TypeId(typeId)
{
}

Json::Value cmSPDXAnnotationType::toJsonLD() const
{
  switch (TypeId) {
    case OTHER:
      return "other";
    case REVIEW:
      return "review";
    default:
      return "INVALID_ANNOTATION_TYPE_ID";
  }
}

cmSPDXExternalIdentifierType::cmSPDXExternalIdentifierType(
  cmSPDXExternalIdentifierTypeId typeId)
  : TypeId(typeId)
{
}

Json::Value cmSPDXExternalIdentifierType::toJsonLD() const
{
  switch (TypeId) {
    case CPE22:
      return "cpe22";
    case CPE23:
      return "cpe23";
    case CVE:
      return "cve";
    case EMAIL:
      return "email";
    case GITOID:
      return "gitoid";
    case OTHER:
      return "other";
    case PACKAGE_URL:
      return "packageUrl";
    case SECURITY_OTHER:
      return "securityOther";
    case SWHID:
      return "swhid";
    case SWID:
      return "swid";
    case URL_SCHEME:
      return "urlScheme";
    default:
      return "INVALID_EXTERNAL_IDENTIFIER_TYPE_ID";
  }
}

cmSPDXExternalRefType::cmSPDXExternalRefType(cmSPDXExternalRefTypeId typeId)
  : TypeId(typeId)
{
}

Json::Value cmSPDXExternalRefType::toJsonLD() const
{
  switch (TypeId) {
    case ALT_DOWNLOAD_LOCATION:
      return "altDownloadLocation:";
    case ALT_WEB_PAGE:
      return "altWebPage";
    case BINARY_ARTIFACT:
      return "binaryArtifact";
    case BOWER:
      return "bower";
    case BUILD_META:
      return "buildMeta";
    case BUILD_SYSTEM:
      return "buildSystem";
    case CERTIFICATION_REPORT:
      return "certificationReport";
    case CHAT:
      return "chat";
    case COMPONENT_ANALYSIS_REPORT:
      return "componentAnalysisReport";
    case CWE:
      return "cwe";
    case DOCUMENTATION:
      return "documentation";
    case DYNAMIC_ANALYSIS_REPORT:
      return "dynamicAnalysisReport";
    case EOL_NOTICE:
      return "eolNotice";
    case EXPORT_CONTROL_ASSESSMENT:
      return "exportControlAssessment";
    case FUNDING:
      return "funding";
    case ISSUE_TRACKER:
      return "issueTracker";
    case LICENSE:
      return "license";
    case MAILING_LIST:
      return "mailingList";
    case MAVEN_CENTRAL:
      return "mavenCentral";
    case METRICS:
      return "metrics";
    case NPM:
      return "npm";
    case NUGET:
      return "nuget";
    case OTHER:
      return "other";
    case PRIVACY_ASSESSMENT:
      return "privacyAssessment";
    case PRODUCT_METADATA:
      return "productMetadata";
    case PURCHASE_ORDER:
      return "purchaseOrder";
    case QUALITY_ASSESSMENT_REPORT:
      return "qualityAssessmentReport";
    case RELEASE_HISTORY:
      return "releaseHistory";
    case RELEASE_NOTES:
      return "releaseNotes";
    case RISK_ASSESSMENT:
      return "riskAssessment";
    case RUNTIME_ANALYSIS_REPORT:
      return "runtimeAnalysisReport";
    case SECURE_SOFTWARE_ATTESTATION:
      return "secureSoftwareAttestation";
    case SECURITY_ADVERSARY_MODEL:
      return "securityAdversaryModel";
    case SECURITY_ADVISORY:
      return "securityAdvisory";
    case SECURITY_FIX:
      return "securityFix";
    case SECURITY_OTHER:
      return "securityOther";
    case SECURITY_PEN_TEST_REPORT:
      return "securityPenTestReport";
    case SECURITY_POLICY:
      return "securityPolicy";
    case SECURITY_THREAT_MODEL:
      return "securityThreatModel";
    case SOCIAL_MEDIA:
      return "socialMedia";
    case SOURCE_ARTIFACT:
      return "sourceArtifact";
    case STATIC_ANALYSIS_REPORT:
      return "staticAnalysisReport";
    case SUPPORT:
      return "support";
    case VCS:
      return "vcs";
    case VULNERABILITY_DISCLOSURE_REPORT:
      return "vulnerabilityDisclosureReport";
    case VULNERABILITY_EXPLOITABILITY_ASSESSMENT:
      return "vulnerabilityExploitabilityAssessment";
    default:
      return "INVALID_EXTERNAL_REF_TYPE_ID";
  }
}

cmSPDXHashAlgorithm::cmSPDXHashAlgorithm(cmSPDXHashAlgorithmId typeId)
  : TypeId(typeId)
{
}

Json::Value cmSPDXHashAlgorithm::toJsonLD() const
{
  switch (TypeId) {
    case ADLER32:
      return "adler32";
    case BLAKE2B256:
      return "blake2b256";
    case BLAKE2B384:
      return "blake2b384";
    case BLAKE2B512:
      return "blake2b512";
    case BLAKE3:
      return "blake3";
    case CRYSTALS_DILITHIUM:
      return "crystalsDilithium";
    case CRYSTALS_KYBER:
      return "crystalsLyber";
    case FALCON:
      return "falcon";
    case MD2:
      return "md2";
    case MD4:
      return "md4";
    case MD5:
      return "md5";
    case MD6:
      return "md6";
    case OTHER:
      return "other";
    case SHA1:
      return "sha1";
    case SHA224:
      return "sha224";
    case SHA256:
      return "sha256";
    case SHA384:
      return "sha384";
    case SHA3_224:
      return "sha3_224";
    case SHA3_256:
      return "sha3_256";
    case SHA3_384:
      return "sha3_384";
    case SHA3_512:
      return "sha3_512";
    case SHA512:
      return "sha512";
    default:
      return "INVALID_HASH_TYPE_ID";
  }
}

cmSPDXLifecycleScopeType::cmSPDXLifecycleScopeType(
  cmSPDXLifecycleScopeTypeId typeId)
  : TypeId(typeId)
{
}

Json::Value cmSPDXLifecycleScopeType::toJsonLD() const
{
  switch (TypeId) {
    case BUILD:
      return "build";
    case DESIGN:
      return "design";
    case DEVELOPMENT:
      return "development";
    case OTHER:
      return "other";
    case RUNTIME:
      return "runtime";
    case TEST:
      return "test";
    default:
      return "INVALID_LIFECYCLE_SCOPE_TYPE_ID";
  }
}

cmSPDXProfileIdentifierType::cmSPDXProfileIdentifierType(
  cmSPDXProfileIdentifierTypeId typeId)
  : TypeId(typeId)
{
}

Json::Value cmSPDXProfileIdentifierType::toJsonLD() const
{
  switch (TypeId) {
    case AI:
      return "ai";
    case BUILD:
      return "build";
    case CORE:
      return "code";
    case DATASET:
      return "dataset";
    case EXPANDED_LICENSING:
      return "expandedLicensing";
    case EXTENSION:
      return "extension";
    case LITE:
      return "lite";
    case SECURITY:
      return "security";
    case SIMPLE_LICENSING:
      return "simpleLicensing";
    case SOFTWARE:
      return "software";
    default:
      return "INVALID_PROFILE_IDENTIFIER_TYPE_ID";
  }
}

cmSPDXRelationshipCompletenessType::cmSPDXRelationshipCompletenessType(
  cmSPDXRelationshipCompletenessTypeId typeId)
  : TypeId(typeId)
{
}

Json::Value cmSPDXRelationshipCompletenessType::toJsonLD() const
{
  switch (TypeId) {
    case COMPLETE:
      return "complete";
    case INCOMPLETE:
      return "incomplete";
    case NO_ASSERTION:
      return "noAssertion";
    default:
      return "INVALID_RELATIONSHIP_COMPLETENESS_TYPE_ID";
  }
}

cmSPDXRelationshipType::cmSPDXRelationshipType(cmSPDXRelationshipTypeId typeId)
  : TypeId(typeId)
{
}

Json::Value cmSPDXRelationshipType::toJsonLD() const
{
  switch (TypeId) {
    case AFFECTS:
      return "affects";
    case AMENDED_BY:
      return "amendedBy";
    case ANCESTOR_OF:
      return "ancestorOf";
    case AVAILABLE_FROM:
      return "availableFrom";
    case CONFIGURES:
      return "configures";
    case CONTAINS:
      return "contains";
    case COORDINATED_BY:
      return "coordinatedBy";
    case COPIED_TO:
      return "copiedTo";
    case DELEGATED_TO:
      return "delegatedTo";
    case DEPENDS_ON:
      return "dependsOn";
    case DESCENDANT_OF:
      return "descendantOf";
    case DESCRIBES:
      return "describes";
    case DOES_NOT_AFFECT:
      return "doesNotAffect";
    case EXPANDS_TO:
      return "expandsTo";
    case EXPLOIT_CREATED_BY:
      return "exploitCreatedBy";
    case FIXED_BY:
      return "fixedBy";
    case FIXED_IN:
      return "fixedIn";
    case FOUND_BY:
      return "foundBy";
    case GENERATES:
      return "generates";
    case HAS_ADDED_FILE:
      return "hasAddedFile";
    case HAS_ASSESSMENT_FOR:
      return "hasAssessmentFor";
    case HAS_ASSOCIATED_VULNERABILITY:
      return "hasAssociatedVulnerability";
    case HAS_CONCLUDED_LICENSE:
      return "hasConcludedLicense";
    case HAS_DATA_FILE:
      return "hasDataFile";
    case HAS_DECLARED_LICENSE:
      return "hasDeclaredLicense";
    case HAS_DELETED_FILE:
      return "hasDeletedFile";
    case HAS_DEPENDENCY_MANIFEST:
      return "hasDependencyManifest";
    case HAS_DISTRIBUTION_ARTIFACT:
      return "hasDistributionArtifact";
    case HAS_DOCUMENTATION:
      return "hasDocumentation";
    case HAS_DYNAMIC_LINK:
      return "hasDynamicLink";
    case HAS_EVIDENCE:
      return "hasEvidence";
    case HAS_EXAMPLE:
      return "hasExample";
    case HAS_HOST:
      return "hasHost";
    case HAS_INPUT:
      return "hasInput";
    case HAS_METADATA:
      return "hasMetadata";
    case HAS_OPTIONAL_COMPONENT:
      return "hasOptionalComponent";
    case HAS_OPTIONAL_DEPENDENCY:
      return "hasOptionalDependency";
    case HAS_OUTPUT:
      return "hasOutput";
    case HAS_PREREQUISITE:
      return "hasPrerequisite";
    case HAS_PROVIDED_DEPENDENCY:
      return "hasProvidedDependency";
    case HAS_REQUIREMENT:
      return "hasRequirement";
    case HAS_SPECIFICATION:
      return "hasSpecification";
    case HAS_STATIC_LINK:
      return "hasStaticLink";
    case HAS_TEST:
      return "hasTest";
    case HAS_TEST_CASE:
      return "hasTestCase";
    case HAS_VARIANT:
      return "hasVariant";
    case INVOKED_BY:
      return "invokedBy";
    case MODIFIED_BY:
      return "modifiedBy";
    case OTHER:
      return "other";
    case PACKAGED_BY:
      return "packagedBy";
    case PATCHED_BY:
      return "patchedBy";
    case PUBLISHED_BY:
      return "publishedBy";
    case REPORTED_BY:
      return "reportedBy";
    case REPUBLISHED_BY:
      return "republishedBy";
    case SERIALIZED_IN_ARTIFACT:
      return "serializedInArtifact";
    case TESTED_ON:
      return "testedOn";
    case TRAINED_ON:
      return "trainedOn";
    case UNDER_INVESTIGATION_FOR:
      return "underInvestigationFor";
    case USES_TOOL:
      return "usesTool";
    default:
      return "INVALID_RELATIONSHIP_TYPE_ID";
  }
}

cmSPDXSupportType::cmSPDXSupportType(cmSPDXSupportTypeId typeId)
  : TypeId(typeId)
{
}

Json::Value cmSPDXSupportType::toJsonLD() const
{
  switch (TypeId) {
    case DEPLOYED:
      return "deployed";
    case DEVELOPMENT:
      return "development";
    case END_OF_SUPPORT:
      return "endOfSupport";
    case LIMITED_SUPPORT:
      return "limitedSupport";
    case NO_ASSERTION:
      return "noAssertion";
    case NO_SUPPORT:
      return "noSupport";
    case SUPPORT:
      return "support";
    default:
      return "INVALID_SUPPORT_TYPE_ID";
  }
}

// SPDX Core NonElement Classes, Abstract

cmSPDXIntegrityMethod::cmSPDXIntegrityMethod(SPDXTypeId id)
  : cmSPDXNonElementBase(id)
{
}

Json::Value cmSPDXIntegrityMethod::toJsonLD() const
{
  auto obj = cmSPDXNonElementBase::toJsonLD();
  addOptionalSPDXValue(obj, "comment", Comment);
  return obj;
}

// SPDX Core NonElement Classes, Concrete

cmSPDXCreationInfo::cmSPDXCreationInfo()
  : cmSPDXNonElementBase(CORE_CREATION_INFO)
{
}

Json::Value cmSPDXCreationInfo::toJsonLD() const
{
  auto obj = cmSPDXNonElementBase::toJsonLD();
  obj["type"] = "CreationInfo";
  addOptionalSPDXValue(obj, "Comment", Comment);
  obj["created"] = Created;
  addVectorSPDXValue(obj, "createdBy", CreatedBy);
  addOptionalSPDXValue(obj, "createdUsing", CreatedUsing);
  return obj;
}

cmSPDXDictionaryEntry::cmSPDXDictionaryEntry()
  : cmSPDXNonElementBase(CORE_DICTIONARY_ENTRY)
{
}
cmSPDXDictionaryEntry::cmSPDXDictionaryEntry(std::string key)
  : cmSPDXNonElementBase(CORE_DICTIONARY_ENTRY)
  , Key(std::move(key))
{
}
cmSPDXDictionaryEntry::cmSPDXDictionaryEntry(std::string key, std::string val)
  : cmSPDXNonElementBase(CORE_DICTIONARY_ENTRY)
  , Key(std::move(key))
  , Value(std::move(val))
{
}

Json::Value cmSPDXDictionaryEntry::toJsonLD() const
{
  auto obj = cmSPDXNonElementBase::toJsonLD();
  obj["type"] = "DictionaryEntry";
  obj["key"] = Key;
  addOptionalSPDXValue(obj, "value", Value);
  return obj;
}

cmSPDXExternalIdentifier::cmSPDXExternalIdentifier()
  : cmSPDXNonElementBase(CORE_EXTERNAL_IDENTIFIER)
{
}

Json::Value cmSPDXExternalIdentifier::toJsonLD() const
{
  auto obj = cmSPDXNonElementBase::toJsonLD();
  obj["type"] = "ExternalIdentifier";
  addOptionalSPDXValue(obj, "comment", Comment);
  obj["externalIdentifierType"] = ExternalIdentifierType.toJsonLD();
  obj["identifier"] = Identifier;
  addOptionalSPDXValue(obj, "identifierLocator", IdentifierLocator);
  addOptionalSPDXValue(obj, "issuingAuthority", IssuingAuthority);
  return obj;
}

cmSPDXExternalMap::cmSPDXExternalMap()
  : cmSPDXNonElementBase(CORE_EXTERNAL_MAP)
{
}

Json::Value cmSPDXExternalMap::toJsonLD() const
{
  auto obj = cmSPDXNonElementBase::toJsonLD();
  obj["type"] = "ExternalMap";
  addOptionalSPDXValue(obj, "definingArtifact", DefiningArtifact);
  obj["externalSpdxId"] = ExternalSpdxId;
  addOptionalSPDXValue(obj, "locationHint", LocationHint);
  addOptionalSPDXValue(obj, "integrityMethod", IntegrityMethod);
  return obj;
}

cmSPDXExternalRef::cmSPDXExternalRef()
  : cmSPDXNonElementBase(CORE_EXTERNAL_REF)
{
}

Json::Value cmSPDXExternalRef::toJsonLD() const
{
  auto obj = cmSPDXNonElementBase::toJsonLD();
  obj["type"] = "ExternalRef";
  addOptionalSPDXValue(obj, "comment", Comment);
  addOptionalSPDXValue(obj, "contentType", ContentType);
  addOptionalSPDXValue(obj, "externalRefType", ExternalRefType);
  addOptionalSPDXValue(obj, "locator", Locator);
  return obj;
}

cmSPDXHash::cmSPDXHash()
  : cmSPDXIntegrityMethod(CORE_HASH)
{
}

Json::Value cmSPDXHash::toJsonLD() const
{
  auto obj = cmSPDXIntegrityMethod::toJsonLD();
  obj["type"] = "Hash";
  obj["algorithm"] = Algorithm.toJsonLD();
  obj["hashValue"] = HashValue;
  return obj;
}

cmSPDXNamespaceMap::cmSPDXNamespaceMap()
  : cmSPDXNonElementBase(CORE_NAMESPACE_MAP)
{
}

Json::Value cmSPDXNamespaceMap::toJsonLD() const
{
  auto obj = cmSPDXNonElementBase::toJsonLD();
  obj["type"] = "NamespaceMap";
  obj["namespace"] = Namespace;
  obj["prefix"] = Namespace;
  return obj;
}

cmSPDXPackageVerificationCode::cmSPDXPackageVerificationCode()
  : cmSPDXIntegrityMethod(CORE_PACKAGE_VERIFICATION_CODE)
{
}

Json::Value cmSPDXPackageVerificationCode::toJsonLD() const
{
  auto obj = cmSPDXIntegrityMethod::toJsonLD();
  obj["type"] = "PackageVerificationCode";
  obj["algorithm"] = Algorithm.toJsonLD();
  obj["hashValue"] = HashValue;
  return obj;
}

cmSPDXPositiveIntegerRange::cmSPDXPositiveIntegerRange(
  unsigned int beingIntegerRange, unsigned int endIntegerRange)
  : cmSPDXNonElementBase(CORE_POSITIVE_INTEGER_RANGE)
  , BeginIntegerRange(beingIntegerRange)
  , EndIntegerRange(endIntegerRange)
{
}

Json::Value cmSPDXPositiveIntegerRange::toJsonLD() const
{
  auto obj = cmSPDXNonElementBase::toJsonLD();
  obj["type"] = "PositiveIntegerRange";
  obj["beginIntegerRange"] = BeginIntegerRange;
  obj["endIntegerRange"] = EndIntegerRange;
  return obj;
}

// SPDX Core Element Classes, Abstract

cmSPDXElement::cmSPDXElement(SPDXTypeId id,
                             cmSPDXCreationInfo const& creationInfo)
  : cmSPDXSerializationBase(id)
  , CreationInfo(creationInfo)
{
}

Json::Value cmSPDXElement::toJsonLD() const
{
  Json::Value obj(Json::objectValue);
  addOptionalSPDXValue(obj, "comment", Comment);
  obj["creationInfo"] = CreationInfo.toJsonLD();
  addOptionalSPDXValue(obj, "description", Description);
  addOptionalSPDXValue(obj, "extension", Extension);
  addOptionalSPDXValue(obj, "externalIdentifier", ExternalIdentifier);
  addOptionalSPDXValue(obj, "externalRef", ExternalRef);
  addOptionalSPDXValue(obj, "name", Name);
  obj["spdxId"] = NodeId;
  addOptionalSPDXValue(obj, "summary", Summary);
  addOptionalSPDXValue(obj, "verifiedUsing", VerifiedUsing);
  return obj;
}

cmSPDXArtifact::cmSPDXArtifact(SPDXTypeId id,
                               cmSPDXCreationInfo const& creationInfo)
  : cmSPDXElement(id, creationInfo)
{
}

Json::Value cmSPDXArtifact::toJsonLD() const
{
  auto obj = cmSPDXElement::toJsonLD();
  addOptionalSPDXValue(obj, "builtTime", BuiltTime);
  addOptionalSPDXValue(obj, "originateBy", OriginatedBy);
  addOptionalSPDXValue(obj, "releaseTime", ReleaseTime);
  addOptionalSPDXValue(obj, "standardName", StandardName);
  addOptionalSPDXValue(obj, "suppliedBy", SuppliedBy);
  addOptionalSPDXValue(obj, "supportType", SupportType);
  addOptionalSPDXValue(obj, "validUntilTime", ValidUntilTime);
  return obj;
}

cmSPDXElementCollection::cmSPDXElementCollection(
  SPDXTypeId id, cmSPDXCreationInfo const& creationInfo)
  : cmSPDXElement(id, creationInfo)
{
}

Json::Value cmSPDXElementCollection::toJsonLD() const
{
  auto obj = cmSPDXElement::toJsonLD();
  addOptionalSPDXValue(obj, "element", Element);
  addOptionalSPDXValue(obj, "profileConformance", ProfileConformance);
  addOptionalSPDXValue(obj, "rootElement", RootElement);
  return obj;
}

// SPDX Implicit Core Element Classes, Abstract

// Nominally an inheritable class, but adds nothing to Element
using cmSPDXAgentAbstract = cmSPDXElement;

cmSPDXBundleAbstract::cmSPDXBundleAbstract(
  SPDXTypeId id, cmSPDXCreationInfo const& creationInfo)
  : cmSPDXElementCollection(id, creationInfo)
{
}

Json::Value cmSPDXBundleAbstract::toJsonLD() const
{
  auto obj = cmSPDXElementCollection::toJsonLD();
  obj["context"] = Context;
  return obj;
}

// Nominally an inheritable class, but adds nothing to Bundle
using cmSPDXBomAbstract = cmSPDXBundleAbstract;

cmSPDXRelationshipAbstract::cmSPDXRelationshipAbstract(
  SPDXTypeId id, cmSPDXCreationInfo const& creationInfo)
  : cmSPDXElement(id, creationInfo)
{
}

Json::Value cmSPDXRelationshipAbstract::toJsonLD() const
{
  auto obj = cmSPDXElement::toJsonLD();
  addOptionalSPDXValue(obj, "completeness", Completeness);
  addOptionalSPDXValue(obj, "endTime", EndTime);
  obj["from"] = From.toJsonLD();
  obj["relationshipType"] = RelationshipType.toJsonLD();
  addOptionalSPDXValue(obj, "startTime", StartTime);
  addVectorSPDXValue(obj, "to", To);
  return obj;
}

// SPDX Core Element Classes, Concrete

cmSPDXAgent::cmSPDXAgent(cmSPDXCreationInfo const& creationInfo)
  : cmSPDXAgentAbstract(CORE_AGENT, creationInfo)
{
}

Json::Value cmSPDXAgent::toJsonLD() const
{
  auto obj = cmSPDXAgentAbstract::toJsonLD();
  obj["type"] = "Agent";
  return obj;
}

cmSPDXAnnotation::cmSPDXAnnotation(cmSPDXCreationInfo const& creationInfo,
                                   cmSPDXIdentifierReference subject)
  : cmSPDXElement(CORE_ANNOTATION, creationInfo)
  , Subject(std::move(subject))
{
}

Json::Value cmSPDXAnnotation::toJsonLD() const
{
  auto obj = cmSPDXElement::toJsonLD();
  obj["type"] = "Annotation";
  obj["annotationType"] = AnnotationType.toJsonLD();
  addOptionalSPDXValue(obj, "contentType", ContentType);
  addOptionalSPDXValue(obj, "statement", Statement);
  obj["subject"] = Subject.toJsonLD();
  return obj;
}

cmSPDXBom::cmSPDXBom(cmSPDXCreationInfo const& creationInfo)
  : cmSPDXBomAbstract(CORE_BOM, creationInfo)
{
}

Json::Value cmSPDXBom::toJsonLD() const
{
  auto obj = cmSPDXBomAbstract::toJsonLD();
  obj["type"] = "Bom";
  return obj;
}

cmSPDXBundle::cmSPDXBundle(cmSPDXCreationInfo const& creationInfo)
  : cmSPDXBundleAbstract(CORE_BUNDLE, creationInfo)
{
}

Json::Value cmSPDXBundle::toJsonLD() const
{
  auto obj = cmSPDXBundleAbstract::toJsonLD();
  obj["type"] = "Bundle";
  return obj;
}

cmSPDXIndividualElement::cmSPDXIndividualElement(
  cmSPDXCreationInfo const& creationInfo)
  : cmSPDXElement(CORE_INDIVIDUAL_ELEMENT, creationInfo)
{
}

Json::Value cmSPDXIndividualElement::toJsonLD() const
{
  auto obj = cmSPDXElement::toJsonLD();
  obj["type"] = "IndividualElement";
  return obj;
}

cmSPDXLifecycleScopedRelationship::cmSPDXLifecycleScopedRelationship(
  cmSPDXCreationInfo const& creationInfo)
  : cmSPDXRelationshipAbstract(CORE_LIFECYCLE_SCOPED_RELATIONSHIP,
                               creationInfo)
{
}

Json::Value cmSPDXLifecycleScopedRelationship::toJsonLD() const
{
  auto obj = cmSPDXRelationshipAbstract::toJsonLD();
  obj["type"] = "LifecycleScopedRelationship";
  addOptionalSPDXValue(obj, "scope", Scope);
  return obj;
}

cmSPDXOrganization::cmSPDXOrganization(cmSPDXCreationInfo const& creationInfo)
  : cmSPDXAgentAbstract(CORE_ORGANIZATION, creationInfo)
{
}

Json::Value cmSPDXOrganization::toJsonLD() const
{
  auto obj = cmSPDXAgentAbstract::toJsonLD();
  obj["type"] = "Organization";
  return obj;
}

cmSPDXPerson::cmSPDXPerson(cmSPDXCreationInfo const& creationInfo)
  : cmSPDXAgentAbstract(CORE_PERSON, creationInfo)
{
}

Json::Value cmSPDXPerson::toJsonLD() const
{
  auto obj = cmSPDXAgentAbstract::toJsonLD();
  obj["type"] = "Person";
  return obj;
}

cmSPDXRelationship::cmSPDXRelationship(cmSPDXCreationInfo const& creationInfo)
  : cmSPDXRelationshipAbstract(CORE_RELATIONSHIP, creationInfo)
{
}

Json::Value cmSPDXRelationship::toJsonLD() const
{
  auto obj = cmSPDXRelationshipAbstract::toJsonLD();
  obj["type"] = "Relationship";
  return obj;
}

cmSPDXSoftwareAgent::cmSPDXSoftwareAgent(
  cmSPDXCreationInfo const& creationInfo)
  : cmSPDXAgentAbstract(CORE_SOFTWARE_AGENT, creationInfo)
{
}

Json::Value cmSPDXSoftwareAgent::toJsonLD() const
{
  auto obj = cmSPDXAgentAbstract::toJsonLD();
  obj["type"] = "SoftwareAgent";
  return obj;
}

cmSPDXSpdxDocument::cmSPDXSpdxDocument(cmSPDXCreationInfo const& creationInfo)
  : cmSPDXElementCollection(CORE_SPDX_DOCUMENT, creationInfo)
{
}

Json::Value cmSPDXSpdxDocument::toJsonLD() const
{
  auto obj = cmSPDXElementCollection::toJsonLD();
  obj["type"] = "SpdxDocument";
  addOptionalSPDXValue(obj, "dataLicense", DataLicense);
  addOptionalSPDXValue(obj, "externalMap", ExternalMap);
  addOptionalSPDXValue(obj, "namespaceMap", NamespaceMap);
  return obj;
}

cmSPDXTool::cmSPDXTool(cmSPDXCreationInfo const& creationInfo)
  : cmSPDXElement(CORE_TOOL, creationInfo)
{
}

Json::Value cmSPDXTool::toJsonLD() const
{
  auto obj = cmSPDXElement::toJsonLD();
  obj["type"] = "Tool";
  return obj;
}

// SPDX Software Enums

cmSPDXContentIdentifierType::cmSPDXContentIdentifierType(
  cmSPDXContentIdentifierTypeId typeId)
  : TypeId(typeId)
{
}

Json::Value cmSPDXContentIdentifierType::toJsonLD() const
{
  switch (TypeId) {
    case GITOID:
      return "gitoid";
    case SWHID:
      return "swhid";
    default:
      return "INVALID_CONTENT_IDENTIFIER_TYPE_ID";
  }
}

cmSPDXFileKindType::cmSPDXFileKindType(cmSPDXFileKindTypeId typeId)
  : TypeId(typeId)
{
}

Json::Value cmSPDXFileKindType::toJsonLD() const
{
  switch (TypeId) {
    case DIRECTORY:
      return "directory";
    case FILE:
      return "file";
    default:
      return "INVALID_FILE_KIND_TYPE_ID";
  }
}

cmSPDXSbomType::cmSPDXSbomType(cmSPDXSbomTypeId typeId)
  : TypeId(typeId)
{
}

Json::Value cmSPDXSbomType::toJsonLD() const
{
  switch (TypeId) {
    case ANALYZED:
      return "analyzed";
    case BUILD:
      return "build";
    case DEPLOYED:
      return "deployed";
    case DESIGN:
      return "design";
    case RUNTIME:
      return "runtime";
    case SOURCE:
      return "source";
    default:
      return "INVALID_SBOM_TYPE_ID";
  }
}

cmSPDXSoftwarePurpose::cmSPDXSoftwarePurpose(cmSPDXSoftwarePurposeId typeId)
  : TypeId(typeId)
{
}

Json::Value cmSPDXSoftwarePurpose::toJsonLD() const
{
  switch (TypeId) {
    case APPLICATION:
      return "application";
    case ARCHIVE:
      return "archive";
    case BOM:
      return "bom";
    case CONFIGURATION:
      return "configuration";
    case CONTAINER:
      return "container";
    case DATA:
      return "data";
    case DEVICE:
      return "device";
    case DEVICE_DRIVER:
      return "deviceDriver";
    case DISK_IMAGE:
      return "diskImage";
    case DOCUMENTATION:
      return "documentation";
    case EVIDENCE:
      return "evidence";
    case EXECUTABLE:
      return "executable";
    case FILE:
      return "file";
    case FILESYSTEM_IMAGE:
      return "filesystemImage";
    case FIRMWARE:
      return "firmware";
    case FRAMEWORK:
      return "framework";
    case INSTALL:
      return "install";
    case LIBRARY:
      return "library";
    case MANIFEST:
      return "manifest";
    case MODEL:
      return "model";
    case MODULE:
      return "module";
    case OPERATING_SYSTEM:
      return "operatingSystem";
    case OTHER:
      return "other";
    case PATCH:
      return "patch";
    case PLATFORM:
      return "platform";
    case REQUIREMENT:
      return "requirement";
    case SOURCE:
      return "source";
    case SPECIFICATION:
      return "specification";
    case TEST:
      return "test";
    default:
      return "INVALID_SOFTWARE_PURPOSE_ID";
  }
}

// SPDX Software NonElement Classes, Concrete

cmSPDXContentIdentifier::cmSPDXContentIdentifier()
  : cmSPDXIntegrityMethod(SOFTWARE_CONTENT_IDENTIFIER)
{
}

Json::Value cmSPDXContentIdentifier::toJsonLD() const
{
  auto obj = cmSPDXIntegrityMethod::toJsonLD();
  obj["type"] = "ContentIdentifier";
  obj["contentIdentifierType"] = ContentIdentifierType.toJsonLD();
  obj["contentIdentifierValue"] = ContentIdentifierValue;
  return obj;
}

// SPDX Software Element Classes, Abstract

cmSPDXSoftwareArtifact::cmSPDXSoftwareArtifact(
  SPDXTypeId id, cmSPDXCreationInfo const& creationInfo)
  : cmSPDXArtifact(id, creationInfo)
{
}

Json::Value cmSPDXSoftwareArtifact::toJsonLD() const
{
  auto obj = cmSPDXArtifact::toJsonLD();
  addOptionalSPDXValue(obj, "additionalPurpose", AdditionalPurpose);
  addOptionalSPDXValue(obj, "attributionText", AttributionText);
  addOptionalSPDXValue(obj, "contentIdentifier", ContentIdentifier);
  addOptionalSPDXValue(obj, "copyrightText", CopyrightText);
  addOptionalSPDXValue(obj, "primaryPurpose", PrimaryPurpose);
  return obj;
}

// SPDX Software Element Classes, Concrete

cmSPDXFile::cmSPDXFile(cmSPDXCreationInfo const& creationInfo)
  : cmSPDXSoftwareArtifact(SOFTWARE_FILE, creationInfo)
{
}

Json::Value cmSPDXFile::toJsonLD() const
{
  auto obj = cmSPDXSoftwareArtifact::toJsonLD();
  obj["type"] = "File";
  addOptionalSPDXValue(obj, "contentType", ContentType);
  addOptionalSPDXValue(obj, "fileKind", FileKind);
  return obj;
}

cmSPDXPackage::cmSPDXPackage(cmSPDXCreationInfo const& creationInfo)
  : cmSPDXSoftwareArtifact(SOFTWARE_PACKAGE, creationInfo)
{
}

Json::Value cmSPDXPackage::toJsonLD() const
{
  auto obj = cmSPDXSoftwareArtifact::toJsonLD();
  obj["type"] = "Package";
  addOptionalSPDXValue(obj, "downloadLocation", DownloadLocation);
  addOptionalSPDXValue(obj, "homePage", HomePage);
  addOptionalSPDXValue(obj, "packageUrl", PackageUrl);
  addOptionalSPDXValue(obj, "packageVersion", PackageVersion);
  addOptionalSPDXValue(obj, "sourceInfo", SourceInfo);
  return obj;
}

cmSPDXSbom::cmSPDXSbom(cmSPDXCreationInfo const& creationInfo)
  : cmSPDXBomAbstract(SOFTWARE_SBOM, creationInfo)
{
}

Json::Value cmSPDXSbom::toJsonLD() const
{
  auto obj = cmSPDXBomAbstract::toJsonLD();
  obj["type"] = "Sbom";
  addOptionalSPDXValue(obj, "sbomType", SbomType);
  return obj;
}

cmSPDXSnippet::cmSPDXSnippet(cmSPDXCreationInfo const& creationInfo)
  : cmSPDXSoftwareArtifact(SOFTWARE_SNIPPET, creationInfo)
{
}

Json::Value cmSPDXSnippet::toJsonLD() const
{
  auto obj = cmSPDXSoftwareArtifact::toJsonLD();
  obj["type"] = "Snippet";
  addOptionalSPDXValue(obj, "byteRange", ByteRange);
  addOptionalSPDXValue(obj, "lineRange", LineRange);
  obj["snippetFromFile"] = SnippetFromFile.toJsonLD();
  return obj;
}

// SPDX SimpleLicensing Element Classes, Concrete

cmSPDXLicenseExpression::cmSPDXLicenseExpression(
  cmSPDXCreationInfo const& creationInfo)
  : cmSPDXAnyLicenseInfo(SIMPLE_LICENSING_LICENSE_EXPRESSION, creationInfo)
{
}

Json::Value cmSPDXLicenseExpression::toJsonLD() const
{
  auto obj = cmSPDXAnyLicenseInfo::toJsonLD();
  obj["type"] = "LicenseExpression";
  addOptionalSPDXValue(obj, "customIdToUri", CustomIdToUri);
  obj["licenseExpression"] = LicenseExpression;
  addOptionalSPDXValue(obj, "licenseListVersion", LicenseListVersion);
  return obj;
}

cmSPDXSimpleLicensingText::cmSPDXSimpleLicensingText(
  cmSPDXCreationInfo const& creationInfo)
  : cmSPDXElement(SIMPLE_LICENSING_SIMPLE_LICENSING_TEXT, creationInfo)
{
}

Json::Value cmSPDXSimpleLicensingText::toJsonLD() const
{
  auto obj = cmSPDXElement::toJsonLD();
  obj["type"] = "SimpleLicensingText";
  obj["licenseText"] = LicenseText;
  return obj;
}

// Graph Manipulation

#define X_SPDX(classtype, enumid, member, camel)                              \
  template <>                                                                 \
  cmSPDXSerializationBase::SPDXTypeId cmSPDXGetTypeId<classtype>()            \
  {                                                                           \
    return cmSPDXSerializationBase::enumid;                                   \
  }                                                                           \
                                                                              \
  template <>                                                                 \
  std::string cmSPDXGetTypeName<classtype>()                                  \
  {                                                                           \
    return #camel;                                                            \
  }                                                                           \
                                                                              \
  cmSPDXObject::cmSPDXObject(classtype val)                                   \
    : member(std::move(val)) {};                                              \
                                                                              \
  void cmSPDXObject::get(classtype** ptr)                                     \
  {                                                                           \
    *ptr = SerializationBase.getTypeId() == cmSPDXSerializationBase::enumid   \
      ? &member                                                               \
      : nullptr;                                                              \
  }
#include "cmSPDXTypes.def"

cmSPDXObject::cmSPDXObject()
  : IdentifierReference("UNINITIALIZED_SPDX_OBJECT")
{
}

cmSPDXObject::cmSPDXObject(cmSPDXObject const& other)
{
  switch (other.SerializationBase.getTypeId()) {
#define X_SPDX(classtype, enumid, member, camel)                              \
  case cmSPDXSerializationBase::enumid:                                       \
    new (&member) classtype(other.member);                                    \
    break;
#include "cmSPDXTypes.def"
    default:
      new (&IdentifierReference)
        cmSPDXIdentifierReference("UNINITIALIZED_SPDX_OBJECT");
  }
}

cmSPDXObject::cmSPDXObject(cmSPDXObject&& other) noexcept
{
  switch (other.SerializationBase.getTypeId()) {
#define X_SPDX(classtype, enumid, member, camel)                              \
  case cmSPDXSerializationBase::enumid:                                       \
    new (&member) classtype(std::move(other.member));                         \
    break;
#include "cmSPDXTypes.def"
    default:
      new (&IdentifierReference)
        cmSPDXIdentifierReference("UNINITIALIZED_SPDX_OBJECT");
  }
}

cmSPDXObject& cmSPDXObject::operator=(cmSPDXObject const& other)
{
  this->~cmSPDXObject();
  new (this) cmSPDXObject(other);
  return *this;
}

cmSPDXObject& cmSPDXObject::operator=(cmSPDXObject&& other) noexcept
{
  this->~cmSPDXObject();
  new (this) cmSPDXObject(std::move(other));
  return *this;
}

cmSPDXObject::~cmSPDXObject()
{
  switch (SerializationBase.getTypeId()) {
#define X_SPDX(classtype, enumid, member, camel)                              \
  case cmSPDXSerializationBase::enumid:                                       \
    member.~classtype();                                                      \
    break;
#include "cmSPDXTypes.def"
    default:
      break;
  }
}

Json::Value cmSPDXObject::toJsonLD() const
{
  switch (SerializationBase.getTypeId()) {
#define X_SPDX(classtype, enumid, member, camel)                              \
  case cmSPDXSerializationBase::enumid:                                       \
    return member.toJsonLD();
#include "cmSPDXTypes.def"
    default:
      return "INVALID_SPDX_OBJECT_TYPE_ID";
  }
}

cmSPDXSimpleGraph::cmSPDXSimpleGraph(std::string iriBase,
                                     cmSPDXCreationInfo creationInfo)
  : IRIBase(std::move(iriBase))
  , CreationInfo(&insert<cmSPDXCreationInfo>(std::move(creationInfo)))
{
}

cmSPDXCreationInfo& cmSPDXSimpleGraph::getCreationInfo()
{
  return *CreationInfo;
}

Json::Value cmSPDXSimpleGraph::toJsonLD()
{
  Json::Value obj(Json::objectValue);
  obj["@context"] = "https://spdx.org/rdf/3.0.1/spdx-context.jsonld";

  auto& graph = obj["@graph"];
  for (auto const& it : Graph) {
    graph.append(it.second.toJsonLD());
  }

  return obj;
}
