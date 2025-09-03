{
  "cps_version": "0.13",
  "name": "ConfigMatchBuildType",
  "cps_path": "@prefix@/cps",
  "configurations": [ "release", "debug"],
  "components": {
    "target": {
      "type": "interface",
      "configurations": {
        "debug": {
          "definitions": {
            "*": {
              "NAME": "debug",
              "MARKER": "1"
            }
          }
        },
        "release": {
          "definitions": {
            "*": {
              "NAME": "release",
              "MARKER": "2"
            }
          }
        },
        "test": {
          "definitions": {
            "*": {
              "NAME": "test",
              "MARKER": "4"
            }
          }
        }
      }
    }
  }
}
