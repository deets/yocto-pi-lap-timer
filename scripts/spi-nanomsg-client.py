# -*- coding: utf-8 -*-
# Copyright: 2020, Diez B. Roggisch, Berlin . All rights reserved.
import sys
import statistics
import time

import nanomsg


class TSAnalyser:

    def __init__(self):
        #self._diffs = []
        self._spi_diffs = []
        self._last = None
        self._max = 0
        self._spi_max = 0

    def feed(self, diff, spi_diff):
        d = diff / 80_000_000
        #self._diffs.append(d)
        #self._diffs = self._diffs[-1000:]
        self._max = max(self._max, d)
        self._spi_max = max(self._spi_max, spi_diff / 1_000_000)

    def stats(self):
        # if len(self._diffs) > 2:
        #     return self._diffs[-1], self._max, self._spi_max # statistics.mean(self._diffs),
        return (self._max, self._spi_max)


def process_message(msg):
    for line in msg.decode("ascii").split("\n"):
        line = line.strip()
        if line.startswith("S"):
            parts = line.split(":")
            yield int(parts[1]), int(parts[2])
        elif line.startswith("D"):
            print(line)


def main():
    ts_analyser = TSAnalyser()
    socket = nanomsg.Socket(nanomsg.PAIR)
    socket.connect(sys.argv[1])
    start = time.monotonic()
    while True:
        msg = socket.recv()
        for diff, spi_diff in process_message(msg):
            ts_analyser.feed(diff, spi_diff)
        #print("max ts diff, max sample diffs:", ts_analyser.stats(), f"{time.monotonic() - start:+.2f}")

if __name__ == '__main__':
    main()
