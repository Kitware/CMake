import os
import sys


def child(stamp):
    data = bytearray(64 * 1024 * 1024)
    data[::4096] = b"x" * ((len(data) + 4095) // 4096)
    open(stamp, "ab").close()
    return 0


def main():
    if len(sys.argv) == 3 and sys.argv[1] == "--child":
        return child(sys.argv[2])

    # Allocate in a separate child process, so that the reported metrics
    # must cover the wrapped process tree rather than only this process.
    args = [sys.executable, os.path.abspath(__file__), "--child", sys.argv[1]]
    if os.name == "nt":
        # os.spawnv joins arguments with spaces on Windows without quoting.
        args = ['"{}"'.format(a) for a in args]
    return os.spawnv(os.P_WAIT, sys.executable, args)


if __name__ == "__main__":
    sys.exit(main())
