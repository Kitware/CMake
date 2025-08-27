{
  "cps_version": "0.13",
  "name": "DefaultConfigurationsTest",
  "cps_path": "@prefix@/cps",
  "configurations": [ "Default" ],
  "components": {
    "Target1": {
      "type": "interface",
      "configurations": {
        "Test": {
          "includes": [ "@prefix@/include" ]
        }
      }
    },
    "Target2": {
      "type": "interface",
      "configurations": {
        "Test": {
          "includes": [ "@prefix@/include" ]
        }
      }
    }
  }
}
