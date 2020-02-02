#!/usr/bin/env python3
# -* mode: python -*-
import os
import sys
import argparse
import pathlib
import time
import logging
import subprocess
from functools import wraps

MACHINES = [
    "raspberrypi3",
    "raspberrypi2",
]

BASE = pathlib.Path(__file__).parent.parent.absolute().resolve()
LAYERS = BASE / "layers"
OE_CORE = LAYERS / "modules" / "openembedded-core"
BITBAKEDIR = LAYERS / "modules" / "bitbake"
LAYERS = [
    str(LAYERS / "modules" / "meta-raspberrypi"),
    str(LAYERS / "meta-laptimer"),
]
WHITELIST = (
    "DL_DIR",
    "SSTATE_DIR",
    "KBUILD_BUILD_USER",
    "KBUILD_BUILD_HOST",
    "ABLETON_OS_HOSTNAME",
    "DISTRO_VERSION",
)

logger = logging.getLogger(__name__)


def timed(description):
    def _d(func):
        @wraps(func)
        def _w(*a, **k):
            start = time.monotonic()
            try:
                return func(*a, **k)
            finally:
                print("{}: {:.2} seconds".format(
                    description,
                    time.monotonic() - start
                    )
                )
        return _w
    return _d


def git_describe():
    return subprocess.run(
        ["git", "describe", "--always"],
        check=True,
        stdout=subprocess.PIPE,
        cwd=str(BASE),
    ).stdout.decode("ascii").strip()


class BitBakery:

    def __init__(self, build, oe_env, opts):
        self._build = build
        self._env = oe_env
        for var in WHITELIST:
            self.whitelist_var(var)
        self._env["SSTATE_DIR"] = opts.shared_state
        self._env["DL_DIR"] = opts.dl_dir
        self._env["DISTRO"] = "laptimer-os"
        self._env["DISTRO_VERSION"] = git_describe()
        for d in (opts.shared_state, opts.dl_dir):
            p = pathlib.Path(d)
            p.mkdir(exist_ok=True)

    def whitelist_var(self, var):
        self._env["BB_ENV_EXTRAWHITE"] = "{} {}".format(
            self._env["BB_ENV_EXTRAWHITE"],
            var,
        )

    def run(self, *args, **kwargs):
        env = self._env
        if "extra_env" in kwargs:
            env = dict(env, **kwargs.pop("extra_env"))
        kwargs["env"] = env
        kwargs["check"] = True
        kwargs["cwd"] = str(self._build)
        return subprocess.run(
            *args,
            **kwargs,
        )

    def shell(self):
        pass


def setup_oe_env(build):
    # prime the setupt script to
    # contain our external bitbake
    # as well as the external build
    # dir
    env = dict(
        os.environ,
        BITBAKEDIR=str(BITBAKEDIR),
        BDIR=str(build),
    )

    captured_env = subprocess.run(
        ". ./oe-init-build-env; env",
        stdout=subprocess.PIPE,
        shell=True,
        check=True,
        cwd=OE_CORE,
        env=env
    ).stdout.decode('ascii')
    for line in captured_env.split("\n"):
        try:
            key, _, value = line.partition("=")
        except ValueError:
            pass
        else:
            if key.strip():
                env[key] = value
    assert env["BUILDDIR"] == str(build), env["BUILDDIR"]
    return env


def setup_layers(bakery):
    bakery.run(
        [
            "bitbake-layers",
            "add-layer",
        ] + LAYERS,
    )


def build_image(bakery, opts):
    machine, image = opts.machine, opts.image
    bakery.run(
        [
            "bitbake",
            image,
        ],
        extra_env=dict(
            MACHINE=machine,
        ),
    )


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-v", "--verbose",
        default=0,
        action="count",
        help="Increase verbosity. Can be given several times."
    )
    parser.add_argument(
        "--shared-state",
        default=os.path.expanduser(
            os.environ.get("OE_SSTATE_DIR")
        ),
        help="Shared state dir"
    )
    parser.add_argument(
        "--dl-dir",
        default=os.path.expanduser(
            os.environ.get("OE_DL_DIR")
        ),
        help="Package download dir"
    )
    parser.add_argument(
        "--machine",
        default=MACHINES[0],
        choices=MACHINES,
        help="Which PI to build for."
    )
    parser.add_argument(
        "--build-dir",
        default=str(BASE / "build"),
        help="Where to build things in."
    )

    subparsers = parser.add_subparsers(help='sub-command help')
    parser_build = subparsers.add_parser('build', help='Build the image')
    parser_build.set_defaults(func=build_image)
    parser_build.add_argument(
        "--image",
        default="laptimer-image",
        help="The image to build.",
    )
    return parser.parse_args()


@timed("OE build took")
def main():
    opts = parse_args()
    logging.basicConfig(
        level=logging.DEBUG if opts.verbose >= 1 else logging.INFO,
    )

    try:
        subcommand = opts.func
    except AttributeError:
        logging.error("Please specify a subcommand! "
                      "Use -h to see the available ones.")
        sys.exit(1)

    build = pathlib.Path(opts.build_dir).absolute().resolve()
    oe_env = setup_oe_env(build)
    bakery = BitBakery(build, oe_env, opts)
    setup_layers(bakery)

    sys.exit(subcommand(bakery, opts))


if __name__ == '__main__':
    main()
