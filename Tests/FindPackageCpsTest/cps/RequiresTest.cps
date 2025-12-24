{
  "cps_version": "0.14.0",
  "name": "RequiresTest",
  "cps_path": "@prefix@/cps",
  "components": {
    "Indirect": {
      "type": "interface",
      "requires": [ ":Direct" ]
    },
    "Direct": {
      "type": "interface",
      "definitions": {
        "*": {
          "ANSWER": 42
        }
      }
    },
    "CompileOnly": {
      "type": "interface",
      "compile_requires": [ ":BrokenLibrary" ]
    },
    "BrokenLibrary": {
      "type": "archive",
      "location": "@prefix@/lib/does-not-exist.a",
      "definitions": {
        "*": {
          "ANSWER": 42
        }
      }
    }
  }
}
