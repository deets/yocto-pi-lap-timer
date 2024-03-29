LICENSE = "CLOSED"

FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI:append = " \
    file://bootconf.py\
    file://wpasupplicant.py\
    file://configfiltering.py\
    file://udhcpd.conf\
    file://autostart-bootconf.sh\
"

RDEPENDS:${PN} = "\
   python3-asyncio-glib\
   python3-core\
   python3-dbus\
   python3-debugger\
   python3-misc\
   python3-pygobject\
   python3-gpiozero \
"

inherit update-rc.d

INITSCRIPT_NAME = "bootconf"

# The script parameters were adapted from D-Bus, as it is a dependency for Bootconf
INITSCRIPT_PARAMS = "start 03 5 3 2 . stop 97 0 1 6 ."


FILES:${PN}:append = " /opt/bootconf/* /home/root/.ssh"

do_install:append () {
    install -d ${D}/opt/bootconf
    install -m 0755 ${WORKDIR}/*.py ${D}/opt/bootconf
    install -d ${D}${sysconfdir}
    install ${WORKDIR}/udhcpd.conf ${D}/etc
    install -d ${D}${sysconfdir}/init.d
    install -m 0755 ${WORKDIR}/autostart-bootconf.sh ${D}${sysconfdir}/init.d/bootconf
    install -m 0700 -d ${D}/home ${D}/home/root ${D}/home/root/.ssh
}
