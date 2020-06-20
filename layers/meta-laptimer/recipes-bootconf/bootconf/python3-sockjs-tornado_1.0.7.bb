inherit setuptools3

SUMMARY = "Python SockJS-tornado is a Python server side counterpart of SockJS-client browser library running on top of Tornado framework."
SECTION = "devel/python"
HOMEPAGE = "https://pypi.org/project/sockjs-tornado/"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=8f68f551a860a1c94f9732b89fc8a331"

inherit pypi

SRC_URI[md5sum] = "dd2548af527ba50e76a8497d1bfe3fd9"
SRC_URI[sha256sum] = "02ff25466b3a46b1a7dbe477340b042770ac078de7ea475a6285a28a75eb1fab"

BBCLASSEXTEND = "native nativesdk"

RDEPENDS_${PN} += "python3-tornado"
