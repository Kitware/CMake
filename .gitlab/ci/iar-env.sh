files='
bxarm-9.50.2.deb
bxavr-8.10.2.deb
bxrh850-3.10.2.deb
bxriscv-3.30.1.deb
bxrl78-5.10.3.deb
bxrx-5.10.1.deb
'
for f in $files; do
  # This URL is only visible inside of Kitware's network.
  curl -OJLs https://cmake.org/files/dependencies/internal/iar/$f
done

echo '
cb6a276ace472939fbb76fc5ce517149296ac2c87047b59504f9fe95aed81794  bxarm-9.50.2.deb
4a1065291952a23a8bfbbaa4eb36ca49b0af8653b8faab34ce955d9d48d64506  bxavr-8.10.2.deb
b14085a0f21750c58168125d3cece2e3fcbd4c6495c652b5e65b6637bac0ac31  bxrh850-3.10.2.deb
517e18dffdd4345f97c480b5128c7feea25ec1c3f06e62d8e2e6808c401d514a  bxriscv-3.30.1.deb
3deca7f6afd5f47684464ad748334ab0690097a109d9c680603450074fc32ccf  bxrl78-5.10.3.deb
260e592c48cbaf902b13bdb2feeeba83068978131fcb5c027dab17e715dec7e7  bxrx-5.10.1.deb
' > bxdebs.sha256sum
sha256sum --check bxdebs.sha256sum

dpkg -i bx*.deb
rm bx*.deb bxdebs.sha256sum

find /opt/iarsystems -executable -wholename "*bin/icc*"

if test -n "$CMAKE_CI_IAR_LICENSE_SERVER"; then
  find /opt/iarsystems -executable -wholename '*bin/lightlicensemanager' -exec {} setup --host "$CMAKE_CI_IAR_LICENSE_SERVER" ';'
  find /opt/iarsystems -executable -wholename "*bin/icc*" -exec {} --version ';'
fi
