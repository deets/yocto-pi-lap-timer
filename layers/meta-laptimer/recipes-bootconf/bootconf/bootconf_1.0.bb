LICENSE = "CLOSED"

FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SRC_URI_append = " \
    file://bootconf.py\
    file://wpasupplicant.py\
    file://udhcpd.conf\
"

RDEPENDS_${PN} = "\
   python3-asyncio-glib\
   python3-core\
   python3-dbus\
   python3-debugger\
   python3-misc\
   python3-pygobject\
"

FILES_${PN}_append = " /opt/bootconf/*"

do_install_append () {
    install -d ${D}/opt/bootconf
    install ${WORKDIR}/bootconf.py ${D}/opt/bootconf
    install ${WORKDIR}/wpasupplicant.py ${D}/opt/bootconf
    install -d ${D}/etc
    install ${WORKDIR}/udhcpd.conf ${D}/etc
}
