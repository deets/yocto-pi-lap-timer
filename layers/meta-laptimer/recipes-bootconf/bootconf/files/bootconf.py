#!/usr/bin/python3
import os
import argparse
import logging.handlers
import asyncio
import asyncio_glib
import dbus.mainloop.glib
import pathlib
import wpasupplicant
import configfiltering
import stat

logger = logging.getLogger("bootconf")

WIFI_INTERFACE = "wlan0"

UDHCPD_CONF_FILENAME = pathlib.Path("/etc/udhcpd.conf")
assert UDHCPD_CONF_FILENAME.exists()

AUTHORIZED_KEYS = "/home/root/.ssh/authorized_keys"


def safe_get(d, *path, default=None):
    sentinel = object()
    for p in path:
        d = d.get(p, sentinel)
        if d is sentinel:
            return default
    return d


def with_dbus(func):
    async def _w(*args, **kwargs):
        p = pathlib.Path("/var/run/dbus/system_bus_socket")
        while not p.exists():
            logger.info("No system DBUS found")
            await asyncio.sleep(1)

        await func(*args, **kwargs)

    return _w


async def setup_ap(ifname, hostname, network_config):
    logger.info("Setting up Access Point")
    bus = dbus.SystemBus()
    wpa_supplicant = wpasupplicant.WPASupplicant(bus, ifname)
    ssid = safe_get(network_config, "ssid", default=hostname)
    frequency = safe_get(
        network_config,
        "ap", "frequency",
        default=2437,
    )
    wpa_supplicant.run_access_point(ssid, frequency)

    # We have to assign an IP to the interface, and start the udhcpd
    # daemon. We currently only allow hard-coded values for IPs and
    # ranges, to keep things simple.
    proc = await asyncio.create_subprocess_exec(
        "ifconfig", WIFI_INTERFACE, "192.168.0.1"
    )
    await proc.wait()
    # it seems we have to wait a bit before starting
    # udhcpd
    await asyncio.sleep(5)
    proc = await asyncio.create_subprocess_exec(
        "udhcpd", "-S",  str(UDHCPD_CONF_FILENAME)
    )
    await proc.wait()  # this should go to the backround
    logger.info("AP set up")


async def setup_station(ifname, network_config):
    logger.info("Setting up Station")
    station_config = network_config["station"]
    logger.debug("Station  config: %r", station_config)
    bus = dbus.SystemBus()
    networks = [
        (
            safe_get(entry, "ssid", default="undefined-ssid"),
            safe_get(entry, "password", default=None),
        )
        for entry in station_config
    ]
    logger.debug("networks: %r", networks)
    wpa_supplicant = wpasupplicant.WPASupplicant(bus, ifname)
    wpa_supplicant.run_station(networks)


@with_dbus
async def setup_network(ifname, hostname, network_config):
    # always prioritise ap!
    if "ap" in network_config:
        await setup_ap(ifname, hostname, network_config)
    elif "station" in network_config:
        await setup_station(ifname, network_config)


async def setup_authorized_keys(authorized_keys):
    if authorized_keys:
        with open(AUTHORIZED_KEYS, "w") as outf:
            outf.writelines(authorized_keys + [""])
        os.chmod(AUTHORIZED_KEYS, stat.S_IRUSR | stat.S_IWUSR)


async def setup_hostname(hostname):
    proc = await asyncio.create_subprocess_exec(
        "hostname", hostname,
    )
    await proc.wait()


def setup_mainloop():
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
    asyncio.set_event_loop_policy(asyncio_glib.GLibEventLoopPolicy())
    loop = asyncio.new_event_loop()
    asyncio.set_event_loop(loop)
    return loop


def setup_logging(loglevel):
    root = logging.getLogger()
    root.addHandler(logging.handlers.SysLogHandler("/dev/log"))
    root.setLevel(loglevel)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("config")
    opts = parser.parse_args()
    config = configfiltering.configuration_from_file_filtered_by_selection(
        opts.config
    )
    loglevel = safe_get(config, "loglevel", default="DEBUG")
    loglevel = getattr(logging, loglevel.upper())
    setup_logging(loglevel)


    logger.debug("config %r", config)
    loop = setup_mainloop()

    loop.create_task(
        setup_authorized_keys(
            safe_get(config, "authorized-keys", default=[])
        )
    )
    loop.create_task(
        setup_hostname(
            safe_get(config, "hostname", default="fpv-laptimer")
        )
    )
    loop.create_task(
        setup_network(
            WIFI_INTERFACE,
            config["hostname"],
            safe_get(config, "network", WIFI_INTERFACE)
        )
    )
    loop.run_forever()


if __name__ == '__main__':
    main()
