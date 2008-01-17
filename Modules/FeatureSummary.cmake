# - Macros for generating a summary of enabled/disabled features
#
# PRINT_ENABLED_FEATURES()
#   Print a summary of all enabled features. By default all successfull
#   FIND_PACKAGE() calls will appear here, except the ones which used the QUIET keyword.
#   Additional features can be added by appending an entry to the global ENABLED_FEATURES
#   property. If SET_FEATURE_INFO() is used for that feature, the output will be much 
#   more informative.
#
# PRINT_DISABLED_FEATURES()
#   Same as PRINT_ENABLED_FEATURES(), but for disabled features. It can be extended
#   the same way by adding to the global property DISABLED_FEATURES.
#
# SET_FEATURE_INFO(NAME DESCRIPTION [URL [COMMENT] ] )
#    Use this macro to set up information about the named feature, which will
#    then be displayed by PRINT_ENABLED/DISABLED_FEATURES().
#    Example: SET_FEATURE_INFO(LibXml2 "XML processing library." "http://xmlsoft.org/")
#


MACRO(SET_FEATURE_INFO _name _desc)
  SET(_url "${ARGV2}")
  SET(_comment "${ARGV3}")
  SET_PROPERTY(GLOBAL PROPERTY ${_name}_DESCRIPTION "${_desc}" )
  IF(_url MATCHES ".+")
    SET_PROPERTY(GLOBAL PROPERTY ${_name}_URL "${_url}" )
  ENDIF(_url MATCHES ".+")
  IF(_comment MATCHES ".+")
    SET_PROPERTY(GLOBAL PROPERTY ${_name}_COMMENT "${_comment}" )
  ENDIF(_comment MATCHES ".+")
ENDMACRO(SET_FEATURE_INFO)


MACRO(_PRINT_FEATURES _property _text)
  SET(_currentFeatureText "${_text}")
  GET_PROPERTY(_EnabledFeatures  GLOBAL PROPERTY ${_property})
  FOREACH(_currentFeature ${_EnabledFeatures})
    SET(_currentFeatureText "${_currentFeatureText}\n${_currentFeature}")
    GET_PROPERTY(_info  GLOBAL PROPERTY ${_currentFeature}_DESCRIPTION)
    IF(_info)
      SET(_currentFeatureText "${_currentFeatureText} , ${_info}")
    ENDIF(_info)
    GET_PROPERTY(_info  GLOBAL PROPERTY ${_currentFeature}_URL)
    IF(_info)
      SET(_currentFeatureText "${_currentFeatureText} , <${_info}>")
    ENDIF(_info)
    GET_PROPERTY(_info  GLOBAL PROPERTY ${_currentFeature}_COMMENT)
    IF(_info)
      SET(_currentFeatureText "${_currentFeatureText} , ${_info}")
    ENDIF(_info)
  ENDFOREACH(_currentFeature)
  MESSAGE(STATUS "${_currentFeatureText}\n")
ENDMACRO(_PRINT_FEATURES)


MACRO(PRINT_ENABLED_FEATURES)
   _PRINT_FEATURES( ENABLED_FEATURES "Enabled features:")
ENDMACRO(PRINT_ENABLED_FEATURES)


MACRO(PRINT_DISABLED_FEATURES)
   _PRINT_FEATURES( DISABLED_FEATURES "Disabled features:")
ENDMACRO(PRINT_DISABLED_FEATURES)

