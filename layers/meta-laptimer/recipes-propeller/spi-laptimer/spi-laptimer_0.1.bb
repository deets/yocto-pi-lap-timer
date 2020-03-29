SUMMARY = "Upload the propeller binary to the propeller at system-start"
LICENSE = "CLOSED"

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

DEPENDS += "propman"

inherit pkgconfig update-rc.d

SRC_URI = "\
    file://spi-laptimer-upload.sh\
    file://spi-laptimer.binary\
"

FILES_${PN} += "\
    ${datadir}/spi-laptimer/spi-laptimer.binary\
"

INITSCRIPT_NAME = "spi-laptimer-upload"
INITSCRIPT_PARAMS = "start 50 5 3 2 ."

do_install() {
    # start script
    install -d ${D}${sysconfdir}/init.d
    install -m 0755 ${WORKDIR}/spi-laptimer-upload.sh ${D}${sysconfdir}/init.d/spi-laptimer-upload

    # binary
    install -d ${D}${datadir}/spi-laptimer
    install -m 0555 ${WORKDIR}/spi-laptimer.binary ${D}${datadir}/spi-laptimer/spi-laptimer.binary
}
