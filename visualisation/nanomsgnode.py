# -*- coding: utf-8 -*-
# Copyright: 2020, Diez B. Roggisch, Berlin . All rights reserved.
import nanomsg

CHANNELS = {
    "A": [5865, 5845, 5825, 5805, 5785, 5765, 5745, 5725],  # Band A
    "B": [5733, 5752, 5771, 5790, 5809, 5828, 5847, 5866],  # Band B
    "E": [5705, 5685, 5665, 5645, 5885, 5905, 5925, 5945],  # Band E
    "F": [5740, 5760, 5780, 5800, 5820, 5840, 5860, 5880],  # Band F / Airwave
    "R": [5658, 5695, 5732, 5769, 5806, 5843, 5880, 5917],  # Race Band
}


CHANNEL_NAMES = [
    "E4", "R1", "E3", "E2", "R2", "E1", "A8", "R3",
    "B1", "F1", "A7", "B2", "F2", "A6", "R4", "B3",
    "F3", "A5", "B4", "F4", "A4", "R5", "B5", "F5",
    "A3", "B6", "F6", "R6", "A2", "B7", "F7", "A1",
    "B8", "F8", "R7", "E5", "E6", "R8", "E7", "E8",
]

FREQUENCIES = sorted(
    frequency
    for _, frequencies in CHANNELS.items()
    for frequency in frequencies
)


NAME2FREQ = dict(zip(CHANNEL_NAMES, FREQUENCIES))

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
        frequency = NAME2FREQ[channel_name]
        msg = f"T{node_number}:{frequency}"
        print(msg)
        self._socket.send(msg)
