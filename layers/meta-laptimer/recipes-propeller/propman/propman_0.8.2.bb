SUMMARY = "Parallax Propeller proman tool"
LICENSE = "GPL-3.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=784d7dc7357bd924e8d5642892bf1b6b"

DEPENDS += "qtbase qtserialport"

SRC_URI = "git://github.com/parallaxinc/PropellerManager.git;tag=${PV}"

S = "${WORKDIR}/git"

do_install_append() {
    install -d ${D}${bindir}
    install -m 0755 ${S}/bin/propman ${D}${bindir}
    # these are installed by the qmake5 class
    rm -rf ${D}/usr/local
}

FILES_${PN} += "/usr/bin/propman"

inherit qmake5