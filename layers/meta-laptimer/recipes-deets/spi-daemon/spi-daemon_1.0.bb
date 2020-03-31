DESCRIPTION = "spi daemon "
SECTION = "devtools"
LICENSE = "MIT"
FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

LIC_FILES_CHKSUM = "file://LICENSE;md5=eecac97884d7374757727f363dfede69"

SRC_URI = "\
    file://LICENSE \
    file://CMakeLists.txt \
    file://LICENSE \
    file://src/tx.cpp \
    file://src/spi-daemon.cpp \
    file://src/datagram.h \
    file://src/atomicops.h \
    file://src/readerwriterqueue.h \
    file://src/tx.h \
    file://src/datagram.cpp \
    "

S="${WORKDIR}"

inherit cmake pkgconfig

FILES_${PN} += "\
    ${bindir}/spi-daemon \
"

DEPENDS = "nanomsg"

do_install_append() {
    install -d ${D}${bindir}
    install -m 0755 spi-daemon ${D}${bindir}
}
