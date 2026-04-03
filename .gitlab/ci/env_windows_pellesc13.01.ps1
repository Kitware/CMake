. .gitlab/ci/ninja-env.ps1
. .gitlab/ci/pellesc-env.ps1

# FIXME(#21536): Avoid requiring the end user to enable
# Pelles C's Microsoft extensions to use Windows APIs.
$env:CFLAGS = "-Ze"
