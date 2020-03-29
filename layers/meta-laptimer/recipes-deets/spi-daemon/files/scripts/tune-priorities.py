#!/usr/bin/python3
import subprocess

FRAGMENTS = [
    ("[spi0]", 97),
    ("3f204000", 98),
    ]


def main():
    p = subprocess.run(["ps"], capture_output=True, check=True)
    for line in p.stdout.decode("ascii").split("\n"):
        for fragment, priority in FRAGMENTS:
            if fragment in line:
                pid = line.strip().split()[0]
                subprocess.run(
                    ["chrt", "-p", str(priority), pid],
                    check=True,
                    capture_output=True,
                )


if __name__ == '__main__':
    main()
