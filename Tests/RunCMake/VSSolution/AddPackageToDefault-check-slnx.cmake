RunCMake_check_slnx("${RunCMake_TEST_BINARY_DIR}/AddPackageToDefault.slnx" [[
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
  </Project>
  <Project Path="PACKAGE.vcxproj" Type="8bc9ceb8-8b4a-11d0-8d11-00a0c91bc942" Id="[0-9a-f-]+">
    <BuildDependency Project="ALL_BUILD.vcxproj"/>
    <BuildDependency Project="ZERO_CHECK.vcxproj"/>
  </Project>
  <Project Path="ZERO_CHECK\.vcxproj" Type="8bc9ceb8-8b4a-11d0-8d11-00a0c91bc942" Id="[0-9a-f-]+"/>
</Solution>$]])
