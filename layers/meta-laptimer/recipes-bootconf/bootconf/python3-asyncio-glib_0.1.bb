inherit setuptools3

SUMMARY = "Python asyncio glib integration"
SECTION = "devel/python"
HOMEPAGE = "https://github.com/jhenstridge/asyncio-glib"
LICENSE = "LGPLv2.1"
LIC_FILES_CHKSUM = "file://COPYING.LGPL;md5=4fbd65380cdd255951079008b364516c"

inherit pypi

SRC_URI[md5sum] = "60153055e76ceaacdfbaeafb03d61dd9"
SRC_URI[sha256sum] = "fe3ceb2ba5f541330c07ca1bd7ae792468d625bad1acf5354a3a7a0b9fd87521"

BBCLASSEXTEND = "native nativesdk"

RDEPENDS_${PN} += "python3-pygobject python3-asyncio"
