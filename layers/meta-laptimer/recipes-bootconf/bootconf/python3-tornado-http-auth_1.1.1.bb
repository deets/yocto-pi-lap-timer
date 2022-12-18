inherit setuptools3

SUMMARY = "Digest and basic authentication for the Tornado web framework."
SECTION = "devel/python"
HOMEPAGE = "https://pypi.org/project/tornado-http-auth/"
LICENSE = "Apache-2"
LIC_FILES_CHKSUM = "file://README.rst;beginline=62;md5=2f5d9db500ef36a65b04f1c622e81bb9"

inherit pypi

SRC_URI[md5sum] = "d7a3a5d3a6fe111e9e15a77e56652ef9"
SRC_URI[sha256sum] = "1d6cc051ebf93807138367b3b2f2822c266edeed8bcc9bd2f22526a0802bcc43"

RDEPENDS_${PN}:append = " python3-tornado "

BBCLASSEXTEND = "native nativesdk"
