The supported languages are:

``C``

``CXX``
  C++

``CSharp``
  .. versionadded:: 3.8

  C#

``CUDA``
  .. versionadded:: 3.8

``OBJC``
  .. versionadded:: 3.16

  Objective-C

``OBJCXX``
  .. versionadded:: 3.16

  Objective-C++

``Fortran``

``HIP``
  .. versionadded:: 3.21

``ISPC``
  .. versionadded:: 3.18

``Swift``
  .. versionadded:: 3.15

``ASM``
  Assembly language supported by the C compiler.

  If enabling ``ASM``, list it last so that CMake can check
  whether the ``C`` or ``CXX`` compiler supports assembly.

``ASM_NASM``
  Netwide Assembler

``ASM_MARMASM``
  .. versionadded:: 3.26

  Microsoft Assembler (ARM, ARM64)

``ASM_MASM``
  Microsoft Assembler (x86, x64)

``ASM-ATT``
