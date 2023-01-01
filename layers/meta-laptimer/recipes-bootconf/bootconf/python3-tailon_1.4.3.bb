SUMMARY = "Python tailon daemon"
SECTION = "devel/python"
HOMEPAGE = "https://github.com/gvalkov/tailon"
LICENSE = "Apache-2"
LIC_FILES_CHKSUM = "file://LICENSE;md5=521a47d7ba3fef1505c0d8d77ba2a1b8"
FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

inherit setuptools3 pypi deploy update-rc.d

SRC_URI[md5sum] = "72b3c051147f53b96567b35a31c73608"
SRC_URI[sha256sum] = "8cbb0406f9afdd96c93cc8947d0fb4fda534ce3e465794314f4cd3f877cc7576"

INITSCRIPT_NAME = "tailon"
INITSCRIPT_PARAMS = "start 10 5 3 2 . stop 90 0 1 6 ."
SRC_URI:append = "\
    file://tailon\
    "

do_install:append() {
    install -Dm 0755 ${WORKDIR}/tailon ${D}${sysconfdir}/init.d/tailon
}

RDEPENDS:${PN}:append = " python3-pyyaml python3-sockjs-tornado python3-tornado-http-auth python3-deepmerge"

BBCLASSEXTEND = "native nativesdk"
