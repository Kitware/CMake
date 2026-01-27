string(JSON object SET {} key \"value\")

cmake_instrumentation(
  API_VERSION 1
  DATA_VERSION 1
  CUSTOM_CONTENT nConfigure STRING "${N}"
  CUSTOM_CONTENT myString STRING "string"
  CUSTOM_CONTENT myList   LIST "a;b;c"
  CUSTOM_CONTENT myBool   BOOL OFF
  CUSTOM_CONTENT myObject JSON "${object}"
  CUSTOM_CONTENT myInt    JSON 1
  CUSTOM_CONTENT myFloat  JSON 2.5
  CUSTOM_CONTENT myTrue   JSON true
)
