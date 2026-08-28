import os
import sys


def main():
    data = bytearray(64 * 1024 * 1024)
    data[::4096] = b"x" * ((len(data) + 4095) // 4096)
    open(sys.argv[1], "ab").close()
    return 0


if __name__ == "__main__":
    sys.exit(main())
