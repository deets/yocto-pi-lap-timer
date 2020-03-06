import dbus
import urllib
import subprocess
from dbus.mainloop.glib import DBusGMainLoop
from gi.repository import GLib


WPAS_DBUS_SERVICE = "fi.w1.wpa_supplicant1"
WPAS_DBUS_INTERFACE = "fi.w1.wpa_supplicant1"
WPAS_DBUS_OPATH = "/fi/w1/wpa_supplicant1"
WPAS_DBUS_INTERFACES_INTERFACE = "fi.w1.wpa_supplicant1.Interface"
WPAS_DBUS_INTERFACES_OPATH = "/fi/w1/wpa_supplicant1/Interfaces"
WPAS_DBUS_BSS_INTERFACE = "fi.w1.wpa_supplicant1.BSS"
WPAS_DBUS_NETWORK_INTERFACE = "fi.w1.wpa_supplicant1.Network"


def byte_array_to_string(s):
    r = ""
    for c in s:
        if c >= 32 and c < 127:
            r += "%c" % c
        else:
            r += urllib.quote(chr(c))
    return r


def list_interfaces(wpas_obj):
    ifaces = wpas_obj.Get(WPAS_DBUS_INTERFACE, 'Interfaces',
                          dbus_interface=dbus.PROPERTIES_IFACE)
    for path in ifaces:
        if_obj = bus.get_object(WPAS_DBUS_SERVICE, path)
        ifname = if_obj.Get(WPAS_DBUS_INTERFACES_INTERFACE, 'Ifname',
                            dbus_interface=dbus.PROPERTIES_IFACE)
        print(ifname)


def show_bss(bss, bus):
    net_obj = bus.get_object(WPAS_DBUS_SERVICE, bss)
    # Convert the byte-array for SSID and BSSID to printable strings
    val = net_obj.Get(WPAS_DBUS_BSS_INTERFACE, 'BSSID',
                      dbus_interface=dbus.PROPERTIES_IFACE)
    bssid = ""
    for item in val:
        bssid = bssid + ":%02x" % item
    bssid = bssid[1:]
    val = net_obj.Get(WPAS_DBUS_BSS_INTERFACE, 'SSID',
                      dbus_interface=dbus.PROPERTIES_IFACE)
    ssid = byte_array_to_string(val)
    val = net_obj.Get(WPAS_DBUS_BSS_INTERFACE, 'WPA',
                      dbus_interface=dbus.PROPERTIES_IFACE)
    wpa = "no"
    if len(val["KeyMgmt"]) > 0:
        wpa = "yes"
    val = net_obj.Get(WPAS_DBUS_BSS_INTERFACE, 'RSN',
              dbus_interface=dbus.PROPERTIES_IFACE)
    wpa2 = "no"
    if len(val["KeyMgmt"]) > 0:
        wpa2 = "yes"
    freq = net_obj.Get(WPAS_DBUS_BSS_INTERFACE, 'Frequency',
                       dbus_interface=dbus.PROPERTIES_IFACE)
    signal = net_obj.Get(WPAS_DBUS_BSS_INTERFACE, 'Signal',
                         dbus_interface=dbus.PROPERTIES_IFACE)
    val = net_obj.Get(WPAS_DBUS_BSS_INTERFACE, 'Rates',
                      dbus_interface=dbus.PROPERTIES_IFACE)
    if len(val) > 0:
        maxrate = val[0] / 1000000
    else:
        maxrate = 0
    print("  %s  ::  ssid='%s'  wpa=%s  wpa2=%s  signal=%d  rate=%d  freq=%d" %
          (bssid, ssid, wpa, wpa2, signal, maxrate, freq))


class WPASupplicant:

    def __init__(self, bus, ifname):
        self._bus = bus
        self._ifname = ifname

        wpas_obj = bus.get_object(WPAS_DBUS_SERVICE, WPAS_DBUS_OPATH)
        self._wpas = dbus.Interface(wpas_obj, WPAS_DBUS_INTERFACE)

        bus.add_signal_receiver(
            self.scan_done,
            dbus_interface=WPAS_DBUS_INTERFACES_INTERFACE,
            signal_name="ScanDone"
        )
        bus.add_signal_receiver(
            self.bss_added,
            dbus_interface=WPAS_DBUS_INTERFACES_INTERFACE,
            signal_name="BSSAdded"
        )
        bus.add_signal_receiver(
            self.bss_removed,
            dbus_interface=WPAS_DBUS_INTERFACES_INTERFACE,
            signal_name="BSSRemoved"
        )
        bus.add_signal_receiver(
            self.properties_changed,
            dbus_interface=WPAS_DBUS_INTERFACES_INTERFACE,
            signal_name="PropertiesChanged"
        )
        # See if wpa_supplicant already knows about this interface
        path = None
        try:
            path = self._wpas.GetInterface(ifname)
        except dbus.DBusException as exc:
            if not str(exc).startswith(
                "fi.w1.wpa_supplicant1.InterfaceUnknown:"
            ):
                raise
            try:
                path = self._wpas.CreateInterface(
                        {'Ifname': ifname, 'Driver': 'nl80211'}
                )
            except dbus.DBusException as exc:
                if not str(exc).startswith(
                                "fi.w1.wpa_supplicant1.InterfaceExists:"
                ):
                    raise

        self._if_obj = bus.get_object(WPAS_DBUS_SERVICE, path)
        self._iface = dbus.Interface(
                self._if_obj,
                WPAS_DBUS_INTERFACES_INTERFACE
        )
        self._udhcpc = None

    def scan(self):
        self._iface.Scan({'Type': 'active'})

    def scan_done(self, success):
        res = self._if_obj.Get(WPAS_DBUS_INTERFACES_INTERFACE, 'BSSs',
                               dbus_interface=dbus.PROPERTIES_IFACE)
        for opath in res:
            show_bss(opath, self._bus)

    def bss_added(self, bss, properties):
        show_bss(bss, self._bus)

    def bss_removed(self, bss):
        pass

    def properties_changed(self, properties):
        if "State" in properties:
            for key, value in properties.items():
                print(key, value)
            print("PropertiesChanged: State: %s" % (properties["State"]))

    def run_access_point(self, ssid, frequency):
        key_mgmt = "NONE"
        args = {
            'ssid': ssid,
            'key_mgmt': key_mgmt,
            'mode': 2,
            'frequency': frequency
        }

        netw = self._iface.AddNetwork(args)
        self._iface.SelectNetwork(netw)

    def run_station(self, networks):
        self._udhcpc = subprocess.Popen(
            [
                "udhcpc", "-R", "-b", "-p",
                f"/var/run/udhcpc.{self._ifname}.pid",
                "-i", self._ifname
            ]
        )
        for ssid, password in networks:
            self.get_or_add_network(
                ssid,
                password=password,
            )

    def __getitem__(self, property_):
        """
        To read properties, use self["Propertyname"], e.g.
        self["Networks"]
        """
        return self._if_obj.Get(
            WPAS_DBUS_INTERFACES_INTERFACE,
            property_,
            dbus_interface=dbus.PROPERTIES_IFACE
        )

    def list_networks(self):
        for network_path in self["Networks"]:
            yield self._bus.get_object(WPAS_DBUS_SERVICE, network_path)

    def get_or_add_network(self, ssid, *, password=None):
        for network in self.list_networks():
            nw_props = network.Get(
                WPAS_DBUS_NETWORK_INTERFACE,
                "Properties",
                dbus_interface=dbus.PROPERTIES_IFACE
            )
            # this is positively weird, but I get
            # the string back with extra quotes. So they need removal
            if nw_props["ssid"][1:-1] == ssid:
                return network

        d = {"ssid": dbus.String(ssid)}
        if password is not None:
            d["psk"] = dbus.String(password)
        else:
            d["key_mgmt"] = dbus.String("NONE")

        network_path = self._iface.AddNetwork(dbus.Dictionary(d))
        network_object = self._bus.get_object(WPAS_DBUS_SERVICE, network_path)
        network_object.Set(
            WPAS_DBUS_NETWORK_INTERFACE,
            "Enabled", True,
            dbus_interface=dbus.PROPERTIES_IFACE
        )
        return network_object

    def reconnect(self):
        self._iface.Reconnect()


def main():
    # not entirely sure what this is good for
    DBusGMainLoop(set_as_default=True)
    loop = GLib.MainLoop()
    bus = dbus.SystemBus()
    interface = "wlan0"
    wpa_supplicant = WPASupplicant(bus, interface)
    wpa_supplicant.run_station(
        [("TP-LINK_2.4GHz_BBADE9", "51790684")]
    )
    loop.run()


if __name__ == "__main__":
    main()
