DESCRIPTION = "spi daemon "
SECTION = "devtools"
LICENSE = "MIT"
FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

LIC_FILES_CHKSUM = "file://LICENSE;md5=eecac97884d7374757727f363dfede69"

SRC_URI = "\
    file://LICENSE \
    file://CMakeLists.txt \
    file://LICENSE \
    file://tests/tst_parser.cpp \
    file://include/catch.hpp \
    file://include/cxxopts.hpp \
    file://src/realtime.cpp \
    file://src/rotorhazard.hpp \
    file://src/nanomsg-helper.hpp \
    file://src/tx.cpp \
    file://src/parser.cpp \
    file://src/timetracker.hpp \
    file://src/nanomsg-helper.cpp \
    file://src/hub.hpp \
    file://src/timetracker.cpp \
    file://src/hub.cpp \
    file://src/spi-daemon.cpp \
    file://src/realtime.h \
    file://src/arguments.hpp \
    file://src/tx.hpp \
    file://src/datagram.h \
    file://src/atomicops.h \
    file://src/arguments.cpp \
    file://src/readerwriterqueue.h \
    file://src/rotorhazard.cpp \
    file://src/FastRunningMedian.hpp \
    file://src/datagram.cpp \
    file://src/parser.hpp \
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
