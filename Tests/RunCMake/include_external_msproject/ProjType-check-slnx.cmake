RunCMake_check_slnx("${RunCMake_TEST_BINARY_DIR}/ProjType.slnx" [[
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
    <BuildDependency Project="AspNetCore\.project"/>
    <BuildDependency Project="DotNetCore\.project"/>
    <BuildDependency Project="JScript\.project"/>
    <BuildDependency Project="Misc\.project"/>
    <BuildDependency Project="SqlSrv\.project"/>
    <BuildDependency Project="WebSite\.project"/>
    <BuildDependency Project="ZERO_CHECK\.vcxproj"/>
    <Build Solution="Debug\|\*" Project="false"/>
    <Build Solution="Release\|\*" Project="false"/>
    <Build Solution="MinSizeRel\|\*" Project="false"/>
    <Build Solution="RelWithDebInfo\|\*" Project="false"/>
  </Project>
  <Project Path="AspNetCore.project" Type="8bb2217d-0f2d-49d1-97bc-3654ed321f3b" Id="[0-9a-f-]+">
    <BuildDependency Project="ZERO_CHECK\.vcxproj"/>
  </Project>
  <Project Path="DotNetCore.project" Type="9a19103f-16f7-4668-be54-9a1e7a4f7556" Id="[0-9a-f-]+">
    <BuildDependency Project="ZERO_CHECK\.vcxproj"/>
    <Platform Project="[^"]+"/>
  </Project>
  <Project Path="JScript.project" Type="262852c6-cd72-467d-83fe-5eeb1973a190" Id="[0-9a-f-]+">
    <BuildDependency Project="ZERO_CHECK\.vcxproj"/>
  </Project>
  <Project Path="Misc.project" Type="66a2671d-8fb5-11d2-aa7e-00c04f688dde" Id="[0-9a-f-]+">
    <BuildDependency Project="ZERO_CHECK\.vcxproj"/>
  </Project>
  <Project Path="SqlSrv.project" Type="00d1a9c2-b5f0-4af3-8072-f6c62b433612" Id="[0-9a-f-]+">
    <BuildDependency Project="ZERO_CHECK\.vcxproj"/>
  </Project>
  <Project Path="WebSite.project" Type="e24c65dc-7377-472b-9aba-bc803b73c61a" Id="[0-9a-f-]+">
    <BuildDependency Project="ZERO_CHECK\.vcxproj"/>
  </Project>
  <Project Path="ZERO_CHECK\.vcxproj" Type="8bc9ceb8-8b4a-11d0-8d11-00a0c91bc942" Id="[0-9a-f-]+"/>
</Solution>$]])
