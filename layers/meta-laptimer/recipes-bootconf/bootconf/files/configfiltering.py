#!/usr/bin/env python3.7
# -*- coding: utf-8 -*-
# Copyright: 2020, Diez B. Roggisch, Berlin . All rights reserved.
import io
import ast


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
    buf = io.StringIO(sys.stdin.read())
    active_config = sys.argv[1]
    pprint.pprint(filter_config(load_conf(buf), active_config))
