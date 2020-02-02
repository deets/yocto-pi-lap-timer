#!/usr/bin/env python3
# -*- mode: python -*-
# Copyright: 2020, Diez B. Roggisch, Berlin . All rights reserved.
import argparse
import json
import logging
import os
import subprocess
import sys

USAGE = """Images containing several partitions are
cumbersome to inspect. You need to figure out partition block ranges,
and mount these using the loopback device.

This script encapsulates this in a convenient package and prints out
the needed umount commands."""


def collect_partitions_dims(image):
    p = subprocess.run(
        [
            "fdisk",
            "-usectors",
            "-l", image,
        ],
        check=True,
        stdout=subprocess.PIPE,
    )

    def safe_int(value):
        try:
            return int(value)
        except ValueError:
            return None

    partitions = [
        line for line in p.stdout.decode("ascii").split("\n")
        if line.startswith(image)
    ]
    dims = [
        [safe_int(v) for v in p.split() if safe_int(v) is not None][0:3:2]
        for p in partitions
    ]
    return dims


def parse_args():
    parser = argparse.ArgumentParser(usage=USAGE)
    parser.add_argument("image", help="The disk image to mount")
    parser.add_argument(
        "--json",
        action="store_true",
        help="When given, write JSON data with mountpoints instead of umount commands",
    )
    parser.add_argument(
        "--prefix",
        help="Where to mount the partitions - defaults to /media",
        default="/media",
    )
    parser.add_argument(
        "--name",
        help="Name of the partitions (will be appended with p0, p1, ..)."
        " Defaults to the basename of the image.",
    )
    parser.add_argument(
        "--sudo",
        action="store_true",
        help="Prepend 'sudo' to the mount commands executed by this script",
    )
    parser.add_argument(
        "-n", "--dry-run", action="store_true", help="Don't actually mount, only pretend"
    )
    parser.add_argument(
        "-v", "--verbose", action="store_true", help="More verbose output."
    )
    return parser.parse_args()


def main():
    opts = parse_args()
    logging.basicConfig(
        level=logging.DEBUG if opts.verbose else logging.INFO,
        stream=sys.stderr,
    )

    if opts.name is None:
        opts.name = os.path.splitext(os.path.basename(opts.image))[0]

    partition_dims = collect_partitions_dims(opts.image)
    mountpoints = []
    pending_error = None

    for i, (offset, size) in enumerate(partition_dims):
        logging.debug(f"partition {i}-> offset: {offset}, size: {size}")
        mountpoint = "{}/{}p{}".format(opts.prefix, opts.name, i)
        if not os.path.exists(mountpoint) and not opts.dry_run:
            os.mkdir(mountpoint)
        cmd = [
            "mount",
            "-o",
            "rw,loop,offset={},sizelimit={}".format(offset * 512, size * 512),
            "-t",
            "auto",
            opts.image,
            mountpoint,
        ]
        if opts.sudo:
            cmd.insert(0, "sudo")
        try:
            if not opts.dry_run:
                subprocess.run(cmd, check=True)
                mountpoints.append(mountpoint)
            else:
                logging.debug(" ".join(cmd))
        except subprocess.CalledProcessError as e:
            logging.exception("Failed to mount partition p%d", i)
            pending_error = e

    if opts.json:
        print(json.dumps(mountpoints))
    else:
        for x in mountpoints:
            print(f"{'sudo' if opts.sudo else ''} umount {x}")

    if pending_error:
        logging.error("One or more partitions failed to mount")
        sys.exit(1)


if __name__ == "__main__":
    main()
