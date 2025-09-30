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
  <Project Path="ALL_BUILD\.vcxproj" Id="[0-9a-f-]+">
    <BuildDependency Project="ZERO_CHECK\.vcxproj"/>
    <BuildDependency Project="external\.csproj"/>
    <BuildDependency Project="external\.dbproj"/>
    <BuildDependency Project="external\.fsproj"/>
    <BuildDependency Project="external\.pyproj"/>
    <BuildDependency Project="external\.vbproj"/>
    <BuildDependency Project="external\.vdproj"/>
    <BuildDependency Project="external\.vfproj"/>
    <BuildDependency Project="external\.wixproj"/>
    <Build Solution="Debug\|\*" Project="false"/>
    <Build Solution="Release\|\*" Project="false"/>
    <Build Solution="MinSizeRel\|\*" Project="false"/>
    <Build Solution="RelWithDebInfo\|\*" Project="false"/>
  </Project>
  <Project Path="ZERO_CHECK\.vcxproj" Id="[0-9a-f-]+"/>
  <Project Path="external\.csproj" Id="[0-9a-f-]+">
    <BuildDependency Project="ZERO_CHECK.vcxproj"/>
  </Project>
  <Project Path="external\.dbproj" Id="[0-9a-f-]+">
    <BuildDependency Project="ZERO_CHECK.vcxproj"/>
  </Project>
  <Project Path="external\.fsproj" Id="[0-9a-f-]+">
    <BuildDependency Project="ZERO_CHECK.vcxproj"/>
  </Project>
  <Project Path="external\.pyproj" Id="[0-9a-f-]+">
    <BuildDependency Project="ZERO_CHECK.vcxproj"/>
  </Project>
  <Project Path="external\.vbproj" Id="[0-9a-f-]+">
    <BuildDependency Project="ZERO_CHECK.vcxproj"/>
  </Project>
  <Project Path="external\.vdproj" Id="[0-9a-f-]+">
    <BuildDependency Project="ZERO_CHECK.vcxproj"/>
  </Project>
  <Project Path="external\.vfproj" Id="[0-9a-f-]+">
    <BuildDependency Project="ZERO_CHECK.vcxproj"/>
  </Project>
  <Project Path="external\.wixproj" Id="[0-9a-f-]+">
    <BuildDependency Project="ZERO_CHECK.vcxproj"/>
  </Project>
</Solution>$]])
