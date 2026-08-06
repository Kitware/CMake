import multiprocessing
import sys


WORK = 4000000


def work():
    x = 0
    for i in range(WORK):
        x += i * i


def main():
    procs = []
    for _ in range(2):
        proc = multiprocessing.Process(target=work)
        proc.start()
        procs.append(proc)

    for proc in procs:
        proc.join()
        if proc.exitcode != 0:
            return proc.exitcode

    open(sys.argv[1], "ab").close()
    return 0


if __name__ == "__main__":
    sys.exit(main())
