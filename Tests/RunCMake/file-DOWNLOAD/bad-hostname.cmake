include(common.cmake)

# Do not use any proxy for lookup of an invalid site.
# DNS failure by proxy looks different than DNS failure without proxy.
set(ENV{no_proxy} "$ENV{no_proxy},badhostname.invalid")

set(url "badhostname.invalid")

file_download()
