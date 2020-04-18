# -*- coding: utf-8 -*-
# Copyright: 2020, Diez B. Roggisch, Berlin . All rights reserved.
import pathlib
import sys

from gpiozero.pins.mock import MockFactory
from gpiozero import Device, Button


# Set the default pin factory to a mock factory
Device.pin_factory = MockFactory()

layer_files_directory = pathlib.Path(__file__).parent.parent / \
    "layers" / "meta-laptimer" / "recipes-bootconf" \
    / "bootconf" / "files"

assert layer_files_directory.exists()

sys.path.append(str(layer_files_directory))

from configfiltering import parse_config_number_spec, evaluate_config_number



def test_active_low_configuration_number():
    pins = parse_config_number_spec("21L,20L")
    btn_pin = Device.pin_factory.pin(21)
    btn_pin.drive_high()
    btn_pin = Device.pin_factory.pin(20)
    btn_pin.drive_low()
    no = evaluate_config_number(pins)
    assert(no == 2)


def test_active_high_configuration_number():
    pins = parse_config_number_spec("21H,20H")
    btn_pin = Device.pin_factory.pin(21)
    btn_pin.drive_low()
    btn_pin = Device.pin_factory.pin(20)
    btn_pin.drive_high()
    no = evaluate_config_number(pins)
    assert(no == 2)
