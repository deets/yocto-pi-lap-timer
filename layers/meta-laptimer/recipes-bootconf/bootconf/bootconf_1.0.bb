LICENSE = "CLOSED"

FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SRC_URI_append = " \
    file://bootconf.py\
"

RDEPENDS_${PN} = "\
   python3-core\
   python3-debugger\
   python3-dbus\
   python3-pygobject\
"

do_install_append () {
    install -d ${D}/usr/bin
    install ${WORKDIR}/bootconf.py ${D}/usr/bin
}
