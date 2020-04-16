inherit setuptools3

SUMMARY = "Python colozero library"
SECTION = "devel/python"
HOMEPAGE = "https://github.com/waveform80/colorzero"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://LICENSE.txt;md5=b2854a8defdf90fac461ff4aba9cc0ea"

inherit pypi

SRC_URI[md5sum] = "7581c4b7fc5f52bf41422e4bbb29731a"
SRC_URI[sha256sum] = "acba47119b5d8555680d3cda9afe6ccc5481385ccc3c00084dd973f7aa184599"

BBCLASSEXTEND = "native nativesdk"
