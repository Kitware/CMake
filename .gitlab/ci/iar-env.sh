files='
bxarm-9.70.1.deb
bxavr-8.10.2.deb
bxrh850-3.10.2.deb
bxriscv-3.40.1.deb
bxrl78-5.20.1.deb
bxrx-5.20.1.deb
'
for f in $files; do
  # This URL is only visible inside of Kitware's network.
  curl -OJLs https://cmake.org/files/dependencies/internal/iar/$f
done

echo '
3b16748e560ab8fa3ffe6d6807186ac706134c78bc9db911112ab3ee67c7b997  bxarm-9.70.1.deb
4a1065291952a23a8bfbbaa4eb36ca49b0af8653b8faab34ce955d9d48d64506  bxavr-8.10.2.deb
b14085a0f21750c58168125d3cece2e3fcbd4c6495c652b5e65b6637bac0ac31  bxrh850-3.10.2.deb
2e7de58a3aad43ef4199b811edd4dae9c4bff633376393f12fcb77ca27aba831  bxriscv-3.40.1.deb
6a2b6163dd971635715f49cb072c853e5de55d2b0089f319a6a9f6db540af4bd  bxrl78-5.20.1.deb
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
