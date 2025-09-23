{
  "cps_version": "0.13",
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
    }
  }
}
