#!/usr/bin/env python3
"""Generate minimal ELF corpus files for fuzzing.

These are hand-crafted minimal ELF headers that exercise the ELF parser
without requiring actual compiled binaries. They contain just enough
structure to be recognized as valid ELF files.

Usage: python3 generate.py

Output files can be verified with: file *.elf
"""

import os
import struct

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))


def generate_minimal_elf64():
    """Generate a minimal 64-bit ELF executable header (72 bytes).

    Structure: ELF64 header (64 bytes) with e_type=ET_EXEC, e_machine=EM_X86_64,
    plus 8 bytes padding for fuzzer mutation headroom.
    """
    elf = bytearray()

    # e_ident[16]: ELF identification
    elf.extend(b'\x7fELF')  # EI_MAG: magic number
    elf.append(2)           # EI_CLASS: ELFCLASS64
    elf.append(1)           # EI_DATA: ELFDATA2LSB (little-endian)
    elf.append(1)           # EI_VERSION: EV_CURRENT
    elf.append(0)           # EI_OSABI: ELFOSABI_NONE
    elf.extend(b'\x00' * 8) # EI_PAD: padding

    # ELF header fields
    elf.extend(struct.pack('<H', 2))      # e_type: ET_EXEC
    elf.extend(struct.pack('<H', 0x3e))   # e_machine: EM_X86_64
    elf.extend(struct.pack('<I', 1))      # e_version: EV_CURRENT
    elf.extend(struct.pack('<Q', 0))      # e_entry: entry point
    elf.extend(struct.pack('<Q', 0))      # e_phoff: program header offset
    elf.extend(struct.pack('<Q', 0))      # e_shoff: section header offset
    elf.extend(struct.pack('<I', 0))      # e_flags
    elf.extend(struct.pack('<H', 0))      # e_ehsize
    elf.extend(struct.pack('<H', 0))      # e_phentsize
    elf.extend(struct.pack('<H', 0))      # e_phnum
    elf.extend(struct.pack('<H', 0))      # e_shentsize
    elf.extend(struct.pack('<H', 0))      # e_shnum
    elf.extend(struct.pack('<H', 0))      # e_shstrndx

    # Padding for fuzzer mutation headroom
    elf.extend(b'\x00' * 8)

    return bytes(elf)


def generate_minimal_elf32():
    """Generate a minimal 32-bit ELF PIE header (60 bytes).

    Structure: ELF32 header (52 bytes) with e_type=ET_DYN, e_machine=EM_386,
    plus 8 bytes padding for fuzzer mutation headroom.
    """
    elf = bytearray()

    # e_ident[16]: ELF identification
    elf.extend(b'\x7fELF')  # EI_MAG: magic number
    elf.append(1)           # EI_CLASS: ELFCLASS32
    elf.append(1)           # EI_DATA: ELFDATA2LSB (little-endian)
    elf.append(1)           # EI_VERSION: EV_CURRENT
    elf.append(0)           # EI_OSABI: ELFOSABI_NONE
    elf.extend(b'\x00' * 8) # EI_PAD: padding

    # ELF header fields
    elf.extend(struct.pack('<H', 3))      # e_type: ET_DYN (PIE)
    elf.extend(struct.pack('<H', 3))      # e_machine: EM_386
    elf.extend(struct.pack('<I', 1))      # e_version: EV_CURRENT
    elf.extend(struct.pack('<I', 0))      # e_entry: entry point
    elf.extend(struct.pack('<I', 0))      # e_phoff: program header offset
    elf.extend(struct.pack('<I', 0))      # e_shoff: section header offset
    elf.extend(struct.pack('<I', 0))      # e_flags
    elf.extend(struct.pack('<H', 0))      # e_ehsize
    elf.extend(struct.pack('<H', 0))      # e_phentsize
    elf.extend(struct.pack('<H', 0))      # e_phnum
    elf.extend(struct.pack('<H', 0))      # e_shentsize
    elf.extend(struct.pack('<H', 0))      # e_shnum
    elf.extend(struct.pack('<H', 0))      # e_shstrndx

    # Padding for fuzzer mutation headroom
    elf.extend(b'\x00' * 8)

    return bytes(elf)


def main():
    os.chdir(SCRIPT_DIR)

    elf64 = generate_minimal_elf64()
    elf32 = generate_minimal_elf32()

    with open('minimal64.elf', 'wb') as f:
        f.write(elf64)
    print(f"Generated minimal64.elf ({len(elf64)} bytes)")

    with open('minimal32.elf', 'wb') as f:
        f.write(elf32)
    print(f"Generated minimal32.elf ({len(elf32)} bytes)")


if __name__ == '__main__':
    main()
