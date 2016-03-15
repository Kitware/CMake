DEPLOYMENT_REMOTE_DIRECTORY 
------

Set RemoteDirectory in DeploymentTool and RemoteExecutable in DebuggerTool section in vcproj configuration for VisualStudio 2005 and 2008. 

This functionality is only for WinCE project.

This is useful when you want to debug on remote WinCE device.

E.g.
set_target_properties(${TARGET} PROPERTIES
			DEPLOYMENT_REMOTE_DIRECTORY  "\\FlashStorage"
)

creates section in vcproj->VisualStudioProject->Configurations->Configuration:
<DeploymentTool
	ForceDirty="-1"
	RemoteDirectory="\FlashStorage"
	RegisterOutput="0"
	AdditionalFiles=""/>
<DebuggerTool
	RemoteExecutable="\FlashStorage\target_full_name"
	Arguments=""
/>