inherit setuptools3

SUMMARY = "Python gpiozero library"
SECTION = "devel/python"
HOMEPAGE = "https://github.com/gpiozero/gpiozero"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://LICENSE.rst;md5=96cd08cadd8a35cf94fc1770ee0743da"

inherit pypi

RDEPENDS_${PN} = "\
     python3-colorzero \
"

SRC_URI[md5sum] = "3783098afbc8e0997a70e5595a365adf"
SRC_URI[sha256sum] = "ae1a8dc4e6e793ffd8f900968f3290d218052c46347fa0c0503c65fabe422e4d"

BBCLASSEXTEND = "native nativesdk"
