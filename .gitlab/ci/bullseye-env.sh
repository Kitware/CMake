# Install Bullseye
.gitlab/ci/bullseye.sh
unset CMAKE_CI_BULLSEYE_LICENSE

# Make Bullseye tools available but do not override compilers.
export PATH="$PATH:/opt/bullseye/bin"

# Print the Bullseye startup banner once.
covc --help 2>&1 | head -1

# Suppress the Bullseye startup banner.
for tool in cov01 covc; do
  echo "--no-banner" > "/opt/bullseye/bin/$tool.cfg"
done
