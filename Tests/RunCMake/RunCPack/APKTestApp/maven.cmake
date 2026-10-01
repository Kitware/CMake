# A local Maven repository, to resolve CPACK_APK_MAVEN_DEPENDENCIES from
# without network access.  Artifacts that must not be resolved or
# downloaded are left out of it, so that using them fails.
set(maven_repo "${CMAKE_CURRENT_BINARY_DIR}/maven-repo")
set(maven_group_dir "${maven_repo}/com/example/maven")
file(REMOVE_RECURSE "${maven_repo}")

# Zip `files` of the directory `dir` into `archive`.
function(maven_archive archive dir)
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E tar cf "${archive}" --format=zip ${ARGN}
    WORKING_DIRECTORY "${dir}"
    COMMAND_ERROR_IS_FATAL ANY
  )
endfunction()

# A jar of `artifact` `version`, published with a POM holding `pom_body`.
function(maven_pom_artifact artifact version pom_body)
  set(dir "${maven_group_dir}/${artifact}/${version}")
  set(contents "${CMAKE_CURRENT_BINARY_DIR}/maven-contents/${artifact}-${version}")
  file(WRITE "${contents}/${artifact}.txt" "${artifact} ${version}\n")
  file(MAKE_DIRECTORY "${dir}")
  maven_archive("${dir}/${artifact}-${version}.jar" "${contents}" ${artifact}.txt)
  file(SHA1 "${dir}/${artifact}-${version}.jar" sha1)
  if(APK_TEST_MAVEN_BAD_HASH AND artifact STREQUAL "constrained"
      AND version STREQUAL "2.0")
    string(REPEAT "0" 40 sha1)
  endif()
  file(WRITE "${dir}/${artifact}-${version}.jar.sha1" "${sha1}\n")
  file(WRITE "${dir}/${artifact}-${version}.pom" "<?xml version=\"1.0\"?>
<project>
  <modelVersion>4.0.0</modelVersion>
  <groupId>com.example.maven</groupId>
  <artifactId>${artifact}</artifactId>
  <version>${version}</version>
  <packaging>jar</packaging>
${pom_body}</project>
")
endfunction()

# The Gradle module metadata of `artifact` `version`, with `variants`.
function(maven_module artifact version variants)
  file(WRITE "${maven_group_dir}/${artifact}/${version}/${artifact}-${version}.module" "{
  \"formatVersion\": \"1.1\",
  \"component\": {
    \"group\": \"com.example.maven\",
    \"module\": \"${artifact}\",
    \"version\": \"${version}\"
  },
  \"variants\": [${variants}
  ]
}
")
endfunction()

# The artifact listed in CPACK_APK_MAVEN_DEPENDENCIES: an Android archive
# whose manifest declares a permission.
set(contents "${CMAKE_CURRENT_BINARY_DIR}/maven-contents/app-dep")
file(WRITE "${contents}/AndroidManifest.xml" [[
<manifest xmlns:android="http://schemas.android.com/apk/res/android"
          package="com.example.maven">
  <uses-permission android:name="com.example.maven.PERMISSION"/>
</manifest>
]])
file(MAKE_DIRECTORY "${maven_group_dir}/app-dep/1.0")
maven_archive("${maven_group_dir}/app-dep/1.0/app-dep-1.0.aar" "${contents}"
  AndroidManifest.xml)
file(SHA256 "${maven_group_dir}/app-dep/1.0/app-dep-1.0.aar" aar_sha256)
# Only the runtime variant counts: not the one for compiling against, nor
# the sources.  A platform gives the version of a dependency that has none,
# and a constraint only raises the version of an artifact required anyway.
# A pre-release is lower than the release that the Android variant of
# `kmp` requires.
maven_module(app-dep 1.0 "
    {
      \"name\": \"releaseApiElements\",
      \"attributes\": {
        \"org.gradle.category\": \"library\",
        \"org.gradle.usage\": \"java-api\"
      },
      \"dependencies\": [
        { \"group\": \"com.example.maven\", \"module\": \"api-only\",
          \"version\": { \"requires\": \"1.0\" } }
      ],
      \"files\": [
        { \"name\": \"app-dep-1.0.aar\", \"url\": \"app-dep-1.0.aar\",
          \"sha256\": \"${aar_sha256}\" }
      ]
    },
    {
      \"name\": \"releaseRuntimeElements\",
      \"attributes\": {
        \"org.gradle.category\": \"library\",
        \"org.gradle.usage\": \"java-runtime\"
      },
      \"dependencies\": [
        { \"group\": \"com.example.maven\", \"module\": \"kmp\",
          \"version\": { \"requires\": \"1.0\" } },
        { \"group\": \"com.example.maven\", \"module\": \"pom-dep\",
          \"version\": { \"requires\": \"2.0-beta01\" } },
        { \"group\": \"com.example.maven\", \"module\": \"bom\",
          \"version\": { \"requires\": \"1.0\" },
          \"attributes\": { \"org.gradle.category\": \"platform\" } },
        { \"group\": \"com.example.maven\", \"module\": \"bom-managed\" },
        { \"group\": \"com.example.maven\", \"module\": \"evicted\",
          \"version\": { \"requires\": \"1.0\" } },
        { \"group\": \"com.example.maven\", \"module\": \"evicting\",
          \"version\": { \"requires\": \"1.0\" } },
        { \"group\": \"com.example.maven\", \"module\": \"lowered\",
          \"version\": { \"requires\": \"1.0\" } }
      ],
      \"dependencyConstraints\": [
        { \"group\": \"com.example.maven\", \"module\": \"constrained\",
          \"version\": { \"requires\": \"2.0\" } },
        { \"group\": \"com.example.maven\", \"module\": \"unused\",
          \"version\": { \"requires\": \"1.0\" } }
      ],
      \"files\": [
        { \"name\": \"app-dep-1.0.aar\", \"url\": \"app-dep-1.0.aar\",
          \"sha256\": \"${aar_sha256}\" }
      ]
    },
    {
      \"name\": \"releaseSourcesElements\",
      \"attributes\": {
        \"org.gradle.category\": \"documentation\",
        \"org.gradle.usage\": \"java-runtime\"
      },
      \"files\": [
        { \"name\": \"app-dep-1.0-sources.jar\",
          \"url\": \"app-dep-1.0-sources.jar\", \"sha256\": \"00\" }
      ]
    }")

# The platform, whose only variant is for platforms.
maven_module(bom 1.0 "
    {
      \"name\": \"runtimeElements\",
      \"attributes\": {
        \"org.gradle.category\": \"platform\",
        \"org.gradle.usage\": \"java-runtime\"
      },
      \"dependencyConstraints\": [
        { \"group\": \"com.example.maven\", \"module\": \"bom-managed\",
          \"version\": { \"requires\": \"1.0\" } }
      ]
    }")
maven_pom_artifact(bom-managed 1.0 "")

# A Kotlin Multiplatform library, whose Android variant is published as a
# module of its own and preferred over the JVM one.
maven_module(kmp 1.0 "
    {
      \"name\": \"metadataApiElements\",
      \"attributes\": {
        \"org.gradle.category\": \"library\",
        \"org.gradle.usage\": \"kotlin-metadata\",
        \"org.jetbrains.kotlin.platform.type\": \"common\"
      },
      \"files\": [
        { \"name\": \"kmp-1.0.jar\", \"url\": \"kmp-1.0.jar\", \"sha256\": \"00\" }
      ]
    },
    {
      \"name\": \"jvmRuntimeElements-published\",
      \"attributes\": {
        \"org.gradle.category\": \"library\",
        \"org.gradle.usage\": \"java-runtime\",
        \"org.jetbrains.kotlin.platform.type\": \"jvm\"
      },
      \"available-at\": {
        \"url\": \"../../kmp-jvm/1.0/kmp-jvm-1.0.module\",
        \"group\": \"com.example.maven\", \"module\": \"kmp-jvm\",
        \"version\": \"1.0\"
      }
    },
    {
      \"name\": \"androidReleaseRuntimeElements-published\",
      \"attributes\": {
        \"org.gradle.category\": \"library\",
        \"org.gradle.usage\": \"java-runtime\",
        \"org.jetbrains.kotlin.platform.type\": \"androidJvm\"
      },
      \"available-at\": {
        \"url\": \"../../kmp-android/1.0/kmp-android-1.0.module\",
        \"group\": \"com.example.maven\", \"module\": \"kmp-android\",
        \"version\": \"1.0\"
      }
    },
    {
      \"name\": \"iosArm64ApiElements-published\",
      \"attributes\": {
        \"org.gradle.category\": \"library\",
        \"org.gradle.usage\": \"kotlin-api\",
        \"org.jetbrains.kotlin.platform.type\": \"native\"
      }
    }")

# The Android variant requires a newer version of an artifact the
# application requires as well.  Of its debug and release variants, the
# release one is packaged.
set(contents "${CMAKE_CURRENT_BINARY_DIR}/maven-contents/kmp-android")
file(WRITE "${contents}/kmp-android.txt" "kmp-android 1.0\n")
file(MAKE_DIRECTORY "${maven_group_dir}/kmp-android/1.0")
maven_archive("${maven_group_dir}/kmp-android/1.0/kmp-android-1.0.jar"
  "${contents}" kmp-android.txt)
file(SHA256 "${maven_group_dir}/kmp-android/1.0/kmp-android-1.0.jar"
  jar_sha256)
maven_module(kmp-android 1.0 "
    {
      \"name\": \"androidDebugRuntimeElements-published\",
      \"attributes\": {
        \"com.android.build.api.attributes.BuildTypeAttr\": \"debug\",
        \"org.gradle.category\": \"library\",
        \"org.gradle.usage\": \"java-runtime\",
        \"org.jetbrains.kotlin.platform.type\": \"androidJvm\"
      },
      \"files\": [
        { \"name\": \"kmp-android-1.0-debug.jar\",
          \"url\": \"kmp-android-1.0-debug.jar\", \"sha256\": \"00\" }
      ]
    },
    {
      \"name\": \"androidReleaseRuntimeElements-published\",
      \"attributes\": {
        \"com.android.build.api.attributes.BuildTypeAttr\": \"release\",
        \"org.gradle.category\": \"library\",
        \"org.gradle.usage\": \"java-runtime\",
        \"org.jetbrains.kotlin.platform.type\": \"androidJvm\"
      },
      \"dependencies\": [
        { \"group\": \"com.example.maven\", \"module\": \"pom-dep\",
          \"version\": { \"requires\": \"2.0\" } }
      ],
      \"files\": [
        { \"name\": \"kmp-android-1.0.jar\", \"url\": \"kmp-android-1.0.jar\",
          \"sha256\": \"${jar_sha256}\" }
      ]
    }")

# Artifacts published with a POM only.  Test and optional dependencies, and
# managed versions, are not dependencies.
maven_pom_artifact(pom-dep 2.0-beta01 [[
  <dependencyManagement>
    <dependencies>
      <dependency>
        <groupId>com.example.maven</groupId>
        <artifactId>managed-only</artifactId>
        <version>1.0</version>
      </dependency>
    </dependencies>
  </dependencyManagement>
  <dependencies>
    <dependency>
      <groupId>com.example.maven</groupId>
      <artifactId>constrained</artifactId>
      <version>1.0</version>
      <scope>compile</scope>
    </dependency>
    <dependency>
      <groupId>com.example.maven</groupId>
      <artifactId>test-only</artifactId>
      <version>1.0</version>
      <scope>test</scope>
    </dependency>
    <dependency>
      <groupId>com.example.maven</groupId>
      <artifactId>optional-only</artifactId>
      <version>1.0</version>
      <optional>true</optional>
    </dependency>
  </dependencies>
]])
# Neither are the dependencies of plugins, of profiles or in comments, nor
# the excluded dependencies of a dependency.  A managed version is used
# for a dependency that gives none.  The `[` of text and of version ranges
# do not hide the dependencies after them.
maven_pom_artifact(pom-dep 2.0 [[
  <description>Unbalanced [ bracket</description>
  <properties>
    <constrained.version>1.0</constrained.version>
  </properties>
  <dependencyManagement>
    <dependencies>
      <dependency>
        <groupId>com.example.maven</groupId>
        <artifactId>test-only</artifactId>
        <version>[1.0,)</version>
        <scope>test</scope>
      </dependency>
      <dependency>
        <groupId>com.example.maven</groupId>
        <artifactId>pom-managed</artifactId>
        <version>1.0</version>
      </dependency>
    </dependencies>
  </dependencyManagement>
  <dependencies>
    <dependency>
      <groupId>com.example.maven</groupId>
      <artifactId>test-only</artifactId>
      <version>[1.0,)</version>
      <scope>test</scope>
    </dependency>
    <dependency>
      <groupId>com.example.maven</groupId>
      <artifactId>constrained</artifactId>
      <version>${constrained.version}</version>
    </dependency>
    <dependency>
      <groupId>com.example.maven</groupId>
      <artifactId>pom-managed</artifactId>
    </dependency>
    <dependency>
      <groupId>com.example.maven</groupId>
      <artifactId>excluding</artifactId>
      <version>[1.0]</version>
      <exclusions>
        <exclusion>
          <groupId>com.example.maven</groupId>
          <artifactId>excluded</artifactId>
        </exclusion>
      </exclusions>
    </dependency>
    <!--
    <dependency>
      <groupId>com.example.maven</groupId>
      <artifactId>commented-out</artifactId>
      <version>1.0</version>
    </dependency>
    -->
  </dependencies>
  <build>
    <plugins>
      <plugin>
        <groupId>com.example.maven</groupId>
        <artifactId>plugin</artifactId>
        <version>1.0</version>
        <dependencies>
          <dependency>
            <groupId>com.example.maven</groupId>
            <artifactId>plugin-only</artifactId>
            <version>1.0</version>
          </dependency>
        </dependencies>
      </plugin>
    </plugins>
  </build>
  <profiles>
    <profile>
      <id>profile</id>
      <dependencies>
        <dependency>
          <groupId>com.example.maven</groupId>
          <artifactId>profile-only</artifactId>
          <version>1.0</version>
        </dependency>
      </dependencies>
    </profile>
  </profiles>
]])
maven_pom_artifact(pom-managed 1.0 "")
maven_pom_artifact(excluding 1.0 [[
  <dependencies>
    <dependency>
      <groupId>com.example.maven</groupId>
      <artifactId>excluded</artifactId>
      <version>1.0</version>
    </dependency>
  </dependencies>
]])
maven_pom_artifact(constrained 1.0 "")
maven_pom_artifact(constrained 2.0 "")

# `evicting` raises `evicted` to a version that no longer requires the
# higher version of `lowered`, so the version the application requires
# wins.  The group of a sibling is often given as a property.
maven_pom_artifact(evicted 1.0 [[
  <dependencies>
    <dependency>
      <groupId>com.example.maven</groupId>
      <artifactId>lowered</artifactId>
      <version>2.0</version>
    </dependency>
  </dependencies>
]])
maven_pom_artifact(evicted 1.1 "")
maven_pom_artifact(evicting 1.0 [[
  <dependencies>
    <dependency>
      <groupId>${project.groupId}</groupId>
      <artifactId>evicted</artifactId>
      <version>1.1</version>
    </dependency>
  </dependencies>
]])
maven_pom_artifact(lowered 1.0 "")
maven_pom_artifact(lowered 2.0 "")

set(CPACK_APK_MAVEN_DEPENDENCIES com.example.maven:app-dep:1.0)
# Relative to the directory cpack runs in.  The first one does not exist.
set(CPACK_APK_MAVEN_REPOSITORIES missing-repo maven-repo)
