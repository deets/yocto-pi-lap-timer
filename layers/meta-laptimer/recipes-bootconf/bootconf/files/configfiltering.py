#!/usr/bin/env python3.7
# -*- coding: utf-8 -*-
# Copyright: 2020, Diez B. Roggisch, Berlin . All rights reserved.
import io
import ast
import re
import gpiozero
import logging
from collections import namedtuple

from gpiozero.pins.pigpio import PiGPIOFactory


logger = logging.getLogger(__name__)


NopButton = namedtuple("NopButton", "value")


def parse_config_number_spec(spec):
    try:
        logger.debug("parse_config_number_spec %s", spec)
        pins = []
        for definition in spec.split(","):
            definition = definition.strip()
            m = re.match(r"^(\d+)(L|H)$", definition)
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
    except:
        logger.exception("Error reading configuration spec %r", spec)
        return [NopButton(0)]


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
    active_config = str(active_config)

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


def configuration_from_file_filtered_by_selection(config_file_or_buffer):
    gpiozero.Device.pin_factory = PiGPIOFactory()
    raw_config = load_conf(config_file_or_buffer)
    spec = raw_config.get("config-selector")
    pins = parse_config_number_spec(spec)
    active_config = evaluate_config_number(pins)
    logger.debug("active config: %i", active_config)
    return filter_config(raw_config, active_config)


if __name__ == '__main__':
    import sys
    import pprint
    logging.basicConfig(level=logging.DEBUG)
    buf = io.StringIO(sys.stdin.read())
    pprint.pprint(configuration_from_file_filtered_by_selection(buf))
