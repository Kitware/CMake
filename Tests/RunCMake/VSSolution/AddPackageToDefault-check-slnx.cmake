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
  <Project Path="ALL_BUILD\.vcxproj" Id="[0-9a-f-]+">
    <BuildDependency Project="ZERO_CHECK\.vcxproj"/>
  </Project>
  <Project Path="PACKAGE.vcxproj" Id="[0-9a-f-]+">
    <BuildDependency Project="ALL_BUILD.vcxproj"/>
    <BuildDependency Project="ZERO_CHECK.vcxproj"/>
  </Project>
  <Project Path="ZERO_CHECK\.vcxproj" Id="[0-9a-f-]+"/>
</Solution>$]])
