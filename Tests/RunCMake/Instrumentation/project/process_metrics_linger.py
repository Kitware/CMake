import os
import subprocess
import sys
import time


def grandchild(survived):
    # Outlive the wrapped command, then record that it was not killed.
    time.sleep(2)
    open(survived, "ab").close()
    return 0


def main():
    if len(sys.argv) == 3 and sys.argv[1] == "--grandchild":
        return grandchild(sys.argv[2])

    # Leave a process running after this one exits.  Do not share stdio,
    # so that the instrumentation wrapper does not wait for it.
    subprocess.Popen(
        [sys.executable, os.path.abspath(__file__), "--grandchild", sys.argv[2]],
        stdin=subprocess.DEVNULL,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )
    open(sys.argv[1], "ab").close()
    return 0


if __name__ == "__main__":
    sys.exit(main())
