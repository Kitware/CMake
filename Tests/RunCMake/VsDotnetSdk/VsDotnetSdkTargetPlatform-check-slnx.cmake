RunCMake_check_slnx("${RunCMake_TEST_BINARY_DIR}/VsDotnetSdkTargetPlatform.slnx" [[
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
    <BuildDependency Project="foo\.csproj"/>
    <Build Solution="Debug\|\*" Project="false"/>
    <Build Solution="Release\|\*" Project="false"/>
    <Build Solution="MinSizeRel\|\*" Project="false"/>
    <Build Solution="RelWithDebInfo\|\*" Project="false"/>
  </Project>
  <Project Path="ZERO_CHECK\.vcxproj" Type="8bc9ceb8-8b4a-11d0-8d11-00a0c91bc942" Id="[0-9a-f-]+"/>
  <Project Path="foo\.csproj" Type="fae04ec0-301f-11d3-bf4b-00c04f79efbc" Id="[0-9a-f-]+">
    <BuildDependency Project="ZERO_CHECK\.vcxproj"/>
    <Platform Project="[^"]+"/>
  </Project>
</Solution>$]])
