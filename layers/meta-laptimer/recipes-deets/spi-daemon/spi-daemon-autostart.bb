DESCRIPTION = "spi daemon "
SECTION = "devtools"
LICENSE = "MIT"
FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

LIC_FILES_CHKSUM = "file://LICENSE;md5=eecac97884d7374757727f363dfede69"

SRC_URI = "\
    file://LICENSE \
    file://scripts/spi-daemon-init.sh \
    file://scripts/tune-priorities.py \
    "

S="${WORKDIR}"

inherit update-rc.d

RDEPENDS_${PN} = "spi-daemon python3-core"

FILES_${PN} += "\
    ${bindir}/tune-priorities.py \
"

INITSCRIPT_NAME = "spi-daemon-init"
INITSCRIPT_PARAMS = "start 80 5 3 2 . stop 01 0 1 6 ."

do_install_append() {
    install -d ${D}${bindir}
    install -m 0755 ${WORKDIR}/scripts/tune-priorities.py ${D}${bindir}

    install -d ${D}${sysconfdir}/init.d
    install -m 0755 ${WORKDIR}/scripts/spi-daemon-init.sh ${D}${sysconfdir}/init.d/spi-daemon-init
}
