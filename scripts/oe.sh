#!/usr/bin/env python3
# -* mode: python -*-
import os
import sys
import signal
import argparse
import pathlib
import time
import logging
import subprocess
from functools import wraps
from contextlib import contextmanager
from shutil import get_terminal_size

IMAGES = [
    "deets-image",
    "rotorhazard-image",
    "laptimer-image",
]

MACHINES = [
    "raspberrypi3",
    "raspberrypi2",
]

BASE = pathlib.Path(__file__).parent.parent.absolute().resolve()
LAYERS = BASE / "layers"
OE_CORE = LAYERS / "modules" / "openembedded-core"
META_OPENEMBEDDED_LAYERS = LAYERS / "modules" / "meta-openembedded"
BITBAKEDIR = LAYERS / "modules" / "bitbake"
LAYERS = [
    str(LAYERS / "modules" / "meta-raspberrypi"),
    str(LAYERS / "modules" / "meta-qt5"),
    str(LAYERS / "meta-laptimer"),
    str(META_OPENEMBEDDED_LAYERS / "meta-oe"),
    str(META_OPENEMBEDDED_LAYERS / "meta-python"),
    str(META_OPENEMBEDDED_LAYERS / "meta-networking"),
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


# Borrowed from Pew.
# See https://github.com/berdario/pew/blob/master/pew/_utils.py#L82
@contextmanager
def temp_environ():
    """Allow the ability to set os.environ temporarily"""
    environ = dict(os.environ)
    try:
        yield
    finally:
        os.environ.clear()
        os.environ.update(environ)


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

    def items(self):
        return self._env.items()


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


def build_sdk(bakery, opts):
    machine, image = opts.machine, opts.image
    bakery.run(
        [
            "bitbake",
            image,
            "-c", "populate_sdk",
        ],
        extra_env=dict(
            MACHINE=machine,
        ),
    )


def choose_shell():
    from shellingham import detect_shell
    return detect_shell()


def bitbake_shell(bakery, opts):
    # These imports are local so the dependencies
    # are only needed if one really wants a shell
    import pexpect
    if "BITBAKE_SHELL_GUARD" in os.environ:
        logging.error("Already in bitbake shell, please exit first.")

    kind, command = choose_shell()
    # Grab current terminal dimensions to replace the hardcoded default
    # dimensions of pexpect.
    dims = get_terminal_size()
    with temp_environ():
        for key, value in bakery.items():
            os.environ[key] = value
        os.environ["BITBAKE_SHELL_GUARD"] = "1"
        os.environ["MACHINE"] = opts.machine
        c = pexpect.spawn(
            command,
            ["-i"],
            dimensions=(dims.lines, dims.columns),
        )

    # Handler for terminal resizing events
    # Must be defined here to have the shell process in its context, since
    # we can't pass it as an argument
    def sigwinch_passthrough(sig, data):
        dims = get_terminal_size()
        c.setwinsize(dims.lines, dims.columns)

    signal.signal(signal.SIGWINCH, sigwinch_passthrough)

    # Interact with the new shell.
    c.interact(escape_character=None)
    c.close()
    return c.exitstatus


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
    parser.add_argument(
        "--image",
        choices=IMAGES,
        default=IMAGES[0],
        help="The image to build.",
    )

    subparsers = parser.add_subparsers(help='sub-command help')
    parser_build = subparsers.add_parser('build', help='Build the image')
    parser_build.set_defaults(func=build_image)

    parser_shell = subparsers.add_parser(
        'shell',
        help='Open bitbake shell. This spawns a new shell '
        'set up to run bitbake.'
    )
    parser_shell.set_defaults(func=bitbake_shell)

    parser_sdk = subparsers.add_parser(
        'sdk',
        help='Build the Yocto SDK'
    )
    parser_sdk.set_defaults(func=build_sdk)
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
