DESCRIPTION = "stress-ng will stress test a computer system in various selectable ways. It \
was designed to exercise various physical subsystems of a computer as well as the various \
operating system kernel interfaces. \
\
Lifted from https://github.com/facebook/openbmc\
"
HOMEPAGE = "http://kernel.ubuntu.com/~cking/stress-ng/"

LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=b234ee4d69f5fce4486a80fdaf4a4263"

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"
SRC_URI = "https://kernel.ubuntu.com/~cking/tarballs/stress-ng/stress-ng-${PV}.tar.xz \
           file://Makefile.patch \
           "

SRC_URI[md5sum] = "75b1e29ce20f6c9b5370a6db1204a2f8"
SRC_URI[sha256sum] = "db2196fd9d37ed6ba12aead8ca10977e4bf1cb05b273faf3b3c5e98ed154ac3e"

S = "${WORKDIR}/stress-ng-${PV}"

do_install_append() {
    install -d ${D}${bindir}
    install -m 755 ${S}/stress-ng ${D}${bindir}/stress-ng
}
