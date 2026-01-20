RunCMake_check_slnx("${RunCMake_TEST_BINARY_DIR}/AutoType.slnx" [[
^<\?xml version="1\.0" encoding="UTF-8"\?>
<Solution>
  <Configurations>
    <BuildType Name="Debug"/>
    <BuildType Name="Release"/>
    <BuildType Name="MinSizeRel"/>
    <BuildType Name="RelWithDebInfo"/>
    <Platform Name="[^"]+"/>
  </Configurations>
  <Project Path="ALL_BUILD\.vcxproj" Type="8bc9ceb8-8b4a-11d0-8d11-00a0c91bc942" Id="[0-9a-f-]+" DefaultStartup="true">
    <BuildDependency Project="ZERO_CHECK\.vcxproj"/>
    <BuildDependency Project="external\.csproj"/>
    <BuildDependency Project="external\.dbproj"/>
    <BuildDependency Project="external\.fsproj"/>
    <BuildDependency Project="external\.njsproj"/>
    <BuildDependency Project="external\.pyproj"/>
    <BuildDependency Project="external\.vbproj"/>
    <BuildDependency Project="external\.vdproj"/>
    <BuildDependency Project="external\.vfproj"/>
    <BuildDependency Project="external\.wapproj"/>
    <BuildDependency Project="external\.wixproj"/>
    <Build Solution="Debug\|\*" Project="false"/>
    <Build Solution="Release\|\*" Project="false"/>
    <Build Solution="MinSizeRel\|\*" Project="false"/>
    <Build Solution="RelWithDebInfo\|\*" Project="false"/>
  </Project>
  <Project Path="ZERO_CHECK\.vcxproj" Type="8bc9ceb8-8b4a-11d0-8d11-00a0c91bc942" Id="[0-9a-f-]+"/>
  <Project Path="external\.csproj" Type="fae04ec0-301f-11d3-bf4b-00c04f79efbc" Id="[0-9a-f-]+">
    <BuildDependency Project="ZERO_CHECK.vcxproj"/>
    <Platform Project="[^"]+"/>
  </Project>
  <Project Path="external\.dbproj" Type="c8d11400-126e-41cd-887f-60bd40844f9e" Id="[0-9a-f-]+">
    <BuildDependency Project="ZERO_CHECK.vcxproj"/>
  </Project>
  <Project Path="external\.fsproj" Type="f2a71f9b-5d33-465a-a702-920d77279786" Id="[0-9a-f-]+">
    <BuildDependency Project="ZERO_CHECK.vcxproj"/>
  </Project>
  <Project Path="external\.njsproj" Type="9092aa53-fb77-4645-b42d-1ccca6bd08bd" Id="[0-9a-f-]+">
    <BuildDependency Project="ZERO_CHECK.vcxproj"/>
  </Project>
  <Project Path="external\.pyproj" Type="888888a0-9f3d-457c-b088-3a5042f75d52" Id="[0-9a-f-]+">
    <BuildDependency Project="ZERO_CHECK.vcxproj"/>
  </Project>
  <Project Path="external\.vbproj" Type="f184b08f-c81c-45f6-a57f-5abd9991f28f" Id="[0-9a-f-]+">
    <BuildDependency Project="ZERO_CHECK.vcxproj"/>
  </Project>
  <Project Path="external\.vdproj" Type="54435603-dbb4-11d2-8724-00a0c9a8b90c" Id="[0-9a-f-]+">
    <BuildDependency Project="ZERO_CHECK.vcxproj"/>
  </Project>
  <Project Path="external\.vfproj" Type="6989167d-11e4-40fe-8c1a-2192a86a7e90" Id="[0-9a-f-]+">
    <BuildDependency Project="ZERO_CHECK.vcxproj"/>
  </Project>
  <Project Path="external\.wapproj" Type="c7167f0d-bc9f-4e6e-afe1-012c56b48db5" Id="[0-9a-f-]+">
    <BuildDependency Project="ZERO_CHECK.vcxproj"/>
  </Project>
  <Project Path="external\.wixproj" Type="930c7802-8a8c-48f9-8165-68863bccd9dd" Id="[0-9a-f-]+">
    <BuildDependency Project="ZERO_CHECK.vcxproj"/>
  </Project>
</Solution>$]])
