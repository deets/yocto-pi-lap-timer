SUMMARY = "The RotorHazard FPV Timer"
HOMEPAGE = "https://github.com/RotorHazard/RotorHazard"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=4a2e7d8803dc38c927d4dc23b4cd3053"

SRC_URI = "git://github.com/deets/RotorHazard.git;branch=master"
SRCREV = "90d0988ecda67e49b143340e05c0a1f7a0752f5e"

SRC_URI[md5sum] = "0e3ed44ece1c489ed835d1b7047e349c"
SRC_URI[sha256sum] = "13f9f196f330c7c2c5d7a5cf91af894110ca0215ac051b5844701f2bfd934d52"

RDEPENDS_${PN} += "python python-flask"

# This is needed because the subdirectory otherwise is PN-git
S = "${WORKDIR}/git"

FILES_${PN} += "/home/pi/RotorHazard/src"

do_install() {
    install -d ${D}/home/pi/RotorHazard/src
    cp -r ${S}/src ${D}/home/pi/RotorHazard/
}
