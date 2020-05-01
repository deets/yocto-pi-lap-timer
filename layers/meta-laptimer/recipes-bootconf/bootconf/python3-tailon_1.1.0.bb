inherit setuptools3

SUMMARY = "Python tailon daemon"
SECTION = "devel/python"
HOMEPAGE = "https://github.com/gvalkov/tailon"
LICENSE = "Apache-2"
LIC_FILES_CHKSUM = "file://LICENSE;md5=074a08ee94a1390227edc10317eb0676"

inherit pypi

SRC_URI[md5sum] = "0568a91b80663e8d95c8e0b21714ed2d"
SRC_URI[sha256sum] = "a968a61457e488d1e0b1571f381114f5c3eaa4b6475810a95584761115d2281a"

BBCLASSEXTEND = "native nativesdk"
