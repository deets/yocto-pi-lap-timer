# -*- coding: utf-8 -*-
# Copyright: 2020, Diez B. Roggisch, Berlin . All rights reserved.
import sys
import pathlib
import io

layer_files_directory = pathlib.Path(__file__).parent.parent / \
    "layers" / "meta-laptimer" / "recipes-bootconf" \
    / "bootconf" / "files"

assert layer_files_directory.exists()

sys.path.append(str(layer_files_directory))

from configfiltering import filter_config, load_conf

TEST_CONF = {
    # A key with no qualification
    "always": True,
    "network": {
        "wlan0": {
            "ap:0": {
                "frequency": 2437,
            },
            "station:1": [
                {
                    "ssid": "my-network",
                    "password": "password",
                },
            ]
        }
    }
}


def test_config_0():
    filtered_conf = filter_config(TEST_CONF, "0")
    assert filtered_conf == {
        "always": True,
        "network": {
            "wlan0": {
                "ap": {
                    "frequency": 2437,
                },
            }
        }
    }


def test_config_1():
    filtered_conf = filter_config(TEST_CONF, "1")
    assert filtered_conf == {
        "always": True,
        "network": {
            "wlan0": {
                "station": [
                    {
                        "ssid": "my-network",
                        "password": "password",
                    },
                ]
            }
        }
    }


def test_config_loading_from_io_buffer():
    config_contents = """{
    "always": True,
    "network": {
        "wlan0": {
            "ap:0": {
                "frequency": 2437,
            },
            "station:1": [
                {
                    "ssid": "my-network",
                    "password": "password",
                },
            ]
        }
    }
    }"""
    assert load_conf(io.StringIO(config_contents)) == TEST_CONF


def test_config_loading_from_file(tmp_path):
    config_contents = """{
    "always": True,
    "network": {
        "wlan0": {
            "ap:0": {
                "frequency": 2437,
            },
            "station:1": [
                {
                    "ssid": "my-network",
                    "password": "password",
                },
            ]
        }
    }
    }"""
    config_path = tmp_path / "system.conf"
    with config_path.open("w") as outf:
        outf.write(config_contents)

    assert load_conf(config_path) == TEST_CONF
