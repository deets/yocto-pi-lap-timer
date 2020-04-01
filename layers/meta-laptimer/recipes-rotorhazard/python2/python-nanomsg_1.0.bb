DESCRIPTION = "Python wrapper for nanomsg"
HOMEPAGE = "https://github.com/tonysimpson/nanomsg-python"
LICENSE = "CLOSED"

SRC_URI = "git://github.com/tonysimpson/nanomsg-python.git;tag=${PV}"
SRC_URI[md5sum] = "102bb729d07c1ecbc90bccff07a31be2"

# This is needed because the subdirectory otherwise is PN-git
S = "${WORKDIR}/git"

inherit pypi


RDEPENDS_${PN}_class-target += " \
    ${PYTHON_PN}-crypt \
    ${PYTHON_PN}-ctypes \
    nanomsg \
"

BBCLASSEXTEND = "native"

inherit setuptools
