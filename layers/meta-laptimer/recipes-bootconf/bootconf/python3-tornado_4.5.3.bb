inherit setuptools3

SUMMARY = "Tornado is a Python web framework and asynchronous networking library."
SECTION = "devel/python"
HOMEPAGE = "https://pypi.org/project/tornado/"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=3b83ef96387f14655fc854ddc3c6bd57"

inherit pypi

SRC_URI[md5sum] = "a5fc7fd4aea94867609a491ce65786f1"
SRC_URI[sha256sum] = "6d14e47eab0e15799cf3cdcc86b0b98279da68522caace2bd7ce644287685f0a"

BBCLASSEXTEND = "native nativesdk"
