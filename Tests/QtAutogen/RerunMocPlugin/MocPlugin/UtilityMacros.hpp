#ifndef UTILITYMACROS_HPP
#define UTILITYMACROS_HPP

#define A_CUSTOM_MACRO(url, jsonFile, pluginRegistrations)                    \
  Q_PLUGIN_METADATA(IID #url FILE jsonFile)

#endif
