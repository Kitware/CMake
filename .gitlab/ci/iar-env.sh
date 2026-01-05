files='
bxarm-9.70.2.deb
bxavr-8.10.3.deb
bxrh850-3.20.1.deb
bxriscv-3.40.2.deb
bxrl78-5.20.2.deb
bxrx-5.20.1.deb
'
for f in $files; do
  # This URL is only visible inside of Kitware's network.
  curl -OJLs https://cmake.org/files/dependencies/internal/iar/$f
done

echo '
0a90ec54f097ebc24f7e12aa665b22cdaa78ea0a59792814392dce510d9fece1  bxarm-9.70.2.deb
4fc428eee8617d365c526d67f9a30ed0873cbb916160a35ab7b50b4de0628dc0  bxavr-8.10.3.deb
48b85860fc226aa8284b21ba3a49bfe018b12d61b5d0ab184e90840836583521  bxrh850-3.20.1.deb
e701a208d4fac3fe52f685b785c1983383de48d556a77c5921f649d983c7339f  bxriscv-3.40.2.deb
eb672e903d11f45adb591cf4a56e45187fe979c05e1c21d30ba7c1f6d95ea648  bxrl78-5.20.2.deb
059667a53b6683b1b09b7842d71e784c04bc2376734421c3a628acdbc6bef9e3  bxrx-5.20.1.deb
' > bxdebs.sha256sum
sha256sum --check bxdebs.sha256sum

dpkg -i bx*.deb
rm bx*.deb bxdebs.sha256sum

find /opt/iarsystems -executable -wholename "*bin/icc*"

if test -n "$CMAKE_CI_IAR_LICENSE_SERVER"; then
  find /opt/iarsystems -executable -wholename '*bin/lightlicensemanager' -exec {} setup --host "$CMAKE_CI_IAR_LICENSE_SERVER" ';'
  find /opt/iarsystems -executable -wholename "*bin/icc*" -exec {} --version ';'
fi
