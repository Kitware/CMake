{
  "cps_version": "0.13",
  "name": "DefaultConfigurationsTest",
  "cps_path": "@prefix@/cps",
  "configurations": [ "Default" ],
  "components": {
    "Target": {
      "type": "interface",
      "configurations": {
        "Test": {
          "includes": [ "@prefix@/include" ]
        }
      }
    }
  }
}
