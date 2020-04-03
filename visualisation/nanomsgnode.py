# -*- coding: utf-8 -*-
# Copyright: 2020, Diez B. Roggisch, Berlin . All rights reserved.
import nanomsg

CHANNEL_NAMES = (
    "E4", "R1", "E3", "E2", "R2", "E1", "A8", "R3",
    "B1", "F1", "A7", "B2", "F2", "A6", "R4", "B3",
    "F3", "A5", "B4", "F4", "A4", "R5", "B5", "F5",
    "A3", "B6", "F6", "R6", "A2", "B7", "F7", "A1",
    "B8", "F8", "R7", "E5", "E6", "R8", "E7", "E8",
)


class PropellerNodeController:

    def __init__(self, uri):
        self._socket = nanomsg.Socket(nanomsg.PAIR)
        self._socket.connect(uri)

    def configuration(self):
        return 1, None

    def readlines(self):
        msg = self._socket.recv()
        for line in msg.decode("ascii").split("\n"):
            line = line.strip()
            if line.startswith("D"):
                yield self._transform_to_laptime(line)

    def _transform_to_laptime(self, line):
        """
        D00000008:845274c1:00000091:84527e21:845287a1:84529121:84529aa1:8452a421:8452ada1
        """
        parts = line.split(":")[1:3]
        return f"l{parts[0]}:{parts[1]}"


    def tune(self, node_number, channel_name):
        pass
