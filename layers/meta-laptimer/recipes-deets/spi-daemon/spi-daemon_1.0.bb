DESCRIPTION = "spi daemon "
SECTION = "devtools"
LICENSE = "MIT"
FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

LIC_FILES_CHKSUM = "file://LICENSE;md5=eecac97884d7374757727f363dfede69"

SRC_URI = "\
    file://LICENSE \
    file://CMakeLists.txt \
    file://LICENSE \
    file://src/atomicops.h \
    file://src/datagram.cpp \
    file://src/datagram.h \
    file://src/readerwriterqueue.h \
    file://src/realtime.cpp \
    file://src/realtime.h \
    file://src/spi-daemon.cpp \
    file://src/tx.cpp \
    file://src/tx.h \
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
