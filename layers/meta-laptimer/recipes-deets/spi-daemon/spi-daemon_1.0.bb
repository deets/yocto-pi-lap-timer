DESCRIPTION = "spi daemon "
SECTION = "devtools"
LICENSE = "MIT"
FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

LIC_FILES_CHKSUM = "file://LICENSE;md5=eecac97884d7374757727f363dfede69"

SRC_URI = "\
    file://*\
    file://LICENSE\
    "

S="${WORKDIR}"

inherit cmake pkgconfig
#inherit update-rc.d

FILES_${PN} += "${bindir}/spi-daemon\
"

DEPENDS = "nanomsg"

#INITSCRIPT_NAME = "spi-daemon-init"
#INITSCRIPT_PARAMS = "start 10 5 3 2 . stop 90 0 1 6 ."

do_install() {
    install -d ${D}${bindir}
    install -m 0755 spi-daemon ${D}${bindir}
}
