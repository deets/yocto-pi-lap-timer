DESCRIPTION = "pigpio"
SECTION = "devtools"
LICENSE = "CLOSED"
LIC_FILES_CHKSUM = "file://UNLICENCE"

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI = "git://github.com/joan2937/pigpio.git;protocol=https;tag=${PV}\
    file://0001-Fix-CFLAGS-so-cross-compilation-in-YOCTO-works.patch\
    file://pigpiod.sh\
    "

S = "${WORKDIR}/git"

inherit pkgconfig cmake update-rc.d

RDEPENDS_${PN} = "python3"

FILES_${PN} += "${bindir}/pigpiod\
    ${libdir}/libpigpio.so\
    ${libdir}/libpigpiod_if.so\
    ${libdir}/libpigpiod_if2.so\
    ${libdir}/python3.7/site-packages/pigpio.py\
"

FILES_${PN}-dev = "${includedir}"

INITSCRIPT_NAME = "pigpiod"
INITSCRIPT_PARAMS = "start 01 5 3 2 . stop 99 0 1 6 ."

do_install() {
    install -d ${D}${bindir}
    install -d ${D}${libdir}
    install -d ${D}${libdir}/python3.7/site-packages

    install -m 0755 pigpiod ${D}${bindir}
    install -m 0755 libpigpio.so ${D}${libdir}
    install -m 0755 libpigpiod_if.so ${D}${libdir}
    install -m 0755 libpigpiod_if2.so ${D}${libdir}
    install -m 0644 ${S}/pigpio.py ${D}${libdir}/python3.7/site-packages

    install -d ${D}${sysconfdir}/init.d
    install -m 0755 ${WORKDIR}/pigpiod.sh ${D}${sysconfdir}/init.d/pigpiod
}
