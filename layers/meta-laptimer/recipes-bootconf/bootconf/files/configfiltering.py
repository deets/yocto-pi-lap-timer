#!/usr/bin/env python3.7
# -*- coding: utf-8 -*-
# Copyright: 2020, Diez B. Roggisch, Berlin . All rights reserved.
import io
import ast
import re
import gpiozero

from gpiozero.pins.pigpio import PiGPIOFactory


def parse_config_number_spec(spec):
    pins = []
    for definition in spec.split(","):
        definition = definition.strip()
        m = re.match(r"(\d+)(L|H)", definition)
        assert m
        pin, active = m.groups()
        pull_up = active == "L"
        pins.append(
            gpiozero.Button(
                int(pin),
                pull_up=pull_up,
            )
        )
    return pins


def evaluate_config_number(pins):
    return sum(pin.value << i for i, pin in enumerate(pins))


def load_conf(configfile):
    if isinstance(configfile, io.IOBase):
        content = configfile.read()
    else:
        with open(configfile) as inf:
            content = inf.read()
    return ast.literal_eval(content)


def filter_config(config, active_config):
    def key_in_config(key):
        try:
            key, config = key.rsplit(":", 1)
            return any(active_config == c for c in config)
        except ValueError:
            return True

    def normalise(key):
        try:
            key, _ = key.rsplit(":", 1)
            return key
        except ValueError:
            return key

    if isinstance(config, dict):
        return {
            normalise(key): filter_config(value, active_config)
            for key, value in config.items()
            if key_in_config(key)
        }
    else:
        return config


if __name__ == '__main__':
    import sys
    import pprint
    gpiozero.Device.pin_factory = PiGPIOFactory()
    buf = io.StringIO(sys.stdin.read())
    active_config = sys.argv[1]
    pins = parse_config_number_spec("21H,20H")
    no = evaluate_config_number(pins)
    print(no)

    pprint.pprint(filter_config(load_conf(buf), active_config))
